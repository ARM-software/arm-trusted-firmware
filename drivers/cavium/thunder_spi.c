/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <thunder_private.h>
#include <io_storage.h>
#include <assert.h>
#include <io_driver.h>
#include <debug.h>

#define PLL_REF_CLK 50000000	/* 50 MHz */


typedef struct {
	/* Use the 'in_use' flag as any value for base and file_pos could be
	 * valid.
	 */
	int		in_use;
	unsigned	node;
	unsigned	cs;
	size_t		file_pos;
	size_t		offset_address;
} file_state_t;

static file_state_t current_file = { 0 };

#define	SPI_CS_HIGH		0x04			/* CS active high */
#define	SPI_LSB_FIRST	0x08			/* per-word bits-on-wire */
#define	SPI_3WIRE		0x10			/* SI/SO signals shared */
#define MPI_MAX_DATA	8

#define SPI_NOR_CMD_READ		0x03
#define SPI_NOR_CMD_READ_FAST		0x0b

#define SPI_NOR_CMD_WREN		0x06
#define SPI_NOR_CMD_WRDI		0x04

#define SPI_NOR_CMD_PROGRAM		0x02

#define SPI_NOR_CMD_ERASE		0x20

#define SPI_NOR_CMD_RDSR		0x5

#define SPI_STATUS_WIP			(1 << 0)

#define SPI_ADDRESSING_24BIT		24
#define SPI_ADDRESSING_32BIT		32

#define SPI_PAGE_SIZE			256

int spi_config(uint64_t spi_clk, uint32_t mode, int cpol, int cpha, int cs)
{
	uint64_t sclk;
	union cavm_rst_boot rst_boot;
	union cavm_mpi_cfg mpi_cfg;

	rst_boot.u = CSR_READ_PA(current_file.node, CAVM_RST_BOOT);
	mpi_cfg.u = CSR_READ_PA(current_file.node, CAVM_MPI_CFG);
	sclk = PLL_REF_CLK * rst_boot.s.pnr_mul;

	switch (cs) {
	case 0:
		mpi_cfg.s.csena0 = 1;
		break;
	case 1:
		mpi_cfg.s.csena1 = 1;
		break;
	case 2:
		mpi_cfg.s.csena2 = 1;
		break;
	case 3:
		mpi_cfg.s.csena3 = 1;
		break;
	default:
		break;
	}
	mpi_cfg.s.clkdiv = (sclk >> 1) / spi_clk;
	mpi_cfg.s.cshi = !!(mode & SPI_CS_HIGH);
	mpi_cfg.s.lsbfirst = !!(mode & SPI_LSB_FIRST);
	mpi_cfg.s.wireor = !!(mode & SPI_3WIRE);
	mpi_cfg.s.idlelo = cpha != cpol;
	mpi_cfg.s.cslate = cpha;
	mpi_cfg.s.enable = 1;
	CSR_WRITE_PA(current_file.node, CAVM_MPI_CFG, mpi_cfg.u);

	return 0;
}

int spi_xfer(unsigned char *dout, unsigned char *din, int len, int cs, int last_data)
{
	union cavm_mpi_tx mpi_tx;
	union cavm_mpi_sts mpi_sts;
	int i;

	while (len > 0) {
		int size = MIN(len, MPI_MAX_DATA);

		if (dout) {
			for (i = 0; i < size; i++) {
				CSR_WRITE_PA(current_file.node, CAVM_MPI_DATX(i), *dout++);
			}
		}

		len -= size;
		mpi_tx.u = 0;
		mpi_tx.s.csid = cs;
		mpi_tx.s.leavecs = 1;

		if (last_data && len <= 0)
			mpi_tx.s.leavecs = 0;

		mpi_tx.s.txnum = dout ? size : 0;
		mpi_tx.s.totnum = size;
		CSR_WRITE_PA(current_file.node, CAVM_MPI_TX, mpi_tx.u);

		/* Wait for tx/rx to complete */
		do {
			mpi_sts.u = CSR_READ_PA(current_file.node, CAVM_MPI_STS);
		} while (mpi_sts.s.busy != 0);

		if (din) {
			for (i = 0; i < size; i++) {
				*din++ = CSR_READ_PA(current_file.node, CAVM_MPI_DATX(i));
			}
		}
	}

	return 0;
}

static int spi_nor_read(uint8_t *buf, int buf_size, uint32_t addr, int addr_len, int cs)
{
	uint8_t cmd[9];
	int i;

	if (addr_len != SPI_ADDRESSING_24BIT && addr_len != SPI_ADDRESSING_32BIT) {
		printf("Unsupported addressing mode %d\n", addr_len);
		return -1;
	}

	cmd[0] = SPI_NOR_CMD_READ;

	for (i = 1; i <= (addr_len >> 3); i++)
		cmd[i] = addr >> (addr_len - i * 8);

	if (spi_xfer(cmd, NULL, (addr_len >> 3) + 1, cs, 0))
		return -1;

	if (spi_xfer(NULL, buf, buf_size, cs, 1))
		return -1;

	return buf_size;
}

static int spi_get_status(uint8_t *status, int cs)
{
	uint8_t cmd[1];

	cmd[0] = SPI_NOR_CMD_RDSR;

	if (spi_xfer(cmd, NULL, 1, cs, 0))
		return -1;
	if (spi_xfer(NULL, status, 1, cs, 1))
		return -1;

	return 0;
}

static int spi_nor_write(void *buf, int buf_size, uint32_t addr, int addr_len, int cs, int fast)
{
	uint8_t cmd[9];
	uint8_t status;
	int i;
	int len = buf_size;
	int total = 0;

	if (addr_len != SPI_ADDRESSING_24BIT && addr_len != SPI_ADDRESSING_32BIT) {
		printf("Unsupported addressing mode %d\n", addr_len);
		return -1;
	}

	while (buf_size > 0) {
		len = MIN(buf_size, SPI_PAGE_SIZE);

		cmd[0] = SPI_NOR_CMD_WREN;

		if (spi_xfer(cmd, NULL, 1, cs, 1))
			return -1;

		cmd[0] = SPI_NOR_CMD_PROGRAM;

		for (i = 1; i <= (addr_len >> 3); i++)
			cmd[i] = addr >> (addr_len - i * 8);

		if (spi_xfer(cmd, NULL, (addr_len >> 3) + 1, cs, 0))
			return -1;

		if (spi_xfer(buf, NULL, len, cs, 1))
			return -1;

		do {
			if (spi_get_status(&status, cs) < 0)
				return -1;
		} while (status & SPI_STATUS_WIP);

		buf_size -= len;
		addr += len;
		total += len;
		buf = (char *)buf + len;
	}

	cmd[0] = SPI_NOR_CMD_WRDI;

	if (spi_xfer(cmd, NULL, 1, cs, 1))
		return -1;

	return total;
}

static int spi_nor_erase(uint32_t addr, int addr_len, int cs, int fast)
{
	uint8_t cmd[9];
	uint8_t status;
	int i;

	if (addr_len != SPI_ADDRESSING_24BIT && addr_len != SPI_ADDRESSING_32BIT) {
		printf("Unsupported addressing mode %d\n", addr_len);
		return -1;
	}

	cmd[0] = SPI_NOR_CMD_WREN;

	if (spi_xfer(cmd, NULL, 1, cs, 1))
		return -1;

	cmd[0] = SPI_NOR_CMD_ERASE;

	for (i = 1; i <= (addr_len >> 3); i++)
		cmd[i] = addr >> (addr_len - i * 8);

	if (spi_xfer(cmd, NULL, (addr_len >> 3) + 1, cs, 1))
		return -1;

	cmd[0] = SPI_NOR_CMD_WRDI;

	if (spi_xfer(cmd, NULL, 1, cs, 1))
		return -1;

	do {
		if (spi_get_status(&status, cs))
			return -1;
	} while (status & SPI_STATUS_WIP);

	return 0;
}


/* APIs to read from SPI NOR flash
 *
 */

#define CONFIG_SPI_FREQUENCY	16000000

/* ---- */

/* Identify the device type as spi */
static io_type_t device_type_spi(void)
{
	return IO_TYPE_SPI;
}

static int spi_block_open(io_dev_info_t *dev_info, const uintptr_t spec,
			     io_entity_t *entity)
{
	int result = -ENOMEM;
	const io_block_spec_t *block_spec = (io_block_spec_t *)spec;

	/* Since we need to track open state for seek() we only allow one open
	 * spec at a time. When we have dynamic memory we can malloc and set
	 * entity->info.
	 */
	if (current_file.in_use == 0) {
		assert(block_spec != NULL);
		assert(entity != NULL);

		current_file.in_use = 1;
		// FIXME current_file.cs = block_spec->offset;
		/* File cursor offset for seek and incremental reads etc. */
		current_file.file_pos = 0;
		current_file.offset_address = block_spec->offset;
		current_file.cs = 0; // XXX
		entity->info = (uintptr_t)&current_file;
		current_file.node = cavm_numa_local();

		return spi_config(CONFIG_SPI_FREQUENCY, 0, 0, 0, 0);
	} else {
		WARN("An SPI device is already active. Close first.\n");
	}

	return result;
}

static int spi_block_seek(io_entity_t *entity, int mode,
			     ssize_t offset)
{
	int result = -ENOENT;

	/* We only support IO_SEEK_SET and CUR for the moment. */
	if (mode == IO_SEEK_SET) {
		assert(entity != NULL);

		/* TODO: can we do some basic limit checks on seek? */
		((file_state_t *)entity->info)->file_pos = offset;
		result = 0;
	} else if (mode == IO_SEEK_CUR) {
		assert(entity != NULL);

		/* TODO: can we do some basic limit checks on seek? */
		((file_state_t *)entity->info)->file_pos += offset;
		result = 0;
	}

	return result;
}

static int spi_block_read(io_entity_t *entity, uintptr_t buffer,
			     size_t length, size_t *length_read)
{
	file_state_t *fp;
	ssize_t ret;

	assert(entity != NULL);
	assert(buffer != (uintptr_t)NULL);
	assert(length_read != NULL);

	fp = (file_state_t *)entity->info;

	ret = spi_nor_read((void *)buffer, length, fp->offset_address + fp->file_pos, SPI_ADDRESSING_24BIT, fp->cs);
	if (ret < 0)
		return ret;

	*length_read = ret;
	fp->file_pos += ret;

	return 0;
}

static int spi_block_close(io_entity_t *entity)
{
	assert(entity != NULL);

	entity->info = 0;

	/* This would be a mem free() if we had malloc.*/
	memset((void *)&current_file, 0, sizeof(current_file));

	return 0;
}

static int spi_dev_close(io_dev_info_t *dev_info)
{
	/* NOP */
	/* TODO: Consider tracking open files and cleaning them up here */
	return 0;
}


static const io_dev_funcs_t spi_dev_funcs = {
	.type = device_type_spi,
	.open = spi_block_open,
	.seek = spi_block_seek,
	.size = NULL,
	.read = spi_block_read,
	.write = NULL,
	.close = spi_block_close,
	.dev_init = NULL,
	.dev_close = spi_dev_close,
};


/* No state associated with this device so structure can be const */
static const io_dev_info_t spi_dev_info = {
	.funcs = &spi_dev_funcs,
	.info = (uintptr_t)NULL
};

/* Open a connection to the spi device */
static int spi_dev_open(const uintptr_t dev_spec __attribute__((unused)),
			   io_dev_info_t **dev_info)
{
	assert(dev_info != NULL);
	*dev_info = (io_dev_info_t *)&spi_dev_info; /* cast away const */

	return 0;
}

static const io_dev_connector_t spi_dev_connector = {
	.dev_open = spi_dev_open
};

/*
 * Functions necessary for implementing nor read/write svc calls outside of
 * io_dev abstraction.
 */

int spi_nor_init()
{
	current_file.node = cavm_numa_local();

	spi_config(CONFIG_SPI_FREQUENCY, 0, 0, 0, 0);
	return 0;
}

int spi_nor_rw_data(int write, unsigned long addr, int size, void *buf, int buf_size)
{
	int cs = 0;
	int ret;

	if (buf_size < size) {
		printf("%s::%d  buf size(0x%x) is less than requested read size(0x%x)\n",
				__FILE__, __LINE__, buf_size, size);
		return -1;
	}

	if (!write) {
		ret = spi_nor_read(buf, size, addr, SPI_ADDRESSING_24BIT, cs);
	} else {
		ret = spi_nor_write(buf, size, addr, SPI_ADDRESSING_24BIT, cs, 0);
	}

	return ret;
}

int spi_nor_erase_sect(uint32_t addr)
{
	int cs = 0;

	return spi_nor_erase(addr, SPI_ADDRESSING_24BIT, cs, 0);

}

/* Exported functions */

/* Register the spi driver with the IO abstraction */
int register_io_dev_spi(const io_dev_connector_t **dev_con)
{
	int result;
	assert(dev_con != NULL);

	result = io_register_device(&spi_dev_info);
	if (result == 0)
		*dev_con = &spi_dev_connector;

	return result;
}

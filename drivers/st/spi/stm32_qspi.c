/*
 * Copyright (c) 2019-2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#include <inttypes.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#include <drivers/spi_mem.h>
#include <drivers/st/stm32_gpio.h>
#include <drivers/st/stm32_qspi.h>
#include <drivers/st/stm32mp_reset.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <libfdt.h>

#include <platform_def.h>

/* Timeout for device interface reset */
#define TIMEOUT_US_1_MS			1000U

/* QUADSPI registers */
#define QSPI_CR			0x00U
#define QSPI_DCR		0x04U
#define QSPI_SR			0x08U
#define QSPI_FCR		0x0CU
#define QSPI_DLR		0x10U
#define QSPI_CCR		0x14U
#define QSPI_AR			0x18U
#define QSPI_ABR		0x1CU
#define QSPI_DR			0x20U
#define QSPI_PSMKR		0x24U
#define QSPI_PSMAR		0x28U
#define QSPI_PIR		0x2CU
#define QSPI_LPTR		0x30U

/* QUADSPI control register */
#define QSPI_CR_EN		BIT(0)
#define QSPI_CR_ABORT		BIT(1)
#define QSPI_CR_DMAEN		BIT(2)
#define QSPI_CR_TCEN		BIT(3)
#define QSPI_CR_SSHIFT		BIT(4)
#define QSPI_CR_DFM		BIT(6)
#define QSPI_CR_FSEL		BIT(7)
#define QSPI_CR_FTHRES		GENMASK_32(12, 8)
#define QSPI_CR_FTHRES_SHIFT	8U
#define QSPI_CR_TEIE		BIT(16)
#define QSPI_CR_TCIE		BIT(17)
#define QSPI_CR_FTIE		BIT(18)
#define QSPI_CR_SMIE		BIT(19)
#define QSPI_CR_TOIE		BIT(20)
#define QSPI_CR_APMS		BIT(22)
#define QSPI_CR_PMM		BIT(23)
#define QSPI_CR_PRESCALER_MASK	GENMASK_32(31, 24)
#define QSPI_CR_PRESCALER_SHIFT	24U

/* QUADSPI device configuration register */
#define QSPI_DCR_CKMODE		BIT(0)
#define QSPI_DCR_CSHT_MASK	GENMASK_32(10, 8)
#define QSPI_DCR_CSHT_SHIFT	8U
#define QSPI_DCR_FSIZE_MASK	GENMASK_32(20, 16)
#define QSPI_DCR_FSIZE_SHIFT	16U

/* QUADSPI status register */
#define QSPI_SR_TEF		BIT(0)
#define QSPI_SR_TCF		BIT(1)
#define QSPI_SR_FTF		BIT(2)
#define QSPI_SR_SMF		BIT(3)
#define QSPI_SR_TOF		BIT(4)
#define QSPI_SR_BUSY		BIT(5)

/* QUADSPI flag clear register */
#define QSPI_FCR_CTEF		BIT(0)
#define QSPI_FCR_CTCF		BIT(1)
#define QSPI_FCR_CSMF		BIT(3)
#define QSPI_FCR_CTOF		BIT(4)

/* QUADSPI communication configuration register */
#define QSPI_CCR_DDRM		BIT(31)
#define QSPI_CCR_DHHC		BIT(30)
#define QSPI_CCR_SIOO		BIT(28)
#define QSPI_CCR_FMODE_SHIFT	26U
#define QSPI_CCR_DMODE_SHIFT	24U
#define QSPI_CCR_DCYC_SHIFT	18U
#define QSPI_CCR_ABSIZE_SHIFT	16U
#define QSPI_CCR_ABMODE_SHIFT	14U
#define QSPI_CCR_ADSIZE_SHIFT	12U
#define QSPI_CCR_ADMODE_SHIFT	10U
#define QSPI_CCR_IMODE_SHIFT	8U
#define QSPI_CCR_IND_WRITE	0U
#define QSPI_CCR_IND_READ	1U
#define QSPI_CCR_MEM_MAP	3U

#define QSPI_MAX_CHIP		2U

#define QSPI_FIFO_TIMEOUT_US	30U
#define QSPI_CMD_TIMEOUT_US	1000U
#define QSPI_BUSY_TIMEOUT_US	100U
#define QSPI_ABT_TIMEOUT_US	100U

#define DT_QSPI_COMPAT		"st,stm32f469-qspi"

#define FREQ_100MHZ		100000000U

struct stm32_qspi_ctrl {
	uintptr_t reg_base;
	uintptr_t mm_base;
	size_t mm_size;
	unsigned long clock_id;
	unsigned int reset_id;
};

static struct stm32_qspi_ctrl stm32_qspi;

static uintptr_t qspi_base(void)
{
	return stm32_qspi.reg_base;
}

static int stm32_qspi_wait_for_not_busy(void)
{
	uint64_t timeout = timeout_init_us(QSPI_BUSY_TIMEOUT_US);

	while ((mmio_read_32(qspi_base() + QSPI_SR) & QSPI_SR_BUSY) != 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("%s: busy timeout\n", __func__);
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int stm32_qspi_wait_cmd(const struct spi_mem_op *op)
{
	int ret = 0;
	uint64_t timeout;

	timeout = timeout_init_us(QSPI_CMD_TIMEOUT_US);
	while ((mmio_read_32(qspi_base() + QSPI_SR) & QSPI_SR_TCF) == 0U) {
		if (timeout_elapsed(timeout)) {
			ret = -ETIMEDOUT;
			break;
		}
	}

	if (ret == 0) {
		if ((mmio_read_32(qspi_base() + QSPI_SR) & QSPI_SR_TEF) != 0U) {
			ERROR("%s: transfer error\n", __func__);
			ret = -EIO;
		}
	} else {
		ERROR("%s: cmd timeout\n", __func__);
	}

	/* Clear flags */
	mmio_write_32(qspi_base() + QSPI_FCR, QSPI_FCR_CTCF | QSPI_FCR_CTEF);

	if (ret == 0) {
		ret = stm32_qspi_wait_for_not_busy();
	}

	return ret;
}

static int stm32_qspi_read_fifo(void *val, uintptr_t addr, uint64_t len)
{
	int ret = 0;

	switch (len) {
	case sizeof(uint32_t):
		*((uint32_t *)val) = mmio_read_32(addr);
		break;
	case sizeof(uint8_t):
		*((uint8_t *)val) = mmio_read_8(addr);
		break;
	default:
		ret = -EINVAL;
		break;
	};

	return ret;
}

static int stm32_qspi_write_fifo(void *val, uintptr_t addr, uint64_t len)
{
	int ret = 0;

	switch (len) {
	case sizeof(uint32_t):
		mmio_write_32(addr, *((uint32_t *)val));
		break;
	case sizeof(uint8_t):
		mmio_write_8(addr, *((uint8_t *)val));
		break;
	default:
		ret = -EINVAL;
		break;
	};

	return ret;
}

static int stm32_qspi_poll(const struct spi_mem_op *op)
{
	int (*fifo)(void *val, uintptr_t addr, uint64_t len);
	uint8_t *buff = (uint8_t *)op->data.buf;
	uint64_t length = op->data.nbytes;
	uint32_t tmp;
	uint64_t i = 0U;
	int ret;

	if (op->data.dir == SPI_MEM_DATA_IN) {
		fifo = stm32_qspi_read_fifo;
	} else {
		fifo = stm32_qspi_write_fifo;
	}

	while (length != 0U) {
		uint64_t timeout = timeout_init_us(QSPI_FIFO_TIMEOUT_US);

		while ((mmio_read_32(qspi_base() + QSPI_SR) &
			QSPI_SR_FTF) == 0U) {
			if (timeout_elapsed(timeout)) {
				ERROR("%s: fifo timeout\n", __func__);
				return -ETIMEDOUT;
			}
		}

		if (length >= sizeof(uint32_t)) {
			ret = fifo(&tmp, qspi_base() + QSPI_DR,
				   sizeof(uint32_t));
			if (ret != 0) {
				return ret;
			}

			(void)memcpy((void *)&buff[i], (const void *)&tmp,
				     sizeof(uint32_t));
			length -= sizeof(uint32_t);
			i += sizeof(uint32_t);
		} else {
			ret = fifo(&buff[i], qspi_base() + QSPI_DR,
				   sizeof(uint8_t));
			if (ret != 0) {
				return ret;
			}

			length--;
			i++;
		}
	}

	return 0;
}

static int stm32_qspi_mm(const struct spi_mem_op *op)
{
	uintptr_t from = (uintptr_t)(stm32_qspi.mm_base + op->addr.val);
	uint8_t *buf = (uint8_t *)op->data.buf;
	uint64_t length = op->data.nbytes;
	uint64_t tmp;
	uint64_t i = 0U;

	while ((length != 0U) && ((from & (sizeof(uint64_t) - UL(1))) != 0U)) {
		buf[i] = mmio_read_8(from);
		i++;
		from++;
		length--;
	}

	while (length >= sizeof(uint64_t)) {
		tmp = mmio_read_64(from);
		(void)memcpy((void *)&buf[i], (const void *)&tmp, sizeof(uint64_t));
		i += sizeof(uint64_t);
		from += sizeof(uint64_t);
		length -= sizeof(uint64_t);
	}

	while (length != 0U) {
		buf[i] = mmio_read_8(from);
		i++;
		from++;
		length--;
	}

	return 0;
}

static int stm32_qspi_tx(const struct spi_mem_op *op, uint8_t mode)
{
	if (op->data.nbytes == 0U) {
		return 0;
	}

	if (mode == QSPI_CCR_MEM_MAP) {
		return stm32_qspi_mm(op);
	}

	return stm32_qspi_poll(op);
}

static unsigned int stm32_qspi_get_mode(uint8_t buswidth)
{
	if (buswidth == 4U) {
		return 3U;
	}

	return buswidth;
}

static int stm32_qspi_exec_op(const struct spi_mem_op *op)
{
	uint64_t timeout;
	uint32_t ccr;
	size_t addr_max;
	uint8_t mode = QSPI_CCR_IND_WRITE;
	int ret;

	VERBOSE("%s: cmd:%x mode:%d.%d.%d.%d addr:%" PRIx64 " len:%x\n",
		__func__, op->cmd.opcode, op->cmd.buswidth, op->addr.buswidth,
		op->dummy.buswidth, op->data.buswidth,
		op->addr.val, op->data.nbytes);

	addr_max = op->addr.val + op->data.nbytes + 1U;

	if ((op->data.dir == SPI_MEM_DATA_IN) && (op->data.nbytes != 0U)) {
		if ((addr_max < stm32_qspi.mm_size) &&
		    (op->addr.buswidth != 0U)) {
			mode = QSPI_CCR_MEM_MAP;
		} else {
			mode = QSPI_CCR_IND_READ;
		}
	}

	if (op->data.nbytes != 0U) {
		mmio_write_32(qspi_base() + QSPI_DLR, op->data.nbytes - 1U);
	}

	ccr = mode << QSPI_CCR_FMODE_SHIFT;
	ccr |= op->cmd.opcode;
	ccr |= stm32_qspi_get_mode(op->cmd.buswidth) << QSPI_CCR_IMODE_SHIFT;

	if (op->addr.nbytes != 0U) {
		ccr |= (op->addr.nbytes - 1U) << QSPI_CCR_ADSIZE_SHIFT;
		ccr |= stm32_qspi_get_mode(op->addr.buswidth) <<
			QSPI_CCR_ADMODE_SHIFT;
	}

	if ((op->dummy.buswidth != 0U) && (op->dummy.nbytes != 0U)) {
		ccr |= (op->dummy.nbytes * 8U / op->dummy.buswidth) <<
			QSPI_CCR_DCYC_SHIFT;
	}

	if (op->data.nbytes != 0U) {
		ccr |= stm32_qspi_get_mode(op->data.buswidth) <<
			QSPI_CCR_DMODE_SHIFT;
	}

	mmio_write_32(qspi_base() + QSPI_CCR, ccr);

	if ((op->addr.nbytes != 0U) && (mode != QSPI_CCR_MEM_MAP)) {
		mmio_write_32(qspi_base() + QSPI_AR, op->addr.val);
	}

	ret = stm32_qspi_tx(op, mode);

	/*
	 * Abort in:
	 * - Error case.
	 * - Memory mapped read: prefetching must be stopped if we read the last
	 *   byte of device (device size - fifo size). If device size is not
	 *   known then prefetching is always stopped.
	 */
	if ((ret != 0) || (mode == QSPI_CCR_MEM_MAP)) {
		goto abort;
	}

	/* Wait end of TX in indirect mode */
	ret = stm32_qspi_wait_cmd(op);
	if (ret != 0) {
		goto abort;
	}

	return 0;

abort:
	mmio_setbits_32(qspi_base() + QSPI_CR, QSPI_CR_ABORT);

	/* Wait clear of abort bit by hardware */
	timeout = timeout_init_us(QSPI_ABT_TIMEOUT_US);
	while ((mmio_read_32(qspi_base() + QSPI_CR) & QSPI_CR_ABORT) != 0U) {
		if (timeout_elapsed(timeout)) {
			ret = -ETIMEDOUT;
			break;
		}
	}

	mmio_write_32(qspi_base() + QSPI_FCR, QSPI_FCR_CTCF);

	if (ret != 0) {
		ERROR("%s: exec op error\n", __func__);
	}

	return ret;
}

static int stm32_qspi_claim_bus(unsigned int cs)
{
	uint32_t cr;

	if (cs >= QSPI_MAX_CHIP) {
		return -ENODEV;
	}

	/* Set chip select and enable the controller */
	cr = QSPI_CR_EN;
	if (cs == 1U) {
		cr |= QSPI_CR_FSEL;
	}

	mmio_clrsetbits_32(qspi_base() + QSPI_CR, QSPI_CR_FSEL, cr);

	return 0;
}

static void stm32_qspi_release_bus(void)
{
	mmio_clrbits_32(qspi_base() + QSPI_CR, QSPI_CR_EN);
}

static int stm32_qspi_set_speed(unsigned int hz)
{
	unsigned long qspi_clk = clk_get_rate(stm32_qspi.clock_id);
	uint32_t prescaler = UINT8_MAX;
	uint32_t csht;
	int ret;

	if (qspi_clk == 0U) {
		return -EINVAL;
	}

	if (hz > 0U) {
		prescaler = div_round_up(qspi_clk, hz) - 1U;
		if (prescaler > UINT8_MAX) {
			prescaler = UINT8_MAX;
		}
	}

	csht = div_round_up((5U * qspi_clk) / (prescaler + 1U), FREQ_100MHZ);
	csht = ((csht - 1U) << QSPI_DCR_CSHT_SHIFT) & QSPI_DCR_CSHT_MASK;

	ret = stm32_qspi_wait_for_not_busy();
	if (ret != 0) {
		return ret;
	}

	mmio_clrsetbits_32(qspi_base() + QSPI_CR, QSPI_CR_PRESCALER_MASK,
			   prescaler << QSPI_CR_PRESCALER_SHIFT);

	mmio_clrsetbits_32(qspi_base() + QSPI_DCR, QSPI_DCR_CSHT_MASK, csht);

	VERBOSE("%s: speed=%lu\n", __func__, qspi_clk / (prescaler + 1U));

	return 0;
}

static int stm32_qspi_set_mode(unsigned int mode)
{
	int ret;

	ret = stm32_qspi_wait_for_not_busy();
	if (ret != 0) {
		return ret;
	}

	if ((mode & SPI_CS_HIGH) != 0U) {
		return -ENODEV;
	}

	if (((mode & SPI_CPHA) != 0U) && ((mode & SPI_CPOL) != 0U)) {
		mmio_setbits_32(qspi_base() + QSPI_DCR, QSPI_DCR_CKMODE);
	} else if (((mode & SPI_CPHA) == 0U) && ((mode & SPI_CPOL) == 0U)) {
		mmio_clrbits_32(qspi_base() + QSPI_DCR, QSPI_DCR_CKMODE);
	} else {
		return -ENODEV;
	}

	VERBOSE("%s: mode=0x%x\n", __func__, mode);

	if ((mode & SPI_RX_QUAD) != 0U) {
		VERBOSE("rx: quad\n");
	} else if ((mode & SPI_RX_DUAL) != 0U) {
		VERBOSE("rx: dual\n");
	} else {
		VERBOSE("rx: single\n");
	}

	if ((mode & SPI_TX_QUAD) != 0U) {
		VERBOSE("tx: quad\n");
	} else if ((mode & SPI_TX_DUAL) != 0U) {
		VERBOSE("tx: dual\n");
	} else {
		VERBOSE("tx: single\n");
	}

	return 0;
}

static const struct spi_bus_ops stm32_qspi_bus_ops = {
	.claim_bus = stm32_qspi_claim_bus,
	.release_bus = stm32_qspi_release_bus,
	.set_speed = stm32_qspi_set_speed,
	.set_mode = stm32_qspi_set_mode,
	.exec_op = stm32_qspi_exec_op,
};

int stm32_qspi_init(void)
{
	size_t size;
	int qspi_node;
	struct dt_node_info info;
	void *fdt = NULL;
	int ret;

	if (fdt_get_address(&fdt) == 0) {
		return -FDT_ERR_NOTFOUND;
	}

	qspi_node = dt_get_node(&info, -1, DT_QSPI_COMPAT);
	if (qspi_node < 0) {
		ERROR("No QSPI ctrl found\n");
		return -FDT_ERR_NOTFOUND;
	}

	if (info.status == DT_DISABLED) {
		return -FDT_ERR_NOTFOUND;
	}

	ret = fdt_get_reg_props_by_name(fdt, qspi_node, "qspi",
					&stm32_qspi.reg_base, &size);
	if (ret != 0) {
		return ret;
	}

	ret = fdt_get_reg_props_by_name(fdt, qspi_node, "qspi_mm",
					&stm32_qspi.mm_base,
					&stm32_qspi.mm_size);
	if (ret != 0) {
		return ret;
	}

	if (dt_set_pinctrl_config(qspi_node) != 0) {
		return -FDT_ERR_BADVALUE;
	}

	if ((info.clock < 0) || (info.reset < 0)) {
		return -FDT_ERR_BADVALUE;
	}

	stm32_qspi.clock_id = (unsigned long)info.clock;
	stm32_qspi.reset_id = (unsigned int)info.reset;

	clk_enable(stm32_qspi.clock_id);

	ret = stm32mp_reset_assert(stm32_qspi.reset_id, TIMEOUT_US_1_MS);
	if (ret != 0) {
		panic();
	}
	ret = stm32mp_reset_deassert(stm32_qspi.reset_id, TIMEOUT_US_1_MS);
	if (ret != 0) {
		panic();
	}

	mmio_write_32(qspi_base() + QSPI_CR, QSPI_CR_SSHIFT);
	mmio_write_32(qspi_base() + QSPI_DCR, QSPI_DCR_FSIZE_MASK);
	mmio_clrsetbits_32(qspi_base() + QSPI_CR, QSPI_CR_FTHRES,
			   3U << QSPI_CR_FTHRES_SHIFT);

	return spi_mem_init_slave(fdt, qspi_node, &stm32_qspi_bus_ops);
};

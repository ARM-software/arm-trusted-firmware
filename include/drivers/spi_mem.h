/*
 * Copyright (c) 2019-2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DRIVERS_SPI_MEM_H
#define DRIVERS_SPI_MEM_H

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#define SPI_MEM_BUSWIDTH_1_LINE		1U
#define SPI_MEM_BUSWIDTH_2_LINE		2U
#define SPI_MEM_BUSWIDTH_4_LINE		4U
#define SPI_MEM_BUSWIDTH_8_LINE		8U

/*
 * enum spi_mem_data_dir - Describes the direction of a SPI memory data
 *			   transfer from the controller perspective.
 * @SPI_MEM_DATA_IN: data coming from the SPI memory.
 * @SPI_MEM_DATA_OUT: data sent to the SPI memory.
 */
enum spi_mem_data_dir {
	SPI_MEM_DATA_IN,
	SPI_MEM_DATA_OUT,
};

/*
 * struct spi_mem_op - Describes a SPI memory operation.
 *
 * @cmd.nbytes: number of opcode bytes (only 1 or 2 are valid). The opcode is
 *		sent MSB-first.
 * @cmd.buswidth: Number of IO lines used to transmit the command.
 * @cmd.opcode: Operation opcode.
 * @cmd.dtr: whether the command opcode should be sent in DTR mode or not
 * @addr.nbytes: Number of address bytes to send. Can be zero if the operation
 *		 does not need to send an address.
 * @addr.buswidth: Number of IO lines used to transmit the address.
 * @addr.val: Address value. This value is always sent MSB first on the bus.
 *	      Note that only @addr.nbytes are taken into account in this
 *	      address value, so users should make sure the value fits in the
 *	      assigned number of bytes.
 * @addr.dtr: whether the address should be sent in DTR mode or not
 * @dummy.nbytes: Number of dummy bytes to send after an opcode or address. Can
 *		  be zero if the operation does not require dummy bytes.
 * @dummy.buswidth: Number of IO lines used to transmit the dummy bytes.
 * @dummy.dtr: whether the dummy bytes should be sent in DTR mode or not
 * @data.buswidth: Number of IO lines used to send/receive the data.
 * @data.dtr: whether the data should be sent in DTR mode or not
 * @data.dtr_swab16: whether the byte order of 16-bit words is swapped when read
 *		     or written in Octal DTR mode compared to STR mode.
 * @data.dir: Direction of the transfer.
 * @data.nbytes: Number of data bytes to transfer.
 * @data.buf: Input or output data buffer depending on data::dir.
 */
struct spi_mem_op {
	struct {
		uint8_t nbytes;
		uint8_t buswidth;
		bool dtr;
		uint16_t opcode;
	} cmd;

	struct {
		uint8_t nbytes;
		uint8_t buswidth;
		bool dtr;
		uint64_t val;
	} addr;

	struct {
		uint8_t nbytes;
		uint8_t buswidth;
		bool dtr;
	} dummy;

	struct {
		uint8_t buswidth;
		bool dtr;
		bool dtr_swab16;
		enum spi_mem_data_dir dir;
		unsigned int nbytes;
		void *buf;
	} data;
};

/* SPI mode flags */
#define SPI_CPHA	BIT(0)			/* clock phase */
#define SPI_CPOL	BIT(1)			/* clock polarity */
#define SPI_CS_HIGH	BIT(2)			/* CS active high */
#define SPI_LSB_FIRST	BIT(3)			/* per-word bits-on-wire */
#define SPI_3WIRE	BIT(4)			/* SI/SO signals shared */
#define SPI_PREAMBLE	BIT(5)			/* Skip preamble bytes */
#define SPI_TX_DUAL	BIT(6)			/* transmit with 2 wires */
#define SPI_TX_QUAD	BIT(7)			/* transmit with 4 wires */
#define SPI_RX_DUAL	BIT(8)			/* receive with 2 wires */
#define SPI_RX_QUAD	BIT(9)			/* receive with 4 wires */
#define SPI_TX_OCTAL	BIT(10)			/* transmit with 8 wires */
#define SPI_RX_OCTAL	BIT(11)			/* receive with 8 wires */

struct spi_bus_ops {
	/*
	 * Claim the bus and prepare it for communication.
	 *
	 * @cs:	The chip select.
	 * Returns: 0 if the bus was claimed successfully, or a negative value
	 * if it wasn't.
	 */
	int (*claim_bus)(unsigned int cs);

	/*
	 * Release the SPI bus.
	 */
	void (*release_bus)(void);

	/*
	 * Set transfer speed.
	 *
	 * @hz:	The transfer speed in Hertz.
	 * Returns: 0 on success, a negative error code otherwise.
	 */
	int (*set_speed)(unsigned int hz);

	/*
	 * Set the SPI mode/flags.
	 *
	 * @mode: Requested SPI mode (SPI_... flags).
	 * Returns: 0 on success, a negative error code otherwise.
	 */
	int (*set_mode)(unsigned int mode);

	/*
	 * Execute a SPI memory operation.
	 *
	 * @op:	The memory operation to execute.
	 * Returns: 0 on success, a negative error code otherwise.
	 */
	int (*exec_op)(const struct spi_mem_op *op);

	/*
	 * Read data through a direct mapping.
	 *
	 * @op: The memory operation to execute.
	 * Returns: 0 on success, a negative error code otherwise.
	 */
	int (*dirmap_read)(const struct spi_mem_op *op);

	/*
	 * Check if an operation is supported by the controller.
	 *
	 * @op: The memory operation to execute.
	 * Returns: 0 on success, a negative error code otherwise.
	 */
	bool (*supports_op)(const struct spi_mem_op *op);
};

bool spi_mem_dtr_supports_op(const struct spi_mem_op *op);
bool spi_mem_default_supports_op(const struct spi_mem_op *op);
int spi_mem_exec_op(const struct spi_mem_op *op);
int spi_mem_dirmap_read(const struct spi_mem_op *op);
int spi_mem_init_slave(void *fdt, int bus_node,
		       const struct spi_bus_ops *ops);

#endif /* DRIVERS_SPI_MEM_H */

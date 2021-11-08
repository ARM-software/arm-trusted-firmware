/*
 * Copyright (c) 2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

#include <libfdt.h>

#include <drivers/spi_mem.h>
#include <lib/utils_def.h>

#define SPI_MEM_DEFAULT_SPEED_HZ 100000U

/*
 * struct spi_slave - Representation of a SPI slave.
 *
 * @max_hz:		Maximum speed for this slave in Hertz.
 * @cs:			ID of the chip select connected to the slave.
 * @mode:		SPI mode to use for this slave (see SPI mode flags).
 * @ops:		Ops defined by the bus.
 */
struct spi_slave {
	unsigned int max_hz;
	unsigned int cs;
	unsigned int mode;
	const struct spi_bus_ops *ops;
};

static struct spi_slave spi_slave;

static bool spi_mem_check_buswidth_req(uint8_t buswidth, bool tx)
{
	switch (buswidth) {
	case 1U:
		return true;

	case 2U:
		if ((tx && (spi_slave.mode & (SPI_TX_DUAL | SPI_TX_QUAD)) !=
		     0U) ||
		    (!tx && (spi_slave.mode & (SPI_RX_DUAL | SPI_RX_QUAD)) !=
		     0U)) {
			return true;
		}
		break;

	case 4U:
		if ((tx && (spi_slave.mode & SPI_TX_QUAD) != 0U) ||
		    (!tx && (spi_slave.mode & SPI_RX_QUAD) != 0U)) {
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}

static bool spi_mem_supports_op(const struct spi_mem_op *op)
{
	if (!spi_mem_check_buswidth_req(op->cmd.buswidth, true)) {
		return false;
	}

	if ((op->addr.nbytes != 0U) &&
	    !spi_mem_check_buswidth_req(op->addr.buswidth, true)) {
		return false;
	}

	if ((op->dummy.nbytes != 0U) &&
	    !spi_mem_check_buswidth_req(op->dummy.buswidth, true)) {
		return false;
	}

	if ((op->data.nbytes != 0U) &&
	    !spi_mem_check_buswidth_req(op->data.buswidth,
				       op->data.dir == SPI_MEM_DATA_OUT)) {
		return false;
	}

	return true;
}

static int spi_mem_set_speed_mode(void)
{
	const struct spi_bus_ops *ops = spi_slave.ops;
	int ret;

	ret = ops->set_speed(spi_slave.max_hz);
	if (ret != 0) {
		VERBOSE("Cannot set speed (err=%d)\n", ret);
		return ret;
	}

	ret = ops->set_mode(spi_slave.mode);
	if (ret != 0) {
		VERBOSE("Cannot set mode (err=%d)\n", ret);
		return ret;
	}

	return 0;
}

static int spi_mem_check_bus_ops(const struct spi_bus_ops *ops)
{
	bool error = false;

	if (ops->claim_bus == NULL) {
		VERBOSE("Ops claim bus is not defined\n");
		error = true;
	}

	if (ops->release_bus == NULL) {
		VERBOSE("Ops release bus is not defined\n");
		error = true;
	}

	if (ops->exec_op == NULL) {
		VERBOSE("Ops exec op is not defined\n");
		error = true;
	}

	if (ops->set_speed == NULL) {
		VERBOSE("Ops set speed is not defined\n");
		error = true;
	}

	if (ops->set_mode == NULL) {
		VERBOSE("Ops set mode is not defined\n");
		error = true;
	}

	return error ? -EINVAL : 0;
}

/*
 * spi_mem_exec_op() - Execute a memory operation.
 * @op: The memory operation to execute.
 *
 * This function first checks that @op is supported and then tries to execute
 * it.
 *
 * Return: 0 in case of success, a negative error code otherwise.
 */
int spi_mem_exec_op(const struct spi_mem_op *op)
{
	const struct spi_bus_ops *ops = spi_slave.ops;
	int ret;

	VERBOSE("%s: cmd:%x mode:%d.%d.%d.%d addqr:%" PRIx64 " len:%x\n",
		__func__, op->cmd.opcode, op->cmd.buswidth, op->addr.buswidth,
		op->dummy.buswidth, op->data.buswidth,
		op->addr.val, op->data.nbytes);

	if (!spi_mem_supports_op(op)) {
		WARN("Error in spi_mem_support\n");
		return -ENOTSUP;
	}

	ret = ops->claim_bus(spi_slave.cs);
	if (ret != 0) {
		WARN("Error claim_bus\n");
		return ret;
	}

	ret = ops->exec_op(op);

	ops->release_bus();

	return ret;
}

/*
 * spi_mem_init_slave() - SPI slave device initialization.
 * @fdt: Pointer to the device tree blob.
 * @bus_node: Offset of the bus node.
 * @ops: The SPI bus ops defined.
 *
 * This function first checks that @ops are supported and then tries to find
 * a SPI slave device.
 *
 * Return: 0 in case of success, a negative error code otherwise.
 */
int spi_mem_init_slave(void *fdt, int bus_node, const struct spi_bus_ops *ops)
{
	int ret;
	int mode = 0;
	int nchips = 0;
	int bus_subnode = 0;
	const fdt32_t *cuint = NULL;

	ret = spi_mem_check_bus_ops(ops);
	if (ret != 0) {
		return ret;
	}

	fdt_for_each_subnode(bus_subnode, fdt, bus_node) {
		nchips++;
	}

	if (nchips != 1) {
		ERROR("Only one SPI device is currently supported\n");
		return -EINVAL;
	}

	fdt_for_each_subnode(bus_subnode, fdt, bus_node) {
		/* Get chip select */
		cuint = fdt_getprop(fdt, bus_subnode, "reg", NULL);
		if (cuint == NULL) {
			ERROR("Chip select not well defined\n");
			return -EINVAL;
		}
		spi_slave.cs = fdt32_to_cpu(*cuint);

		/* Get max slave frequency */
		spi_slave.max_hz = SPI_MEM_DEFAULT_SPEED_HZ;
		cuint = fdt_getprop(fdt, bus_subnode,
				    "spi-max-frequency", NULL);
		if (cuint != NULL) {
			spi_slave.max_hz = fdt32_to_cpu(*cuint);
		}

		/* Get mode */
		if ((fdt_getprop(fdt, bus_subnode, "spi-cpol", NULL)) != NULL) {
			mode |= SPI_CPOL;
		}
		if ((fdt_getprop(fdt, bus_subnode, "spi-cpha", NULL)) != NULL) {
			mode |= SPI_CPHA;
		}
		if ((fdt_getprop(fdt, bus_subnode, "spi-cs-high", NULL)) !=
		    NULL) {
			mode |= SPI_CS_HIGH;
		}
		if ((fdt_getprop(fdt, bus_subnode, "spi-3wire", NULL)) !=
		    NULL) {
			mode |= SPI_3WIRE;
		}
		if ((fdt_getprop(fdt, bus_subnode, "spi-half-duplex", NULL)) !=
		    NULL) {
			mode |= SPI_PREAMBLE;
		}

		/* Get dual/quad mode */
		cuint = fdt_getprop(fdt, bus_subnode, "spi-tx-bus-width", NULL);
		if (cuint != NULL) {
			switch (fdt32_to_cpu(*cuint)) {
			case 1U:
				break;
			case 2U:
				mode |= SPI_TX_DUAL;
				break;
			case 4U:
				mode |= SPI_TX_QUAD;
				break;
			default:
				WARN("spi-tx-bus-width %d not supported\n",
				     fdt32_to_cpu(*cuint));
				return -EINVAL;
			}
		}

		cuint = fdt_getprop(fdt, bus_subnode, "spi-rx-bus-width", NULL);
		if (cuint != NULL) {
			switch (fdt32_to_cpu(*cuint)) {
			case 1U:
				break;
			case 2U:
				mode |= SPI_RX_DUAL;
				break;
			case 4U:
				mode |= SPI_RX_QUAD;
				break;
			default:
				WARN("spi-rx-bus-width %d not supported\n",
				     fdt32_to_cpu(*cuint));
				return -EINVAL;
			}
		}

		spi_slave.mode = mode;
		spi_slave.ops = ops;
	}

	return spi_mem_set_speed_mode();
}

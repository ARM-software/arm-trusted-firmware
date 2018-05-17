/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Zynq UltraScale+ MPSoC IPI agent registers access management
 */

#include <bakery_lock.h>
#include <debug.h>
#include <errno.h>
#include <mmio.h>
#include <runtime_svc.h>
#include <string.h>
#include "zynqmp_ipi.h"
#include "../zynqmp_private.h"

/*********************************************************************
 * Macros definitions
 ********************************************************************/

/* IPI registers base address */
#define IPI_REGS_BASE   0xFF300000U

/* IPI registers offsets macros */
#define IPI_TRIG_OFFSET 0x00U
#define IPI_OBR_OFFSET  0x04U
#define IPI_ISR_OFFSET  0x10U
#define IPI_IMR_OFFSET  0x14U
#define IPI_IER_OFFSET  0x18U
#define IPI_IDR_OFFSET  0x1CU

/* IPI register start offset */
#define IPI_REG_BASE(I) (zynqmp_ipi_table[(I)].ipi_reg_base)

/* IPI register bit mask */
#define IPI_BIT_MASK(I) (zynqmp_ipi_table[(I)].ipi_bit_mask)

/* IPI secure check */
#define IPI_SECURE_MASK  0x1U
#define IPI_IS_SECURE(I) ((zynqmp_ipi_table[(I)].secure_only & \
			   IPI_SECURE_MASK) ? 1 : 0)

/*********************************************************************
 * Struct definitions
 ********************************************************************/

/* structure to maintain IPI configuration information */
struct zynqmp_ipi_config {
	unsigned int ipi_bit_mask;
	unsigned int ipi_reg_base;
	unsigned char secure_only;
};

/* Zynqmp ipi configuration table */
const static struct zynqmp_ipi_config zynqmp_ipi_table[] = {
	/* APU IPI */
	{
		.ipi_bit_mask = 0x1,
		.ipi_reg_base = 0xFF300000,
		.secure_only = 0,
	},
	/* RPU0 IPI */
	{
		.ipi_bit_mask = 0x100,
		.ipi_reg_base = 0xFF310000,
		.secure_only = 0,
	},
	/* RPU1 IPI */
	{
		.ipi_bit_mask = 0x200,
		.ipi_reg_base = 0xFF320000,
		.secure_only = 0,
	},
	/* PMU0 IPI */
	{
		.ipi_bit_mask = 0x10000,
		.ipi_reg_base = 0xFF330000,
		.secure_only = IPI_SECURE_MASK,
	},
	/* PMU1 IPI */
	{
		.ipi_bit_mask = 0x20000,
		.ipi_reg_base = 0xFF331000,
		.secure_only = 0,
	},
	/* PMU2 IPI */
	{
		.ipi_bit_mask = 0x40000,
		.ipi_reg_base = 0xFF332000,
		.secure_only = IPI_SECURE_MASK,
	},
	/* PMU3 IPI */
	{
		.ipi_bit_mask = 0x80000,
		.ipi_reg_base = 0xFF333000,
		.secure_only = IPI_SECURE_MASK,
	},
	/* PL0 IPI */
	{
		.ipi_bit_mask = 0x1000000,
		.ipi_reg_base = 0xFF340000,
		.secure_only = 0,
	},
	/* PL1 IPI */
	{
		.ipi_bit_mask = 0x2000000,
		.ipi_reg_base = 0xFF350000,
		.secure_only = 0,
	},
	/* PL2 IPI */
	{
		.ipi_bit_mask = 0x4000000,
		.ipi_reg_base = 0xFF360000,
		.secure_only = 0,
	},
	/* PL3 IPI */
	{
		.ipi_bit_mask = 0x8000000,
		.ipi_reg_base = 0xFF370000,
		.secure_only = 0,
	},
};

/* is_ipi_mb_within_range() - verify if IPI mailbox is within range
 *
 * @local  - local IPI ID
 * @remote - remote IPI ID
 *
 * return - 1 if within range, 0 if not
 */
static inline int is_ipi_mb_within_range(uint32_t local, uint32_t remote)
{
	int ret = 1;
	uint32_t ipi_total = ARRAY_SIZE(zynqmp_ipi_table);

	if (remote >= ipi_total || local >= ipi_total)
		ret = 0;

	return ret;
}

/**
 * ipi_mb_validate() - validate IPI mailbox access
 *
 * @local  - local IPI ID
 * @remote - remote IPI ID
 * @is_secure - indicate if the requester is from secure software
 *
 * return - 0 success, negative value for errors
 */
int ipi_mb_validate(uint32_t local, uint32_t remote, unsigned int is_secure)
{
	int ret = 0;

	if (!is_ipi_mb_within_range(local, remote))
		ret = -EINVAL;
	else if (IPI_IS_SECURE(local) && !is_secure)
		ret = -EPERM;
	else if (IPI_IS_SECURE(remote) && !is_secure)
		ret = -EPERM;

	return ret;
}

/**
 * ipi_mb_open() - Open IPI mailbox.
 *
 * @local  - local IPI ID
 * @remote - remote IPI ID
 *
 */
void ipi_mb_open(uint32_t local, uint32_t remote)
{
	mmio_write_32(IPI_REG_BASE(local) + IPI_IDR_OFFSET,
		      IPI_BIT_MASK(remote));
	mmio_write_32(IPI_REG_BASE(local) + IPI_ISR_OFFSET,
		      IPI_BIT_MASK(remote));
}

/**
 * ipi_mb_release() - Open IPI mailbox.
 *
 * @local  - local IPI ID
 * @remote - remote IPI ID
 *
 */
void ipi_mb_release(uint32_t local, uint32_t remote)
{
	mmio_write_32(IPI_REG_BASE(local) + IPI_IDR_OFFSET,
		      IPI_BIT_MASK(remote));
}

/**
 * ipi_mb_enquire_status() - Enquire IPI mailbox status
 *
 * @local  - local IPI ID
 * @remote - remote IPI ID
 *
 * return - 0 idle, positive value for pending sending or receiving,
 *          negative value for errors
 */
int ipi_mb_enquire_status(uint32_t local, uint32_t remote)
{
	int ret = 0;
	uint32_t status;

	status = mmio_read_32(IPI_REG_BASE(local) + IPI_OBR_OFFSET);
	if (status & IPI_BIT_MASK(remote))
		ret |= IPI_MB_STATUS_SEND_PENDING;
	status = mmio_read_32(IPI_REG_BASE(local) + IPI_ISR_OFFSET);
	if (status & IPI_BIT_MASK(remote))
		ret |= IPI_MB_STATUS_RECV_PENDING;

	return ret;
}

/* ipi_mb_notify() - Trigger IPI mailbox notification
 *
 * @local - local IPI ID
 * @remote - remote IPI ID
 * @is_blocking - if to trigger the notification in blocking mode or not.
 *
 * It sets the remote bit in the IPI agent trigger register.
 *
 */
void ipi_mb_notify(uint32_t local, uint32_t remote, uint32_t is_blocking)
{
	uint32_t status;

	mmio_write_32(IPI_REG_BASE(local) + IPI_TRIG_OFFSET,
		      IPI_BIT_MASK(remote));
	if (is_blocking) {
		do {
			status = mmio_read_32(IPI_REG_BASE(local) +
					      IPI_OBR_OFFSET);
		} while (status & IPI_BIT_MASK(remote));
	}
}

/* ipi_mb_ack() - Ack IPI mailbox notification from the other end
 *
 * @local - local IPI ID
 * @remote - remote IPI ID
 *
 * It will clear the remote bit in the isr register.
 *
 */
void ipi_mb_ack(uint32_t local, uint32_t remote)
{
	mmio_write_32(IPI_REG_BASE(local) + IPI_ISR_OFFSET,
		      IPI_BIT_MASK(remote));
}

/* ipi_mb_disable_irq() - Disable IPI mailbox notification interrupt
 *
 * @local - local IPI ID
 * @remote - remote IPI ID
 *
 * It will mask the remote bit in the idr register.
 *
 */
void ipi_mb_disable_irq(uint32_t local, uint32_t remote)
{
	mmio_write_32(IPI_REG_BASE(local) + IPI_IDR_OFFSET,
		      IPI_BIT_MASK(remote));
}

/* ipi_mb_enable_irq() - Enable IPI mailbox notification interrupt
 *
 * @local - local IPI ID
 * @remote - remote IPI ID
 *
 * It will mask the remote bit in the idr register.
 *
 */
void ipi_mb_enable_irq(uint32_t local, uint32_t remote)
{
	mmio_write_32(IPI_REG_BASE(local) + IPI_IER_OFFSET,
		      IPI_BIT_MASK(remote));
}

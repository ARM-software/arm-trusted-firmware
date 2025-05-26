/*
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2020-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2025, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Xilinx IPI agent registers access management
 */

#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>

#include <ipi.h>
#include <plat_private.h>
#include "pm_defs.h"

/*********************************************************************
 * Macros definitions
 ********************************************************************/

/* IPI registers offsets macros */
#define IPI_TRIG_OFFSET 0x00U
#define IPI_OBR_OFFSET  0x04U
#define IPI_ISR_OFFSET  0x10U
#define IPI_IMR_OFFSET  0x14U
#define IPI_IER_OFFSET  0x18U
#define IPI_IDR_OFFSET  0x1CU

/* IPI register start offset */
#define IPI_REG_BASE(I) (ipi_table[(I)].ipi_reg_base)

/* IPI register bit mask */
#define IPI_BIT_MASK(I) (ipi_table[(I)].ipi_bit_mask)

/* IPI configuration table */
static const struct ipi_config *ipi_table;

/* Total number of IPI */
static uint32_t ipi_total;

/**
 * ipi_config_table_init() - Initialize IPI configuration data.
 * @ipi_config_table: IPI configuration table.
 * @total_ipi: Total number of IPI available.
 *
 */
void ipi_config_table_init(const struct ipi_config *ipi_config_table,
			   uint32_t total_ipi)
{
	ipi_table = ipi_config_table;
	ipi_total = total_ipi;
}

/**
 * is_ipi_mb_within_range() - verify if IPI mailbox is within range.
 * @local: local IPI ID.
 * @remote: remote IPI ID.
 *
 * Return: - 1 if within range, 0 if not.
 *
 */
static inline uint32_t is_ipi_mb_within_range(uint32_t local, uint32_t remote)
{
	uint32_t ret = 1U;

	if ((remote >= ipi_total) || (local >= ipi_total)) {
		ret = 0U;
	}

	return ret;
}

/**
 * ipi_mb_validate() - validate IPI mailbox access.
 * @local: local IPI ID.
 * @remote: remote IPI ID.
 * @is_secure: indicate if the requester is from secure software.
 *
 * Return: 0 success, negative value for errors.
 *
 */
int32_t ipi_mb_validate(uint32_t local, uint32_t remote, uint32_t is_secure)
{
	int32_t ret = 0;

	if (is_ipi_mb_within_range(local, remote) == 0U) {
		ret = -EINVAL;
	} else if (IPI_IS_SECURE(local) && (is_secure == 0U)) {
		ret = -EPERM;
	} else if (IPI_IS_SECURE(remote) && (is_secure == 0U)) {
		ret = -EPERM;
	} else {
		/* To fix the misra 15.7 warning */
	}

	return ret;
}

/**
 * ipi_mb_open() - Open IPI mailbox.
 * @local: local IPI ID.
 * @remote: remote IPI ID.
 *
 */
void ipi_mb_open(uint32_t local, uint32_t remote)
{
	uint64_t idr_offset = (uint64_t)(IPI_REG_BASE(local) + IPI_IDR_OFFSET);
	uint64_t isr_offset = (uint64_t)(IPI_REG_BASE(local) + IPI_ISR_OFFSET);

	mmio_write_32(idr_offset,
		      IPI_BIT_MASK(remote));
	mmio_write_32(isr_offset,
		      IPI_BIT_MASK(remote));
}

/**
 * ipi_mb_release() - Open IPI mailbox.
 * @local: local IPI ID.
 * @remote: remote IPI ID.
 *
 */
void ipi_mb_release(uint32_t local, uint32_t remote)
{
	uint64_t idr_offset = (uint64_t)(IPI_REG_BASE(local) + IPI_IDR_OFFSET);

	mmio_write_32(idr_offset,
		      IPI_BIT_MASK(remote));
}

/**
 * ipi_mb_enquire_status() - Enquire IPI mailbox status.
 * @local: local IPI ID.
 * @remote: remote IPI ID.
 *
 * Return: 0 idle and positive value for pending sending or receiving.
 *
 */
uint32_t ipi_mb_enquire_status(uint32_t local, uint32_t remote)
{
	uint32_t ret = (uint32_t)PM_RET_SUCCESS;
	uint32_t status;
	uint64_t obr_offset = (uint64_t)(IPI_REG_BASE(local) + IPI_OBR_OFFSET);
	uint64_t isr_offset = (uint64_t)(IPI_REG_BASE(local) + IPI_ISR_OFFSET);

	status = mmio_read_32(obr_offset);
	if ((status & IPI_BIT_MASK(remote)) != 0U) {
		ret |= IPI_MB_STATUS_SEND_PENDING;
	}
	status = mmio_read_32(isr_offset);
	if ((status & IPI_BIT_MASK(remote)) != 0U) {
		ret |= IPI_MB_STATUS_RECV_PENDING;
	}

	return ret;
}

/**
 * ipi_mb_notify() - Trigger IPI mailbox notification.
 * @local: local IPI ID.
 * @remote: remote IPI ID.
 * @is_blocking: if to trigger the notification in blocking mode or not.
 *
 * It sets the remote bit in the IPI agent trigger register.
 *
 */
void ipi_mb_notify(uint32_t local, uint32_t remote, uint32_t is_blocking)
{
	uint32_t status;
	uint64_t trig_offset = (uint64_t)(IPI_REG_BASE(local) + IPI_TRIG_OFFSET);
	uint64_t obr_offset = (uint64_t)(IPI_REG_BASE(local) + IPI_OBR_OFFSET);

	mmio_write_32(trig_offset,
		      IPI_BIT_MASK(remote));
	if (is_blocking != 0U) {
		do {
			status = mmio_read_32(obr_offset);
		} while ((status & IPI_BIT_MASK(remote)) != 0U);
	}
}

/**
 * ipi_mb_ack() - Ack IPI mailbox notification from the other end.
 * @local: local IPI ID.
 * @remote: remote IPI ID.
 *
 * It will clear the remote bit in the isr register.
 *
 */
void ipi_mb_ack(uint32_t local, uint32_t remote)
{
	uint64_t isr_offset = (uint64_t)(IPI_REG_BASE(local) + IPI_ISR_OFFSET);

	mmio_write_32(isr_offset,
		      IPI_BIT_MASK(remote));
}

/**
 * ipi_mb_disable_irq() - Disable IPI mailbox notification interrupt.
 * @local: local IPI ID.
 * @remote: remote IPI ID.
 *
 * It will mask the remote bit in the idr register.
 *
 */
void ipi_mb_disable_irq(uint32_t local, uint32_t remote)
{
	uint64_t idr_offset = (uint64_t)(IPI_REG_BASE(local) + IPI_IDR_OFFSET);

	mmio_write_32(idr_offset,
		      IPI_BIT_MASK(remote));
}

/**
 * ipi_mb_enable_irq() - Enable IPI mailbox notification interrupt.
 * @local: local IPI ID.
 * @remote: remote IPI ID.
 *
 * It will mask the remote bit in the idr register.
 *
 */
void ipi_mb_enable_irq(uint32_t local, uint32_t remote)
{
	uint64_t ier_offset = (uint64_t)(IPI_REG_BASE(local) + IPI_IER_OFFSET);

	mmio_write_32(ier_offset,
		      IPI_BIT_MASK(remote));
}

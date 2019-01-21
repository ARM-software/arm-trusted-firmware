/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Top-level SMC handler for ZynqMP IPI Mailbox doorbell functions.
 */

#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>

#include <ipi.h>
#include <plat_ipi.h>
#include <plat_private.h>

#include "ipi_mailbox_svc.h"
#include "../../../services/spd/trusty/smcall.h"

/*********************************************************************
 * Macros definitions
 ********************************************************************/

/* IPI SMC calls macros: */
#define IPI_SMC_OPEN_IRQ_MASK		0x00000001U /* IRQ enable bit in IPI
						     * open SMC call
						     */
#define IPI_SMC_NOTIFY_BLOCK_MASK	0x00000001U /* Flag to indicate if
						     * IPI notification needs
						     * to be blocking.
						     */
#define IPI_SMC_ENQUIRY_DIRQ_MASK	0x00000001U /* Flag to indicate if
						     * notification interrupt
						     * to be disabled.
						     */
#define IPI_SMC_ACK_EIRQ_MASK		0x00000001U /* Flag to indicate if
						     * notification interrupt
						     * to be enable.
						     */

#define UNSIGNED32_MASK			0xFFFFFFFFU /* 32bit mask */

/**
 * ipi_smc_handler() - SMC handler for IPI SMC calls
 *
 * @smc_fid - Function identifier
 * @x1 - x4 - Arguments
 * @cookie  - Unused
 * @handler - Pointer to caller's context structure
 *
 * @return  - Unused
 *
 * Determines that smc_fid is valid and supported PM SMC Function ID from the
 * list of pm_api_ids, otherwise completes the request with
 * the unknown SMC Function ID
 *
 * The SMC calls for PM service are forwarded from SIP Service SMC handler
 * function with rt_svc_handle signature
 */
uint64_t ipi_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2,
			 uint64_t x3, uint64_t x4, void *cookie,
			 void *handle, uint64_t flags)
{
	int ret;
	uint32_t ipi_local_id;
	uint32_t ipi_remote_id;
	unsigned int is_secure;

	ipi_local_id = x1 & UNSIGNED32_MASK;
	ipi_remote_id = x2 & UNSIGNED32_MASK;

	if (SMC_ENTITY(smc_fid) >= SMC_ENTITY_TRUSTED_APP)
		is_secure = 1;
	else
		is_secure = 0;

	/* Validate IPI mailbox access */
	ret = ipi_mb_validate(ipi_local_id, ipi_remote_id, is_secure);
	if (ret)
		SMC_RET1(handle, ret);

	switch (SMC_FUNCTION(smc_fid)) {
	case IPI_MAILBOX_OPEN:
		ipi_mb_open(ipi_local_id, ipi_remote_id);
		SMC_RET1(handle, 0);
	case IPI_MAILBOX_RELEASE:
		ipi_mb_release(ipi_local_id, ipi_remote_id);
		SMC_RET1(handle, 0);
	case IPI_MAILBOX_STATUS_ENQUIRY:
	{
		int disable_irq;

		disable_irq = (x3 & IPI_SMC_ENQUIRY_DIRQ_MASK) ? 1 : 0;
		ret = ipi_mb_enquire_status(ipi_local_id, ipi_remote_id);
		if ((ret & IPI_MB_STATUS_RECV_PENDING) && disable_irq)
			ipi_mb_disable_irq(ipi_local_id, ipi_remote_id);
		SMC_RET1(handle, ret);
	}
	case IPI_MAILBOX_NOTIFY:
	{
		uint32_t is_blocking;

		is_blocking = (x3 & IPI_SMC_NOTIFY_BLOCK_MASK) ? 1 : 0;
		ipi_mb_notify(ipi_local_id, ipi_remote_id, is_blocking);
		SMC_RET1(handle, 0);
	}
	case IPI_MAILBOX_ACK:
	{
		int enable_irq;

		enable_irq = (x3 & IPI_SMC_ACK_EIRQ_MASK) ? 1 : 0;
		ipi_mb_ack(ipi_local_id, ipi_remote_id);
		if (enable_irq)
			ipi_mb_enable_irq(ipi_local_id, ipi_remote_id);
		SMC_RET1(handle, 0);
	}
	case IPI_MAILBOX_ENABLE_IRQ:
		ipi_mb_enable_irq(ipi_local_id, ipi_remote_id);
		SMC_RET1(handle, 0);
	case IPI_MAILBOX_DISABLE_IRQ:
		ipi_mb_disable_irq(ipi_local_id, ipi_remote_id);
		SMC_RET1(handle, 0);
	default:
		WARN("Unimplemented IPI service call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* ZynqMP IPI mailbox doorbell service enums and defines */

#ifndef IPI_MAILBOX_SVC_H
#define IPI_MAILBOX_SVC_H

#include <stdint.h>

/*********************************************************************
 * Enum definitions
 ********************************************************************/

/* IPI SMC function numbers enum definition */
enum ipi_api_id {
	/* IPI mailbox operations functions: */
	IPI_MAILBOX_OPEN = 0x1000,
	IPI_MAILBOX_RELEASE,
	IPI_MAILBOX_STATUS_ENQUIRY,
	IPI_MAILBOX_NOTIFY,
	IPI_MAILBOX_ACK,
	IPI_MAILBOX_ENABLE_IRQ,
	IPI_MAILBOX_DISABLE_IRQ
};

/*********************************************************************
 * IPI mailbox service APIs declarations
 ********************************************************************/

/* IPI SMC handler */
uint64_t ipi_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2,
			 uint64_t x3, uint64_t x4, void *cookie, void *handle,
			 uint64_t flags);

#endif /* IPI_MAILBOX_SVC_H */

/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* ZynqMP IPI management enums and defines */

#ifndef _ZYNQMP_IPI_H_
#define _ZYNQMP_IPI_H_

#include <stdint.h>

/*********************************************************************
 * IPI agent IDs macros
 ********************************************************************/
#define IPI_ID_APU	0U
#define IPI_ID_RPU0	1U
#define IPI_ID_RPU1	2U
#define IPI_ID_PMU0	3U
#define IPI_ID_PMU1	4U
#define IPI_ID_PMU2	5U
#define IPI_ID_PMU3	6U
#define IPI_ID_PL0	7U
#define IPI_ID_PL1	8U
#define IPI_ID_PL2	9U
#define IPI_ID_PL3	10U

/*********************************************************************
 * IPI mailbox status macros
 ********************************************************************/
#define IPI_MB_STATUS_IDLE		0
#define IPI_MB_STATUS_SEND_PENDING	1
#define IPI_MB_STATUS_RECV_PENDING	2

/*********************************************************************
 * IPI mailbox call is secure or not macros
 ********************************************************************/
#define IPI_MB_CALL_NOTSECURE	0
#define IPI_MB_CALL_SECURE	1

/*********************************************************************
 * IPI APIs declarations
 ********************************************************************/

/* Validate IPI mailbox access */
int ipi_mb_validate(uint32_t local, uint32_t remote, unsigned int is_secure);

/* Open the IPI mailbox */
void ipi_mb_open(uint32_t local, uint32_t remote);

/* Release the IPI mailbox */
void ipi_mb_release(uint32_t local, uint32_t remote);

/* Enquire IPI mailbox status */
int ipi_mb_enquire_status(uint32_t local, uint32_t remote);

/* Trigger notification on the IPI mailbox */
void ipi_mb_notify(uint32_t local, uint32_t remote, uint32_t is_blocking);

/* Ack IPI mailbox notification */
void ipi_mb_ack(uint32_t local, uint32_t remote);

/* Disable IPI mailbox notification interrupt */
void ipi_mb_disable_irq(uint32_t local, uint32_t remote);

/* Enable IPI mailbox notification interrupt */
void ipi_mb_enable_irq(uint32_t local, uint32_t remote);

#endif /* _ZYNQMP_IPI_H_ */

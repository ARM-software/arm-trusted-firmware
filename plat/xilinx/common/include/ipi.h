/*
 * Copyright (c) 2018, Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Xilinx IPI management configuration data and macros */

#ifndef IPI_H
#define IPI_H

#include <stdint.h>

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
 * IPI secure check
 ********************************************************************/
#define IPI_SECURE_MASK  0x1U
#define IPI_IS_SECURE(I) ((ipi_table[(I)].secure_only & \
			   IPI_SECURE_MASK) ? 1 : 0)

/*********************************************************************
 * Struct definitions
 ********************************************************************/

/* structure to maintain IPI configuration information */
struct ipi_config {
	unsigned int ipi_bit_mask;
	unsigned int ipi_reg_base;
	unsigned char secure_only;
};

/*********************************************************************
 * IPI APIs declarations
 ********************************************************************/

/* Initialize IPI configuration table */
void ipi_config_table_init(const struct ipi_config *ipi_table,
			   uint32_t total_ipi);

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

#endif /* IPI_H */

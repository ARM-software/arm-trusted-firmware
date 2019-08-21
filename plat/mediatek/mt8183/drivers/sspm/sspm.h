/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __SSPM_H__
#define __SSPM_H__
/* These should sync with sspm.bin */
#define IPI_ID_PLATFORM			0
#define IPI_ID_SUSPEND			6
#define PINR_OFFSET_PLATFORM		0
#define PINR_SIZE_PLATFORM		3
#define PINR_OFFSET_SUSPEND		2
#define PINR_SIZE_SUSPEND		8

#define MBOX0_BASE			0x10450000
#define MBOX1_BASE			0x10460000
#define MBOX3_BASE			0x10480000
#define MBOX_OUT_IRQ_OFS		0x1000
#define MBOX_IN_IRQ_OFS			0x1004

#define SHAREMBOX_OFFSET_MCDI		0
#define SHAREMBOX_SIZE_MCDI		20
#define SHAREMBOX_OFFSET_SUSPEND	26
#define SHAREMBOX_SIZE_SUSPEND		6

int sspm_mbox_read(uint32_t slot, uint32_t *data, uint32_t len);
int sspm_mbox_write(uint32_t slot, uint32_t *data, uint32_t len);
int sspm_ipi_send_non_blocking(uint32_t id, uint32_t *data);
int sspm_ipi_recv_non_blocking(uint32_t slot, uint32_t *data, uint32_t len);
int sspm_alive_show(void);
#endif /* __SSPM_H__ */

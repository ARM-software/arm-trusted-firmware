/*
 * Copyright (C) 2021 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef MSS_DEFS_H
#define MSS_DEFS_H

#define MSS_DMA_SRCBR(base)		(base + 0xC0)
#define MSS_DMA_DSTBR(base)		(base + 0xC4)
#define MSS_DMA_CTRLR(base)		(base + 0xC8)
#define MSS_M3_RSTCR(base)		(base + 0xFC)

#define MSS_DMA_CTRLR_SIZE_OFFSET	(0)
#define MSS_DMA_CTRLR_REQ_OFFSET	(15)
#define MSS_DMA_CTRLR_REQ_SET		(1)
#define MSS_DMA_CTRLR_ACK_OFFSET	(12)
#define MSS_DMA_CTRLR_ACK_MASK		(0x1)
#define MSS_DMA_CTRLR_ACK_READY		(1)
#define MSS_M3_RSTCR_RST_OFFSET		(0)
#define MSS_M3_RSTCR_RST_OFF		(1)

#define MSS_FW_READY_MAGIC		0x46575144 /* FWRD */

#define MSS_AP_REGS_OFFSET		0x00580000
#define MSS_CP_SRAM_OFFSET		0x00220000
#define MSS_CP_REGS_OFFSET		0x00280000

void mss_start_cp_cm3(int cp);

#endif /* MSS_DEFS_H */

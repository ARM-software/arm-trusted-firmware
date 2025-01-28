/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_SSPM_INTC_H
#define MT_SPM_SSPM_INTC_H

#define MT_SPM_SSPM_INTC_SEL_0		0x10
#define MT_SPM_SSPM_INTC_SEL_1		0x20
#define MT_SPM_SSPM_INTC_SEL_2		0x40
#define MT_SPM_SSPM_INTC_SEL_3		0x80

#define MT_SPM_SSPM_INTC_TRIGGER(id, sg) \
	(((0x10 << (id)) | ((sg) << (id))) & 0xFF)

#define SSPM_CFGREG_ADDR(ofs)		(SSPM_CFGREG_BASE + (ofs))
#define AP_SSPM_IRQ			SSPM_CFGREG_ADDR(0x018)

#define DO_SPM_SSPM_LP_NOTIFY()		mmio_write_32(AP_SSPM_IRQ, 1)

#endif /* MT_SPM_SSPM_INTC_H */

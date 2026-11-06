/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_SECURE_IO_CFG_H
#define QTI_SECURE_IO_CFG_H

#include <stdint.h>

/*
 * Registers the non-secure world may access through the secure IO calls:
 * the download mode cookie, the TCSR register holding the bit that permits
 * EUD to be enabled, and, in debug builds only, the EUD enable itself,
 * which opens the SWD debug port over USB.
 */
#define TCSR_TCSR_BOOT_MISC_DETECT		0x003d3000
#define TCSR_APSS_SPARE_REG1			0x003e5018
#if DEBUG
#define AHB2PHY_USBEUD_EUD_EN2			0x01612000
#endif

static const uintptr_t qti_secure_io_allowed_regs[] = {
	TCSR_TCSR_BOOT_MISC_DETECT,
	TCSR_APSS_SPARE_REG1,
#if DEBUG
	AHB2PHY_USBEUD_EUD_EN2,
#endif
};

#endif /* QTI_SECURE_IO_CFG_H */

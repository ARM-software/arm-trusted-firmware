/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <agatti_def.h>

#define MAX_IO_HANDLES			U(2)
#define MAX_IO_DEVICES			U(2)
#define MAX_IO_BLOCK_DEVICES		U(1)

/* XBL loads the TZ image into the IMEM TZ window and enters it at EL3. */
#define BL2_BASE			0x0c100000
#define BL2_SIZE			0x00019000
#define BL2_LIMIT			(BL2_BASE + BL2_SIZE)

#define BL31_BASE			0x10100000
#define BL31_SIZE			0x00100000
#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)

/*
 * OP-TEE carve-out, in the free DDR window between smem and the modem PIL
 * region. Sized for 4 MiB of TEE RAM plus 33 MiB of TA RAM, matching the other
 * Bruin targets; with less TA RAM than that, xtest fails as soon as a test
 * opens a nested TA session. Keep in sync with OP-TEE CFG_TZDRAM_START and the
 * optee_mem reserved-memory node in the Linux DT.
 */
#define BL32_BASE			0x46200000
#define BL32_SIZE			0x02500000
#define BL32_LIMIT			(BL32_BASE + BL32_SIZE)

/* XBL loads the uefi partition image (the FIP in an ELF) here. */
#define PLAT_QTI_FIP_IOBASE		0x5f800000
#define PLAT_QTI_FIP_MAXSIZE		0x00400000

#define BL33_BASE			0x5fc00000
#define BL33_SIZE			0x00400000

#endif /* PLATFORM_DEF_H */

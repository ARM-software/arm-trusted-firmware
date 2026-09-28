/*
 * Copyright (c) 2024-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * MCSR mailbox backend umbrella header.
 *
 * Devices that use the MCSR (multi-channel/ring based) SDM mailbox include
 * only this header from their socfpga_plat_def.h.  It pulls in the C2S/S2C
 * ring helpers, exposes the MCSR backend vtable and advertises MCSR support
 * to the common core dispatcher (mailbox_core_init).
 */

#ifndef SOCFPGA_MCSR_MAILBOX_H
#define SOCFPGA_MCSR_MAILBOX_H

#ifndef __ASSEMBLER__

#include "socfpga_mcsr_c2s.h"
#include "socfpga_mcsr_s2c.h"

struct mailbox_backend;

/* MCSR backend vtable, defined in socfpga_mcsr_mailbox.c. */
extern struct mailbox_backend mcsr_backend;

#endif /* __ASSEMBLER__ */

#endif /* SOCFPGA_MCSR_MAILBOX_H */

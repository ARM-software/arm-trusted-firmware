/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef S32G_SCMI_H
#define S32G_SCMI_H

#include <lib/utils_def.h>

#define S32G_SCMI_SMC_ID	U(0xc20000fe)

/* Only a single SCMI agent (the non-secure OSPM, i.e. U-Boot/Linux). */
#define S32G_SCMI_AGENT_OSPM	U(0)
#define S32G_SCMI_AGENT_COUNT	U(1)

/*
 * Map the SCMI shared memory and initialise the agent channel(s). Must be
 * called from BL31 platform setup, after the MMU is enabled and before the
 * runtime services are initialised.
 */
void s32g_scmi_setup(void);

#endif /* S32G_SCMI_H */

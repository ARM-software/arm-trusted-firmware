/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX_SCMI_CLIENT_H
#define IMX_SCMI_CLIENT_H

#include <platform_def.h>

extern void *imx9_scmi_handle;

void plat_imx9_scmi_setup(void);

#define SCMI_CPU_A55_ID(cpu)		((cpu) + IMX9_SCMI_CPU_A55C0)
#define SCMI_CPU_A55_PD(cpu)		((cpu) + SCMI_PWR_MIX_SLICE_IDX_A55C0)

#endif /* IMX_SCMI_CLIENT_H */

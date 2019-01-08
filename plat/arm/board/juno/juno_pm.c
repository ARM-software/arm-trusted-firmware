/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat_arm.h>
#include <scmi.h>

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
#if CSS_USE_SCMI_SDS_DRIVER
	return css_scmi_override_pm_ops(ops);
#else
	return ops;
#endif /* CSS_USE_SCMI_SDS_DRIVER */
}

/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>

#include "scmi_rstd.h"

#pragma weak rockchip_scmi_rstd_count
#pragma weak rockchip_scmi_get_rstd

size_t rockchip_scmi_rstd_count(unsigned int agent_id __unused)
{
	return 0U;
}

rk_scmi_rstd_t *rockchip_scmi_get_rstd(unsigned int agent_id __unused,
				       unsigned int scmi_id __unused)
{
	return NULL;
}

size_t plat_scmi_rstd_count(unsigned int agent_id)
{
	return rockchip_scmi_rstd_count(agent_id);
}

const char *plat_scmi_rstd_get_name(unsigned int agent_id,
				    unsigned int scmi_id)
{
	rk_scmi_rstd_t *rstd;

	rstd = rockchip_scmi_get_rstd(agent_id, scmi_id);
	if (rstd == NULL)
		return NULL;

	return rstd->name;
}

int32_t plat_scmi_rstd_autonomous(unsigned int agent_id,
				  unsigned int scmi_id,
				  unsigned int state)
{
	rk_scmi_rstd_t *rstd;

	rstd = rockchip_scmi_get_rstd(agent_id, scmi_id);
	if (rstd == NULL)
		return SCMI_NOT_FOUND;

	if ((rstd->rstd_ops && rstd->rstd_ops->reset_auto) != 0)
		return rstd->rstd_ops->reset_auto(rstd, state);
	else
		return SCMI_NOT_SUPPORTED;
}

int32_t plat_scmi_rstd_set_state(unsigned int agent_id,
				 unsigned int scmi_id,
				 bool assert_not_deassert)
{
	rk_scmi_rstd_t *rstd;

	rstd = rockchip_scmi_get_rstd(agent_id, scmi_id);
	if (rstd == NULL)
		return SCMI_NOT_FOUND;

	if ((rstd->rstd_ops && rstd->rstd_ops->reset_explicit) != 0)
		return rstd->rstd_ops->reset_explicit(rstd,
						      assert_not_deassert);
	else
		return SCMI_NOT_SUPPORTED;
}

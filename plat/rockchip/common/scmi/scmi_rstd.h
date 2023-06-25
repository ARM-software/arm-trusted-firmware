/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RK_SCMI_RESET_DOMAIN_H
#define RK_SCMI_RESET_DOMAIN_H

#include <stdint.h>

#include <common.h>

struct rk_scmi_rstd;

struct rk_scmi_rstd_ops {
	int (*reset_auto)(struct rk_scmi_rstd *rstd, uint32_t state);
	int (*reset_explicit)(struct rk_scmi_rstd *rstd, bool assert_not_deassert);
};

typedef struct rk_scmi_rstd {
	char name[SCMI_RESET_DOMAIN_ATTR_NAME_SZ];
	uint32_t id;
	uint32_t attribute;
	uint32_t latency;
	struct rk_scmi_rstd_ops *rstd_ops;
} rk_scmi_rstd_t;

/*
 * Return number of reset domain for an agent
 * @agent_id: SCMI agent ID
 * Return number of reset domain
 */
size_t rockchip_scmi_rstd_count(unsigned int agent_id);

/*
 * Get rk_scmi_rstd_t point
 * @agent_id: SCMI agent ID
 * @scmi_id: SCMI rstd ID
 * Return a rk_scmi_rstd_t point
 */
rk_scmi_rstd_t *rockchip_scmi_get_rstd(unsigned int agent_id,
				       unsigned int scmi_id);

#endif /* RK_SCMI_RESET_DOMAIN_H */

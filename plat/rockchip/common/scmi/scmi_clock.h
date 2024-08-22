/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RK_SCMI_CLOCK_H
#define RK_SCMI_CLOCK_H

#include <stdint.h>

#include <common.h>

struct rk_scmi_clock;

struct rk_clk_ops {
	unsigned long (*get_rate)(struct rk_scmi_clock *clock);
	int (*set_rate)(struct rk_scmi_clock *clock, unsigned long rate);
	int (*set_status)(struct rk_scmi_clock *clock, bool status);
};

typedef struct rk_scmi_clock {
	char name[SCMI_CLOCK_NAME_LENGTH_MAX];
	uint8_t enable;
	int8_t is_security;
	uint32_t id;
	uint32_t rate_cnt;
	uint64_t cur_rate;
	uint32_t enable_count;
	const struct rk_clk_ops *clk_ops;
	unsigned long *rate_table;
} rk_scmi_clock_t;

/*
 * Return number of clock controllers for an agent
 * @agent_id: SCMI agent ID
 * Return number of clock controllers
 */
size_t rockchip_scmi_clock_count(unsigned int agent_id);

/*
 * Get rk_scmi_clock_t point
 * @agent_id: SCMI agent ID
 * @scmi_id: SCMI clock ID
 * Return a rk_scmi_clock_t point
 */
rk_scmi_clock_t *rockchip_scmi_get_clock(uint32_t agent_id,
					 uint32_t scmi_id);

#endif /* RK_SCMI_CLOCK_H */

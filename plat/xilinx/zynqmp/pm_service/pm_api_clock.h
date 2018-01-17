/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * ZynqMP system level PM-API functions for clock control.
 */

#ifndef _PM_API_CLOCK_H_
#define _PM_API_CLOCK_H_

#include "pm_common.h"

#define CLK_NAME_LEN		15

enum pm_ret_status pm_api_clock_get_name(unsigned int clock_id, char *name);
enum pm_ret_status pm_api_clock_get_topology(unsigned int clock_id,
					     unsigned int index,
					     uint32_t *topology);
enum pm_ret_status pm_api_clock_get_fixedfactor_params(unsigned int clock_id,
						       uint32_t *mul,
						       uint32_t *div);
enum pm_ret_status pm_api_clock_get_parents(unsigned int clock_id,
					    unsigned int index,
					    uint32_t *parents);
enum pm_ret_status pm_api_clock_get_attributes(unsigned int clock_id,
					       uint32_t *attr);
enum pm_ret_status pm_api_clock_enable(unsigned int clock_id);
enum pm_ret_status pm_api_clock_disable(unsigned int clock_id);
enum pm_ret_status pm_api_clock_getstate(unsigned int clock_id,
					 unsigned int *state);
enum pm_ret_status pm_api_clock_setdivider(unsigned int clock_id,
					   unsigned int divider);
enum pm_ret_status pm_api_clock_getdivider(unsigned int clock_id,
					   unsigned int *divider);
enum pm_ret_status pm_api_clock_setrate(unsigned int clock_id,
					uint64_t rate);
enum pm_ret_status pm_api_clock_getrate(unsigned int clock_id,
					uint64_t *rate);
enum pm_ret_status pm_api_clock_setparent(unsigned int clock_id,
					  unsigned int parent_idx);
enum pm_ret_status pm_api_clock_getparent(unsigned int clock_id,
					  unsigned int *parent_idx);
enum pm_ret_status pm_api_clk_set_pll_mode(unsigned int pll,
					   unsigned int mode);
enum pm_ret_status pm_api_clk_get_pll_mode(unsigned int pll,
					   unsigned int *mode);
enum pm_ret_status pm_api_clk_set_pll_frac_data(unsigned int pll,
						unsigned int data);
enum pm_ret_status pm_api_clk_get_pll_frac_data(unsigned int pll,
						unsigned int *data);
#endif /* _PM_API_CLOCK_H_ */

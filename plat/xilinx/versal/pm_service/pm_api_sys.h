/*
 * Copyright (c) 2019-2020, Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PM_API_SYS_H
#define PM_API_SYS_H

#include <stdint.h>
#include "pm_defs.h"

/**********************************************************
 * PM API function declarations
 **********************************************************/

enum pm_ret_status pm_get_api_version(unsigned int *version, uint32_t flag);
enum pm_ret_status pm_init_finalize(uint32_t flag);
enum pm_ret_status pm_self_suspend(uint32_t nid,
				   unsigned int latency,
				   unsigned int state,
				   uintptr_t address, uint32_t flag);
enum pm_ret_status pm_abort_suspend(enum pm_abort_reason reason, uint32_t flag);
enum pm_ret_status pm_req_suspend(uint32_t target,
				  uint8_t ack,
				  unsigned int latency,
				  unsigned int state, uint32_t flag);
enum pm_ret_status pm_req_wakeup(uint32_t target, uint32_t set_address,
				 uintptr_t address, uint8_t ack, uint32_t flag);
enum pm_ret_status pm_set_wakeup_source(uint32_t target, uint32_t device_id,
					uint8_t enable, uint32_t flag);
enum pm_ret_status pm_request_device(uint32_t device_id, uint32_t capabilities,
				     uint32_t qos, uint32_t ack, uint32_t flag);
enum pm_ret_status pm_release_device(uint32_t device_id, uint32_t flag);
enum pm_ret_status pm_set_requirement(uint32_t device_id, uint32_t capabilities,
				      uint32_t latency, uint32_t qos,
				      uint32_t flag);
enum pm_ret_status pm_get_device_status(uint32_t device_id, uint32_t *response,
					uint32_t flag);
enum pm_ret_status pm_reset_assert(uint32_t reset, bool assert, uint32_t flag);
enum pm_ret_status pm_reset_get_status(uint32_t reset, uint32_t *status,
				       uint32_t flag);
void pm_get_callbackdata(uint32_t *data, size_t count, uint32_t flag);
enum pm_ret_status pm_pinctrl_request(uint32_t pin, uint32_t flag);
enum pm_ret_status pm_pinctrl_release(uint32_t pin, uint32_t flag);
enum pm_ret_status pm_pinctrl_set_function(uint32_t pin, uint32_t function,
					   uint32_t flag);
enum pm_ret_status pm_pinctrl_get_function(uint32_t pin, uint32_t *function,
					   uint32_t flag);
enum pm_ret_status pm_pinctrl_set_pin_param(uint32_t pin, uint32_t param,
					    uint32_t value, uint32_t flag);
enum pm_ret_status pm_pinctrl_get_pin_param(uint32_t pin, uint32_t param,
					    uint32_t *value, uint32_t flag);
enum pm_ret_status pm_clock_enable(uint32_t clk_id, uint32_t flag);
enum pm_ret_status pm_clock_disable(uint32_t clk_id, uint32_t flag);
enum pm_ret_status pm_clock_get_state(uint32_t clk_id, uint32_t *state,
				      uint32_t flag);
enum pm_ret_status pm_clock_set_divider(uint32_t clk_id, uint32_t divider,
					uint32_t flag);
enum pm_ret_status pm_clock_get_divider(uint32_t clk_id, uint32_t *divider,
					uint32_t flag);
enum pm_ret_status pm_clock_set_parent(uint32_t clk_id, uint32_t parent,
				       uint32_t flag);
enum pm_ret_status pm_clock_get_parent(uint32_t clk_id, uint32_t *parent,
				       uint32_t flag);
enum pm_ret_status pm_clock_get_rate(uint32_t clk_id, uint32_t *clk_rate,
				     uint32_t flag);
enum pm_ret_status pm_pll_set_param(uint32_t clk_id, uint32_t param,
				    uint32_t value, uint32_t flag);
enum pm_ret_status pm_pll_get_param(uint32_t clk_id, uint32_t param,
				    uint32_t *value, uint32_t flag);
enum pm_ret_status pm_pll_set_mode(uint32_t clk_id, uint32_t mode,
				   uint32_t flag);
enum pm_ret_status pm_pll_get_mode(uint32_t clk_id, uint32_t *mode,
				   uint32_t flag);
enum pm_ret_status pm_force_powerdown(uint32_t target, uint8_t ack,
				      uint32_t flag);
enum pm_ret_status pm_system_shutdown(uint32_t type, uint32_t subtype,
				      uint32_t flag);
enum pm_ret_status pm_api_ioctl(uint32_t device_id, uint32_t ioctl_id,
				uint32_t arg1, uint32_t arg2, uint32_t *value,
				uint32_t flag);
enum pm_ret_status pm_query_data(uint32_t qid, uint32_t arg1, uint32_t arg2,
				 uint32_t arg3, uint32_t *data, uint32_t flag);
unsigned int pm_get_shutdown_scope(void);
enum pm_ret_status pm_get_chipid(uint32_t *value, uint32_t flag);
enum pm_ret_status pm_feature_check(uint32_t api_id, unsigned int *version,
				    uint32_t flag);
enum pm_ret_status pm_load_pdi(uint32_t src, uint32_t address_low,
			       uint32_t address_high, uint32_t flag);
enum pm_ret_status pm_get_op_characteristic(uint32_t device_id,
					    enum pm_opchar_type type,
					    uint32_t *result, uint32_t flag);
enum pm_ret_status pm_set_max_latency(uint32_t device_id, uint32_t latency,
				      uint32_t flag);
enum pm_ret_status pm_register_notifier(uint32_t device_id, uint32_t event,
					uint32_t wake, uint32_t enable,
					uint32_t flag);
#endif /* PM_API_SYS_H */

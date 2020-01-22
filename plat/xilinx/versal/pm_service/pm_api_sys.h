/*
 * Copyright (c) 2019, Xilinx, Inc. All rights reserved.
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

enum pm_ret_status pm_get_api_version(unsigned int *version);
enum pm_ret_status pm_self_suspend(uint32_t nid,
				   unsigned int latency,
				   unsigned int state,
				   uintptr_t address);
enum pm_ret_status pm_abort_suspend(enum pm_abort_reason reason);
enum pm_ret_status pm_req_suspend(uint32_t target,
				  uint8_t ack,
				  unsigned int latency,
				  unsigned int state);
enum pm_ret_status pm_req_wakeup(uint32_t target, uint32_t set_address,
				 uintptr_t address, uint8_t ack);
enum pm_ret_status pm_set_wakeup_source(uint32_t target, uint32_t device_id,
					uint8_t enable);
enum pm_ret_status pm_request_device(uint32_t device_id, uint32_t capabilities,
				     uint32_t qos, uint32_t ack);
enum pm_ret_status pm_release_device(uint32_t device_id);
enum pm_ret_status pm_set_requirement(uint32_t device_id, uint32_t capabilities,
				      uint32_t latency, uint32_t qos);
enum pm_ret_status pm_get_device_status(uint32_t device_id, uint32_t *response);
enum pm_ret_status pm_reset_assert(uint32_t reset, bool assert);
enum pm_ret_status pm_reset_get_status(uint32_t reset, uint32_t *status);
void pm_get_callbackdata(uint32_t *data, size_t count);
enum pm_ret_status pm_pinctrl_request(uint32_t pin);
enum pm_ret_status pm_pinctrl_release(uint32_t pin);
enum pm_ret_status pm_pinctrl_set_function(uint32_t pin, uint32_t function);
enum pm_ret_status pm_pinctrl_get_function(uint32_t pin, uint32_t *function);
enum pm_ret_status pm_pinctrl_set_pin_param(uint32_t pin, uint32_t param,
					    uint32_t value);
enum pm_ret_status pm_pinctrl_get_pin_param(uint32_t pin, uint32_t param,
					    uint32_t *value);
enum pm_ret_status pm_clock_enable(uint32_t clk_id);
enum pm_ret_status pm_clock_disable(uint32_t clk_id);
enum pm_ret_status pm_clock_get_state(uint32_t clk_id, uint32_t *state);
enum pm_ret_status pm_clock_set_divider(uint32_t clk_id, uint32_t divider);
enum pm_ret_status pm_clock_get_divider(uint32_t clk_id, uint32_t *divider);
enum pm_ret_status pm_clock_set_parent(uint32_t clk_id, uint32_t parent);
enum pm_ret_status pm_clock_get_parent(uint32_t clk_id, uint32_t *parent);
enum pm_ret_status pm_pll_set_param(uint32_t clk_id, uint32_t param,
				    uint32_t value);
enum pm_ret_status pm_pll_get_param(uint32_t clk_id, uint32_t param,
				    uint32_t *value);
enum pm_ret_status pm_pll_set_mode(uint32_t clk_id, uint32_t mode);
enum pm_ret_status pm_pll_get_mode(uint32_t clk_id, uint32_t *mode);
enum pm_ret_status pm_force_powerdown(uint32_t target, uint8_t ack);
enum pm_ret_status pm_system_shutdown(uint32_t type, uint32_t subtype);
enum pm_ret_status pm_api_ioctl(uint32_t device_id, uint32_t ioctl_id,
				uint32_t arg1, uint32_t arg2, uint32_t *value);
enum pm_ret_status pm_query_data(uint32_t qid, uint32_t arg1, uint32_t arg2,
				 uint32_t arg3, uint32_t *data);
unsigned int pm_get_shutdown_scope(void);
enum pm_ret_status pm_get_chipid(uint32_t *value);
enum pm_ret_status pm_feature_check(uint32_t api_id, unsigned int *version);
enum pm_ret_status pm_load_pdi(uint32_t src, uint32_t address_low,
			       uint32_t address_high);
enum pm_ret_status pm_get_op_characteristic(uint32_t device_id,
					    enum pm_opchar_type type,
					    uint32_t *result);
#endif /* PM_API_SYS_H */

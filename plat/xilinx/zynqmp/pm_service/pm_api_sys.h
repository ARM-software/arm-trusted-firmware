/*
 * Copyright (c) 2013-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PM_API_SYS_H
#define PM_API_SYS_H

#include <stdint.h>

#include "pm_defs.h"

enum pm_query_id {
	PM_QID_INVALID,
	PM_QID_CLOCK_GET_NAME,
	PM_QID_CLOCK_GET_TOPOLOGY,
	PM_QID_CLOCK_GET_FIXEDFACTOR_PARAMS,
	PM_QID_CLOCK_GET_PARENTS,
	PM_QID_CLOCK_GET_ATTRIBUTES,
	PM_QID_PINCTRL_GET_NUM_PINS,
	PM_QID_PINCTRL_GET_NUM_FUNCTIONS,
	PM_QID_PINCTRL_GET_NUM_FUNCTION_GROUPS,
	PM_QID_PINCTRL_GET_FUNCTION_NAME,
	PM_QID_PINCTRL_GET_FUNCTION_GROUPS,
	PM_QID_PINCTRL_GET_PIN_GROUPS,
	PM_QID_CLOCK_GET_NUM_CLOCKS,
	PM_QID_CLOCK_GET_MAX_DIVISOR,
};

enum pm_register_access_id {
	CONFIG_REG_WRITE,
	CONFIG_REG_READ,
};

/**
 * Assigning of argument values into array elements.
 */
#define PM_PACK_PAYLOAD1(pl, arg0) {	\
	pl[0] = (uint32_t)(arg0);	\
}

#define PM_PACK_PAYLOAD2(pl, arg0, arg1) {	\
	pl[1] = (uint32_t)(arg1);		\
	PM_PACK_PAYLOAD1(pl, arg0);		\
}

#define PM_PACK_PAYLOAD3(pl, arg0, arg1, arg2) {	\
	pl[2] = (uint32_t)(arg2);			\
	PM_PACK_PAYLOAD2(pl, arg0, arg1);		\
}

#define PM_PACK_PAYLOAD4(pl, arg0, arg1, arg2, arg3) {	\
	pl[3] = (uint32_t)(arg3);			\
	PM_PACK_PAYLOAD3(pl, arg0, arg1, arg2);		\
}

#define PM_PACK_PAYLOAD5(pl, arg0, arg1, arg2, arg3, arg4) {	\
	pl[4] = (uint32_t)(arg4);				\
	PM_PACK_PAYLOAD4(pl, arg0, arg1, arg2, arg3);		\
}

#define PM_PACK_PAYLOAD6(pl, arg0, arg1, arg2, arg3, arg4, arg5) {	\
	pl[5] = (uint32_t)(arg5);					\
	PM_PACK_PAYLOAD5(pl, arg0, arg1, arg2, arg3, arg4);		\
}

/**********************************************************
 * System-level API function declarations
 **********************************************************/
enum pm_ret_status pm_req_suspend(enum pm_node_id target,
				  enum pm_request_ack ack,
				  uint32_t latency,
				  uint32_t state);

enum pm_ret_status pm_self_suspend(enum pm_node_id nid,
				   uint32_t latency,
				   uint32_t state,
				   uintptr_t address);

enum pm_ret_status pm_force_powerdown(enum pm_node_id target,
				      enum pm_request_ack ack);

enum pm_ret_status pm_abort_suspend(enum pm_abort_reason reason);

enum pm_ret_status pm_req_wakeup(enum pm_node_id target,
				 uint32_t set_address,
				 uintptr_t address,
				 enum pm_request_ack ack);

enum pm_ret_status pm_set_wakeup_source(enum pm_node_id target,
					enum pm_node_id wkup_node,
					uint32_t enable);

enum pm_ret_status pm_system_shutdown(uint32_t type, uint32_t subtype);

/* API functions for managing PM Slaves */
enum pm_ret_status pm_req_node(enum pm_node_id nid,
			       uint32_t capabilities,
			       uint32_t qos,
			       enum pm_request_ack ack);

enum pm_ret_status pm_set_requirement(enum pm_node_id nid,
				      uint32_t capabilities,
				      uint32_t qos,
				      enum pm_request_ack ack);

/* Miscellaneous API functions */
enum pm_ret_status pm_get_api_version(uint32_t *version);
enum pm_ret_status pm_get_node_status(enum pm_node_id nid,
				      uint32_t *ret_buff);

/* Direct-Control API functions */
enum pm_ret_status pm_mmio_write(uintptr_t address,
				 uint32_t mask,
				 uint32_t value);
enum pm_ret_status pm_mmio_read(uintptr_t address, uint32_t *value);
enum pm_ret_status pm_fpga_load(uint32_t address_low,
				uint32_t address_high,
				uint32_t size,
				uint32_t flags);
enum pm_ret_status pm_fpga_get_status(uint32_t *value);

enum pm_ret_status pm_get_chipid(uint32_t *value);
enum pm_ret_status pm_secure_rsaaes(uint32_t address_low,
				    uint32_t address_high,
				    uint32_t size,
				    uint32_t flags);
unsigned int pm_get_shutdown_scope(void);
void pm_get_callbackdata(uint32_t *data, size_t count);
enum pm_ret_status pm_ioctl(enum pm_node_id nid,
			    uint32_t ioctl_id,
			    uint32_t arg1,
			    uint32_t arg2,
			    uint32_t *value);
enum pm_ret_status pm_clock_enable(uint32_t clock_id);
enum pm_ret_status pm_clock_disable(uint32_t clock_id);
enum pm_ret_status pm_clock_getstate(uint32_t clock_id,
				     uint32_t *state);
enum pm_ret_status pm_clock_setdivider(uint32_t clock_id,
				       uint32_t divider);
enum pm_ret_status pm_clock_getdivider(uint32_t clock_id,
				       uint32_t *divider);
enum pm_ret_status pm_clock_setrate(uint32_t clock_id,
				    uint64_t rate);
enum pm_ret_status pm_clock_getrate(uint32_t clock_id,
				    uint64_t *rate);
enum pm_ret_status pm_clock_setparent(uint32_t clock_id,
				      uint32_t parent_index);
enum pm_ret_status pm_clock_getparent(uint32_t clock_id,
				      uint32_t *parent_index);
void pm_query_data(enum pm_query_id qid, uint32_t arg1, uint32_t arg2,
		   uint32_t arg3, uint32_t *data);
enum pm_ret_status pm_sha_hash(uint32_t address_high,
				    uint32_t address_low,
				    uint32_t size,
				    uint32_t flags);
enum pm_ret_status pm_rsa_core(uint32_t address_high,
				    uint32_t address_low,
				    uint32_t size,
				    uint32_t flags);
enum pm_ret_status pm_secure_image(uint32_t address_low,
				   uint32_t address_high,
				   uint32_t key_lo,
				   uint32_t key_hi,
				   uint32_t *value);
enum pm_ret_status pm_fpga_read(uint32_t reg_numframes,
				uint32_t address_low,
				uint32_t address_high,
				uint32_t readback_type,
				uint32_t *value);
enum pm_ret_status pm_aes_engine(uint32_t address_high,
				 uint32_t address_low,
				 uint32_t  *value);
enum pm_ret_status pm_register_access(uint32_t register_access_id,
				      uint32_t address,
				      uint32_t mask,
				      uint32_t value,
				      uint32_t *out);
enum pm_ret_status pm_pll_set_parameter(enum pm_node_id nid,
					enum pm_pll_param param_id,
					uint32_t value);
enum pm_ret_status pm_pll_get_parameter(enum pm_node_id nid,
					enum pm_pll_param param_id,
					uint32_t *value);
enum pm_ret_status pm_pll_set_mode(enum pm_node_id nid, enum pm_pll_mode mode);
enum pm_ret_status pm_pll_get_mode(enum pm_node_id nid, enum pm_pll_mode *mode);
enum pm_ret_status pm_efuse_access(uint32_t address_high,
				   uint32_t address_low, uint32_t *value);
enum pm_ret_status em_set_action(uint32_t *value);
enum pm_ret_status em_remove_action(uint32_t *value);
enum pm_ret_status em_send_errors(uint32_t *value);
enum pm_ret_status pm_feature_check(uint32_t api_id, uint32_t *version,
				    uint32_t *bit_mask, uint8_t len);
enum pm_ret_status check_api_dependency(uint8_t id);

#endif /* PM_API_SYS_H */

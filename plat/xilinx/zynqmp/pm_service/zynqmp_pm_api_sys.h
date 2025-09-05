/*
 * Copyright (c) 2013-2022, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2023-2025, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ZYNQMP_PM_API_SYS_H
#define ZYNQMP_PM_API_SYS_H

#include <stdint.h>

#include "pm_defs.h"
#include "zynqmp_pm_defs.h"

enum pm_query_ids {
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

/*
 * Assigning of argument values into array elements.
 */
#define PM_PACK_PAYLOAD1(pl, flag, arg0) {			\
	pl[0] = ((uint32_t)(arg0) | ((uint32_t)(flag) << 24U));	\
}

#define PM_PACK_PAYLOAD2(pl, flag, arg0, arg1) {		\
	pl[1] = (uint32_t)(arg1);				\
	PM_PACK_PAYLOAD1(pl, (flag), (arg0));			\
}

#define PM_PACK_PAYLOAD3(pl, flag, arg0, arg1, arg2) {		\
	pl[2] = (uint32_t)(arg2);				\
	PM_PACK_PAYLOAD2(pl, (flag), (arg0), (arg1));		\
}

#define PM_PACK_PAYLOAD4(pl, flag, arg0, arg1, arg2, arg3) {	\
	pl[3] = (uint32_t)(arg3);				\
	PM_PACK_PAYLOAD3(pl, (flag), (arg0), (arg1), (arg2));	\
}

#define PM_PACK_PAYLOAD5(pl, flag, arg0, arg1, arg2, arg3, arg4) {	\
	pl[4] = (uint32_t)(arg4);					\
	PM_PACK_PAYLOAD4(pl, (flag), (arg0), (arg1), (arg2), (arg3));	\
}

#define PM_PACK_PAYLOAD6(pl, flag, arg0, arg1, arg2, arg3, arg4, arg5) {	\
	pl[5] = (uint32_t)(arg5);						\
	PM_PACK_PAYLOAD5(pl, (flag), (arg0), (arg1), (arg2), (arg3), (arg4));	\
}

/**********************************************************
 * System-level API function declarations
 **********************************************************/
enum pm_ret_status pm_req_suspend(enum pm_node_id target,
				  enum pm_request_ack ack,
				  uint32_t latency,
				  uint32_t state,
				  uint32_t flag);

enum pm_ret_status pm_self_suspend(enum pm_node_id nid,
				   uint32_t latency,
				   uint32_t state,
				   uintptr_t address,
				   uint32_t flag);

enum pm_ret_status pm_force_powerdown(enum pm_node_id target,
				      enum pm_request_ack ack,
				      uint32_t flag);

enum pm_ret_status pm_abort_suspend(enum pm_abort_reason reason,
				    uint32_t flag);

enum pm_ret_status pm_req_wakeup(enum pm_node_id target,
				 uint32_t set_address,
				 uintptr_t address,
				 enum pm_request_ack ack,
				 uint32_t flag);

enum pm_ret_status pm_set_wakeup_source(enum pm_node_id target,
					enum pm_node_id wkup_node,
					uint32_t enable,
					uint32_t flag);

enum pm_ret_status pm_system_shutdown(uint32_t type, uint32_t subtype,
				      uint32_t flag);

/* API functions for managing PM Slaves */
enum pm_ret_status pm_req_node(enum pm_node_id nid,
			       uint32_t capabilities,
			       uint32_t qos,
			       enum pm_request_ack ack,
			       uint32_t flag);

enum pm_ret_status pm_set_requirement(enum pm_node_id nid,
				      uint32_t capabilities,
				      uint32_t qos,
				      enum pm_request_ack ack,
				      uint32_t flag);

/* Miscellaneous API functions */
enum pm_ret_status pm_get_api_version(uint32_t *version, uint32_t flag);
enum pm_ret_status pm_get_node_status(enum pm_node_id nid,
				      uint32_t *ret_buff, uint32_t flag);

/* Direct-Control API functions */
enum pm_ret_status pm_mmio_write(uintptr_t address,
				 uint32_t mask,
				 uint32_t value, uint32_t flag);
enum pm_ret_status pm_mmio_read(uintptr_t address, uint32_t *value,
				uint32_t flag);
enum pm_ret_status pm_fpga_load(uint32_t address_low,
				uint32_t address_high,
				uint32_t size,
				uint32_t flags,
				uint32_t security_flag);
enum pm_ret_status pm_fpga_get_status(uint32_t *value, uint32_t flag);

enum pm_ret_status pm_get_chipid(uint32_t *value, uint32_t flag);
enum pm_ret_status pm_secure_rsaaes(uint32_t address_low,
				    uint32_t address_high,
				    uint32_t size,
				    uint32_t flags,
				    uint32_t security_flag);
uint32_t pm_get_shutdown_scope(void);
enum pm_ret_status pm_get_callbackdata(uint32_t *data, size_t count);
enum pm_ret_status pm_ioctl(enum pm_node_id nid,
			    uint32_t ioctl_id,
			    uint32_t arg1,
			    uint32_t arg2,
			    uint32_t *value,
			    uint32_t flag);
enum pm_ret_status pm_clock_enable(uint32_t clock_id, uint32_t flag);
enum pm_ret_status pm_clock_disable(uint32_t clock_id, uint32_t flag);
enum pm_ret_status pm_clock_getstate(uint32_t clock_id,
				     uint32_t *state, uint32_t flag);
enum pm_ret_status pm_clock_setdivider(uint32_t clock_id,
				       uint32_t divider, uint32_t flag);
enum pm_ret_status pm_clock_getdivider(uint32_t clock_id,
				       uint32_t *divider, uint32_t flag);
enum pm_ret_status pm_clock_setparent(uint32_t clock_id,
				      uint32_t parent_index, uint32_t flag);
enum pm_ret_status pm_clock_getparent(uint32_t clock_id,
				      uint32_t *parent_index, uint32_t flag);
void pm_query_data(enum pm_query_ids qid, uint32_t arg1, uint32_t arg2,
		   uint32_t arg3, uint32_t *data, uint32_t flag);
enum pm_ret_status pm_sha_hash(uint32_t address_high,
			       uint32_t address_low,
			       uint32_t size,
			       uint32_t flags,
			       uint32_t security_flag);
enum pm_ret_status pm_rsa_core(uint32_t address_high,
			       uint32_t address_low,
			       uint32_t size,
			       uint32_t flags,
			       uint32_t security_flag);
enum pm_ret_status pm_secure_image(uint32_t address_low,
				   uint32_t address_high,
				   uint32_t key_lo,
				   uint32_t key_hi,
				   uint32_t *value,
				   uint32_t flag);
enum pm_ret_status pm_fpga_read(uint32_t reg_numframes,
				uint32_t address_low,
				uint32_t address_high,
				uint32_t readback_type,
				uint32_t *value,
				uint32_t flag);
enum pm_ret_status pm_aes_engine(uint32_t address_high,
				 uint32_t address_low,
				 uint32_t  *value,
				 uint32_t flag);
enum pm_ret_status pm_register_access(uint32_t register_access_id,
				      uint32_t address,
				      uint32_t mask,
				      uint32_t value,
				      uint32_t *out,
				      uint32_t flag);
enum pm_ret_status pm_pll_set_parameter(enum pm_node_id nid,
					enum pm_pll_param param_id,
					uint32_t value,
					uint32_t flag);
enum pm_ret_status pm_pll_get_parameter(enum pm_node_id nid,
					enum pm_pll_param param_id,
					uint32_t *value,
					uint32_t flag);
enum pm_ret_status pm_pll_set_mode(enum pm_node_id nid,
				   enum pm_pll_mode mode,
				   uint32_t flag);
enum pm_ret_status pm_pll_get_mode(enum pm_node_id nid,
				   enum pm_pll_mode *mode,
				   uint32_t flag);
enum pm_ret_status pm_efuse_access(uint32_t address_high,
				   uint32_t address_low, uint32_t *value,
				   uint32_t flag);
enum pm_ret_status pm_feature_check(uint32_t api_id, uint32_t *version,
				    uint32_t *bit_mask, uint8_t len,
				    uint32_t flag);
enum pm_ret_status check_api_dependency(uint8_t id, uint32_t flag);

#endif /* ZYNQMP_PM_API_SYS_H */

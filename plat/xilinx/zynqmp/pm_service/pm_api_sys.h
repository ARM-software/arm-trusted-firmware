/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
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

/**********************************************************
 * System-level API function declarations
 **********************************************************/
enum pm_ret_status pm_req_suspend(enum pm_node_id nid,
				  enum pm_request_ack ack,
				  unsigned int latency,
				  unsigned int state);

enum pm_ret_status pm_self_suspend(enum pm_node_id nid,
				   unsigned int latency,
				   unsigned int state,
				   uintptr_t address);

enum pm_ret_status pm_force_powerdown(enum pm_node_id nid,
				      enum pm_request_ack ack);

enum pm_ret_status pm_abort_suspend(enum pm_abort_reason reason);

enum pm_ret_status pm_req_wakeup(enum pm_node_id nid,
				 unsigned int set_address,
				 uintptr_t address,
				 enum pm_request_ack ack);

enum pm_ret_status pm_set_wakeup_source(enum pm_node_id target,
					enum pm_node_id wkup_node,
					unsigned int enable);

enum pm_ret_status pm_system_shutdown(unsigned int type, unsigned int subtype);

enum pm_ret_status pm_init_suspend_cb(enum pm_suspend_reason reason,
				      unsigned int latency,
				      unsigned int state,
				      unsigned int timeout);

/* API functions for managing PM Slaves */
enum pm_ret_status pm_req_node(enum pm_node_id nid,
			       unsigned int capabilities,
			       unsigned int qos,
			       enum pm_request_ack ack);
enum pm_ret_status pm_release_node(enum pm_node_id nid);

enum pm_ret_status pm_set_requirement(enum pm_node_id nid,
				      unsigned int capabilities,
				      unsigned int qos,
				      enum pm_request_ack ack);
enum pm_ret_status pm_set_max_latency(enum pm_node_id nid,
				      unsigned int latency);

/* Miscellaneous API functions */
enum pm_ret_status pm_get_api_version(unsigned int *version);
enum pm_ret_status pm_set_configuration(unsigned int phys_addr);
enum pm_ret_status pm_init_finalize(void);
enum pm_ret_status pm_get_node_status(enum pm_node_id node,
				      uint32_t *ret_buff);
enum pm_ret_status pm_register_notifier(enum pm_node_id nid,
					unsigned int event,
					unsigned int wake,
					unsigned int enable);
enum pm_ret_status pm_get_op_characteristic(enum pm_node_id nid,
					    enum pm_opchar_type type,
					    uint32_t *result);
enum pm_ret_status pm_acknowledge_cb(enum pm_node_id nid,
				     enum pm_ret_status status,
				     unsigned int oppoint);
enum pm_ret_status pm_notify_cb(enum pm_node_id nid,
				unsigned int event,
				unsigned int oppoint);

/* Direct-Control API functions */
enum pm_ret_status pm_reset_assert(unsigned int reset_id,
				   unsigned int assert);
enum pm_ret_status pm_reset_get_status(unsigned int reset_id,
				       unsigned int *reset_status);
enum pm_ret_status pm_mmio_write(uintptr_t address,
				 unsigned int mask,
				 unsigned int value);
enum pm_ret_status pm_mmio_read(uintptr_t address, unsigned int *value);
enum pm_ret_status pm_fpga_load(uint32_t address_low,
				uint32_t address_high,
				uint32_t size,
				uint32_t flags);
enum pm_ret_status pm_fpga_get_status(unsigned int *value);

enum pm_ret_status pm_get_chipid(uint32_t *value);
enum pm_ret_status pm_secure_rsaaes(uint32_t address_high,
				    uint32_t address_low,
				    uint32_t size,
				    uint32_t flags);
unsigned int pm_get_shutdown_scope(void);
void pm_get_callbackdata(uint32_t *data, size_t count);
enum pm_ret_status pm_pinctrl_request(unsigned int pin);
enum pm_ret_status pm_pinctrl_release(unsigned int pin);
enum pm_ret_status pm_pinctrl_get_function(unsigned int pin,
					   enum pm_node_id *nid);
enum pm_ret_status pm_pinctrl_set_function(unsigned int pin,
					   enum pm_node_id nid);
enum pm_ret_status pm_pinctrl_get_config(unsigned int pin,
					 unsigned int param,
					 unsigned int *value);
enum pm_ret_status pm_pinctrl_set_config(unsigned int pin,
					 unsigned int param,
					 unsigned int value);
enum pm_ret_status pm_ioctl(enum pm_node_id nid,
			    unsigned int ioctl_id,
			    unsigned int arg1,
			    unsigned int arg2,
			    unsigned int *value);
enum pm_ret_status pm_clock_enable(unsigned int clock_id);
enum pm_ret_status pm_clock_disable(unsigned int clock_id);
enum pm_ret_status pm_clock_getstate(unsigned int clock_id,
				     unsigned int *state);
enum pm_ret_status pm_clock_setdivider(unsigned int clock_id,
				       unsigned int divider);
enum pm_ret_status pm_clock_getdivider(unsigned int clock_id,
				       unsigned int *divider);
enum pm_ret_status pm_clock_setrate(unsigned int clock_id,
				    uint64_t rate);
enum pm_ret_status pm_clock_getrate(unsigned int clock_id,
				    uint64_t *rate);
enum pm_ret_status pm_clock_setparent(unsigned int clock_id,
				      unsigned int parent_id);
enum pm_ret_status pm_clock_getparent(unsigned int clock_id,
				      unsigned int *parent_id);
enum pm_ret_status pm_query_data(enum pm_query_id qid,
				 unsigned int arg1,
				 unsigned int arg2,
				 unsigned int arg3,
				 unsigned int *data);
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

enum pm_ret_status pm_pll_set_parameter(enum pm_node_id nid,
				enum pm_pll_param param_id,
				unsigned int value);

enum pm_ret_status pm_pll_get_parameter(enum pm_node_id nid,
				enum pm_pll_param param_id,
				unsigned int *value);

enum pm_ret_status pm_pll_set_mode(enum pm_node_id nid, enum pm_pll_mode mode);
enum pm_ret_status pm_pll_get_mode(enum pm_node_id nid, enum pm_pll_mode *mode);

#endif /* PM_API_SYS_H */

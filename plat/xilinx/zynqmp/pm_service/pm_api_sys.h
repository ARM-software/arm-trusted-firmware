/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PM_API_SYS_H_
#define _PM_API_SYS_H_

#include <stdint.h>
#include "pm_defs.h"

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
enum pm_ret_status pm_get_node_status(enum pm_node_id node);
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
void pm_get_callbackdata(uint32_t *data, size_t count);

#endif /* _PM_API_SYS_H_ */

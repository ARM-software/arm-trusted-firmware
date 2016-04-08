/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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

enum pm_ret_status pm_system_shutdown(unsigned int restart);

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
					    enum pm_opchar_type type);
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
#endif /* _PM_API_SYS_H_ */

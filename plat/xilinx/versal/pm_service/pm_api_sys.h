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
enum pm_ret_status pm_request_device(uint32_t device_id, uint32_t capabilities,
				     uint32_t qos, uint32_t ack);
enum pm_ret_status pm_release_device(uint32_t device_id);
enum pm_ret_status pm_set_requirement(uint32_t device_id, uint32_t capabilities,
				      uint32_t latency, uint32_t qos);
enum pm_ret_status pm_get_device_status(uint32_t device_id, uint32_t *response);
enum pm_ret_status pm_reset_assert(uint32_t reset, bool assert);
enum pm_ret_status pm_reset_get_status(uint32_t reset, uint32_t *status);

#endif /* PM_API_SYS_H */

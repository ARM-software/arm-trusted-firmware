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

#endif /* PM_API_SYS_H */

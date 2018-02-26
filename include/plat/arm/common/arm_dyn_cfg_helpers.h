/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __ARM_DYN_CFG_HELPERS_H__
#define __ARM_DYN_CFG_HELPERS_H__

#include <stdint.h>

/* Function declaration */
int arm_dyn_get_hwconfig_info(void *dtb, int node,
		uint64_t *hw_config_addr, uint32_t *hw_config_size);
int arm_dyn_tb_fw_cfg_init(void *dtb, int *node);

#endif /* __ARM_DYN_CFG_HELPERS_H__ */

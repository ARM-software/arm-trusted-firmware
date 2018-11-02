/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ARM_DYN_CFG_HELPERS_H
#define ARM_DYN_CFG_HELPERS_H

#include <stddef.h>
#include <stdint.h>

/* Function declarations */
int arm_dyn_get_config_load_info(void *dtb, int node, unsigned int config_id,
		uint64_t *config_addr, uint32_t *config_size);
int arm_dyn_tb_fw_cfg_init(void *dtb, int *node);
int arm_dyn_get_disable_auth(void *dtb, int node, uint32_t *disable_auth);
int arm_get_dtb_mbedtls_heap_info(void *dtb, void **heap_addr,
	size_t *heap_size);
int arm_set_dtb_mbedtls_heap_info(void *dtb, void *heap_addr,
	size_t heap_size);

#endif /* ARM_DYN_CFG_HELPERS_H */

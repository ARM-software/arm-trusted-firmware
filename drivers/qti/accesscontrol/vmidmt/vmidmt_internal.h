/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VMIDMT_INTERNAL_H
#define VMIDMT_INTERNAL_H

#include <stdint.h>

#include <vmidmt_hal.h>

#define ACC_VMIDMT_ERR_INT_STATUS_REG_NUM 1
#define ACC_VMIDMT_ERR_NUM_PER_REG 32

#define VMIDMT_ERR_OPT                                                  \
	(HAL_VMIDMT_ERROR_O_SMCFCFG_EN | HAL_VMIDMT_ERROR_O_USFCFG_EN | \
	 HAL_VMIDMT_ERROR_O_GCFGFIE | HAL_VMIDMT_ERROR_O_GCFGFRE |      \
	 HAL_VMIDMT_ERROR_O_GFIE)

/* Default MemType when not overridden (implementation defined). */
#define ACC_VMIDMT_MEMTYPE_DEFAULT 0xFF
#define ACC_VMIDMT_MEMTYPE_STRONGLY_ORDERED 0

struct vmidmt_err_pos_to_hal_map {
	uint8_t bit_pos; /* Bit position in status register */
	uint8_t vmidmt; /* Corresponding HAL VMIDMT index */
};

struct vmidmt_cfg {
	uint8_t table; /* VMIDMT master port ID */
	uint8_t err_opt; /* Error reporting options */
	hal_vmidmt_secure_status_det *ssd_table; /* Pointer to SSD table */
	uint8_t ssd_table_num_elements; /* Number of SSD elements */
	uint8_t static_cfg; /* 1: configured statically at boot; 0: runtime */
};

struct vmidmt_map {
	uint8_t table; /* VMIDMT master port ID */
	uint8_t index; /* Master index before VMID mapping */
	uint32_t sid_list[5]; /* SID list */
	uint8_t num_sids; /* Number of SIDs */
	uint8_t vmid; /* VMID to assign */
	uint8_t static_cfg; /* 1: static at boot; 0: configured at runtime */
	uint8_t memattr; /* Non-default memory attribute override */
};

#endif /* VMIDMT_INTERNAL_H */

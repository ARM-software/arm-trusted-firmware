/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARM_TRUSTED_FIRMWARE_EXPORT_LIB_BL_AUX_PARAMS_EXP_H
#define ARM_TRUSTED_FIRMWARE_EXPORT_LIB_BL_AUX_PARAMS_EXP_H

/* EXPORT HEADER -- See include/export/README for details! -- EXPORT HEADER */

#include "../../drivers/gpio_exp.h"

/*
 * This API implements a lightweight parameter passing mechanism that can be
 * used to pass SoC Firmware configuration data from BL2 to BL31 by platforms or
 * configurations that do not want to depend on libfdt. It is structured as a
 * singly-linked list of parameter structures that all share the same common
 * header but may have different (and differently-sized) structure bodies after
 * that. The header contains a type field to indicate the parameter type (which
 * is used to infer the structure length and how to interpret its contents) and
 * a next pointer which contains the absolute physical address of the next
 * parameter structure. The next pointer in the last structure block is set to
 * NULL. The picture below shows how the parameters are kept in memory.
 *
 * head of list  ---> +----------------+ --+
 *                    |      type      |   |
 *                    +----------------+   |--> struct bl_aux_param
 *               +----|      next      |   |
 *               |    +----------------+ --+
 *               |    | parameter data |
 *               |    +----------------+
 *               |
 *               +--> +----------------+ --+
 *                    |      type      |   |
 *                    +----------------+   |--> struct bl_aux_param
 *           NULL <---|      next      |   |
 *                    +----------------+ --+
 *                    | parameter data |
 *                    +----------------+
 *
 * Note: The SCTLR_EL3.A bit (Alignment fault check enable) is set in TF-A, so
 * BL2 must ensure that each parameter struct starts on a 64-bit aligned address
 * to avoid alignment faults. Parameters may be allocated in any address range
 * accessible at the time of BL31 handoff (e.g. SRAM, DRAM, SoC-internal scratch
 * registers, etc.), in particular address ranges that may not be mapped in
 * BL31's page tables, so the parameter list must be parsed before the MMU is
 * enabled and any information that is required at a later point should be
 * deep-copied out into BL31-internal data structures.
 */

enum bl_aux_param_type {
	BL_AUX_PARAM_NONE = 0,
	BL_AUX_PARAM_VENDOR_SPECIFIC_FIRST = 0x1,
	/* 0x1 - 0x7fffffff can be used by vendor-specific handlers. */
	BL_AUX_PARAM_VENDOR_SPECIFIC_LAST = 0x7fffffff,
	BL_AUX_PARAM_GENERIC_FIRST = 0x80000001,
	BL_AUX_PARAM_COREBOOT_TABLE = BL_AUX_PARAM_GENERIC_FIRST,
	/* 0x80000001 - 0xffffffff are reserved for the generic handler. */
	BL_AUX_PARAM_GENERIC_LAST = 0xffffffff,
	/* Top 32 bits of the type field are reserved for future use. */
};

/* common header for all BL aux parameters */
struct bl_aux_param_header {
	uint64_t type;
	uint64_t next;
};

/* commonly useful parameter structures that can be shared by multiple types */
struct bl_aux_param_uint64 {
	struct bl_aux_param_header h;
	uint64_t value;
};

struct bl_aux_gpio_info {
	uint8_t polarity;
	uint8_t direction;
	uint8_t pull_mode;
	uint8_t reserved;
	uint32_t index;
};

struct bl_aux_param_gpio {
	struct bl_aux_param_header h;
	struct bl_aux_gpio_info gpio;
};

#endif /* ARM_TRUSTED_FIRMWARE_EXPORT_LIB_BL_AUX_PARAMS_EXP_H */

/*
 * Copyright (c) 2022, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef COLD_BOOT_H
#define COLD_BOOT_H

#include <stdint.h>

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
#define LINUX_KERNEL_32	(0)
#define LINUX_KERNEL_64	(1)
#define DEVINFO_SIZE	(4)

struct atf_arg_t {
	uint32_t atf_magic;
	uint32_t tee_support;
	uint64_t tee_entry;
	uint64_t tee_boot_arg_addr;
	uint32_t hwuid[4]; /* HW Unique id for t-base used */
	uint32_t HRID[8]; /* HW random id for t-base used */
	uint32_t devinfo[DEVINFO_SIZE];
};

struct mtk_bl31_fw_config {
	void *from_bl2; /* MTK boot tag */
	void *soc_fw_config;
	void *hw_config;
	void *reserved;
};

enum {
	BOOT_ARG_FROM_BL2,
	BOOT_ARG_SOC_FW_CONFIG,
	BOOT_ARG_HW_CONFIG,
	BOOT_ARG_RESERVED
};

struct kernel_info {
	uint64_t pc;
	uint64_t r0;
	uint64_t r1;
	uint64_t r2;
	uint64_t k32_64;
};

struct mtk_bl_param_t {
	uint64_t bootarg_loc;
	uint64_t bootarg_size;
	uint64_t bl33_start_addr;
	uint64_t atf_arg_addr;
};

void *get_mtk_bl31_fw_config(int index);
bool is_el1_2nd_bootloader(void);

#endif /* COLD_BOOT_H */

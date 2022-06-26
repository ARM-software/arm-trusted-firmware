/*
 * Copyright (c) 2016-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MTK_PLAT_COMMON_H
#define MTK_PLAT_COMMON_H

#include <stdint.h>

#include <common/bl_common.h>
#include <common/param_header.h>

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
#define SMC32_PARAM_MASK		(0xFFFFFFFF)

#define JEDEC_MTK_BKID U(4)
#define JEDEC_MTK_MFID U(0x26)

struct mtk_bl31_params {
       param_header_t h;
       image_info_t *bl31_image_info;
       entry_point_info_t *bl32_ep_info;
       image_info_t *bl32_image_info;
       entry_point_info_t *bl33_ep_info;
       image_info_t *bl33_image_info;
};

/* Declarations for mtk_plat_common.c */
uint32_t plat_get_spsr_for_bl32_entry(void);
uint32_t plat_get_spsr_for_bl33_entry(void);
void clean_top_32b_of_param(uint32_t smc_fid, u_register_t *x1,
				u_register_t *x2,
				u_register_t *x3,
				u_register_t *x4);
void bl31_prepare_kernel_entry(uint64_t k32_64);
void enable_ns_access_to_cpuectlr(void);
void boot_to_kernel(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4);
uint64_t get_kernel_info_pc(void);
uint64_t get_kernel_info_r0(void);
uint64_t get_kernel_info_r1(void);
uint64_t get_kernel_info_r2(void);

extern struct atf_arg_t gteearg;
#endif /* MTK_PLAT_COMMON_H */

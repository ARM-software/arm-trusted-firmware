/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
#ifndef __MTK_PLAT_COMMON_H__
#define __MTK_PLAT_COMMON_H__
#include <stdint.h>
/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
#define DEVINFO_SIZE 4
#define LINUX_KERNEL_32 0
#define SMC32_PARAM_MASK		(0xFFFFFFFF)

struct atf_arg_t {
	unsigned int atf_magic;
	unsigned int tee_support;
	unsigned int tee_entry;
	unsigned int tee_boot_arg_addr;
	unsigned int hwuid[4];     /* HW Unique id for t-base used */
	unsigned int HRID[2];      /* HW random id for t-base used */
	unsigned int atf_log_port;
	unsigned int atf_log_baudrate;
	unsigned int atf_log_buf_start;
	unsigned int atf_log_buf_size;
	unsigned int atf_irq_num;
	unsigned int devinfo[DEVINFO_SIZE];
	unsigned int atf_aee_debug_buf_start;
	unsigned int atf_aee_debug_buf_size;
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
	uint64_t tee_info_addr;
};

/* Declarations for mtk_plat_common.c */
uint32_t plat_get_spsr_for_bl32_entry(void);
uint32_t plat_get_spsr_for_bl33_entry(void);
void clean_top_32b_of_param(uint32_t smc_fid, uint64_t *x1,
				uint64_t *x2,
				uint64_t *x3,
				uint64_t *x4);
void bl31_prepare_kernel_entry(uint64_t k32_64);
void enable_ns_access_to_cpuectlr(void);
void boot_to_kernel(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4);
uint64_t get_kernel_info_pc(void);
uint64_t get_kernel_info_r0(void);
uint64_t get_kernel_info_r1(void);
uint64_t get_kernel_info_r2(void);

extern struct atf_arg_t gteearg;
#endif

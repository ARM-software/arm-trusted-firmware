/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PMU_SRAM_H__
#define __PMU_SRAM_H__

/*****************************************************************************
 * define data offset in struct psram_data
 *****************************************************************************/
#define PSRAM_DT_SP		0x0
#define PSRAM_DT_DDR_FUNC	0x8
#define PSRAM_DT_DDR_DATA	0x10
#define PSRAM_DT_DDRFLAG	0x18
#define PSRAM_DT_MPIDR		0x1c
#define PSRAM_DT_END		0x20
/******************************************************************************
 * Allocate data region for struct psram_data_t in pmusram
 ******************************************************************************/
/* Needed aligned 16 bytes for sp stack top */
#define PSRAM_DT_SIZE		(((PSRAM_DT_END + 16) / 16) * 16)
#define PSRAM_DT_BASE		((PMUSRAM_BASE + PMUSRAM_RSIZE) - PSRAM_DT_SIZE)
#define PSRAM_SP_TOP		PSRAM_DT_BASE

#ifndef __ASSEMBLY__

struct psram_data_t {
	uint64_t sp;
	uint64_t ddr_func;
	uint64_t ddr_data;
	uint32_t ddr_flag;
	uint32_t boot_mpidr;
};

CASSERT(sizeof(struct psram_data_t) <= PSRAM_DT_SIZE,
	assert_psram_dt_size_mismatch);
CASSERT(__builtin_offsetof(struct psram_data_t, sp) == PSRAM_DT_SP,
	assert_psram_dt_sp_offset_mistmatch);
CASSERT(__builtin_offsetof(struct psram_data_t, ddr_func) == PSRAM_DT_DDR_FUNC,
	assert_psram_dt_ddr_func_offset_mistmatch);
CASSERT(__builtin_offsetof(struct psram_data_t, ddr_data) == PSRAM_DT_DDR_DATA,
	assert_psram_dt_ddr_data_offset_mistmatch);
CASSERT(__builtin_offsetof(struct psram_data_t, ddr_flag) == PSRAM_DT_DDRFLAG,
	assert_psram_dt_ddr_flag_offset_mistmatch);
CASSERT(__builtin_offsetof(struct psram_data_t, boot_mpidr) == PSRAM_DT_MPIDR,
	assert_psram_dt_mpidr_offset_mistmatch);
void u32_align_cpy(uint32_t *dst, const uint32_t *src, size_t bytes);

#endif  /* __ASSEMBLY__ */

#endif

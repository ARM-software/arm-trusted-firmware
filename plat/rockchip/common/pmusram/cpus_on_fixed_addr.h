/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CPU_ON_FIXED_ADDR_H__
#define __CPU_ON_FIXED_ADDR_H__

/*****************************************************************************
 * define data offset in struct psram_data
 *****************************************************************************/
#define PSRAM_DT_SP		0x0
#define PSRAM_DT_DDR_FUNC	0x8
#define PSRAM_DT_DDR_DATA	0x10
#define PSRAM_DT_DDRFLAG	0x18
#define PSRAM_DT_MPIDR		0x1c
#define PSRAM_DT_PM_FLAG	0x20
#define PSRAM_DT_END		0x24

/* reserve 4 byte */
#define PSRAM_DT_END_RES4	(PSRAM_DT_END + 4)

#define PSRAM_DT_SIZE_WORDS	(PSRAM_DT_END_RES4 / 4)

#define PM_WARM_BOOT_SHT	0
#define PM_WARM_BOOT_BIT	(1 << PM_WARM_BOOT_SHT)

#ifndef __ASSEMBLER__

struct psram_data_t {
	uint64_t sp;
	uint64_t ddr_func;
	uint64_t ddr_data;
	uint32_t ddr_flag;
	uint32_t boot_mpidr;
	uint32_t pm_flag;
};

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

extern void *sys_sleep_flag_sram;

#endif  /* __ASSEMBLER__ */

#endif

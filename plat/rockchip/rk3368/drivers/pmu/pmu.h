/*
 * Copyright (C) 2015, Fuzhou Rockchip Electronics Co., Ltd
 * Author: Tony.Xie
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __PMU_H__
#define __PMU_H__

#ifndef __ASSEMBLY__

/*****************************************************************************
 * pmu con,reg
 *****************************************************************************/
#define PMU_WKUP_CFG0		0x0
#define PMU_WKUP_CFG1		0x4
#define PMU_WKUP_CFG2		0x8
#define PMU_TIMEOUT_CNT		0x7c
#define PMU_PWRDN_CON		0xc
#define PMU_PWRDN_ST		0x10
#define PMU_CORE_PWR_ST		0x38

typedef enum {
	clstl_cpu_wfe = 2,
	clstl_cpu_wfi = 6,
	clstb_cpu_wfe = 12,
	clstb_cpu_wfi = 16,
} pmu_core_pwr_t;

typedef enum {
	PD_CPUL0 = 0,
	PD_CPUL1,
	PD_CPUL2,
	PD_CPUL3,
	PD_SCUL,
	PD_CPUB0 = 5,
	PD_CPUB1,
	PD_CPUB2,
	PD_CPUB3,
	PD_SCUB = 9,
	PD_PERI = 13,
	PD_VIDEO,
	PD_VIO,
	PD_GPU0,
	PD_GPU1,
	PD_END
} pmu_pdid_t;

/*****************************************************************************
 * power domain on or off
 *****************************************************************************/
typedef enum {
	pmu_pd_on = 0,
	pmu_pd_off = 1
} pmu_pd_state_t;

#define CKECK_WFE_MSK		0x1
#define CKECK_WFI_MSK		0x10
#define CKECK_WFEI_MSK		0x11

extern void *pmu_cpuson_entrypoint_start;
extern void *pmu_cpuson_entrypoint_end;

extern uint64_t cpuson_entry_point[PLATFORM_CORE_COUNT];
extern uint32_t cpuson_flags[PLATFORM_CORE_COUNT];
void pmusram_prepare(void);
int pmu_cpus_hotplug(unsigned long mpidr, pmu_pd_state_t off,
		     uint64_t entrypoint);
void nonboot_cpus_off(void);
void pmu_init(void);
#endif /* __ASSEMBLY__ */

/*****************************************************************************
 * cpu up status
 *****************************************************************************/
#define PMU_SYS_SLP_MODE	0xa5
#define PMU_SYS_ON_MODE		0x0

/*****************************************************************************
 * define data offset in struct psram_data
 *****************************************************************************/
#define PSRAM_DT_SYS_MODE	0x0
#define PSRAM_DT_MPIDR		0x4
#define PSRAM_DT_SP		0x8
#define PSRAM_DT_DDR_FUNC	0x10
#define PSRAM_DT_DDR_DATA	0x14
#define PSRAM_DT_DDRFLAG	0x18

#endif /* __PMU_H__ */

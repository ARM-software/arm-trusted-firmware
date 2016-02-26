/*
 * Copyright (C) 2016, Fuzhou Rockchip Electronics Co., Ltd.
 * All rights reserved.
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

/*
 * the shift of bits for cores status
 */
enum pmu_core_pwrst_shift {
	clstl_cpu_wfe = 2,
	clstl_cpu_wfi = 6,
	clstb_cpu_wfe = 12,
	clstb_cpu_wfi = 16
};

enum pmu_pdid {
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
};

/*****************************************************************************
 * power domain on or off
 *****************************************************************************/
enum pmu_pd_state {
	pmu_pd_on = 0,
	pmu_pd_off = 1
};

#define CKECK_WFE_MSK		0x1
#define CKECK_WFI_MSK		0x10
#define CKECK_WFEI_MSK		0x11

#define PD_CTR_LOOP		500
#define CHK_CPU_LOOP		500

extern void *pmu_cpuson_entrypoint_start;
extern void *pmu_cpuson_entrypoint_end;

extern uint64_t cpuson_entry_point[PLATFORM_CORE_COUNT];
extern uint32_t cpuson_flags[PLATFORM_CORE_COUNT];
void pmusram_prepare(void);
int pmu_cpus_hotplug(unsigned long mpidr, uint32_t off, uint64_t entrypoint);
void nonboot_cpus_off(void);
void pmu_init(void);
int pmu_soc_power_off(void);
int cpus_power_domain_by_wfi(unsigned long mpidr);

#endif /* __ASSEMBLY__ */

/*****************************************************************************
 * cpu up status
 *****************************************************************************/
#define PMU_SYS_SLP_MODE	0xa5
#define PMU_SYS_ON_MODE		0x0

/*****************************************************************************
 * define data offset in struct psram_data
 *****************************************************************************/
#define PSRAM_DT_SP		0x0
#define PSRAM_DT_DDR_FUNC	0x8
#define PSRAM_DT_DDR_DATA	0x10
#define PSRAM_DT_DDRFLAG	0x18
#define PSRAM_DT_SYS_MODE	0x1c
#define PSRAM_DT_MPIDR		0x20
#define PSRAM_DT_END		0x24

/******************************************************************************
 * allocate pmu sram
 ******************************************************************************/
/*
 * Needed aligned 16 bytes for sp stack top
 */
#define PSRAM_DT_SIZE		(((PSRAM_DT_END + 16) / 16) * 16)
#define PSRAM_DT_BASE		((PMUSRAM_BASE + 0x1000) - PSRAM_DT_SIZE)

#endif /* __PMU_H__ */

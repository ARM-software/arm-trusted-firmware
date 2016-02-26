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

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <platform_def.h>
#include <plat_private.h>
#include <rk3368_def.h>

/*
 * The struct is used in pmu_cpus_on.S which
 * gets the data of the struct by the following index
 * #define PSRAM_DT_SP		0x0
 * #define PSRAM_DT_DDR_FUNC	0x8
 * #define PSRAM_DT_DDR_DATA	0x10
 * #define PSRAM_DT_DDRFLAG	0x18
 * #define PSRAM_DT_SYS_MODE	0x1c
 * #define PSRAM_DT_MPIDR	0x20
 */
struct psram_data_t {
	uint64_t sp;
	uint64_t ddr_func;
	uint64_t ddr_data;
	uint32_t ddr_flag;
	uint32_t sys_mode;
	uint32_t boot_mpidr;
};

static struct psram_data_t *psram_sleep_cfg =
	(struct psram_data_t *)PSRAM_DT_BASE;

/*****************************************************************************
 * sram only surpport 32-bits access
 ******************************************************************************/
static void
u32_align_cpy(unsigned int *dst, const unsigned int *scr, size_t len)
{
	uint32_t i;

	for (i = 0; i < len; i++)
		dst[i] = scr[i];
}

void pmusram_prepare(void)
{
	uint32_t *sram_dst, *sram_src;
	size_t sram_size = 2;

	/*
	 * pmu sram code and data prepare
	 */
	sram_dst = (uint32_t *)PMUSRAM_BASE;
	sram_src = (uint32_t *)&pmu_cpuson_entrypoint_start;
	sram_size = (uint32_t *)&pmu_cpuson_entrypoint_end -
		    (uint32_t *)sram_src;

	u32_align_cpy(sram_dst, sram_src, sram_size);

	psram_sleep_cfg->sp = PSRAM_DT_BASE;
}

static inline uint32_t pmu_power_domain_st(uint32_t pd)
{
	uint32_t pwrdn_st = mmio_read_32(PMU_BASE + PMU_PWRDN_ST) &  BIT(pd);

	if (pwrdn_st)
		return pmu_pd_off;
	else
		return pmu_pd_on;
}

static int pmu_power_domain_ctr(uint32_t pd, uint32_t pd_state)
{
	uint32_t val = mmio_read_32(PMU_BASE + PMU_PWRDN_CON);
	uint32_t loop = 0;

	if (pd_state)
		val |=  BIT(pd);
	else
		val &= ~BIT(pd);

	mmio_write_32(PMU_BASE + PMU_PWRDN_CON, val);
	dsb();

	while (!(pmu_power_domain_st(pd) == pd_state) && (loop < PD_CTR_LOOP)) {
		udelay(1);
		loop++;
	}

	if (pmu_power_domain_st(pd) != pd_state)
		WARN("%s: %d, %d, error!\n", __func__, pd, pd_state);

	return 0;
}

static int
check_cpu_wfie(uint32_t cluster_id, uint32_t cpu_id, uint32_t wfie_msk)
{
	uint32_t loop = 0;

	if (cluster_id)
		wfie_msk <<= (clstb_cpu_wfe + cpu_id);
	else
		wfie_msk <<= (clstl_cpu_wfe + cpu_id);

	while (!(mmio_read_32(PMU_BASE + PMU_CORE_PWR_ST) & wfie_msk) &&
	       (loop < CHK_CPU_LOOP)) {
		udelay(1);
		loop++;
	}

	if ((mmio_read_32(PMU_BASE + PMU_CORE_PWR_ST) & wfie_msk) == 0)
		WARN("%s: %d, %d, %d, error!\n", __func__,
		     cluster_id, cpu_id, wfie_msk);

	return 0;
}

int cpus_power_domain_by_wfi(unsigned long mpidr)
{
	return 0;
}

static int cpus_id_power_domain(uint32_t cluster,
				uint32_t cpu,
				uint32_t pd_state,
				uint32_t wfie_msk)
{
	uint32_t pd;

	if (cluster)
		pd = PD_CPUB0 + cpu;
	else
		pd = PD_CPUL0 + cpu;

	if (pmu_power_domain_st(pd) == pd_state)
		return 0;

	if (pd_state == pmu_pd_off) {
		if (check_cpu_wfie(cluster, cpu, wfie_msk))
			return -1;
	}

	return pmu_power_domain_ctr(pd, pd_state);
}

int
pmu_cpus_hotplug(unsigned long mpidr, uint32_t pd_state, uint64_t entrypoint)
{
	uint32_t cpu, cluster;
	uint32_t cpuon_id;

	cpu = MPIDR_AFFLVL0_VAL(mpidr);
	cluster = MPIDR_AFFLVL1_VAL(mpidr);

	/*
	 * Make sure the cpu is off,Before power up the cpu!
	 */
	if (pd_state == pmu_pd_on) {
		cpus_id_power_domain(cluster, cpu, pmu_pd_off, CKECK_WFI_MSK);

		cpuon_id = (cluster * PLATFORM_CLUSTER0_CORE_COUNT) + cpu;
		assert(cpuson_flags[cpuon_id] == 0);
		cpuson_flags[cpuon_id] = PMU_CPU_HOTPLUG;
		cpuson_entry_point[cpuon_id] = entrypoint;
		mmio_write_32(SGRF_BASE + SGRF_SOC_CON(1 + cluster),
			      (PMUSRAM_BASE >> 16) | 0xffff0000);
		dsb();
	}

	cpus_id_power_domain(cluster, cpu, pd_state, CKECK_WFI_MSK);

	if (pd_state == pmu_pd_on) {
		mmio_write_32(SGRF_BASE + SGRF_SOC_CON(1 + cluster),
			      0xffff0000 | 0xffff);
	}

	return 0;
}

void nonboot_cpus_off(void)
{
	uint32_t boot_cpu, boot_cluster, cpu;

	boot_cpu = MPIDR_AFFLVL0_VAL(read_mpidr_el1());
	boot_cluster = MPIDR_AFFLVL1_VAL(read_mpidr_el1());

	/* turn off noboot cpus */
	for (cpu = 0; cpu < PLATFORM_CLUSTER0_CORE_COUNT; cpu++) {
		if (!boot_cluster && (cpu == boot_cpu))
			continue;
		cpus_id_power_domain(0, cpu, pmu_pd_off, CKECK_WFE_MSK);
	}

	for (cpu = 0; cpu < PLATFORM_CLUSTER1_CORE_COUNT; cpu++) {
		if (boot_cluster && (cpu == boot_cpu))
			continue;
		cpus_id_power_domain(1, cpu, pmu_pd_off, CKECK_WFE_MSK);
	}
}

int pmu_soc_power_off(void)
{
	return 0;
}

void pmu_init(void)
{
	uint32_t cpu;

	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++)
		cpuson_flags[cpu] = 0;

	psram_sleep_cfg->sys_mode = PMU_SYS_ON_MODE;

	psram_sleep_cfg->boot_mpidr = read_mpidr_el1() & 0xffff;

	INFO("%s(%d): pd status %x\n", __func__, __LINE__,
	     mmio_read_32(PMU_BASE + PMU_PWRDN_ST));
}

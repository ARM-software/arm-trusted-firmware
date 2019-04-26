/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMU_COM_H
#define PMU_COM_H

#ifndef CHECK_CPU_WFIE_BASE
#define CHECK_CPU_WFIE_BASE (PMU_BASE + PMU_CORE_PWR_ST)
#endif
/*
 * Use this macro to instantiate lock before it is used in below
 * rockchip_pd_lock_xxx() macros
 */
DECLARE_BAKERY_LOCK(rockchip_pd_lock);

/*
 * These are wrapper macros to the powe domain Bakery Lock API.
 */
#define rockchip_pd_lock_init() bakery_lock_init(&rockchip_pd_lock)
#define rockchip_pd_lock_get() bakery_lock_get(&rockchip_pd_lock)
#define rockchip_pd_lock_rls() bakery_lock_release(&rockchip_pd_lock)

/*****************************************************************************
 * power domain on or off
 *****************************************************************************/
enum pmu_pd_state {
	pmu_pd_on = 0,
	pmu_pd_off = 1
};

#pragma weak plat_ic_get_pending_interrupt_id
#pragma weak pmu_power_domain_ctr
#pragma weak check_cpu_wfie

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
	uint32_t val;
	uint32_t loop = 0;
	int ret = 0;

	rockchip_pd_lock_get();

	val = mmio_read_32(PMU_BASE + PMU_PWRDN_CON);
	if (pd_state == pmu_pd_off)
		val |=  BIT(pd);
	else
		val &= ~BIT(pd);

	mmio_write_32(PMU_BASE + PMU_PWRDN_CON, val);
	dsb();

	while ((pmu_power_domain_st(pd) != pd_state) && (loop < PD_CTR_LOOP)) {
		udelay(1);
		loop++;
	}

	if (pmu_power_domain_st(pd) != pd_state) {
		WARN("%s: %d, %d, error!\n", __func__, pd, pd_state);
		ret = -EINVAL;
	}

	rockchip_pd_lock_rls();

	return ret;
}

static int check_cpu_wfie(uint32_t cpu_id, uint32_t wfie_msk)
{
	uint32_t cluster_id, loop = 0;

	if (cpu_id >= PLATFORM_CLUSTER0_CORE_COUNT) {
		cluster_id = 1;
		cpu_id -= PLATFORM_CLUSTER0_CORE_COUNT;
	} else {
		cluster_id = 0;
	}

	/*
	 * wfe/wfi tracking not possible, hopefully the host
	 * was sucessful in enabling wfe/wfi.
	 * We'll give a bit of additional time, like the kernel does.
	 */
	if ((cluster_id && clstb_cpu_wfe < 0) ||
	    (!cluster_id && clstl_cpu_wfe < 0)) {
		mdelay(1);
		return 0;
	}

	if (cluster_id)
		wfie_msk <<= (clstb_cpu_wfe + cpu_id);
	else
		wfie_msk <<= (clstl_cpu_wfe + cpu_id);

	while (!(mmio_read_32(CHECK_CPU_WFIE_BASE) & wfie_msk) &&
	       (loop < CHK_CPU_LOOP)) {
		udelay(1);
		loop++;
	}

	if ((mmio_read_32(CHECK_CPU_WFIE_BASE) & wfie_msk) == 0) {
		WARN("%s: %d, %d, %d, error!\n", __func__,
		     cluster_id, cpu_id, wfie_msk);
		return -EINVAL;
	}

	return 0;
}

#endif /* PMU_COM_H */

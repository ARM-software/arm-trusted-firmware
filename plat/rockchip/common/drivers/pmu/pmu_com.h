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

#ifndef __PMU_COM_H__
#define __PMU_COM_H__

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

	if (cluster_id)
		wfie_msk <<= (clstb_cpu_wfe + cpu_id);
	else
		wfie_msk <<= (clstl_cpu_wfe + cpu_id);

	while (!(mmio_read_32(PMU_BASE + PMU_CORE_PWR_ST) & wfie_msk) &&
	       (loop < CHK_CPU_LOOP)) {
		udelay(1);
		loop++;
	}

	if ((mmio_read_32(PMU_BASE + PMU_CORE_PWR_ST) & wfie_msk) == 0) {
		WARN("%s: %d, %d, %d, error!\n", __func__,
		     cluster_id, cpu_id, wfie_msk);
		return -EINVAL;
	}

	return 0;
}

#endif /* __PMU_COM_H__ */

/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#if MTK_PUBEVENT_ENABLE
#include <lib/pm/mtk_pm.h>
#endif
#include <ptp3_plat.h>

#define PTP3_CORE_OFT(core)	(0x800 * (core))

static void ptp3_init(unsigned int core)
{
	unsigned int i, addr, value;

	if (core < PTP3_CFG_CPU_START_ID_B) {
		mmio_clrsetbits_32(ptp3_cfg1[0][PTP3_CFG_ADDR], PTP3_CFG1_MASK,
				   ptp3_cfg1[0][PTP3_CFG_VALUE]);
	} else {
		mmio_clrsetbits_32(ptp3_cfg1[1][PTP3_CFG_ADDR], PTP3_CFG1_MASK,
				   ptp3_cfg1[1][PTP3_CFG_VALUE]);
	}

	if (core < PTP3_CFG_CPU_START_ID_B) {
		for (i = 0; i < NR_PTP3_CFG2_DATA; i++) {
			addr = ptp3_cfg2[i][PTP3_CFG_ADDR] + PTP3_CORE_OFT(core);
			value = ptp3_cfg2[i][PTP3_CFG_VALUE];

			mmio_write_32(addr, value);
		}
	} else {
		for (i = 0; i < NR_PTP3_CFG2_DATA; i++) {
			addr = ptp3_cfg2[i][PTP3_CFG_ADDR] + PTP3_CORE_OFT(core);

			if (i == 2) {
				value = ptp3_cfg2[i][PTP3_CFG_VALUE] + 0x5E0;
			} else {
				value = ptp3_cfg2[i][PTP3_CFG_VALUE];
			}
			mmio_write_32(addr, value);
		}
	}

	if (core < PTP3_CFG_CPU_START_ID_B) {
		addr = ptp3_cfg3[PTP3_CFG_ADDR] + PTP3_CORE_OFT(core);
		value = ptp3_cfg3[PTP3_CFG_VALUE];
	} else {
		addr = ptp3_cfg3_ext[PTP3_CFG_ADDR] + PTP3_CORE_OFT(core);
		value = ptp3_cfg3_ext[PTP3_CFG_VALUE];
	}
	mmio_write_32(addr, value & PTP3_CFG3_MASK1);
	mmio_write_32(addr, value & PTP3_CFG3_MASK2);
	mmio_write_32(addr, value & PTP3_CFG3_MASK3);
}

static void pdp_proc_arm_write(unsigned int pdp_n)
{
	unsigned long v = 0;

	dsb();
	__asm__ volatile ("mrs %0, S3_6_C15_C2_0" : "=r" (v));
	v |= (UL(0x0) << 52);
	v |= (UL(0x1) << 53);
	v |= (UL(0x0) << 54);
	v |= (UL(0x0) << 48);
	v |= (UL(0x1) << 49);
	__asm__ volatile ("msr S3_6_C15_C2_0, %0" : : "r" (v));
	dsb();
}

static void pdp_init(unsigned int pdp_cpu)
{
	if ((pdp_cpu >= PTP3_CFG_CPU_START_ID_B) && (pdp_cpu < NR_PTP3_CFG_CPU)) {
		pdp_proc_arm_write(pdp_cpu);
	}
}

void ptp3_core_init(unsigned int core)
{
	ptp3_init(core);
	pdp_init(core);
}

void ptp3_core_deinit(unsigned int core)
{
	/* TBD */
}

#if MTK_PUBEVENT_ENABLE
/* Handle for power on domain */
void *ptp3_handle_pwr_on_event(const void *arg)
{
	if (arg != NULL) {
		struct mt_cpupm_event_data *data = (struct mt_cpupm_event_data *)arg;

		if ((data->pwr_domain & MT_CPUPM_PWR_DOMAIN_CORE) > 0) {
			ptp3_core_init(data->cpuid);
		}
	}
	return (void *)arg;
}
MT_CPUPM_SUBCRIBE_EVENT_PWR_ON(ptp3_handle_pwr_on_event);

/* Handle for power off domain */
void *ptp3_handle_pwr_off_event(const void *arg)
{
	if (arg != NULL) {
		struct mt_cpupm_event_data *data = (struct mt_cpupm_event_data *)arg;

		if ((data->pwr_domain & MT_CPUPM_PWR_DOMAIN_CORE) > 0) {
			ptp3_core_deinit(data->cpuid);
		}
	}
	return (void *)arg;
}
MT_CPUPM_SUBCRIBE_EVENT_PWR_OFF(ptp3_handle_pwr_off_event);
#else
#pragma message "PSCI hint not enable"
#endif

/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdbool.h>

#include <platform_def.h>
#include <pwr_ctrl.h>

/*Do the necessary GPC, SRC, BLK_CTRL_S init */
void pwr_sys_init(void)
{
	unsigned int cpu;

	/*
	 * Assigned A55 cluster to 3, m33 to 2, A55 CORE0 & CORE1 to 0/1.
	 * domain0/1 only used for trigger LPM of themselves. A55 cluster & M33's
	 * domain assignment should be align with the TRDC DID.
	 */
	gpc_assign_domains(0x3102);

	/* CA55 core0/1 config */
	for (cpu = CPU_A55C0; cpu <= CPU_A55_PLAT; cpu++) {
		/* clear the cpu sleep hold */
		gpc_clear_cpu_sleep_hold(cpu);
		/* use gic wakeup source by default */
		gpc_select_wakeup_gic(cpu);
		/*
		 * Ignore A55 core0/1's LPM trigger for system sleep.
		 * normally, for A55 side, only the A55 cluster(plat)
		 * domain will be used to trigger the system wide low
		 * power mode transition.
		 */
		if (cpu != CPU_A55_PLAT) {
			gpc_force_cpu_suspend(cpu);
		}
	}

	/* boot core(A55C0) */
	src_mem_lpm_en(SRC_A55P0_MEM, MEM_OFF);
	/* For A55 core, only need to be on in RUN mode */
	src_mix_set_lpm(SRC_A55C0, 0x0, CM_MODE_WAIT);
	/* whitelist: 0x1 for domain 0 only */
	src_authen_config(SRC_A55C0, 0x1, 0x1);

	/* A55 cluster */
	gpc_select_wakeup_gic(CPU_A55_PLAT);
	gpc_clear_cpu_sleep_hold(CPU_A55_PLAT);

	/* SCU MEM must be OFF when A55 PLAT OFF */
	src_mem_lpm_en(SRC_A55_SCU_MEM, MEM_OFF);
	/* L3 memory in retention by default */
	src_mem_lpm_en(SRC_A55_L3_MEM, MEM_RETN);

	src_mix_set_lpm(SRC_A55P, 0x3, 0x1);
	/* whitelist: 0x8 for domain 3 only */
	src_authen_config(SRC_A55P, 0x8, 0x1);

	/* enable the HW LP handshake between S401 & A55 cluster */
	mmio_setbits_32(BLK_CTRL_S_BASE + HW_LP_HANDHSK, BIT(5));
}


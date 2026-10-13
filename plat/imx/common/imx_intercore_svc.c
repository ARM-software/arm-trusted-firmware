/*
 * Copyright 2024-2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <plat/common/platform.h>

#include <imx_intercore_svc.h>
#include <imx_interrupt_mgmt.h>
#include <platform_def.h>

static percpu_svc_inf_t svc_info[PLATFORM_CORE_COUNT];

static uint64_t logical_id_to_hwid(unsigned int cpu_id)
{
	uint64_t hwid = cpu_id;

	if ((read_mpidr_el1() & MPIDR_MT_MASK) != 0U) {
		hwid = cpu_id << MPIDR_AFF1_SHIFT;
	}

	return hwid;
}

int register_intercore_cpuoff_svc(interrupt_type_handler_t handler)
{
	percpu_svc_inf_t *inf;
	uint32_t i;

	if (!handler) {
		return -EINVAL;
	}

	for (i = 0; i < PLATFORM_CORE_COUNT; i++) {
		inf = &svc_info[i];
		inf->cpu_off_svc.enabled = true;
		inf->cpu_off_svc.handler = handler;
	}

	return 0;
}

int trigger_intercore_cpuoff_svc(unsigned int cpu)
{
	percpu_svc_inf_t *inf;

	if (cpu >= PLATFORM_CORE_COUNT) {
		return -EINVAL;
	}

	inf = &svc_info[cpu];
	inf->cpu_off_svc.trigged = true;
	inf->cpu_off_svc.result = 0;

	plat_ic_raise_el3_sgi(INTERCORE_SVC_IRQ, logical_id_to_hwid(cpu));

	/* Wait 1ms in case of returned error from cpu_off_svc handler */
	mdelay(1);

	return inf->cpu_off_svc.result;
}

int register_intercore_dvfs_svc(interrupt_type_handler_t handler)
{
	percpu_svc_inf_t *inf;
	uint32_t i;

	if (!handler) {
		return -EINVAL;
	}

	for (i = 0U; i < PLATFORM_CORE_COUNT; i++) {
		inf = &svc_info[i];
		inf->dvfs_svc.enabled = true;
		inf->dvfs_svc.handler = handler;
	}

	return 0;
}

int trigger_intercore_dvfs_svc(unsigned int cpu)
{
	percpu_svc_inf_t *inf;

	if (cpu >= PLATFORM_CORE_COUNT) {
		return -EINVAL;
	}

	inf = &svc_info[cpu];
	inf->dvfs_svc.trigged = true;
	inf->dvfs_svc.result = 0;

	plat_ic_raise_el3_sgi(INTERCORE_SVC_IRQ, logical_id_to_hwid(cpu));

	return 0;
}

static uint64_t intercore_service(uint32_t id, uint32_t flags,
		void *handle, void *cookie)
{
	unsigned int cpu_id = plat_my_core_pos();
	percpu_svc_inf_t *inf = &svc_info[cpu_id];

	if (inf->cpu_off_svc.enabled && inf->cpu_off_svc.trigged && inf->cpu_off_svc.handler) {
		inf->cpu_off_svc.trigged = false;
		inf->cpu_off_svc.result = inf->cpu_off_svc.handler(id, flags, handle, cookie);
	}

	if (inf->dvfs_svc.enabled && inf->dvfs_svc.trigged && inf->dvfs_svc.handler) {
		inf->dvfs_svc.trigged = false;
		inf->dvfs_svc.result = inf->dvfs_svc.handler(id, flags, handle, cookie);
	}

	return 0;
}

void imx_intercore_svc_init(void)
{
	int rc;

	/* Register the EL3 handler for Intercore service */
	rc = request_intr_type_el3(INTERCORE_SVC_IRQ, intercore_service);
	if (rc) {
		panic();
	}
}

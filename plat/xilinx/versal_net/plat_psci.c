/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2021-2022, Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <plat_arm.h>

#include <plat_private.h>

#define PM_RET_ERROR_NOFEATURE U(19)

#define PM_IOCTL	34U

static uintptr_t versal_net_sec_entry;

static void zynqmp_cpu_standby(plat_local_state_t cpu_state)
{
	dsb();
	wfi();
}

static int32_t zynqmp_nopmu_pwr_domain_on(u_register_t mpidr)
{
	uint32_t cpu_id = plat_core_pos_by_mpidr(mpidr);
	uint32_t cpu = cpu_id % PLATFORM_CORE_COUNT_PER_CLUSTER;
	uint32_t cluster = cpu_id / PLATFORM_CORE_COUNT_PER_CLUSTER;
	uintptr_t apu_cluster_base = 0, apu_pcli_base, apu_pcli_cluster = 0;
	uintptr_t rst_apu_cluster = PSX_CRF + RST_APU0_OFFSET + (cluster * 0x4);

	VERBOSE("%s: mpidr: 0x%lx, cpuid: %x, cpu: %x, cluster: %x\n",
		__func__, mpidr, cpu_id, cpu, cluster);

	if (cpu_id == -1) {
		return PSCI_E_INTERN_FAIL;
	}

	if (platform_id == VERSAL_NET_SPP && cluster > 1) {
		panic();
	}

	if (cluster > 3) {
		panic();
	}

	apu_pcli_cluster = APU_PCLI + APU_PCLI_CLUSTER_OFFSET + (cluster * APU_PCLI_CLUSTER_STEP);
	apu_cluster_base = APU_CLUSTER0 + (cluster * APU_CLUSTER_STEP);

	/* Enable clock */
	mmio_setbits_32(PSX_CRF + ACPU0_CLK_CTRL + (cluster * 0x4), ACPU_CLK_CTRL_CLKACT);

	/* Enable cluster states */
	mmio_setbits_32(apu_pcli_cluster + PCLI_PSTATE_OFFSET, PCLI_PSTATE_VAL_SET);
	mmio_setbits_32(apu_pcli_cluster + PCLI_PREQ_OFFSET, PREQ_CHANGE_REQUEST);

	/* assert core reset */
	mmio_setbits_32(rst_apu_cluster, ((RST_APU_COLD_RESET|RST_APU_WARN_RESET) << cpu));

	/* program RVBAR */
	mmio_write_32(apu_cluster_base + APU_RVBAR_L_0 + (cpu << 3),
		      (uint32_t)versal_net_sec_entry);
	mmio_write_32(apu_cluster_base + APU_RVBAR_H_0 + (cpu << 3),
		      versal_net_sec_entry >> 32);

	/* de-assert core reset */
	mmio_clrbits_32(rst_apu_cluster, ((RST_APU_COLD_RESET|RST_APU_WARN_RESET) << cpu));

	/* clear cluster resets */
	mmio_clrbits_32(rst_apu_cluster, RST_APU_CLUSTER_WARM_RESET);
	mmio_clrbits_32(rst_apu_cluster, RST_APU_CLUSTER_COLD_RESET);

	apu_pcli_base = APU_PCLI + (APU_PCLI_CPU_STEP * cpu) +
			(APU_PCLI_CLUSTER_CPU_STEP * cluster);

	mmio_write_32(apu_pcli_base + PCLI_PSTATE_OFFSET, PCLI_PSTATE_VAL_CLEAR);
	mmio_write_32(apu_pcli_base + PCLI_PREQ_OFFSET, PREQ_CHANGE_REQUEST);

	return PSCI_E_SUCCESS;
}

static void zynqmp_nopmu_pwr_domain_off(const psci_power_state_t *target_state)
{
}

static void __dead2 zynqmp_nopmu_system_reset(void)
{
	while (1)
		wfi();
}

static int32_t zynqmp_validate_ns_entrypoint(uint64_t ns_entrypoint)
{
	return PSCI_E_SUCCESS;
}

static void zynqmp_pwr_domain_suspend(const psci_power_state_t *target_state)
{
}

static void zynqmp_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	plat_versal_net_gic_pcpu_init();
	plat_versal_net_gic_cpuif_enable();
}

static void zynqmp_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
}

static void __dead2 zynqmp_system_off(void)
{
	while (1)
		wfi();
}

static int32_t zynqmp_validate_power_state(uint32_t power_state, psci_power_state_t *req_state)
{
	return PSCI_E_SUCCESS;
}

static void zynqmp_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	req_state->pwr_domain_state[PSCI_CPU_PWR_LVL] = PLAT_MAX_OFF_STATE;
	req_state->pwr_domain_state[1] = PLAT_MAX_OFF_STATE;
}

static const struct plat_psci_ops versal_net_nopmc_psci_ops = {
	.cpu_standby			= zynqmp_cpu_standby,
	.pwr_domain_on			= zynqmp_nopmu_pwr_domain_on,
	.pwr_domain_off			= zynqmp_nopmu_pwr_domain_off,
	.system_reset			= zynqmp_nopmu_system_reset,
	.validate_ns_entrypoint		= zynqmp_validate_ns_entrypoint,
	.pwr_domain_suspend		= zynqmp_pwr_domain_suspend,
	.pwr_domain_on_finish		= zynqmp_pwr_domain_on_finish,
	.pwr_domain_suspend_finish	= zynqmp_pwr_domain_suspend_finish,
	.system_off			= zynqmp_system_off,
	.validate_power_state		= zynqmp_validate_power_state,
	.get_sys_suspend_power_state	= zynqmp_get_sys_suspend_power_state,
};

/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int32_t plat_setup_psci_ops(uintptr_t sec_entrypoint,
			    const struct plat_psci_ops **psci_ops)
{
	versal_net_sec_entry = sec_entrypoint;

	VERBOSE("Setting up entry point %lx\n", versal_net_sec_entry);

	*psci_ops = &versal_net_nopmc_psci_ops;

	return 0;
}

int sip_svc_setup_init(void)
{
	return 0;
}

static int32_t no_pm_ioctl(uint32_t device_id, uint32_t ioctl_id,
			   uint32_t arg1, uint32_t arg2)
{
	VERBOSE("%s: ioctl_id: %x, arg1: %x\n", __func__, ioctl_id, arg1);
	if (ioctl_id == IOCTL_OSPI_MUX_SELECT) {
		mmio_write_32(SLCR_OSPI_QSPI_IOU_AXI_MUX_SEL, arg1);
		return 0;
	}
	return PM_RET_ERROR_NOFEATURE;
}

static uint64_t no_pm_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3,
			      uint64_t x4, void *cookie, void *handle, uint64_t flags)
{
	int32_t ret;
	uint32_t arg[4], api_id;

	arg[0] = (uint32_t)x1;
	arg[1] = (uint32_t)(x1 >> 32);
	arg[2] = (uint32_t)x2;
	arg[3] = (uint32_t)(x2 >> 32);

	api_id = smc_fid & FUNCID_NUM_MASK;
	VERBOSE("%s: smc_fid: %x, api_id=0x%x\n", __func__, smc_fid, api_id);

	switch (api_id) {
	case PM_IOCTL:
	{
		ret = no_pm_ioctl(arg[0], arg[1], arg[2], arg[3]);
		SMC_RET1(handle, (uint64_t)ret);
	}
	case PM_GET_CHIPID:
	{
		uint32_t idcode, version;

		idcode  = mmio_read_32(PMC_TAP);
		version = mmio_read_32(PMC_TAP_VERSION);
		SMC_RET2(handle, ((uint64_t)idcode << 32), version);
	}
	default:
		WARN("Unimplemented PM Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

uint64_t smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4,
		     void *cookie, void *handle, uint64_t flags)
{
	return no_pm_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags);
}

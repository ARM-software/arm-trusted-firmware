/*
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2021-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2024, Advanced Micro Devices, Inc. All rights reserved.
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
#include <pm_defs.h>

#define PM_RET_ERROR_NOFEATURE U(19)
#define ALWAYSTRUE true
#define LINEAR_MODE BIT(1)

static uintptr_t _sec_entry;

static void zynqmp_cpu_standby(plat_local_state_t cpu_state)
{
	dsb();
	wfi();
}

#define MPIDR_MT_BIT	(24)

static int32_t zynqmp_nopmu_pwr_domain_on(u_register_t mpidr)
{
	uint32_t cpu_id = plat_core_pos_by_mpidr(mpidr) & ~BIT(MPIDR_MT_BIT);
	uint32_t cpu = cpu_id % PLATFORM_CORE_COUNT_PER_CLUSTER;
	uint32_t cluster = cpu_id / PLATFORM_CORE_COUNT_PER_CLUSTER;
	uintptr_t apu_cluster_base = 0, apu_pcli_base, apu_pcli_cluster = 0;
	uintptr_t rst_apu_cluster = PSX_CRF + RST_APU0_OFFSET + ((uint64_t)cluster * 0x4U);

	VERBOSE("%s: mpidr: 0x%lx, cpuid: %x, cpu: %x, cluster: %x\n",
		__func__, mpidr, cpu_id, cpu, cluster);

	if (cpu_id == -1) {
		return PSCI_E_INTERN_FAIL;
	}

	if (cluster > 3) {
		panic();
	}

	apu_pcli_cluster = APU_PCLI + APU_PCLI_CLUSTER_OFFSET + ((uint64_t)cluster * APU_PCLI_CLUSTER_STEP);
	apu_cluster_base = APU_CLUSTER0 + ((uint64_t)cluster * APU_CLUSTER_STEP);

	/* Enable clock */
	mmio_setbits_32(PSX_CRF + ACPU0_CLK_CTRL + ((uint64_t)cluster * 0x4U), ACPU_CLK_CTRL_CLKACT);

	/* Enable cluster states */
	mmio_setbits_32(apu_pcli_cluster + PCLI_PSTATE_OFFSET, PCLI_PSTATE_VAL_SET);
	mmio_setbits_32(apu_pcli_cluster + PCLI_PREQ_OFFSET, PREQ_CHANGE_REQUEST);

	/* assert core reset */
	mmio_setbits_32(rst_apu_cluster, ((RST_APU_COLD_RESET|RST_APU_WARN_RESET) << cpu));

	/* program RVBAR */
	mmio_write_32(apu_cluster_base + APU_RVBAR_L_0 + (cpu << 3),
		      (uint32_t)_sec_entry);
	mmio_write_32(apu_cluster_base + APU_RVBAR_H_0 + (cpu << 3),
		      _sec_entry >> 32);

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
	plat_gic_cpuif_disable();
}

static void __dead2 zynqmp_nopmu_system_reset(void)
{
	while (ALWAYSTRUE) {
		wfi();
	}
}

static int32_t zynqmp_validate_ns_entrypoint(uint64_t ns_entrypoint)
{
	VERBOSE("Validate ns_entry point %lx\n", ns_entrypoint);

	if ((ns_entrypoint) != 0U) {
		return PSCI_E_SUCCESS;
	} else {
		return PSCI_E_INVALID_ADDRESS;
	}
}

static void zynqmp_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	plat_gic_pcpu_init();
	plat_gic_cpuif_enable();
}

static void __dead2 zynqmp_system_off(void)
{
	while (ALWAYSTRUE) {
		wfi();
	}
}

static int32_t zynqmp_validate_power_state(uint32_t power_state, psci_power_state_t *req_state)
{
	return PSCI_E_SUCCESS;
}

static const struct plat_psci_ops _nopmc_psci_ops = {
	.cpu_standby			= zynqmp_cpu_standby,
	.pwr_domain_on			= zynqmp_nopmu_pwr_domain_on,
	.pwr_domain_off			= zynqmp_nopmu_pwr_domain_off,
	.system_reset			= zynqmp_nopmu_system_reset,
	.validate_ns_entrypoint		= zynqmp_validate_ns_entrypoint,
	.pwr_domain_on_finish		= zynqmp_pwr_domain_on_finish,
	.system_off			= zynqmp_system_off,
	.validate_power_state		= zynqmp_validate_power_state,
};

/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int32_t plat_setup_psci_ops(uintptr_t sec_entrypoint,
			    const struct plat_psci_ops **psci_ops)
{
	_sec_entry = sec_entrypoint;

	VERBOSE("Setting up entry point %lx\n", _sec_entry);

	*psci_ops = &_nopmc_psci_ops;

	return 0;
}

int sip_svc_setup_init(void)
{
	return 0;
}

static int32_t no_pm_ioctl(uint32_t device_id, uint32_t ioctl_id,
			   uint32_t arg1, uint32_t arg2)
{
	int32_t ret = 0;
	VERBOSE("%s: ioctl_id: %x, arg1: %x\n", __func__, ioctl_id, arg1);

	switch (ioctl_id) {
	case IOCTL_OSPI_MUX_SELECT:
		if ((arg1 == 0) || (arg1 == 1)) {
			mmio_clrsetbits_32(SLCR_OSPI_QSPI_IOU_AXI_MUX_SEL, LINEAR_MODE,
					(arg1 ? LINEAR_MODE : 0));
		} else {
			ret = PM_RET_ERROR_ARGS;
		}
		break;
	case IOCTL_UFS_TXRX_CFGRDY_GET:
		ret = (int32_t) mmio_read_32(PMXC_IOU_SLCR_TX_RX_CONFIG_RDY);
		break;
	case IOCTL_UFS_SRAM_CSR_SEL:
		if (arg1 == 1) {
			ret = (int32_t) mmio_read_32(PMXC_IOU_SLCR_SRAM_CSR);
		} else if (arg1 == 0) {
			mmio_write_32(PMXC_IOU_SLCR_SRAM_CSR, arg2);
		}
		break;
	case IOCTL_USB_SET_STATE:
		break;
	default:
		ret = PM_RET_ERROR_NOFEATURE;
		break;
	}

	return ret;
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
		/* Firmware driver expects return code in upper 32 bits and
		 * status in lower 32 bits.
		 * status is always SUCCESS(0) for mmio low level register
		 * r/w calls and return value is the value returned from
		 * no_pm_ioctl
		 */
		SMC_RET1(handle, ((uint64_t)ret << 32));
	}
	case PM_GET_CHIPID:
	{
		uint32_t idcode, version_type;

		idcode  = mmio_read_32(PMC_TAP);
		version_type = mmio_read_32(PMC_TAP_VERSION);
		SMC_RET2(handle, ((uint64_t)idcode << 32), version_type);
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

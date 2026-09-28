/*
 * Copyright (c) 2019-2026, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <drivers/arm/gic.h>
#include <drivers/arm/gic_common.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include "agilex72_cache.h"
#include "agilex72_clock_manager.h"
#include "agilex72_power_manager.h"
#include "ccu/ncore_ccu.h"
#include "mailbox/socfpga_mailbox_core.h"
#include "socfpga_dt.h"
#include "socfpga_private.h"
#include "socfpga_reset_manager.h"

/* Get non-secure SPSR for BL33. Zephyr and Linux */
uint32_t arm_get_spsr_for_bl33_entry(void);

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

/* The GICv3 driver only needs to be initialized in EL3 */

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	next_image_info = (type == NON_SECURE) ?
			  &bl33_image_ep_info : &bl32_image_ep_info;

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	static console_t console;

	mmio_write_64(PLAT_SEC_ENTRY, PLAT_SEC_WARM_ENTRY);

	console_16550_register(PLAT_INTEL_UART_BASE, PLAT_UART_CLOCK,
			       PLAT_BAUDRATE, &console);

	/* Enable TF-A BL31 logs when running from non-secure world also. */
	console_set_scope(&console, (CONSOLE_FLAG_BOOT |
				     CONSOLE_FLAG_RUNTIME |
				     CONSOLE_FLAG_CRASH));

#if RESET_TO_BL31
	/* There are no parameters from BL2 if BL31 is a reset vector */
	assert(arg0 == 0U);
	assert(arg3 == 0U);

	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info, PARAM_EP, VERSION_1, 0);

# if ARM_LINUX_KERNEL_AS_BL33
	/*
	 * According to the file ``Documentation/arm64/booting.txt`` of the
	 * Linux kernel tree, Linux expects the physical address of the device
	 * tree blob (DTB) in x0, while x1-x3 are reserved for future use and
	 * must be 0.
	 */
	bl33_image_ep_info.args.arg0 = (u_register_t)ARM_PRELOADED_DTB_BASE;
	bl33_image_ep_info.args.arg1 = 0U;
	bl33_image_ep_info.args.arg2 = 0U;
	bl33_image_ep_info.args.arg3 = 0U;
# endif

#else /* RESET_TO_BL31 */
	void *from_bl2 = (void *)arg0;
	bl_params_t *params_from_bl2 = (bl_params_t *)from_bl2;

	assert(params_from_bl2 != NULL);

	/*
	 * Copy BL32 (if populated by BL31) and BL33 entry point information.
	 * They are stored in Secure RAM, in BL31's address space.
	 */

	if (params_from_bl2->h.type == PARAM_BL_PARAMS &&
	    params_from_bl2->h.version >= VERSION_2) {

		bl_params_node_t *bl_params = params_from_bl2->head;

		while (bl_params) {
			if (bl_params->image_id == BL33_IMAGE_ID) {
				bl33_image_ep_info = *bl_params->ep_info;
			}
				bl_params = bl_params->next_params_info;
		}
	} else {
		struct socfpga_bl31_params *arg_from_bl2 =
			(struct socfpga_bl31_params *)from_bl2;

		assert(arg_from_bl2->h.type == PARAM_BL31);
		assert(arg_from_bl2->h.version >= VERSION_1);

		bl32_image_ep_info = *arg_from_bl2->bl32_ep_info;
		bl33_image_ep_info = *arg_from_bl2->bl33_ep_info;
	}

	bl33_image_ep_info.args.arg0 = (u_register_t)ARM_PRELOADED_DTB_BASE;
	bl33_image_ep_info.args.arg1 = 0U;
	bl33_image_ep_info.args.arg2 = 0U;
	bl33_image_ep_info.args.arg3 = 0U;
#endif

	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = arm_get_spsr_for_bl33_entry();

	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

}

static const interrupt_prop_t AGX72_interrupt_props[] = {
	PLAT_INTEL_SOCFPGA_G1S_IRQ_PROPS(INTR_GROUP1S),
	PLAT_INTEL_SOCFPGA_G0_IRQ_PROPS(INTR_GROUP0)
};

static const uintptr_t agx72_gicr_base_addrs[2] = {
	PLAT_INTEL_SOCFPGA_GICR_BASE,
	0U
};


/*******************************************************************************
 * Perform any BL3-1 platform setup code
 ******************************************************************************/
void bl31_platform_setup(void)
{
	/*
	 * GIC init is performed by bl31_main via USE_GIC_DRIVER=3
	 * (gic_init / gic_pcpu_init / gic_cpuif_enable). Frame/base/IRQ
	 * props are configured in bl31_plat_arch_setup().
	 */

#if SIP_SVC_V3
	/*
	 * Re-initialize the mailbox to include V3 specific routines.
	 * In V3, this re-initialize is required because prior to BL31, U-Boot
	 * SPL has its own mailbox settings and this initialization will
	 * override to those settings as required by the V3 framework.
	 */
	/* Initialize MCSR backend */
	int status;

	status = mailbox_core_init(MAILBOX_BACKEND_MCSR);
	if (status != 0) {
		ERROR("MCSR mailbox init failed: %d\n", status);
	}
#endif

	mailbox_hps_stage_notify(HPS_EXECUTION_STATE_SSBL);
}

const mmap_region_t plat_agilex_mmap[] = {
	MAP_REGION_FLAT(DRAM_BASE, DRAM_SIZE, MT_MEMORY | MT_RW | MT_NS),
	MAP_REGION_FLAT(PSS_BASE, PSS_SIZE, MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(MPFE_BASE, MPFE_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(OCRAM_BASE, OCRAM_SIZE, MT_NON_CACHEABLE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(CCU_BASE, CCU_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MEM64_BASE, MEM64_SIZE, MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(GIC_BASE, GIC_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	{0}
};

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only initializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	uint32_t boot_core = 0x00;
	uint32_t cpuid = 0x00;

	cpuid = MPIDR_AFFLVL1_VAL(read_mpidr());
	boot_core = (mmio_read_32(AGX72_SECMGR_MPU_BOOTCONFIG) &
		     AGX72_SECMGR_BOOTCONFIG_CORE_MASK) >>
		     AGX72_SECMGR_BOOTCONFIG_CORE_SHIFT;
	NOTICE("SOCFPGA: Boot Core = %x\n", boot_core);
	NOTICE("SOCFPGA: CPU ID = %x\n", cpuid);
	INFO("SOCFPGA: Invalidate Data cache\n");

	invalidate_dcache_all();

	/* Invalidate for NS EL2 and EL1 */
	invalidate_cache_low_el();

	NOTICE("SOCFPGA: Setting CLUSTERECTRL_EL1\n");
	setup_clusterectlr_el1();

	/* Configure generic GIC driver (USE_GIC_DRIVER=3); bl31_main calls gic_init(). */
	gic_set_gicd_base(PLAT_INTEL_SOCFPGA_GICD_BASE);
	gic_set_gicr_frames(agx72_gicr_base_addrs);
	gic_set_interrupt_props(AGX72_interrupt_props,
				ARRAY_SIZE(AGX72_interrupt_props));
}

void plat_gic_pre_pcpu_init(unsigned int cpu_idx)
{
	(void)cpu_idx;
}

/* Get non-secure image entrypoint for BL33. Zephyr and Linux */
uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	return PLAT_NS_IMAGE_OFFSET;
#endif
}

/* Get non-secure SPSR for BL33. Zephyr and Linux */
uint32_t arm_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	mode = (el_implemented(2) != EL_IMPL_NONE) ? MODE_EL2 : MODE_EL1;

	spsr = SPSR_64((uint64_t)mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

/* CPU2/3 are Cortex-A720; enable cluster clocks before PSCI release */
void bl31_enable_a720_cluster_clocks(void)
{
	static bool a720_clocks_done;

	/* PSCI serialises CPU_ON; one-time cluster clock enable is enough */
	if (a720_clocks_done) {
		return;
	}

	/*
	 * enable the A720 cluster gates in mainpllgrp.en (0x30) and release
	 * the externally-timed counter reset for core2/core3 in extcntrst (0xA8).
	 */
	mmio_setbits_32(CLKMGR(MAINPLL_EN), CLKMGR_MAINPLL_EN_SMP_A720);
	mmio_clrbits_32(CLKMGR(EXTCNTRST),
			CLKMGR_EXTCNTRST_CORE2 |
			CLKMGR_EXTCNTRST_CORE3);
	a720_clocks_done = true;
}

static void bl31_write_cpu_reset_entry(uintptr_t resetbase_low, uint64_t entry)
{
	mmio_write_32(resetbase_low, (uint32_t)entry);
	mmio_write_32(resetbase_low + 4U, (uint32_t)(entry >> 32));
}

/* SMP: Secondary cores BL31 setup reset vector */
void bl31_plat_set_secondary_cpu_entrypoint(unsigned int cpu_id)
{
	unsigned int pch_cpu = 0x00;
	uint64_t entry = ((uint64_t)(uintptr_t)plat_secondary_cpus_bl31_entry) >>
			 CPU_RESETBASELOW_CPU_SHIFT;

	/* Set bit for SMP secondary cores boot */
	mmio_clrsetbits_32(L2_RESET_DONE_REG, BS_REG_MAGIC_KEYS_MASK,
			   SMP_SEC_CORE_BOOT_REQ);

	pch_cpu = 1U << cpu_id;

	/*
	 * Program the BL31 secondary entrypoint for ALL four cores
	 * unconditionally. Any of cpu0..cpu3 may be the primary (selected via
	 * SECMGR_MPU_BOOTCONFIG bits[11:10]); writing the boot core's own
	 * RESETBASE is harmless because RESETBASE only takes effect on the
	 * next reset of that core. The earlier boot_core==0 special case
	 * dropped CPU2's RESETBASE for boot_core in {1,3}, leaving CPU2
	 * stranded when it was later released by PSCI.
	 */
	bl31_write_cpu_reset_entry(RSTMGR_CPUxRESETBASELOW_CPU0, entry);
	bl31_write_cpu_reset_entry(RSTMGR_CPUxRESETBASELOW_CPU1, entry);
	bl31_write_cpu_reset_entry(RSTMGR_CPUxRESETBASELOW_CPU2, entry);
	bl31_write_cpu_reset_entry(RSTMGR_CPUxRESETBASELOW_CPU3, entry);

	/* PCH trigger bits live at [4:1]; shift CPU mask by one */
	mmio_setbits_32(AGX72_PWRMGR(MPU_PCHCTLR), pch_cpu << 1);

	/* Agilex3: write release mask (same addr as CPURSTRELEASE) */
	mmio_write_32(RSTMGR_CPUSTRELEASE_CPUx, pch_cpu);
}

void bl31_plat_reset_secondary_cpu(unsigned int cpu_id)
{
	uint32_t mask = 0x1;
	uint32_t value = 0;
	uint32_t pwrctlr_addr = 0;
	uint32_t pwrstat_addr = 0;
	uint32_t ret = 0;

	mask <<= cpu_id;

	switch (cpu_id) {
	case 0:
		pwrctlr_addr = AGX72_PWRMGR(CPU_PWRCTLR0);
		pwrstat_addr = AGX72_PWRMGR(CPU_PWRSTAT0);
		break;
	case 1:
		pwrctlr_addr = AGX72_PWRMGR(CPU_PWRCTLR1);
		pwrstat_addr = AGX72_PWRMGR(CPU_PWRSTAT1);
		break;
	case 2:
		pwrctlr_addr = AGX72_PWRMGR(CPU_PWRCTLR2);
		pwrstat_addr = AGX72_PWRMGR(CPU_PWRSTAT2);
		break;
	case 3:
		pwrctlr_addr = AGX72_PWRMGR(CPU_PWRCTLR3);
		pwrstat_addr = AGX72_PWRMGR(CPU_PWRSTAT3);
		break;
	default:
		ERROR("BL31: %s: Invalid CPU ID\n", __func__);
		break;
	}

	/* PSTATE = 0, RUN_PCH = 1 */
	mmio_write_32(pwrctlr_addr, AGX72_PWRMGR_CPU_RUN_PCH(1));

	/* Poll for CPU OFF */
	SOCFPGA_POLL(!((AGX72_PWRMGR_CPU_RUN_PCH(mmio_read_32(pwrstat_addr)) == 0) ||
		       (AGX72_PWRMGR_CPU_SINGLE_FSM_STATE(mmio_read_32(pwrstat_addr)) != 0)),
		     AGX72_PWRMGR_CPU_POLL_COUNT,
		     AGX72_PWRMGR_CPU_DELAY_10_US, udelay, ret);

	if (ret) {
		ERROR("BL31: %s: Timeout when polling for CPU OFF\n", __func__);
	}

	/* Performs the warm reset CPUx */
	value = mmio_read_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_WARM_9));
	mmio_write_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_WARM_9), value | mask);
	udelay(1);
	mmio_write_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_WARM_9), value & ~mask);
	udelay(1);

	/* Power up sequence */
	mmio_write_32(pwrctlr_addr, AGX72_PWRMGR_CPU_PROG_CPU_ON_STATE |
		      AGX72_PWRMGR_CPU_RUN_PCH(1));

	/* Poll for CPU ON */
	SOCFPGA_POLL(!((AGX72_PWRMGR_CPU_RUN_PCH(mmio_read_32(pwrstat_addr)) == 0) ||
		       (AGX72_PWRMGR_CPU_SINGLE_FSM_STATE(mmio_read_32(pwrstat_addr)) == 0)),
		     AGX72_PWRMGR_CPU_POLL_COUNT,
		     AGX72_PWRMGR_CPU_DELAY_10_US, udelay, ret);

	if (ret)
		ERROR("BL31: %s: Timeout when polling for CPU ON\n", __func__);
}

void bl31_plat_set_secondary_cpu_off(void)
{
	unsigned int pch_cpu = 0x00;
	unsigned int pch_cpu_off = 0x00;
	unsigned int cpu_id = plat_my_core_pos();

	pch_cpu_off = 1 << cpu_id;

	pch_cpu = mmio_read_32(AGX72_PWRMGR(MPU_PCHCTLR));
	pch_cpu = pch_cpu & ~(pch_cpu_off << 1);

	mmio_write_32(AGX72_PWRMGR(MPU_PCHCTLR), pch_cpu);
}

void setup_clusterectlr_el1(void)
{
	uint64_t value = 0;

	/*
	 * CLUSTERECTLR_EL1 is not programmed by USE_DSU_DRIVER /
	 * dsu_driver_init() (those cover CLUSTERPWR* / PMU). Keep this
	 * platform-specific CLUSTERECTLR tweak for Simics/silicon bring-up.
	 */
	asm volatile("mrs %0, S3_0_C15_C3_4" : "=r"(value));

	/* Disable broadcasting atomics */
	value |= BIT_64(7);
	/* Disable sending data with clean evicts */
	value &= ~BIT_64(14);

	/* Write CLUSTERECTLR_EL1 */
	asm volatile("msr S3_0_C15_C3_4, %0" :: "r"(value));
}

void bl31_plat_runtime_setup(void)
{
}

void bl31_plat_enable_mmu(uint32_t flags)
{
	(void)flags;
}

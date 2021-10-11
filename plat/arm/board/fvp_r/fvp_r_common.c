/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* This uses xlat_mpu, but tables are set up using V2 mmap_region_t */
#define XLAT_TABLES_LIB_V2	1

#include <assert.h>
#include <common/debug.h>

#include <drivers/arm/cci.h>
#include <drivers/arm/gicv2.h>
#include <drivers/arm/sp804_delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <lib/smccc.h>
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <services/arm_arch_svc.h>

#include "fvp_r_private.h"
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>


/* Defines for GIC Driver build time selection */
#define FVP_R_GICV3		2

/*******************************************************************************
 * arm_config holds the characteristics of the differences between the FVP_R
 * platforms. It will be populated during cold boot at each boot stage by the
 * primary before enabling the MPU (to allow interconnect configuration) &
 * used thereafter. Each BL will have its own copy to allow independent
 * operation.
 ******************************************************************************/
arm_config_t arm_config;

#define MAP_DEVICE0	MAP_REGION_FLAT(DEVICE0_BASE,			\
					DEVICE0_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_DEVICE1	MAP_REGION_FLAT(DEVICE1_BASE,			\
					DEVICE1_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

/*
 * Need to be mapped with write permissions in order to set a new non-volatile
 * counter value.
 */
#define MAP_DEVICE2	MAP_REGION_FLAT(DEVICE2_BASE,			\
					DEVICE2_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

/*
 * Table of memory regions for various BL stages to map using the MPU.
 * This doesn't include Trusted SRAM as setup_page_tables() already takes care
 * of mapping it.
 *
 * The flash needs to be mapped as writable in order to erase the FIP's Table of
 * Contents in case of unrecoverable error (see plat_error_handler()).
 */
#ifdef IMAGE_BL1
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_FLASH0_RW,
	V2M_MAP_IOFPGA,
	MAP_DEVICE0,
	MAP_DEVICE1,
#if TRUSTED_BOARD_BOOT
	/* To access the Root of Trust Public Key registers. */
	MAP_DEVICE2,
#endif
	{0}
};
#endif

ARM_CASSERT_MMAP

static const int fvp_cci400_map[] = {
	PLAT_FVP_R_CCI400_CLUS0_SL_PORT,
	PLAT_FVP_R_CCI400_CLUS1_SL_PORT,
};

static const int fvp_cci5xx_map[] = {
	PLAT_FVP_R_CCI5XX_CLUS0_SL_PORT,
	PLAT_FVP_R_CCI5XX_CLUS1_SL_PORT,
};

static unsigned int get_interconnect_master(void)
{
	unsigned int master;
	u_register_t mpidr;

	mpidr = read_mpidr_el1();
	master = ((arm_config.flags & ARM_CONFIG_FVP_SHIFTED_AFF) != 0U) ?
		MPIDR_AFFLVL2_VAL(mpidr) : MPIDR_AFFLVL1_VAL(mpidr);

	assert(master < FVP_R_CLUSTER_COUNT);
	return master;
}

/*******************************************************************************
 * Initialize the platform config for future decision making
 ******************************************************************************/
void __init fvp_config_setup(void)
{
	unsigned int rev, hbi, bld, arch, sys_id;

	arm_config.flags |= ARM_CONFIG_BASE_MMAP;
	sys_id = mmio_read_32(V2M_FVP_R_SYSREGS_BASE + V2M_SYS_ID);
	rev = (sys_id >> V2M_SYS_ID_REV_SHIFT) & V2M_SYS_ID_REV_MASK;
	hbi = (sys_id >> V2M_SYS_ID_HBI_SHIFT) & V2M_SYS_ID_HBI_MASK;
	bld = (sys_id >> V2M_SYS_ID_BLD_SHIFT) & V2M_SYS_ID_BLD_MASK;
	arch = (sys_id >> V2M_SYS_ID_ARCH_SHIFT) & V2M_SYS_ID_ARCH_MASK;

	if (arch != ARCH_MODEL) {
		ERROR("This firmware is for FVP_R models\n");
		panic();
	}

	/*
	 * The build field in the SYS_ID tells which variant of the GIC
	 * memory is implemented by the model.
	 */
	switch (bld) {
	case BLD_GIC_VE_MMAP:
		ERROR("Legacy Versatile Express memory map for GIC %s",
				"peripheral is not supported\n");
		panic();
		break;
	case BLD_GIC_A53A57_MMAP:
		break;
	default:
		ERROR("Unsupported board build %x\n", bld);
		panic();
	}

	/*
	 * The hbi field in the SYS_ID is 0x020 for the Base FVP_R & 0x010
	 * for the Foundation FVP_R.
	 */
	switch (hbi) {
	case HBI_FOUNDATION_FVP_R:
		arm_config.flags = 0;

		/*
		 * Check for supported revisions of Foundation FVP_R
		 * Allow future revisions to run but emit warning diagnostic
		 */
		switch (rev) {
		case REV_FOUNDATION_FVP_R_V2_0:
		case REV_FOUNDATION_FVP_R_V2_1:
		case REV_FOUNDATION_FVP_R_v9_1:
		case REV_FOUNDATION_FVP_R_v9_6:
			break;
		default:
			WARN("Unrecognized Foundation FVP_R revision %x\n", rev);
			break;
		}
		break;
	case HBI_BASE_FVP_R:
		arm_config.flags |= (ARM_CONFIG_BASE_MMAP | ARM_CONFIG_HAS_TZC);

		/*
		 * Check for supported revisions
		 * Allow future revisions to run but emit warning diagnostic
		 */
		switch (rev) {
		case REV_BASE_FVP_R_V0:
			arm_config.flags |= ARM_CONFIG_FVP_HAS_CCI400;
			break;
		default:
			WARN("Unrecognized Base FVP_R revision %x\n", rev);
			break;
		}
		break;
	default:
		ERROR("Unsupported board HBI number 0x%x\n", hbi);
		panic();
	}

	/*
	 * We assume that the presence of MT bit, and therefore shifted
	 * affinities, is uniform across the platform: either all CPUs, or no
	 * CPUs implement it.
	 */
	if ((read_mpidr_el1() & MPIDR_MT_MASK) != 0U) {
		arm_config.flags |= ARM_CONFIG_FVP_SHIFTED_AFF;
	}
}


void __init fvp_interconnect_init(void)
{
	uintptr_t cci_base = 0U;
	const int *cci_map = NULL;
	unsigned int map_size = 0U;

	/* Initialize the right interconnect */
	if ((arm_config.flags & ARM_CONFIG_FVP_HAS_CCI5XX) != 0U) {
		cci_base = PLAT_FVP_R_CCI5XX_BASE;
		cci_map = fvp_cci5xx_map;
		map_size = ARRAY_SIZE(fvp_cci5xx_map);
	} else if ((arm_config.flags & ARM_CONFIG_FVP_HAS_CCI400) != 0U) {
		cci_base = PLAT_FVP_R_CCI400_BASE;
		cci_map = fvp_cci400_map;
		map_size = ARRAY_SIZE(fvp_cci400_map);
	} else {
		return;
	}

	assert(cci_base != 0U);
	assert(cci_map != NULL);
	cci_init(cci_base, cci_map, map_size);
}

void fvp_interconnect_enable(void)
{
	unsigned int master;

	if ((arm_config.flags & (ARM_CONFIG_FVP_HAS_CCI400 |
				 ARM_CONFIG_FVP_HAS_CCI5XX)) != 0U) {
		master = get_interconnect_master();
		cci_enable_snoop_dvm_reqs(master);
	}
}

void fvp_interconnect_disable(void)
{
	unsigned int master;

	if ((arm_config.flags & (ARM_CONFIG_FVP_HAS_CCI400 |
				 ARM_CONFIG_FVP_HAS_CCI5XX)) != 0U) {
		master = get_interconnect_master();
		cci_disable_snoop_dvm_reqs(master);
	}
}

#if TRUSTED_BOARD_BOOT
int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	assert(heap_addr != NULL);
	assert(heap_size != NULL);

	return arm_get_mbedtls_heap(heap_addr, heap_size);
}
#endif

void fvp_timer_init(void)
{
#if USE_SP804_TIMER
	/* Enable the clock override for SP804 timer 0, which means that no
	 * clock dividers are applied and the raw (35MHz) clock will be used.
	 */
	mmio_write_32(V2M_SP810_BASE, FVP_R_SP810_CTRL_TIM0_OV);

	/* Initialize delay timer driver using SP804 dual timer 0 */
	sp804_timer_init(V2M_SP804_TIMER0_BASE,
			SP804_TIMER_CLKMULT, SP804_TIMER_CLKDIV);
#else
	generic_delay_timer_init();

	/* Enable System level generic timer */
	mmio_write_32(ARM_SYS_CNTCTL_BASE + CNTCR_OFF,
			CNTCR_FCREQ(0U) | CNTCR_EN);
#endif /* USE_SP804_TIMER */
}

/* Get SOC version */
int32_t plat_get_soc_version(void)
{
	return (int32_t)
		((ARM_SOC_IDENTIFICATION_CODE << ARM_SOC_IDENTIFICATION_SHIFT)
		 | (ARM_SOC_CONTINUATION_CODE << ARM_SOC_CONTINUATION_SHIFT)
		 | FVP_R_SOC_ID);
}

/* Get SOC revision */
int32_t plat_get_soc_revision(void)
{
	unsigned int sys_id;

	sys_id = mmio_read_32(V2M_SYSREGS_BASE + V2M_SYS_ID);
	return (int32_t)((sys_id >> V2M_SYS_ID_REV_SHIFT) &
			V2M_SYS_ID_REV_MASK);
}

/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/arm/cci.h>
#include <drivers/arm/ccn.h>
#include <drivers/arm/gicv2.h>
#include <drivers/arm/sp804_delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <services/spm_mm_partition.h>

#include "fvp_private.h"

/* Defines for GIC Driver build time selection */
#define FVP_GICV2		1
#define FVP_GICV3		2

/*******************************************************************************
 * arm_config holds the characteristics of the differences between the three FVP
 * platforms (Base, A53_A57 & Foundation). It will be populated during cold boot
 * at each boot stage by the primary before enabling the MMU (to allow
 * interconnect configuration) & used thereafter. Each BL will have its own copy
 * to allow independent operation.
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
 * Table of memory regions for various BL stages to map using the MMU.
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
	/* Map DRAM to authenticate NS_BL2U image. */
	ARM_MAP_NS_DRAM1,
#endif
	{0}
};
#endif
#ifdef IMAGE_BL2
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_FLASH0_RW,
	V2M_MAP_IOFPGA,
	MAP_DEVICE0,
	MAP_DEVICE1,
	ARM_MAP_NS_DRAM1,
#ifdef __aarch64__
	ARM_MAP_DRAM2,
#endif
#if defined(SPD_spmd)
	ARM_MAP_TRUSTED_DRAM,
#endif
#ifdef SPD_tspd
	ARM_MAP_TSP_SEC_MEM,
#endif
#if TRUSTED_BOARD_BOOT
	/* To access the Root of Trust Public Key registers. */
	MAP_DEVICE2,
#if !BL2_AT_EL3
	ARM_MAP_BL1_RW,
#endif
#endif /* TRUSTED_BOARD_BOOT */
#if SPM_MM
	ARM_SP_IMAGE_MMAP,
#endif
#if ARM_BL31_IN_DRAM
	ARM_MAP_BL31_SEC_DRAM,
#endif
#ifdef SPD_opteed
	ARM_MAP_OPTEE_CORE_MEM,
	ARM_OPTEE_PAGEABLE_LOAD_MEM,
#endif
	{0}
};
#endif
#ifdef IMAGE_BL2U
const mmap_region_t plat_arm_mmap[] = {
	MAP_DEVICE0,
	V2M_MAP_IOFPGA,
	{0}
};
#endif
#ifdef IMAGE_BL31
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
#if USE_DEBUGFS
	/* Required by devfip, can be removed if devfip is not used */
	V2M_MAP_FLASH0_RW,
#endif /* USE_DEBUGFS */
	ARM_MAP_EL3_TZC_DRAM,
	V2M_MAP_IOFPGA,
	MAP_DEVICE0,
	MAP_DEVICE1,
	ARM_V2M_MAP_MEM_PROTECT,
#if SPM_MM
	ARM_SPM_BUF_EL3_MMAP,
#endif
	{0}
};

#if defined(IMAGE_BL31) && SPM_MM
const mmap_region_t plat_arm_secure_partition_mmap[] = {
	V2M_MAP_IOFPGA_EL0, /* for the UART */
	MAP_REGION_FLAT(DEVICE0_BASE,				\
			DEVICE0_SIZE,				\
			MT_DEVICE | MT_RO | MT_SECURE | MT_USER),
	ARM_SP_IMAGE_MMAP,
	ARM_SP_IMAGE_NS_BUF_MMAP,
	ARM_SP_IMAGE_RW_MMAP,
	ARM_SPM_BUF_EL0_MMAP,
	{0}
};
#endif
#endif
#ifdef IMAGE_BL32
const mmap_region_t plat_arm_mmap[] = {
#ifndef __aarch64__
	ARM_MAP_SHARED_RAM,
	ARM_V2M_MAP_MEM_PROTECT,
#endif
	V2M_MAP_IOFPGA,
	MAP_DEVICE0,
	MAP_DEVICE1,
	{0}
};
#endif

ARM_CASSERT_MMAP

#if FVP_INTERCONNECT_DRIVER != FVP_CCN
static const int fvp_cci400_map[] = {
	PLAT_FVP_CCI400_CLUS0_SL_PORT,
	PLAT_FVP_CCI400_CLUS1_SL_PORT,
};

static const int fvp_cci5xx_map[] = {
	PLAT_FVP_CCI5XX_CLUS0_SL_PORT,
	PLAT_FVP_CCI5XX_CLUS1_SL_PORT,
};

static unsigned int get_interconnect_master(void)
{
	unsigned int master;
	u_register_t mpidr;

	mpidr = read_mpidr_el1();
	master = ((arm_config.flags & ARM_CONFIG_FVP_SHIFTED_AFF) != 0U) ?
		MPIDR_AFFLVL2_VAL(mpidr) : MPIDR_AFFLVL1_VAL(mpidr);

	assert(master < FVP_CLUSTER_COUNT);
	return master;
}
#endif

#if defined(IMAGE_BL31) && SPM_MM
/*
 * Boot information passed to a secure partition during initialisation. Linear
 * indices in MP information will be filled at runtime.
 */
static spm_mm_mp_info_t sp_mp_info[] = {
	[0] = {0x80000000, 0},
	[1] = {0x80000001, 0},
	[2] = {0x80000002, 0},
	[3] = {0x80000003, 0},
	[4] = {0x80000100, 0},
	[5] = {0x80000101, 0},
	[6] = {0x80000102, 0},
	[7] = {0x80000103, 0},
};

const spm_mm_boot_info_t plat_arm_secure_partition_boot_info = {
	.h.type              = PARAM_SP_IMAGE_BOOT_INFO,
	.h.version           = VERSION_1,
	.h.size              = sizeof(spm_mm_boot_info_t),
	.h.attr              = 0,
	.sp_mem_base         = ARM_SP_IMAGE_BASE,
	.sp_mem_limit        = ARM_SP_IMAGE_LIMIT,
	.sp_image_base       = ARM_SP_IMAGE_BASE,
	.sp_stack_base       = PLAT_SP_IMAGE_STACK_BASE,
	.sp_heap_base        = ARM_SP_IMAGE_HEAP_BASE,
	.sp_ns_comm_buf_base = PLAT_SP_IMAGE_NS_BUF_BASE,
	.sp_shared_buf_base  = PLAT_SPM_BUF_BASE,
	.sp_image_size       = ARM_SP_IMAGE_SIZE,
	.sp_pcpu_stack_size  = PLAT_SP_IMAGE_STACK_PCPU_SIZE,
	.sp_heap_size        = ARM_SP_IMAGE_HEAP_SIZE,
	.sp_ns_comm_buf_size = PLAT_SP_IMAGE_NS_BUF_SIZE,
	.sp_shared_buf_size  = PLAT_SPM_BUF_SIZE,
	.num_sp_mem_regions  = ARM_SP_IMAGE_NUM_MEM_REGIONS,
	.num_cpus            = PLATFORM_CORE_COUNT,
	.mp_info             = &sp_mp_info[0],
};

const struct mmap_region *plat_get_secure_partition_mmap(void *cookie)
{
	return plat_arm_secure_partition_mmap;
}

const struct spm_mm_boot_info *plat_get_secure_partition_boot_info(
		void *cookie)
{
	return &plat_arm_secure_partition_boot_info;
}
#endif

/*******************************************************************************
 * A single boot loader stack is expected to work on both the Foundation FVP
 * models and the two flavours of the Base FVP models (AEMv8 & Cortex). The
 * SYS_ID register provides a mechanism for detecting the differences between
 * these platforms. This information is stored in a per-BL array to allow the
 * code to take the correct path.Per BL platform configuration.
 ******************************************************************************/
void __init fvp_config_setup(void)
{
	unsigned int rev, hbi, bld, arch, sys_id;

	sys_id = mmio_read_32(V2M_SYSREGS_BASE + V2M_SYS_ID);
	rev = (sys_id >> V2M_SYS_ID_REV_SHIFT) & V2M_SYS_ID_REV_MASK;
	hbi = (sys_id >> V2M_SYS_ID_HBI_SHIFT) & V2M_SYS_ID_HBI_MASK;
	bld = (sys_id >> V2M_SYS_ID_BLD_SHIFT) & V2M_SYS_ID_BLD_MASK;
	arch = (sys_id >> V2M_SYS_ID_ARCH_SHIFT) & V2M_SYS_ID_ARCH_MASK;

	if (arch != ARCH_MODEL) {
		ERROR("This firmware is for FVP models\n");
		panic();
	}

	/*
	 * The build field in the SYS_ID tells which variant of the GIC
	 * memory is implemented by the model.
	 */
	switch (bld) {
	case BLD_GIC_VE_MMAP:
		ERROR("Legacy Versatile Express memory map for GIC peripheral"
				" is not supported\n");
		panic();
		break;
	case BLD_GIC_A53A57_MMAP:
		break;
	default:
		ERROR("Unsupported board build %x\n", bld);
		panic();
	}

	/*
	 * The hbi field in the SYS_ID is 0x020 for the Base FVP & 0x010
	 * for the Foundation FVP.
	 */
	switch (hbi) {
	case HBI_FOUNDATION_FVP:
		arm_config.flags = 0;

		/*
		 * Check for supported revisions of Foundation FVP
		 * Allow future revisions to run but emit warning diagnostic
		 */
		switch (rev) {
		case REV_FOUNDATION_FVP_V2_0:
		case REV_FOUNDATION_FVP_V2_1:
		case REV_FOUNDATION_FVP_v9_1:
		case REV_FOUNDATION_FVP_v9_6:
			break;
		default:
			WARN("Unrecognized Foundation FVP revision %x\n", rev);
			break;
		}
		break;
	case HBI_BASE_FVP:
		arm_config.flags |= (ARM_CONFIG_BASE_MMAP | ARM_CONFIG_HAS_TZC);

		/*
		 * Check for supported revisions
		 * Allow future revisions to run but emit warning diagnostic
		 */
		switch (rev) {
		case REV_BASE_FVP_V0:
			arm_config.flags |= ARM_CONFIG_FVP_HAS_CCI400;
			break;
		case REV_BASE_FVP_REVC:
			arm_config.flags |= (ARM_CONFIG_FVP_HAS_SMMUV3 |
					ARM_CONFIG_FVP_HAS_CCI5XX);
			break;
		default:
			WARN("Unrecognized Base FVP revision %x\n", rev);
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
	if ((read_mpidr_el1() & MPIDR_MT_MASK) != 0U)
		arm_config.flags |= ARM_CONFIG_FVP_SHIFTED_AFF;
}


void __init fvp_interconnect_init(void)
{
#if FVP_INTERCONNECT_DRIVER == FVP_CCN
	if (ccn_get_part0_id(PLAT_ARM_CCN_BASE) != CCN_502_PART0_ID) {
		ERROR("Unrecognized CCN variant detected. Only CCN-502 is supported");
		panic();
	}

	plat_arm_interconnect_init();
#else
	uintptr_t cci_base = 0U;
	const int *cci_map = NULL;
	unsigned int map_size = 0U;

	/* Initialize the right interconnect */
	if ((arm_config.flags & ARM_CONFIG_FVP_HAS_CCI5XX) != 0U) {
		cci_base = PLAT_FVP_CCI5XX_BASE;
		cci_map = fvp_cci5xx_map;
		map_size = ARRAY_SIZE(fvp_cci5xx_map);
	} else if ((arm_config.flags & ARM_CONFIG_FVP_HAS_CCI400) != 0U) {
		cci_base = PLAT_FVP_CCI400_BASE;
		cci_map = fvp_cci400_map;
		map_size = ARRAY_SIZE(fvp_cci400_map);
	} else {
		return;
	}

	assert(cci_base != 0U);
	assert(cci_map != NULL);
	cci_init(cci_base, cci_map, map_size);
#endif
}

void fvp_interconnect_enable(void)
{
#if FVP_INTERCONNECT_DRIVER == FVP_CCN
	plat_arm_interconnect_enter_coherency();
#else
	unsigned int master;

	if ((arm_config.flags & (ARM_CONFIG_FVP_HAS_CCI400 |
				 ARM_CONFIG_FVP_HAS_CCI5XX)) != 0U) {
		master = get_interconnect_master();
		cci_enable_snoop_dvm_reqs(master);
	}
#endif
}

void fvp_interconnect_disable(void)
{
#if FVP_INTERCONNECT_DRIVER == FVP_CCN
	plat_arm_interconnect_exit_coherency();
#else
	unsigned int master;

	if ((arm_config.flags & (ARM_CONFIG_FVP_HAS_CCI400 |
				 ARM_CONFIG_FVP_HAS_CCI5XX)) != 0U) {
		master = get_interconnect_master();
		cci_disable_snoop_dvm_reqs(master);
	}
#endif
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
#if FVP_USE_SP804_TIMER
	/* Enable the clock override for SP804 timer 0, which means that no
	 * clock dividers are applied and the raw (35MHz) clock will be used.
	 */
	mmio_write_32(V2M_SP810_BASE, FVP_SP810_CTRL_TIM0_OV);

	/* Initialize delay timer driver using SP804 dual timer 0 */
	sp804_timer_init(V2M_SP804_TIMER0_BASE,
			SP804_TIMER_CLKMULT, SP804_TIMER_CLKDIV);
#else
	generic_delay_timer_init();

	/* Enable System level generic timer */
	mmio_write_32(ARM_SYS_CNTCTL_BASE + CNTCR_OFF,
			CNTCR_FCREQ(0U) | CNTCR_EN);
#endif /* FVP_USE_SP804_TIMER */
}

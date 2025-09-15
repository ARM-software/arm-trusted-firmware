/*
 * Copyright (c) 2013-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/cci.h>
#include <drivers/arm/ccn.h>
#include <drivers/arm/gicv2.h>
#include <drivers/arm/sp804_delay_timer.h>
#include <drivers/arm/smmu_v3.h>
#include <drivers/generic_delay_timer.h>
#include <fconf_hw_config_getter.h>
#include <lib/mmio.h>
#include <lib/smccc.h>
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <platform_def.h>
#include <services/arm_arch_svc.h>
#include <services/rmm_core_manifest.h>
#if SPM_MM
#include <services/spm_mm_partition.h>
#endif

#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include "fvp_private.h"

/* Defines for GIC Driver build time selection */
#define FVP_GICV2		1
#define FVP_GICV3		2

/* Defines for RMM Console */
#define FVP_RMM_CONSOLE_BASE		UL(0x1c0c0000)
#define FVP_RMM_CONSOLE_BAUD		UL(115200)
#define FVP_RMM_CONSOLE_CLK_IN_HZ	UL(14745600)
#define FVP_RMM_CONSOLE_NAME		"pl011"
#define FVP_RMM_CONSOLE_COUNT		UL(1)

/* Defines for RMM PCIe ECAM */
#define FVP_RMM_ECAM_BASE		PCIE_EXP_BASE
#define FVP_RMM_ECAM_SEGMENT		UL(0x0)
#define FVP_RMM_ECAM_BDF		UL(0x0)

/* Defines for RMM SMMUv3 */
#define FVP_RMM_SMMU_BASE		PLAT_FVP_SMMUV3_BASE
#define FVP_RMM_SMMU_COUNT		UL(1)

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
					MT_DEVICE | MT_RW | EL3_PAS)

#define MAP_DEVICE1	MAP_REGION_FLAT(DEVICE1_BASE,			\
					DEVICE1_SIZE,			\
					MT_DEVICE | MT_RW | EL3_PAS)

#define MAP_CCN		MAP_REGION_FLAT(CCN_BASE,			\
					CCN_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#if FVP_GICR_REGION_PROTECTION
#define MAP_GICD_MEM	MAP_REGION_FLAT(BASE_GICD_BASE,			\
					BASE_GICD_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

/* Map all core's redistributor memory as read-only. After boots up,
 * per-core map its redistributor memory as read-write */
#define MAP_GICR_MEM	MAP_REGION_FLAT(BASE_GICR_BASE,			\
					(BASE_GICR_SIZE * PLATFORM_CORE_COUNT),\
					MT_DEVICE | MT_RO | MT_SECURE)
#endif /* FVP_GICR_REGION_PROTECTION */

/*
 * Need to be mapped with write permissions in order to set a new non-volatile
 * counter value.
 */
#define MAP_DEVICE2	MAP_REGION_FLAT(DEVICE2_BASE,			\
					DEVICE2_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#if TRANSFER_LIST
#ifdef FW_NS_HANDOFF_BASE
#define MAP_FW_NS_HANDOFF                                             \
	MAP_REGION_FLAT(FW_NS_HANDOFF_BASE, PLAT_ARM_FW_HANDOFF_SIZE, \
			MT_MEMORY | MT_RW | MT_NS)
#endif
#ifdef PLAT_ARM_EL3_FW_HANDOFF_BASE
#define MAP_EL3_FW_HANDOFF                            \
	MAP_REGION_FLAT(PLAT_ARM_EL3_FW_HANDOFF_BASE, \
			PLAT_ARM_FW_HANDOFF_SIZE, MT_MEMORY | MT_RW | EL3_PAS)
#endif
#endif

/*
 * Table of memory regions for various BL stages to map using the MMU.
 * This doesn't include Trusted SRAM as setup_page_tables() already takes care
 * of mapping it.
 */
#ifdef IMAGE_BL1
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_FLASH0_RO,
	V2M_MAP_IOFPGA,
	MAP_DEVICE0,
#if FVP_INTERCONNECT_DRIVER == FVP_CCN
	MAP_CCN,
#endif
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
#if FVP_INTERCONNECT_DRIVER == FVP_CCN
	MAP_CCN,
#endif
	ARM_MAP_NS_DRAM1,
#ifdef __aarch64__
	ARM_MAP_DRAM2,
#endif
	/*
	 * Required to load HW_CONFIG, SPMC and SPs to trusted DRAM.
	 */
	ARM_MAP_TRUSTED_DRAM,

	/*
	 * Required to load Event Log in TZC secured memory
	 */
#if MEASURED_BOOT && (defined(SPD_tspd) || defined(SPD_opteed) || \
defined(SPD_spmd))
	ARM_MAP_EVENT_LOG_DRAM1,
#endif /* MEASURED_BOOT && (SPD_tspd || SPD_opteed || SPD_spmd) */

#if ENABLE_RME
	ARM_MAP_RMM_DRAM,
	ARM_MAP_GPT_L1_DRAM,
#endif /* ENABLE_RME */
#ifdef SPD_tspd
	ARM_MAP_TSP_SEC_MEM,
#endif
#if TRUSTED_BOARD_BOOT
	/* To access the Root of Trust Public Key registers. */
	MAP_DEVICE2,
#endif /* TRUSTED_BOARD_BOOT */

#if CRYPTO_SUPPORT && !RESET_TO_BL2
	/*
	 * To access shared the Mbed TLS heap while booting the
	 * system with Crypto support
	 */
	ARM_MAP_BL1_RW,
#endif /* CRYPTO_SUPPORT && !RESET_TO_BL2 */
#if SPM_MM || SPMC_AT_EL3
	ARM_SP_IMAGE_MMAP,
#endif
#if ARM_BL31_IN_DRAM
	ARM_MAP_BL31_SEC_DRAM,
#endif
#ifdef SPD_opteed
	ARM_MAP_OPTEE_CORE_MEM,
	ARM_OPTEE_PAGEABLE_LOAD_MEM,
#endif
#ifdef MAP_EL3_FW_HANDOFF
	MAP_EL3_FW_HANDOFF,
#endif
	{ 0 }
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
#if FVP_GICR_REGION_PROTECTION
	MAP_GICD_MEM,
	MAP_GICR_MEM,
#else
#if FVP_INTERCONNECT_DRIVER == FVP_CCN
	MAP_CCN,
#endif
	MAP_DEVICE1,
#endif /* FVP_GICR_REGION_PROTECTION */
	ARM_V2M_MAP_MEM_PROTECT,
#if SPM_MM
	ARM_SPM_BUF_EL3_MMAP,
#endif
#if ENABLE_RME
	ARM_MAP_GPT_L1_DRAM,
	ARM_MAP_EL3_RMM_SHARED_MEM,
#endif
#ifdef MAP_FW_NS_HANDOFF
	MAP_FW_NS_HANDOFF,
#endif
#if defined(MAP_EL3_FW_HANDOFF) && !RESET_TO_BL31
	MAP_EL3_FW_HANDOFF,
#endif
	{ 0 }
};

#if defined(IMAGE_BL31) && SPM_MM
const mmap_region_t plat_arm_secure_partition_mmap[] = {
	V2M_MAP_IOFPGA_EL0, /* for the UART */
	V2M_MAP_SECURE_SYSTEMREG_EL0, /* for initializing flash */
#if PSA_FWU_SUPPORT
	V2M_MAP_FLASH0_RW_EL0, /* for firmware update service in standalone mm */
#endif
	V2M_MAP_FLASH1_RW_EL0, /* for secure variable service in standalone mm */
	MAP_REGION_FLAT(DEVICE0_BASE,
			DEVICE0_SIZE,
			MT_DEVICE | MT_RO | MT_SECURE | MT_USER),
	ARM_SP_IMAGE_MMAP,
	ARM_SP_IMAGE_NS_BUF_MMAP,
	ARM_SP_IMAGE_RW_MMAP,
	ARM_SPM_BUF_EL0_MMAP,
	ARM_SP_PSEUDO_NS_CRB_MMAP,
	ARM_SP_PSEUDO_S_CRB_MMAP,
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
#if FVP_INTERCONNECT_DRIVER == FVP_CCN
	MAP_CCN,
#endif
	MAP_DEVICE1,
	{0}
};
#endif

#ifdef IMAGE_RMM
const mmap_region_t plat_arm_mmap[] = {
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

#if CRYPTO_SUPPORT
int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	assert(heap_addr != NULL);
	assert(heap_size != NULL);

	return arm_get_mbedtls_heap(heap_addr, heap_size);
}
#endif /* CRYPTO_SUPPORT */

void fvp_timer_init(void)
{
#if USE_SP804_TIMER
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
#endif /* USE_SP804_TIMER */
}

/*****************************************************************************
 * plat_is_smccc_feature_available() - This function checks whether SMCCC
 *                                     feature is availabile for platform.
 * @fid: SMCCC function id
 *
 * Return SMC_ARCH_CALL_SUCCESS if SMCCC feature is available and
 * SMC_ARCH_CALL_NOT_SUPPORTED otherwise.
 *****************************************************************************/
int32_t plat_is_smccc_feature_available(u_register_t fid)
{
	switch (fid) {
	case SMCCC_ARCH_SOC_ID:
		return SMC_ARCH_CALL_SUCCESS;
	default:
		return SMC_ARCH_CALL_NOT_SUPPORTED;
	}
}

/* Get SOC version */
int32_t plat_get_soc_version(void)
{
	return (int32_t)
		(SOC_ID_SET_JEP_106(ARM_SOC_CONTINUATION_CODE,
				    ARM_SOC_IDENTIFICATION_CODE) |
		 (FVP_SOC_ID & SOC_ID_IMPL_DEF_MASK));
}

/* Get SOC revision */
int32_t plat_get_soc_revision(void)
{
	unsigned int sys_id;

	sys_id = mmio_read_32(V2M_SYSREGS_BASE + V2M_SYS_ID);
	return (int32_t)(((sys_id >> V2M_SYS_ID_REV_SHIFT) &
			  V2M_SYS_ID_REV_MASK) & SOC_ID_REV_MASK);
}

/* Get SoC name */
int32_t plat_get_soc_name(char *soc_name)
{
	snprintf(soc_name, SMCCC_SOC_NAME_LEN, "Arm Platform Revision %d",
			plat_get_soc_revision());
	return SMC_ARCH_CALL_SUCCESS;
}
#if ENABLE_RME

/* BDF mappings for RP0 RC0 */
const struct bdf_mapping_info rc0rp0_bdf_data[] = {
	/* BDF0 */
	{0U,		/* mapping_base */
	 0x8000U,	/* mapping_top */
	 0U,		/* mapping_off */
	 0U		/* smmu_idx */
	}
};

/* Root ports for RC0 */
const struct root_port_info rc0rp_data[] = {
	/* RP0 */
	{0U,						/* root_port_id */
	 0U,						/* padding */
	 ARRAY_SIZE(rc0rp0_bdf_data),			/* num_bdf_mappings */
	 (struct bdf_mapping_info *)rc0rp0_bdf_data	/* bdf_mappings */
	}
};

/* Root complexes */
const struct root_complex_info rc_data[] = {
	/* RC0 */
	{PCIE_EXP_BASE,				/* ecam_base */
	 0U,					/* segment */
	 {0U, 0U, 0U},				/* padding */
	 ARRAY_SIZE(rc0rp_data),		/* num_root_ports */
	 (struct root_port_info *)rc0rp_data	/* root_ports */
	}
};

/* Number of PCIe Root Complexes */
#define FVP_RMM_RC_COUNT	ARRAY_SIZE(rc_data)

/*
 * Get a pointer to the RMM-EL3 Shared buffer and return it
 * through the pointer passed as parameter.
 *
 * This function returns the size of the shared buffer.
 */
size_t plat_rmmd_get_el3_rmm_shared_mem(uintptr_t *shared)
{
	*shared = (uintptr_t)RMM_SHARED_BASE;

	return (size_t)RMM_SHARED_SIZE;
}

/*
 * Calculate checksum of 64-bit words @buffer with @size length
 */
static uint64_t checksum_calc(uint64_t *buffer, size_t size)
{
	uint64_t sum = 0UL;

	assert(((uintptr_t)buffer & (sizeof(uint64_t) - 1UL)) == 0UL);
	assert((size & (sizeof(uint64_t) - 1UL)) == 0UL);

	for (unsigned long i = 0UL; i < (size / sizeof(uint64_t)); i++) {
		sum += buffer[i];
	}

	return sum;
}
/*
 * Boot Manifest v0.5 structure illustration, with two DRAM banks,
 * a single console and one device memory with two PCIe device
 * non-coherent address ranges.
 *
 * +--------------------------------------------------+
 * | offset |        field       |      comment       |
 * +--------+--------------------+--------------------+
 * |   0    |       version      |     0x00000005     |
 * +--------+--------------------+--------------------+
 * |   4    |       padding      |     0x00000000     |
 * +--------+--------------------+--------------------+
 * |   8    |      plat_data     |       NULL         |
 * +--------+--------------------+--------------------+
 * |   16   |      num_banks     |                    |
 * +--------+--------------------+                    |
 * |   24   |       banks        |     plat_dram      +--+
 * +--------+--------------------+                    |  |
 * |   32   |      checksum      |                    |  |
 * +--------+--------------------+--------------------+  |
 * |   40   |    num_consoles    |                    |  |
 * +--------+--------------------+                    |  |
 * |   48   |      consoles      |    plat_console    +--|--+
 * +--------+--------------------+                    |  |  |
 * |   56   |      checksum      |                    |  |  |
 * +--------+--------------------+--------------------+  |  |
 * |   64   |      num_banks     |                    |  |  |
 * +--------+--------------------+                    |  |  |
 * |   72   |        banks       |  plat_ncoh_region  +--|--|--+
 * +--------+--------------------+                    |  |  |  |
 * |   80   |      checksum      |                    |  |  |  |
 * +--------+--------------------+--------------------+  |  |  |
 * |   88   |      num_banks     |                    |  |  |  |
 * +--------+--------------------+                    |  |  |  |
 * |   96   |       banks        |   plat_coh_region  |  |  |  |
 * +--------+--------------------+                    |  |  |  |
 * |   104  |      checksum      |                    |  |  |  |
 * +--------+--------------------+--------------------+  |  |  |
 * |   112  |     num_smmus      |                    |  |  |  |
 * +--------+--------------------+                    |  |  |  |
 * |   120  |       smmus        |     plat_smmu      +--|--|--|--+
 * +--------+--------------------+                    |  |  |  |  |
 * |   128  |      checksum      |                    |  |  |  |  |
 * +--------+--------------------+--------------------+  |  |  |  |
 * |   136  |  num_root_complex  |                    |  |  |  |  |
 * +--------+--------------------+                    |  |  |  |  |
 * |   144  |   rc_info_version  |                    |  |  |  |  |
 * +--------+--------------------+                    |  |  |  |  |
 * |   148  |      padding       | plat_root_complex  +--|--|--|--|--+
 * +--------+--------------------+                    |  |  |  |  |  |
 * |   152  |    root_complex    |                    |  |  |  |  |  |
 * +--------+--------------------+                    |  |  |  |  |  |
 * |   160  |      checksum      |                    |  |  |  |  |  |
 * +--------+--------------------+--------------------+<-+  |  |  |  |
 * |   168  |       base 0       |                    |     |  |  |  |
 * +--------+--------------------+     mem_bank[0]    |     |  |  |  |
 * |   176  |       size 0       |                    |     |  |  |  |
 * +--------+--------------------+--------------------+     |  |  |  |
 * |   184  |       base 1       |                    |     |  |  |  |
 * +--------+--------------------+     mem_bank[1]    |     |  |  |  |
 * |   192  |       size 1       |                    |     |  |  |  |
 * +--------+--------------------+--------------------+<----+  |  |  |
 * |   200  |       base         |                    |        |  |  |
 * +--------+--------------------+                    |        |  |  |
 * |   208  |      map_pages     |                    |        |  |  |
 * +--------+--------------------+                    |        |  |  |
 * |   216  |       name         |                    |        |  |  |
 * +--------+--------------------+     consoles[0]    |        |  |  |
 * |   224  |     clk_in_hz      |                    |        |  |  |
 * +--------+--------------------+                    |        |  |  |
 * |   232  |     baud_rate      |                    |        |  |  |
 * +--------+--------------------+                    |        |  |  |
 * |   240  |       flags        |                    |        |  |  |
 * +--------+--------------------+--------------------+<-------+  |  |
 * |   248  |       base 0       |                    |           |  |
 * +--------+--------------------+    ncoh_region[0]  |           |  |
 * |   256  |       size 0       |                    |           |  |
 * +--------+--------------------+--------------------+           |  |
 * |   264  |       base 1       |                    |           |  |
 * +--------+--------------------+    ncoh_region[1]  |           |  |
 * |   272  |       size 1       |                    |           |  |
 * +--------+--------------------+--------------------+<----------+  |
 * |   280  |     smmu_base      |                    |              |
 * +--------+--------------------+      smmus[0]      |              |
 * |   288  |     smmu_r_base    |                    |              |
 * +--------+--------------------+--------------------+<-------------+
 * |   296  |     ecam_base      |                    |
 * +--------+--------------------+                    |
 * |   304  |      segment       |                    |
 * +--------+--------------------+                    |
 * |   305  |      padding       |   root_complex[0]  +--+
 * +--------+--------------------+                    |  |
 * |   308  |   num_root_ports   |                    |  |
 * +--------+--------------------+                    |  |
 * |   312  |     root_ports     |                    |  |
 * +--------+--------------------+--------------------+<-+
 * |   320  |    root_port_id    |                    |
 * +--------+--------------------+                    |
 * |   322  |      padding       |                    |
 * +--------+--------------------+   root_ports[0]    +--+
 * |   324  |  num_bdf_mappings  |                    |  |
 * +--------+--------------------+                    |  |
 * |   328  |    bdf_mappings    |                    |  |
 * +--------+--------------------+--------------------+<-+
 * |   336  |    mapping_base    |                    |
 * +--------+--------------------+                    |
 * |   338  |    mapping_top     |                    |
 * +--------+--------------------+   bdf_mappings[0]  |
 * |   340  |    mapping_off     |                    |
 * +--------+--------------------+                    |
 * |   342  |     smmu_idx       |                    |
 * +--------+--------------------+--------------------+
 */
int plat_rmmd_load_manifest(struct rmm_manifest *manifest)
{
	uint64_t checksum, num_banks, num_consoles;
	uint64_t num_ncoh_regions, num_coh_regions;
	uint64_t num_smmus, num_root_complex;
	unsigned int num_root_ports, num_bdf_mappings;
	uint32_t o_realm;
	struct memory_bank *bank_ptr, *ncoh_region_ptr, *coh_region_ptr;
	struct console_info *console_ptr;
	struct smmu_info *smmu_ptr;
	struct root_complex_info *root_complex_ptr, *rc_ptr;
	struct root_port_info *root_port_ptr, *rp_ptr;
	struct bdf_mapping_info *bdf_mapping_ptr, *bdf_ptr;

	assert(manifest != NULL);

	/* Get number of DRAM banks */
	num_banks = FCONF_GET_PROPERTY(hw_config, dram_layout, num_banks);
	assert(num_banks <= ARM_DRAM_NUM_BANKS);

	/* Set number of consoles */
	num_consoles = FVP_RMM_CONSOLE_COUNT;

	/* Set number of device non-coherent address ranges for FVP RevC */
	num_ncoh_regions = 2;

	/* Set number of SMMUs */
	num_smmus = FVP_RMM_SMMU_COUNT;

	/* Set number of PCIe root complexes */
	num_root_complex = FVP_RMM_RC_COUNT;

	/* Calculate and set number of all PCIe root ports and BDF mappings */
	num_root_ports = 0U;
	num_bdf_mappings = 0U;

	/* Scan all root complex entries */
	for (unsigned long i = 0UL; i < num_root_complex; i++) {
		num_root_ports += rc_data[i].num_root_ports;

		/* Scan all root ports entries in root complex */
		for (unsigned int j = 0U; j < rc_data[i].num_root_ports; j++) {
			num_bdf_mappings += rc_data[i].root_ports[j].num_bdf_mappings;
		}
	}

	manifest->version = RMMD_MANIFEST_VERSION;
	manifest->padding = 0U; /* RES0 */
	manifest->plat_data = 0UL;
	manifest->plat_dram.num_banks = num_banks;
	manifest->plat_console.num_consoles = num_consoles;
	manifest->plat_ncoh_region.num_banks = num_ncoh_regions;
	manifest->plat_smmu.num_smmus = num_smmus;
	manifest->plat_root_complex.num_root_complex = num_root_complex;
	manifest->plat_root_complex.rc_info_version = PCIE_RC_INFO_VERSION;
	manifest->plat_root_complex.padding = 0U; /* RES0 */

	/* FVP does not support device coherent address ranges */
	num_coh_regions = 0UL;
	manifest->plat_coh_region.num_banks = num_coh_regions;
	manifest->plat_coh_region.banks = NULL;
	manifest->plat_coh_region.checksum = 0UL;

	bank_ptr = (struct memory_bank *)
			(((uintptr_t)manifest) + sizeof(struct rmm_manifest));
	console_ptr = (struct console_info *)
			((uintptr_t)bank_ptr + (num_banks *
						sizeof(struct memory_bank)));
	ncoh_region_ptr = (struct memory_bank *)
			((uintptr_t)console_ptr + (num_consoles *
						sizeof(struct console_info)));
	coh_region_ptr = (struct memory_bank *)
			((uintptr_t)ncoh_region_ptr + (num_ncoh_regions *
						sizeof(struct memory_bank)));
	smmu_ptr = (struct smmu_info *)
			((uintptr_t)coh_region_ptr + (num_coh_regions *
						sizeof(struct memory_bank)));
	root_complex_ptr = (struct root_complex_info *)
			((uintptr_t)smmu_ptr + (num_smmus *
						sizeof(struct smmu_info)));
	root_port_ptr = (struct	root_port_info *)
			((uintptr_t)root_complex_ptr + (num_root_complex *
						sizeof(struct root_complex_info)));
	bdf_mapping_ptr = (struct bdf_mapping_info *)
			((uintptr_t)root_port_ptr + (num_root_ports *
						sizeof(struct root_port_info)));

	manifest->plat_dram.banks = bank_ptr;
	manifest->plat_console.consoles = console_ptr;
	manifest->plat_ncoh_region.banks = ncoh_region_ptr;
	manifest->plat_smmu.smmus = smmu_ptr;
	manifest->plat_root_complex.root_complex = root_complex_ptr;

	/* Ensure the manifest is not larger than the shared buffer */
	assert((sizeof(struct rmm_manifest) +
		(sizeof(struct memory_bank) *
			manifest->plat_dram.num_banks) +
		(sizeof(struct console_info) *
			manifest->plat_console.num_consoles) +
		(sizeof(struct memory_bank) *
			manifest->plat_ncoh_region.num_banks) +
		(sizeof(struct memory_bank) *
			manifest->plat_coh_region.num_banks) +
		(sizeof(struct smmu_info) *
			manifest->plat_smmu.num_smmus) +
		(sizeof(struct root_complex_info) *
			manifest->plat_root_complex.num_root_complex) +
		(sizeof(struct root_port_info) * num_root_ports) +
		(sizeof(struct bdf_mapping_info) * num_bdf_mappings))
		<= ARM_EL3_RMM_SHARED_SIZE);

	/* Calculate checksum of plat_dram structure */
	checksum = num_banks + (uint64_t)bank_ptr;

	/* Store FVP DRAM banks data in Boot Manifest */
	for (unsigned long i = 0UL; i < num_banks; i++) {
		bank_ptr[i].base = FCONF_GET_PROPERTY(hw_config, dram_layout, dram_bank[i].base);
		bank_ptr[i].size = FCONF_GET_PROPERTY(hw_config, dram_layout, dram_bank[i].size);
	}

	/* Update checksum */
	checksum += checksum_calc((uint64_t *)bank_ptr, sizeof(struct memory_bank) * num_banks);

	/* Checksum must be 0 */
	manifest->plat_dram.checksum = ~checksum + 1UL;

	/* Calculate the checksum of plat_consoles structure */
	checksum = num_consoles + (uint64_t)console_ptr;

	/* Zero out the console info struct */
	(void)memset((void *)console_ptr, '\0',
			sizeof(struct console_info) * num_consoles);

	console_ptr[0].base = FVP_RMM_CONSOLE_BASE;
	console_ptr[0].map_pages = 1UL;
	console_ptr[0].clk_in_hz = FVP_RMM_CONSOLE_CLK_IN_HZ;
	console_ptr[0].baud_rate = FVP_RMM_CONSOLE_BAUD;

	(void)strlcpy(console_ptr[0].name, FVP_RMM_CONSOLE_NAME,
						RMM_CONSOLE_MAX_NAME_LEN - 1UL);

	/* Update checksum */
	checksum += checksum_calc((uint64_t *)console_ptr,
					sizeof(struct console_info) * num_consoles);
	/* Checksum must be 0 */
	manifest->plat_console.checksum = ~checksum + 1UL;

	/*
	 * Calculate the checksum of device non-coherent address ranges
	 * info structure
	 */
	checksum = num_ncoh_regions + (uint64_t)ncoh_region_ptr;

	/* Zero out the PCIe region info struct */
	(void)memset((void *)ncoh_region_ptr, 0,
			sizeof(struct memory_bank) * num_ncoh_regions);

	/* Set number of device non-coherent address ranges based on DT */
	num_ncoh_regions = FCONF_GET_PROPERTY(hw_config, pci_props, num_ncoh_regions);
	/* At least 1 PCIe region need to be described in DT */
	assert((num_ncoh_regions > 0) && (num_ncoh_regions <= 2));

	for (unsigned long i = 0UL; i < num_ncoh_regions; i++) {
		ncoh_region_ptr[i].base =
			FCONF_GET_PROPERTY(hw_config, pci_props, ncoh_regions[i].base);
		ncoh_region_ptr[i].size =
			FCONF_GET_PROPERTY(hw_config, pci_props, ncoh_regions[i].size);
	}

	/*
	 * Workaround if the DT does not specify the 2nd PCIe region. This code can be
	 * removed when upstream DT is updated to have 2nd PCIe region.
	 */
	if (num_ncoh_regions == 1) {
		num_ncoh_regions++;
		/* Add 3GB of 2nd PCIe region */
		ncoh_region_ptr[1].base = 0x4000000000;
		ncoh_region_ptr[1].size = 0xc0000000;
	}

	/* Update checksum */
	checksum += checksum_calc((uint64_t *)ncoh_region_ptr,
			sizeof(struct memory_bank) * num_ncoh_regions);

	/* Checksum must be 0 */
	manifest->plat_ncoh_region.checksum = ~checksum + 1UL;

	/* Calculate the checksum of the plat_smmu structure */
	checksum = num_smmus + (uint64_t)smmu_ptr;

	smmu_ptr[0].smmu_base = FVP_RMM_SMMU_BASE;

	/* Read SMMU_ROOT_IDR0.BA_REALM[31:22] register field */
	o_realm = mmio_read_32(FVP_RMM_SMMU_BASE + SMMU_ROOT_IDR0) &
				SMMU_ROOT_IDR0_BA_REALM_MASK;
	/*
	 * Calculate the base address offset of Realm Register Page 0.
	 * O_REALM = 0x20000 + (BA_REALM * 0x10000)
	 * SMMU_REALM_BASE = SMMU_PAGE_0_BASE + O_REALM
	 */
	o_realm = 0x20000 + (o_realm >> (SMMU_ROOT_IDR0_BA_REALM_SHIFT - 16U));

	smmu_ptr[0].smmu_r_base = FVP_RMM_SMMU_BASE + o_realm;

	/* Update checksum */
	checksum += checksum_calc((uint64_t *)smmu_ptr,
					sizeof(struct smmu_info) * num_smmus);
	/* Checksum must be 0 */
	manifest->plat_smmu.checksum = ~checksum + 1UL;

	/* Calculate the checksum of the plat_root_complex structure */
	checksum = num_root_complex + (uint64_t)root_complex_ptr;

	/* Zero out PCIe root complex info structures */
	(void)memset((void *)root_complex_ptr, 0,
			sizeof(struct root_complex_info) * num_root_complex);

	/* Set pointers for data in manifest */
	rc_ptr = root_complex_ptr;
	rp_ptr = root_port_ptr;
	bdf_ptr = bdf_mapping_ptr;

	/* Fill PCIe root complex info structures */
	for (unsigned long i = 0U; i < num_root_complex; i++) {
		const struct root_complex_info *rc_info = &rc_data[i];
		const struct root_port_info *rp_info = rc_info->root_ports;

		/* Copy root complex data, except root_ports pointer */
		(void)memcpy((void *)rc_ptr, (void *)rc_info,
			sizeof(struct root_complex_info) - sizeof(struct root_port_info *));

		/* Set root_ports for root complex */
		rc_ptr->root_ports = rp_ptr;

		/* Scan root ports */
		for (unsigned int j = 0U; j < rc_ptr->num_root_ports; j++) {
			const struct bdf_mapping_info *bdf_info = rp_info->bdf_mappings;

			/* Copy root port data, except bdf_mappings pointer */
			(void)memcpy((void *)rp_ptr, (void *)rp_info,
				sizeof(struct root_port_info) - sizeof(struct bdf_mapping_info *));

			/* Set bdf_mappings for root port */
			rp_ptr->bdf_mappings = bdf_ptr;

			/* Copy all BDF mappings for root port */
			(void)memcpy((void *)bdf_ptr, (void *)bdf_info,
				sizeof(struct bdf_mapping_info) * rp_ptr->num_bdf_mappings);

			bdf_ptr += rp_ptr->num_bdf_mappings;
			rp_ptr++;
			rp_info++;
		}
		rc_ptr++;
	}

	/* Check that all data are written in manifest */
	assert(rc_ptr == (root_complex_ptr + num_root_complex));
	assert(rp_ptr == (root_port_ptr + num_root_ports));
	assert(bdf_ptr == (bdf_mapping_ptr + num_bdf_mappings));

	/* Update checksum for all PCIe data */
	checksum += checksum_calc((uint64_t *)root_complex_ptr,
				(uintptr_t)bdf_ptr - (uintptr_t)root_complex_ptr);

	/* Checksum must be 0 */
	manifest->plat_root_complex.checksum = ~checksum + 1UL;

	return 0;
}

/*
 * Update encryption key associated with @mecid.
 */
int plat_rmmd_mecid_key_update(uint16_t mecid, unsigned int reason)
{
	/*
	 * FVP does not provide an interface to change the encryption key associated
	 * with MECID. Hence always return success.
	 */
	return 0;
}
#endif /* ENABLE_RME */

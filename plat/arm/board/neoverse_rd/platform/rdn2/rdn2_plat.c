/*
 * Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/gic600_multichip.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <services/el3_spmc_ffa_memory.h>

#include <nrd_plat.h>
#include <rdn2_ras.h>

#if defined(IMAGE_BL31)
#if (NRD_PLATFORM_VARIANT == 2)
static const mmap_region_t rdn2mc_dynamic_mmap[] = {
#if NRD_CHIP_COUNT > 1
	NRD_CSS_SHARED_RAM_MMAP(1),
	NRD_CSS_PERIPH_MMAP(1),
#endif
#if NRD_CHIP_COUNT > 2
	NRD_CSS_SHARED_RAM_MMAP(2),
	NRD_CSS_PERIPH_MMAP(2),
#endif
#if NRD_CHIP_COUNT > 3
	NRD_CSS_SHARED_RAM_MMAP(3),
	NRD_CSS_PERIPH_MMAP(3),
#endif
};
#endif

#if (NRD_PLATFORM_VARIANT == 2)
static struct gic600_multichip_data rdn2mc_multichip_data __init = {
	.rt_owner_base = PLAT_ARM_GICD_BASE,
	.rt_owner = 0,
	.chip_count = NRD_CHIP_COUNT,
	.chip_addrs = {
		PLAT_ARM_GICD_BASE >> 16,
#if NRD_CHIP_COUNT > 1
		(PLAT_ARM_GICD_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(1)) >> 16,
#endif
#if NRD_CHIP_COUNT > 2
		(PLAT_ARM_GICD_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(2)) >> 16,
#endif
#if NRD_CHIP_COUNT > 3
		(PLAT_ARM_GICD_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(3)) >> 16,
#endif
	},
	.spi_ids = {
		{PLAT_ARM_GICD_BASE,
		NRD_CHIP0_SPI_MIN,
		NRD_CHIP0_SPI_MAX},
	#if NRD_CHIP_COUNT > 1
		{PLAT_ARM_GICD_BASE,
		NRD_CHIP1_SPI_MIN,
		NRD_CHIP1_SPI_MAX},
	#endif
	#if NRD_CHIP_COUNT > 2
		{PLAT_ARM_GICD_BASE,
		NRD_CHIP2_SPI_MIN,
		NRD_CHIP2_SPI_MAX},
	#endif
	#if NRD_CHIP_COUNT > 3
		{PLAT_ARM_GICD_BASE,
		NRD_CHIP3_SPI_MIN,
		NRD_CHIP3_SPI_MAX},
	#endif
	}
};
#endif

#if (NRD_PLATFORM_VARIANT == 2)
static uintptr_t rdn2mc_multichip_gicr_frames[] = {
	/* Chip 0's GICR Base */
	PLAT_ARM_GICR_BASE,
#if NRD_CHIP_COUNT > 1
	/* Chip 1's GICR BASE */
	PLAT_ARM_GICR_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(1),
#endif
#if NRD_CHIP_COUNT > 2
	/* Chip 2's GICR BASE */
	PLAT_ARM_GICR_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(2),
#endif
#if NRD_CHIP_COUNT > 3
	/* Chip 3's GICR BASE */
	PLAT_ARM_GICR_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(3),
#endif
	UL(0)	/* Zero Termination */
};
#endif
#endif /* IMAGE_BL31 */

unsigned int plat_arm_nrd_get_platform_id(void)
{
	return mmio_read_32(SID_REG_BASE + SID_SYSTEM_ID_OFFSET)
			    & SID_SYSTEM_ID_PART_NUM_MASK;
}

unsigned int plat_arm_nrd_get_config_id(void)
{
	return mmio_read_32(SID_REG_BASE + SID_SYSTEM_CFG_OFFSET);
}

unsigned int plat_arm_nrd_get_multi_chip_mode(void)
{
	return (mmio_read_32(SID_REG_BASE + SID_NODE_ID_OFFSET) &
			     SID_MULTI_CHIP_MODE_MASK) >>
			     SID_MULTI_CHIP_MODE_SHIFT;
}

#if defined(IMAGE_BL31)
void bl31_platform_setup(void)
{
#if (NRD_PLATFORM_VARIANT == 2)
	int ret;
	unsigned int i;

	if (plat_arm_nrd_get_multi_chip_mode() == 0) {
		ERROR("Chip Count is %u but multi-chip mode is not enabled\n",
			NRD_CHIP_COUNT);
		panic();
	} else {
		INFO("Enabling multi-chip support for RD-N2 variant\n");

		for (i = 0; i < ARRAY_SIZE(rdn2mc_dynamic_mmap); i++) {
			ret = mmap_add_dynamic_region(
					rdn2mc_dynamic_mmap[i].base_pa,
					rdn2mc_dynamic_mmap[i].base_va,
					rdn2mc_dynamic_mmap[i].size,
					rdn2mc_dynamic_mmap[i].attr);
			if (ret != 0) {
				ERROR("Failed to add dynamic mmap entry for"
					" i: %d " "(ret=%d)\n", i, ret);
				panic();
			}
		}

		plat_arm_override_gicr_frames(
			rdn2mc_multichip_gicr_frames);
		gic600_multichip_init(&rdn2mc_multichip_data);
	}
#endif

	nrd_bl31_common_platform_setup();

#if ENABLE_FEAT_RAS && FFH_SUPPORT
	nrd_ras_platform_setup(&ras_config);
#endif
}
#endif /* IMAGE_BL31 */

#if SPMC_AT_EL3

#define DATASTORE_SIZE 1024

__section("arm_el3_tzc_dram") uint8_t plat_spmc_shmem_datastore[DATASTORE_SIZE];

int plat_spmc_shmem_datastore_get(uint8_t **datastore, size_t *size)
{
	*datastore = plat_spmc_shmem_datastore;
	*size = DATASTORE_SIZE;
	return 0;
}

/*
 * Add dummy implementations of memory management related platform hooks.
 * Memory share/lend operation are not required on RdN2 platform.
 */
int plat_spmc_shmem_begin(struct ffa_mtd *desc)
{
	return 0;
}

int plat_spmc_shmem_reclaim(struct ffa_mtd *desc)
{
	return 0;
}

int plat_spmd_handle_group0_interrupt(uint32_t intid)
{
	/*
	 * As of now, there are no sources of Group0 secure interrupt enabled
	 * for RDN2.
	 */
	(void)intid;
	return -1;
}
#endif

/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/interrupt_props.h>
#include <drivers/arm/gicv2.h>
#include <drivers/arm/gic_common.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include "rcar_def.h"
#include "rcar_private.h"
#include "rcar_version.h"

#if (IMAGE_BL2)
extern void rcar_read_certificate(uint64_t cert, uint32_t *len, uintptr_t *p);
extern int32_t rcar_get_certificate(const int32_t name, uint32_t *cert);
#endif

const uint8_t version_of_renesas[VERSION_OF_RENESAS_MAXLEN]
		__attribute__ ((__section__("ro"))) = VERSION_OF_RENESAS;

#define MAP_SHARED_RAM		MAP_REGION_FLAT(RCAR_SHARED_MEM_BASE,	\
					RCAR_SHARED_MEM_SIZE,		\
					MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_FLASH0		MAP_REGION_FLAT(FLASH0_BASE,		\
					FLASH0_SIZE,			\
					MT_MEMORY | MT_RO | MT_SECURE)

#define MAP_DRAM1_NS		MAP_REGION_FLAT(DRAM1_NS_BASE,		\
					DRAM1_NS_SIZE,			\
					MT_MEMORY | MT_RW | MT_NS)

#define MAP_DEVICE_RCAR		MAP_REGION_FLAT(DEVICE_RCAR_BASE,	\
					DEVICE_RCAR_SIZE,		\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_DEVICE_RCAR2	MAP_REGION_FLAT(DEVICE_RCAR_BASE2,	\
					DEVICE_RCAR_SIZE2,		\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_SRAM		MAP_REGION_FLAT(DEVICE_SRAM_BASE,	\
					DEVICE_SRAM_SIZE,		\
					MT_MEMORY | MT_RO | MT_SECURE)

#define MAP_SRAM_STACK		MAP_REGION_FLAT(DEVICE_SRAM_STACK_BASE,	\
					DEVICE_SRAM_STACK_SIZE,		\
					MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_ATFW_CRASH  	MAP_REGION_FLAT(RCAR_BL31_CRASH_BASE,	\
					RCAR_BL31_CRASH_SIZE,		\
					MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_ATFW_LOG		MAP_REGION_FLAT(RCAR_BL31_LOG_BASE,	\
					RCAR_BL31_LOG_SIZE,		\
					MT_DEVICE | MT_RW | MT_SECURE)
#if IMAGE_BL2
#define MAP_DRAM0		MAP_REGION_FLAT(DRAM1_BASE,		\
					DRAM1_SIZE,			\
					MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_REG0		MAP_REGION_FLAT(DEVICE_RCAR_BASE,	\
					DEVICE_RCAR_SIZE,		\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_RAM0		MAP_REGION_FLAT(RCAR_SYSRAM_BASE,	\
					RCAR_SYSRAM_SIZE,		\
					MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_REG1		MAP_REGION_FLAT(REG1_BASE,		\
					REG1_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_ROM			MAP_REGION_FLAT(ROM0_BASE,		\
					ROM0_SIZE,			\
					MT_MEMORY | MT_RO | MT_SECURE)

#define MAP_REG2		MAP_REGION_FLAT(REG2_BASE,		\
					REG2_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_DRAM1		MAP_REGION_FLAT(DRAM_40BIT_BASE,	\
					DRAM_40BIT_SIZE,		\
					MT_MEMORY | MT_RW | MT_SECURE)
#endif

#ifdef BL32_BASE
#define MAP_BL32_MEM		MAP_REGION_FLAT(BL32_BASE,		\
					BL32_LIMIT - BL32_BASE,		\
					MT_MEMORY | MT_RW | MT_SECURE)
#endif

#if IMAGE_BL2
static const mmap_region_t rcar_mmap[] = {
	MAP_FLASH0,	/*   0x08000000 -   0x0BFFFFFF  RPC area            */
	MAP_DRAM0,	/*   0x40000000 -   0xBFFFFFFF  DRAM area(Legacy)   */
	MAP_REG0,	/*   0xE6000000 -   0xE62FFFFF  SoC register area   */
	MAP_RAM0,	/*   0xE6300000 -   0xE6303FFF  System RAM area     */
	MAP_REG1,	/*   0xE6400000 -   0xEAFFFFFF  SoC register area   */
	MAP_ROM,	/*   0xEB100000 -   0xEB127FFF  boot ROM area       */
	MAP_REG2,	/*   0xEC000000 -   0xFFFFFFFF  SoC register area   */
	MAP_DRAM1,	/* 0x0400000000 - 0x07FFFFFFFF  DRAM area(4GB over) */
	{0}
};
#endif

#if IMAGE_BL31
static const mmap_region_t rcar_mmap[] = {
	MAP_SHARED_RAM,
	MAP_ATFW_CRASH,
	MAP_ATFW_LOG,
	MAP_DEVICE_RCAR,
	MAP_DEVICE_RCAR2,
	MAP_SRAM,
	MAP_SRAM_STACK,
	{0}
};
#endif

#if IMAGE_BL32
static const mmap_region_t rcar_mmap[] = {
	MAP_DEVICE0,
	MAP_DEVICE1,
	{0}
};
#endif

CASSERT(ARRAY_SIZE(rcar_mmap) + RCAR_BL_REGIONS
	<= MAX_MMAP_REGIONS, assert_max_mmap_regions);

/*
 * Macro generating the code for the function setting up the pagetables as per
 * the platform memory map & initialize the mmu, for the given exception level
 */
#if USE_COHERENT_MEM
void rcar_configure_mmu_el3(unsigned long total_base,
			    unsigned long total_size,
			    unsigned long ro_start,
			    unsigned long ro_limit,
			    unsigned long coh_start,
			    unsigned long coh_limit)
{
	mmap_add_region(total_base, total_base, total_size,
			MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(ro_start, ro_start, ro_limit - ro_start,
			MT_MEMORY | MT_RO | MT_SECURE);
	mmap_add_region(coh_start, coh_start, coh_limit - coh_start,
			MT_DEVICE | MT_RW | MT_SECURE);
	mmap_add(rcar_mmap);

	init_xlat_tables();
	enable_mmu_el3(0);
}
#else
void rcar_configure_mmu_el3(unsigned long total_base,
			    unsigned long total_size,
			    unsigned long ro_start,
			    unsigned long ro_limit)
{
	mmap_add_region(total_base, total_base, total_size,
			MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(ro_start, ro_start, ro_limit - ro_start,
			MT_MEMORY | MT_RO | MT_SECURE);
	mmap_add(rcar_mmap);

	init_xlat_tables();
	enable_mmu_el3(0);
}
#endif

uintptr_t plat_get_ns_image_entrypoint(void)
{
#if (IMAGE_BL2)
	uint32_t cert, len;
	uintptr_t dst;
	int32_t ret;

	ret = rcar_get_certificate(NON_TRUSTED_FW_CONTENT_CERT_ID, &cert);
	if (ret) {
		ERROR("%s : cert file load error", __func__);
		return NS_IMAGE_OFFSET;
	}

	rcar_read_certificate((uint64_t) cert, &len, &dst);

	return dst;
#else
	return NS_IMAGE_OFFSET;
#endif
}

unsigned int plat_get_syscnt_freq2(void)
{
	unsigned int freq;

	freq = mmio_read_32(ARM_SYS_CNTCTL_BASE + CNTFID_OFF);
	if (freq == 0)
		panic();

	return freq;
}

void plat_rcar_gic_init(void)
{
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

static const interrupt_prop_t interrupt_props[] = {
#if IMAGE_BL2
	INTR_PROP_DESC(ARM_IRQ_SEC_WDT, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
#else
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(ARM_IRQ_SEC_RPC, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(ARM_IRQ_SEC_TIMER, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(ARM_IRQ_SEC_TIMER_UP, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(ARM_IRQ_SEC_WDT, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(ARM_IRQ_SEC_CRYPT, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(ARM_IRQ_SEC_CRYPT_SecPKA, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(ARM_IRQ_SEC_CRYPT_PubPKA, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
#endif
};

static const gicv2_driver_data_t plat_gicv2_driver_data = {
	.interrupt_props = interrupt_props,
	.interrupt_props_num = (uint32_t) ARRAY_SIZE(interrupt_props),
	.gicd_base = RCAR_GICD_BASE,
	.gicc_base = RCAR_GICC_BASE,
};

void plat_rcar_gic_driver_init(void)
{
	gicv2_driver_init(&plat_gicv2_driver_data);
}

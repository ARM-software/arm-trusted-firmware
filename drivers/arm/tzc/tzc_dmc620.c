/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/arm/tzc_dmc620.h>
#include <lib/mmio.h>

/* Mask to extract bit 31 to 16 */
#define MASK_31_16 UINT64_C(0x0000ffff0000)
/* Mask to extract bit 47 to 32 */
#define MASK_47_32 UINT64_C(0xffff00000000)

/* Helper macro for getting dmc_base addr of a dmc_inst */
#define DMC_BASE(plat_data, dmc_inst) \
	((uintptr_t)(plat_data->dmc_base[dmc_inst]))

/* Pointer to the tzc_dmc620_config_data structure populated by the platform */
static const tzc_dmc620_config_data_t *g_plat_config_data;

#if ENABLE_ASSERTIONS
/*
 * Helper function to check if the DMC-620 instance is present at the
 * base address provided by the platform and also check if at least
 * one dmc instance is present.
 */
static void tzc_dmc620_validate_plat_driver_data(
			const tzc_dmc620_driver_data_t *plat_driver_data)
{
	uint8_t dmc_inst, dmc_count;
	unsigned int dmc_id;
	uintptr_t base;

	assert(plat_driver_data != NULL);

	dmc_count = plat_driver_data->dmc_count;
	assert(dmc_count > 0U);

	for (dmc_inst = 0U; dmc_inst < dmc_count; dmc_inst++) {
		base = DMC_BASE(plat_driver_data, dmc_inst);
		dmc_id = mmio_read_32(base + DMC620_PERIPHERAL_ID_0);
		assert(dmc_id == DMC620_PERIPHERAL_ID_0_VALUE);
	}
}
#endif

/*
 * Program a region with region base and region top addresses of all
 * DMC-620 instances.
 */
static void tzc_dmc620_configure_region(int region_no,
					unsigned long long region_base,
					unsigned long long region_top,
					unsigned int sec_attr)
{
	uint32_t min_31_00, min_47_32;
	uint32_t max_31_00, max_47_32;
	uint8_t dmc_inst, dmc_count;
	uintptr_t base;
	const tzc_dmc620_driver_data_t *plat_driver_data;

	plat_driver_data = g_plat_config_data->plat_drv_data;
	assert(plat_driver_data != NULL);

	/* Do range checks on regions. */
	assert((region_no >= 0U) && (region_no <= DMC620_ACC_ADDR_COUNT));

	/* region_base and (region_top + 1) must be 4KB aligned */
	assert(((region_base | (region_top + 1U)) & (4096U - 1U)) == 0U);

	dmc_count = plat_driver_data->dmc_count;
	for (dmc_inst = 0U; dmc_inst < dmc_count; dmc_inst++) {
		min_31_00 = (region_base & MASK_31_16) | sec_attr;
		min_47_32 = (region_base & MASK_47_32)
				>> DMC620_ACC_ADDR_WIDTH;
		max_31_00 = (region_top  & MASK_31_16);
		max_47_32 = (region_top  & MASK_47_32)
				>> DMC620_ACC_ADDR_WIDTH;

		/* Extract the base address of the DMC-620 instance */
		base = DMC_BASE(plat_driver_data, dmc_inst);
		/* Configure access address region registers */
		mmio_write_32(base + DMC620_ACC_ADDR_MIN_31_00_NEXT(region_no),
				min_31_00);
		mmio_write_32(base + DMC620_ACC_ADDR_MIN_47_32_NEXT(region_no),
				min_47_32);
		mmio_write_32(base + DMC620_ACC_ADDR_MAX_31_00_NEXT(region_no),
				max_31_00);
		mmio_write_32(base + DMC620_ACC_ADDR_MAX_47_32_NEXT(region_no),
				max_47_32);
	}
}

/*
 * Set the action value for all the DMC-620 instances.
 */
static void tzc_dmc620_set_action(void)
{
	uint8_t dmc_inst, dmc_count;
	uintptr_t base;
	const tzc_dmc620_driver_data_t *plat_driver_data;

	plat_driver_data = g_plat_config_data->plat_drv_data;
	dmc_count = plat_driver_data->dmc_count;
	for (dmc_inst = 0U; dmc_inst < dmc_count; dmc_inst++) {
		/* Extract the base address of the DMC-620 instance */
		base = DMC_BASE(plat_driver_data, dmc_inst);
		/* Switch to READY */
		mmio_write_32(base + DMC620_MEMC_CMD, DMC620_MEMC_CMD_GO);
		mmio_write_32(base + DMC620_MEMC_CMD, DMC620_MEMC_CMD_EXECUTE);
	}
}

/*
 * Verify whether the DMC-620 configuration is complete by reading back
 * configuration registers and comparing it with the configured value. If
 * configuration is incomplete, loop till the configured value is reflected in
 * the register.
 */
static void tzc_dmc620_verify_complete(void)
{
	uint8_t dmc_inst, dmc_count;
	uintptr_t base;
	const tzc_dmc620_driver_data_t *plat_driver_data;

	plat_driver_data = g_plat_config_data->plat_drv_data;
	dmc_count = plat_driver_data->dmc_count;
	for (dmc_inst = 0U; dmc_inst < dmc_count; dmc_inst++) {
		/* Extract the base address of the DMC-620 instance */
		base = DMC_BASE(plat_driver_data, dmc_inst);
		while ((mmio_read_32(base + DMC620_MEMC_STATUS) &
				DMC620_MEMC_CMD_MASK) != DMC620_MEMC_CMD_GO)
			continue;
	}
}

/*
 * Initialize the DMC-620 TrustZone Controller using the region configuration
 * supplied by the platform. The DMC620 controller should be enabled elsewhere
 * before invoking this function.
 */
void arm_tzc_dmc620_setup(const tzc_dmc620_config_data_t *plat_config_data)
{
	int i;

	/* Check if valid pointer is passed */
	assert(plat_config_data != NULL);

	/*
	 * Check if access address count passed by the platform is less than or
	 * equal to DMC620's access address count
	 */
	assert(plat_config_data->acc_addr_count <= DMC620_ACC_ADDR_COUNT);

#if ENABLE_ASSERTIONS
	/* Validates the information passed by platform */
	tzc_dmc620_validate_plat_driver_data(plat_config_data->plat_drv_data);
#endif

	g_plat_config_data = plat_config_data;

	INFO("Configuring DMC-620 TZC settings\n");
	for (i = 0U; i < g_plat_config_data->acc_addr_count; i++)
		tzc_dmc620_configure_region(i,
			g_plat_config_data->plat_acc_addr_data[i].region_base,
			g_plat_config_data->plat_acc_addr_data[i].region_top,
			g_plat_config_data->plat_acc_addr_data[i].sec_attr);

	tzc_dmc620_set_action();
	tzc_dmc620_verify_complete();
	INFO("DMC-620 TZC setup completed\n");
}

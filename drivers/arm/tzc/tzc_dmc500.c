/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <debug.h>
#include <mmio.h>
#include <tzc_dmc500.h>
#include "tzc_common.h"
#include "tzc_common_private.h"

/*
 * Macros which will be used by common core functions.
 */
#define TZC_DMC500_REGION_BASE_LOW_0_OFFSET		0x054
#define TZC_DMC500_REGION_BASE_HIGH_0_OFFSET		0x058
#define TZC_DMC500_REGION_TOP_LOW_0_OFFSET		0x05C
#define TZC_DMC500_REGION_TOP_HIGH_0_OFFSET		0x060
#define TZC_DMC500_REGION_ATTR_0_OFFSET			0x064
#define TZC_DMC500_REGION_ID_ACCESS_0_OFFSET		0x068

#define TZC_DMC500_ACTION_OFF				0x50

/* Pointer to the tzc_dmc500_driver_data structure populated by the platform */
static const tzc_dmc500_driver_data_t *g_driver_data;

#define verify_region_attr(region, attr)	\
		((g_conf_regions[(region)].sec_attr ==			\
			((attr) >> TZC_REGION_ATTR_SEC_SHIFT))		\
		&& ((attr) & (0x1 << TZC_REGION_ATTR_F_EN_SHIFT)))

/*
 * Structure for configured regions attributes in DMC500.
 */
typedef struct tzc_dmc500_regions {
	tzc_region_attributes_t sec_attr;
	int is_enabled;
} tzc_dmc500_regions_t;

/*
 * Array storing the attributes of the configured regions. This array
 * will be used by the `tzc_dmc500_verify_complete` to verify the flush
 * completion.
 */
static tzc_dmc500_regions_t g_conf_regions[MAX_REGION_VAL + 1];

/* Helper Macros for making the code readable */
#define DMC_INST_BASE_ADDR(instance) (g_driver_data->dmc_base[instance])
#define DMC_INST_SI_BASE(instance, interface) \
		(DMC_INST_BASE_ADDR(instance) + IFACE_OFFSET(interface))

DEFINE_TZC_COMMON_WRITE_ACTION(_dmc500, DMC500)
DEFINE_TZC_COMMON_WRITE_REGION_BASE(_dmc500, DMC500)
DEFINE_TZC_COMMON_WRITE_REGION_TOP(_dmc500, DMC500)
DEFINE_TZC_COMMON_WRITE_REGION_ATTRIBUTES(_dmc500, DMC500)
DEFINE_TZC_COMMON_WRITE_REGION_ID_ACCESS(_dmc500, DMC500)

DEFINE_TZC_COMMON_CONFIGURE_REGION0(_dmc500)
DEFINE_TZC_COMMON_CONFIGURE_REGION(_dmc500)

static inline unsigned int _tzc_dmc500_read_region_attr_0(
					uintptr_t dmc_si_base,
					int region_no)
{
	return mmio_read_32(dmc_si_base +
			TZC_REGION_OFFSET(TZC_DMC500_REGION_SIZE, region_no) +
			TZC_DMC500_REGION_ATTR_0_OFFSET);
}

static inline void _tzc_dmc500_write_flush_control(uintptr_t dmc_si_base)
{
	mmio_write_32(dmc_si_base + SI_FLUSH_CTRL_OFFSET, 1);
}

/*
 * Sets the Flush controls for all the DMC Instances and System Interfaces.
 * This initiates the flush of configuration settings from the shadow
 * registers to the actual configuration register. The caller should poll
 * changed register to confirm update.
 */
void tzc_dmc500_config_complete(void)
{
	int dmc_inst, sys_if;

	assert(g_driver_data);

	for (dmc_inst = 0; dmc_inst < g_driver_data->dmc_count; dmc_inst++) {
		assert(DMC_INST_BASE_ADDR(dmc_inst));
		for (sys_if = 0; sys_if < MAX_SYS_IF_COUNT; sys_if++)
			_tzc_dmc500_write_flush_control(
					DMC_INST_SI_BASE(dmc_inst, sys_if));
	}
}

/*
 * This function reads back the secure attributes from the configuration
 * register for each DMC Instance and System Interface and compares it with
 * the configured value. The successful verification of the region attributes
 * confirms that the flush operation has completed.
 * If the verification fails, the caller is expected to invoke this API again
 * till it succeeds.
 * Returns 0 on success and 1 on failure.
 */
int tzc_dmc500_verify_complete(void)
{
	int dmc_inst, sys_if, region_no;
	unsigned int attr;

	assert(g_driver_data);
	/* Region 0 must be configured */
	assert(g_conf_regions[0].is_enabled);

	/* Iterate over all configured regions */
	for (region_no = 0; region_no <= MAX_REGION_VAL; region_no++) {
		if (!g_conf_regions[region_no].is_enabled)
			continue;
		for (dmc_inst = 0; dmc_inst < g_driver_data->dmc_count;
								dmc_inst++) {
			assert(DMC_INST_BASE_ADDR(dmc_inst));
			for (sys_if = 0; sys_if < MAX_SYS_IF_COUNT;
							sys_if++) {
				attr = _tzc_dmc500_read_region_attr_0(
					DMC_INST_SI_BASE(dmc_inst, sys_if),
					region_no);
				VERBOSE("Verifying DMC500 region:%d"
					" dmc_inst:%d sys_if:%d attr:%x\n",
					region_no, dmc_inst, sys_if, attr);
				if (!verify_region_attr(region_no, attr))
					return 1;
			}
		}
	}

	return 0;
}

/*
 * `tzc_dmc500_configure_region0` is used to program region 0 in both the
 * system interfaces of all the DMC-500 instances. Region 0 covers the whole
 * address space that is not mapped to any other region for a system interface,
 * and is always enabled; this cannot be changed. This function only changes
 * the access permissions.
 */
void tzc_dmc500_configure_region0(tzc_region_attributes_t sec_attr,
					unsigned int nsaid_permissions)
{
	int dmc_inst, sys_if;

	/* Assert if DMC-500 is not initialized */
	assert(g_driver_data);

	/* Configure region_0 in all DMC instances */
	for (dmc_inst = 0; dmc_inst < g_driver_data->dmc_count; dmc_inst++) {
		assert(DMC_INST_BASE_ADDR(dmc_inst));
		for (sys_if = 0; sys_if < MAX_SYS_IF_COUNT; sys_if++)
			_tzc_dmc500_configure_region0(
					DMC_INST_SI_BASE(dmc_inst, sys_if),
					sec_attr, nsaid_permissions);
	}

	g_conf_regions[0].sec_attr = sec_attr;
	g_conf_regions[0].is_enabled = 1;
}

/*
 * `tzc_dmc500_configure_region` is used to program a region into all system
 * interfaces of all the DMC instances.
 * NOTE:
 * Region 0 is special; it is preferable to use tzc_dmc500_configure_region0
 * for this region (see comment for that function).
 */
void tzc_dmc500_configure_region(int region_no,
			unsigned long long region_base,
			unsigned long long region_top,
			tzc_region_attributes_t sec_attr,
			unsigned int nsaid_permissions)
{
	int dmc_inst, sys_if;

	assert(g_driver_data);
	/* Do range checks on regions. */
	assert(region_no >= 0 && region_no <= MAX_REGION_VAL);

	/*
	 * Do address range check based on DMC-TZ configuration. A 43bit address
	 * is the max and expected case.
	 */
	assert(((region_top <= _tzc_get_max_top_addr(43)) &&
		(region_base < region_top)));

	/* region_base and (region_top + 1) must be 4KB aligned */
	assert(((region_base | (region_top + 1)) & (4096 - 1)) == 0);

	for (dmc_inst = 0; dmc_inst < g_driver_data->dmc_count; dmc_inst++) {
		assert(DMC_INST_BASE_ADDR(dmc_inst));
		for (sys_if = 0; sys_if < MAX_SYS_IF_COUNT; sys_if++)
			_tzc_dmc500_configure_region(
					DMC_INST_SI_BASE(dmc_inst, sys_if),
					TZC_DMC500_REGION_ATTR_F_EN_MASK,
					region_no, region_base, region_top,
					sec_attr, nsaid_permissions);
	}

	g_conf_regions[region_no].sec_attr = sec_attr;
	g_conf_regions[region_no].is_enabled = 1;
}

/* Sets the action value for all the DMC instances */
void tzc_dmc500_set_action(tzc_action_t action)
{
	int dmc_inst;

	assert(g_driver_data);

	for (dmc_inst = 0; dmc_inst < g_driver_data->dmc_count; dmc_inst++) {
		assert(DMC_INST_BASE_ADDR(dmc_inst));
		/*
		 * - Currently no handler is provided to trap an error via
		 *   interrupt or exception.
		 * - The interrupt action has not been tested.
		 */
		_tzc_dmc500_write_action(DMC_INST_BASE_ADDR(dmc_inst), action);
	}
}

/*
 * A DMC-500 instance must be present at each base address provided by the
 * platform. It also expects platform to pass at least one instance of
 * DMC-500.
 */
static void validate_plat_driver_data(
			const tzc_dmc500_driver_data_t *plat_driver_data)
{
#if ENABLE_ASSERTIONS
	int i;
	unsigned int dmc_id;
	uintptr_t dmc_base;

	assert(plat_driver_data);
	assert(plat_driver_data->dmc_count > 0 &&
		(plat_driver_data->dmc_count <= MAX_DMC_COUNT));

	for (i = 0; i < plat_driver_data->dmc_count; i++) {
		dmc_base = plat_driver_data->dmc_base[i];
		assert(dmc_base);

		dmc_id = _tzc_read_peripheral_id(dmc_base);
		assert(dmc_id == DMC500_PERIPHERAL_ID);
	}
#endif /* ENABLE_ASSERTIONS */
}


/*
 * Initializes the base address and count of DMC instances.
 *
 * Note : Only pointer to plat_driver_data is saved, so it is caller's
 * responsibility to keep it valid until the driver is used.
 */
void tzc_dmc500_driver_init(const tzc_dmc500_driver_data_t *plat_driver_data)
{
	/* Check valid pointer is passed */
	assert(plat_driver_data);

	/*
	 * NOTE: This driver expects the DMC-500 controller is already in
	 * READY state. Hence, it uses the reconfiguration method for
	 * programming TrustZone regions
	 */
	/* Validates the information passed by platform */
	validate_plat_driver_data(plat_driver_data);
	g_driver_data = plat_driver_data;
}

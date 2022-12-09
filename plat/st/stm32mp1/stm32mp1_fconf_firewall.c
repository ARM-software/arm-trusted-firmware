/*
 * Copyright (c) 2021-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/arm/tzc400.h>
#include <drivers/clk.h>
#include <dt-bindings/clock/stm32mp1-clks.h>
#include <lib/fconf/fconf.h>
#include <lib/object_pool.h>
#include <libfdt.h>
#include <tools_share/firmware_image_package.h>

#include <platform_def.h>
#include <stm32mp_fconf_getter.h>

#define STM32MP_REGION_PARAMS	4
#define STM32MP_MAX_REGIONS	8
#define FORCE_SEC_REGION	BIT(31)

static uint32_t nb_regions;

struct dt_id_attr {
	fdt32_t id_attr[STM32MP_MAX_REGIONS];
};

void stm32mp1_arch_security_setup(void)
{
#if STM32MP13
	clk_enable(TZC);
#endif
#if STM32MP15
	clk_enable(TZC1);
	clk_enable(TZC2);
#endif

	tzc400_init(STM32MP1_TZC_BASE);
	tzc400_disable_filters();

	/*
	 * Region 0 set to cover all DRAM at 0xC000_0000
	 * Only secure access is granted in read/write.
	 */
	tzc400_configure_region0(TZC_REGION_S_RDWR, 0);

	tzc400_set_action(TZC_ACTION_ERR);
	tzc400_enable_filters();
}

void stm32mp1_security_setup(void)
{
	uint8_t i;

	assert(nb_regions > 0U);

	tzc400_init(STM32MP1_TZC_BASE);
	tzc400_disable_filters();

	/*
	 * Region 0 set to cover all DRAM at 0xC000_0000
	 * No access is allowed.
	 */
	tzc400_configure_region0(TZC_REGION_S_NONE, 0);

	for (i = 1U; i <= nb_regions; i++) {
		tzc400_update_filters(i, STM32MP1_FILTER_BIT_ALL);
	}

	tzc400_set_action(TZC_ACTION_INT);
	tzc400_enable_filters();
}

static int fconf_populate_stm32mp1_firewall(uintptr_t config)
{
	int node, len;
	unsigned int i;
	const struct dt_id_attr *conf_list;
	const void *dtb = (const void *)config;

	/* Assert the node offset point to "st,mem-firewall" compatible property */
	const char *compatible_str = "st,mem-firewall";

	node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);
	if (node < 0) {
		ERROR("FCONF: Can't find %s compatible in dtb\n", compatible_str);
		return node;
	}

	conf_list = (const struct dt_id_attr *)fdt_getprop(dtb, node, "memory-ranges", &len);
	if (conf_list == NULL) {
		WARN("FCONF: Read cell failed for %s\n", "memory-ranges");
		return -1;
	}

	/* Locate the memory cells and read all values */
	for (i = 0U; i < (unsigned int)(len / (sizeof(uint32_t) * STM32MP_REGION_PARAMS)); i++) {
		uint32_t idx = i * STM32MP_REGION_PARAMS;
		uint32_t base;
		uint32_t size;
		uint32_t sec_attr;
		uint32_t nsaid;

		base = fdt32_to_cpu(conf_list->id_attr[idx]);
		size = fdt32_to_cpu(conf_list->id_attr[idx + 1]);
		sec_attr = fdt32_to_cpu(conf_list->id_attr[idx + 2]);
		nsaid = fdt32_to_cpu(conf_list->id_attr[idx + 3]);

		VERBOSE("FCONF: stm32mp1-firewall cell found with value = 0x%x 0x%x 0x%x 0x%x\n",
			base, size, sec_attr, nsaid);

		nb_regions++;

		/* Configure region but keep disabled for secure access for BL2 load */
		tzc400_configure_region(0U, nb_regions, (unsigned long long)base,
					(unsigned long long)base + size - 1ULL, sec_attr, nsaid);
	}

	/* Force flush as the value will be used cache off */
	flush_dcache_range((uintptr_t)&nb_regions, sizeof(uint32_t));

	return 0;
}

FCONF_REGISTER_POPULATOR(FW_CONFIG, stm32mp1_firewall, fconf_populate_stm32mp1_firewall);

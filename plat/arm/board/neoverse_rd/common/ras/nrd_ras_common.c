/*
 * Copyright (c) 2018-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <bl31/interrupt_mgmt.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include <nrd_ras.h>

static struct plat_nrd_ras_config *nrd_ras_config;

/*
 * Find event map for a given interrupt number. On success, returns pointer to
 * the event map. On error, returns NULL.
 */
struct nrd_ras_ev_map *nrd_find_ras_event_map_by_intr(uint32_t intr_num)
{
	struct nrd_ras_ev_map *map;
	int size;
	int i;

	if (nrd_ras_config == NULL) {
		ERROR("RAS config is NULL\n");
		return NULL;
	}

	map = nrd_ras_config->ev_map;
	size = nrd_ras_config->ev_map_size;

	for (i = 0; i < size; i++) {
		if (map->intr == intr_num)
			return map;

		map++;
	}

	return NULL;
}

/*
 * Programs GIC registers and configures interrupt ID's as Group0 EL3
 * interrupts. Current support is to register PPI and SPI interrupts.
 */
static void nrd_ras_intr_configure(int intr, int intr_type)
{
	plat_ic_set_interrupt_type(intr, INTR_TYPE_EL3);
	plat_ic_set_interrupt_priority(intr, PLAT_RAS_PRI);
	plat_ic_clear_interrupt_pending(intr);

	/* Routing mode option available only for SPI interrupts */
	if (intr_type == NRD_RAS_INTR_TYPE_SPI) {
		plat_ic_set_spi_routing(intr, INTR_ROUTING_MODE_ANY,
					(u_register_t)read_mpidr_el1());
	}
	plat_ic_enable_interrupt(intr);
}

/*
 * Initialization function for the framework.
 *
 * Registers RAS config provided by the platform and then configures and
 * enables interrupt for each registered error. On success, return 0.
 */
int nrd_ras_platform_setup(struct plat_nrd_ras_config *config)
{
	struct nrd_ras_ev_map *map;
	int size;
	int i;

	/* Check if parameter is valid. */
	if (config == NULL) {
		ERROR("NRD: Failed to register RAS config\n");
		return -1;
	}

	/*
	 * Maintain a reference to the platform RAS config data for later
	 * use.
	 */
	nrd_ras_config = config;

	map = nrd_ras_config->ev_map;
	size = nrd_ras_config->ev_map_size;

	for (i = 0; i < size; i++) {
		nrd_ras_intr_configure(map->intr, map->intr_type);
		map++;
	}

	INFO("NRD: Platform RAS setup successful\n");

	return 0;
}

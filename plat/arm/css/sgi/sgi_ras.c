/*
 * Copyright (c) 2018-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <bl31/interrupt_mgmt.h>
#include <lib/extensions/ras.h>
#include <plat/common/platform.h>

#include <sgi_ras.h>

struct err_record_info sgi_err_records[] = {

};

struct ras_interrupt sgi_ras_interrupts[] = {

};

REGISTER_ERR_RECORD_INFO(sgi_err_records);
REGISTER_RAS_INTERRUPTS(sgi_ras_interrupts);

static struct sgi_ras_ev_map *plat_sgi_get_ras_ev_map(void)
{
	return NULL;
}

static int plat_sgi_get_ras_ev_map_size(void)
{
	return 0;
}

static void sgi_ras_intr_configure(int intr)
{
	plat_ic_set_interrupt_type(intr, INTR_TYPE_EL3);
	plat_ic_set_interrupt_priority(intr, PLAT_RAS_PRI);
	plat_ic_clear_interrupt_pending(intr);
	plat_ic_set_spi_routing(intr, INTR_ROUTING_MODE_ANY,
				(u_register_t)read_mpidr_el1());
	plat_ic_enable_interrupt(intr);
}

int sgi_ras_intr_handler_setup(void)
{
	int i;
	struct sgi_ras_ev_map *map = plat_sgi_get_ras_ev_map();
	int size = plat_sgi_get_ras_ev_map_size();

	for (i = 0; i < size; i++) {
		sgi_ras_intr_configure(map->intr);
		map++;
	}

	INFO("SGI: RAS Interrupt Handler successfully registered\n");

	return 0;
}

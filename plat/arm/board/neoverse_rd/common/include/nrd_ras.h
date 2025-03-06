/*
 * Copyright (c) 2018-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NRD_RAS_H
#define NRD_RAS_H

#include <lib/extensions/ras.h>
#include <plat/common/platform.h>

/*
 * Interrupt type supported.
 * - NRD_RAS_INTR_TYPE_SPI: Denotes a SPI interrupt
 * - NRD_RAS_INTR_TYPE_PPI: Denotes a PPI interrupt
 */
#define NRD_RAS_INTR_TYPE_SPI 0
#define NRD_RAS_INTR_TYPE_PPI 1

/* RAS error info data structure. */
struct nrd_ras_ev_map {
	int sdei_ev_num;	/* SDEI Event number */
	int intr;		/* Physical intr number */
	int intr_type;          /* Interrupt Type (SPI or PPI)*/
};

/* RAS config data structure. Must be defined by each platform. */
struct plat_nrd_ras_config {
	struct nrd_ras_ev_map *ev_map;
	int ev_map_size;
};

/*
 * Find event map for a given interrupt number. On success, returns pointer
 * to the event map. On error, returns NULL.
 */
struct nrd_ras_ev_map *nrd_find_ras_event_map_by_intr(uint32_t intr_num);

/*
 * Initialization function for the framework.
 *
 * Registers RAS config provided by the platform and then configures and
 * enables interrupt for each registered error. On success, return 0.
 */
int nrd_ras_platform_setup(struct plat_nrd_ras_config *config);

/* Base element RAM RAS interrupt handler function. */
int nrd_ras_sram_intr_handler(const struct err_record_info *err_rec,
				int probe_data,
				const struct err_handler_data *const data);

/* CPU RAS interrupt handler */
int nrd_ras_cpu_intr_handler(const struct err_record_info *err_rec,
				int probe_data,
				const struct err_handler_data *const data);

#endif /* NRD_RAS_H */

/*
 * Copyright (c) 2018-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGI_RAS_H
#define SGI_RAS_H

#include <lib/extensions/ras.h>
#include <plat/common/platform.h>

/*
 * Interrupt type supported.
 * - SGI_RAS_INTR_TYPE_SPI: Denotes a SPI interrupt
 * - SGI_RAS_INTR_TYPE_PPI: Denotes a PPI interrupt
 */
#define SGI_RAS_INTR_TYPE_SPI 0
#define SGI_RAS_INTR_TYPE_PPI 1

/*
 * MM Communicate information structure. Required to generate MM Communicate
 * payload to be shared with Standalone MM.
 */
typedef struct mm_communicate_header {
	struct efi_guid	header_guid;
	size_t		message_len;
	uint8_t		data[1];
} mm_communicate_header_t;

/* RAS error info data structure. */
struct sgi_ras_ev_map {
	int sdei_ev_num;	/* SDEI Event number */
	int intr;		/* Physical intr number */
	int intr_type;          /* Interrupt Type (SPI or PPI)*/
};

/* RAS config data structure. Must be defined by each platform. */
struct plat_sgi_ras_config {
	struct sgi_ras_ev_map *ev_map;
	int ev_map_size;
};

/*
 * Find event map for a given interrupt number. On success, returns pointer
 * to the event map. On error, returns NULL.
 */
struct sgi_ras_ev_map *sgi_find_ras_event_map_by_intr(uint32_t intr_num);

/*
 * Initialization function for the framework.
 *
 * Registers RAS config provided by the platform and then configures and
 * enables interrupt for each registered error. On success, return 0.
 */
int sgi_ras_platform_setup(struct plat_sgi_ras_config *config);

/* Base element RAM RAS interrupt handler function. */
int sgi_ras_sram_intr_handler(const struct err_record_info *err_rec,
				int probe_data,
				const struct err_handler_data *const data);

/* CPU RAS interrupt handler */
int sgi_ras_cpu_intr_handler(const struct err_record_info *err_rec,
				int probe_data,
				const struct err_handler_data *const data);

#endif /* SGI_RAS_H */

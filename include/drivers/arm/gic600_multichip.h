/*
 * Copyright (c) 2019, ARM Limited. All rights reserved.
 * Copyright (c) 2023, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GIC600_MULTICHIP_H
#define GIC600_MULTICHIP_H

#include <stdint.h>

/*
 * GIC-600 microarchitecture supports coherent multichip environments containing
 * up to 16 chips.
 */
#define GIC600_MAX_MULTICHIP	16

typedef struct multichip_spi_ids_desc {
	uintptr_t gicd_base;
	uint32_t spi_id_min;
	uint32_t spi_id_max;
} multichip_spi_ids_desc_t;

/*******************************************************************************
 * GIC-600 multichip data structure describes platform specific attributes
 * related to GIC-600 multichip. Platform port is expected to define these
 * attributes to initialize the multichip related registers and create
 * successful connections between the GIC-600s in a multichip system.
 *
 * The 'rt_owner_base' field contains the base address of the GIC Distributor
 * which owns the routing table.
 *
 * The 'rt_owner' field contains the chip number which owns the routing table.
 * Chip number or chip_id starts from 0.
 *
 * The 'chip_count' field contains the total number of chips in a multichip
 * system. This should match the number of entries in 'chip_addrs' and 'spi_ids'
 * fields.
 *
 * The 'chip_addrs' field contains array of chip addresses. These addresses are
 * implementation specific values.
 *
 * The 'multichip_spi_ids_desc_t' field contains array of descriptors used to
 * provide minimum and maximum SPI interrupt ids that each chip owns and the
 * corresponding chip base address. Note that SPI interrupt ids can range from
 * 32 to 960 and it should be group of 32 (i.e., SPI minimum and (SPI maximum +
 * 1) should be a multiple of 32). If a chip doesn't own any SPI interrupts a
 * value of {0, 0, 0} should be passed.
 ******************************************************************************/
struct gic600_multichip_data {
	uintptr_t rt_owner_base;
	unsigned int rt_owner;
	unsigned int chip_count;
	uint64_t chip_addrs[GIC600_MAX_MULTICHIP];
	multichip_spi_ids_desc_t spi_ids[GIC600_MAX_MULTICHIP];
};

uintptr_t gic600_multichip_gicd_base_for_spi(uint32_t spi_id);
void gic600_multichip_init(struct gic600_multichip_data *multichip_data);
bool gic600_multichip_is_initialized(void);

#endif /* GIC600_MULTICHIP_H */

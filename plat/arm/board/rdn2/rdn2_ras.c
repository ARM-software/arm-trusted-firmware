/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>
#include <sgi_ras.h>
#include <sgi_sdei.h>

struct sgi_ras_ev_map plat_ras_map[] = {
	/* Non Secure base RAM ECC CE interrupt */
	{SGI_SDEI_DS_EVENT_0, NS_RAM_ECC_CE_INT, SGI_RAS_INTR_TYPE_SPI},

	/* Non Secure base RAM ECC UE interrupt */
	{SGI_SDEI_DS_EVENT_0, NS_RAM_ECC_UE_INT, SGI_RAS_INTR_TYPE_SPI},
};

/* RAS error record list definition, used by the common RAS framework. */
struct err_record_info plat_err_records[] = {
	/* Base element RAM Non-secure error record. */
	ERR_RECORD_MEMMAP_V1(SOC_NS_RAM_ERR_REC_BASE, 4, NULL,
				&sgi_ras_sram_intr_handler, 0),
};

/* RAS error interrupt list definition, used by the common RAS framework. */
struct ras_interrupt plat_ras_interrupts[] = {
	{
		.intr_number = NS_RAM_ECC_CE_INT,
		.err_record = &plat_err_records[0],
	}, {
		.intr_number = NS_RAM_ECC_UE_INT,
		.err_record = &plat_err_records[0],
	},
};

/* Registers the RAS error record list with common RAS framework. */
REGISTER_ERR_RECORD_INFO(plat_err_records);
/* Registers the RAS error interrupt info list with common RAS framework. */
REGISTER_RAS_INTERRUPTS(plat_ras_interrupts);

/* Platform RAS handling config data definition */
struct plat_sgi_ras_config ras_config = {
	plat_ras_map,
	ARRAY_SIZE(plat_ras_map)
};

/*
 * Copyright (c) 2019-2022, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOCFPGA_PRIVATE_H
#define SOCFPGA_PRIVATE_H

#include "socfpga_plat_def.h"

#define EMMC_DESC_SIZE		(1<<20)

#define EMMC_INIT_PARAMS(base, clk)			\
	{	.bus_width = MMC_BUS_WIDTH_4,		\
		.clk_rate = (clk),			\
		.desc_base = (base),			\
		.desc_size = EMMC_DESC_SIZE,		\
		.flags = 0,				\
		.reg_base = SOCFPGA_MMC_REG_BASE	\
	}

typedef enum {
	BOOT_SOURCE_FPGA = 0,
	BOOT_SOURCE_SDMMC,
	BOOT_SOURCE_NAND,
	BOOT_SOURCE_RSVD,
	BOOT_SOURCE_QSPI
} boot_source_type;

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/

void enable_nonsecure_access(void);

void socfpga_io_setup(int boot_source);

void socfgpa_configure_mmu_el3(unsigned long total_base,
			unsigned long total_size,
			unsigned long ro_start,
			unsigned long ro_limit,
			unsigned long coh_start,
			unsigned long coh_limit);


void socfpga_configure_mmu_el1(unsigned long total_base,
			unsigned long total_size,
			unsigned long ro_start,
			unsigned long ro_limit,
			unsigned long coh_start,
			unsigned long coh_limit);

void socfpga_delay_timer_init(void);

void socfpga_gic_driver_init(void);

void socfpga_delay_timer_init_args(void);

uint32_t socfpga_get_spsr_for_bl32_entry(void);

uint32_t socfpga_get_spsr_for_bl33_entry(void);

unsigned long socfpga_get_ns_image_entrypoint(void);

void plat_secondary_cpus_bl31_entry(void);

#endif /* SOCFPGA_PRIVATE_H */

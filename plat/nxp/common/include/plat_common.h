/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLAT_COMMON_H
#define PLAT_COMMON_H

#include <stdbool.h>
#include <lib/el3_runtime/cpu_data.h>
#include <platform_def.h>

/* Structure to define SoC personality */
struct soc_type {
	char name[10];
	uint32_t version;
	uint32_t num_clusters;
	uint32_t cores_per_cluster;
};

#define SOC_ENTRY(n, v, ncl, nc) \
	{ .name = #n, .version = SVR_##v, \
	  .num_clusters = (ncl), \
	  .cores_per_cluster = (nc) }

enum boot_device {
	BOOT_DEVICE_IFC_NOR,
	BOOT_DEVICE_IFC_NAND,
	BOOT_DEVICE_QSPI,
	BOOT_DEVICE_EMMC,
	BOOT_DEVICE_SDHC2_EMMC,
	BOOT_DEVICE_FLEXSPI_NOR,
	BOOT_DEVICE_FLEXSPI_NAND,
	BOOT_DEVICE_NONE
};

uint32_t read_reg_porsr1(void);
uint32_t read_saved_porsr1(void);
enum boot_device get_boot_dev(void);

struct region_info {
	uint64_t addr;
	uint64_t size;
};

struct dram_rgn_info {
	uint64_t num_dram_regions;
	uint64_t total_dram_size;
	struct region_info region[NUM_DRAM_REGIONS];
};


struct dram_rgn_info *get_dram_regions_info(void);
void mmap_add_ddr_region_dynamically(void);
/* DDR Related functions */
#if DDR_INIT
long long _init_ddr(void);
void i2c_init(void);
#endif


/* Interconnect CCI/CCN functions */
void plat_interconnect_enter_coherency(unsigned int num_clusters);
void enable_timer_base_to_cluster(void);
void enable_core_tb(void);

/* IO storage utility functions */
int plat_io_setup(void);
int open_backend(const uintptr_t spec);
int plat_io_block_setup(size_t fip_offset, uintptr_t block_dev_spec);
int plat_io_memmap_setup(size_t fip_offset);

/* GIC common API's */
void plat_gic_driver_init(void);
void plat_gic_init(void);
void plat_gic_cpuif_enable(void);
void plat_gic_cpuif_disable(void);
void plat_gic_redistif_on(void);
void plat_gic_redistif_off(void);
void plat_gic_pcpu_init(void);
/* GIC utility functions */
void get_gic_offset(uint32_t *gicc_base, uint32_t *gicd_base);


void plat_setup_page_tables(uintptr_t total_base,
			size_t total_size,
			uintptr_t code_start,
			uintptr_t code_limit,
			uintptr_t rodata_start,
			uintptr_t rodata_limit
#if USE_COHERENT_MEM
			, uintptr_t coh_start,
			uintptr_t coh_limit
#endif
);

/* Security utility functions */
int tzc380_setup(void);

/* Timer utility functions */
uint64_t get_timer_val(uint64_t start);
void delay_timer_init(void);
void set_base_freq_CNTFID0(void);

int load_img(unsigned int image_id, uintptr_t *image_base,
		      uint32_t *image_size);



unsigned int plat_my_core_mask(void);
unsigned int plat_core_mask(u_register_t mpidr);
unsigned int plat_my_core_pos(void);
unsigned int plat_core_pos(u_register_t mpidr);

/* SoC specific functions */
void soc_init_start(void);
void soc_init_finish(void);
void soc_init_percpu(void);
void _soc_set_start_addr(u_register_t addr);
void _init_global_data(void);
void _initialize_psci(void);
uint32_t _getCoreState(u_register_t core_mask);
void _setCoreState(u_register_t core_mask, u_register_t core_state);
void soc_early_init(void);
void soc_init(void);
void soc_mem_access(void);
bool check_boot_mode_secure(uint32_t *mode);

int ls_sd_emmc_init(uintptr_t *block_dev_spec);

/*
 * Function to initialize platform's console
 * and register with console framework
 */
void plat_console_init(void);

struct sysinfo {
	unsigned long freq_platform;
	unsigned long freq_ddr_pll0;
	unsigned long freq_ddr_pll1;
};

void get_clocks(struct sysinfo *sys);

uint64_t bl31_get_porsr1(void);
uint64_t el2_2_aarch32(u_register_t smc_id, u_register_t start_addr,
			u_register_t parm1, u_register_t parm2);

uint64_t prefetch_disable(u_register_t smc_id, u_register_t mask);

#ifdef TEST_BL31
uint64_t _get_test_entry(void);
#endif

#endif /* PLAT_COMMON_H */

/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLAT_COMMON_H
#define PLAT_COMMON_H

#include <stdbool.h>

#include <lib/el3_runtime/cpu_data.h>
#include <platform_def.h>

#ifdef IMAGE_BL31

#define BL31_END (uintptr_t)(&__BL31_END__)

/*******************************************************************************
 * This structure represents the superset of information that can be passed to
 * BL31 e.g. while passing control to it from BL2. The BL32 parameters will be
 * populated only if BL2 detects its presence. A pointer to a structure of this
 * type should be passed in X0 to BL31's cold boot entrypoint.
 *
 * Use of this structure and the X0 parameter is not mandatory: the BL31
 * platform code can use other mechanisms to provide the necessary information
 * about BL32 and BL33 to the common and SPD code.
 *
 * BL31 image information is mandatory if this structure is used. If either of
 * the optional BL32 and BL33 image information is not provided, this is
 * indicated by the respective image_info pointers being zero.
 ******************************************************************************/
typedef struct bl31_params {
	param_header_t h;
	image_info_t *bl31_image_info;
	entry_point_info_t *bl32_ep_info;
	image_info_t *bl32_image_info;
	entry_point_info_t *bl33_ep_info;
	image_info_t *bl33_image_info;
} bl31_params_t;

/* BL3 utility functions */
void ls_bl31_early_platform_setup(void *from_bl2,
				void *plat_params_from_bl2);
/* LS Helper functions	*/
unsigned int plat_my_core_mask(void);
unsigned int plat_core_mask(u_register_t mpidr);
unsigned int plat_core_pos(u_register_t mpidr);
//unsigned int plat_my_core_pos(void);

/* BL31 Data API(s) */
void _init_global_data(void);
void _initialize_psci(void);
uint32_t _getCoreState(u_register_t core_mask);
void _setCoreState(u_register_t core_mask, u_register_t core_state);

/* SoC defined structure and API(s) */
void soc_runtime_setup(void);
void soc_init(void);
void soc_platform_setup(void);
void soc_early_platform_setup2(void);
#endif /* IMAGE_BL31 */

#ifdef IMAGE_BL2
void soc_early_init(void);
void soc_mem_access(void);
void soc_preload_setup(void);
void soc_bl2_prepare_exit(void);

/* IO storage utility functions */
int plat_io_setup(void);
int open_backend(const uintptr_t spec);

void ls_bl2_plat_arch_setup(void);
void ls_bl2_el3_plat_arch_setup(void);

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

enum boot_device get_boot_dev(void);

/* DDR Related functions */
#if DDR_INIT
#ifdef NXP_WARM_BOOT
long long init_ddr(uint32_t wrm_bt_flg);
#else
long long init_ddr(void);
#endif
#endif

/* Board specific weak functions */
bool board_enable_povdd(void);
bool board_disable_povdd(void);

void mmap_add_ddr_region_dynamically(void);
#endif /* IMAGE_BL2 */

typedef struct {
	uint64_t addr;
	uint64_t size;
} region_info_t;

typedef struct {
	uint64_t num_dram_regions;
	uint64_t total_dram_size;
	region_info_t region[NUM_DRAM_REGIONS];
} dram_regions_info_t;

dram_regions_info_t *get_dram_regions_info(void);

void ls_setup_page_tables(uintptr_t total_base,
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


/* Structure to define SoC personality */
struct soc_type {
	char name[10];
	uint32_t personality;
	uint32_t num_clusters;
	uint32_t cores_per_cluster;
};

#define SOC_ENTRY(n, v, ncl, nc) {	\
		.name = #n,		\
		.personality = SVR_##v,	\
		.num_clusters = (ncl),	\
		.cores_per_cluster = (nc)}

#endif /* PLAT_COMMON_H */

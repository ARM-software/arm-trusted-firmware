/*
 * Copyright (c) 2022-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RMM_CORE_MANIFEST_H
#define RMM_CORE_MANIFEST_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <lib/cassert.h>

#define RMMD_MANIFEST_VERSION_MAJOR		U(0)
#define RMMD_MANIFEST_VERSION_MINOR		U(5)

#define RMM_CONSOLE_MAX_NAME_LEN		U(8)

/*
 * Version encoding:
 *	- Bit[31] RES0
 *	- Bits [30:16] Major version
 *	- Bits [15:0] Minor version
 */
#define SET_VERSION(_major, _minor)				\
	((((_major) & 0x7FFF) << 16) | ((_minor) & 0xFFFF))

/* Boot Manifest version */
#define RMMD_MANIFEST_VERSION	SET_VERSION(			\
				RMMD_MANIFEST_VERSION_MAJOR,	\
				RMMD_MANIFEST_VERSION_MINOR)

#define RMMD_GET_MANIFEST_VERSION_MAJOR(_version)		\
	((_version >> 16) & 0x7FFF)

#define RMMD_GET_MANIFEST_VERSION_MINOR(_version)		\
	(_version & 0xFFFF)

#define PCIE_RC_INFO_VERSION_MAJOR		U(0)
#define PCIE_RC_INFO_VERSION_MINOR		U(1)

/* PCIe Root Complex info structure version */
#define PCIE_RC_INFO_VERSION	SET_VERSION(			\
				PCIE_RC_INFO_VERSION_MAJOR,	\
				PCIE_RC_INFO_VERSION_MINOR)

/* Memory bank/device region structure */
struct memory_bank {
	uint64_t base;			/* Base address */
	uint64_t size;			/* Size of memory bank/device region */
};

CASSERT(offsetof(struct memory_bank, base) == 0UL,
			rmm_manifest_base_unaligned);
CASSERT(offsetof(struct memory_bank, size) == 8UL,
			rmm_manifest_size_unaligned);

/* Memory/device region layout info structure */
struct memory_info {
	uint64_t num_banks;		/* Number of memory banks/device regions */
	struct memory_bank *banks;	/* Pointer to memory_bank[] */
	uint64_t checksum;		/* Checksum of memory_info data */
};

CASSERT(offsetof(struct memory_info, num_banks) == 0UL,
			rmm_manifest_num_banks_unaligned);
CASSERT(offsetof(struct memory_info, banks) == 8UL,
			rmm_manifest_dram_data_unaligned);
CASSERT(offsetof(struct memory_info, checksum) == 16UL,
			rmm_manifest_checksum_unaligned);

/* Console info structure */
struct console_info {
	uint64_t base;			/* Console base address */
	uint64_t map_pages;		/* Num of pages to be mapped in RMM for the console MMIO */
	char name[RMM_CONSOLE_MAX_NAME_LEN];	/* Name of console */
	uint64_t clk_in_hz;		/* UART clock (in Hz) for the console */
	uint64_t baud_rate;		/* Baud rate */
	uint64_t flags;			/* Additional flags RES0 */
};

CASSERT(offsetof(struct console_info, base) == 0UL,
			rmm_manifest_console_base_unaligned);
CASSERT(offsetof(struct console_info, map_pages) == 8UL,
			rmm_manifest_console_map_pages_unaligned);
CASSERT(offsetof(struct console_info, name) == 16UL,
			rmm_manifest_console_name_unaligned);
CASSERT(offsetof(struct console_info, clk_in_hz) == 24UL,
			rmm_manifest_console_clk_in_hz_unaligned);
CASSERT(offsetof(struct console_info, baud_rate) == 32UL,
			rmm_manifest_console_baud_rate_unaligned);
CASSERT(offsetof(struct console_info, flags) == 40UL,
			rmm_manifest_console_flags_unaligned);

struct console_list {
	uint64_t num_consoles;		/* Number of consoles */
	struct console_info *consoles;	/* Pointer to console_info[] */
	uint64_t checksum;		/* Checksum of console_list data */
};

CASSERT(offsetof(struct console_list, num_consoles) == 0UL,
			rmm_manifest_num_consoles);
CASSERT(offsetof(struct console_list, consoles) == 8UL,
			rmm_manifest_consoles);
CASSERT(offsetof(struct console_list, checksum) == 16UL,
			rmm_manifest_console_list_checksum);

/* SMMUv3 Info structure */
struct smmu_info {
	uint64_t smmu_base;		/* SMMUv3 base address */
	uint64_t smmu_r_base;		/* SMMUv3 Realm Pages base address */
};

CASSERT(offsetof(struct smmu_info, smmu_base) == 0UL,
			rmm_manifest_smmu_base);
CASSERT(offsetof(struct smmu_info, smmu_r_base) == 8UL,
			rmm_manifest_smmu_r_base);

/* SMMUv3 Info List structure */
struct smmu_list {
	uint64_t num_smmus;		/* Number of smmu_info entries */
	struct smmu_info *smmus;	/* Pointer to smmu_info[] array */
	uint64_t checksum;		/* Checksum of smmu_list data */
};

CASSERT(offsetof(struct smmu_list, num_smmus) == 0UL,
			rmm_manifest_num_smmus);
CASSERT(offsetof(struct smmu_list, smmus) == 8UL,
			rmm_manifest_smmus);
CASSERT(offsetof(struct smmu_list, checksum) == 16UL,
			rmm_manifest_smmu_list_checksum);

/* PCIe BDF Mapping Info structure */
struct bdf_mapping_info {
	uint16_t mapping_base;	/* Base of BDF mapping (inclusive) */
	uint16_t mapping_top;	/* Top of BDF mapping (exclusive) */
	uint16_t mapping_off;	/* Mapping offset, as per Arm Base System Architecture: */
				/* StreamID = zero_extend(RequesterID[N-1:0]) + (1<<N)*Constant_B */
	uint16_t smmu_idx;	/* SMMU index in smmu_info[] array */
};

CASSERT(offsetof(struct bdf_mapping_info, mapping_base) == 0UL,
			rmm_manifest_mapping_base);
CASSERT(offsetof(struct bdf_mapping_info, mapping_top) == 2UL,
			rmm_manifest_mapping_top);
CASSERT(offsetof(struct bdf_mapping_info, mapping_off) == 4UL,
			rmm_manifest_mapping_off);
CASSERT(offsetof(struct bdf_mapping_info, smmu_idx) == 6UL,
			rmm_manifest_smmu_ptr);

/* PCIe Root Port Info structure */
struct root_port_info {
	uint16_t root_port_id;			/* Root Port identifier */
	uint16_t padding;			/* RES0 */
	uint32_t num_bdf_mappings;		/* Number of BDF mappings */
	struct bdf_mapping_info *bdf_mappings;	/* Pointer to bdf_mapping_info[] array */
};

CASSERT(offsetof(struct root_port_info, root_port_id) == 0UL,
			rmm_manifest_root_port_id);
CASSERT(offsetof(struct root_port_info, num_bdf_mappings) == 4UL,
			rmm_manifest_num_bdf_mappingss);
CASSERT(offsetof(struct root_port_info, bdf_mappings) == 8UL,
			rmm_manifest_bdf_mappings);

/* PCIe Root Complex info structure v0.1 */
struct root_complex_info {
	uint64_t ecam_base;			/* ECAM base address. Size is implicitly 256MB */
	uint8_t segment;			/* PCIe segment identifier */
	uint8_t padding[3];			/* RES0 */
	uint32_t num_root_ports;		/* Number of root ports */
	struct root_port_info *root_ports;	/* Pointer to root_port_info[] array */
};

CASSERT(offsetof(struct root_complex_info, ecam_base) == 0UL,
			rmm_manifest_ecam_base);
CASSERT(offsetof(struct root_complex_info, segment) == 8UL,
			rmm_manifest_segment);
CASSERT(offsetof(struct root_complex_info, num_root_ports) == 12UL,
			rmm_manifest_num_root_ports);
CASSERT(offsetof(struct root_complex_info, root_ports) == 16UL,
			rmm_manifest_root_ports);

/* PCIe Root Complex List structure */
struct root_complex_list {
	uint64_t num_root_complex;		/* Number of pci_rc_info entries */
	uint32_t rc_info_version;		/* PCIe Root Complex info structure version */
	uint32_t padding;			/* RES0 */
	struct root_complex_info *root_complex;	/* Pointer to pci_rc_info[] array */
	uint64_t checksum;			/* Checksum of pci_rc_list data */
};

CASSERT(offsetof(struct root_complex_list, num_root_complex) == 0UL,
			rmm_manifest_num_root_complex);
CASSERT(offsetof(struct root_complex_list, rc_info_version) == 8UL,
			rmm_manifest_rc_info_version);
CASSERT(offsetof(struct root_complex_list, root_complex) == 16UL,
			rmm_manifest_root_complex);
CASSERT(offsetof(struct root_complex_list, checksum) == 24UL,
			rmm_manifest_root_complex_list_checksum);

/* Boot manifest core structure as per v0.5 */
struct rmm_manifest {
	uint32_t version;			/* Manifest version */
	uint32_t padding;			/* RES0 */
	uint64_t plat_data;			/* Manifest platform data */
	/* Platform NS DRAM data (v0.2) */
	struct memory_info plat_dram;
	/* Platform console list (v0.3) */
	struct console_list plat_console;
	/* Platform device address ranges (v0.4) */
	struct memory_info plat_ncoh_region;
	struct memory_info plat_coh_region;
	/* Platform SMMUv3 list (v0.5) */
	struct smmu_list plat_smmu;
	/* Platform PCIe Root Complex list (v0.5) */
	struct root_complex_list plat_root_complex;
};

CASSERT(offsetof(struct rmm_manifest, version) == 0UL,
			rmm_manifest_version_unaligned);
CASSERT(offsetof(struct rmm_manifest, plat_data) == 8UL,
			rmm_manifest_plat_data_unaligned);
CASSERT(offsetof(struct rmm_manifest, plat_dram) == 16UL,
			rmm_manifest_plat_dram_unaligned);
CASSERT(offsetof(struct rmm_manifest, plat_console) == 40UL,
			rmm_manifest_plat_console_unaligned);
CASSERT(offsetof(struct rmm_manifest, plat_ncoh_region) == 64UL,
			rmm_manifest_plat_ncoh_region_unaligned);
CASSERT(offsetof(struct rmm_manifest, plat_coh_region) == 88UL,
			rmm_manifest_plat_coh_region_unaligned);
CASSERT(offsetof(struct rmm_manifest, plat_smmu) == 112UL,
			rmm_manifest_plat_smmu_unaligned);
CASSERT(offsetof(struct rmm_manifest, plat_root_complex) == 136UL,
			rmm_manifest_plat_root_complex);

#endif /* RMM_CORE_MANIFEST_H */

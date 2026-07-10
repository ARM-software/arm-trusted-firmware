/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * SMEM HW_SW_BUILD_ID (socinfo) layout, populated by boot firmware.
 */

#ifndef PLATFORMINFODEFS_H
#define PLATFORMINFODEFS_H

#include <stddef.h>
#include <stdint.h>

/*
 * Generate a platform version number.
 */
#define PLATFORMINFO_VERSION(major, minor) (((major) << 16) | (minor))

/*
 * SMEM format version numbers. Each version adds fields at the end of
 * struct platforminfo_smem; only access fields whose version is <=
 * smem->format.
 *
 * PLATFORMINFO_SMEM_SIZE_Vn is the minimum SMEM item size needed to safely
 * read every field through that version. Check both format and size before
 * reading a version-gated field.
 */
#define PLATFORMINFO_FORMAT_VER_1	1U	/* chip_id, chip_version */
#define PLATFORMINFO_SMEM_SIZE_V1 \
	(offsetof(struct platforminfo_smem, chip_version) + sizeof(uint32_t))

#define PLATFORMINFO_FORMAT_VER_12	12U	/* chip_family, raw_device_family/number */
#define PLATFORMINFO_SMEM_SIZE_V12 \
	(offsetof(struct platforminfo_smem, raw_device_number) + sizeof(uint32_t))

/*
 * Length of the build ID buffer in platforminfo_smem_t.
 */
#define PLATFORMINFO_SMEM_BUILD_ID_LENGTH 32

/*
 * Length of the chip ID buffer in platforminfo_smem_t
 */
#define PLATFORMINFO_CHIP_NAME_MAX_LENGTH 32

/*
 * Maximum number of PMIC devices in platforminfo_smem_t.
 */
#define PLATFORMINFO_SMEM_MAX_PMIC_DEVICES 3

/*
 * MEM structure for PMIC information.
 */
struct platforminfo_smem_pmic {
	/* PMIC device model type, for Badger matches the revision id subtype */
	uint32_t pmic_model;
	uint32_t pmic_version; /* PMIC version, same format as Platform version */
};

/*
 * Structure for the shared memory location which is used to store
 * platform, chip, build, and pmic information.
 */
struct platforminfo_smem {
	uint32_t format; /* Format of the structure. */
	uint32_t chip_id; /* Chip ID. */
	uint32_t chip_version; /* Chip version. */
	char build_id[PLATFORMINFO_SMEM_BUILD_ID_LENGTH]; /* Build ID. */
	uint32_t raw_chip_id; /* Raw chip ID. */
	uint32_t raw_chip_version; /* Raw chip version. */
	uint32_t platform_type; /* Platform type. */
	uint32_t platform_version; /* Platform version. */
	uint32_t fusion; /* TRUE if Fusion; FALSE otherwise. */
	uint32_t platform_subtype; /* Platform subtype. */
	struct platforminfo_smem_pmic pmic_info[PLATFORMINFO_SMEM_MAX_PMIC_DEVICES];
	/* DEPRECATED. May not contain the full list. Use array at pmic_array_offset instead. */
	uint32_t foundry_id; /* Chip foundry ID. */
	uint32_t chip_serial; /* Chip serial number. */
	uint32_t num_pmics; /* Number of PMICs in array. */
	uint32_t pmic_array_offset; /* Offset to array of platforminfo_smem_pmic */
	uint32_t chip_family; /* Chip family. */
	uint32_t raw_device_family; /* Raw device family. */
	uint32_t raw_device_number; /* Raw device number. */
	uint32_t qfprom_chip_id; /* QFPROM Chip ID   */
	char chip_id_str[PLATFORMINFO_CHIP_NAME_MAX_LENGTH]; /* Chip name */
	uint32_t num_clusters; /* Number of clusters used by chipinfo_get_disabled_cpus API */
	uint32_t cluster_array_offset; /* Offset to uint32 array of disabled CPU clusters */
	uint32_t num_parts; /* Number of parts supported by chipinfo_get_disabled_features API */
	uint32_t disabled_features_array_offset; /* Offset to uint32 array of disabled parts */
	uint32_t modem_supported; /* 0 if not supported, nonzero if supported */
	uint32_t feature_code; /* Feature Code  enum for this device */
	uint32_t p_code; /* PCode for this device */
	uint32_t part_name_map_offset; /* Offset of the part name strings for the current chip */
	uint32_t num_part_name_mappings; /* Number of part name strings. */
	uint32_t oem_variant_id; /* OEM Variant ID */
	uint32_t num_kvps; /* Number of KVPS */
	uint32_t kvps_offset; /* Offset of the KVPS */
	uint32_t num_functional_clusters; /* Number of clusters with >=1 enabled core */
	uint32_t boot_cluster; /* Boot cluster index */
	uint32_t boot_core; /* Boot core index */
	uint32_t raw_package_type; /* Raw package type. */
	uint32_t partial_features_array_offset; /* Offset to uint32 array of partial features */
	uint32_t cpu_cores_array_offset; /* Offset to uint32 array of CPU cores per cluster */
	uint32_t anPartInstancesOffset; /* Offset to array of part-instance structures */
	uint32_t nNumPartInstances; /* Length of the part-instance array */
};

#endif /* PLATFORMINFODEFS_H */

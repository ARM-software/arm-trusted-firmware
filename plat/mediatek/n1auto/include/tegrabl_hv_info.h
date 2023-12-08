/*
 * Copyright (c) 2022, NVIDIA Corporation.  All Rights Reserved.
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

/**
 * @file tegrabl_hv_info.h
 */

#ifndef INCLUDED_TEGRABL_HV_INFO_H
#define INCLUDED_TEGRABL_HV_INFO_H

#define TEGRABL_PACKED(var) var __attribute__((packed))
#define TEGRABL_DECLARE_ALIGNED(var, size) var __attribute__((aligned(size)))

#define MAX_BL_PT_ENTRIES 250U

#define BL_PARTITION_NAME_LENGTH  24U

#define UID_SIZE_BYTES 16U

#define HV_INFO_PAGE_OFFSET		(40U * 1024U)

#define HV_INFO_BASE_PHY   0x90035000U
#define HV_INFO_SIZE       20480U

/**
 * Defines the partition type, an enumeration to identify the type of partition.
 */
typedef enum {
	/*
	 *********************** NOTE *************************
	 * New qb_partition_type enum shall be added at the
	 * end (but before QB_PARTITION_TYPE_FORCE32) only.
	 * Enums should NOT be added in the middle in order to
	 * maintain backward compatibility for Libnvupdate.
	 ******************************************************
	 */
	/// Partition Invalid.
	QB_PARTITION_TYPE_INVALID,
	/// Partition BCT - Boot ROM BCT.
	QB_PARTITION_TYPE_BCT = 0x1,
	/// Partition MB1.
	QB_PARTITION_TYPE_MB1,
	/// Partition PT (Partition Table).
	QB_PARTITION_TYPE_PT,
	/// Partition OS (kernel).
	QB_PARTITION_TYPE_OS,
	/// Partition OS_DTB (kernel DTB).
	QB_PARTITION_TYPE_OS_DTB,
	/// Partition NV_DATA.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_NV_DATA,
	/// Partition DATA.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_DATA,
	/// Partition MBR.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_MBR,
	/// Partition EBR.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_EBR,
	/// Partition GP1.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_GP1,
	/// Partition GPT.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_GPT,
	/// Partition BL_STAGE2.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_BL_STAGE2,
	/// Partition FUSE_BYPASS.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_FUSE_BYPASS,
	/// Partition CONFIG_TABLE.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_CONFIG_TABLE,
	/// Partition WB0.
	QB_PARTITION_TYPE_WB0,
	///This is not used in  Safety Build - Deprecated.
	/// Partition SECURE_OS (TOS).
	QB_PARTITION_TYPE_SECURE_OS,
	/// Partition MB1_BCT.
	QB_PARTITION_TYPE_MB1_BCT,
	/// Partition MEM_BCT.
	QB_PARTITION_TYPE_MEM_BCT,
	/// Partition PE_FW.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_SPE_FW,
	/// Partition MTS_PRE.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_MTS_PRE,
	/// Partition MTS_BPK.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_MTS_BPK,
	/// Partition DRAM_ECC.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_DRAM_ECC,
	/// Partition BLACKLIST_INFO.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_BLACKLIST_INFO,
	/// Partition TSEC_FW.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_TSEC_FW,
	/// Partition MB2.
	QB_PARTITION_TYPE_MB2,
	/// Partition APE_FW.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_APE_FW,
	/// Partition SCE_FW.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_SCE_FW,
	/// Partition CPU_BL.
	QB_PARTITION_TYPE_CPU_BL,
	/// Partition EKS.
	QB_PARTITION_TYPE_EKS,
	/// Partition BPMP_FW.
	QB_PARTITION_TYPE_BPMP_FW,
	/// Partition BPMP_FW_DTB.
	QB_PARTITION_TYPE_BPMP_FW_DTB,
	/// Partition GPH.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_GPH,
	/// Partition RAMDISK.
	QB_PARTITION_TYPE_RAMDISK,
	/// Partition EXTENDED_CAN.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_EXTENDED_CAN,
	/// Partition SPLASH.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_SPLASH,
	/// Partition MTS_MCE.
	QB_PARTITION_TYPE_MTS_MCE,
	/// Partition RCE_FW.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_RCE_FW,
	/// Partition XUSB_FW.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_XUSB_FW,
	/// Partition IST_UCODE.
	QB_PARTITION_TYPE_IST_UCODE,
	/// Partition BPMP_IST.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_BPMP_IST,
	/// Partition IST_CONFIG.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_IST_CONFIG,
	/// Partition DEFAULT - This is default partition type.
	QB_PARTITION_TYPE_DEFAULT,
	/// Partition VBMETA.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_VBMETA,
	/// Partition FSKP.
	///This is not used in  Safety Build - Deprecated.
	QB_PARTITION_TYPE_FSKP,
	/// Partition CPUBL DTB
	QB_PARTITION_TYPE_CPU_BL_DTB = 63,
	/// Partition LIMIT.
	QB_PARTITION_TYPE_FORCE32 = 0x7FFFFFFF,
} qb_partition_type;

/**
 * @brief Defines the partition information data structure.
 * This holds partition specific information.
 */
TEGRABL_PACKED(
typedef struct bl_part_info_rec {
	/** Holds the device ID of device on which partition is located. */
	uint32_t dev_id;

	/** Holds the device instance of the device on which partition is located.
	The valid range for dev_instance depends on the device ID. */
	uint32_t dev_instance;

	/** Holds boot component header partition ID.
	It stores the partition ID of the partition which contains the boot component
	header (BCH) for the given partition. */
	uint32_t bch_part_id;

	/** Holds the partition type. */
	qb_partition_type part_type;

	/** Holds the logical size of the partition.
	Partition Size field shall store the total size (in bytes)
	that is reserved for the partition on storage device. */
	uint64_t num_bytes;

	/** Start Address field for a partition shall store the start address,
	in bytes, relative to the start of its container partition.
	Here, the container partition is:
	-  storage device for partition at first level of partition layout.
	-  boot chain container partition for partition at second level of partition layout.
	-  guest container partition for partition at third level of partition layout. */
	uint64_t start_addr;

	union {
		uint32_t bit_info;
		struct {
			/** Flag to store partition level {1, 2 ,3}*/
			uint32_t part_level:2;

			/** Flag to store partition chain
			{0=No_Chain, 1=Chain_A, 2=Chain_B, 3=Chain_C, 4=Chain_D}*/
			uint32_t part_chain:3;
		};
	};

	/** Reserved for future use. */
	uint32_t reserved[4];
}
) bl_part_info;

/**
 * @brief Defines partition table entry.
 */
typedef struct bl_pt_entry_rec {
	/** Holds the partition names string. */
	char part_name[BL_PARTITION_NAME_LENGTH];

	/** Holds the partition information data. */
	bl_part_info part_info;
} bl_pt_entry;

struct hv_sys_info_type {
	union {
		uint8_t byte_array[HV_INFO_SIZE];

		struct {
			/** PT entries info. */
			bl_pt_entry pt_entries[MAX_BL_PT_ENTRIES];

			/** Number of valid entries in pt_entries. */
			uint32_t num_pt_entries;

			/** ECID SHA512. */
			uint8_t ecid_sha512[64];

			/** CBB NOC frequency */
			int64_t clk_cbb_noc;

			/** AON NOC frequency */
			int64_t clk_aon_noc;

			/** HOST1X frequency */
			int64_t clk_host1x;
		};
	};
};

#endif

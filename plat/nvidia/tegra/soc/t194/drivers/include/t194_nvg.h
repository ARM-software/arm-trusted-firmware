/*
 * Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef T194_NVG_H
#define T194_NVG_H

/**
 * t194_nvg.h - Header for the NVIDIA Generic interface (NVG).
 * Official documentation for this interface is included as part
 * of the T194 TRM.
 */

/**
 * Current version - Major version increments may break backwards
 * compatiblity and binary compatibility. Minor version increments
 * occur when there is only new functionality.
 */
enum {
	TEGRA_NVG_VERSION_MAJOR = 6,
	TEGRA_NVG_VERSION_MINOR = 6
};

typedef enum {
	TEGRA_NVG_CHANNEL_VERSION				= 0,
	TEGRA_NVG_CHANNEL_POWER_PERF				= 1,
	TEGRA_NVG_CHANNEL_POWER_MODES				= 2,
	TEGRA_NVG_CHANNEL_WAKE_TIME				= 3,
	TEGRA_NVG_CHANNEL_CSTATE_INFO				= 4,
	TEGRA_NVG_CHANNEL_CROSSOVER_C6_LOWER_BOUND		= 5,
	TEGRA_NVG_CHANNEL_CROSSOVER_CC6_LOWER_BOUND		= 6,
	TEGRA_NVG_CHANNEL_CROSSOVER_CG7_LOWER_BOUND		= 8,
	TEGRA_NVG_CHANNEL_CSTATE_STAT_QUERY_REQUEST		= 10,
	TEGRA_NVG_CHANNEL_CSTATE_STAT_QUERY_VALUE		= 11,
	TEGRA_NVG_CHANNEL_NUM_CORES				= 20,
	TEGRA_NVG_CHANNEL_UNIQUE_LOGICAL_ID			= 21,
	TEGRA_NVG_CHANNEL_LOGICAL_TO_PHYSICAL_MAPPING		= 22,
	TEGRA_NVG_CHANNEL_LOGICAL_TO_MPIDR			= 23,
	TEGRA_NVG_CHANNEL_SHUTDOWN				= 42,
	TEGRA_NVG_CHANNEL_IS_SC7_ALLOWED			= 43,
	TEGRA_NVG_CHANNEL_ONLINE_CORE				= 44,
	TEGRA_NVG_CHANNEL_CC3_CTRL				= 45,
	TEGRA_NVG_CHANNEL_CCPLEX_CACHE_CONTROL			= 49,
	TEGRA_NVG_CHANNEL_UPDATE_CCPLEX_GSC			= 50,
	TEGRA_NVG_CHANNEL_HSM_ERROR_CTRL			= 53,
	TEGRA_NVG_CHANNEL_SECURITY_CONFIG			= 54,
	TEGRA_NVG_CHANNEL_DEBUG_CONFIG				= 55,
	TEGRA_NVG_CHANNEL_DDA_SNOC_MCF				= 56,
	TEGRA_NVG_CHANNEL_DDA_MCF_ORD1				= 57,
	TEGRA_NVG_CHANNEL_DDA_MCF_ORD2				= 58,
	TEGRA_NVG_CHANNEL_DDA_MCF_ORD3				= 59,
	TEGRA_NVG_CHANNEL_DDA_MCF_ISO				= 60,
	TEGRA_NVG_CHANNEL_DDA_MCF_SISO				= 61,
	TEGRA_NVG_CHANNEL_DDA_MCF_NISO				= 62,
	TEGRA_NVG_CHANNEL_DDA_MCF_NISO_REMOTE			= 63,
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_ISO			= 64,
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_SISO			= 65,
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_NISO			= 66,
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_NISO_REMOTE		= 67,
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_L3FILL			= 68,
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_L3WR			= 69,
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_RSP_L3RD_DMA		= 70,
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_RSP_MCFRD_DMA		= 71,
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_GLOBAL			= 72,
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_LL				= 73,
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_L3D			= 74,
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_FCM_RD			= 75,
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_FCM_WR			= 76,
	TEGRA_NVG_CHANNEL_DDA_SNOC_GLOBAL_CTRL			= 77,
	TEGRA_NVG_CHANNEL_DDA_SNOC_CLIENT_REQ_CTRL		= 78,
	TEGRA_NVG_CHANNEL_DDA_SNOC_CLIENT_REPLENTISH_CTRL	= 79,

	TEGRA_NVG_CHANNEL_LAST_INDEX
} tegra_nvg_channel_id_t;

typedef enum {
	NVG_STAT_QUERY_SC7_ENTRIES		= 1,
	NVG_STAT_QUERY_CC6_ENTRIES		= 6,
	NVG_STAT_QUERY_CG7_ENTRIES		= 7,
	NVG_STAT_QUERY_C6_ENTRIES		= 10,
	NVG_STAT_QUERY_C7_ENTRIES		= 14,
	NVG_STAT_QUERY_SC7_RESIDENCY_SUM	= 32,
	NVG_STAT_QUERY_CC6_RESIDENCY_SUM	= 41,
	NVG_STAT_QUERY_CG7_RESIDENCY_SUM	= 46,
	NVG_STAT_QUERY_C6_RESIDENCY_SUM		= 51,
	NVG_STAT_QUERY_C7_RESIDENCY_SUM		= 56,
	NVG_STAT_QUERY_SC7_ENTRY_TIME_SUM	= 60,
	NVG_STAT_QUERY_CC6_ENTRY_TIME_SUM	= 61,
	NVG_STAT_QUERY_CG7_ENTRY_TIME_SUM	= 62,
	NVG_STAT_QUERY_C6_ENTRY_TIME_SUM	= 63,
	NVG_STAT_QUERY_C7_ENTRY_TIME_SUM	= 64,
	NVG_STAT_QUERY_SC7_EXIT_TIME_SUM	= 70,
	NVG_STAT_QUERY_CC6_EXIT_TIME_SUM	= 71,
	NVG_STAT_QUERY_CG7_EXIT_TIME_SUM	= 72,
	NVG_STAT_QUERY_C6_EXIT_TIME_SUM		= 73,
	NVG_STAT_QUERY_C7_EXIT_TIME_SUM		= 74,
	NVG_STAT_QUERY_SC7_ENTRY_LAST		= 80,
	NVG_STAT_QUERY_CC6_ENTRY_LAST		= 81,
	NVG_STAT_QUERY_CG7_ENTRY_LAST		= 82,
	NVG_STAT_QUERY_C6_ENTRY_LAST		= 83,
	NVG_STAT_QUERY_C7_ENTRY_LAST		= 84,
	NVG_STAT_QUERY_SC7_EXIT_LAST		= 90,
	NVG_STAT_QUERY_CC6_EXIT_LAST		= 91,
	NVG_STAT_QUERY_CG7_EXIT_LAST		= 92,
	NVG_STAT_QUERY_C6_EXIT_LAST		= 93,
	NVG_STAT_QUERY_C7_EXIT_LAST		= 94
} tegra_nvg_stat_query_t;

typedef enum {
	TEGRA_NVG_CORE_C0 = 0,
	TEGRA_NVG_CORE_C1 = 1,
	TEGRA_NVG_CORE_C6 = 6,
	TEGRA_NVG_CORE_C7 = 7,
	TEGRA_NVG_CORE_WARMRSTREQ = 8
} tegra_nvg_core_sleep_state_t;

typedef enum {
	TEGRA_NVG_SHUTDOWN = 0U,
	TEGRA_NVG_REBOOT = 1U
} tegra_nvg_shutdown_reboot_state_t;

typedef enum {
	TEGRA_NVG_CLUSTER_CC0 = 0,
	TEGRA_NVG_CLUSTER_AUTO_CC1 = 1,
	TEGRA_NVG_CLUSTER_CC6 = 6
} tegra_nvg_cluster_sleep_state_t;

typedef enum {
	TEGRA_NVG_CG_CG0 = 0,
	TEGRA_NVG_CG_CG7 = 7
} tegra_nvg_cluster_group_sleep_state_t;

typedef enum {
	TEGRA_NVG_SYSTEM_SC0 = 0,
	TEGRA_NVG_SYSTEM_SC7 = 7,
	TEGRA_NVG_SYSTEM_SC8 = 8
} tegra_nvg_system_sleep_state_t;

// ---------------------------------------------------------------------------
// NVG Data subformats
// ---------------------------------------------------------------------------

typedef union {
	uint64_t flat;
	struct nvg_version_channel_t {
		uint32_t minor_version	: 32;
		uint32_t major_version	: 32;
	} bits;
} nvg_version_data_t;

typedef union {
	uint64_t flat;
	struct nvg_power_perf_channel_t {
		uint32_t perf_per_watt	: 1;
		uint32_t reserved_31_1	: 31;
		uint32_t reserved_63_32	: 32;
	} bits;
} nvg_power_perf_channel_t;

typedef union {
	uint64_t flat;
	struct nvg_power_modes_channel_t {
		uint32_t low_battery	: 1;
		uint32_t reserved_1_1	: 1;
		uint32_t battery_save	: 1;
		uint32_t reserved_31_3	: 29;
		uint32_t reserved_63_32	: 32;
	} bits;
} nvg_power_modes_channel_t;

typedef union nvg_channel_1_data_u {
	uint64_t flat;
	struct nvg_channel_1_data_s {
		uint32_t perf_per_watt_mode	: 1;
		uint32_t reserved_31_1		: 31;
		uint32_t reserved_63_32		: 32;
	} bits;
} nvg_channel_1_data_t;

typedef union {
	uint64_t flat;
	struct nvg_ccplex_cache_control_channel_t {
		uint32_t gpu_ways	: 5;
		uint32_t reserved_7_5	: 3;
		uint32_t gpu_only_ways	: 5;
		uint32_t reserved_31_13	: 19;
		uint32_t reserved_63_32	: 32;
	} bits;
} nvg_ccplex_cache_control_channel_t;

typedef union nvg_channel_2_data_u {
	uint64_t flat;
	struct nvg_channel_2_data_s {
		uint32_t reserved_1_0		: 2;
		uint32_t battery_saver_mode	: 1;
		uint32_t reserved_31_3		: 29;
		uint32_t reserved_63_32		: 32;
	} bits;
} nvg_channel_2_data_t;

typedef union {
	uint64_t flat;
	struct nvg_wake_time_channel_t {
		uint32_t wake_time	: 32;
		uint32_t reserved_63_32	: 32;
	} bits;
} nvg_wake_time_channel_t;

typedef union {
	uint64_t flat;
	struct nvg_cstate_info_channel_t {
		uint32_t cluster_state			: 3;
		uint32_t reserved_6_3			: 4;
		uint32_t update_cluster			: 1;
		uint32_t cg_cstate				: 3;
		uint32_t reserved_14_11			: 4;
		uint32_t update_cg				: 1;
		uint32_t system_cstate			: 4;
		uint32_t reserved_22_20			: 3;
		uint32_t update_system			: 1;
		uint32_t reserved_30_24			: 7;
		uint32_t update_wake_mask		: 1;
		union {
			uint32_t flat				: 32;
			struct {
				uint32_t vfiq			: 1;
				uint32_t virq			: 1;
				uint32_t fiq			: 1;
				uint32_t irq			: 1;
				uint32_t serror			: 1;
				uint32_t reserved_10_5	: 6;
				uint32_t fiqout			: 1;
				uint32_t irqout			: 1;
				uint32_t reserved_31_13	: 19;
			} carmel;
		} wake_mask;
	} bits;
} nvg_cstate_info_channel_t;

typedef union {
	uint64_t flat;
	struct nvg_lower_bound_channel_t {
		uint32_t crossover_value : 32;
		uint32_t reserved_63_32	: 32;
	} bits;
} nvg_lower_bound_channel_t;

typedef union {
	uint64_t flat;
	struct nvg_cstate_stat_query_channel_t {
		uint32_t unit_id	: 4;
		uint32_t reserved_15_4	: 12;
		uint32_t stat_id	: 16;
		uint32_t reserved_63_32	: 32;
	} bits;
} nvg_cstate_stat_query_channel_t;

typedef union {
	uint64_t flat;
	struct nvg_num_cores_channel_t {
		uint32_t num_cores		: 4;
		uint32_t reserved_31_4	: 28;
		uint32_t reserved_63_32 : 32;
	} bits;
} nvg_num_cores_channel_t;

typedef union {
	uint64_t flat;
	struct nvg_unique_logical_id_channel_t {
		uint32_t unique_core_id	: 3;
		uint32_t reserved_31_3	: 29;
		uint32_t reserved_63_32 : 32;
	} bits;
} nvg_unique_logical_id_channel_t;

typedef union {
	uint64_t flat;
	struct nvg_logical_to_physical_mappings_channel_t {
		uint32_t lcore0_pcore_id	: 4;
		uint32_t lcore1_pcore_id	: 4;
		uint32_t lcore2_pcore_id	: 4;
		uint32_t lcore3_pcore_id	: 4;
		uint32_t lcore4_pcore_id	: 4;
		uint32_t lcore5_pcore_id	: 4;
		uint32_t lcore6_pcore_id	: 4;
		uint32_t lcore7_pcore_id	: 4;
		uint32_t reserved_63_32		: 32;
	} bits;
} nvg_logical_to_physical_mappings_channel_t;

typedef union {
	uint64_t flat;
	struct nvg_logical_to_mpidr_channel_write_t {
		uint32_t lcore_id		: 3;
		uint32_t reserved_31_3	: 29;
		uint32_t reserved_63_32	: 32;
	} write;
	struct nvg_logical_to_mpidr_channel_read_t {
		uint32_t mpidr			: 32;
		uint32_t reserved_63_32	: 32;
	} read;
} nvg_logical_to_mpidr_channel_t;

typedef union {
	uint64_t flat;
	struct nvg_is_sc7_allowed_channel_t {
		uint32_t is_sc7_allowed	: 1;
		uint32_t reserved_31_1	: 31;
		uint32_t reserved_63_32	: 32;
	} bits;
} nvg_is_sc7_allowed_channel_t;

typedef union {
	uint64_t flat;
	struct nvg_core_online_channel_t {
		uint32_t core_id	: 4;
		uint32_t reserved_31_4	: 28;
		uint32_t reserved_63_32	: 32;
	} bits;
} nvg_core_online_channel_t;

typedef union {
	uint64_t flat;
	struct nvg_cc3_control_channel_t {
		uint32_t freq_req	: 9;
		uint32_t reserved_30_9	: 22;
		uint32_t enable		: 1;
		uint32_t reserved_63_32	: 32;
	} bits;
} nvg_cc3_control_channel_t;

typedef enum {
	TEGRA_NVG_CHANNEL_UPDATE_GSC_ALL			=	0,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_NVDEC			=	1,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_WPR1			=	2,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_WPR2			=	3,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_TSECA			=	4,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_TSECB			=	5,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_BPMP			=	6,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_APE			=	7,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_SPE			=	8,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_SCE			=	9,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_APR			=	10,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_TZRAM			=	11,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_IPC_SE_TSEC		=	12,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_BPMP_TO_RCE		=	13,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_BPMP_TO_MCE		=	14,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_SE_SC7			=	15,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_BPMP_TO_SPE		=	16,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_RCE			=	17,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_CPU_TZ_TO_BPMP		=	18,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_VM_ENCR1			=	19,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_CPU_NS_TO_BPMP		=	20,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_OEM_SC7			=	21,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_IPC_SE_SPE_SCE_BPMP	=	22,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_SC7_RESUME_FW		=	23,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_CAMERA_TASKLIST		=	24,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_XUSB			=	25,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_CV				=	26,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_VM_ENCR2			=	27,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_HYPERVISOR_SW		=	28,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_SMMU_PAGETABLES		=	29,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_30				=	30,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_31				=	31,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_TZ_DRAM			=	32,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_NVLINK			=	33,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_SBS			=	34,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_VPR			=	35,
	TEGRA_NVG_CHANNEL_UPDATE_GSC_LAST_INDEX
} tegra_nvg_channel_update_gsc_gsc_enum_t;

typedef union {
	uint64_t flat;
	struct nvg_update_ccplex_gsc_channel_t {
		uint32_t gsc_enum	: 16;
		uint32_t reserved_31_16	: 16;
		uint32_t reserved_63_32	: 32;
	} bits;
} nvg_update_ccplex_gsc_channel_t;

typedef union {
	uint64_t flat;
	struct nvg_security_config_channel_t {
		uint32_t strict_checking_enabled : 1;
		uint32_t strict_checking_locked	: 1;
		uint32_t reserved_31_2		: 30;
		uint32_t reserved_63_32		: 32;
	} bits;
} nvg_security_config_t;

typedef union {
	uint64_t flat;
	struct nvg_shutdown_channel_t {
		uint32_t reboot		: 1;
		uint32_t reserved_31_1	: 31;
		uint32_t reserved_63_32	: 32;
	} bits;
} nvg_shutdown_t;

typedef union {
	uint64_t flat;
	struct nvg_debug_config_channel_t {
		uint32_t enter_debug_state_on_mca : 1;
		uint32_t reserved_31_1            : 31;
		uint32_t reserved_63_32           : 32;
	} bits;
} nvg_debug_config_t;

typedef union {
	uint64_t flat;
	struct nvg_hsm_error_ctrl_channel_t {
		uint32_t uncorr			: 1;
		uint32_t corr			: 1;
		uint32_t reserved_31_2	: 30;
		uint32_t reserved_63_32	: 32;
	} bits;
} nvg_hsm_error_ctrl_channel_t;

extern nvg_debug_config_t nvg_debug_config;

#endif


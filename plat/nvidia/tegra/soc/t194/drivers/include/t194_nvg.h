/*
 * Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef T194_NVG_H
#define T194_NVG_H

#include <lib/utils_def.h>

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
	TEGRA_NVG_VERSION_MAJOR = U(6),
	TEGRA_NVG_VERSION_MINOR = U(7)
};

typedef enum {
	TEGRA_NVG_CHANNEL_VERSION				= U(0),
	TEGRA_NVG_CHANNEL_POWER_PERF				= U(1),
	TEGRA_NVG_CHANNEL_POWER_MODES				= U(2),
	TEGRA_NVG_CHANNEL_WAKE_TIME				= U(3),
	TEGRA_NVG_CHANNEL_CSTATE_INFO				= U(4),
	TEGRA_NVG_CHANNEL_CROSSOVER_C6_LOWER_BOUND		= U(5),
	TEGRA_NVG_CHANNEL_CROSSOVER_CC6_LOWER_BOUND		= U(6),
	TEGRA_NVG_CHANNEL_CROSSOVER_CG7_LOWER_BOUND		= U(8),
	TEGRA_NVG_CHANNEL_CSTATE_STAT_QUERY_REQUEST		= U(10),
	TEGRA_NVG_CHANNEL_CSTATE_STAT_QUERY_VALUE		= U(11),
	TEGRA_NVG_CHANNEL_NUM_CORES				= U(20),
	TEGRA_NVG_CHANNEL_UNIQUE_LOGICAL_ID			= U(21),
	TEGRA_NVG_CHANNEL_LOGICAL_TO_PHYSICAL_MAPPING		= U(22),
	TEGRA_NVG_CHANNEL_LOGICAL_TO_MPIDR			= U(23),
	TEGRA_NVG_CHANNEL_SHUTDOWN				= U(42),
	TEGRA_NVG_CHANNEL_IS_SC7_ALLOWED			= U(43),
	TEGRA_NVG_CHANNEL_ONLINE_CORE				= U(44),
	TEGRA_NVG_CHANNEL_CC3_CTRL				= U(45),
	TEGRA_NVG_CHANNEL_CCPLEX_CACHE_CONTROL			= U(49),
	TEGRA_NVG_CHANNEL_UPDATE_CCPLEX_GSC			= U(50),
	TEGRA_NVG_CHANNEL_HSM_ERROR_CTRL			= U(53),
	TEGRA_NVG_CHANNEL_SECURITY_CONFIG			= U(54),
	TEGRA_NVG_CHANNEL_DEBUG_CONFIG				= U(55),
	TEGRA_NVG_CHANNEL_DDA_SNOC_MCF				= U(56),
	TEGRA_NVG_CHANNEL_DDA_MCF_ORD1				= U(57),
	TEGRA_NVG_CHANNEL_DDA_MCF_ORD2				= U(58),
	TEGRA_NVG_CHANNEL_DDA_MCF_ORD3				= U(59),
	TEGRA_NVG_CHANNEL_DDA_MCF_ISO				= U(60),
	TEGRA_NVG_CHANNEL_DDA_MCF_SISO				= U(61),
	TEGRA_NVG_CHANNEL_DDA_MCF_NISO				= U(62),
	TEGRA_NVG_CHANNEL_DDA_MCF_NISO_REMOTE			= U(63),
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_ISO			= U(64),
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_SISO			= U(65),
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_NISO			= U(66),
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_NISO_REMOTE		= U(67),
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_L3FILL			= U(68),
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_L3WR			= U(69),
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_RSP_L3RD_DMA		= U(70),
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_RSP_MCFRD_DMA		= U(71),
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_GLOBAL			= U(72),
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_LL				= U(73),
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_L3D			= U(74),
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_FCM_RD			= U(75),
	TEGRA_NVG_CHANNEL_DDA_L3CTRL_FCM_WR			= U(76),
	TEGRA_NVG_CHANNEL_DDA_SNOC_GLOBAL_CTRL			= U(77),
	TEGRA_NVG_CHANNEL_DDA_SNOC_CLIENT_REQ_CTRL		= U(78),
	TEGRA_NVG_CHANNEL_DDA_SNOC_CLIENT_REPLENTISH_CTRL	= U(79),
	TEGRA_NVG_CHANNEL_RT_SAFE_MASK				= U(80),
	TEGRA_NVG_CHANNEL_RT_WINDOW_US				= U(81),
	TEGRA_NVG_CHANNEL_RT_FWD_PROGRESS_US			= U(82),

	TEGRA_NVG_CHANNEL_LAST_INDEX
} tegra_nvg_channel_id_t;

typedef enum {
	NVG_STAT_QUERY_SC7_ENTRIES				= U(1),
	NVG_STAT_QUERY_CC6_ENTRIES				= U(6),
	NVG_STAT_QUERY_CG7_ENTRIES				= U(7),
	NVG_STAT_QUERY_C6_ENTRIES				= U(10),
	NVG_STAT_QUERY_C7_ENTRIES				= U(14),
	NVG_STAT_QUERY_SC7_RESIDENCY_SUM			= U(32),
	NVG_STAT_QUERY_CC6_RESIDENCY_SUM			= U(41),
	NVG_STAT_QUERY_CG7_RESIDENCY_SUM			= U(46),
	NVG_STAT_QUERY_C6_RESIDENCY_SUM				= U(51),
	NVG_STAT_QUERY_C7_RESIDENCY_SUM				= U(56),
	NVG_STAT_QUERY_SC7_ENTRY_TIME_SUM			= U(60),
	NVG_STAT_QUERY_CC6_ENTRY_TIME_SUM			= U(61),
	NVG_STAT_QUERY_CG7_ENTRY_TIME_SUM			= U(62),
	NVG_STAT_QUERY_C6_ENTRY_TIME_SUM			= U(63),
	NVG_STAT_QUERY_C7_ENTRY_TIME_SUM			= U(64),
	NVG_STAT_QUERY_SC7_EXIT_TIME_SUM			= U(70),
	NVG_STAT_QUERY_CC6_EXIT_TIME_SUM			= U(71),
	NVG_STAT_QUERY_CG7_EXIT_TIME_SUM			= U(72),
	NVG_STAT_QUERY_C6_EXIT_TIME_SUM				= U(73),
	NVG_STAT_QUERY_C7_EXIT_TIME_SUM				= U(74),
	NVG_STAT_QUERY_SC7_ENTRY_LAST				= U(80),
	NVG_STAT_QUERY_CC6_ENTRY_LAST				= U(81),
	NVG_STAT_QUERY_CG7_ENTRY_LAST				= U(82),
	NVG_STAT_QUERY_C6_ENTRY_LAST				= U(83),
	NVG_STAT_QUERY_C7_ENTRY_LAST				= U(84),
	NVG_STAT_QUERY_SC7_EXIT_LAST				= U(90),
	NVG_STAT_QUERY_CC6_EXIT_LAST				= U(91),
	NVG_STAT_QUERY_CG7_EXIT_LAST				= U(92),
	NVG_STAT_QUERY_C6_EXIT_LAST				= U(93),
	NVG_STAT_QUERY_C7_EXIT_LAST				= U(94)

} tegra_nvg_stat_query_t;

typedef enum {
	TEGRA_NVG_CORE_C0 = U(0),
	TEGRA_NVG_CORE_C1 = U(1),
	TEGRA_NVG_CORE_C6 = U(6),
	TEGRA_NVG_CORE_C7 = U(7),
	TEGRA_NVG_CORE_WARMRSTREQ = U(8)
} tegra_nvg_core_sleep_state_t;

typedef enum {
	TEGRA_NVG_SHUTDOWN = U(0),
	TEGRA_NVG_REBOOT = U(1)
} tegra_nvg_shutdown_reboot_state_t;

typedef enum {
	TEGRA_NVG_CLUSTER_CC0		= U(0),
	TEGRA_NVG_CLUSTER_AUTO_CC1	= U(1),
	TEGRA_NVG_CLUSTER_CC6		= U(6)
} tegra_nvg_cluster_sleep_state_t;

typedef enum {
	TEGRA_NVG_CG_CG0 = U(0),
	TEGRA_NVG_CG_CG7 = U(7)
} tegra_nvg_cluster_group_sleep_state_t;

typedef enum {
	TEGRA_NVG_SYSTEM_SC0 = U(0),
	TEGRA_NVG_SYSTEM_SC7 = U(7),
	TEGRA_NVG_SYSTEM_SC8 = U(8)
} tegra_nvg_system_sleep_state_t;

// ---------------------------------------------------------------------------
// NVG Data subformats
// ---------------------------------------------------------------------------

typedef union {
	uint64_t flat;
	struct nvg_version_channel_t {
		uint32_t minor_version : U(32);
		uint32_t major_version : U(32);
	} bits;
} nvg_version_data_t;

typedef union {
	uint64_t flat;
	struct {
		uint32_t perf_per_watt	: U(1);
		uint32_t reserved_31_1	: U(31);
		uint32_t reserved_63_32	: U(32);
	} bits;
} nvg_power_perf_channel_t;

typedef union {
	uint64_t flat;
	struct {
		uint32_t low_battery	: U(1);
		uint32_t reserved_1_1	: U(1);
		uint32_t battery_save	: U(1);
		uint32_t reserved_31_3	: U(29);
		uint32_t reserved_63_32	: U(32);
	} bits;
} nvg_power_modes_channel_t;

typedef union nvg_channel_1_data_u {
	uint64_t flat;
	struct nvg_channel_1_data_s {
		uint32_t perf_per_watt_mode	: U(1);
		uint32_t reserved_31_1		: U(31);
		uint32_t reserved_63_32		: U(32);
	} bits;
} nvg_channel_1_data_t;

typedef union {
	uint64_t flat;
	struct {
		uint32_t gpu_ways	: U(5);
		uint32_t reserved_7_5	: U(3);
		uint32_t gpu_only_ways	: U(5);
		uint32_t reserved_31_13 : U(19);
		uint32_t reserved_63_32 : U(32);
	} bits;
} nvg_ccplex_cache_control_channel_t;

typedef union nvg_channel_2_data_u {
	uint64_t flat;
	struct nvg_channel_2_data_s {
		uint32_t reserved_1_0		: U(2);
		uint32_t battery_saver_mode	: U(1);
		uint32_t reserved_31_3		: U(29);
		uint32_t reserved_63_32		: U(32);
	} bits;
} nvg_channel_2_data_t;

typedef union {
	uint64_t flat;
	struct {
		uint32_t wake_time		: U(32);
		uint32_t reserved_63_32		: U(32);
	} bits;
} nvg_wake_time_channel_t;

typedef union {
	uint64_t flat;
	struct {
		uint32_t cluster_state			: U(3);
		uint32_t reserved_6_3			: U(4);
		uint32_t update_cluster			: U(1);
		uint32_t cg_cstate			: U(3);
		uint32_t reserved_14_11			: U(4);
		uint32_t update_cg			: U(1);
		uint32_t system_cstate			: U(4);
		uint32_t reserved_22_20			: U(3);
		uint32_t update_system			: U(1);
		uint32_t reserved_30_24			: U(7);
		uint32_t update_wake_mask		: U(1);
		union {
			uint32_t flat			: U(32);
			struct {
				uint32_t vfiq		: U(1);
				uint32_t virq		: U(1);
				uint32_t fiq		: U(1);
				uint32_t irq		: U(1);
				uint32_t serror		: U(1);
				uint32_t reserved_10_5	: U(6);
				uint32_t fiqout		: U(1);
				uint32_t irqout		: U(1);
				uint32_t reserved_31_13	: U(19);
			} carmel;
		} wake_mask;
	} bits;
} nvg_cstate_info_channel_t;

typedef union {
	uint64_t flat;
	struct {
		uint32_t crossover_value	: U(32);
		uint32_t reserved_63_32		: U(32);
	} bits;
} nvg_lower_bound_channel_t;

typedef union {
	uint64_t flat;
	struct {
		uint32_t unit_id		: U(4);
		uint32_t reserved_15_4		: U(12);
		uint32_t stat_id		: U(16);
		uint32_t reserved_63_32		: U(32);
	} bits;
} nvg_cstate_stat_query_channel_t;

typedef union {
	uint64_t flat;
	struct {
		uint32_t num_cores		: U(4);
		uint32_t reserved_31_4		: U(28);
		uint32_t reserved_63_32		: U(32);
	} bits;
} nvg_num_cores_channel_t;

typedef union {
	uint64_t flat;
	struct {
		uint32_t unique_core_id		: U(3);
		uint32_t reserved_31_3		: U(29);
		uint32_t reserved_63_32		: U(32);
	} bits;
} nvg_unique_logical_id_channel_t;

typedef union {
	uint64_t flat;
	struct {
		uint32_t lcore0_pcore_id	: U(4);
		uint32_t lcore1_pcore_id	: U(4);
		uint32_t lcore2_pcore_id	: U(4);
		uint32_t lcore3_pcore_id	: U(4);
		uint32_t lcore4_pcore_id	: U(4);
		uint32_t lcore5_pcore_id	: U(4);
		uint32_t lcore6_pcore_id	: U(4);
		uint32_t lcore7_pcore_id	: U(4);
		uint32_t reserved_63_32		: U(32);
	} bits;
} nvg_logical_to_physical_mappings_channel_t;

typedef union {
	uint64_t flat;
	struct nvg_logical_to_mpidr_channel_write_t {
		uint32_t lcore_id		: U(3);
		uint32_t reserved_31_3		: U(29);
		uint32_t reserved_63_32		: U(32);
	} write;
	struct nvg_logical_to_mpidr_channel_read_t {
		uint32_t mpidr			: U(32);
		uint32_t reserved_63_32		: U(32);
	} read;
} nvg_logical_to_mpidr_channel_t;

typedef union {
	uint64_t flat;
	struct {
		uint32_t is_sc7_allowed		: U(1);
		uint32_t reserved_31_1		: U(31);
		uint32_t reserved_63_32		: U(32);
	} bits;
} nvg_is_sc7_allowed_channel_t;

typedef union {
	uint64_t flat;
	struct {
		uint32_t core_id		: U(4);
		uint32_t reserved_31_4		: U(28);
		uint32_t reserved_63_32		: U(32);
	} bits;
} nvg_core_online_channel_t;

typedef union {
	uint64_t flat;
	struct {
		uint32_t freq_req		: U(9);
		uint32_t reserved_30_9		: U(22);
		uint32_t enable			: U(1);
		uint32_t reserved_63_32		: U(32);
	} bits;
} nvg_cc3_control_channel_t;

typedef enum {
	TEGRA_NVG_CHANNEL_UPDATE_GSC_ALL		=	U(0),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_NVDEC		=	U(1),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_WPR1		=	U(2),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_WPR2		=	U(3),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_TSECA		=	U(4),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_TSECB		=	U(5),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_BPMP		=	U(6),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_APE		=	U(7),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_SPE		=	U(8),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_SCE		=	U(9),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_APR		=	U(10),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_TZRAM		=	U(11),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_IPC_SE_TSEC	=	U(12),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_BPMP_TO_RCE	=	U(13),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_BPMP_TO_MCE	=	U(14),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_SE_SC7		=	U(15),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_BPMP_TO_SPE	=	U(16),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_RCE		=	U(17),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_CPU_TZ_TO_BPMP	=	U(18),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_VM_ENCR1		=	U(19),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_CPU_NS_TO_BPMP	=	U(20),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_OEM_SC7		=	U(21),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_IPC_SE_SPE_SCE_BPMP =	U(22),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_SC7_RESUME_FW	=	U(23),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_CAMERA_TASKLIST	=	U(24),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_XUSB		=	U(25),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_CV			=	U(26),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_VM_ENCR2		=	U(27),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_HYPERVISOR_SW	=	U(28),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_SMMU_PAGETABLES	=	U(29),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_30			=	U(30),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_31			=	U(31),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_TZ_DRAM		=	U(32),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_NVLINK		=	U(33),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_SBS		=	U(34),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_VPR		=	U(35),
	TEGRA_NVG_CHANNEL_UPDATE_GSC_LAST_INDEX
} tegra_nvg_channel_update_gsc_gsc_enum_t;

typedef union {
	uint64_t flat;
	struct {
		uint32_t gsc_enum	: U(16);
		uint32_t reserved_31_16 : U(16);
		uint32_t reserved_63_32 : U(32);
	} bits;
} nvg_update_ccplex_gsc_channel_t;

typedef union {
	uint64_t flat;
	struct nvg_security_config_channel_t {
		uint32_t strict_checking_enabled	: U(1);
		uint32_t strict_checking_locked		: U(1);
		uint32_t reserved_31_2			: U(30);
		uint32_t reserved_63_32			: U(32);
	} bits;
} nvg_security_config_t;

typedef union {
	uint64_t flat;
	struct nvg_shutdown_channel_t {
		uint32_t reboot				: U(1);
		uint32_t reserved_31_1			: U(31);
		uint32_t reserved_63_32			: U(32);
	} bits;
} nvg_shutdown_t;

typedef union {
	uint64_t flat;
	struct nvg_debug_config_channel_t {
		uint32_t enter_debug_state_on_mca	: U(1);
		uint32_t reserved_31_1			: U(31);
		uint32_t reserved_63_32			: U(32);
	} bits;
} nvg_debug_config_t;

typedef union {
	uint64_t flat;
	struct {
		uint32_t uncorr				: U(1);
		uint32_t corr				: U(1);
		uint32_t reserved_31_2			: U(30);
		uint32_t reserved_63_32			: U(32);
	} bits;
} nvg_hsm_error_ctrl_channel_t;

extern nvg_debug_config_t nvg_debug_config;

#endif /* T194_NVG_H */

/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
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
    TEGRA_NVG_VERSION_MINOR = 0,
};

typedef enum {
    TEGRA_NVG_CHANNEL_VERSION = 0,
    TEGRA_NVG_CHANNEL_POWER_PERF = 1,
    TEGRA_NVG_CHANNEL_POWER_MODES = 2,
    TEGRA_NVG_CHANNEL_WAKE_TIME = 3,
    TEGRA_NVG_CHANNEL_CSTATE_INFO = 4,
    TEGRA_NVG_CHANNEL_CROSSOVER_C6_LOWER_BOUND = 5,
    TEGRA_NVG_CHANNEL_CROSSOVER_CC6_LOWER_BOUND = 6,
    // Value 7 reserved
    TEGRA_NVG_CHANNEL_CROSSOVER_CG7_LOWER_BOUND = 8,
    // Value 9 reserved
    TEGRA_NVG_CHANNEL_CSTATE_STAT_QUERY_REQUEST = 10,
    TEGRA_NVG_CHANNEL_CSTATE_STAT_QUERY_VALUE = 11,
    // Values 12-42 reserved
    TEGRA_NVG_CHANNEL_IS_SC7_ALLOWED = 43,
    TEGRA_NVG_CHANNEL_ONLINE_CORE = 44,
    TEGRA_NVG_CHANNEL_CC3_CTRL = 45,
    TEGRA_NVG_CHANNEL_UPDATE_CCPLEX_GSC = 50,
    TEGRA_NVG_CHANNEL_CCPLEX_CACHE_INVAL = 51,
    // 52 FREQ FEEDBACK
    TEGRA_NVG_CHANNEL_HSM_ERROR_CTRL = 53,
    TEGRA_NVG_CHANNEL_SECURITY_CONFIG = 54,
    TEGRA_NVG_CHANNEL_LAST_INDEX,
} tegra_nvg_channel_id_t;


typedef enum {
    // Value 0 reserved
    NVG_STAT_QUERY_SC7_ENTRIES = 1,
    // Values 2-5 reserved
    NVG_STAT_QUERY_CC6_ENTRIES = 6,
    NVG_STAT_QUERY_CG7_ENTRIES = 7,
    // Values 8-9 reserved
    NVG_STAT_QUERY_C6_ENTRIES = 10,
    // Values 11-13 reserved
    NVG_STAT_QUERY_C7_ENTRIES = 14,
    // Values 15-31 reserved
    NVG_STAT_QUERY_SC7_RESIDENCY_SUM = 32,
    NVG_STAT_QUERY_CC6_RESIDENCY_SUM = 41,
    NVG_STAT_QUERY_CG7_RESIDENCY_SUM = 46,
    NVG_STAT_QUERY_C6_RESIDENCY_SUM = 51,
    NVG_STAT_QUERY_C7_RESIDENCY_SUM = 56,
} tegra_nvg_stat_query_t;


typedef enum {
    TEGRA_NVG_CORE_C0 = 0,
    TEGRA_NVG_CORE_C1 = 1,
    TEGRA_NVG_CORE_C6 = 6,
    TEGRA_NVG_CORE_C7 = 7,
    TEGRA_NVG_CORE_WARMRSTREQ = 8,
} tegra_nvg_core_sleep_state_t;

typedef enum {
    TEGRA_NVG_CLUSTER_CC0 = 0,
    TEGRA_NVG_CLUSTER_CC6 = 6,
} tegra_nvg_cluster_sleep_state_t;

typedef enum {
    TEGRA_NVG_CCPLEX_CG0 = 0,
    TEGRA_NVG_CCPLEX_CG7 = 1,

} tegra_nvg_cluster_group_sleep_state_t;

typedef enum {
    TEGRA_NVG_SYSTEM_SC0 = 0,
    TEGRA_NVG_SYSTEM_SC7 = 7,
    TEGRA_NVG_SYSTEM_SC8 = 8,
} tegra_nvg_system_sleep_state_t;

// ---------------------------------------------------------------------------
// NVG Data subformats
// ---------------------------------------------------------------------------

typedef union
{
    uint64_t flat;
    struct nvg_version_channel_t {
        uint64_t minor_version : 32;
        uint64_t major_version : 32;
    } bits;
} nvg_version_data_t;

typedef union nvg_channel_1_data_u
{
    uint64_t flat;
    struct nvg_channel_1_data_s
    {
        uint64_t perf_per_watt_mode : 1;
        uint64_t reserved_63_1 : 63;
    } bits;
} nvg_channel_1_data_t;

typedef union nvg_channel_2_data_u
{
    uint64_t flat;
    struct nvg_channel_2_data_s
    {
        uint64_t reserved_1_0 : 2;
        uint64_t battery_saver_mode : 1;
        uint64_t reserved_63_3 : 61;
    } bits;
} nvg_channel_2_data_t;

typedef union
{
    uint64_t flat;
    struct nvg_wake_time_channel_t {
        uint64_t wake_time : 32;
        uint64_t reserved_63_32 : 32;
    } bits;
} nvg_wake_time_channel_t;

typedef union
{
    uint64_t flat;
    struct nvg_cstate_info_channel_t {
        uint64_t cluster_state : 3;
        uint64_t reserved_6_3 : 4;
        uint64_t update_cluster : 1;
        uint64_t cg_cstate : 3;
        uint64_t reserved_14_11 : 4;
        uint64_t update_cg : 1;
        uint64_t system_cstate : 4;
        uint64_t reserved_22_20 : 3;
        uint64_t update_system : 1;
        uint64_t reserved_30_24 : 7;
        uint64_t update_wake_mask : 1;
        uint64_t wake_mask : 32;
    } bits;
} nvg_cstate_info_channel_t;

typedef union
{
    uint64_t flat;
    struct nvg_lower_bound_channel_t {
        uint64_t crossover_value : 32;
        uint64_t reserved_63_32 : 32;
    } bits;
} nvg_lower_bound_channel_t;


typedef union
{
    uint64_t flat;
    struct nvg_cstate_stat_query_channel_t {
        uint64_t unit_id : 4;
        uint64_t reserved_15_4 : 12;
        uint64_t stat_id : 16;
        uint64_t reserved_63_32 : 32;
    } bits;
} nvg_cstate_stat_query_channel_t;

typedef union
{
    uint64_t flat;
    struct nvg_is_sc7_allowed_channel_t {
        uint64_t is_sc7_allowed : 1;
        uint64_t reserved_63_32 : 63;
    } bits;
} nvg_is_sc7_allowed_channel_t;


typedef union
{
    uint64_t flat;
    struct nvg_core_online_channel_t {
        uint64_t core_id : 4;
        uint64_t reserved_63_4 : 60;
    } bits;
} nvg_core_online_channel_t;


typedef union
{
    uint64_t flat;
    struct nvg_cc3_control_channel_t {
        uint64_t freq_req : 8;
        uint64_t reserved_30_8 : 23;
        uint64_t enable : 1;
        uint64_t reserved_63_32 : 32;
    } bits;
} nvg_cc3_control_channel_t;


typedef union
{
    uint64_t flat;
    struct nvg_update_gsc_channel_t {
        uint64_t gsc_enum : 16;
        uint64_t reserved_63_16 : 48;
    } bits;
} nvg_update_gsc_channel_t;


typedef union
{
    uint64_t flat;
    struct nvg_cache_inval_channel_t {
        uint64_t cache_clean : 1;
        uint64_t cache_clean_inval : 1;
        uint64_t cache_clean_inval_tr : 1;
        uint64_t reserved_63_3 : 61;
    } bits;
} nvg_cache_inval_channel_t;


/* GSC type define */
typedef enum {
    TEGRA_NVG_GSC_ALL=0,
    TEGRA_NVG_GSC_NVDEC=1,
    TEGRA_NVG_GSC_WPR1=2,
    TEGRA_NVG_GSC_WPR2=3,
    TEGRA_NVG_GSC_TSECA=4,
    TEGRA_NVG_GSC_TSECB=5,

    TEGRA_NVG_GSC_BPMP=6,
    TEGRA_NVG_GSC_APE=7,
    TEGRA_NVG_GSC_SPE=8,
    TEGRA_NVG_GSC_SCE=9,
    TEGRA_NVG_GSC_APR=10,
    TEGRA_NVG_GSC_TZRAM=11,
    TEGRA_NVG_GSC_SE=12,

    TEGRA_NVG_GSC_DMCE=13,
    TEGRA_NVG_GSC_BPMP_TO_DMCE=14,
    TEGRA_NVG_GSC_BPMP_TO_SPE=16,
    TEGRA_NVG_GSC_CPU_TZ_TO_BPMP=18,
    TEGRA_NVG_GSC_CPU_NS_TO_BPMP=20,
    TEGRA_NVG_GSC_IPC_SE_SPE_SCE_BPMP=22,
    TEGRA_NVG_GSC_SC7_RESUME_FW=23,

    TEGRA_NVG_GSC_VPR_RESIZE=24,
    TEGRA_NVG_GSC_RCE=25,
    TEGRA_NVG_GSC_CV=26,

    TEGRA_NVG_GSC_BO_MTS_PACKAGE=28,
    TEGRA_NVG_GSC_BO_MCE_PREBOOT=29,

    TEGRA_NVG_GSC_TZ_DRAM_IDX=34,
    TEGRA_NVG_GSC_VPR_IDX=35,
} tegra_nvg_gsc_index_t;

typedef enum {
    TEGRA_NVG_CROSSOVER_C6 = 0,
    TEGRA_NVG_CROSSOVER_CC6 = 1,
    TEGRA_NVG_CROSSOVER_CG7 = 2,
} tegra_nvg_crossover_index_t;

#endif // T194_NVG_H

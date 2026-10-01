/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPUCP_SHARED_H
#define QTI_CPUCP_SHARED_H

#include <stdint.h>

#include <clkdom_config.h>

/*
 * Interface version 2 data shared with the CPUCP firmware at
 * CPUCP_SHARED_DATA_BASE. The firmware image initialises it; the host fills
 * in the chip identification, CPR fuse data and MX rail levels before
 * starting CPUCP.
 */
#define CPUCP_SHARED_VERSION		2U

#define CPUCP_NUM_CPR			16
#define CPUCP_NUM_CORNERS		8
#define CPUCP_NUM_RAILS			2

enum cpucp_corner {
	CPUCP_CORNER_SVSL1,
	CPUCP_CORNER_NOM,
	CPUCP_CORNER_TUR,
	CPUCP_CORNER_TURL1,
	CPUCP_NUM_FUSED_CORNERS
};

struct cpucp_soc_info {
	uint32_t chip_version;
	uint32_t foundry_id;
	uint32_t speed_bin;
	uint32_t feature_id;
	uint32_t jtag_id;
	uint32_t vp_id;
	uint32_t soft_sku_lval[CD_MAX];
};

struct cpucp_cpr_info {
	uint8_t local_rev;
	uint8_t aging[CPUCP_NUM_CPR];
	uint8_t rosel[CPUCP_NUM_CPR][CPUCP_NUM_CORNERS];
	int16_t targ_volt[CPUCP_NUM_CPR][CPUCP_NUM_CORNERS];
	int16_t quot_offset[CPUCP_NUM_CPR][CPUCP_NUM_CORNERS];
	uint16_t quot_vmin[CPUCP_NUM_CPR][CPUCP_NUM_CORNERS];
};

struct cpucp_acc_info {
	uint8_t acc_fuse;
	uint8_t acc_corner[CPUCP_NUM_CORNERS];
};

struct cpucp_cmd_db {
	uint32_t addr;
	uint16_t len;
	uint16_t data[14];
};

struct cpucp_shared_data {
	uint32_t version;
	uint8_t apm_crossover_corner[CD_MAX];
	uint32_t reserved1;
	uint8_t acd_dvm_lval[CD_MAX][7];
	uint32_t acd_dvm_val[CD_MAX][7];
	uint8_t acd_avg_lval[CD_MAX][7];
	uint32_t acd_avg_cfg[CD_MAX][7];
	uint8_t acd_acdsscr_lval[CD_MAX][7];
	uint32_t acd_acdsscr_val[CD_MAX][7];
	uint8_t bcl_lval[CD_MAX][4];
	uint8_t park_pll_lval[CD_MAX];
	uint8_t park_vc[CD_MAX];
	uint8_t vc_880mv[CD_MAX];
	uint32_t reserved2;
	uint8_t mem_acc_nom_vc[CD_MAX];
	uint8_t mem_acc_tur_vc[CD_MAX];
	uint32_t reserved3;
	uint8_t l3_mx_vote_vc_crossover[6];
	uint8_t l3_mx_vote_data[6];
	uint16_t dcvs_saw_avs_dly[CPUCP_NUM_RAILS];
	uint8_t band_vc_th[CD_MAX][4];
	uint8_t pmic_step_rates[CD_MAX][4];
	uint32_t avg_enable_lut_vector0[CD_MAX];
	uint32_t avg_enable_lut_vector1[CD_MAX];
	uint16_t gplus_vmax[6];
	uint32_t reserved4;
	struct cpucp_soc_info soc_info;
	struct cpucp_cpr_info cpr_info;
	struct cpucp_acc_info acc_info[CD_MAX];
	struct cpucp_cmd_db mx_cmd_db;
	uint8_t per_core_dcvs_disable;
	int32_t is_slt_plat;
};

#endif /* QTI_CPUCP_SHARED_H */

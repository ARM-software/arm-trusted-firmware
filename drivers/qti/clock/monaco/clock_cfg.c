/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Clock configuration data for monaco (qcs8300). Only the data the boot flow uses is
 * defined: the init clock groups and the PLL sources the back-end votes on.
 */

#include <stdint.h>

#include "clock_hwio.h"
#include <drivers/qti/clock/clock.h>
#include <drivers/qti/clock/clock_cfg.h>
#include <drivers/qti/clock/clock_driver.h>
#include <drivers/qti/pwr_utils/voltage_level.h>

/* GPLL0 is voted on during init and kept on for the CXPC/DPM handshake. */
static struct clock_source sources[CLOCK_SOURCE_TOTAL] = {
	[CLOCK_SOURCE_GPLL0] = {
		.hw_source = { GCC_GPLL0_PLL_MODE,
			       PLL_VOTE(GCC_GPLL0) },
		.source = &sources[CLOCK_SOURCE_XO]
	}
};

static struct clock_group clock_groups[CLOCK_GROUP_TOTAL] = {
	[CLOCK_GROUP_INIT] = {
		.clks = (struct clock_desc[]) {
			{ GCC_CE0_AHB_CBCR, VOTE(CE0_AHB_CLK_ENA) },
			{ GCC_CE1_AHB_CBCR, VOTE(CE1_AHB_CLK_ENA) },
			{ GCC_DCC_AHB_CBCR },
			{ GCC_IPA_2X_CBCR },
			{ GCC_IPA_AHB_CBCR },
			{ GCC_IPA_CBCR },
			{ GCC_MMSS_QM_AHB_CBCR },
			{ GCC_MMSS_QM_CORE_CBCR },
			{ GCC_QUPV3_WRAP0_CORE_CBCR,
			  VOTE_1(QUPV3_WRAP0_CORE_CLK_ENA) },
			{ GCC_QUPV3_WRAP_0_M_AHB_CBCR,
			  VOTE_1(QUPV3_WRAP_0_M_AHB_CLK_ENA) },
			{ GCC_QUPV3_WRAP_0_S_AHB_CBCR,
			  VOTE_1(QUPV3_WRAP_0_S_AHB_CLK_ENA) },
			{ GCC_QUPV3_WRAP1_CORE_CBCR,
			  VOTE_1(QUPV3_WRAP1_CORE_CLK_ENA) },
			{ GCC_QUPV3_WRAP_1_M_AHB_CBCR,
			  VOTE_1(QUPV3_WRAP_1_M_AHB_CLK_ENA) },
			{ GCC_QUPV3_WRAP_1_S_AHB_CBCR,
			  VOTE_1(QUPV3_WRAP_1_S_AHB_CLK_ENA) },
			{ GCC_QUPV3_WRAP3_CORE_CBCR,
			  VOTE(QUPV3_WRAP3_CORE_CLK_ENA) },
			{ GCC_QUPV3_WRAP_3_M_AHB_CBCR,
			  VOTE(QUPV3_WRAP_3_M_AHB_CLK_ENA) },
			{ GCC_QUPV3_WRAP_3_S_AHB_CBCR,
			  VOTE(QUPV3_WRAP_3_S_AHB_CLK_ENA) },
			{ GCC_SDCC1_AHB_CBCR },
			{ GCC_SDCC1_ICE_CORE_CBCR },
			{ GCC_UFS_PHY_AHB_CBCR },
			{ GCC_UFS_PHY_ICE_CORE_CBCR },
			{ CAM_CC_CPAS_AHB_CBCR,      .part = CHIPINFO_PART_CAMERA },
			{ CAM_CC_CPAS_FAST_AHB_CBCR, .part = CHIPINFO_PART_CAMERA },
			{ CAM_CC_CORE_AHB_CBCR,      .part = CHIPINFO_PART_CAMERA },
			{ MDSS_0_DISP_CC_MDSS_AHB_CBCR,  .part = CHIPINFO_PART_DISPLAY },
			{ MDSS_0_DISP_CC_MDSS_AHB1_CBCR, .part = CHIPINFO_PART_DISPLAY },
			{ GCC_TURING_0_Q6_TBU0_CBCR, .part = CHIPINFO_PART_NSP },
			{ GCC_TURING_0_Q6_TBU1_CBCR, .part = CHIPINFO_PART_NSP },
			{ LPASS_AON_CC_DEBUG_XPU_CLIENT_CBCR, .part = CHIPINFO_PART_AUDIO },
			{ LPASS_AON_CC_PDC_H_CBCR,            .part = CHIPINFO_PART_AUDIO },
			{ LPASS_AON_CC_Q6_XPU2_CLIENT_CBCR,   .part = CHIPINFO_PART_AUDIO },
			{ LPASS_AON_CC_Q6_XPU2_CONFIG_CBCR,   .part = CHIPINFO_PART_AUDIO },
			{ LPASS_AON_CC_VA_XPU2_CLIENT_CBCR,   .part = CHIPINFO_PART_AUDIO },
			{ LPASS_AUDIO_CC_XPU2_CLIENT_CBCR,    .part = CHIPINFO_PART_AUDIO },
			{ LPASS_AUDIO_CC_BUS_CBCR,            .part = CHIPINFO_PART_AUDIO },
			{ LPASS_LPASS_TOP_CC_AGGNOC_MPU_LS_CBCR, .part = CHIPINFO_PART_AUDIO },
			{ LPASS_AUDIO_CC_SLIMBUS_CORE_CBCR,   .part = CHIPINFO_PART_AUDIO },
			{ 0 }
		},

		.pwr_domains = (struct clock_power_domain_desc[]) {
			{ CAM_CC_TITAN_TOP_GDSCR,             .part = CHIPINFO_PART_CAMERA  },
			{ MDSS_0_DISP_CC_MDSS_CORE_GDSCR,    .part = CHIPINFO_PART_DISPLAY },
			{ MDSS_0_DISP_CC_MDSS_CORE_INT2_GDSCR, .part = CHIPINFO_PART_DISPLAY },
			{ LPASS_AON_CC_LPASS_AUDIO_HM_GDSCR, .part = CHIPINFO_PART_AUDIO },
			{ LPASS_AON_CC_LPASS_PDC_HM_GDSCR,   .part = CHIPINFO_PART_AUDIO },
			{ LPASS_AON_CC_LPASS_AUDIO_ML_GDSCR,  .part = CHIPINFO_PART_AUDIO },
			{ GCC_USB30_PRIM_GDSCR },
			{ GCC_PCIE_0_GDSCR },
			{ GCC_PCIE_1_GDSCR },
			{ GCC_UFS_PHY_GDSCR },
			{ 0 }
		},
		.access_clks = (struct clock_desc[]) {
			{ GCC_CAMERA_AHB_CBCR,         .part = CHIPINFO_PART_CAMERA    },
			{ GCC_CAMERA_XO_CBCR,          .part = CHIPINFO_PART_CAMERA    },
			{ GCC_DISP_AHB_CBCR,           .part = CHIPINFO_PART_DISPLAY   },
			{ GCC_DISP_XO_CBCR,            .part = CHIPINFO_PART_DISPLAY   },
			{ GCC_GPU_CFG_AHB_CBCR,        .part = CHIPINFO_PART_GPU       },
			{ GCC_LPASS_CFG_NOC_SWAY_CBCR, .part = CHIPINFO_PART_AUDIO     },
			{ GCC_VIDEO_AHB_CBCR,          .part = CHIPINFO_PART_VIDEO     },
			{ 0 }
		},
		.volt_reqs = (struct clock_voltage_request[]) {
			{ "mmcx.lvl", RAIL_VOLTAGE_LEVEL_NOM },
			{ "gfx.lvl",  RAIL_VOLTAGE_LEVEL_NOM, .part = CHIPINFO_PART_GPU },
			{ 0 }
		},
	},

	[CLOCK_GROUP_QDSS] = {
		.clks = (struct clock_desc[]) {
			/*
			 * This register enables the CDBGPWRUPREQ signal that
			 * enables all QDSS-related clocks.
			 */
			{ GCC_DEBUG_EN },
			{ 0 }
		},
	},
};

struct clock_config clock_cfg = {
	.clock_groups = clock_groups,
	.sources      = sources,
};

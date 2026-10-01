/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <cpucp_config.h>
#include <cpucp_hwio.h>
#include <cpucp_shared.h>
#include <lib/cassert.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <platform_def.h>

/* Offsets the firmware expects for the interface version 2 host fields. */
CASSERT(offsetof(struct cpucp_shared_data, soc_info.chip_version) == 0x23cU,
	assert_cpucp_chip_version_offset);
CASSERT(offsetof(struct cpucp_shared_data, soc_info.foundry_id) == 0x240U,
	assert_cpucp_foundry_id_offset);
CASSERT(offsetof(struct cpucp_shared_data, soc_info.speed_bin) == 0x244U,
	assert_cpucp_speed_bin_offset);
CASSERT(offsetof(struct cpucp_shared_data, soc_info.feature_id) == 0x248U,
	assert_cpucp_feature_id_offset);
CASSERT(offsetof(struct cpucp_shared_data, soc_info.jtag_id) == 0x24cU,
	assert_cpucp_jtag_id_offset);
CASSERT(offsetof(struct cpucp_shared_data, soc_info.vp_id) == 0x250U,
	assert_cpucp_vp_id_offset);
CASSERT(offsetof(struct cpucp_shared_data, soc_info.soft_sku_lval) == 0x254U,
	assert_cpucp_soft_sku_lval_offset);
CASSERT(offsetof(struct cpucp_shared_data, cpr_info.local_rev) == 0x264U,
	assert_cpucp_local_rev_offset);
CASSERT(offsetof(struct cpucp_shared_data, cpr_info.aging) == 0x265U,
	assert_cpucp_aging_offset);
CASSERT(offsetof(struct cpucp_shared_data, cpr_info.targ_volt) == 0x2f6U,
	assert_cpucp_targ_volt_offset);
CASSERT(offsetof(struct cpucp_shared_data, cpr_info.quot_offset) == 0x3f6U,
	assert_cpucp_quot_offset_offset);
CASSERT(offsetof(struct cpucp_shared_data, cpr_info.quot_vmin) == 0x4f6U,
	assert_cpucp_quot_vmin_offset);
CASSERT(offsetof(struct cpucp_shared_data, acc_info) == 0x5f6U,
	assert_cpucp_acc_info_offset);
CASSERT(sizeof(struct cpucp_acc_info) == 9U, assert_cpucp_acc_info_size);
CASSERT(offsetof(struct cpucp_shared_data, mx_cmd_db.addr) == 0x61cU,
	assert_cpucp_mx_cmd_db_addr_offset);
CASSERT(offsetof(struct cpucp_shared_data, mx_cmd_db.len) == 0x620U,
	assert_cpucp_mx_cmd_db_len_offset);
CASSERT(offsetof(struct cpucp_shared_data, mx_cmd_db.data) == 0x622U,
	assert_cpucp_mx_cmd_db_data_offset);
CASSERT(offsetof(struct cpucp_shared_data, is_slt_plat) == 0x644U,
	assert_cpucp_is_slt_plat_offset);
CASSERT(sizeof(struct cpucp_shared_data) == 0x648U,
	assert_cpucp_shared_data_size);

static const struct apss_clk apss_clks[] = {
	{ APSS_AHB_CLK_CFG,           8U, 4U, APSS_AHB_CLK_CFG,             5U },
	{ APSS_LMH_CDIV_COUNT,        0U, 3U, APSS_LMH_GFMUX_CFG,           5U },
	{ APSS_OSM_CDIV_COUNT,        0U, 3U, APSS_OSM_GFMUX_CFG,           1U },
	{ APSS_PERIPH_CDIV_COUNT,     0U, 4U, APSS_PERIPHCLK_GFMUX_CFG,     1U },
	{ APSS_CL1_PERIPH_CDIV_COUNT, 0U, 4U, APSS_CL1_PERIPHCLK_GFMUX_CFG, 1U },
	{ APSS_IPM_CDIV_COUNT,        0U, 4U, APSS_IPM_GFMUX_CFG,           3U },
};

/*
 * CPR instances 12 to 15 serve, in order, the cluster 0 L3 and gold domains on
 * the APC0 rail and the cluster 1 L3 and gold domains on APC1.
 */
#define FIRST_APSS_CPR		12U
#define NUM_APSS_CPR		4U

/* Closed-loop quotient offsets, fused in units of 5. */
#define QUOT_OFFSET_STEP	5U

/* Open-loop target voltage codes: 8 mV steps in bits [4:0], sign in bit 5. */
static const struct fuse_field
targ_volt_fuses[NUM_APSS_CPR][CPUCP_NUM_FUSED_CORNERS] = {
	{
		F(ROW_LSB(7), 19U, 6U), F(ROW_LSB(7), 13U, 6U),
		F(ROW_LSB(7), 7U, 6U), F(ROW_LSB(7), 1U, 6U),
	}, {
		F(ROW_MSB(7), 11U, 6U), F(ROW_MSB(7), 5U, 6U),
		F2(ROW_LSB(7), 31U, 1U, ROW_MSB(7), 0U, 5U),
		F(ROW_LSB(7), 25U, 6U),
	}, {
		F(ROW_LSB(8), 3U, 6U),
		F2(ROW_MSB(7), 29U, 3U, ROW_LSB(8), 0U, 3U),
		F(ROW_MSB(7), 23U, 6U), F(ROW_MSB(7), 17U, 6U),
	}, {
		F2(ROW_LSB(8), 27U, 5U, ROW_MSB(8), 0U, 1U),
		F(ROW_LSB(8), 21U, 6U), F(ROW_LSB(8), 15U, 6U),
		F(ROW_LSB(8), 9U, 6U),
	},
};

static const struct fuse_field
quot_offset_fuses[NUM_APSS_CPR][CPUCP_NUM_FUSED_CORNERS] = {
	{
		F(ROW_MSB(11), 24U, 7U), F(ROW_MSB(11), 17U, 7U),
		F(ROW_MSB(11), 9U, 8U), F(ROW_MSB(11), 1U, 8U),
	}, {
		F(ROW_LSB(12), 22U, 7U), F(ROW_LSB(12), 15U, 7U),
		F(ROW_LSB(12), 7U, 8U),
		F2(ROW_MSB(11), 31U, 1U, ROW_LSB(12), 0U, 7U),
	}, {
		F(ROW_MSB(12), 20U, 7U), F(ROW_MSB(12), 13U, 7U),
		F(ROW_MSB(12), 5U, 8U),
		F2(ROW_LSB(12), 29U, 3U, ROW_MSB(12), 0U, 5U),
	}, {
		F(ROW_LSB(13), 18U, 7U), F(ROW_LSB(13), 11U, 7U),
		F(ROW_LSB(13), 3U, 8U),
		F2(ROW_MSB(12), 27U, 5U, ROW_LSB(13), 0U, 3U),
	},
};

static const struct fuse_field
quot_vmin_fuses[NUM_APSS_CPR][CPUCP_NUM_FUSED_CORNERS] = {
	{
		F(ROW_LSB(9), 5U, 12U),
		F2(ROW_MSB(8), 25U, 7U, ROW_LSB(9), 0U, 5U),
		F(ROW_MSB(8), 13U, 12U), F(ROW_MSB(8), 1U, 12U),
	}, {
		F2(ROW_MSB(9), 21U, 11U, ROW_LSB(10), 0U, 1U),
		F(ROW_MSB(9), 9U, 12U),
		F2(ROW_LSB(9), 29U, 3U, ROW_MSB(9), 0U, 9U),
		F(ROW_LSB(9), 17U, 12U),
	}, {
		F(ROW_MSB(10), 5U, 12U),
		F2(ROW_LSB(10), 25U, 7U, ROW_MSB(10), 0U, 5U),
		F(ROW_LSB(10), 13U, 12U), F(ROW_LSB(10), 1U, 12U),
	}, {
		F2(ROW_LSB(11), 21U, 11U, ROW_MSB(11), 0U, 1U),
		F(ROW_LSB(11), 9U, 12U),
		F2(ROW_MSB(10), 29U, 3U, ROW_LSB(11), 0U, 9U),
		F(ROW_MSB(10), 17U, 12U),
	},
};

/* Each rail has one aging fuse shared by its CPR instances. */
static const struct fuse_field aging_fuses[NUM_APSS_CPR] = {
	F(ROW_MSB(22), 0U, 8U), F(ROW_MSB(22), 0U, 8U),
	F(ROW_MSB(22), 8U, 8U), F(ROW_MSB(22), 8U, 8U),
};

static const struct cpucp_config lemans_cpucp_config = {
	.apss_clks = apss_clks,
	.num_apss_clks = ARRAY_SIZE(apss_clks),
	.cpr = {
		.targ_volt = &targ_volt_fuses[0][0],
		.quot_offset = &quot_offset_fuses[0][0],
		.quot_vmin = &quot_vmin_fuses[0][0],
		.aging = aging_fuses,
		.first_cpr = FIRST_APSS_CPR,
		.num_cpr = NUM_APSS_CPR,
		.quot_offset_step = QUOT_OFFSET_STEP,
		.num_acc_domains = CD_MAX,
	},
	.secure_access_reg = EPSSTOP_SECURE_ACCESS_OVERRIDE,
	.secure_access_en = EPSSTOP_SECURE_ACCESS_OVERRIDE_EN,
};

const struct cpucp_config *cpucp_get_config(void)
{
	return &lemans_cpucp_config;
}

void cpucp_fill_soc_info(void)
{
	mmio_write_32(SHARED(soc_info.chip_version),
		      (cpucp_read_bits(QTI_SOC_REVISION_REG, 8U, 8U) << 16) |
		      cpucp_read_bits(QTI_SOC_REVISION_REG, 0U, 8U));
	mmio_write_32(SHARED(soc_info.foundry_id),
		      cpucp_read_bits(QFPROM_CORR_FEATURE_CONFIG_NM_ROW2_LSB,
				      6U, 4U));
	mmio_write_32(SHARED(soc_info.speed_bin),
		      cpucp_read_bits(QFPROM_CORR_PTE_ROW0_LSB, 29U, 3U));
	mmio_write_32(SHARED(soc_info.feature_id),
		      cpucp_read_bits(QFPROM_CORR_PTE_ROW0_LSB, 20U, 8U));
	mmio_write_32(SHARED(soc_info.jtag_id),
		      cpucp_read_bits(QFPROM_CORR_PTE_ROW0_LSB, 0U, 20U));
	mmio_write_32(SHARED(soc_info.vp_id),
		      cpucp_read_bits(QFPROM_CORR_QC_SPARE_20_MSB, 4U, 4U));

	/* The L3 domains have no frequency cap fuse. */
	mmio_write_32(SHARED(soc_info.soft_sku_lval[CD_CL0_L3]), 0U);
	mmio_write_32(SHARED(soc_info.soft_sku_lval[CD_CL1_L3]), 0U);
	mmio_write_32(SHARED(soc_info.soft_sku_lval[CD_CL0_GOLD]),
		      cpucp_soft_sku_lval(
			      cpucp_read_bits(SOFT_SKU_APC0_FREQ, 8U, 1U),
			      cpucp_read_bits(SOFT_SKU_APC0_FREQ, 0U, 8U)));
	mmio_write_32(SHARED(soc_info.soft_sku_lval[CD_CL1_GOLD]),
		      cpucp_soft_sku_lval(
			      cpucp_read_bits(SOFT_SKU_APC1_FREQ, 8U, 1U),
			      cpucp_read_bits(SOFT_SKU_APC1_FREQ, 0U, 8U)));
}

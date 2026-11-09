/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <cpu_config.h>
#include <cpucp.h>
#include <cpucp_config.h>
#include <cpucp_hwio.h>
#include <cpucp_shared.h>
#include <drivers/qti/cmd_db/cmd_db.h>
#include <lib/cassert.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

/*
 * CPUCP start for the version 3 host interface of Kodiak. The shared data
 * carries a ring oscillator select per CPR corner, a firmware configuration
 * block and the cores present in each clock domain, and the fused CPR
 * corners differ per CPR instance, with most quotient offsets derived from
 * the corner below, so this does not use the common hoya start sequence.
 */

CASSERT(offsetof(struct cpucp_shared_data, ns_logbuf_access_en) == 0x21cU,
	assert_cpucp_ns_logbuf_access_en_offset);
CASSERT(offsetof(struct cpucp_shared_data, soc_info) == 0x220U,
	assert_cpucp_soc_info_offset);
CASSERT(offsetof(struct cpucp_shared_data, cpr_info.local_rev) == 0x238U,
	assert_cpucp_local_rev_offset);
CASSERT(offsetof(struct cpucp_shared_data, cpr_info.rosel) == 0x249U,
	assert_cpucp_rosel_offset);
CASSERT(offsetof(struct cpucp_shared_data, cpr_info.targ_volt) == 0x2caU,
	assert_cpucp_targ_volt_offset);
CASSERT(offsetof(struct cpucp_shared_data, cpr_info.quot_offset) == 0x3caU,
	assert_cpucp_quot_offset_offset);
CASSERT(offsetof(struct cpucp_shared_data, cpr_info.quot_vmin) == 0x4caU,
	assert_cpucp_quot_vmin_offset);
CASSERT(offsetof(struct cpucp_shared_data, acc_info) == 0x5caU,
	assert_cpucp_acc_info_offset);
CASSERT(offsetof(struct cpucp_shared_data, mx_cmd_db) == 0x5f0U,
	assert_cpucp_mx_cmd_db_offset);
CASSERT(offsetof(struct cpucp_shared_data, fw_config) == 0x614U,
	assert_cpucp_fw_config_offset);
CASSERT(offsetof(struct cpucp_shared_data, clkdom_cpumasks) == 0x624U,
	assert_cpucp_clkdom_cpumasks_offset);
CASSERT(sizeof(struct cpucp_shared_data) == 0x638U,
	assert_cpucp_shared_data_size);

/* About 1 ms of the 19.2 MHz reference clock. */
#define CPUCP_HANG_THRESHOLD		0x4b1eU

#define GFMUX_SRC_SEL_MASK		GENMASK_32(1, 0)
#define GFMUX_SRC_GPLL0			1U

/* Fused quotient offsets are in steps of 5. */
#define QUOT_OFFSET_STEP		5U

#define MAX_CPR_CORNERS			5U

static const struct apss_clk apss_clks[] = {
	{ APSS_AHB_CLK_CFG,       8U, 4U, APSS_AHB_CLK_CFG,         2U },
	{ APSS_LMH_CDIV_COUNT,    0U, 3U, APSS_LMH_GFMUX_CFG,       2U },
	{ APSS_OSM_CDIV_COUNT,    0U, 3U, APSS_OSM_GFMUX_CFG,       0U },
	{ APSS_PERIPH_CDIV_COUNT, 0U, 4U, APSS_PERIPHCLK_GFMUX_CFG, 0U },
	{ APSS_IPM_CDIV_COUNT,    0U, 4U, APSS_IPM_GFMUX_CFG,       1U },
};

/*
 * One CPR corner. A corner with a fused offset uses it; any other corner after
 * the first gets the difference to the previous corner's fused quotient, which
 * is only valid when both use the same ring oscillator. The first corner has
 * no offset.
 */
struct cpr_corner {
	uint8_t corner;
	uint8_t rosel;
	struct fuse_field targ_volt;
	struct fuse_field quot_vmin;
	struct fuse_field quot_offset;
};

struct cpr_instance {
	unsigned int inst;
	struct fuse_field aging;
	unsigned int num_corners;
	struct cpr_corner corners[MAX_CPR_CORNERS];
};

#define NONE		{ { 0U, 0U, 0U }, { 0U, 0U, 0U } }

/*
 * CPR 11 and 12 serve the L3 and silver domains on the APC0 rail, 13 the
 * gold+ and 14 the gold domain on APC1.
 */
static const struct cpr_instance cpr_instances[] = {
	{ 11U, F(ROW_MSB(7), 10U, 8U), 4U, {
		{ CPUCP_CORNER_SVS2, 8U, F(ROW_LSB(3), 13U, 6U),
		  F(ROW_LSB(7), 8U, 12U), NONE },
		{ CPUCP_CORNER_SVSL1, 8U,
		  F2(ROW_LSB(6), 30U, 2U, ROW_MSB(6), 0U, 4U),
		  F2(ROW_MSB(6), 28U, 4U, ROW_LSB(7), 0U, 8U), NONE },
		{ CPUCP_CORNER_NOML1, 8U, F(ROW_LSB(6), 24U, 6U),
		  F(ROW_MSB(6), 16U, 12U), NONE },
		{ CPUCP_CORNER_TURL1, 8U, F(ROW_LSB(6), 18U, 6U),
		  F(ROW_MSB(6), 4U, 12U), NONE },
	} },
	{ 12U, F(ROW_MSB(7), 10U, 8U), 4U, {
		{ CPUCP_CORNER_SVS2, 8U, F(ROW_LSB(8), 4U, 6U),
		  F(ROW_MSB(8), 14U, 12U), NONE },
		{ CPUCP_CORNER_SVSL1, 8U,
		  F2(ROW_MSB(7), 30U, 2U, ROW_LSB(8), 0U, 4U),
		  F(ROW_MSB(8), 2U, 12U), NONE },
		{ CPUCP_CORNER_NOML1, 9U, F(ROW_MSB(7), 24U, 6U),
		  F2(ROW_LSB(8), 22U, 10U, ROW_MSB(8), 0U, 2U),
		  F2(ROW_MSB(8), 26U, 6U, ROW_LSB(9), 0U, 2U) },
		{ CPUCP_CORNER_TURL1, 9U, F(ROW_MSB(7), 18U, 6U),
		  F(ROW_LSB(8), 10U, 12U), NONE },
	} },
	{ 13U, F2(ROW_LSB(11), 28U, 4U, ROW_MSB(11), 0U, 4U), 5U, {
		{ CPUCP_CORNER_SVS2, 13U,
		  F2(ROW_MSB(9), 28U, 4U, ROW_LSB(10), 0U, 2U),
		  F(ROW_MSB(10), 18U, 12U), NONE },
		{ CPUCP_CORNER_SVSL1, 13U, F(ROW_MSB(9), 22U, 6U),
		  F(ROW_MSB(10), 6U, 12U), NONE },
		{ CPUCP_CORNER_NOML1, 11U, F(ROW_MSB(9), 16U, 6U),
		  F2(ROW_LSB(10), 26U, 6U, ROW_MSB(10), 0U, 6U),
		  F(ROW_LSB(11), 14U, 8U) },
		{ CPUCP_CORNER_TURL1, 11U, F(ROW_MSB(9), 10U, 6U),
		  F(ROW_LSB(10), 14U, 12U), NONE },
		{ CPUCP_CORNER_TURL3, 11U, F(ROW_MSB(9), 4U, 6U),
		  F(ROW_LSB(10), 2U, 12U), NONE },
	} },
	{ 14U, F2(ROW_LSB(11), 28U, 4U, ROW_MSB(11), 0U, 4U), 4U, {
		{ CPUCP_CORNER_SVS2, 8U, F(ROW_MSB(11), 22U, 6U),
		  F(ROW_MSB(12), 0U, 12U), NONE },
		{ CPUCP_CORNER_SVSL1, 8U, F(ROW_MSB(11), 16U, 6U),
		  F(ROW_LSB(12), 20U, 12U), NONE },
		{ CPUCP_CORNER_NOML1, 8U, F(ROW_MSB(11), 10U, 6U),
		  F(ROW_LSB(12), 8U, 12U), NONE },
		{ CPUCP_CORNER_TURL3, 8U, F(ROW_MSB(11), 4U, 6U),
		  F2(ROW_MSB(11), 28U, 4U, ROW_LSB(12), 0U, 8U), NONE },
	} },
};

/* Firmware configuration for Linux on QCM6490: no per-core L3 voting. */
static const struct cpucp_fw_config fw_config = {
	.version = 3U,
	.size = (uint8_t)sizeof(struct cpucp_fw_config),
	.acd_enable = 1U,
	.acd_margin_enable = 1U,
	.aging_enable = 0U,
	.aging_margin_enable = 1U,
	.avg_enable = 0U,
	.apm_auto_enable = 1U,
	.per_core_dcvs_enable = 0U,
	.pmfs_enable = 1U,
	.l3_per_core_voting_enable = 0U,
	.ns_irq_in_cpuss_enable = 1U,
	.gold_boot_boost_enable = 0U,
};

static uint32_t read_bits(uintptr_t reg, unsigned int lsb, unsigned int width)
{
	return (mmio_read_32(reg) >> lsb) & (BIT_32(width) - 1U);
}

static uint32_t fuse_read(const struct fuse_field *f)
{
	uint32_t val = read_bits(f->lo.reg, f->lo.lsb, f->lo.width);

	if (f->hi.width != 0U) {
		val |= read_bits(f->hi.reg, f->hi.lsb, f->hi.width) <<
		       f->lo.width;
	}

	return val;
}

static int16_t targ_volt_mv(uint32_t code)
{
	int16_t mv = (int16_t)((code & 0x1fU) * 8U);

	return ((code & BIT_32(5)) != 0U) ? (int16_t)-mv : mv;
}

static void cpucp_set_clocks(void)
{
	for (unsigned int i = 0U; i < ARRAY_SIZE(apss_clks); i++) {
		const struct apss_clk *clk = &apss_clks[i];

		mmio_clrsetbits_32(clk->cdiv,
				   (BIT_32(clk->cdiv_width) - 1U) <<
				   clk->cdiv_lsb,
				   clk->div << clk->cdiv_lsb);
		mmio_clrsetbits_32(clk->gfmux, GFMUX_SRC_SEL_MASK,
				   GFMUX_SRC_GPLL0);
	}
}

static void cpucp_fill_chip_info(void)
{
	mmio_write_32(SHARED(soc_info.chip_version),
		      (read_bits(TCSR_SOC_HW_VERSION, 8U, 8U) << 16) |
		      read_bits(TCSR_SOC_HW_VERSION, 0U, 8U));
	mmio_write_32(SHARED(soc_info.foundry_id),
		      read_bits(QFPROM_CORR_FEATURE_CONFIG_NM_ROW2_LSB, 6U, 4U));
	mmio_write_32(SHARED(soc_info.speed_bin),
		      read_bits(QFPROM_CORR_PTE_ROW0_LSB, 29U, 3U));
	mmio_write_32(SHARED(soc_info.feature_id),
		      read_bits(QFPROM_CORR_PTE_ROW0_LSB, 20U, 8U));
	mmio_write_32(SHARED(soc_info.jtag_id),
		      read_bits(QFPROM_CORR_PTE_ROW0_LSB, 0U, 20U));
	mmio_write_32(SHARED(soc_info.vp_id),
		      read_bits(QFPROM_RAW_QC_SPARE_21_MSB, 0U, 4U));
}

static void cpucp_fill_cpr_info(void)
{
	mmio_write_8(SHARED(cpr_info.local_rev),
		     (uint8_t)read_bits(QFPROM_RAW_CALIBRATION_ROW12_MSB, 29U, 3U));

	for (unsigned int i = 0U; i < ARRAY_SIZE(cpr_instances); i++) {
		const struct cpr_instance *cpr = &cpr_instances[i];
		uint32_t prev_vmin = 0U;

		mmio_write_8(SHARED(cpr_info.aging[cpr->inst]),
			     (uint8_t)fuse_read(&cpr->aging));

		for (unsigned int c = 0U; c < cpr->num_corners; c++) {
			const struct cpr_corner *cr = &cpr->corners[c];
			uint32_t vmin = fuse_read(&cr->quot_vmin);

			mmio_write_8(SHARED(cpr_info.rosel[cpr->inst][cr->corner]),
				     cr->rosel);
			mmio_write_16(SHARED(cpr_info.targ_volt[cpr->inst][cr->corner]),
				      (uint16_t)targ_volt_mv(fuse_read(&cr->targ_volt)));
			mmio_write_16(SHARED(cpr_info.quot_vmin[cpr->inst][cr->corner]),
				      (uint16_t)vmin);

			/*
			 * The first corner has no lower corner to be offset
			 * from: its slot keeps the value the firmware image
			 * carries, as with QTISECLIB.
			 */
			if (c != 0U) {
				uint32_t off = (cr->quot_offset.lo.width != 0U) ?
					       fuse_read(&cr->quot_offset) *
					       QUOT_OFFSET_STEP :
					       vmin - prev_vmin;

				mmio_write_16(SHARED(cpr_info.quot_offset[cpr->inst][cr->corner]),
					      (uint16_t)off);
			}

			prev_vmin = vmin;
		}
	}

	mmio_write_8(SHARED(acc_info[CD_L3].acc_fuse), 0U);
	mmio_write_8(SHARED(acc_info[CD_SILVER].acc_fuse), 0U);
	mmio_write_8(SHARED(acc_info[CD_GOLD].acc_fuse), 0U);
}

/* The firmware votes the MX rail for L3 from the levels in the command DB. */
static int cpucp_fill_mx_levels(void)
{
	uint8_t data[sizeof(((struct cpucp_cmd_db *)NULL)->data)];
	uint8_t len = (uint8_t)sizeof(data);
	uint32_t addr = cmd_db_query_addr("mx.lvl");

	if ((addr == 0U) || (cmd_db_query_aux_data("mx.lvl", &len, data) != 0)) {
		return -1;
	}

	mmio_write_32(SHARED(mx_cmd_db.addr), addr);
	mmio_write_16(SHARED(mx_cmd_db.len), len);
	for (unsigned int i = 0U; i < len; i++) {
		mmio_write_8(SHARED(mx_cmd_db.data) + i, data[i]);
	}

	return 0;
}

static void cpucp_fill_fw_config(void)
{
	const uint8_t *src = (const uint8_t *)&fw_config;

	for (unsigned int i = 0U; i < sizeof(fw_config); i++) {
		mmio_write_8(SHARED(fw_config) + i, src[i]);
	}
}

/* Drop the cores disabled in the fuses from their clock domains. */
static void cpucp_fill_cpumasks(void)
{
	uint32_t disabled = read_bits(QFPROM_RAW_PTE_ROW1_MSB, 7U, 8U);
	uint32_t all = mmio_read_32(SHARED(cpumasks));

	for (unsigned int i = 0U; i < CD_MAX; i++) {
		clkdom_cpumasks[i].cpumask &= ~disabled;
		mmio_write_32(SHARED(clkdom_cpumasks[i]),
			      clkdom_cpumasks[i].cpumask);
		all |= clkdom_cpumasks[i].cpumask;
	}

	mmio_write_32(SHARED(cpumasks), all);
}

/*
 * The boot firmware loads CPUCP and hands over with it not started and
 * with EPSS open to non-secure writes for that load. Booting on without a
 * started CPUCP leaves the CPU clocks and the EPSS access in that state, so
 * a CPUCP that cannot be started stops the boot.
 */
void qti_cpucp_init(void)
{
	uint32_t version = mmio_read_32(SHARED(version));

	if (version != CPUCP_SHARED_VERSION) {
		ERROR("CPUCP: unsupported firmware interface %u\n", version);
		panic();
	}

	cpucp_set_clocks();

	mmio_write_32(EPSSTOP_MUC_HANG_COUNT_THRESHOLD, CPUCP_HANG_THRESHOLD);
	mmio_setbits_32(EPSSTOP_MUC_HANG_DET_CTRL,
			EPSSTOP_MUC_HANG_DET_CTRL_IRQ_EN);

	cpucp_fill_cpumasks();
	cpucp_fill_chip_info();
	cpucp_fill_cpr_info();
	cpucp_fill_fw_config();
	if (cpucp_fill_mx_levels() != 0) {
		ERROR("CPUCP: no MX levels in the command DB\n");
		panic();
	}

	/* Close the EPSS override the boot firmware opened to load CPUCP. */
	mmio_clrbits_32(EPSSTOP_SECURE_ACCESS_OVERRIDE,
			EPSSTOP_SECURE_ACCESS_OVERRIDE_EN);
	mmio_write_32(EPSSTOP_L3_VOTING_EN, 0U);
	mmio_setbits_32(EPSSTOP_GLOBAL_ENABLE, 1U);
	mmio_setbits_32(EPSSFAST_EPSS_MUC_CLK_CTRL,
			EPSS_MUC_CLK_CTRL_CORE_CLK_EN);
}

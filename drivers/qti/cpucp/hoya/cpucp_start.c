/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <cpucp.h>
#include <cpucp_config.h>
#include <cpucp_hwio.h>
#include <cpucp_shared.h>
#include <drivers/qti/cmd_db/cmd_db.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <platform_def.h>

/* About 1 ms of the 19.2 MHz reference clock. */
#define CPUCP_HANG_THRESHOLD		0x4b1eU

#define GFMUX_SRC_SEL_MASK		GENMASK_32(1, 0)
#define GFMUX_SRC_GPLL0			1U

uint32_t cpucp_read_bits(uintptr_t reg, unsigned int lsb, unsigned int width)
{
	return (mmio_read_32(reg) >> lsb) & (BIT_32(width) - 1U);
}

static uint32_t fuse_read(const struct fuse_field *f)
{
	uint32_t val = cpucp_read_bits(f->lo.reg, f->lo.lsb, f->lo.width);

	if (f->hi.width != 0U) {
		val |= cpucp_read_bits(f->hi.reg, f->hi.lsb, f->hi.width) <<
		       f->lo.width;
	}

	return val;
}

static int16_t targ_volt_mv(uint32_t code)
{
	int16_t mv = (int16_t)((code & 0x1fU) * 8U);

	return ((code & BIT_32(5)) != 0U) ? (int16_t)-mv : mv;
}

/*
 * A fused frequency cap is passed as the complement of its L value, or as
 * 0xfff when the part has none.
 */
uint32_t cpucp_soft_sku_lval(uint32_t enabled, uint32_t lval)
{
	return (enabled != 0U) ? (~lval | 0xffffff00U) : 0xfffU;
}

static uint32_t soc_major_version(void)
{
	return cpucp_read_bits(QTI_SOC_REVISION_REG, 8U, 8U);
}

static void cpucp_set_clocks(const struct cpucp_config *cfg)
{
	for (unsigned int i = 0U; i < cfg->num_apss_clks; i++) {
		const struct apss_clk *clk = &cfg->apss_clks[i];

		mmio_clrsetbits_32(clk->cdiv,
				   (BIT_32(clk->cdiv_width) - 1U) <<
				   clk->cdiv_lsb,
				   clk->div << clk->cdiv_lsb);
		mmio_clrsetbits_32(clk->gfmux, GFMUX_SRC_SEL_MASK,
				   GFMUX_SRC_GPLL0);
	}
}

static void cpucp_fill_cpr_info(const struct cpucp_cpr_config *cpr)
{
	uintptr_t targ_volt = SHARED(cpr_info.targ_volt);
	uintptr_t quot_offset = SHARED(cpr_info.quot_offset);
	uintptr_t quot_vmin = SHARED(cpr_info.quot_vmin);

	mmio_write_8(SHARED(cpr_info.local_rev),
		     (uint8_t)cpucp_read_bits(ROW_MSB(22), 27U, 3U));

	for (unsigned int i = 0U; i < cpr->num_cpr; i++) {
		unsigned int inst = cpr->first_cpr + i;

		mmio_write_8(SHARED(cpr_info.aging) + inst,
			     (uint8_t)fuse_read(&cpr->aging[i]));

		for (unsigned int c = 0U; c < CPUCP_NUM_FUSED_CORNERS; c++) {
			unsigned int f = (i * CPUCP_NUM_FUSED_CORNERS) + c;
			uintptr_t idx = ((inst * CPUCP_NUM_CORNERS) + c) *
					sizeof(uint16_t);
			uint16_t off = 0U;

			if (cpr->quot_offset != NULL) {
				off = (uint16_t)(fuse_read(&cpr->quot_offset[f]) *
						 cpr->quot_offset_step);
			}

			mmio_write_16(targ_volt + idx, (uint16_t)targ_volt_mv(
				fuse_read(&cpr->targ_volt[f])));
			mmio_write_16(quot_offset + idx, off);
			mmio_write_16(quot_vmin + idx,
				      (uint16_t)fuse_read(&cpr->quot_vmin[f]));
		}
	}

	for (unsigned int d = 0U; d < cpr->num_acc_domains; d++) {
		mmio_write_8(SHARED(acc_info[0].acc_fuse) +
			     (d * sizeof(struct cpucp_acc_info)), 0U);
	}
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

void qti_cpucp_init(void)
{
	const struct cpucp_config *cfg = cpucp_get_config();
	uint32_t version = mmio_read_32(SHARED(version));

	if (version != CPUCP_SHARED_VERSION) {
		ERROR("CPUCP: unsupported firmware interface %u\n", version);
		return;
	}

	cpucp_set_clocks(cfg);

	mmio_write_32(EPSSTOP_MUC_HANG_COUNT_THRESHOLD, CPUCP_HANG_THRESHOLD);
	mmio_setbits_32(EPSSTOP_MUC_HANG_DET_CTRL,
			EPSSTOP_MUC_HANG_DET_CTRL_IRQ_EN);

	cpucp_fill_soc_info();
	cpucp_fill_cpr_info(&cfg->cpr);
	if (cpucp_fill_mx_levels() != 0) {
		ERROR("CPUCP: no MX levels in the command DB\n");
		return;
	}

	/* SLT status unknown: there is no platform info service. */
	mmio_write_32(SHARED(is_slt_plat), (uint32_t)-1);

	/*
	 * The boot firmware opens EPSS to non-secure writes to load CPUCP.
	 * Only version 1 silicon keeps that for kernel L3 voting; later parts
	 * grant it through a dedicated L3 voting permission instead.
	 */
	if (cfg->secure_access_reg != 0U) {
		mmio_clrsetbits_32(cfg->secure_access_reg, cfg->secure_access_en,
				   (soc_major_version() < 2U) ?
				   cfg->secure_access_en : 0U);
	}

	mmio_write_32(EPSSTOP_L3_VOTING_EN, 1U);
	mmio_setbits_32(EPSSTOP_GLOBAL_ENABLE, 1U);
	mmio_setbits_32(EPSSFAST_EPSS_MUC_CLK_CTRL,
			EPSS_MUC_CLK_CTRL_CORE_CLK_EN);
}

/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <common/debug.h>
#include <ddr.h>
#include <lib/utils.h>

static inline unsigned int cal_cwl(const unsigned long clk)
{
	const unsigned int mclk_ps = get_memory_clk_ps(clk);

	return mclk_ps >= 1250U ? 9U :
		(mclk_ps >= 1070U ? 10U :
		 (mclk_ps >= 935U ? 11U :
		  (mclk_ps >= 833U ? 12U :
		   (mclk_ps >= 750U ? 14U :
		    (mclk_ps >= 625U ? 16U : 18U)))));
}

static void cal_csn_config(int i,
			   struct ddr_cfg_regs *regs,
			   const struct memctl_opt *popts,
			   const struct dimm_params *pdimm)
{
	unsigned int intlv_en = 0U;
	unsigned int intlv_ctl = 0U;
	const unsigned int cs_n_en = 1U;
	const unsigned int ap_n_en = popts->cs_odt[i].auto_precharge;
	const unsigned int odt_rd_cfg = popts->cs_odt[i].odt_rd_cfg;
	const unsigned int odt_wr_cfg = popts->cs_odt[i].odt_wr_cfg;
	const unsigned int ba_bits_cs_n = pdimm->bank_addr_bits;
	const unsigned int row_bits_cs_n = pdimm->n_row_addr - 12U;
	const unsigned int col_bits_cs_n = pdimm->n_col_addr - 8U;
	const unsigned int bg_bits_cs_n = pdimm->bank_group_bits;

	if (i == 0) {
		/* These fields only available in CS0_CONFIG */
		if (popts->ctlr_intlv != 0) {
			switch (popts->ctlr_intlv_mode) {
			case DDR_256B_INTLV:
				intlv_en = popts->ctlr_intlv;
				intlv_ctl = popts->ctlr_intlv_mode;
				break;
			default:
				break;
			}
		}
	}
	regs->cs[i].config = ((cs_n_en & 0x1) << 31)		|
			    ((intlv_en & 0x3) << 29)		|
			    ((intlv_ctl & 0xf) << 24)		|
			    ((ap_n_en & 0x1) << 23)		|
			    ((odt_rd_cfg & 0x7) << 20)		|
			    ((odt_wr_cfg & 0x7) << 16)		|
			    ((ba_bits_cs_n & 0x3) << 14)	|
			    ((row_bits_cs_n & 0x7) << 8)	|
			    ((bg_bits_cs_n & 0x3) << 4)		|
			    ((col_bits_cs_n & 0x7) << 0);
	debug("cs%d\n", i);
	debug("   _config = 0x%x\n", regs->cs[i].config);
}

static inline int avoid_odt_overlap(const struct ddr_conf *conf,
				    const struct dimm_params *pdimm)
{
	if ((conf->cs_in_use == 0xf) != 0) {
		return 2;
	}

#if DDRC_NUM_DIMM >= 2
	if (conf->dimm_in_use[0] != 0 && conf->dimm_in_use[1] != 0) {
		return 1;
	}
#endif
	return 0;
}

/* Requires rcw2 set first */
static void cal_timing_cfg(const unsigned long clk,
			   struct ddr_cfg_regs *regs,
			   const struct memctl_opt *popts,
			   const struct dimm_params *pdimm,
			   const struct ddr_conf *conf,
			   unsigned int cas_latency,
			   unsigned int additive_latency)
{
	const unsigned int mclk_ps = get_memory_clk_ps(clk);
	/* tXP=max(4nCK, 6ns) */
	const int txp = max((int)mclk_ps * 4, 6000);
	/* DDR4 supports 10, 12, 14, 16, 18, 20, 24 */
	static const int wrrec_table[] = {
		10, 10, 10, 10, 10,
		10, 10, 10, 10, 10,
		12, 12, 14, 14, 16,
		16, 18, 18, 20, 20,
		24, 24, 24, 24,
	};
	int trwt_mclk = (clk / 1000000 > 1900) ? 3 : 2;
	int twrt_mclk;
	int trrt_mclk;
	int twwt_mclk;
	const int act_pd_exit_mclk = picos_to_mclk(clk, txp);
	const int pre_pd_exit_mclk = act_pd_exit_mclk;
	const int taxpd_mclk = 0;
	/*
	 * MRS_CYC = max(tMRD, tMOD)
	 * tMRD = 8nCK, tMOD = max(24nCK, 15ns)
	 */
	const int tmrd_mclk = max(24U, picos_to_mclk(clk, 15000));
	const int pretoact_mclk = picos_to_mclk(clk, pdimm->trp_ps);
	const int acttopre_mclk = picos_to_mclk(clk, pdimm->tras_ps);
	const int acttorw_mclk = picos_to_mclk(clk, pdimm->trcd_ps);
	const int caslat_ctrl = (cas_latency - 1) << 1;
	const int trfc1_min = pdimm->die_density >= 0x3 ? 16000 :
			      (pdimm->die_density == 0x4 ? 26000 :
			       (pdimm->die_density == 0x5 ? 35000 :
				55000));
	const int refrec_ctrl = picos_to_mclk(clk,
							pdimm->trfc1_ps) - 8;
	int wrrec_mclk = picos_to_mclk(clk, pdimm->twr_ps);
	const int acttoact_mclk = max(picos_to_mclk(clk,
							      pdimm->trrds_ps),
						4U);
	int wrtord_mclk = max(2U, picos_to_mclk(clk, 2500));
	const unsigned int cpo = 0U;
	const int wr_lat = cal_cwl(clk);
	int rd_to_pre = picos_to_mclk(clk, 7500);
	const int wr_data_delay = popts->wr_data_delay;
	const int cke_pls = max(3U, picos_to_mclk(clk, 5000));
#ifdef ERRATA_DDR_A050450
	const unsigned short four_act = ((popts->twot_en == 0) &&
					 (popts->threet_en == 0) &&
					 (popts->tfaw_ps % 2 == 0)) ?
						(picos_to_mclk(clk, popts->tfaw_ps) + 1) :
						picos_to_mclk(clk, popts->tfaw_ps);
#else
	const unsigned short four_act = picos_to_mclk(clk,
					 popts->tfaw_ps);
#endif
	const unsigned int cntl_adj = 0U;
	const unsigned int ext_pretoact = picos_to_mclk(clk,
							pdimm->trp_ps) >> 4U;
	const unsigned int ext_acttopre = picos_to_mclk(clk,
							pdimm->tras_ps) >> 4U;
	const unsigned int ext_acttorw = picos_to_mclk(clk,
						       pdimm->trcd_ps) >> 4U;
	const unsigned int ext_caslat = (2U * cas_latency - 1U) >> 4U;
	const unsigned int ext_add_lat = additive_latency >> 4U;
	const unsigned int ext_refrec = (picos_to_mclk(clk,
					       pdimm->trfc1_ps) - 8U) >> 4U;
	const unsigned int ext_wrrec = (picos_to_mclk(clk, pdimm->twr_ps) +
				  (popts->otf_burst_chop_en ? 2U : 0U)) >> 4U;
	const unsigned int rwt_same_cs = 0U;
	const unsigned int wrt_same_cs = 0U;
	const unsigned int rrt_same_cs = popts->burst_length == DDR_BL8 ? 0U : 2U;
	const unsigned int wwt_same_cs = popts->burst_length == DDR_BL8 ? 0U : 2U;
	const unsigned int dll_lock = 2U;
	unsigned int rodt_on = 0U;
	const unsigned int rodt_off = 4U;
	const unsigned int wodt_on = 1U;
	const unsigned int wodt_off = 4U;
	const unsigned int hs_caslat = 0U;
	const unsigned int hs_wrlat = 0U;
	const unsigned int hs_wrrec = 0U;
	const unsigned int hs_clkadj = 0U;
	const unsigned int hs_wrlvl_start = 0U;
	const unsigned int txpr = max(5U,
				      picos_to_mclk(clk,
						    pdimm->trfc1_ps + 10000U));
	const unsigned int tcksre = max(5U, picos_to_mclk(clk, 10000U));
	const unsigned int tcksrx = max(5U, picos_to_mclk(clk, 10000U));
	const unsigned int cs_to_cmd = 0U;
	const unsigned int cke_rst = txpr <= 200U ? 0U :
				     (txpr <= 256U ? 1U :
				      (txpr <= 512U ? 2U : 3U));
	const unsigned int cksre = tcksre <= 19U ? tcksre - 5U : 15U;
	const unsigned int cksrx = tcksrx <= 19U ? tcksrx - 5U : 15U;
	unsigned int par_lat = 0U;
	const int tccdl = max(5U, picos_to_mclk(clk, pdimm->tccdl_ps));
	int rwt_bg = cas_latency + 2 + 4 - wr_lat;
	int wrt_bg = wr_lat + 4 + 1 - cas_latency;
	const int rrt_bg = popts->burst_length == DDR_BL8 ?
				tccdl - 4 : tccdl - 2;
	const int wwt_bg = popts->burst_length == DDR_BL8 ?
					tccdl - 4 : tccdl - 2;
	const unsigned int acttoact_bg = picos_to_mclk(clk, pdimm->trrdl_ps);
	const unsigned int wrtord_bg = max(4U, picos_to_mclk(clk, 7500)) +
				       (popts->otf_burst_chop_en ? 2 : 0);
	const unsigned int pre_all_rec = 0;
	const unsigned int refrec_cid_mclk = pdimm->package_3ds ?
				picos_to_mclk(clk, pdimm->trfc_slr_ps) : 0;
	const unsigned int acttoact_cid_mclk = pdimm->package_3ds ? 4U : 0;


	/* for two dual-rank DIMMs to avoid ODT overlap */
	if (avoid_odt_overlap(conf, pdimm) == 2) {
		twrt_mclk = 2;
		twwt_mclk = 2;
		trrt_mclk = 2;
	} else {
		twrt_mclk = 1;
		twwt_mclk = 1;
		trrt_mclk = 0;
	}

	if (popts->trwt_override != 0) {
		trwt_mclk = popts->trwt;
		if (popts->twrt != 0) {
			twrt_mclk = popts->twrt;
		}
		if (popts->trrt != 0) {
			trrt_mclk = popts->trrt;
		}
		if (popts->twwt != 0) {
			twwt_mclk = popts->twwt;
		}
	}
	regs->timing_cfg[0] = (((trwt_mclk & 0x3) << 30)		|
			     ((twrt_mclk & 0x3) << 28)			|
			     ((trrt_mclk & 0x3) << 26)			|
			     ((twwt_mclk & 0x3) << 24)			|
			     ((act_pd_exit_mclk & 0xf) << 20)		|
			     ((pre_pd_exit_mclk & 0xF) << 16)		|
			     ((taxpd_mclk & 0xf) << 8)			|
			     ((tmrd_mclk & 0x1f) << 0));
	debug("timing_cfg[0] = 0x%x\n", regs->timing_cfg[0]);

	if ((wrrec_mclk < 1) || (wrrec_mclk > 24)) {
		ERROR("WRREC doesn't support clock %d\n", wrrec_mclk);
	} else {
		wrrec_mclk = wrrec_table[wrrec_mclk - 1];
	}

	if (popts->otf_burst_chop_en != 0) {
		wrrec_mclk += 2;
		wrtord_mclk += 2;
	}

	if (pdimm->trfc1_ps < trfc1_min) {
		ERROR("trfc1_ps (%d) < %d\n", pdimm->trfc1_ps, trfc1_min);
	}

	regs->timing_cfg[1] = (((pretoact_mclk & 0x0F) << 28)		|
			     ((acttopre_mclk & 0x0F) << 24)		|
			     ((acttorw_mclk & 0xF) << 20)		|
			     ((caslat_ctrl & 0xF) << 16)		|
			     ((refrec_ctrl & 0xF) << 12)		|
			     ((wrrec_mclk & 0x0F) << 8)			|
			     ((acttoact_mclk & 0x0F) << 4)		|
			     ((wrtord_mclk & 0x0F) << 0));
	debug("timing_cfg[1] = 0x%x\n", regs->timing_cfg[1]);

	if (rd_to_pre < 4) {
		rd_to_pre = 4;
	}
	if (popts->otf_burst_chop_en) {
		rd_to_pre += 2;
	}

	regs->timing_cfg[2] = (((additive_latency & 0xf) << 28)		|
			     ((cpo & 0x1f) << 23)			|
			     ((wr_lat & 0xf) << 19)			|
			     (((wr_lat & 0x10) >> 4) << 18)		|
			     ((rd_to_pre & 0xf) << 13)			|
			     ((wr_data_delay & 0xf) << 9)		|
			     ((cke_pls & 0x7) << 6)			|
			     ((four_act & 0x3f) << 0));
	debug("timing_cfg[2] = 0x%x\n", regs->timing_cfg[2]);

	regs->timing_cfg[3] = (((ext_pretoact & 0x1) << 28)		|
			     ((ext_acttopre & 0x3) << 24)		|
			     ((ext_acttorw & 0x1) << 22)		|
			     ((ext_refrec & 0x3F) << 16)		|
			     ((ext_caslat & 0x3) << 12)			|
			     ((ext_add_lat & 0x1) << 10)		|
			     ((ext_wrrec & 0x1) << 8)			|
			     ((cntl_adj & 0x7) << 0));
	debug("timing_cfg[3] = 0x%x\n", regs->timing_cfg[3]);

	regs->timing_cfg[4] = (((rwt_same_cs & 0xf) << 28)		|
			     ((wrt_same_cs & 0xf) << 24)		|
			     ((rrt_same_cs & 0xf) << 20)		|
			     ((wwt_same_cs & 0xf) << 16)		|
			     ((trwt_mclk & 0xc) << 12)			|
			     ((twrt_mclk & 0x4) << 10)			|
			     ((trrt_mclk & 0x4) << 8)			|
			     ((twwt_mclk & 0x4) << 6)			|
			     (dll_lock & 0x3));
	debug("timing_cfg[4] = 0x%x\n", regs->timing_cfg[4]);

	/* rodt_on = timing_cfg_1[caslat] - timing_cfg_2[wrlat] + 1 */
	if (cas_latency >= wr_lat) {
		rodt_on = cas_latency - wr_lat + 1;
	}

	regs->timing_cfg[5] = (((rodt_on & 0x1f) << 24)			|
			     ((rodt_off & 0x7) << 20)			|
			     ((wodt_on & 0x1f) << 12)			|
			     (wodt_off & 0x7) << 8);
	debug("timing_cfg[5] = 0x%x\n", regs->timing_cfg[5]);

	regs->timing_cfg[6] = (((hs_caslat & 0x1f) << 24)		|
			     ((hs_wrlat & 0x1f) << 19)			|
			     ((hs_wrrec & 0x1f) << 12)			|
			     ((hs_clkadj & 0x1f) << 6)			|
			     ((hs_wrlvl_start & 0x1f) << 0));
	debug("timing_cfg[6] = 0x%x\n", regs->timing_cfg[6]);

	if (popts->ap_en != 0) {
		par_lat = (regs->sdram_rcw[1] & 0xf) + 1;
		debug("PAR_LAT = 0x%x\n", par_lat);
	}

	regs->timing_cfg[7] = (((cke_rst & 0x3) << 28)			|
			     ((cksre & 0xf) << 24)			|
			     ((cksrx & 0xf) << 20)			|
			     ((par_lat & 0xf) << 16)			|
			     ((cs_to_cmd & 0xf) << 4));
	debug("timing_cfg[7] = 0x%x\n", regs->timing_cfg[7]);

	if (rwt_bg < tccdl) {
		rwt_bg = tccdl - rwt_bg;
	} else {
		rwt_bg = 0;
	}
	if (wrt_bg < tccdl) {
		wrt_bg = tccdl - wrt_bg;
	} else {
		wrt_bg = 0;
	}
	regs->timing_cfg[8] = (((rwt_bg & 0xf) << 28)			|
			     ((wrt_bg & 0xf) << 24)			|
			     ((rrt_bg & 0xf) << 20)			|
			     ((wwt_bg & 0xf) << 16)			|
			     ((acttoact_bg & 0xf) << 12)		|
			     ((wrtord_bg & 0xf) << 8)			|
			     ((pre_all_rec & 0x1f) << 0));
	debug("timing_cfg[8] = 0x%x\n", regs->timing_cfg[8]);

	regs->timing_cfg[9] = (refrec_cid_mclk & 0x3ff) << 16		|
			      (acttoact_cid_mclk & 0xf) << 8;
	debug("timing_cfg[9] = 0x%x\n", regs->timing_cfg[9]);
}

static void cal_ddr_sdram_rcw(const unsigned long clk,
			      struct ddr_cfg_regs *regs,
			      const struct memctl_opt *popts,
			      const struct dimm_params *pdimm)
{
	const unsigned int freq = clk / 1000000U;
	unsigned int rc0a, rc0f;

	if (pdimm->rdimm == 0) {
		return;
	}

	rc0a = freq > 3200U ? 7U :
	       (freq > 2933U ? 6U :
		(freq > 2666U ? 5U :
		 (freq > 2400U ? 4U :
		  (freq > 2133U ? 3U :
		   (freq > 1866U ? 2U :
		    (freq > 1600U ? 1U : 0U))))));
	rc0f = freq > 3200U ? 3U :
		(freq > 2400U ? 2U :
		 (freq > 2133U ? 1U : 0U));
	rc0f = (regs->sdram_cfg[1] & SDRAM_CFG2_AP_EN) ? rc0f : 4;
	regs->sdram_rcw[0] =
		pdimm->rcw[0] << 28	|
		pdimm->rcw[1] << 24	|
		pdimm->rcw[2] << 20	|
		pdimm->rcw[3] << 16	|
		pdimm->rcw[4] << 12	|
		pdimm->rcw[5] << 8	|
		pdimm->rcw[6] << 4	|
		pdimm->rcw[7];
	regs->sdram_rcw[1] =
		pdimm->rcw[8] << 28	|
		pdimm->rcw[9] << 24	|
		rc0a << 20		|
		pdimm->rcw[11] << 16	|
		pdimm->rcw[12] << 12	|
		pdimm->rcw[13] << 8	|
		pdimm->rcw[14] << 4	|
		rc0f;
	regs->sdram_rcw[2] =
		((freq - 1260 + 19) / 20) << 8;

	debug("sdram_rcw[0] = 0x%x\n", regs->sdram_rcw[0]);
	debug("sdram_rcw[1] = 0x%x\n", regs->sdram_rcw[1]);
	debug("sdram_rcw[2] = 0x%x\n", regs->sdram_rcw[2]);
}

static void cal_ddr_sdram_cfg(const unsigned long clk,
			      struct ddr_cfg_regs *regs,
			      const struct memctl_opt *popts,
			      const struct dimm_params *pdimm,
			      const unsigned int ip_rev)
{
	const unsigned int mem_en = 1U;
	const unsigned int sren = popts->self_refresh_in_sleep;
	const unsigned int ecc_en = popts->ecc_mode;
	const unsigned int rd_en = (pdimm->rdimm != 0U) ? 1U : 0U;
	const unsigned int dyn_pwr = popts->dynamic_power;
	const unsigned int dbw = popts->data_bus_used;
	const unsigned int eight_be = (dbw == 1U ||
				       popts->burst_length == DDR_BL8) ? 1U : 0U;
	const unsigned int ncap = 0U;
	const unsigned int threet_en = popts->threet_en;
	const unsigned int twot_en = pdimm->rdimm ?
					0U : popts->twot_en;
	const unsigned int ba_intlv = popts->ba_intlv;
	const unsigned int x32_en = 0U;
	const unsigned int pchb8 = 0U;
	const unsigned int hse = popts->half_strength_drive_en;
	const unsigned int acc_ecc_en = (dbw != 0U && ecc_en == 1U) ? 1U : 0U;
	const unsigned int mem_halt = 0U;
#ifdef PHY_GEN2
	const unsigned int bi = 1U;
#else
	const unsigned int bi = 0U;
#endif
	const unsigned int sdram_type = SDRAM_TYPE_DDR4;
	unsigned int odt_cfg = 0U;
	const unsigned int frc_sr = 0U;
	const unsigned int sr_ie = popts->self_refresh_irq_en;
	const unsigned int num_pr = pdimm->package_3ds + 1U;
	const unsigned int slow = (clk < 1249000000U) ? 1U : 0U;
	const unsigned int x4_en = popts->x4_en;
	const unsigned int obc_cfg = popts->otf_burst_chop_en;
	const unsigned int ap_en = ip_rev == 0x50500U ? 0U : popts->ap_en;
	const unsigned int d_init = popts->ctlr_init_ecc;
	const unsigned int rcw_en = popts->rdimm;
	const unsigned int md_en = popts->mirrored_dimm;
	const unsigned int qd_en = popts->quad_rank_present;
	const unsigned int unq_mrs_en = ip_rev < 0x50500U ? 1U : 0U;
	const unsigned int rd_pre = popts->quad_rank_present;
	int i;

	regs->sdram_cfg[0] = ((mem_en & 0x1) << 31)		|
				((sren & 0x1) << 30)		|
				((ecc_en & 0x1) << 29)		|
				((rd_en & 0x1) << 28)		|
				((sdram_type & 0x7) << 24)	|
				((dyn_pwr & 0x1) << 21)		|
				((dbw & 0x3) << 19)		|
				((eight_be & 0x1) << 18)	|
				((ncap & 0x1) << 17)		|
				((threet_en & 0x1) << 16)	|
				((twot_en & 0x1) << 15)		|
				((ba_intlv & 0x7F) << 8)	|
				((x32_en & 0x1) << 5)		|
				((pchb8 & 0x1) << 4)		|
				((hse & 0x1) << 3)		|
				((acc_ecc_en & 0x1) << 2)	|
				((mem_halt & 0x1) << 1)		|
				((bi & 0x1) << 0);
	debug("sdram_cfg[0] = 0x%x\n", regs->sdram_cfg[0]);

	for (i = 0; i < DDRC_NUM_CS; i++) {
		if (popts->cs_odt[i].odt_rd_cfg != 0 ||
		    popts->cs_odt[i].odt_wr_cfg != 0) {
			odt_cfg = SDRAM_CFG2_ODT_ONLY_READ;
			break;
		}
	}

	regs->sdram_cfg[1] = (0
		| ((frc_sr & 0x1) << 31)
		| ((sr_ie & 0x1) << 30)
		| ((odt_cfg & 0x3) << 21)
		| ((num_pr & 0xf) << 12)
		| ((slow & 1) << 11)
		| (x4_en << 10)
		| (qd_en << 9)
		| (unq_mrs_en << 8)
		| ((obc_cfg & 0x1) << 6)
		| ((ap_en & 0x1) << 5)
		| ((d_init & 0x1) << 4)
		| ((rcw_en & 0x1) << 2)
		| ((md_en & 0x1) << 0)
		);
	debug("sdram_cfg[1] = 0x%x\n", regs->sdram_cfg[1]);

	regs->sdram_cfg[2] = (rd_pre & 0x1) << 16	|
				 (popts->rdimm ? 1 : 0);
	if (pdimm->package_3ds != 0) {
		if (((pdimm->package_3ds + 1) & 0x1) != 0) {
			WARN("Unsupported 3DS DIMM\n");
		} else {
			regs->sdram_cfg[2] |= ((pdimm->package_3ds + 1) >> 1)
						  << 4;
		}
	}
	debug("sdram_cfg[2] = 0x%x\n", regs->sdram_cfg[2]);
}


static void cal_ddr_sdram_interval(const unsigned long clk,
				   struct ddr_cfg_regs *regs,
				   const struct memctl_opt *popts,
				   const struct dimm_params *pdimm)
{
	const unsigned int refint = picos_to_mclk(clk, pdimm->refresh_rate_ps);
	const unsigned int bstopre = popts->bstopre;

	regs->interval = ((refint & 0xFFFF) << 16)	|
				  ((bstopre & 0x3FFF) << 0);
	debug("interval = 0x%x\n", regs->interval);
}

/* Require cs and cfg first */
static void cal_ddr_sdram_mode(const unsigned long clk,
			       struct ddr_cfg_regs *regs,
			       const struct memctl_opt *popts,
			       const struct ddr_conf *conf,
			       const struct dimm_params *pdimm,
			       unsigned int cas_latency,
			       unsigned int additive_latency,
			       const unsigned int ip_rev)
{
	int i;
	unsigned short esdmode;		/* Extended SDRAM mode */
	unsigned short sdmode;		/* SDRAM mode */

	/* Mode Register - MR1 */
	const unsigned int qoff = 0;
	const unsigned int tdqs_en = 0;
	unsigned int rtt;
	const unsigned int wrlvl_en = 0;
	unsigned int al = 0;
	unsigned int dic = 0;
	const unsigned int dll_en = 1;

	/* Mode Register - MR0 */
	unsigned int wr = 0;
	const unsigned int dll_rst = 0;
	const unsigned int mode = 0;
	unsigned int caslat = 4;/* CAS# latency, default set as 6 cycles */
	/* BT: Burst Type (0=Nibble Sequential, 1=Interleaved) */
	const unsigned int bt = 0;
	const unsigned int bl = popts->burst_length == DDR_BL8 ? 0 :
				 (popts->burst_length == DDR_BC4 ? 2 : 1);

	const unsigned int wr_mclk = picos_to_mclk(clk, pdimm->twr_ps);
	/* DDR4 support WR 10, 12, 14, 16, 18, 20, 24 */
	static const int wr_table[] = {
		0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6
	};
	/* DDR4 support CAS 9, 10, 11, 12, 13, 14, 15, 16, 18, 20, 22, 24 */
	static const int cas_latency_table[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 13, 8,
		14, 9, 15, 10, 12, 11, 16, 17,
		18, 19, 20, 21, 22, 23
	};
	const unsigned int unq_mrs_en = ip_rev < U(0x50500) ? 1U : 0U;
	unsigned short esdmode2 = 0U;
	unsigned short esdmode3 = 0U;
	const unsigned int wr_crc = 0U;
	unsigned int rtt_wr = 0U;
	const unsigned int srt = 0U;
	unsigned int cwl = cal_cwl(clk);
	const unsigned int mpr = 0U;
	const unsigned int mclk_ps = get_memory_clk_ps(clk);
	const unsigned int wc_lat = 0U;
	unsigned short esdmode4 = 0U;
	unsigned short esdmode5;
	int rtt_park_all = 0;
	unsigned int rtt_park;
	const bool four_cs = conf->cs_in_use == 0xf ? true : false;
	unsigned short esdmode6 = 0U;	/* Extended SDRAM mode 6 */
	unsigned short esdmode7 = 0U;	/* Extended SDRAM mode 7 */
	const unsigned int tccdl_min = max(5U,
					   picos_to_mclk(clk, pdimm->tccdl_ps));

	if (popts->rtt_override != 0U) {
		rtt = popts->rtt_override_value;
	} else {
		rtt = popts->cs_odt[0].odt_rtt_norm;
	}

	if (additive_latency == (cas_latency - 1)) {
		al = 1;
	}
	if (additive_latency == (cas_latency - 2)) {
		al = 2;
	}

	if (popts->quad_rank_present != 0 || popts->output_driver_impedance != 0) {
		dic = 1;	/* output driver impedance 240/7 ohm */
	}

	esdmode = (((qoff & 0x1) << 12)				|
		   ((tdqs_en & 0x1) << 11)			|
		   ((rtt & 0x7) << 8)				|
		   ((wrlvl_en & 0x1) << 7)			|
		   ((al & 0x3) << 3)				|
		   ((dic & 0x3) << 1)				|
		   ((dll_en & 0x1) << 0));

	if (wr_mclk >= 10 && wr_mclk <= 24) {
		wr = wr_table[wr_mclk - 10];
	} else {
		ERROR("unsupported wc_mclk = %d for mode register\n", wr_mclk);
	}

	/* look up table to get the cas latency bits */
	if (cas_latency >= 9 && cas_latency <= 32) {
		caslat = cas_latency_table[cas_latency - 9];
	} else {
		WARN("Error: unsupported cas latency for mode register\n");
	}

	sdmode = (((caslat & 0x10) << 8)			|
		  ((wr & 0x7) << 9)				|
		  ((dll_rst & 0x1) << 8)			|
		  ((mode & 0x1) << 7)				|
		  (((caslat >> 1) & 0x7) << 4)			|
		  ((bt & 0x1) << 3)				|
		  ((caslat & 1) << 2)				|
		  ((bl & 0x3) << 0));

	regs->sdram_mode[0] = (((esdmode & 0xFFFF) << 16)	|
				 ((sdmode & 0xFFFF) << 0));
	debug("sdram_mode[0] = 0x%x\n", regs->sdram_mode[0]);

	switch (cwl) {
	case 9:
	case 10:
	case 11:
	case 12:
		cwl -= 9;
		break;
	case 14:
		cwl -= 10;
		break;
	case 16:
		cwl -= 11;
		break;
	case 18:
		cwl -= 12;
		break;
	case 20:
		cwl -= 13;
		break;
	default:
		printf("Error CWL\n");
		break;
	}

	if (popts->rtt_override != 0) {
		rtt_wr = popts->rtt_wr_override_value;
	} else {
		rtt_wr = popts->cs_odt[0].odt_rtt_wr;
	}

	esdmode2 = ((wr_crc & 0x1) << 12)			|
		   ((rtt_wr & 0x7) << 9)			|
		   ((srt & 0x3) << 6)				|
		   ((cwl & 0x7) << 3);
	esdmode3 = ((mpr & 0x3) << 11) | ((wc_lat & 0x3) << 9);

	regs->sdram_mode[1] = ((esdmode2 & 0xFFFF) << 16)	|
				((esdmode3 & 0xFFFF) << 0);
	debug("sdram_mode[1] = 0x%x\n", regs->sdram_mode[1]);

	esdmode6 = ((tccdl_min - 4) & 0x7) << 10;
	if (popts->vref_dimm != 0) {
		esdmode6 |= popts->vref_dimm & 0x7f;
	} else if ((popts->ddr_cdr2 & DDR_CDR2_VREF_RANGE_2) != 0) {
		esdmode6 |= 1 << 6;	/* Range 2 */
	}

	regs->sdram_mode[9] = ((esdmode6 & 0xffff) << 16)	|
				 ((esdmode7 & 0xffff) << 0);
	debug("sdram_mode[9] = 0x%x\n", regs->sdram_mode[9]);

	rtt_park = (popts->rtt_park != 0) ? popts->rtt_park : 240;
	switch (rtt_park) {
	case 240:
		rtt_park = 0x4;
		break;
	case 120:
		rtt_park = 0x2;
		break;
	case 80:
		rtt_park = 0x6;
		break;
	case 60:
		rtt_park = 0x1;
		break;
	case 48:
		rtt_park = 0x5;
		break;
	case 40:
		rtt_park = 0x3;
		break;
	case 34:
		rtt_park = 0x7;
		break;
	default:
		rtt_park = 0;
		break;
	}

	for (i = 0; i < DDRC_NUM_CS; i++) {
		if (i != 0 && unq_mrs_en == 0) {
			break;
		}

		if (popts->rtt_override != 0) {
			rtt = popts->rtt_override_value;
			rtt_wr = popts->rtt_wr_override_value;
		} else {
			rtt = popts->cs_odt[i].odt_rtt_norm;
			rtt_wr = popts->cs_odt[i].odt_rtt_wr;
		}

		esdmode &= 0xF8FF;	/* clear bit 10,9,8 for rtt */
		esdmode |= (rtt & 0x7) << 8;
		esdmode2 &= 0xF9FF;	/* clear bit 10, 9 */
		esdmode2 |= (rtt_wr & 0x3) << 9;
		esdmode5 = (popts->x4_en) ? 0 : 0x400; /* data mask */

		if (rtt_park_all == 0 &&
		    ((regs->cs[i].config & SDRAM_CS_CONFIG_EN) != 0)) {
			esdmode5 |= rtt_park << 6;
			rtt_park_all = four_cs ? 0 : 1;
		}

		if (((regs->sdram_cfg[1] & SDRAM_CFG2_AP_EN) != 0) &&
		    (popts->rdimm == 0)) {
			if (mclk_ps >= 935) {
				esdmode5 |= DDR_MR5_CA_PARITY_LAT_4_CLK;
			} else if (mclk_ps >= 833) {
				esdmode5 |= DDR_MR5_CA_PARITY_LAT_5_CLK;
			} else {
				esdmode5 |= DDR_MR5_CA_PARITY_LAT_5_CLK;
				WARN("mclk_ps not supported %d", mclk_ps);

			}
		}

		switch (i) {
		case 0:
			regs->sdram_mode[8] = ((esdmode4 & 0xffff) << 16) |
						((esdmode5 & 0xffff) << 0);
			debug("sdram_mode[8] = 0x%x\n", regs->sdram_mode[8]);
			break;
		case 1:
			regs->sdram_mode[2] = (((esdmode & 0xFFFF) << 16) |
					      ((sdmode & 0xFFFF) << 0));
			regs->sdram_mode[3] = ((esdmode2 & 0xFFFF) << 16) |
					      ((esdmode3 & 0xFFFF) << 0);
			regs->sdram_mode[10] = ((esdmode4 & 0xFFFF) << 16) |
					       ((esdmode5 & 0xFFFF) << 0);
			regs->sdram_mode[11] = ((esdmode6 & 0xFFFF) << 16) |
					       ((esdmode7 & 0xFFFF) << 0);
			debug("sdram_mode[2] = 0x%x\n", regs->sdram_mode[2]);
			debug("sdram_mode[3] = 0x%x\n", regs->sdram_mode[3]);
			debug("sdram_mode[10] = 0x%x\n", regs->sdram_mode[10]);
			debug("sdram_mode[11] = 0x%x\n", regs->sdram_mode[11]);
			break;
		case 2:
			regs->sdram_mode[4] = (((esdmode & 0xFFFF) << 16) |
					      ((sdmode & 0xFFFF) << 0));
			regs->sdram_mode[5] = ((esdmode2 & 0xFFFF) << 16) |
					      ((esdmode3 & 0xFFFF) << 0);
			regs->sdram_mode[12] = ((esdmode4 & 0xFFFF) << 16) |
					       ((esdmode5 & 0xFFFF) << 0);
			regs->sdram_mode[13] = ((esdmode6 & 0xFFFF) << 16) |
					       ((esdmode7 & 0xFFFF) << 0);
			debug("sdram_mode[4] = 0x%x\n", regs->sdram_mode[4]);
			debug("sdram_mode[5] = 0x%x\n", regs->sdram_mode[5]);
			debug("sdram_mode[12] = 0x%x\n", regs->sdram_mode[12]);
			debug("sdram_mode[13] = 0x%x\n", regs->sdram_mode[13]);
			break;
		case 3:
			regs->sdram_mode[6] = (((esdmode & 0xFFFF) << 16) |
					      ((sdmode & 0xFFFF) << 0));
			regs->sdram_mode[7] = ((esdmode2 & 0xFFFF) << 16) |
					      ((esdmode3 & 0xFFFF) << 0);
			regs->sdram_mode[14] = ((esdmode4 & 0xFFFF) << 16) |
					       ((esdmode5 & 0xFFFF) << 0);
			regs->sdram_mode[15] = ((esdmode6 & 0xFFFF) << 16) |
					       ((esdmode7 & 0xFFFF) << 0);
			debug("sdram_mode[6] = 0x%x\n", regs->sdram_mode[6]);
			debug("sdram_mode[7] = 0x%x\n", regs->sdram_mode[7]);
			debug("sdram_mode[14] = 0x%x\n", regs->sdram_mode[14]);
			debug("sdram_mode[15] = 0x%x\n", regs->sdram_mode[15]);
			break;
		default:
			break;
		}
	}
}

#ifndef CONFIG_MEM_INIT_VALUE
#define CONFIG_MEM_INIT_VALUE 0xDEADBEEF
#endif
static void cal_ddr_data_init(struct ddr_cfg_regs *regs)
{
	regs->data_init = CONFIG_MEM_INIT_VALUE;
}

static void cal_ddr_dq_mapping(struct ddr_cfg_regs *regs,
			       const struct dimm_params *pdimm)
{
	const unsigned int acc_ecc_en = (regs->sdram_cfg[0] >> 2) & 0x1;
/* FIXME: revert the dq mapping from DIMM */
	regs->dq_map[0] = ((pdimm->dq_mapping[0] & 0x3F) << 26)	|
			 ((pdimm->dq_mapping[1] & 0x3F) << 20)	|
			 ((pdimm->dq_mapping[2] & 0x3F) << 14)	|
			 ((pdimm->dq_mapping[3] & 0x3F) << 8)	|
			 ((pdimm->dq_mapping[4] & 0x3F) << 2);

	regs->dq_map[1] = ((pdimm->dq_mapping[5] & 0x3F) << 26)	|
			 ((pdimm->dq_mapping[6] & 0x3F) << 20)	|
			 ((pdimm->dq_mapping[7] & 0x3F) << 14)	|
			 ((pdimm->dq_mapping[10] & 0x3F) << 8)	|
			 ((pdimm->dq_mapping[11] & 0x3F) << 2);

	regs->dq_map[2] = ((pdimm->dq_mapping[12] & 0x3F) << 26)	|
			 ((pdimm->dq_mapping[13] & 0x3F) << 20)		|
			 ((pdimm->dq_mapping[14] & 0x3F) << 14)		|
			 ((pdimm->dq_mapping[15] & 0x3F) << 8)		|
			 ((pdimm->dq_mapping[16] & 0x3F) << 2);

	/* dq_map for ECC[4:7] is set to 0 if accumulated ECC is enabled */
	regs->dq_map[3] = ((pdimm->dq_mapping[17] & 0x3F) << 26)	|
			 ((pdimm->dq_mapping[8] & 0x3F) << 20)		|
			 ((acc_ecc_en != 0) ? 0 :
			  (pdimm->dq_mapping[9] & 0x3F) << 14)		|
			 pdimm->dq_mapping_ors;
	debug("dq_map[0] = 0x%x\n", regs->dq_map[0]);
	debug("dq_map[1] = 0x%x\n", regs->dq_map[1]);
	debug("dq_map[2] = 0x%x\n", regs->dq_map[2]);
	debug("dq_map[3] = 0x%x\n", regs->dq_map[3]);
}
static void cal_ddr_zq_cntl(struct ddr_cfg_regs *regs)
{
	const unsigned int zqinit = 10U;	/* 1024 clocks */
	const unsigned int zqoper = 9U;		/* 512 clocks */
	const unsigned int zqcs = 7U;		/* 128 clocks */
	const unsigned int zqcs_init = 5U;	/* 1024 refresh seqences */
	const unsigned int zq_en = 1U;		/* enabled */

	regs->zq_cntl = ((zq_en & 0x1) << 31)			|
			   ((zqinit & 0xF) << 24)		|
			   ((zqoper & 0xF) << 16)		|
			   ((zqcs & 0xF) << 8)			|
			   ((zqcs_init & 0xF) << 0);
	debug("zq_cntl = 0x%x\n", regs->zq_cntl);
}

static void cal_ddr_sr_cntr(struct ddr_cfg_regs *regs,
			    const struct memctl_opt *popts)
{
	const unsigned int sr_it = (popts->auto_self_refresh_en) ?
					popts->sr_it : 0;

	regs->ddr_sr_cntr = (sr_it & 0xF) << 16;
	debug("ddr_sr_cntr = 0x%x\n", regs->ddr_sr_cntr);
}

static void cal_ddr_eor(struct ddr_cfg_regs *regs,
			const struct memctl_opt *popts)
{
	if (popts->addr_hash != 0) {
		regs->eor = 0x40000000;	/* address hash enable */
		debug("eor = 0x%x\n", regs->eor);
	}
}

static void cal_ddr_csn_bnds(struct ddr_cfg_regs *regs,
			     const struct memctl_opt *popts,
			     const struct ddr_conf *conf,
			     const struct dimm_params *pdimm)
{
	int i;
	unsigned long long ea, sa;

	/* Chip Select Memory Bounds (CSn_BNDS) */
	for (i = 0;
		i < DDRC_NUM_CS && conf->cs_size[i];
		i++) {
		debug("cs_in_use = 0x%x\n", conf->cs_in_use);
		if (conf->cs_in_use != 0) {
			sa = conf->cs_base_addr[i];
			ea = sa + conf->cs_size[i] - 1;
			sa >>= 24;
			ea >>= 24;
			regs->cs[i].bnds = ((sa & 0xffff) << 16) |
					   ((ea & 0xffff) << 0);
			cal_csn_config(i, regs, popts, pdimm);
		} else {
			/* setting bnds to 0xffffffff for inactive CS */
			regs->cs[i].bnds = 0xffffffff;
		}

		debug("cs[%d].bnds = 0x%x\n", i, regs->cs[i].bnds);
	}
}

static void cal_ddr_addr_dec(struct ddr_cfg_regs *regs)
{
#ifdef CONFIG_DDR_ADDR_DEC
	unsigned int ba_bits __unused;
	char p __unused;
	const unsigned int cs0_config = regs->cs[0].config;
	const int cacheline = PLATFORM_CACHE_LINE_SHIFT;
	unsigned int bg_bits;
	unsigned int row_bits;
	unsigned int col_bits;
	unsigned int cs;
	unsigned int map_row[18];
	unsigned int map_col[11];
	unsigned int map_ba[2];
	unsigned int map_cid[2] = {0x3F, 0x3F};
	unsigned int map_bg[2] = {0x3F, 0x3F};
	unsigned int map_cs[2] = {0x3F, 0x3F};
	unsigned int dbw;
	unsigned int ba_intlv;
	int placement;
	int intlv;
	int abort = 0;
	int i;
	int j;

	col_bits = (cs0_config >> 0) & 0x7;
	if (col_bits < 4) {
		col_bits += 8;
	} else if (col_bits < 7 || col_bits > 10) {
		ERROR("Error %s col_bits = %d\n", __func__, col_bits);
	}
	row_bits = ((cs0_config >> 8) & 0x7) + 12;
	ba_bits = ((cs0_config >> 14) & 0x3) + 2;
	bg_bits = ((cs0_config >> 4) & 0x3) + 0;
	intlv = (cs0_config >> 24) & 0xf;
	ba_intlv = (regs->sdram_cfg[0] >> 8) & 0x7f;
	switch (ba_intlv) {
	case DDR_BA_INTLV_CS01:
		cs = 1;
		break;
	case DDR_BA_INTLV_CS0123:
		cs = 2;
		break;
	case DDR_BA_NONE:
		cs = 0;
		break;
	default:
		ERROR("%s ba_intlv 0x%x\n", __func__, ba_intlv);
		return;
	}
	debug("col %d, row %d, ba %d, bg %d, intlv %d\n",
			col_bits, row_bits, ba_bits, bg_bits, intlv);
	/*
	 * Example mapping of 15x2x2x10
	 * ---- --rr rrrr rrrr rrrr rCBB Gccc cccI cGcc cbbb
	 */
	dbw = (regs->sdram_cfg[0] >> 19) & 0x3;
	switch (dbw) {
	case 0:	/* 64-bit */
		placement = 3;
		break;
	case 1:	/* 32-bit */
		placement = 2;
		break;
	default:
		ERROR("%s dbw = %d\n", __func__, dbw);
		return;
	}
	debug("cacheline size %d\n", cacheline);
	for (i = 0; placement < cacheline; i++) {
		map_col[i] = placement++;
	}
	map_bg[0] = placement++;
	for ( ; i < col_bits; i++) {
		map_col[i] = placement++;
		if (placement == intlv) {
			placement++;
		}
	}
	for ( ; i < 11; i++) {
		map_col[i] = 0x3F;	/* unused col bits */
	}

	if (bg_bits >= 2) {
		map_bg[1] = placement++;
	}
	map_ba[0] = placement++;
	map_ba[1] = placement++;
	if (cs != 0U) {
		map_cs[0] = placement++;
		if (cs == 2U) {
			map_cs[1] = placement++;
		}
	} else {
		map_cs[0] = U(0x3F);
	}

	for (i = 0; i < row_bits; i++) {
		map_row[i] = placement++;
	}

	for ( ; i < 18; i++) {
		map_row[i] = 0x3F;	/* unused row bits */
	}

	for (i = 39; i >= 0 ; i--) {
		if (i == intlv) {
			placement = 8;
			p = 'I';
		} else if (i < 3) {
			p = 'b';
			placement = 0;
		} else {
			placement = 0;
			p = '-';
		}
		for (j = 0; j < 18; j++) {
			if (map_row[j] != i) {
				continue;
			}
			if (placement != 0) {
				abort = 1;
				ERROR("%s wrong address bit %d\n", __func__, i);
			}
			placement = i;
			p = 'r';
		}
		for (j = 0; j < 11; j++) {
			if (map_col[j] != i) {
				continue;
			}
			if (placement != 0) {
				abort = 1;
				ERROR("%s wrong address bit %d\n", __func__, i);
			}
			placement = i;
			p = 'c';
		}
		for (j = 0; j < 2; j++) {
			if (map_ba[j] != i) {
				continue;
			}
			if (placement != 0) {
				abort = 1;
				ERROR("%s wrong address bit %d\n", __func__, i);
			}
			placement = i;
			p = 'B';
		}
		for (j = 0; j < 2; j++) {
			if (map_bg[j] != i) {
				continue;
			}
			if (placement != 0) {
				abort = 1;
				ERROR("%s wrong address bit %d\n", __func__, i);
			}
			placement = i;
			p = 'G';
		}
		for (j = 0; j < 2; j++) {
			if (map_cs[j] != i) {
				continue;
			}
			if (placement != 0) {
				abort = 1;
				ERROR("%s wrong address bit %d\n", __func__, i);
			}
			placement = i;
			p = 'C';
		}
#ifdef DDR_DEBUG
		printf("%c", p);
		if ((i % 4) == 0) {
			printf(" ");
		}
#endif
	}
#ifdef DDR_DEBUG
	puts("\n");
#endif

	if (abort != 0) {
		return;
	}

	regs->dec[0] = map_row[17] << 26		|
		      map_row[16] << 18			|
		      map_row[15] << 10			|
		      map_row[14] << 2;
	regs->dec[1] = map_row[13] << 26		|
		      map_row[12] << 18			|
		      map_row[11] << 10			|
		      map_row[10] << 2;
	regs->dec[2] = map_row[9] << 26			|
		      map_row[8] << 18			|
		      map_row[7] << 10			|
		      map_row[6] << 2;
	regs->dec[3] = map_row[5] << 26			|
		      map_row[4] << 18			|
		      map_row[3] << 10			|
		      map_row[2] << 2;
	regs->dec[4] = map_row[1] << 26			|
		      map_row[0] << 18			|
		      map_col[10] << 10			|
		      map_col[9] << 2;
	regs->dec[5] = map_col[8] << 26			|
		      map_col[7] << 18			|
		      map_col[6] << 10			|
		      map_col[5] << 2;
	regs->dec[6] = map_col[4] << 26			|
		      map_col[3] << 18			|
		      map_col[2] << 10			|
		      map_col[1] << 2;
	regs->dec[7] = map_col[0] << 26			|
		      map_ba[1] << 18			|
		      map_ba[0] << 10			|
		      map_cid[1] << 2;
	regs->dec[8] = map_cid[1] << 26			|
		      map_cs[1] << 18			|
		      map_cs[0] << 10			|
		      map_bg[1] << 2;
	regs->dec[9] = map_bg[0] << 26			|
		      1;
	for (i = 0; i < 10; i++) {
		debug("dec[%d] = 0x%x\n", i, regs->dec[i]);
	}
#endif
}
static unsigned int skip_caslat(unsigned int tckmin_ps,
				unsigned int taamin_ps,
				unsigned int mclk_ps,
				unsigned int package_3ds)
{
	int i, j, k;
	struct cas {
		const unsigned int tckmin_ps;
		const unsigned int caslat[4];
	};
	struct speed {
		const struct cas *cl;
		const unsigned int taamin_ps[4];
	};
	const struct cas cl_3200[] = {
		{625,	{0xa00000, 0xb00000, 0xf000000,} },
		{750,	{ 0x20000,  0x60000,  0xe00000,} },
		{833,	{  0x8000,  0x18000,   0x38000,} },
		{937,	{  0x4000,   0x4000,    0xc000,} },
		{1071,	{  0x1000,   0x1000,    0x3000,} },
		{1250,	{   0x400,    0x400,     0xc00,} },
		{1500,	{       0,    0x600,     0x200,} },
	};
	const struct cas cl_2933[] = {
		{682,	{       0,  0x80000, 0x180000, 0x380000} },
		{750,	{ 0x20000,  0x60000,  0x60000,  0xe0000} },
		{833,	{  0x8000,  0x18000,  0x18000,  0x38000} },
		{937,	{  0x4000,   0x4000,   0x4000,   0xc000} },
		{1071,	{  0x1000,   0x1000,   0x1000,   0x3000} },
		{1250,	{   0x400,    0x400,    0x400,    0xc00} },
		{1500,	{       0,    0x200,    0x200,    0x200} },
	};
	const struct cas cl_2666[] = {
		{750,	{       0,  0x20000,  0x60000,  0xe0000} },
		{833,	{  0x8000,  0x18000,  0x18000,  0x38000} },
		{937,	{  0x4000,   0x4000,   0x4000,   0xc000} },
		{1071,	{  0x1000,   0x1000,   0x1000,   0x3000} },
		{1250,	{   0x400,    0x400,    0x400,    0xc00} },
		{1500,	{       0,        0,    0x200,    0x200} },
	};
	const struct cas cl_2400[] = {
		{833,	{       0,   0x8000,  0x18000,  0x38000} },
		{937,	{  0xc000,   0x4000,   0x4000,   0xc000} },
		{1071,	{  0x3000,   0x1000,   0x1000,   0x3000} },
		{1250,	{   0xc00,    0x400,    0x400,    0xc00} },
		{1500,	{       0,    0x400,    0x200,    0x200} },
	};
	const struct cas cl_2133[] = {
		{937,	{       0,   0x4000,   0xc000,} },
		{1071,	{  0x2000,        0,   0x2000,} },
		{1250,	{   0x800,        0,    0x800,} },
		{1500,	{       0,    0x400,    0x200,} },
	};
	const struct cas cl_1866[] = {
		{1071,	{       0,   0x1000,   0x3000,} },
		{1250,	{   0xc00,    0x400,    0xc00,} },
		{1500,	{       0,    0x400,    0x200,} },
	};
	const struct cas cl_1600[] = {
		{1250,	{       0,    0x400,    0xc00,} },
		{1500,	{       0,    0x400,    0x200,} },
	};
	const struct speed bin_0[] = {
		{cl_3200, {12500, 13750, 15000,} },
		{cl_2933, {12960, 13640, 13750, 15000,} },
		{cl_2666, {12750, 13500, 13750, 15000,} },
		{cl_2400, {12500, 13320, 13750, 15000,} },
		{cl_2133, {13130, 13500, 15000,} },
		{cl_1866, {12850, 13500, 15000,} },
		{cl_1600, {12500, 13500, 15000,} }
	};
	const struct cas cl_3200_3ds[] = {
		{625,	{ 0xa000000, 0xb000000, 0xf000000,} },
		{750,	{ 0xaa00000, 0xab00000, 0xef00000,} },
		{833,	{ 0xaac0000, 0xaac0000, 0xebc0000,} },
		{937,	{ 0xaab0000, 0xaab0000, 0xeaf0000,} },
		{1071,	{ 0xaaa4000, 0xaaac000, 0xeaec000,} },
		{1250,	{ 0xaaa0000, 0xaaa2000, 0xeaeb000,} },
	};
	const struct cas cl_2666_3ds[] = {
		{750,	{ 0xa00000, 0xb00000, 0xf00000,} },
		{833,	{ 0xac0000, 0xac0000, 0xbc0000,} },
		{937,	{ 0xab0000, 0xab0000, 0xaf0000,} },
		{1071,	{ 0xaa4000, 0xaac000, 0xaac000,} },
		{1250,	{ 0xaa0000, 0xaaa000, 0xaaa000,} },
	};
	const struct cas cl_2400_3ds[] = {
		{833,	{ 0xe00000, 0xe40000, 0xec0000, 0xb00000} },
		{937,	{ 0xe00000, 0xe00000, 0xea0000, 0xae0000} },
		{1071,	{ 0xe00000, 0xe04000, 0xeac000, 0xaec000} },
		{1250,	{ 0xe00000, 0xe00000, 0xeaa000, 0xae2000} },
	};
	const struct cas cl_2133_3ds[] = {
		{937,	{  0x90000,  0xb0000,  0xf0000,} },
		{1071,	{  0x84000,  0xac000,  0xec000,} },
		{1250,	{  0x80000,  0xa2000,  0xe2000,} },
	};
	const struct cas cl_1866_3ds[] = {
		{1071,	{        0,   0x4000,   0xc000,} },
		{1250,	{        0,   0x1000,   0x3000,} },
	};
	const struct cas cl_1600_3ds[] = {
		{1250,	{        0,   0x1000,   0x3000,} },
	};
	const struct speed bin_3ds[] = {
		{cl_3200_3ds, {15000, 16250, 17140,} },
		{cl_2666_3ds, {15000, 16500, 17140,} },
		{cl_2400_3ds, {15000, 15830, 16670, 17140} },
		{cl_2133_3ds, {15950, 16880, 17140,} },
		{cl_1866_3ds, {15000, 16070, 17140,} },
		{cl_1600_3ds, {15000, 16250, 17500,} },
	};
	const struct speed *bin;
	int size;
	unsigned int taamin_max, tck_max;

	if (taamin_ps > ((package_3ds != 0) ? 21500 : 18000)) {
		ERROR("taamin_ps %u invalid\n", taamin_ps);
		return 0;
	}
	if (package_3ds != 0) {
		bin = bin_3ds;
		size = ARRAY_SIZE(bin_3ds);
		taamin_max = 1250;
		tck_max = 1500;
	} else {
		bin = bin_0;
		size = ARRAY_SIZE(bin_0);
		taamin_max = 1500;
		tck_max = 1600;
	}
	if (mclk_ps < 625 || mclk_ps > tck_max) {
		ERROR("mclk %u invalid\n", mclk_ps);
		return 0;
	}

	for (i = 0; i < size; i++) {
		if (bin[i].cl[0].tckmin_ps >= tckmin_ps) {
			break;
		}
	}
	if (i >= size) {
		ERROR("speed bin not found\n");
		return 0;
	}
	if (bin[i].cl[0].tckmin_ps > tckmin_ps && i > 0) {
		i--;
	}

	for (j = 0; j < 4; j++) {
		if ((bin[i].taamin_ps[j] == 0) ||
		    bin[i].taamin_ps[j] >= taamin_ps) {
			break;
		}
	}

	if (j >= 4) {
		ERROR("taamin_ps out of range.\n");
		return 0;
	}

	if ((bin[i].taamin_ps[j] == 0) ||
	    (bin[i].taamin_ps[j] > taamin_ps && j > 0)) {
		j--;
	}

	for (k = 0; bin[i].cl[k].tckmin_ps < mclk_ps &&
		    bin[i].cl[k].tckmin_ps < taamin_max; k++)
		;
	if (bin[i].cl[k].tckmin_ps > mclk_ps && k > 0) {
		k--;
	}

	debug("Skip CL mask for this speed 0x%x\n", bin[i].cl[k].caslat[j]);

	return bin[i].cl[k].caslat[j];
}

int compute_ddrc(const unsigned long clk,
		 const struct memctl_opt *popts,
		 const struct ddr_conf *conf,
		 struct ddr_cfg_regs *regs,
		 const struct dimm_params *pdimm,
		 unsigned int ip_rev)
{
	unsigned int cas_latency;
	unsigned int caslat_skip;
	unsigned int additive_latency;
	const unsigned int mclk_ps = get_memory_clk_ps(clk);
	int i;

	zeromem(regs, sizeof(struct ddr_cfg_regs));

	if (mclk_ps < pdimm->tckmin_x_ps) {
		ERROR("DDR Clk: MCLK cycle is %u ps.\n", mclk_ps);
		ERROR("DDR Clk is faster than DIMM can support.\n");
	}

	/* calculate cas latency, override first */
	cas_latency = (popts->caslat_override != 0) ?
			popts->caslat_override_value :
			(pdimm->taa_ps + mclk_ps - 1) / mclk_ps;

	/* skip unsupported caslat based on speed bin */
	caslat_skip = skip_caslat(pdimm->tckmin_x_ps,
				  pdimm->taa_ps,
				  mclk_ps,
				  pdimm->package_3ds);
	debug("Skip caslat 0x%x\n", caslat_skip);

	/* Check if DIMM supports the cas latency */
	i = 24;
	while (((pdimm->caslat_x & ~caslat_skip & (1 << cas_latency)) == 0) &&
	       (i-- > 0)) {
		cas_latency++;
	}

	if (i <= 0) {
		ERROR("Failed to find a proper cas latency\n");
		return -EINVAL;
	}
	/* Verify cas latency does not exceed 18ns for DDR4 */
	if (cas_latency * mclk_ps > 18000) {
		ERROR("cas latency is too large %d\n", cas_latency);
		return -EINVAL;
	}

	additive_latency = (popts->addt_lat_override != 0) ?
				popts->addt_lat_override_value : 0;

	cal_ddr_csn_bnds(regs, popts, conf, pdimm);
	cal_ddr_sdram_cfg(clk, regs, popts, pdimm, ip_rev);
	cal_ddr_sdram_rcw(clk, regs, popts, pdimm);
	cal_timing_cfg(clk, regs, popts, pdimm, conf, cas_latency,
		       additive_latency);
	cal_ddr_dq_mapping(regs, pdimm);

	if (ip_rev >= 0x50500) {
		cal_ddr_addr_dec(regs);
	}

	cal_ddr_sdram_mode(clk, regs, popts, conf, pdimm, cas_latency,
			   additive_latency, ip_rev);
	cal_ddr_eor(regs, popts);
	cal_ddr_data_init(regs);
	cal_ddr_sdram_interval(clk, regs, popts, pdimm);
	cal_ddr_zq_cntl(regs);
	cal_ddr_sr_cntr(regs, popts);

	return 0;
}

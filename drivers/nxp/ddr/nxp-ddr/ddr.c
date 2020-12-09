/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/debug.h>
#include <ddr.h>
#ifndef CONFIG_DDR_NODIMM
#include <i2c.h>
#endif
#include <nxp_timer.h>

struct dynamic_odt {
	unsigned int odt_rd_cfg;
	unsigned int odt_wr_cfg;
	unsigned int odt_rtt_norm;
	unsigned int odt_rtt_wr;
};

#ifndef CONFIG_STATIC_DDR
#if defined(PHY_GEN2_FW_IMAGE_BUFFER) && !defined(NXP_DDR_PHY_GEN2)
#error Missing NXP_DDR_PHY_GEN2
#endif
#ifdef NXP_DDR_PHY_GEN2
static const struct dynamic_odt single_D[4] = {
	{	/* cs0 */
		DDR_ODT_NEVER,
		DDR_ODT_ALL,
		DDR4_RTT_80_OHM,
		DDR4_RTT_WR_OFF
	},
	{	/* cs1 */
		DDR_ODT_NEVER,
		DDR_ODT_NEVER,
		DDR4_RTT_OFF,
		DDR4_RTT_WR_OFF
	},
	{},
	{}
};

static const struct dynamic_odt single_S[4] = {
	{	/* cs0 */
		DDR_ODT_NEVER,
		DDR_ODT_ALL,
		DDR4_RTT_80_OHM,
		DDR4_RTT_WR_OFF
	},
	{},
	{},
	{},
};

static const struct dynamic_odt dual_DD[4] = {
	{	/* cs0 */
		DDR_ODT_OTHER_DIMM,
		DDR_ODT_ALL,
		DDR4_RTT_60_OHM,
		DDR4_RTT_WR_240_OHM
	},
	{	/* cs1 */
		DDR_ODT_OTHER_DIMM,
		DDR_ODT_ALL,
		DDR4_RTT_60_OHM,
		DDR4_RTT_WR_240_OHM
	},
	{	/* cs2 */
		DDR_ODT_OTHER_DIMM,
		DDR_ODT_ALL,
		DDR4_RTT_60_OHM,
		DDR4_RTT_WR_240_OHM
	},
	{	/* cs3 */
		DDR_ODT_OTHER_DIMM,
		DDR_ODT_ALL,
		DDR4_RTT_60_OHM,
		DDR4_RTT_WR_240_OHM
	}
};

static const struct dynamic_odt dual_SS[4] = {
	{	/* cs0 */
		DDR_ODT_NEVER,
		DDR_ODT_ALL,
		DDR4_RTT_80_OHM,
		DDR4_RTT_WR_OFF
	},
	{},
	{	/* cs2 */
		DDR_ODT_NEVER,
		DDR_ODT_ALL,
		DDR4_RTT_80_OHM,
		DDR4_RTT_WR_OFF
	},
	{}
};

static const struct dynamic_odt dual_D0[4] = {
	{	/* cs0 */
		DDR_ODT_NEVER,
		DDR_ODT_SAME_DIMM,
		DDR4_RTT_80_OHM,
		DDR4_RTT_WR_OFF
	},
	{	/* cs1 */
		DDR_ODT_NEVER,
		DDR_ODT_NEVER,
		DDR4_RTT_80_OHM,
		DDR4_RTT_WR_OFF
	},
	{},
	{}
};

static const struct dynamic_odt dual_S0[4] = {
	{	/* cs0 */
		DDR_ODT_NEVER,
		DDR_ODT_CS,
		DDR4_RTT_80_OHM,
		DDR4_RTT_WR_OFF
	},
	{},
	{},
	{}
};
#else
static const struct dynamic_odt single_D[4] = {
	{	/* cs0 */
		DDR_ODT_NEVER,
		DDR_ODT_ALL,
		DDR4_RTT_40_OHM,
		DDR4_RTT_WR_OFF
	},
	{	/* cs1 */
		DDR_ODT_NEVER,
		DDR_ODT_NEVER,
		DDR4_RTT_OFF,
		DDR4_RTT_WR_OFF
	},
	{},
	{}
};

static const struct dynamic_odt single_S[4] = {
	{	/* cs0 */
		DDR_ODT_NEVER,
		DDR_ODT_ALL,
		DDR4_RTT_40_OHM,
		DDR4_RTT_WR_OFF
	},
	{},
	{},
	{},
};

static const struct dynamic_odt dual_DD[4] = {
	{	/* cs0 */
		DDR_ODT_NEVER,
		DDR_ODT_SAME_DIMM,
		DDR4_RTT_120_OHM,
		DDR4_RTT_WR_OFF
	},
	{	/* cs1 */
		DDR_ODT_OTHER_DIMM,
		DDR_ODT_OTHER_DIMM,
		DDR4_RTT_34_OHM,
		DDR4_RTT_WR_OFF
	},
	{	/* cs2 */
		DDR_ODT_NEVER,
		DDR_ODT_SAME_DIMM,
		DDR4_RTT_120_OHM,
		DDR4_RTT_WR_OFF
	},
	{	/* cs3 */
		DDR_ODT_OTHER_DIMM,
		DDR_ODT_OTHER_DIMM,
		DDR4_RTT_34_OHM,
		DDR4_RTT_WR_OFF
	}
};

static const struct dynamic_odt dual_SS[4] = {
	{	/* cs0 */
		DDR_ODT_OTHER_DIMM,
		DDR_ODT_ALL,
		DDR4_RTT_34_OHM,
		DDR4_RTT_WR_120_OHM
	},
	{},
	{	/* cs2 */
		DDR_ODT_OTHER_DIMM,
		DDR_ODT_ALL,
		DDR4_RTT_34_OHM,
		DDR4_RTT_WR_120_OHM
	},
	{}
};

static const struct dynamic_odt dual_D0[4] = {
	{	/* cs0 */
		DDR_ODT_NEVER,
		DDR_ODT_SAME_DIMM,
		DDR4_RTT_40_OHM,
		DDR4_RTT_WR_OFF
	},
	{	/* cs1 */
		DDR_ODT_NEVER,
		DDR_ODT_NEVER,
		DDR4_RTT_OFF,
		DDR4_RTT_WR_OFF
	},
	{},
	{}
};

static const struct dynamic_odt dual_S0[4] = {
	{	/* cs0 */
		DDR_ODT_NEVER,
		DDR_ODT_CS,
		DDR4_RTT_40_OHM,
		DDR4_RTT_WR_OFF
	},
	{},
	{},
	{}
};
#endif /* NXP_DDR_PHY_GEN2 */

/*
 * Automatically select bank interleaving mode based on DIMMs
 * in this order: cs0_cs1_cs2_cs3, cs0_cs1, null.
 * This function only deal with one or two slots per controller.
 */
static inline unsigned int auto_bank_intlv(const int cs_in_use,
					   const struct dimm_params *pdimm)
{
	switch (cs_in_use) {
	case 0xf:
		return DDR_BA_INTLV_CS0123;
	case 0x3:
		return DDR_BA_INTLV_CS01;
	case 0x1:
		return DDR_BA_NONE;
	case 0x5:
		return DDR_BA_NONE;
	default:
		break;
	}

	return 0U;
}

static int cal_odt(const unsigned int clk,
		   struct memctl_opt *popts,
		   struct ddr_conf *conf,
		   struct dimm_params *pdimm,
		   const int dimm_slot_per_ctrl)

{
	unsigned int i;
	const struct dynamic_odt *pdodt = NULL;

	const static struct dynamic_odt *table[2][5] = {
		{single_S, single_D, NULL, NULL},
		{dual_SS, dual_DD, NULL, NULL},
	};

	if (dimm_slot_per_ctrl != 1 && dimm_slot_per_ctrl != 2) {
		ERROR("Unsupported number of DIMMs\n");
		return -EINVAL;
	}

	pdodt = table[dimm_slot_per_ctrl - 1][pdimm->n_ranks - 1];
	if (pdodt == dual_SS) {
		pdodt = (conf->cs_in_use == 0x5) ? dual_SS :
			((conf->cs_in_use == 0x1) ? dual_S0 : NULL);
	} else if (pdodt == dual_DD) {
		pdodt = (conf->cs_in_use == 0xf) ? dual_DD :
			((conf->cs_in_use == 0x3) ? dual_D0 : NULL);
	}
	if (pdodt == dual_DD && pdimm->package_3ds) {
		ERROR("Too many 3DS DIMMs.\n");
		return -EINVAL;
	}

	if (pdodt == NULL) {
		ERROR("Error determing ODT.\n");
		return -EINVAL;
	}

	/* Pick chip-select local options. */
	for (i = 0U; i < DDRC_NUM_CS; i++) {
		debug("cs %d\n", i);
		popts->cs_odt[i].odt_rd_cfg = pdodt[i].odt_rd_cfg;
		debug("     odt_rd_cfg 0x%x\n",
			  popts->cs_odt[i].odt_rd_cfg);
		popts->cs_odt[i].odt_wr_cfg = pdodt[i].odt_wr_cfg;
		debug("     odt_wr_cfg 0x%x\n",
			  popts->cs_odt[i].odt_wr_cfg);
		popts->cs_odt[i].odt_rtt_norm = pdodt[i].odt_rtt_norm;
		debug("     odt_rtt_norm 0x%x\n",
			  popts->cs_odt[i].odt_rtt_norm);
		popts->cs_odt[i].odt_rtt_wr = pdodt[i].odt_rtt_wr;
		debug("     odt_rtt_wr 0x%x\n",
			  popts->cs_odt[i].odt_rtt_wr);
		popts->cs_odt[i].auto_precharge = 0;
		debug("     auto_precharge %d\n",
			  popts->cs_odt[i].auto_precharge);
	}

	return 0;
}

static int cal_opts(const unsigned int clk,
		    struct memctl_opt *popts,
		    struct ddr_conf *conf,
		    struct dimm_params *pdimm,
		    const int dimm_slot_per_ctrl,
		    const unsigned int ip_rev)
{
	popts->rdimm = pdimm->rdimm;
	popts->mirrored_dimm = pdimm->mirrored_dimm;
#ifdef CONFIG_DDR_ECC_EN
	popts->ecc_mode = pdimm->edc_config == 0x02 ? 1 : 0;
#endif
	popts->ctlr_init_ecc = popts->ecc_mode;
	debug("ctlr_init_ecc %d\n", popts->ctlr_init_ecc);
	popts->self_refresh_in_sleep = 1;
	popts->dynamic_power = 0;

	/*
	 * check sdram width, allow platform override
	 * 0 = 64-bit, 1 = 32-bit, 2 = 16-bit
	 */
	if (pdimm->primary_sdram_width == 64) {
		popts->data_bus_dimm = DDR_DBUS_64;
		popts->otf_burst_chop_en = 1;
	} else if (pdimm->primary_sdram_width == 32) {
		popts->data_bus_dimm = DDR_DBUS_32;
		popts->otf_burst_chop_en = 0;
	} else if (pdimm->primary_sdram_width == 16) {
		popts->data_bus_dimm = DDR_DBUS_16;
		popts->otf_burst_chop_en = 0;
	} else {
		ERROR("primary sdram width invalid!\n");
		return -EINVAL;
	}
	popts->data_bus_used = popts->data_bus_dimm;
	popts->x4_en = (pdimm->device_width == 4) ? 1 : 0;
	debug("x4_en %d\n", popts->x4_en);

	/* for RDIMM and DDR4 UDIMM/discrete memory, address parity enable */
	if (popts->rdimm != 0) {
		popts->ap_en = 1; /* 0 = disable,  1 = enable */
	} else {
		popts->ap_en = 0; /* disabled for DDR4 UDIMM/discrete default */
	}

	if (ip_rev == 0x50500) {
		popts->ap_en = 0;
	}

	debug("ap_en %d\n", popts->ap_en);

	/* BSTTOPRE precharge interval uses 1/4 of refint value. */
	popts->bstopre = picos_to_mclk(clk, pdimm->refresh_rate_ps) >> 2;
	popts->tfaw_ps = pdimm->tfaw_ps;

	return 0;
}

static void cal_intlv(const int num_ctlrs,
		      struct memctl_opt *popts,
		      struct ddr_conf *conf,
		      struct dimm_params *pdimm)
{
#ifdef NXP_DDR_INTLV_256B
	if (num_ctlrs == 2) {
		popts->ctlr_intlv = 1;
		popts->ctlr_intlv_mode = DDR_256B_INTLV;
	}
#endif
	debug("ctlr_intlv %d\n", popts->ctlr_intlv);
	debug("ctlr_intlv_mode %d\n", popts->ctlr_intlv_mode);

	popts->ba_intlv = auto_bank_intlv(conf->cs_in_use, pdimm);
	debug("ba_intlv 0x%x\n", popts->ba_intlv);
}

static int update_burst_length(struct memctl_opt *popts)
{
	/* Choose burst length. */
	if ((popts->data_bus_used == DDR_DBUS_32) ||
	    (popts->data_bus_used == DDR_DBUS_16)) {
		/* 32-bit or 16-bit bus */
		popts->otf_burst_chop_en = 0;
		popts->burst_length = DDR_BL8;
	} else if (popts->otf_burst_chop_en != 0) { /* on-the-fly burst chop */
		popts->burst_length = DDR_OTF;	/* on-the-fly BC4 and BL8 */
	} else {
		popts->burst_length = DDR_BL8;
	}
	debug("data_bus_used %d\n", popts->data_bus_used);
	debug("otf_burst_chop_en %d\n", popts->otf_burst_chop_en);
	debug("burst_length 0x%x\n", popts->burst_length);
	/*
	 * If a reduced data width is requested, but the SPD
	 * specifies a physically wider device, adjust the
	 * computed dimm capacities accordingly before
	 * assigning addresses.
	 * 0 = 64-bit, 1 = 32-bit, 2 = 16-bit
	 */
	if (popts->data_bus_dimm > popts->data_bus_used) {
		ERROR("Data bus configuration error\n");
		return -EINVAL;
	}
	popts->dbw_cap_shift = popts->data_bus_used - popts->data_bus_dimm;
	debug("dbw_cap_shift %d\n", popts->dbw_cap_shift);

	return 0;
}

int cal_board_params(struct ddr_info *priv,
		     const struct board_timing *dimm,
		     int len)
{
	const unsigned long speed = priv->clk / 1000000;
	const struct dimm_params *pdimm = &priv->dimm;
	struct memctl_opt *popts = &priv->opt;
	struct rc_timing const *prt = NULL;
	struct rc_timing const *chosen = NULL;
	int i;

	for (i = 0; i < len; i++) {
		if (pdimm->rc == dimm[i].rc) {
			prt = dimm[i].p;
			break;
		}
	}
	if (prt == NULL) {
		ERROR("Board parameters no match.\n");
		return -EINVAL;
	}
	while (prt->speed_bin != 0) {
		if (speed <= prt->speed_bin) {
			chosen = prt;
			break;
		}
		prt++;
	}
	if (chosen == NULL) {
		ERROR("timing no match for speed %lu\n", speed);
		return -EINVAL;
	}
	popts->clk_adj = prt->clk_adj;
	popts->wrlvl_start = prt->wrlvl;
	popts->wrlvl_ctl_2 = (prt->wrlvl * 0x01010101 + dimm[i].add1) &
			     0xFFFFFFFF;
	popts->wrlvl_ctl_3 = (prt->wrlvl * 0x01010101 + dimm[i].add2) &
			     0xFFFFFFFF;

	return 0;
}

static int synthesize_ctlr(struct ddr_info *priv)
{
	int ret;

	ret = cal_odt(priv->clk,
		      &priv->opt,
		      &priv->conf,
		      &priv->dimm,
		      priv->dimm_on_ctlr);
	if (ret != 0) {
		return ret;
	}

	ret = cal_opts(priv->clk,
		       &priv->opt,
		       &priv->conf,
		       &priv->dimm,
		       priv->dimm_on_ctlr,
		       priv->ip_rev);

	if (ret != 0) {
		return ret;
	}

	cal_intlv(priv->num_ctlrs, &priv->opt, &priv->conf, &priv->dimm);
	ret = ddr_board_options(priv);
	if (ret != 0) {
		ERROR("Failed matching board timing.\n");
	}

	ret = update_burst_length(&priv->opt);

	return ret;
}

/* Return the bit mask of valid DIMMs found */
static int parse_spd(struct ddr_info *priv)
{
	struct ddr_conf *conf = &priv->conf;
	struct dimm_params *dimm = &priv->dimm;
	int j, valid_mask = 0;

#ifdef CONFIG_DDR_NODIMM
	valid_mask = ddr_get_ddr_params(dimm, conf);
	if (valid_mask < 0) {
		ERROR("DDR params error\n");
		return valid_mask;
	}
#else
	const int *spd_addr = priv->spd_addr;
	const int num_ctlrs = priv->num_ctlrs;
	const int num_dimm = priv->dimm_on_ctlr;
	struct ddr4_spd spd[2];
	unsigned int spd_checksum[2];
	int addr_idx = 0;
	int spd_idx = 0;
	int ret, addr, i;

	/* Scan all DIMMs */
	for (i = 0; i < num_ctlrs; i++) {
		debug("Controller %d\n", i);
		for (j = 0; j < num_dimm; j++, addr_idx++) {
			debug("DIMM %d\n", j);
			addr = spd_addr[addr_idx];
			if (addr == 0) {
				if (j == 0) {
					ERROR("First SPD addr wrong.\n");
					return -EINVAL;
				}
				continue;
			}
			debug("addr 0x%x\n", addr);
			ret = read_spd(addr, &spd[spd_idx],
				       sizeof(struct ddr4_spd));
			if (ret != 0) {	/* invalid */
				debug("Invalid SPD at address 0x%x\n", addr);
				continue;
			}

			spd_checksum[spd_idx] =
				(spd[spd_idx].crc[1] << 24) |
				(spd[spd_idx].crc[0] << 16) |
				(spd[spd_idx].mod_section.uc[127] << 8) |
				(spd[spd_idx].mod_section.uc[126] << 0);
			debug("checksum 0x%x\n", spd_checksum[spd_idx]);
			if (spd_checksum[spd_idx] == 0) {
				debug("Bad checksum, ignored.\n");
				continue;
			}
			if (spd_idx == 0) {
				/* first valid SPD */
				ret = cal_dimm_params(&spd[0], dimm);
				if (ret != 0) {
					ERROR("SPD calculation error\n");
					return -EINVAL;
				}
			}

			if (spd_idx != 0 && spd_checksum[0] !=
			    spd_checksum[spd_idx]) {
				ERROR("Not identical DIMMs.\n");
				return -EINVAL;
			}
			conf->dimm_in_use[j] = 1;
			valid_mask |= 1 << addr_idx;
			spd_idx = 1;
		}
		debug("done with controller %d\n", i);
	}
	switch (num_ctlrs) {
	case 1:
		if ((valid_mask & 0x1) == 0) {
			ERROR("First slot cannot be empty.\n");
			return -EINVAL;
		}
		break;
	case 2:
		switch (num_dimm) {
		case 1:
			if (valid_mask == 0) {
				ERROR("Both slot empty\n");
				return -EINVAL;
			}
			break;
		case 2:
			if (valid_mask != 0x5 &&
			    valid_mask != 0xf &&
			    (valid_mask & 0x7) != 0x4 &&
			    (valid_mask & 0xd) != 0x1) {
				ERROR("Invalid DIMM combination.\n");
				return -EINVAL;
			}
			break;
		default:
			ERROR("Invalid number of DIMMs.\n");
			return -EINVAL;
		}
		break;
	default:
		ERROR("Invalid number of controllers.\n");
		return -EINVAL;
	}
	/* now we have valid and identical DIMMs on controllers */
#endif	/* CONFIG_DDR_NODIMM */

	debug("cal cs\n");
	conf->cs_in_use = 0;
	for (j = 0; j < DDRC_NUM_DIMM; j++) {
		if (conf->dimm_in_use[j] == 0) {
			continue;
		}
		switch (dimm->n_ranks) {
		case 4:
			ERROR("Quad-rank DIMM not supported\n");
			return -EINVAL;
		case 2:
			conf->cs_on_dimm[j] = 0x3 << (j * CONFIG_CS_PER_SLOT);
			conf->cs_in_use |= conf->cs_on_dimm[j];
			break;
		case 1:
			conf->cs_on_dimm[j] = 0x1 << (j * CONFIG_CS_PER_SLOT);
			conf->cs_in_use |= conf->cs_on_dimm[j];
			break;
		default:
			ERROR("SPD error with n_ranks\n");
			return -EINVAL;
		}
		debug("cs_in_use = %x\n", conf->cs_in_use);
		debug("cs_on_dimm[%d] = %x\n", j, conf->cs_on_dimm[j]);
	}
#ifndef CONFIG_DDR_NODIMM
	if (priv->dimm.rdimm != 0) {
		NOTICE("RDIMM %s\n", priv->dimm.mpart);
	} else {
		NOTICE("UDIMM %s\n", priv->dimm.mpart);
	}
#else
	NOTICE("%s\n", priv->dimm.mpart);
#endif

	return valid_mask;
}

static unsigned long long assign_intlv_addr(
	const struct dimm_params *pdimm,
	const struct memctl_opt *opt,
	struct ddr_conf *conf,
	const unsigned long long current_mem_base)
{
	int i;
	int ctlr_density_mul = 0;
	const unsigned long long rank_density = pdimm->rank_density >>
						opt->dbw_cap_shift;
	unsigned long long total_ctlr_mem;

	debug("rank density 0x%llx\n", rank_density);
	switch (opt->ba_intlv & DDR_BA_INTLV_CS0123) {
	case DDR_BA_INTLV_CS0123:
		ctlr_density_mul = 4;
		break;
	case DDR_BA_INTLV_CS01:
		ctlr_density_mul = 2;
		break;
	default:
		ctlr_density_mul = 1;
		break;
	}
	debug("ctlr density mul %d\n", ctlr_density_mul);
	switch (opt->ctlr_intlv_mode) {
	case DDR_256B_INTLV:
		total_ctlr_mem = 2 * ctlr_density_mul * rank_density;
		break;
	default:
		ERROR("Unknown interleaving mode");
		return 0;
	}
	conf->base_addr = current_mem_base;
	conf->total_mem = total_ctlr_mem;

	/* overwrite cs_in_use bitmask with controller interleaving */
	conf->cs_in_use = (1 << ctlr_density_mul) - 1;
	debug("Overwrite cs_in_use as %x\n", conf->cs_in_use);

	/* Fill addr with each cs in use */
	for (i = 0; i < ctlr_density_mul; i++) {
		conf->cs_base_addr[i] = current_mem_base;
		conf->cs_size[i] = total_ctlr_mem;
		debug("CS %d\n", i);
		debug("    base_addr 0x%llx\n", conf->cs_base_addr[i]);
		debug("    size 0x%llx\n", conf->cs_size[i]);
	}

	return total_ctlr_mem;
}

static unsigned long long assign_non_intlv_addr(
	const struct dimm_params *pdimm,
	const struct memctl_opt *opt,
	struct ddr_conf *conf,
	unsigned long long current_mem_base)
{
	int i;
	const unsigned long long rank_density = pdimm->rank_density >>
						opt->dbw_cap_shift;
	unsigned long long total_ctlr_mem = 0ULL;

	debug("rank density 0x%llx\n", rank_density);
	conf->base_addr = current_mem_base;

	/* assign each cs */
	switch (opt->ba_intlv & DDR_BA_INTLV_CS0123) {
	case DDR_BA_INTLV_CS0123:
		for (i = 0; i < DDRC_NUM_CS; i++) {
			conf->cs_base_addr[i] = current_mem_base;
			conf->cs_size[i] = rank_density << 2;
			total_ctlr_mem += rank_density;
		}
		break;
	case DDR_BA_INTLV_CS01:
		for (i = 0; ((conf->cs_in_use & (1 << i)) != 0) && i < 2; i++) {
			conf->cs_base_addr[i] = current_mem_base;
			conf->cs_size[i] = rank_density << 1;
			total_ctlr_mem += rank_density;
		}
		current_mem_base += total_ctlr_mem;
		for (; ((conf->cs_in_use & (1 << i)) != 0) && i < DDRC_NUM_CS;
		     i++) {
			conf->cs_base_addr[i] = current_mem_base;
			conf->cs_size[i] = rank_density;
			total_ctlr_mem += rank_density;
			current_mem_base += rank_density;
		}
		break;
	case DDR_BA_NONE:
		for (i = 0; ((conf->cs_in_use & (1 << i)) != 0) &&
			     (i < DDRC_NUM_CS); i++) {
			conf->cs_base_addr[i] = current_mem_base;
			conf->cs_size[i] = rank_density;
			current_mem_base += rank_density;
			total_ctlr_mem += rank_density;
		}
		break;
	default:
		ERROR("Unsupported bank interleaving\n");
		return 0;
	}
	for (i = 0; ((conf->cs_in_use & (1 << i)) != 0) &&
		     (i < DDRC_NUM_CS); i++) {
		debug("CS %d\n", i);
		debug("    base_addr 0x%llx\n", conf->cs_base_addr[i]);
		debug("    size 0x%llx\n", conf->cs_size[i]);
	}

	return total_ctlr_mem;
}

unsigned long long assign_addresses(struct ddr_info *priv)
		   __attribute__ ((weak));

unsigned long long assign_addresses(struct ddr_info *priv)
{
	struct memctl_opt *opt = &priv->opt;
	const struct dimm_params *dimm = &priv->dimm;
	struct ddr_conf *conf = &priv->conf;
	unsigned long long current_mem_base = priv->mem_base;
	unsigned long long total_mem;

	total_mem = 0ULL;
	debug("ctlr_intlv %d\n", opt->ctlr_intlv);
	if (opt->ctlr_intlv != 0) {
		total_mem = assign_intlv_addr(dimm, opt, conf,
					      current_mem_base);
	} else {
		/*
		 * Simple linear assignment if memory controllers are not
		 * interleaved. This is only valid for SoCs with single DDRC.
		 */
		total_mem = assign_non_intlv_addr(dimm, opt, conf,
						  current_mem_base);
	}
	conf->total_mem = total_mem;
	debug("base 0x%llx\n", current_mem_base);
	debug("Total mem by assignment is 0x%llx\n", total_mem);

	return total_mem;
}

static int cal_ddrc_regs(struct ddr_info *priv)
{
	int ret;

	ret = compute_ddrc(priv->clk,
			   &priv->opt,
			   &priv->conf,
			   &priv->ddr_reg,
			   &priv->dimm,
			   priv->ip_rev);
	if (ret != 0) {
		ERROR("Calculating DDR registers failed\n");
	}

	return ret;
}

#endif /* CONFIG_STATIC_DDR */

static int write_ddrc_regs(struct ddr_info *priv)
{
	int i;
	int ret;

	for (i = 0; i < priv->num_ctlrs; i++) {
		ret = ddrc_set_regs(priv->clk, &priv->ddr_reg, priv->ddr[i], 0);
		if (ret != 0) {
			ERROR("Writing DDR register(s) failed\n");
			return ret;
		}
	}

	return 0;
}

long long dram_init(struct ddr_info *priv
#if defined(NXP_HAS_CCN504) || defined(NXP_HAS_CCN508)
		    , uintptr_t nxp_ccn_hn_f0_addr
#endif
		)
{
	uint64_t time __unused;
	long long dram_size;
	int ret;
	const uint64_t time_base = get_timer_val(0);
	unsigned int ip_rev = get_ddrc_version(priv->ddr[0]);

	int valid_spd_mask __unused;
	int scratch = 0x0;

	priv->ip_rev = ip_rev;

#ifndef CONFIG_STATIC_DDR
	INFO("time base %llu ms\n", time_base);
	debug("Parse DIMM SPD(s)\n");
	valid_spd_mask = parse_spd(priv);

	if (valid_spd_mask < 0) {
		ERROR("Parsing DIMM Error\n");
		return valid_spd_mask;
	}

#if defined(NXP_HAS_CCN504) || defined(NXP_HAS_CCN508)
	if (priv->num_ctlrs == 2 || priv->num_ctlrs == 1) {
		ret = disable_unused_ddrc(priv, valid_spd_mask,
					  nxp_ccn_hn_f0_addr);
		if (ret != 0) {
			return ret;
		}
	}
#endif

	time = get_timer_val(time_base);
	INFO("Time after parsing SPD %llu ms\n", time);
	debug("Synthesize configurations\n");
	ret = synthesize_ctlr(priv);
	if (ret != 0) {
		ERROR("Synthesize config error\n");
		return ret;
	}

	debug("Assign binding addresses\n");
	dram_size = assign_addresses(priv);
	if (dram_size == 0) {
		ERROR("Assigning address error\n");
		return -EINVAL;
	}

	debug("Calculate controller registers\n");
	ret = cal_ddrc_regs(priv);
	if (ret != 0) {
		ERROR("Calculate register error\n");
		return ret;
	}

	ret = compute_ddr_phy(priv);
	if (ret != 0)
		ERROR("Calculating DDR PHY registers failed.\n");

#else
	dram_size = board_static_ddr(priv);
	if (dram_size == 0) {
		ERROR("Error getting static DDR settings.\n");
		return -EINVAL;
	}
#endif

	if (priv->warm_boot_flag == DDR_WARM_BOOT) {
		scratch = (priv->ddr_reg).sdram_cfg[1];
		scratch = scratch & ~(SDRAM_CFG2_D_INIT);
		priv->ddr_reg.sdram_cfg[1] = scratch;
	}

	time = get_timer_val(time_base);
	INFO("Time before programming controller %llu ms\n", time);
	debug("Program controller registers\n");
	ret = write_ddrc_regs(priv);
	if (ret != 0) {
		ERROR("Programing DDRC error\n");
		return ret;
	}

	puts("");
	NOTICE("%lld GB ", dram_size >> 30);
	print_ddr_info(priv->ddr[0]);

	time = get_timer_val(time_base);
	INFO("Time used by DDR driver %llu ms\n", time);

	return dram_size;
}

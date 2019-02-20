/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <platform_def.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <common/debug.h>
#include <errno.h>
#include <io.h>
#include <drivers/delay_timer.h>
#include "ddr.h"
#include "immap.h"

#define BIST_CR		0x80060000
#define BIST_CR_EN	0x80000000
#define BIST_CR_STAT	0x00000001
#define CTLR_INTLV_MASK	0x20000000

bool run_bist(void)  __attribute__ ((weak));
bool run_bist(void)
{
#ifdef BIST_EN
	return true;
#else
	return false;
#endif
}

/*
 * Perform build-in test on memory
 * timeout value in 10ms
 */
int bist(const struct ccsr_ddr *ddr, int timeout)
{
	const unsigned int test_pattern[10] = {
		0xffffffff,
		0x00000000,
		0xaaaaaaaa,
		0x55555555,
		0xcccccccc,
		0x33333333,
		0x12345678,
		0xabcdef01,
		0xaa55aa55,
		0x55aa55aa
	};
	unsigned int mtcr, err_detect, err_sbe;
	unsigned int cs0_config;
	unsigned int csn_bnds[4];
	int ret = 0;
	int i;
#ifdef CONFIG_DDR_ADDR_DEC
	uint32_t dec_9 = ddr_in32(&ddr->dec[9]);
	uint32_t pos = 0;
	uint32_t map_save = 0;
	uint32_t temp32 = 0;
	uint32_t map, shift, highest;
#endif

	cs0_config = ddr_in32(&ddr->csn_cfg[0]);
	if (cs0_config & CTLR_INTLV_MASK) {
		/* set bnds to non-interleaving */
		for (i = 0; i < 4; i++) {
			csn_bnds[i] = ddr_in32(&ddr->bnds[i].a);
			ddr_out32(&ddr->bnds[i].a,
				  (csn_bnds[i] & 0xfffefffe) >> 1);
		}
		ddr_out32(&ddr->csn_cfg[0], cs0_config & ~CTLR_INTLV_MASK);
#ifdef CONFIG_DDR_ADDR_DEC
		if (dec_9 & 0x1) {
			highest = (dec_9 >> 26) == 0x3F ? 0 : dec_9 >> 26;
			pos = 37;
			for (i = 0; i < 36; i++) {      /* Go through all 37 */
				if (!(i % 4))
					temp32 = ddr_in32(&ddr->dec[i >> 2]);
				shift = (3 - i % 4) * 8 + 2;
				map = (temp32 >> shift) & 0x3F;
				if (map > highest && map != 0x3F) {
					highest = map;
					pos = i;
				}
			}
			debug("\nFound highest position %d, mapping to %d, ",
			      pos, highest);
			map_save = ddr_in32(&ddr->dec[pos >> 2]);
			shift = (3 - pos % 4) * 8 + 2;
			debug("in dec[%d], bit %d (0x%x)\n",
			      pos >> 2, shift, map_save);
			temp32 = map_save & ~(0x3F << shift);
			temp32 |= 8 << shift;
			ddr_out32(&ddr->dec[pos >> 2], temp32);
			timeout <<= 2;	/* increase timeout value */
			debug("Increase wait time to %d ms\n", timeout * 10);
		}
#endif
	}
	for (i = 0; i < 10; i++)
		ddr_out32(&ddr->mtp[i], test_pattern[i]);
	mtcr = BIST_CR;
	ddr_out32(&ddr->mtcr, mtcr);
	do {
		mdelay(10);
		mtcr = ddr_in32(&ddr->mtcr);
	} while (timeout-- > 0 && (mtcr & BIST_CR_EN));
	if (timeout <= 0)
		ERROR("Timeout\n");
	else
		debug("Timer remains %d\n", timeout);

	err_detect = ddr_in32(&ddr->err_detect);
	err_sbe = ddr_in32(&ddr->err_sbe);
	if (err_detect || (err_sbe & 0xffff)) {
		ERROR("ECC error detected\n");
		ret = -EIO;
	}

	if (cs0_config & CTLR_INTLV_MASK) {
		for (i = 0; i < 4; i++)
			ddr_out32(&ddr->bnds[i].a, csn_bnds[i]);
		ddr_out32(&ddr->csn_cfg[0], cs0_config);
#ifdef CONFIG_DDR_ADDR_DEC
		if (dec_9 & 0x1)
			ddr_out32(&ddr->dec[pos >> 2], map_save);
#endif
	}
	if (mtcr & BIST_CR_STAT) {
		ERROR("Built-in self test failed\n");
		ret = -EIO;
	} else {
		NOTICE("Build-in self test passed\n");
	}

	return ret;
}

void dump_ddrc(unsigned int *ddr)
{
#ifdef DDR_DEBUG
	int i;
	unsigned long val;

	for (i = 0; i < 0x400; i++, ddr++) {
		val = ddr_in32(ddr);
		if (!val)	/* skip zeros */
			continue;
		debug("*0x%lx = 0x%lx\n", (unsigned long)ddr, val);
	}
#endif
}

#ifdef NXP_ERRATUM_A009803
static void set_wait_for_bits_clear(const void *ptr,
				    unsigned int value,
				    unsigned int bits)
{
	int timeout = 1000;

	ddr_out32(ptr, value);
	do {
		udelay(100);
	} while (timeout-- > 0 && (ddr_in32(ptr) & bits));

	if (timeout <= 0)
		ERROR("wait for clear timeout.\n");
}
#endif

#if (DDRC_NUM_CS > 4)
#error Invalid setting for DDRC_NUM_CS
#endif

/*
 * If supported by the platform, writing to DDR controller takes two
 * passes to deassert DDR reset to comply with JEDEC specs for RDIMMs.
 */
int ddrc_set_regs(const unsigned long clk,
		  const struct ddr_cfg_regs *regs,
		  const struct ccsr_ddr *ddr,
		  int twopass)
{
	unsigned int i, bus_width;
	unsigned int temp_sdram_cfg;
	unsigned int total_mem_per_ctrl, total_mem_per_ctrl_adj;
	const int mod_bnds = regs->cs[0].config & CTLR_INTLV_MASK;
	int timeout;
	int ret = 0;
#if defined(NXP_ERRATUM_A009942) || defined(NXP_ERRATUM_A010165)
	unsigned long ddr_freq;
	unsigned int tmp;
#ifdef NXP_ERRATUM_A009942
	unsigned int check;
	unsigned int cpo_min = 0xff;
	unsigned int cpo_max = 0;
#endif
#endif

	if (twopass == 2)
		goto after_reset;

	/* Set cdr1 first in case 0.9v VDD is enabled for some SoCs*/
	ddr_out32(&ddr->ddr_cdr1, regs->cdr[0]);

	ddr_out32(&ddr->sdram_clk_cntl, regs->clk_cntl);

	for (i = 0; i < DDRC_NUM_CS; i++) {
		if (mod_bnds) {
			ddr_out32(&ddr->bnds[i].a,
				  (regs->cs[i].bnds & 0xfffefffe) >> 1);
		} else {
			ddr_out32(&ddr->bnds[i].a, regs->cs[i].bnds);
		}
		if (mod_bnds && !i) {
			ddr_out32(&ddr->csn_cfg[i],
				  (regs->cs[i].config & ~CTLR_INTLV_MASK));
		} else {
			ddr_out32(&ddr->csn_cfg[i], regs->cs[i].config);
		}
		ddr_out32(&ddr->csn_cfg_2[i], regs->cs[i].config_2);
	}

	ddr_out32(&ddr->timing_cfg_0, regs->timing_cfg[0]);
	ddr_out32(&ddr->timing_cfg_1, regs->timing_cfg[1]);
	ddr_out32(&ddr->timing_cfg_2, regs->timing_cfg[2]);
	ddr_out32(&ddr->timing_cfg_3, regs->timing_cfg[3]);
	ddr_out32(&ddr->timing_cfg_4, regs->timing_cfg[4]);
	ddr_out32(&ddr->timing_cfg_5, regs->timing_cfg[5]);
	ddr_out32(&ddr->timing_cfg_6, regs->timing_cfg[6]);
	ddr_out32(&ddr->timing_cfg_7, regs->timing_cfg[7]);
	ddr_out32(&ddr->timing_cfg_8, regs->timing_cfg[8]);
	ddr_out32(&ddr->timing_cfg_9, regs->timing_cfg[9]);
	ddr_out32(&ddr->zq_cntl, regs->zq_cntl);
	for (i = 0; i < 4; i++)
		ddr_out32(&ddr->dq_map[i], regs->dq_map[i]);
	ddr_out32(&ddr->sdram_cfg_3, regs->sdram_cfg[2]);
	ddr_out32(&ddr->sdram_mode, regs->sdram_mode[0]);
	ddr_out32(&ddr->sdram_mode_2, regs->sdram_mode[1]);
	ddr_out32(&ddr->sdram_mode_3, regs->sdram_mode[2]);
	ddr_out32(&ddr->sdram_mode_4, regs->sdram_mode[3]);
	ddr_out32(&ddr->sdram_mode_5, regs->sdram_mode[4]);
	ddr_out32(&ddr->sdram_mode_6, regs->sdram_mode[5]);
	ddr_out32(&ddr->sdram_mode_7, regs->sdram_mode[6]);
	ddr_out32(&ddr->sdram_mode_8, regs->sdram_mode[7]);
	ddr_out32(&ddr->sdram_mode_9, regs->sdram_mode[8]);
	ddr_out32(&ddr->sdram_mode_10, regs->sdram_mode[9]);
	ddr_out32(&ddr->sdram_mode_11, regs->sdram_mode[10]);
	ddr_out32(&ddr->sdram_mode_12, regs->sdram_mode[11]);
	ddr_out32(&ddr->sdram_mode_13, regs->sdram_mode[12]);
	ddr_out32(&ddr->sdram_mode_14, regs->sdram_mode[13]);
	ddr_out32(&ddr->sdram_mode_15, regs->sdram_mode[14]);
	ddr_out32(&ddr->sdram_mode_16, regs->sdram_mode[15]);
	ddr_out32(&ddr->sdram_md_cntl, regs->md_cntl);
#ifdef NXP_ERRATUM_A009663
	ddr_out32(&ddr->sdram_interval,
		  regs->interval & ~SDRAM_INTERVAL_BSTOPRE);
#else
	ddr_out32(&ddr->sdram_interval, regs->interval);
#endif
	ddr_out32(&ddr->sdram_data_init, regs->data_init);
	if (regs->eor)
		ddr_out32(&ddr->eor, regs->eor);

	ddr_out32(&ddr->wrlvl_cntl, regs->wrlvl_cntl[0]);
#ifndef NXP_DDR_EMU
	/*
	 * Skip these two registers if running on emulator
	 * because emulator doesn't have skew between bytes.
	 */

	if (regs->wrlvl_cntl[1])
		ddr_out32(&ddr->ddr_wrlvl_cntl_2, regs->wrlvl_cntl[1]);
	if (regs->wrlvl_cntl[2])
		ddr_out32(&ddr->ddr_wrlvl_cntl_3, regs->wrlvl_cntl[2]);
#endif

	ddr_out32(&ddr->ddr_sr_cntr, regs->ddr_sr_cntr);
	ddr_out32(&ddr->ddr_sdram_rcw_1, regs->sdram_rcw[0]);
	ddr_out32(&ddr->ddr_sdram_rcw_2, regs->sdram_rcw[1]);
	ddr_out32(&ddr->ddr_sdram_rcw_3, regs->sdram_rcw[2]);
	ddr_out32(&ddr->ddr_sdram_rcw_4, regs->sdram_rcw[3]);
	ddr_out32(&ddr->ddr_sdram_rcw_5, regs->sdram_rcw[4]);
	ddr_out32(&ddr->ddr_sdram_rcw_6, regs->sdram_rcw[5]);
	ddr_out32(&ddr->ddr_cdr2, regs->cdr[1]);
	ddr_out32(&ddr->sdram_cfg_2, regs->sdram_cfg[1]);
	ddr_out32(&ddr->init_addr, regs->init_addr);
	ddr_out32(&ddr->init_ext_addr, regs->init_ext_addr);

#ifdef NXP_ERRATUM_A009803
	/* part 1 of 2 */
	if (regs->sdram_cfg[1] & SDRAM_CFG2_AP_EN) {
		if (regs->sdram_cfg[0] & SDRAM_CFG_RD_EN) {
			ddr_out32(&ddr->ddr_sdram_rcw_2,
				  regs->sdram_rcw[1] & ~0xf0);
		}

		ddr_out32(&ddr->err_disable,
				regs->err_disable | DDR_ERR_DISABLE_APED);
	}
#else
	ddr_out32(&ddr->err_disable, regs->err_disable);
#endif
	ddr_out32(&ddr->err_int_en, regs->err_int_en);

	/* For DDRC 5.05 only */
	if (get_ddrc_version(ddr) == 0x50500) {
		ddr_out32(&ddr->tx_cfg[1], 0x1f1f1f1f);
		ddr_out32(&ddr->debug[3], 0x124a02c0);
	}

	for (i = 0; i < 4; i++) {
		if (regs->tx_cfg[i])
			ddr_out32(&ddr->tx_cfg[i], regs->tx_cfg[i]);
	}
	for (i = 0; i < 64; i++) {
		if (regs->debug[i]) {
#ifdef NXP_ERRATUM_A009942
			if (i == 28)
				continue;
#endif
			ddr_out32(&ddr->debug[i], regs->debug[i]);
		}
	}
#ifdef CONFIG_DDR_ADDR_DEC
	if (regs->dec[9] & 1) {
		for (i = 0; i < 10; i++)
			ddr_out32(&ddr->dec[i], regs->dec[i]);
		if (mod_bnds) {
			debug("Disable address decoding\n");
			ddr_out32(&ddr->dec[9], 0);
		}
	}
#endif

#ifdef NXP_ERRATUM_A008511
	/* Part 1 of 2 */
	/* This erraum only applies to verion 5.2.1 */
	if (get_ddrc_version(ddr) == 0x50200) {
		ERROR("Unsupported SoC.\n");
	} else if (get_ddrc_version(ddr) == 0x50201) {
		ddr_out32(&ddr->debug[37], 1 << 31);
		ddr_out32(&ddr->ddr_cdr2,
			  regs->cdr[1] | DDR_CDR2_VREF_TRAIN_EN);
	} else {
		debug("Erratum A008511 doesn't apply.\n");
	}
#endif

#ifdef NXP_ERRATUM_A009942
	ddr_freq = clk / 1000000;
	tmp = ddr_in32(&ddr->debug[28]);
	tmp &= 0xff0fff00;
	tmp |= ddr_freq <= 1333 ? 0x0080006a :
		(ddr_freq <= 1600 ? 0x0070006f :
		 (ddr_freq <= 1867 ? 0x00700076 : 0x0060007b));
	if (regs->debug[28]) {
		tmp &= ~0xff;
		tmp |= regs->debug[28] & 0xff;
	} else {
		WARN("Warning: Optimal CPO value not set.\n");
	}
	ddr_out32(&ddr->debug[28], tmp);
#endif

#ifdef NXP_ERRATUM_A010165
	ddr_freq = clk / 1000000;
	if ((ddr_freq > 1900) && (ddr_freq < 2300)) {
		tmp = ddr_in32(&ddr->debug[28]);
		ddr_out32(&ddr->debug[28], tmp | 0x000a0000);
	}
#endif
	/*
	 * For RDIMMs, JEDEC spec requires clocks to be stable before reset is
	 * deasserted. Clocks start when any chip select is enabled and clock
	 * control register is set. Because all DDR components are connected to
	 * one reset signal, this needs to be done in two steps. Step 1 is to
	 * get the clocks started. Step 2 resumes after reset signal is
	 * deasserted.
	 */
	if (twopass == 1) {
		udelay(200);
		return 0;
	}

after_reset:
	/* Set, but do not enable the memory */
	temp_sdram_cfg = regs->sdram_cfg[0];
	temp_sdram_cfg &= ~(SDRAM_CFG_MEM_EN);
	ddr_out32(&ddr->sdram_cfg, temp_sdram_cfg);

	if (get_ddrc_version(ddr) < 0x50500) {
		/*
		 * 500 painful micro-seconds must elapse between
		 * the DDR clock setup and the DDR config enable.
		 * DDR2 need 200 us, and DDR3 need 500 us from spec,
		 * we choose the max, that is 500 us for all of case.
		 */
		udelay(500);
		mb();
		isb();
	} else {
		/* wait for PHY complete */
		timeout = 40;
		while (!(ddr_in32(&ddr->ddr_dsr2) & 0x4) &&
		       (timeout > 0)) {
			udelay(500);
			timeout--;
		}
		if (timeout <= 0) {
			printf("PHY handshake timeout, ddr_dsr2 = %x\n",
			       ddr_in32(&ddr->ddr_dsr2));
		} else {
			debug("PHY handshake completed, timer remains %d\n",
			      timeout);
		}
	}

	temp_sdram_cfg = ddr_in32(&ddr->sdram_cfg) & ~SDRAM_CFG_BI;
	/* Let the controller go */
	ddr_out32(&ddr->sdram_cfg, temp_sdram_cfg | SDRAM_CFG_MEM_EN);
	mb();
	isb();

	total_mem_per_ctrl = 0;
	for (i = 0; i < DDRC_NUM_CS; i++) {
		if (!(regs->cs[i].config & 0x80000000))
			continue;
		total_mem_per_ctrl += 1 << (
			((regs->cs[i].config >> 14) & 0x3) + 2 +
			((regs->cs[i].config >> 8) & 0x7) + 12 +
			((regs->cs[i].config >> 4) & 0x3) + 0 +
			((regs->cs[i].config >> 0) & 0x7) + 8 +
			((regs->sdram_cfg[2] >> 4) & 0x3) +
			3 - ((regs->sdram_cfg[0] >> 19) & 0x3) -
			26);		/* minus 26 (count of 64M) */
	}
	total_mem_per_ctrl_adj = total_mem_per_ctrl;
	/*
	 * total memory / bus width = transactions needed
	 * transactions needed / data rate = seconds
	 * to add plenty of buffer, double the time
	 * For example, 2GB on 666MT/s 64-bit bus takes about 402ms
	 * Let's wait for 800ms
	 */
	bus_width = 3 - ((ddr_in32(&ddr->sdram_cfg) & SDRAM_CFG_DBW_MASK)
			>> SDRAM_CFG_DBW_SHIFT);
	timeout = ((total_mem_per_ctrl_adj << (6 - bus_width)) * 100 /
		   (clk >> 20)) << 2;
	total_mem_per_ctrl_adj >>= 4;	/* shift down to gb size */
	if (ddr_in32(&ddr->sdram_cfg_2) & SDRAM_CFG2_D_INIT) {
		debug("total size %d GB\n", total_mem_per_ctrl_adj);
		debug("Need to wait up to %d ms\n", timeout * 10);

		do {
			mdelay(10);
		} while (timeout-- > 0 &&
			 (ddr_in32(&ddr->sdram_cfg_2) & SDRAM_CFG2_D_INIT));

		if (timeout <= 0) {
			if (ddr_in32(&ddr->debug[1]) & 0x3d00) {
				ERROR("Found training error(s): 0x%x\n",
				      ddr_in32(&ddr->debug[1]));
			}
			ERROR("Error: Waiting for D_INIT timeout.\n");
			return -EIO;
		}
	}

	if (mod_bnds) {
		debug("Restore original bnds\n");
		for (i = 0; i < DDRC_NUM_CS; i++)
			ddr_out32(&ddr->bnds[i].a, regs->cs[i].bnds);
		ddr_out32(&ddr->csn_cfg[0], regs->cs[0].config);
#ifdef CONFIG_DDR_ADDR_DEC
		if (regs->dec[9] & 0x1) {
			debug("Restore address decoding\n");
			ddr_out32(&ddr->dec[9], regs->dec[9]);
		}
#endif
	}

#ifdef NXP_ERRATUM_A009803
	/* Part 2 of 2 */
	if (regs->sdram_cfg[1] & SDRAM_CFG2_AP_EN) {
		timeout = 400;
		do {
			mdelay(1);
		} while (timeout-- > 0 && !(ddr_in32(&ddr->debug[1]) & 0x2));

		if (regs->sdram_cfg[0] & SDRAM_CFG_RD_EN) {
			for (i = 0; i < DDRC_NUM_CS; i++) {
				if (!(regs->cs[i].config & SDRAM_CS_CONFIG_EN))
					continue;
				set_wait_for_bits_clear(&ddr->sdram_md_cntl,
						MD_CNTL_MD_EN |
						MD_CNTL_CS_SEL(i) |
						0x070000ed,
						MD_CNTL_MD_EN);
				udelay(1);
			}
		}

		ddr_out32(&ddr->err_disable,
			  regs->err_disable & ~DDR_ERR_DISABLE_APED);
	}
#endif

#ifdef NXP_ERRATUM_A009663
	ddr_out32(&ddr->sdram_interval, regs->interval);
#endif

#ifdef NXP_ERRATUM_A009942
	timeout = 400;
	do {
		mdelay(1);
	} while (timeout-- > 0 && !(ddr_in32(&ddr->debug[1]) & 0x2));
	tmp = (regs->sdram_cfg[0] >> 19) & 0x3;
	check = (tmp == DDR_DBUS_64) ? 4 : ((tmp == DDR_DBUS_32) ? 2 : 1);
	for (i = 0; i < check; i++) {
		tmp = ddr_in32(&ddr->debug[9 + i]);
		debug("Reading debug[%d] as 0x%x\n", i + 9, tmp);
		cpo_min = min(cpo_min,
			      min((tmp >> 24) & 0xff, (tmp >> 8) & 0xff));
		cpo_max = max(cpo_max,
			      max((tmp >> 24) & 0xff, (tmp >> 8) & 0xff));
	}
	if (regs->sdram_cfg[0] & SDRAM_CFG_ECC_EN) {
		tmp = ddr_in32(&ddr->debug[13]);
		cpo_min = min(cpo_min, (tmp >> 24) & 0xff);
		cpo_max = max(cpo_max, (tmp >> 24) & 0xff);
	}
	debug("cpo_min 0x%x\n", cpo_min);
	debug("cpo_max 0x%x\n", cpo_max);
	tmp = ddr_in32(&ddr->debug[28]);
	debug("debug[28] 0x%x\n", tmp);
	if ((cpo_min + 0x3B) < (tmp & 0xff)) {
		WARN("Warning: A009942 requires setting cpo_sample to 0x%x\n",
		     (cpo_min + cpo_max) / 2 + 0x27);
	} else {
		debug("Optimal cpo_sample 0x%x\n",
			(cpo_min + cpo_max) / 2 + 0x27);
	}
#endif
	if (run_bist()) {
		if (ddr_in32(&ddr->debug[1]) &
		    ((get_ddrc_version(ddr) == 0x50500) ? 0x3c00 : 0x3d00)) {
			ERROR("Found training error(s): 0x%x\n",
			     ddr_in32(&ddr->debug[1]));
			return -EIO;
		}
		INFO("Running built-in self test ...\n");
		/* give it 10x time to cover whole memory */
		timeout = ((total_mem_per_ctrl << (6 - bus_width)) *
			   100 / (clk >> 20)) * 10;
		INFO("\tWait up to %d ms\n", timeout * 10);
		ret = bist(ddr, timeout);
	}
	dump_ddrc((void *)ddr);

	return ret;
}

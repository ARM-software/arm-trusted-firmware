/*
 * Copyright 2018-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <dram.h>

static void lpddr4_mr_write(uint32_t mr_rank, uint32_t mr_addr, uint32_t mr_data)
{
	/*
	 * 1. Poll MRSTAT.mr_wr_busy until it is 0. This checks that there
	 * is no outstanding MR transaction. No
	 * writes should be performed to MRCTRL0 and MRCTRL1 if MRSTAT.mr_wr_busy = 1.
	 */
	while (mmio_read_32(DDRC_MRSTAT(0)) & 0x1)
		;

	/*
	 * 2. Write the MRCTRL0.mr_type, MRCTRL0.mr_addr,
	 * MRCTRL0.mr_rank and (for MRWs)
	 * MRCTRL1.mr_data to define the MR transaction.
	 */
	mmio_write_32(DDRC_MRCTRL0(0), (mr_rank << 4));
	mmio_write_32(DDRC_MRCTRL1(0), (mr_addr << 8) | mr_data);
	mmio_setbits_32(DDRC_MRCTRL0(0), BIT(31));
}

void lpddr4_swffc(struct dram_info *info, unsigned int init_fsp,
	 unsigned int fsp_index)

{
	uint32_t mr, emr, emr2, emr3;
	uint32_t mr11, mr12, mr22, mr14;
	uint32_t val;
	uint32_t derate_backup[3];
	uint32_t (*mr_data)[8];

	/* 1. program targetd UMCTL2_REGS_FREQ1/2/3,already done, skip it. */

	/* 2. MR13.FSP-WR=1, MRW to update MR registers */
	mr_data = info->mr_table;
	mr = mr_data[fsp_index][0];
	emr  = mr_data[fsp_index][1];
	emr2 = mr_data[fsp_index][2];
	emr3 = mr_data[fsp_index][3];
	mr11 = mr_data[fsp_index][4];
	mr12 = mr_data[fsp_index][5];
	mr22 = mr_data[fsp_index][6];
	mr14 = mr_data[fsp_index][7];

	val = (init_fsp == 1) ? 0x2 << 6 : 0x1 << 6;
	emr3 = (emr3 & 0x003f) | val | 0x0d00;

	/* 12. set PWRCTL.selfref_en=0 */
	mmio_clrbits_32(DDRC_PWRCTL(0), 0xf);

	/* It is more safe to config it here */
	mmio_clrbits_32(DDRC_DFIPHYMSTR(0), 0x1);

	lpddr4_mr_write(3, 13, emr3);
	lpddr4_mr_write(3, 1, mr);
	lpddr4_mr_write(3, 2, emr);
	lpddr4_mr_write(3, 3, emr2);
	lpddr4_mr_write(3, 11, mr11);
	lpddr4_mr_write(3, 12, mr12);
	lpddr4_mr_write(3, 14, mr14);
	lpddr4_mr_write(3, 22, mr22);

	do {
		val = mmio_read_32(DDRC_MRSTAT(0));
	} while (val & 0x1);

	/* 3. disable AXI ports */
	mmio_write_32(DDRC_PCTRL_0(0), 0x0);

	/* 4.Poll PSTAT.rd_port_busy_n=0 and PSTAT.wr_port_busy_n=0. */
	do {
		val = mmio_read_32(DDRC_PSTAT(0));
	} while (val != 0);

	/* 6.disable SBRCTL.scrub_en, skip if never enable it */
	/* 7.poll SBRSTAT.scrub_busy  Q2: should skip phy master if never enable it */
	/* Disable phy master */
#ifdef DFILP_SPT
	/* 8. disable DFI LP */
	/* DFILPCFG0.dfi_lp_en_sr */
	val = mmio_read_32(DDRC_DFILPCFG0(0));
	if (val & 0x100) {
		mmio_write_32(DDRC_DFILPCFG0(0), 0x0);
		do {
			val = mmio_read_32(DDRC_DFISTAT(0)); // dfi_lp_ack
			val2 = mmio_read_32(DDRC_STAT(0)); // operating_mode
		} while (((val & 0x2) == 0x2) && ((val2 & 0x7) == 3));
	}
#endif
	/* 9. wait until in normal or power down states */
	do {
		/* operating_mode */
		val = mmio_read_32(DDRC_STAT(0));
	} while (((val & 0x7) != 1) && ((val & 0x7) != 2));

	/* 10. Disable automatic derating: derate_enable */
	val = mmio_read_32(DDRC_DERATEEN(0));
	derate_backup[0] = val;
	mmio_clrbits_32(DDRC_DERATEEN(0), 0x1);

	val = mmio_read_32(DDRC_FREQ1_DERATEEN(0));
	derate_backup[1] = val;
	mmio_clrbits_32(DDRC_FREQ1_DERATEEN(0), 0x1);

	val = mmio_read_32(DDRC_FREQ2_DERATEEN(0));
	derate_backup[2] = val;
	mmio_clrbits_32(DDRC_FREQ2_DERATEEN(0), 0x1);

	/* 11. disable automatic ZQ calibration */
	mmio_setbits_32(DDRC_ZQCTL0(0), BIT(31));
	mmio_setbits_32(DDRC_FREQ1_ZQCTL0(0), BIT(31));
	mmio_setbits_32(DDRC_FREQ2_ZQCTL0(0), BIT(31));

	/* 12. set PWRCTL.selfref_en=0 */
	mmio_clrbits_32(DDRC_PWRCTL(0), 0x1);

	/* 13.Poll STAT.operating_mode is in "Normal" (001) or "Power-down" (010) */
	do {
		val = mmio_read_32(DDRC_STAT(0));
	} while (((val & 0x7) != 1) && ((val & 0x7) != 2));

	/* 14-15. trigger SW SR */
	/* bit 5: selfref_sw, bit 6: stay_in_selfref */
	mmio_setbits_32(DDRC_PWRCTL(0), 0x60);

	/* 16. Poll STAT.selfref_state in "Self Refresh 1" */
	do {
		val = mmio_read_32(DDRC_STAT(0));
	} while ((val & 0x300) != 0x100);

	/* 17. disable dq */
	mmio_setbits_32(DDRC_DBG1(0), 0x1);

	/* 18. Poll DBGCAM.wr_data_pipeline_empty and DBGCAM.rd_data_pipeline_empty */
	do {
		val = mmio_read_32(DDRC_DBGCAM(0));
		val &= 0x30000000;
	} while (val != 0x30000000);

	/* 19. change MR13.FSP-OP to new FSP and MR13.VRCG to high current */
	emr3 = (((~init_fsp) & 0x1) << 7) | (0x1 << 3) | (emr3 & 0x0077) | 0x0d00;
	lpddr4_mr_write(3, 13, emr3);

	/* 20. enter SR Power Down */
	mmio_clrsetbits_32(DDRC_PWRCTL(0), 0x60, 0x20);

	/* 21. Poll STAT.selfref_state is in "SR Power down" */
	do {
		val = mmio_read_32(DDRC_STAT(0));
	} while ((val & 0x300) != 0x200);

	/* 22. set dfi_init_complete_en = 0 */

	/* 23. switch clock */
	/* set SWCTL.dw_done to 0 */
	mmio_write_32(DDRC_SWCTL(0), 0x0000);

	/* 24. program frequency mode=1(bit 29), target_frequency=target_freq (bit 29) */
	mmio_write_32(DDRC_MSTR2(0), fsp_index);

	/* 25. DBICTL for FSP-OP[1], skip it if never enable it */

	/* 26.trigger initialization in the PHY */

	/* Q3: if refresh level is updated, then should program */
	/* as updating refresh, need to toggle refresh_update_level signal */
	val = mmio_read_32(DDRC_RFSHCTL3(0));
	val = val ^ 0x2;
	mmio_write_32(DDRC_RFSHCTL3(0), val);

	/* Q4: only for legacy PHY, so here can skipped */

	/* dfi_frequency -> 0x1x */
	val = mmio_read_32(DDRC_DFIMISC(0));
	val &= 0xFE;
	val |= (fsp_index << 8);
	mmio_write_32(DDRC_DFIMISC(0), val);
	/* dfi_init_start */
	val |= 0x20;
	mmio_write_32(DDRC_DFIMISC(0), val);

	/* polling dfi_init_complete de-assert */
	do {
		val = mmio_read_32(DDRC_DFISTAT(0));
	} while ((val & 0x1) == 0x1);

	/* change the clock frequency */
	dram_clock_switch(info->timing_info->fsp_table[fsp_index], info->bypass_mode);

	/* dfi_init_start de-assert */
	mmio_clrbits_32(DDRC_DFIMISC(0), 0x20);

	/* polling dfi_init_complete re-assert */
	do {
		val = mmio_read_32(DDRC_DFISTAT(0));
	} while ((val & 0x1) == 0x0);

	/* 27. set ZQCTL0.dis_srx_zqcl = 1 */
	if (fsp_index == 0) {
		mmio_setbits_32(DDRC_ZQCTL0(0), BIT(30));
	} else  if (fsp_index == 1) {
		mmio_setbits_32(DDRC_FREQ1_ZQCTL0(0), BIT(30));
	} else {
		mmio_setbits_32(DDRC_FREQ2_ZQCTL0(0), BIT(30));
	}

	/* 28,29. exit "self refresh power down" to stay "self refresh 2" */
	/* exit SR power down */
	mmio_clrsetbits_32(DDRC_PWRCTL(0), 0x60, 0x40);
	/* 30. Poll STAT.selfref_state in "Self refresh 2" */
	do {
		val = mmio_read_32(DDRC_STAT(0));
	} while ((val & 0x300) != 0x300);

	/* 31. change MR13.VRCG to normal */
	emr3 = (emr3 & 0x00f7) | 0x0d00;
	lpddr4_mr_write(3, 13, emr3);

	/* enable PHY master */
	mmio_write_32(DDRC_DFIPHYMSTR(0), 0x1);

	/* 32. issue ZQ if required: zq_calib_short, bit 4 */
	/* polling zq_calib_short_busy */
	mmio_setbits_32(DDRC_DBGCMD(0), 0x10);

	do {
		val = mmio_read_32(DDRC_DBGSTAT(0));
	} while ((val & 0x10) != 0x0);

	/* 33. Reset ZQCTL0.dis_srx_zqcl=0 */
	if (fsp_index == 1)
		mmio_clrbits_32(DDRC_FREQ1_ZQCTL0(0), BIT(30));
	else if (fsp_index == 2)
		mmio_clrbits_32(DDRC_FREQ2_ZQCTL0(0), BIT(30));
	else
		mmio_clrbits_32(DDRC_ZQCTL0(0), BIT(30));

	/* set SWCTL.dw_done to 1 and poll SWSTAT.sw_done_ack=1 */
	mmio_write_32(DDRC_SWCTL(0), 0x1);

	/* wait SWSTAT.sw_done_ack to 1 */
	do {
		val = mmio_read_32(DDRC_SWSTAT(0));
	} while ((val & 0x1) == 0x0);

	/* 34. set PWRCTL.stay_in_selfreh=0, exit SR */
	mmio_clrbits_32(DDRC_PWRCTL(0), 0x40);
	/* wait tXSR */

	/* 35. Poll STAT.selfref_state in "Idle" */
	do {
		val = mmio_read_32(DDRC_STAT(0));
	} while ((val & 0x300) != 0x0);

#ifdef DFILP_SPT
	/* 36. restore dfi_lp.dfi_lp_en_sr */
	mmio_setbits_32(DDRC_DFILPCFG0(0), BIT(8));
#endif

	/* 37. re-enable CAM: dis_dq */
	mmio_clrbits_32(DDRC_DBG1(0), 0x1);

	/* 38. re-enable automatic SR: selfref_en */
	mmio_setbits_32(DDRC_PWRCTL(0), 0x1);

	/* 39. re-enable automatic ZQ: dis_auto_zq=0 */
	/* disable automatic ZQ calibration */
	if (fsp_index == 1)
		mmio_clrbits_32(DDRC_FREQ1_ZQCTL0(0), BIT(31));
	else if (fsp_index == 2)
		mmio_clrbits_32(DDRC_FREQ2_ZQCTL0(0), BIT(31));
	else
		mmio_clrbits_32(DDRC_ZQCTL0(0), BIT(31));
	/* 40. re-emable automatic derating: derate_enable */
	mmio_write_32(DDRC_DERATEEN(0), derate_backup[0]);
	mmio_write_32(DDRC_FREQ1_DERATEEN(0), derate_backup[1]);
	mmio_write_32(DDRC_FREQ2_DERATEEN(0), derate_backup[2]);

	/* 41. write 1 to PCTRL.port_en */
	mmio_write_32(DDRC_PCTRL_0(0), 0x1);

	/* 42. enable SBRCTL.scrub_en, skip if never enable it */
}

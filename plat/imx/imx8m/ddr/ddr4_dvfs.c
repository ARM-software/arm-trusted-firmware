/*
 * Copyright 2018-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <dram.h>

void ddr4_mr_write(uint32_t mr, uint32_t data, uint32_t mr_type, uint32_t rank)
{
	uint32_t val, mr_mirror, data_mirror;

	/*
	 * 1. Poll MRSTAT.mr_wr_busy until it is 0 to make sure
	 * that there is no outstanding MR transAction.
	 */
	while (mmio_read_32(DDRC_MRSTAT(0)) & 0x1) {
		;
	}

	/*
	 * 2. Write the MRCTRL0.mr_type, MRCTRL0.mr_addr, MRCTRL0.mr_rank
	 * and (for MRWs) MRCTRL1.mr_data to define the MR transaction.
	 */
	val = mmio_read_32(DDRC_DIMMCTL(0));
	if ((val & 0x2) && (rank == 0x2)) {
		mr_mirror = (mr & 0x4) | ((mr & 0x1) << 1) | ((mr & 0x2) >> 1); /* BA0, BA1 swap */
		data_mirror = (data & 0x1607) | ((data & 0x8) << 1) | ((data & 0x10) >> 1) |
				((data & 0x20) << 1) | ((data & 0x40) >> 1) | ((data & 0x80) << 1) |
				 ((data & 0x100) >> 1) | ((data & 0x800) << 2) | ((data & 0x2000) >> 2) ;
	} else {
		mr_mirror = mr;
		data_mirror = data;
	}

	mmio_write_32(DDRC_MRCTRL0(0), mr_type | (mr_mirror << 12) | (rank << 4));
	mmio_write_32(DDRC_MRCTRL1(0), data_mirror);

	/*
	 * 3. In a separate APB transaction, write the MRCTRL0.mr_wr to 1.
	 * This bit is self-clearing, and triggers the MR transaction.
	 * The uMCTL2 then asserts the MRSTAT.mr_wr_busy while it performs
	 * the MR transaction to SDRAM, and no further accesses can be
	 * initiated until it is deasserted.
	 */
	mmio_setbits_32(DDRC_MRCTRL0(0), BIT(31));

	while (mmio_read_32(DDRC_MRSTAT(0))) {
		;
	}
}

void dram_cfg_all_mr(struct dram_info *info, uint32_t pstate)
{
	uint32_t num_rank = info->num_rank;
	/*
	 * 15. Perform MRS commands as required to re-program
	 * timing registers in the SDRAM for the new frequency
	 * (in particular, CL, CWL and WR may need to be changed).
	 */

	for (int i = 1; i <= num_rank; i++) {
		for (int j = 0; j < 6; j++) {
			ddr4_mr_write(j, info->mr_table[pstate][j], 0, i);
		}
		ddr4_mr_write(6, info->mr_table[pstate][7], 0, i);
	}
}

void sw_pstate(uint32_t pstate, uint32_t drate)
{
	uint32_t val;

	mmio_write_32(DDRC_SWCTL(0), 0x0);

	/*
	 * Update any registers which may be required to
	 * change for the new frequency.
	 */
	mmio_write_32(DDRC_MSTR2(0), pstate);
	mmio_setbits_32(DDRC_MSTR(0), (0x1 << 29));

	/*
	 * Toggle RFSHCTL3.refresh_update_level to allow the
	 * new refresh-related register values to propagate
	 * to the refresh logic.
	 */
	val = mmio_read_32(DDRC_RFSHCTL3(0));
	if (val & 0x2) {
		mmio_write_32(DDRC_RFSHCTL3(0), val & 0xFFFFFFFD);
	} else {
		mmio_write_32(DDRC_RFSHCTL3(0), val | 0x2);
	}

	/*
	 * 19. If required, trigger the initialization in the PHY.
	 * If using the gen2 multiPHY, PLL initialization should
	 * be triggered at this point. See the PHY databook for
	 * details about the frequency change procedure.
	 */
	mmio_write_32(DDRC_DFIMISC(0), 0x00000000 | (pstate << 8));
	mmio_write_32(DDRC_DFIMISC(0), 0x00000020 | (pstate << 8));

	/* wait DFISTAT.dfi_init_complete to 0 */
	while (mmio_read_32(DDRC_DFISTAT(0)) & 0x1) {
		;
	}

	/* change the clock to the target frequency */
	dram_clock_switch(drate, false);

	mmio_write_32(DDRC_DFIMISC(0), 0x00000000 | (pstate << 8));

	/* wait DFISTAT.dfi_init_complete to 1 */
	while (!(mmio_read_32(DDRC_DFISTAT(0)) & 0x1)) {
		;
	}

	/*
	 * When changing frequencies the controller may violate the JEDEC
	 * requirement that no more than 16 refreshes should be issued within
	 * 2*tREFI. These extra refreshes are not expected to cause a problem
	 * in the SDRAM. This issue can be avoided by waiting for at least 2*tREFI
	 * before exiting self-refresh in step 19.
	 */
	udelay(14);

	/* 14. Exit the self-refresh state by setting PWRCTL.selfref_sw = 0. */
	mmio_clrbits_32(DDRC_PWRCTL(0), (1 << 5));

	while ((mmio_read_32(DDRC_STAT(0)) & 0x3f) == 0x23) {
		;
	}
}

void ddr4_swffc(struct dram_info *info, unsigned int pstate)
{
	uint32_t drate = info->timing_info->fsp_table[pstate];

	/*
	 * 1. set SWCTL.sw_done to disable quasi-dynamic register
	 * programming outside reset.
	 */
	mmio_write_32(DDRC_SWCTL(0), 0x0);

	/*
	 * 2. Write 0 to PCTRL_n.port_en. This blocks AXI port(s)
	 * from taking any transaction (blocks traffic on AXI ports).
	 */
	mmio_write_32(DDRC_PCTRL_0(0), 0x0);

	/*
	 * 3. Poll PSTAT.rd_port_busy_n=0 and PSTAT.wr_port_busy_n=0.
	 * Wait until all AXI ports are idle (the uMCTL2 core has to
	 * be idle).
	 */
	while (mmio_read_32(DDRC_PSTAT(0)) & 0x10001) {
		;
	}

	/*
	 * 4. Write 0 to SBRCTL.scrub_en. Disable SBR, required only if
	 * SBR instantiated.
	 * 5. Poll SBRSTAT.scrub_busy=0.
	 * 6. Set DERATEEN.derate_enable = 0, if DERATEEN.derate_eanble = 1
	 * and the read latency (RL) value needs to change after the frequency
	 * change (LPDDR2/3/4 only).
	 * 7. Set DBG1.dis_hif=1 so that no new commands will be accepted by the uMCTL2.
	 */
	mmio_setbits_32(DDRC_DBG1(0), (0x1 << 1));

	/*
	 * 8. Poll DBGCAM.dbg_wr_q_empty and DBGCAM.dbg_rd_q_empty to ensure
	 * that write and read data buffers are empty.
	 */
	while ((mmio_read_32(DDRC_DBGCAM(0)) & 0x06000000) != 0x06000000) {
		;
	}

	/*
	 * 9. For DDR4, update MR6 with the new tDLLK value via the Mode
	 * Register Write signals
	 * 10. Set DFILPCFG0.dfi_lp_en_sr = 0, if DFILPCFG0.dfi_lp_en_sr = 1,
	 * and wait until DFISTAT.dfi_lp_ack
	 * 11. If DFI PHY Master interface is active in uMCTL2, then disable it
	 * 12. Wait until STAT.operating_mode[1:0]!=11 indicating that the
	 * controller is not in self-refresh mode.
	 */
	while ((mmio_read_32(DDRC_STAT(0)) & 0x3) == 0x3) {
		;
	}

	/*
	 * 13. Assert PWRCTL.selfref_sw for the DWC_ddr_umctl2 core to enter
	 * the self-refresh mode.
	 */
	mmio_setbits_32(DDRC_PWRCTL(0), (1 << 5));

	/*
	 * 14. Wait until STAT.operating_mode[1:0]==11 indicating that the
	 * controller core is in self-refresh mode.
	 */
	while ((mmio_read_32(DDRC_STAT(0)) & 0x3f) != 0x23) {
		;
	}

	sw_pstate(pstate, drate);
	dram_cfg_all_mr(info, pstate);

	/* 23. Enable HIF commands by setting DBG1.dis_hif=0. */
	mmio_clrbits_32(DDRC_DBG1(0), (0x1 << 1));

	/*
	 * 24. Reset DERATEEN.derate_enable = 1 if DERATEEN.derate_enable
	 * has been set to 0 in step 6.
	 * 25. If DFI PHY Master interface was active before step 11 then
	 * enable it back by programming DFIPHYMSTR.phymstr_en = 1'b1.
	 * 26. Write 1 to PCTRL_n.port_en. AXI port(s) are no longer blocked
	 * from taking transactions (Re-enable traffic on AXI ports)
	 */
	mmio_write_32(DDRC_PCTRL_0(0), 0x1);

	/*
	 * 27. Write 1 to SBRCTL.scrub_en. Enable SBR if desired, only
	 * required if SBR instantiated.
	 */

	/*
	 * set SWCTL.sw_done to enable quasi-dynamic register programming
	 * outside reset.
	 */
	mmio_write_32(DDRC_SWCTL(0), 0x1);

	/* wait SWSTAT.sw_done_ack to 1 */
	while (!(mmio_read_32(DDRC_SWSTAT(0)) & 0x1)) {
		;
	}
}

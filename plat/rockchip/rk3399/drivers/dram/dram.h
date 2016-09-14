/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __SOC_ROCKCHIP_RK3399_SDRAM_H__
#define __SOC_ROCKCHIP_RK3399_SDRAM_H__

struct rk3399_ddr_cic_regs {
	uint32_t cic_ctrl0;
	uint32_t cic_ctrl1;
	uint32_t cic_idle_th;
	uint32_t cic_cg_wait_th;
	uint32_t cic_status0;
	uint32_t cic_status1;
	uint32_t cic_ctrl2;
	uint32_t cic_ctrl3;
	uint32_t cic_ctrl4;
};

/* DENALI_CTL_00 */
#define START		(1)

/* DENALI_CTL_68 */
#define PWRUP_SREFRESH_EXIT	(1 << 16)

/* DENALI_CTL_274 */
#define MEM_RST_VALID	(1)

struct rk3399_ddr_pctl_regs {
	uint32_t denali_ctl[332];
};

struct rk3399_ddr_publ_regs {
	uint32_t denali_phy[959];
};

#define PHY_DRV_ODT_Hi_Z	(0x0)
#define PHY_DRV_ODT_240		(0x1)
#define PHY_DRV_ODT_120		(0x8)
#define PHY_DRV_ODT_80		(0x9)
#define PHY_DRV_ODT_60		(0xc)
#define PHY_DRV_ODT_48		(0xd)
#define PHY_DRV_ODT_40		(0xe)
#define PHY_DRV_ODT_34_3	(0xf)

struct rk3399_ddr_pi_regs {
	uint32_t denali_pi[200];
};
union noc_ddrtiminga0 {
	uint32_t d32;
	struct {
		unsigned acttoact : 6;
		unsigned reserved0 : 2;
		unsigned rdtomiss : 6;
		unsigned reserved1 : 2;
		unsigned wrtomiss : 6;
		unsigned reserved2 : 2;
		unsigned readlatency : 8;
	} b;
};

union noc_ddrtimingb0 {
	uint32_t d32;
	struct {
		unsigned rdtowr : 5;
		unsigned reserved0 : 3;
		unsigned wrtord : 5;
		unsigned reserved1 : 3;
		unsigned rrd : 4;
		unsigned reserved2 : 4;
		unsigned faw : 6;
		unsigned reserved3 : 2;
	} b;
};

union noc_ddrtimingc0 {
	uint32_t d32;
	struct {
		unsigned burstpenalty : 4;
		unsigned reserved0 : 4;
		unsigned wrtomwr : 6;
		unsigned reserved1 : 18;
	} b;
};

union noc_devtodev0 {
	uint32_t d32;
	struct {
		unsigned busrdtord : 3;
		unsigned reserved0 : 1;
		unsigned busrdtowr : 3;
		unsigned reserved1 : 1;
		unsigned buswrtord : 3;
		unsigned reserved2 : 1;
		unsigned buswrtowr : 3;
		unsigned reserved3 : 17;
	} b;
};

union noc_ddrmode {
	uint32_t d32;
	struct {
		unsigned autoprecharge : 1;
		unsigned bypassfiltering : 1;
		unsigned fawbank : 1;
		unsigned burstsize : 2;
		unsigned mwrsize : 2;
		unsigned reserved2 : 1;
		unsigned forceorder : 8;
		unsigned forceorderstate : 8;
		unsigned reserved3 : 8;
	} b;
};

struct rk3399_msch_regs {
	uint32_t coreid;
	uint32_t revisionid;
	uint32_t ddrconf;
	uint32_t ddrsize;
	union noc_ddrtiminga0 ddrtiminga0;
	union noc_ddrtimingb0 ddrtimingb0;
	union noc_ddrtimingc0 ddrtimingc0;
	union noc_devtodev0 devtodev0;
	uint32_t reserved0[(0x110-0x20)/4];
	union noc_ddrmode ddrmode;
	uint32_t reserved1[(0x1000-0x114)/4];
	uint32_t agingx0;
};

struct rk3399_msch_timings {
	union noc_ddrtiminga0 ddrtiminga0;
	union noc_ddrtimingb0 ddrtimingb0;
	union noc_ddrtimingc0 ddrtimingc0;
	union noc_devtodev0 devtodev0;
	union noc_ddrmode ddrmode;
	uint32_t agingx0;
};
#if 1
struct rk3399_sdram_channel {
	unsigned char rank;
	/* col = 0, means this channel is invalid */
	unsigned char col;
	/* 3:8bank, 2:4bank */
	unsigned char bk;
	/* channel buswidth, 2:32bit, 1:16bit, 0:8bit */
	unsigned char bw;
	/* die buswidth, 2:32bit, 1:16bit, 0:8bit */
	unsigned char dbw;
	/* row_3_4 = 1: 6Gb or 12Gb die
	 * row_3_4 = 0: normal die, power of 2
	 */
	unsigned char row_3_4;
	unsigned char cs0_row;
	unsigned char cs1_row;
	uint32_t ddrconfig;
	struct rk3399_msch_timings noc_timings;
};

struct rk3399_sdram_params {
	struct rk3399_sdram_channel ch[2];
	uint32_t ddr_freq;
	unsigned char dramtype;
	unsigned char num_channels;
	unsigned char stride;
	unsigned char odt;
	struct rk3399_ddr_pctl_regs pctl_regs;
	struct rk3399_ddr_pi_regs pi_regs;
	struct rk3399_ddr_publ_regs phy_regs;
};
#endif
struct rk3399_sdram_channel_config {
	uint32_t bus_width;
	uint32_t cs_cnt;
	uint32_t cs0_row;
	uint32_t cs1_row;
	uint32_t bank;
	uint32_t col;
	uint32_t each_die_bus_width;
	uint32_t each_die_6gb_or_12gb;
};

struct rk3399_sdram_config {
	struct rk3399_sdram_channel_config ch[2];
	uint32_t dramtype;
	uint32_t channal_num;
};

struct rk3399_sdram_default_config {
	unsigned char bl;
	/* 1:auto precharge, 0:never auto precharge */
	unsigned char ap;
	/* dram driver strength */
	unsigned char dramds;
	/* dram ODT, if odt=0, this parameter invalid */
	unsigned char dramodt;
	/* ca ODT, if odt=0, this parameter invalid
	 * only used by LPDDR4
	 */
	unsigned char caodt;
	unsigned char burst_ref_cnt;
	/* zqcs period, unit(s) */
	unsigned char zqcsi;
};

struct  ddr_dts_config_timing {
	unsigned int ddr3_speed_bin;
	unsigned int pd_idle;
	unsigned int sr_idle;
	unsigned int sr_mc_gate_idle;
	unsigned int srpd_lite_idle;
	unsigned int standby_idle;
	unsigned int auto_pd_dis_freq;
	unsigned int ddr3_dll_dis_freq;
	unsigned int phy_dll_dis_freq;
	unsigned int ddr3_odt_dis_freq;
	unsigned int ddr3_drv;
	unsigned int ddr3_odt;
	unsigned int phy_ddr3_ca_drv;
	unsigned int phy_ddr3_dq_drv;
	unsigned int phy_ddr3_odt;
	unsigned int lpddr3_odt_dis_freq;
	unsigned int lpddr3_drv;
	unsigned int lpddr3_odt;
	unsigned int phy_lpddr3_ca_drv;
	unsigned int phy_lpddr3_dq_drv;
	unsigned int phy_lpddr3_odt;
	unsigned int lpddr4_odt_dis_freq;
	unsigned int lpddr4_drv;
	unsigned int lpddr4_dq_odt;
	unsigned int lpddr4_ca_odt;
	unsigned int phy_lpddr4_ca_drv;
	unsigned int phy_lpddr4_ck_cs_drv;
	unsigned int phy_lpddr4_dq_drv;
	unsigned int phy_lpddr4_odt;
	uint32_t available;
};

struct drv_odt_lp_config {
	uint32_t ddr3_speed_bin;
	uint32_t pd_idle;
	uint32_t sr_idle;
	uint32_t sr_mc_gate_idle;
	uint32_t srpd_lite_idle;
	uint32_t standby_idle;

	uint32_t ddr3_dll_dis_freq;/* for ddr3 only */
	uint32_t phy_dll_dis_freq;
	uint32_t odt_dis_freq;

	uint32_t dram_side_drv;
	uint32_t dram_side_dq_odt;
	uint32_t dram_side_ca_odt;

	uint32_t phy_side_ca_drv;
	uint32_t phy_side_ck_cs_drv;
	uint32_t phy_side_dq_drv;
	uint32_t phy_side_odt;
};

#define KHz (1000)
#define MHz (1000*KHz)
#define GHz (1000*MHz)

#define PI_CA_TRAINING	(1 << 0)
#define PI_WRITE_LEVELING	(1 << 1)
#define PI_READ_GATE_TRAINING	(1 << 2)
#define PI_READ_LEVELING	(1 << 3)
#define PI_WDQ_LEVELING	(1 << 4)
#define PI_FULL_TARINING	(0xff)

#define READ_CH_CNT(val)			(1+((val>>12)&0x1))
#define READ_CH_INFO(val)			((val>>28)&0x3)
/* row_3_4:0=normal, 1=6Gb or 12Gb */
#define READ_CH_ROW_INFO(val, ch)	((val>>(30+(ch)))&0x1)

#define READ_DRAMTYPE_INFO(val)		((val>>13)&0x7)
#define READ_CS_INFO(val, ch)		((((val)>>(11+(ch)*16))&0x1)+1)
#define READ_BW_INFO(val, ch)		(2>>(((val)>>(2+(ch)*16))&0x3))
#define READ_COL_INFO(val, ch)		(9+(((val)>>(9+(ch)*16))&0x3))
#define READ_BK_INFO(val, ch)		(3-(((val)>>(8+(ch)*16))&0x1))
#define READ_CS0_ROW_INFO(val, ch)	(13+(((val)>>(6+(ch)*16))&0x3))
#define READ_CS1_ROW_INFO(val, ch)	(13+(((val)>>(4+(ch)*16))&0x3))
#define READ_DIE_BW_INFO(val, ch)	(2>>((val>>((ch)*16))&0x3))

#define __sramdata __attribute__((section(".sram.data")))
#define __sramconst __attribute__((section(".sram.rodata")))
#define __sramlocalfunc __attribute__((section(".sram.text")))
#define __sramfunc __attribute__((section(".sram.text"))) \
					__attribute__((noinline))


#define DDR_SAVE_SP(save_sp)   (save_sp = ddr_save_sp(((uint32_t)\
				(SRAM_CODE_BASE + 0x2000) & (~7))))

#define DDR_RESTORE_SP(save_sp)   ddr_save_sp(save_sp)

void ddr_init(void);
uint32_t ddr_set_rate(uint32_t hz);
uint32_t ddr_round_rate(uint32_t hz);
uint32_t ddr_get_rate(void);
void clr_dcf_irq(void);
uint32_t dts_timing_receive(uint32_t timing, uint32_t index);
#endif

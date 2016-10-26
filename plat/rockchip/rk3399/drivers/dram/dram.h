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

#ifndef __SOC_ROCKCHIP_RK3399_DRAM_H__
#define __SOC_ROCKCHIP_RK3399_DRAM_H__
#include <plat_private.h>
#include <stdint.h>

#define CTL_BASE(ch)		(0xffa80000 + (ch) * 0x8000)
#define CTL_REG(ch, n)		(CTL_BASE(ch) + (n) * 0x4)

#define PI_OFFSET		0x800
#define PI_BASE(ch)		(CTL_BASE(ch) + PI_OFFSET)
#define PI_REG(ch, n)		(PI_BASE(ch) + (n) * 0x4)

#define PHY_OFFSET		0x2000
#define PHY_BASE(ch)		(CTL_BASE(ch) + PHY_OFFSET)
#define PHY_REG(ch, n)		(PHY_BASE(ch) + (n) * 0x4)

#define MSCH_BASE(ch)		(0xffa84000 + (ch) * 0x8000)
#define MSCH_ID_COREID		0x0
#define MSCH_ID_REVISIONID	0x4
#define MSCH_DEVICECONF		0x8
#define MSCH_DEVICESIZE		0xc
#define MSCH_DDRTIMINGA0	0x10
#define MSCH_DDRTIMINGB0	0x14
#define MSCH_DDRTIMINGC0	0x18
#define MSCH_DEVTODEV0		0x1c
#define MSCH_DDRMODE		0x110
#define MSCH_AGINGX0		0x1000

#define CIC_CTRL0	0x0
#define CIC_CTRL1	0x4
#define CIC_IDLE_TH	0x8
#define CIC_CG_WAIT_TH	0xc
#define CIC_STATUS0	0x10
#define CIC_STATUS1	0x14
#define CIC_CTRL2	0x18
#define CIC_CTRL3	0x1c
#define CIC_CTRL4	0x20

/* DENALI_CTL_00 */
#define START			1

/* DENALI_CTL_68 */
#define PWRUP_SREFRESH_EXIT	(1 << 16)

/* DENALI_CTL_274 */
#define MEM_RST_VALID		1

#define PHY_DRV_ODT_Hi_Z	0x0
#define PHY_DRV_ODT_240		0x1
#define PHY_DRV_ODT_120		0x8
#define PHY_DRV_ODT_80		0x9
#define PHY_DRV_ODT_60		0xc
#define PHY_DRV_ODT_48		0xd
#define PHY_DRV_ODT_40		0xe
#define PHY_DRV_ODT_34_3	0xf

/*
 * sys_reg bitfield struct
 * [31] row_3_4_ch1
 * [30] row_3_4_ch0
 * [29:28] chinfo
 * [27] rank_ch1
 * [26:25] col_ch1
 * [24] bk_ch1
 * [23:22] cs0_row_ch1
 * [21:20] cs1_row_ch1
 * [19:18] bw_ch1
 * [17:16] dbw_ch1;
 * [15:13] ddrtype
 * [12] channelnum
 * [11] rank_ch0
 * [10:9] col_ch0
 * [8] bk_ch0
 * [7:6] cs0_row_ch0
 * [5:4] cs1_row_ch0
 * [3:2] bw_ch0
 * [1:0] dbw_ch0
 */
#define SYS_REG_ENC_ROW_3_4(n, ch)	((n) << (30 + (ch)))
#define SYS_REG_DEC_ROW_3_4(n, ch)	(((n) >> (30 + (ch))) & 0x1)
#define SYS_REG_ENC_CHINFO(ch)		(1 << (28 + (ch)))
#define SYS_REG_DEC_CHINFO(n, ch)	(((n) >> (28 + (ch))) & 0x1)
#define SYS_REG_ENC_DDRTYPE(n)		((n) << 13)
#define SYS_REG_DEC_DDRTYPE(n)		(((n) >> 13) & 0x7)
#define SYS_REG_ENC_NUM_CH(n)		(((n) - 1) << 12)
#define SYS_REG_DEC_NUM_CH(n)		(1 + (((n) >> 12) & 0x1))
#define SYS_REG_ENC_RANK(n, ch)		(((n) - 1) << (11 + (ch) * 16))
#define SYS_REG_DEC_RANK(n, ch)		(1 + (((n) >> (11 + (ch) * 16)) & 0x1))
#define SYS_REG_ENC_COL(n, ch)		(((n) - 9) << (9 + (ch) * 16))
#define SYS_REG_DEC_COL(n, ch)		(9 + (((n) >> (9 + (ch) * 16)) & 0x3))
#define SYS_REG_ENC_BK(n, ch)		(((n) == 3 ? 0 : 1) << (8 + (ch) * 16))
#define SYS_REG_DEC_BK(n, ch)		(3 - (((n) >> (8 + (ch) * 16)) & 0x1))
#define SYS_REG_ENC_CS0_ROW(n, ch)	(((n) - 13) << (6 + (ch) * 16))
#define SYS_REG_DEC_CS0_ROW(n, ch)	(13 + (((n) >> (6 + (ch) * 16)) & 0x3))
#define SYS_REG_ENC_CS1_ROW(n, ch)	(((n) - 13) << (4 + (ch) * 16))
#define SYS_REG_DEC_CS1_ROW(n, ch)	(13 + (((n) >> (4 + (ch) * 16)) & 0x3))
#define SYS_REG_ENC_BW(n, ch)		((2 >> (n)) << (2 + (ch) * 16))
#define SYS_REG_DEC_BW(n, ch)		(2 >> (((n) >> (2 + (ch) * 16)) & 0x3))
#define SYS_REG_ENC_DBW(n, ch)		((2 >> (n)) << (0 + (ch) * 16))
#define SYS_REG_DEC_DBW(n, ch)		(2 >> (((n) >> (0 + (ch) * 16)) & 0x3))
#define DDR_STRIDE(n)		mmio_write_32(SGRF_BASE + SGRF_SOC_CON3_7(4), \
					      (0x1f<<(10+16))|((n)<<10))

#define CTL_REG_NUM		332
#define PHY_REG_NUM		959
#define PI_REG_NUM		200

enum {
	DDR3 = 3,
	LPDDR2 = 5,
	LPDDR3 = 6,
	LPDDR4 = 7,
	UNUSED = 0xff
};

struct rk3399_ddr_pctl_regs {
	uint32_t denali_ctl[CTL_REG_NUM];
};

struct rk3399_ddr_publ_regs {
	uint32_t denali_phy[PHY_REG_NUM];
};

struct rk3399_ddr_pi_regs {
	uint32_t denali_pi[PI_REG_NUM];
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

struct rk3399_msch_timings {
	union noc_ddrtiminga0 ddrtiminga0;
	union noc_ddrtimingb0 ddrtimingb0;
	union noc_ddrtimingc0 ddrtimingc0;
	union noc_devtodev0 devtodev0;
	union noc_ddrmode ddrmode;
	uint32_t agingx0;
};

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

extern __sramdata struct rk3399_sdram_params sdram_config;

void dram_init(void);

#endif

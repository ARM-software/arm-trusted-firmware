/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DRAM_REGS_H
#define DRAM_REGS_H

#define CTL_REG_NUM		332
#define PHY_REG_NUM		959
#define PI_REG_NUM		200

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

#define CIC_CTRL0		0x0
#define CIC_CTRL1		0x4
#define CIC_IDLE_TH		0x8
#define CIC_CG_WAIT_TH		0xc
#define CIC_STATUS0		0x10
#define CIC_STATUS1		0x14
#define CIC_CTRL2		0x18
#define CIC_CTRL3		0x1c
#define CIC_CTRL4		0x20

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

#endif /* DRAM_REGS_H */

/*
 * Copyright (c) 2024-2025, Rockchip Electronics Co., Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef OTP_H
#define OTP_H

#define WRITE_MASK			(16)

#define OTP_MAX_SIZE			512
#define OTP_MAX_BYTE_SIZE		(OTP_MAX_SIZE * 2)
#define OTP_S_SIZE			448
#define OTP_S_BYTE_SIZE			(OTP_S_SIZE * 2)
#define OTP_NS_SIZE			64
#define OTP_NS_BYTE_SIZE		(OTP_NS_SIZE * 2)
#define OTP_PROG_MASK			0
#define OTP_PROG_MASK_NUM		32
#define OTP_READ_MASK			0x0100
#define OTP_READ_MASK_NUM		32
#define IS_READ_MASK			0
#define IS_WRITE_MASK			1
#define IS_DISBALE_MASK			0
#define IS_ENABLE_MASK			1
#define OTP_MASK_BYPASS			0x200

/* CRU controller register */
#define CLK_NS_OTP_USER_EN		(1 << 11)
#define CLK_NS_OTP_SBPI_EN		(1 << 10)
#define PCLK_NS_OTP_EN			(1 << 9)
#define PCLK_PHY_OTP_EN			(1 << 13)
#define OTP_PHY_SRSTN			(1u << 15)

/* SCRU controller register */
#define CLK_S_OTP_USER_EN		(1 << 7)
#define CLK_S_OTP_SBPI_EN		(1 << 6)
#define PCLK_S_OTP_EN			(1 << 5)

/* SGRF controller register */
#define SGRF_CON_OTP_CKE		(1 << 2)
#define SGRF_CON_OTP_SECURE		(1 << 1)

/* OTP controller register */
#define REG_OTPC_SBPI_CTRL		(0x0020)
#define SBPI_DEV_ID_SHIFT		(8)
#define SBPI_DEV_ID_MASK		(0xFF)
#define SBPI_CS_DEASSERT		(1 << 3)
#define SBPI_CS_AUTO			(1 << 2)
#define SBPI_SP				(1 << 1)
#define SBPI_ENABLE			(1 << 0)
#define REG_OTPC_SBPI_CMD_VALID_PRE	(0x0024)
#define REG_OTPC_SBPI_CS_VALID_PRE	(0x0028)
#define REG_OTPC_SBPI_STATUS		(0x002C)
#define REG_OTPC_USER_CTRL		(0x0100)
#define USER_PD				(1 << 1)
#define USER_DCTRL			(1 << 0)
#define REG_OTPC_USER_ADDR		(0x0104)
#define REG_OTPC_USER_ENABLE		(0x0108)
#define USER_ENABLE			(1 << 0)
#define REG_OTPC_USER_STATUS		(0x0110)
#define REG_OTPC_USER_QP		(0x0120)
#define REG_OTPC_USER_Q			(0x0124)
#define REG_OTPC_USER_QSR		(0x0128)
#define REG_OTPC_USER_QRR		(0x012C)
#define REG_OTPC_SBPI_CMD_OFFSET(n)	(0x1000 + (n << 2))
#define REG_OTPC_SBPI_READ_DATA_BASE	(0x2000)
#define REG_OTPC_INT_CON		(0x0300)
#define REG_OTPC_INT_STATUS		(0x0304)

#define REG_KEY_READER_CONFIG		0x00
#define OTP_KEY_ACCESS_START		(1 << 0)
#define SBPI_VAILI_COMMAND(n)		(0xffff0000 | n)

int rk_otp_read(uint32_t addr, uint32_t length,
		    uint16_t *buf, bool is_need_ecc);
int rk_otp_ns_ecc_flag(uint32_t addr);
#endif /* OTP_H */

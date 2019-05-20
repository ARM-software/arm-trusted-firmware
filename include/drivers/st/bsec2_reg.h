/*
 * Copyright (c) 2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BSEC2_REG_H
#define BSEC2_REG_H

#include <lib/utils_def.h>

/* IP configuration */
#define ADDR_LOWER_OTP_PERLOCK_SHIFT	0x03
#define DATA_LOWER_OTP_PERLOCK_BIT	0x03U /* 2 significants bits are used */
#define DATA_LOWER_OTP_PERLOCK_MASK	GENMASK(2, 0)
#define ADDR_UPPER_OTP_PERLOCK_SHIFT	0x04
#define DATA_UPPER_OTP_PERLOCK_BIT	0x01U /* 1 significants bits are used */
#define DATA_UPPER_OTP_PERLOCK_MASK	GENMASK(3, 0)

/* BSEC REGISTER OFFSET (base relative) */
#define BSEC_OTP_CONF_OFF		U(0x000)
#define BSEC_OTP_CTRL_OFF		U(0x004)
#define BSEC_OTP_WRDATA_OFF		U(0x008)
#define BSEC_OTP_STATUS_OFF		U(0x00C)
#define BSEC_OTP_LOCK_OFF		U(0x010)
#define BSEC_DEN_OFF			U(0x014)
#define BSEC_DISTURBED_OFF		U(0x01C)
#define BSEC_DISTURBED1_OFF		U(0x020)
#define BSEC_DISTURBED2_OFF		U(0x024)
#define BSEC_ERROR_OFF			U(0x034)
#define BSEC_ERROR1_OFF			U(0x038)
#define BSEC_ERROR2_OFF			U(0x03C)
#define BSEC_WRLOCK_OFF			U(0x04C) /* Safmem permanent lock */
#define BSEC_WRLOCK1_OFF		U(0x050)
#define BSEC_WRLOCK2_OFF		U(0x054)
#define BSEC_SPLOCK_OFF			U(0x064) /* Program safmem sticky lock */
#define BSEC_SPLOCK1_OFF		U(0x068)
#define BSEC_SPLOCK2_OFF		U(0x06C)
#define BSEC_SWLOCK_OFF			U(0x07C) /* Write in OTP sticky lock */
#define BSEC_SWLOCK1_OFF		U(0x080)
#define BSEC_SWLOCK2_OFF		U(0x084)
#define BSEC_SRLOCK_OFF			U(0x094) /* Shadowing sticky lock */
#define BSEC_SRLOCK1_OFF		U(0x098)
#define BSEC_SRLOCK2_OFF		U(0x09C)
#define BSEC_JTAG_IN_OFF		U(0x0AC)
#define BSEC_JTAG_OUT_OFF		U(0x0B0)
#define BSEC_SCRATCH_OFF		U(0x0B4)
#define BSEC_OTP_DATA_OFF		U(0x200)
#define BSEC_IPHW_CFG_OFF		U(0xFF0)
#define BSEC_IPVR_OFF			U(0xFF4)
#define BSEC_IP_ID_OFF			U(0xFF8)
#define BSEC_IP_MAGIC_ID_OFF		U(0xFFC)

#define BSEC_WRLOCK(n)			(BSEC_WRLOCK_OFF + U(0x04) * (n))
#define BSEC_SPLOCK(n)			(BSEC_SPLOCK_OFF + U(0x04) * (n))
#define BSEC_SWLOCK(n)			(BSEC_SWLOCK_OFF + U(0x04) * (n))
#define BSEC_SRLOCK(n)			(BSEC_SRLOCK_OFF + U(0x04) * (n))

/* BSEC_CONFIGURATION Register */
#define BSEC_CONF_POWER_UP_MASK		BIT(0)
#define BSEC_CONF_POWER_UP_SHIFT	0
#define BSEC_CONF_FRQ_MASK		GENMASK(2, 1)
#define BSEC_CONF_FRQ_SHIFT		1
#define BSEC_CONF_PRG_WIDTH_MASK	GENMASK(6, 3)
#define BSEC_CONF_PRG_WIDTH_SHIFT	3
#define BSEC_CONF_TREAD_MASK		GENMASK(8, 7)
#define BSEC_CONF_TREAD_SHIFT		7

/* BSEC_CONTROL Register */
#define BSEC_READ			0U
#define BSEC_WRITE			BIT(8)
#define BSEC_LOCK			BIT(9)

/* BSEC_OTP_LOCK register */
#define UPPER_OTP_LOCK_MASK		BIT(0)
#define UPPER_OTP_LOCK_SHIFT		0
#define DENREG_LOCK_MASK		BIT(2)
#define DENREG_LOCK_SHIFT		2
#define GPLOCK_LOCK_MASK		BIT(4)
#define GPLOCK_LOCK_SHIFT		4

/* BSEC_OTP_STATUS Register */
#define BSEC_MODE_STATUS_MASK		GENMASK(2, 0)
#define BSEC_MODE_SECURE_MASK		BIT(0)
#define BSEC_MODE_FULLDBG_MASK		BIT(1)
#define BSEC_MODE_INVALID_MASK		BIT(2)
#define BSEC_MODE_BUSY_MASK		BIT(3)
#define BSEC_MODE_PROGFAIL_MASK		BIT(4)
#define BSEC_MODE_PWR_MASK		BIT(5)
#define BSEC_MODE_BIST1_LOCK_MASK	BIT(6)
#define BSEC_MODE_BIST2_LOCK_MASK	BIT(7)

/* BSEC_DENABLE Register */
#define BSEC_HDPEN			BIT(4)
#define BSEC_SPIDEN			BIT(5)
#define BSEC_SPINDEN			BIT(6)
#define BSEC_DBGSWGEN			BIT(10)
#define BSEC_DEN_ALL_MSK		GENMASK(10, 0)

/* BSEC_FENABLE Register */
#define BSEC_FEN_ALL_MSK		GENMASK(14, 0)

/* BSEC_IPVR Register */
#define BSEC_IPVR_MSK			GENMASK(7, 0)

#endif /* BSEC2_REG_H */

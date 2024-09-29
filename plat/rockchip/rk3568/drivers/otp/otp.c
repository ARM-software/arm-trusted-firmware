/*
 * Copyright (c) 2024-2025, Rockchip Electronics Co., Ltd. All rights reserved..
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>

#include "otp.h"
#include <plat_private.h>
#include <platform_def.h>
#include <rk3568_clk.h>
#include <rk3568_def.h>
#include <soc.h>

/* default SBPI_READ mode */
#define OTP_USER_READ		0
#define USEC_PER_SEC		1000000

enum clk_type {
	PCLK_PHY = 0,
	PCLK_NS,
	PCLK_S,
	CLK_NS_USER,
	CLK_NS_SBPI,
	CLK_S_USER,
	CLK_S_SBPI
};

static uint8_t otp_ns_ecc_flag[OTP_NS_BYTE_SIZE / 2];

static uint32_t enable_otp_clk(int clk)
{
	uint32_t reg = 0;

	switch (clk) {
	case CLK_NS_USER:
		reg = mmio_read_32(CRU_BASE + CRU_CLKGATES_CON(26));
		if (reg & CLK_NS_OTP_USER_EN)
			mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(26),
				      CLK_NS_OTP_USER_EN << WRITE_MASK);

		break;
	case CLK_NS_SBPI:
		reg = mmio_read_32(CRU_BASE + CRU_CLKGATES_CON(26));
		if (reg & CLK_NS_OTP_SBPI_EN)
			mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(26),
				      CLK_NS_OTP_SBPI_EN << WRITE_MASK);

		break;
	case PCLK_NS:
		reg = mmio_read_32(CRU_BASE + CRU_CLKGATES_CON(26));
		if (reg & PCLK_NS_OTP_EN)
			mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(26),
				      PCLK_NS_OTP_EN << WRITE_MASK);

		break;
	case PCLK_PHY:
		reg = mmio_read_32(CRU_BASE + CRU_CLKGATES_CON(34));
		if (reg & PCLK_PHY_OTP_EN)
			mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(34),
				      PCLK_PHY_OTP_EN << WRITE_MASK);

		break;
	case CLK_S_USER:
		reg = mmio_read_32(SCRU_BASE + SCRU_GATE_CON01);
		if (reg & CLK_S_OTP_USER_EN)
			mmio_write_32(SCRU_BASE + SCRU_GATE_CON01,
				      CLK_S_OTP_USER_EN << WRITE_MASK);

		break;
	case CLK_S_SBPI:
		reg = mmio_read_32(SCRU_BASE + SCRU_GATE_CON01);
		if (reg & CLK_S_OTP_SBPI_EN)
			mmio_write_32(SCRU_BASE + SCRU_GATE_CON01,
				      CLK_S_OTP_SBPI_EN << WRITE_MASK);

		break;
	case PCLK_S:
		reg = mmio_read_32(SCRU_BASE + SCRU_GATE_CON01);
		if (reg & PCLK_S_OTP_EN)
			mmio_write_32(SCRU_BASE + SCRU_GATE_CON01,
				      PCLK_S_OTP_EN << WRITE_MASK);

		break;
	default:
		break;
	}

	return reg;
}

static void restore_otp_clk(int clk, uint32_t reg)
{
	switch (clk) {
	case CLK_NS_USER:
		if (reg & CLK_NS_OTP_USER_EN)
			mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(26),
				      (CLK_NS_OTP_USER_EN << WRITE_MASK) | reg);
		break;
	case CLK_NS_SBPI:
		if (reg & CLK_NS_OTP_SBPI_EN)
			mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(26),
				      (CLK_NS_OTP_SBPI_EN << WRITE_MASK) | reg);
		break;
	case PCLK_NS:
		if (reg & PCLK_NS_OTP_EN)
			mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(26),
				      (PCLK_NS_OTP_EN << WRITE_MASK) | reg);
		break;
	case PCLK_PHY:
		if (reg & PCLK_PHY_OTP_EN)
			mmio_write_32(CRU_BASE + CRU_CLKGATES_CON(34),
				      (PCLK_PHY_OTP_EN << WRITE_MASK) | reg);
		break;
	case CLK_S_USER:
		if (reg & CLK_S_OTP_USER_EN)
			mmio_write_32(SCRU_BASE + SCRU_GATE_CON01,
				      (CLK_S_OTP_USER_EN << WRITE_MASK) | reg);
		break;
	case CLK_S_SBPI:
		if (reg & CLK_S_OTP_SBPI_EN)
			mmio_write_32(SCRU_BASE + SCRU_GATE_CON01,
				      (CLK_S_OTP_SBPI_EN << WRITE_MASK) | reg);
		break;
	case PCLK_S:
		if (reg & PCLK_S_OTP_EN)
			mmio_write_32(SCRU_BASE + SCRU_GATE_CON01,
				      (PCLK_S_OTP_EN << WRITE_MASK) | reg);
		break;
	default:
		return;
	}
}

static int check_sbpi_done_int(uint32_t otp_base)
{
	/* wait max 10ms */
	uint32_t timeout = 10000;

	while (1) {
		/* check sbpi DONE INT */
		if (((mmio_read_32(otp_base + REG_OTPC_INT_STATUS) >> 1) & 0x01) == 0x01) {
			mmio_write_32(otp_base + REG_OTPC_INT_STATUS,
				      0xffff0002); /* clear sbpi DONE INT */
			break;
		}

		if (timeout == 0) {
			WARN("---OTP---Check sbpi int done TIMEOUT");
			return -1;
		}

		timeout--;
		udelay(1);
	}

	return 0;
}

static uint32_t otp_select(uint32_t addr)
{
	uint32_t otp_base = 0;

	if (addr < 0x1c0) {  /* 0-447 otp0 S */
		otp_base = OTP_S_BASE;
		mmio_write_32(SGRF_BASE + SGRF_SOC_CON2,
			      (SGRF_CON_OTP_SECURE << WRITE_MASK) |
			       SGRF_CON_OTP_SECURE); /* secure */
	} else if (addr >= 0x1c0 && addr < 0x200) { /* 448-511 otp0 NS */
		otp_base = OTP_NS_BASE;
		mmio_write_32(SGRF_BASE + SGRF_SOC_CON2,
			      SGRF_CON_OTP_SECURE << WRITE_MASK); /* non secure */
	}

	return otp_base;
}

static int rk_otp_ecc_enable(uint32_t otp_base, bool enable)
{
	mmio_write_32(otp_base + REG_OTPC_SBPI_CTRL,
		      BITS_WITH_WMASK(0x2, 0xffu, SBPI_DEV_ID_SHIFT)); /* device id */
	/* a value define number of sbpi valid command */
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_VALID_PRE, SBPI_VAILI_COMMAND(1));
	/* SBPI_CMD will be programmable from offset 0x1000 to 0x2000,
	 * which is 4kBAnd there are 1024 registers totally, which are
	 * correspond to a sertain command.The address of these registers
	 * are: 0x10000x1004...0x1ffc
	 */
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(0), 0xfa); /* sbpi cmd */

	if (enable)
		mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(1),
			      0x00); /* sbpi cmd enable ecc*/
	else
		mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(1),
			      0x09); /* sbpi cmd disable ecc*/

	mmio_write_32(otp_base + REG_OTPC_SBPI_CTRL,
		      (SBPI_ENABLE << WRITE_MASK) | SBPI_ENABLE); /* sbpi enable */

	if (check_sbpi_done_int(otp_base))
		return -1;

	return 0;
}

static int rk_otp_sbpi_read(uint32_t addr, uint16_t *read_data, bool is_need_ecc)
{
	uint32_t otp_base = 0;
	uint32_t otp_qp;

	otp_base = otp_select(addr);

	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(28),
		      (OTP_PHY_SRSTN << WRITE_MASK) | OTP_PHY_SRSTN); /* reset otp phy */
	udelay(2);
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(28),
		      OTP_PHY_SRSTN << WRITE_MASK);
	udelay(1);

	mmio_write_32(SGRF_BASE + SGRF_SOC_CON2,
		      (SGRF_CON_OTP_CKE << WRITE_MASK) | SGRF_CON_OTP_CKE); /* CKE = 1 */
	udelay(2);
	mmio_write_32(otp_base + REG_OTPC_USER_CTRL, USER_DCTRL << WRITE_MASK);
	udelay(2);

	rk_otp_ecc_enable(otp_base, is_need_ecc);

	mmio_write_32(otp_base + REG_OTPC_SBPI_CTRL,
		      (SBPI_CS_AUTO << WRITE_MASK) | SBPI_CS_AUTO); /* cs auto */
	mmio_write_32(otp_base + REG_OTPC_SBPI_CS_VALID_PRE,
		      0xffff0000); /* cs valid number */

	mmio_write_32(otp_base + REG_OTPC_SBPI_CTRL,
		      BITS_WITH_WMASK(0x2, 0xffu, SBPI_DEV_ID_SHIFT)); /* device id */
	/* a value define number of sbpi valid command */
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_VALID_PRE, SBPI_VAILI_COMMAND(2));
	/* SBPI_CMD will be programmable from offset 0x1000 to 0x2000,
	 * which is 4kBAnd there are 1024 registers totally, which are
	 * correspond to a sertain command.The address of these registers
	 * are: 0x10000x1004...0x1ffc
	 */
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(0), 0xfc); /* sbpi cmd */
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(1), addr & 0xff); /* sbpi cmd 3c addr */
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(2),
		      (addr >> 8) & 0xff); /* sbpi cmd 3d addr */
	mmio_write_32(otp_base + REG_OTPC_SBPI_CTRL,
		      (SBPI_ENABLE << WRITE_MASK) | SBPI_ENABLE); /* sbpi enable */

	if (check_sbpi_done_int(otp_base))
		return -1;

	/* a value define number of sbpi valid command */
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_VALID_PRE, SBPI_VAILI_COMMAND(7));
	/* SBPI_CMD will be programmable from offset 0x1000 to 0x2000,
	 * which is 4kBAnd there are 1024 registers totally, which are
	 * correspond to a sertain command.The address of these registers
	 * are: 0x10000x1004...0x1ffc
	 */
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(0), 0x00);
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(1), 0x00);
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(2), 0x40);
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(3), 0x40);
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(4), 0x00);
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(5), 0x02);
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(6), 0x80);
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(7), 0x81);
	/* sbpi enable */
	mmio_write_32(otp_base + REG_OTPC_SBPI_CTRL,
		      (SBPI_ENABLE << WRITE_MASK) | SBPI_ENABLE);

	if (check_sbpi_done_int(otp_base))
		return -1;

	if (is_need_ecc) {
		otp_qp = mmio_read_32(otp_base + REG_OTPC_USER_QP);
		VERBOSE("otp_addr:0x%x, otp_qp:0x%x\n", addr, otp_qp);
		if (((otp_qp & 0xc0) == 0xc0) || (otp_qp & 0x20)) {
			otp_ns_ecc_flag[addr - OTP_S_BYTE_SIZE / 2] = 1;
			ERROR("ecc otp_addr:0x%x, otp_qp failed 0x%x\n", addr, otp_qp);
		}
	}

	*read_data =
		(uint16_t)mmio_read_32(otp_base + REG_OTPC_SBPI_READ_DATA_BASE + 0x20);
	*read_data +=
		(uint16_t)(mmio_read_32(otp_base + REG_OTPC_SBPI_READ_DATA_BASE + 0x24) << 8);
	 /* a value define number of sbpi valid command */
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_VALID_PRE, SBPI_VAILI_COMMAND(1));
	/* SBPI_CMD will be programmable from offset 0x1000 to 0x2000,
	 * which is 4kBAnd there are 1024 registers totally, which are
	 * correspond to a sertain command.The address of these registers
	 * are: 0x10000x1004...0x1ffc
	 */
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(0), 0xa0);
	mmio_write_32(otp_base + REG_OTPC_SBPI_CMD_OFFSET(1), 0x0);
	mmio_write_32(otp_base + REG_OTPC_SBPI_CTRL,
		      (SBPI_ENABLE << WRITE_MASK) | SBPI_ENABLE);

	if (check_sbpi_done_int(otp_base))
		return -1;

	mmio_write_32(otp_base + REG_OTPC_INT_STATUS, 0xffff0003); /* clear sbpi INT */

	mmio_write_32(SGRF_BASE + SGRF_SOC_CON2,
		      SGRF_CON_OTP_SECURE << WRITE_MASK); /* non secure */

	return 0;
}

int rk_otp_read(uint32_t byte_addr, uint32_t byte_len, uint16_t *buf, bool is_need_ecc)
{
	int ret = 0;
	uint32_t i = 0;
	uint32_t clk_ns_user = 0, clk_ns_sbpi = 0, pclk_ns = 0, pclk_phy = 0;
	uint32_t clk_s_user = 0, clk_s_sbpi = 0, pclk_s = 0;
	uint32_t addr, length;

	/*
	 * RK3568 platform OTP R&W by halfword
	 * Address and Length must be an integral multiple of 2
	 */
	if ((byte_addr % 2) != 0 || (byte_len % 2) != 0) {
		ERROR("byte addr and byte length must be even numbers!");
		return -1;
	}

	addr = byte_addr / 2;
	length = byte_len / 2;

	if (addr >= OTP_MAX_SIZE || length <= 0 || length > OTP_MAX_SIZE || !buf)
		return -1;

	if ((addr + length) > OTP_MAX_SIZE)
		return -1;

	if (addr < OTP_S_SIZE && (addr + length) > OTP_S_SIZE) {
		ERROR("Both read secure and non secure otp are not supported!");
		return -1;
	}

	/* enable otp clk if clk is disabled */
	pclk_phy = enable_otp_clk(PCLK_PHY);
	if (addr < 0x1C0) {  /* 0-447 otp0 S */
		pclk_s = enable_otp_clk(PCLK_S);
		clk_s_sbpi = enable_otp_clk(CLK_S_SBPI);
		clk_s_user = enable_otp_clk(CLK_S_USER);
	} else if (addr >= 0x1C0 && addr < 0x200) { /* 448-511 otp0 NS */
		pclk_ns = enable_otp_clk(PCLK_NS);
		clk_ns_sbpi = enable_otp_clk(CLK_NS_SBPI);
		clk_ns_user = enable_otp_clk(CLK_NS_USER);
	}

	for (i = 0; i < length; i++) {

		ret = rk_otp_sbpi_read(addr + i, buf + i, is_need_ecc);
		if (ret) {
			ERROR("---OTP---sbpi read otp failed! addr: 0x%x", addr + i);
			goto out;
		}
	}

out:
	/* restore otp clk state */
	restore_otp_clk(PCLK_PHY, pclk_phy);
	if (addr < 0x1C0) {  /* 0-447 otp0 S */
		restore_otp_clk(PCLK_S, pclk_s);
		restore_otp_clk(CLK_S_SBPI, clk_s_sbpi);
		restore_otp_clk(CLK_S_USER, clk_s_user);
	} else if (addr >= 0x1C0 && addr < 0x200) { /* 448-511 otp0 NS */
		restore_otp_clk(PCLK_NS, pclk_ns);
		restore_otp_clk(CLK_NS_SBPI, clk_ns_sbpi);
		restore_otp_clk(CLK_NS_USER, clk_ns_user);
	}

	return ret;
}

int rk_otp_ns_ecc_flag(uint32_t addr)
{
	if (addr >= OTP_NS_BYTE_SIZE)
		return 0;

	return otp_ns_ecc_flag[addr / 2];
}

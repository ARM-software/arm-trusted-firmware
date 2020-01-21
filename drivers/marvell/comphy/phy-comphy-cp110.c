/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* Marvell CP110 SoC COMPHY unit driver */

#include <errno.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <mg_conf_cm3/mg_conf_cm3.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>

#include <mvebu_def.h>
#include "mvebu.h"
#include "comphy-cp110.h"
#include "phy-comphy-cp110.h"
#include "phy-comphy-common.h"

#if __has_include("phy-porting-layer.h")
#include "phy-porting-layer.h"
#else
#include "phy-default-porting-layer.h"
#endif

/* COMPHY speed macro */
#define COMPHY_SPEED_1_25G		0 /* SGMII 1G */
#define COMPHY_SPEED_2_5G		1
#define COMPHY_SPEED_3_125G		2 /* SGMII 2.5G */
#define COMPHY_SPEED_5G			3
#define COMPHY_SPEED_5_15625G		4 /* XFI 5G */
#define COMPHY_SPEED_6G			5
#define COMPHY_SPEED_10_3125G		6 /* XFI 10G */
#define COMPHY_SPEED_MAX		0x3F
/* The  default speed for IO with fixed known speed */
#define COMPHY_SPEED_DEFAULT		COMPHY_SPEED_MAX

/* Commands for comphy driver */
#define COMPHY_COMMAND_DIGITAL_PWR_OFF		0x00000001
#define COMPHY_COMMAND_DIGITAL_PWR_ON		0x00000002

#define COMPHY_PIPE_FROM_COMPHY_ADDR(x)	((x & ~0xffffff) + 0x120000)

/* System controller registers */
#define PCIE_MAC_RESET_MASK_PORT0	BIT(13)
#define PCIE_MAC_RESET_MASK_PORT1	BIT(11)
#define PCIE_MAC_RESET_MASK_PORT2	BIT(12)
#define SYS_CTRL_UINIT_SOFT_RESET_REG	0x268
#define SYS_CTRL_FROM_COMPHY_ADDR(x)	((x & ~0xffffff) + 0x440000)

/* DFX register spaces */
#define SAR_RST_PCIE0_CLOCK_CONFIG_CP1_OFFSET	(0)
#define SAR_RST_PCIE0_CLOCK_CONFIG_CP1_MASK	(0x1 << \
					SAR_RST_PCIE0_CLOCK_CONFIG_CP1_OFFSET)
#define SAR_RST_PCIE1_CLOCK_CONFIG_CP1_OFFSET	(1)
#define SAR_RST_PCIE1_CLOCK_CONFIG_CP1_MASK	(0x1 << \
					SAR_RST_PCIE1_CLOCK_CONFIG_CP1_OFFSET)
#define SAR_STATUS_0_REG			200
#define DFX_FROM_COMPHY_ADDR(x)			((x & ~0xffffff) + DFX_BASE)

/* The same Units Soft Reset Config register are accessed in all PCIe ports
 * initialization, so a spin lock is defined in case when more than 1 CPUs
 * resets PCIe MAC and need to access the register in the same time. The spin
 * lock is shared by all CP110 units.
 */
spinlock_t cp110_mac_reset_lock;

/* These values come from the PCI Express Spec */
enum pcie_link_width {
	PCIE_LNK_WIDTH_RESRV	= 0x00,
	PCIE_LNK_X1		= 0x01,
	PCIE_LNK_X2		= 0x02,
	PCIE_LNK_X4		= 0x04,
	PCIE_LNK_X8		= 0x08,
	PCIE_LNK_X12		= 0x0C,
	PCIE_LNK_X16		= 0x10,
	PCIE_LNK_X32		= 0x20,
	PCIE_LNK_WIDTH_UNKNOWN  = 0xFF,
};

_Bool rx_trainng_done[AP_NUM][CP_NUM][MAX_LANE_NR] = {0};

static void mvebu_cp110_get_ap_and_cp_nr(uint8_t *ap_nr, uint8_t *cp_nr,
					 uint64_t comphy_base)
{
#if (AP_NUM == 1)
	*ap_nr = 0;
#else
	*ap_nr = (((comphy_base & ~0xffffff) - MVEBU_AP_IO_BASE(0)) /
			 AP_IO_OFFSET);
#endif

	*cp_nr = (((comphy_base & ~0xffffff) - MVEBU_AP_IO_BASE(*ap_nr)) /
		 MVEBU_CP_OFFSET);

	debug("cp_base 0x%llx, ap_io_base 0x%lx, cp_offset 0x%lx\n",
	       comphy_base, (unsigned long)MVEBU_AP_IO_BASE(*ap_nr),
	       (unsigned long)MVEBU_CP_OFFSET);
}

/* Clear PIPE selector - avoid collision with previous configuration */
static void mvebu_cp110_comphy_clr_pipe_selector(uint64_t comphy_base,
						 uint8_t comphy_index)
{
	uint32_t reg, mask, field;
	uint32_t comphy_offset =
			COMMON_SELECTOR_COMPHYN_FIELD_WIDTH * comphy_index;

	mask = COMMON_SELECTOR_COMPHY_MASK << comphy_offset;
	reg = mmio_read_32(comphy_base + COMMON_SELECTOR_PIPE_REG_OFFSET);
	field = reg & mask;

	if (field) {
		reg &= ~mask;
		mmio_write_32(comphy_base + COMMON_SELECTOR_PIPE_REG_OFFSET,
			     reg);
	}
}

/* Clear PHY selector - avoid collision with previous configuration */
static void mvebu_cp110_comphy_clr_phy_selector(uint64_t comphy_base,
						uint8_t comphy_index)
{
	uint32_t reg, mask, field;
	uint32_t comphy_offset =
			COMMON_SELECTOR_COMPHYN_FIELD_WIDTH * comphy_index;

	mask = COMMON_SELECTOR_COMPHY_MASK << comphy_offset;
	reg = mmio_read_32(comphy_base + COMMON_SELECTOR_PHY_REG_OFFSET);
	field = reg & mask;

	/* Clear comphy selector - if it was already configured.
	 * (might be that this comphy was configured as PCIe/USB,
	 * in such case, no need to clear comphy selector because PCIe/USB
	 * are controlled by hpipe selector).
	 */
	if (field) {
		reg &= ~mask;
		mmio_write_32(comphy_base + COMMON_SELECTOR_PHY_REG_OFFSET,
			      reg);
	}
}

/* PHY selector configures SATA and Network modes */
static void mvebu_cp110_comphy_set_phy_selector(uint64_t comphy_base,
				     uint8_t comphy_index, uint32_t comphy_mode)
{
	uint32_t reg, mask;
	uint32_t comphy_offset =
			COMMON_SELECTOR_COMPHYN_FIELD_WIDTH * comphy_index;
	int mode;

	/* If phy selector is used the pipe selector should be marked as
	 * unconnected.
	 */
	mvebu_cp110_comphy_clr_pipe_selector(comphy_base, comphy_index);

	/* Comphy mode (compound of the IO mode and id). Here, only the IO mode
	 * is required to distinguish between SATA and network modes.
	 */
	mode = COMPHY_GET_MODE(comphy_mode);

	mask = COMMON_SELECTOR_COMPHY_MASK << comphy_offset;
	reg = mmio_read_32(comphy_base + COMMON_SELECTOR_PHY_REG_OFFSET);
	reg &= ~mask;

	/* SATA port 0/1 require the same configuration */
	if (mode == COMPHY_SATA_MODE) {
		/* SATA selector values is always 4 */
		reg |= COMMON_SELECTOR_COMPHYN_SATA << comphy_offset;
	} else {
		switch (comphy_index) {
		case(0):
		case(1):
		case(2):
			/* For comphy 0,1, and 2:
			 * Network selector value is always 1.
			 */
			reg |= COMMON_SELECTOR_COMPHY0_1_2_NETWORK <<
				comphy_offset;
			break;
		case(3):
			/* For comphy 3:
			 * 0x1 = RXAUI_Lane1
			 * 0x2 = SGMII/HS-SGMII Port1
			 */
			if (mode == COMPHY_RXAUI_MODE)
				reg |= COMMON_SELECTOR_COMPHY3_RXAUI <<
					comphy_offset;
			else
				reg |= COMMON_SELECTOR_COMPHY3_SGMII <<
					comphy_offset;
			break;
		case(4):
			 /* For comphy 4:
			  * 0x1 = SGMII/HS-SGMII Port1, XFI1/SFI1
			  * 0x2 = SGMII/HS-SGMII Port0: XFI0/SFI0, RXAUI_Lane0
			  *
			  * We want to check if SGMII1/HS_SGMII1 is the
			  * requested mode in order to determine which value
			  * should be set (all other modes use the same value)
			  * so we need to strip the mode, and check the ID
			  * because we might handle SGMII0/HS_SGMII0 too.
			  */
			  /* TODO: need to distinguish between CP110 and CP115
			   * as SFI1/XFI1 available only for CP115.
			   */
			if ((mode == COMPHY_SGMII_MODE ||
			     mode == COMPHY_HS_SGMII_MODE ||
			     mode == COMPHY_SFI_MODE ||
			     mode == COMPHY_XFI_MODE ||
			     mode == COMPHY_AP_MODE)
			    && COMPHY_GET_ID(comphy_mode) == 1)
				reg |= COMMON_SELECTOR_COMPHY4_PORT1 <<
					comphy_offset;
			else
				reg |= COMMON_SELECTOR_COMPHY4_ALL_OTHERS <<
					comphy_offset;
			break;
		case(5):
			/* For comphy 5:
			 * 0x1 = SGMII/HS-SGMII Port2
			 * 0x2 = RXAUI Lane1
			 */
			if (mode == COMPHY_RXAUI_MODE)
				reg |= COMMON_SELECTOR_COMPHY5_RXAUI <<
					comphy_offset;
			else
				reg |= COMMON_SELECTOR_COMPHY5_SGMII <<
					comphy_offset;
			break;
		}
	}

	mmio_write_32(comphy_base + COMMON_SELECTOR_PHY_REG_OFFSET, reg);
}

/* PIPE selector configures for PCIe, USB 3.0 Host, and USB 3.0 Device mode */
static void mvebu_cp110_comphy_set_pipe_selector(uint64_t comphy_base,
				     uint8_t comphy_index, uint32_t comphy_mode)
{
	uint32_t reg;
	uint32_t shift = COMMON_SELECTOR_COMPHYN_FIELD_WIDTH * comphy_index;
	int mode = COMPHY_GET_MODE(comphy_mode);
	uint32_t mask = COMMON_SELECTOR_COMPHY_MASK << shift;
	uint32_t pipe_sel = 0x0;

	/* If pipe selector is used the phy selector should be marked as
	 * unconnected.
	 */
	mvebu_cp110_comphy_clr_phy_selector(comphy_base, comphy_index);

	reg = mmio_read_32(comphy_base + COMMON_SELECTOR_PIPE_REG_OFFSET);
	reg &= ~mask;

	switch (mode) {
	case (COMPHY_PCIE_MODE):
		/* For lanes support PCIE, selector value are all same */
		pipe_sel = COMMON_SELECTOR_PIPE_COMPHY_PCIE;
		break;

	case (COMPHY_USB3H_MODE):
		/* Only lane 1-4 support USB host, selector value is same */
		if (comphy_index == COMPHY_LANE0 ||
		    comphy_index == COMPHY_LANE5)
			ERROR("COMPHY[%d] mode[%d] is invalid\n",
			      comphy_index, mode);
		else
			pipe_sel = COMMON_SELECTOR_PIPE_COMPHY_USBH;
		break;

	case (COMPHY_USB3D_MODE):
		/* Lane 1 and 4 support USB device, selector value is same */
		if (comphy_index == COMPHY_LANE1 ||
		    comphy_index == COMPHY_LANE4)
			pipe_sel = COMMON_SELECTOR_PIPE_COMPHY_USBD;
		else
			ERROR("COMPHY[%d] mode[%d] is invalid\n", comphy_index,
			      mode);
		break;

	default:
		ERROR("COMPHY[%d] mode[%d] is invalid\n", comphy_index, mode);
		break;
	}

	mmio_write_32(comphy_base + COMMON_SELECTOR_PIPE_REG_OFFSET, reg |
		      (pipe_sel << shift));
}

int mvebu_cp110_comphy_is_pll_locked(uint64_t comphy_base, uint8_t comphy_index)
{
	uintptr_t sd_ip_addr, addr;
	uint32_t mask, data;
	int ret = 0;

	debug_enter();

	sd_ip_addr = SD_ADDR(COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base),
			     comphy_index);

	addr = sd_ip_addr + SD_EXTERNAL_STATUS0_REG;
	data = SD_EXTERNAL_STATUS0_PLL_TX_MASK &
		SD_EXTERNAL_STATUS0_PLL_RX_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask,
				    PLL_LOCK_TIMEOUT, REG_32BIT);
	if (data != 0) {
		if (data & SD_EXTERNAL_STATUS0_PLL_RX_MASK)
			ERROR("RX PLL is not locked\n");
		if (data & SD_EXTERNAL_STATUS0_PLL_TX_MASK)
			ERROR("TX PLL is not locked\n");

		ret = -ETIMEDOUT;
	}

	debug_exit();

	return ret;
}

static void mvebu_cp110_polarity_invert(uintptr_t addr, uint8_t phy_polarity_invert)
{
	uint32_t mask, data;

	/* Set RX / TX polarity */
	data = mask = 0x0U;
	if ((phy_polarity_invert & COMPHY_POLARITY_TXD_INVERT) != 0) {
		data |= (1 << HPIPE_SYNC_PATTERN_TXD_INV_OFFSET);
		mask |= HPIPE_SYNC_PATTERN_TXD_INV_MASK;
		debug("%s: inverting TX polarity\n", __func__);
	}

	if ((phy_polarity_invert & COMPHY_POLARITY_RXD_INVERT) != 0) {
		data |= (1 << HPIPE_SYNC_PATTERN_RXD_INV_OFFSET);
		mask |= HPIPE_SYNC_PATTERN_RXD_INV_MASK;
		debug("%s: inverting RX polarity\n", __func__);
	}

	reg_set(addr, data, mask);
}

static int mvebu_cp110_comphy_sata_power_on(uint64_t comphy_base,
				     uint8_t comphy_index, uint32_t comphy_mode)
{
	uintptr_t hpipe_addr, sd_ip_addr, comphy_addr;
	uint32_t mask, data;
	uint8_t ap_nr, cp_nr, phy_polarity_invert;
	int ret = 0;

	debug_enter();

	mvebu_cp110_get_ap_and_cp_nr(&ap_nr, &cp_nr, comphy_base);

	const struct sata_params *sata_static_values =
			&sata_static_values_tab[ap_nr][cp_nr][comphy_index];

	phy_polarity_invert = sata_static_values->polarity_invert;

	/* configure phy selector for SATA */
	mvebu_cp110_comphy_set_phy_selector(comphy_base,
					    comphy_index, comphy_mode);

	hpipe_addr = HPIPE_ADDR(COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base),
				comphy_index);
	sd_ip_addr = SD_ADDR(COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base),
			     comphy_index);
	comphy_addr = COMPHY_ADDR(comphy_base, comphy_index);

	debug(" add hpipe 0x%lx, sd 0x%lx, comphy 0x%lx\n",
					   hpipe_addr, sd_ip_addr, comphy_addr);
	debug("stage: RFU configurations - hard reset comphy\n");
	/* RFU configurations - hard reset comphy */
	mask = COMMON_PHY_CFG1_PWR_UP_MASK;
	data = 0x1 << COMMON_PHY_CFG1_PWR_UP_OFFSET;
	mask |= COMMON_PHY_CFG1_PIPE_SELECT_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_PIPE_SELECT_OFFSET;
	mask |= COMMON_PHY_CFG1_PWR_ON_RESET_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_PWR_ON_RESET_OFFSET;
	mask |= COMMON_PHY_CFG1_CORE_RSTN_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_CORE_RSTN_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG1_REG, data, mask);

	/* Set select data  width 40Bit - SATA mode only */
	reg_set(comphy_addr + COMMON_PHY_CFG6_REG,
		0x1 << COMMON_PHY_CFG6_IF_40_SEL_OFFSET,
		COMMON_PHY_CFG6_IF_40_SEL_MASK);

	/* release from hard reset in SD external */
	mask = SD_EXTERNAL_CONFIG1_RESET_IN_MASK;
	data = 0x1 << SD_EXTERNAL_CONFIG1_RESET_IN_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RESET_CORE_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG1_RESET_CORE_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	/* Wait 1ms - until band gap and ref clock ready */
	mdelay(1);

	debug("stage: Comphy configuration\n");
	/* Start comphy Configuration */
	/* Set reference clock to comes from group 1 - choose 25Mhz */
	reg_set(hpipe_addr + HPIPE_MISC_REG,
		0x0 << HPIPE_MISC_REFCLK_SEL_OFFSET,
		HPIPE_MISC_REFCLK_SEL_MASK);
	/* Reference frequency select set 1 (for SATA = 25Mhz) */
	mask = HPIPE_PWR_PLL_REF_FREQ_MASK;
	data = 0x1 << HPIPE_PWR_PLL_REF_FREQ_OFFSET;
	/* PHY mode select (set SATA = 0x0 */
	mask |= HPIPE_PWR_PLL_PHY_MODE_MASK;
	data |= 0x0 << HPIPE_PWR_PLL_PHY_MODE_OFFSET;
	reg_set(hpipe_addr + HPIPE_PWR_PLL_REG, data, mask);
	/* Set max PHY generation setting - 6Gbps */
	reg_set(hpipe_addr + HPIPE_INTERFACE_REG,
		0x2 << HPIPE_INTERFACE_GEN_MAX_OFFSET,
		HPIPE_INTERFACE_GEN_MAX_MASK);
	/* Set select data  width 40Bit (SEL_BITS[2:0]) */
	reg_set(hpipe_addr + HPIPE_LOOPBACK_REG,
		0x2 << HPIPE_LOOPBACK_SEL_OFFSET, HPIPE_LOOPBACK_SEL_MASK);

	debug("stage: Analog parameters from ETP(HW)\n");
	/* G1 settings */
	mask = HPIPE_G1_SET_1_G1_RX_SELMUPI_MASK;
	data = sata_static_values->g1_rx_selmupi <<
			HPIPE_G1_SET_1_G1_RX_SELMUPI_OFFSET;
	mask |= HPIPE_G1_SET_1_G1_RX_SELMUPF_MASK;
	data |= sata_static_values->g1_rx_selmupf <<
			HPIPE_G1_SET_1_G1_RX_SELMUPF_OFFSET;
	mask |= HPIPE_G1_SET_1_G1_RX_SELMUFI_MASK;
	data |= sata_static_values->g1_rx_selmufi <<
			HPIPE_G1_SET_1_G1_RX_SELMUFI_OFFSET;
	mask |= HPIPE_G1_SET_1_G1_RX_SELMUFF_MASK;
	data |= sata_static_values->g1_rx_selmuff <<
			HPIPE_G1_SET_1_G1_RX_SELMUFF_OFFSET;
	mask |= HPIPE_G1_SET_1_G1_RX_DIGCK_DIV_MASK;
	data |= 0x1 << HPIPE_G1_SET_1_G1_RX_DIGCK_DIV_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SET_1_REG, data, mask);

	mask = HPIPE_G1_SETTINGS_3_G1_FFE_CAP_SEL_MASK;
	data = 0xf << HPIPE_G1_SETTINGS_3_G1_FFE_CAP_SEL_OFFSET;
	mask |= HPIPE_G1_SETTINGS_3_G1_FFE_RES_SEL_MASK;
	data |= 0x2 << HPIPE_G1_SETTINGS_3_G1_FFE_RES_SEL_OFFSET;
	mask |= HPIPE_G1_SETTINGS_3_G1_FFE_SETTING_FORCE_MASK;
	data |= 0x1 << HPIPE_G1_SETTINGS_3_G1_FFE_SETTING_FORCE_OFFSET;
	mask |= HPIPE_G1_SETTINGS_3_G1_FFE_DEG_RES_LEVEL_MASK;
	data |= 0x1 << HPIPE_G1_SETTINGS_3_G1_FFE_DEG_RES_LEVEL_OFFSET;
	mask |= HPIPE_G1_SETTINGS_3_G1_FFE_LOAD_RES_LEVEL_MASK;
	data |= 0x1 << HPIPE_G1_SETTINGS_3_G1_FFE_LOAD_RES_LEVEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SETTINGS_3_REG, data, mask);

	/* G2 settings */
	mask = HPIPE_G2_SET_1_G2_RX_SELMUPI_MASK;
	data = sata_static_values->g2_rx_selmupi <<
			HPIPE_G2_SET_1_G2_RX_SELMUPI_OFFSET;
	mask |= HPIPE_G2_SET_1_G2_RX_SELMUPF_MASK;
	data |= sata_static_values->g2_rx_selmupf <<
			HPIPE_G2_SET_1_G2_RX_SELMUPF_OFFSET;
	mask |= HPIPE_G2_SET_1_G2_RX_SELMUFI_MASK;
	data |= sata_static_values->g2_rx_selmufi <<
			HPIPE_G2_SET_1_G2_RX_SELMUFI_OFFSET;
	mask |= HPIPE_G2_SET_1_G2_RX_SELMUFF_MASK;
	data |= sata_static_values->g2_rx_selmuff <<
			HPIPE_G2_SET_1_G2_RX_SELMUFF_OFFSET;
	mask |= HPIPE_G2_SET_1_G2_RX_DIGCK_DIV_MASK;
	data |= 0x1 << HPIPE_G2_SET_1_G2_RX_DIGCK_DIV_OFFSET;
	reg_set(hpipe_addr + HPIPE_G2_SET_1_REG, data, mask);

	/* G3 settings */
	mask = HPIPE_G3_SET_1_G3_RX_SELMUPI_MASK;
	data = sata_static_values->g3_rx_selmupi <<
			HPIPE_G3_SET_1_G3_RX_SELMUPI_OFFSET;
	mask |= HPIPE_G3_SET_1_G3_RX_SELMUPF_MASK;
	data |= sata_static_values->g3_rx_selmupf <<
			HPIPE_G3_SET_1_G3_RX_SELMUPF_OFFSET;
	mask |= HPIPE_G3_SET_1_G3_RX_SELMUFI_MASK;
	data |= sata_static_values->g3_rx_selmufi <<
			HPIPE_G3_SET_1_G3_RX_SELMUFI_OFFSET;
	mask |= HPIPE_G3_SET_1_G3_RX_SELMUFF_MASK;
	data |= sata_static_values->g3_rx_selmuff <<
			HPIPE_G3_SET_1_G3_RX_SELMUFF_OFFSET;
	mask |= HPIPE_G3_SET_1_G3_RX_DFE_EN_MASK;
	data |= 0x1 << HPIPE_G3_SET_1_G3_RX_DFE_EN_OFFSET;
	mask |= HPIPE_G3_SET_1_G3_RX_DIGCK_DIV_MASK;
	data |= 0x2 << HPIPE_G3_SET_1_G3_RX_DIGCK_DIV_OFFSET;
	mask |= HPIPE_G3_SET_1_G3_SAMPLER_INPAIRX2_EN_MASK;
	data |= 0x0 << HPIPE_G3_SET_1_G3_SAMPLER_INPAIRX2_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_G3_SET_1_REG, data, mask);

	/* DTL Control */
	mask = HPIPE_PWR_CTR_DTL_SQ_DET_EN_MASK;
	data = 0x1 << HPIPE_PWR_CTR_DTL_SQ_DET_EN_OFFSET;
	mask |= HPIPE_PWR_CTR_DTL_SQ_PLOOP_EN_MASK;
	data |= 0x1 << HPIPE_PWR_CTR_DTL_SQ_PLOOP_EN_OFFSET;
	mask |= HPIPE_PWR_CTR_DTL_FLOOP_EN_MASK;
	data |= 0x1 << HPIPE_PWR_CTR_DTL_FLOOP_EN_OFFSET;
	mask |= HPIPE_PWR_CTR_DTL_CLAMPING_SEL_MASK;
	data |= 0x1 << HPIPE_PWR_CTR_DTL_CLAMPING_SEL_OFFSET;
	mask |= HPIPE_PWR_CTR_DTL_INTPCLK_DIV_FORCE_MASK;
	data |= 0x1 << HPIPE_PWR_CTR_DTL_INTPCLK_DIV_FORCE_OFFSET;
	mask |= HPIPE_PWR_CTR_DTL_CLK_MODE_MASK;
	data |= 0x1 << HPIPE_PWR_CTR_DTL_CLK_MODE_OFFSET;
	mask |= HPIPE_PWR_CTR_DTL_CLK_MODE_FORCE_MASK;
	data |= 0x1 << HPIPE_PWR_CTR_DTL_CLK_MODE_FORCE_OFFSET;
	reg_set(hpipe_addr + HPIPE_PWR_CTR_DTL_REG, data, mask);

	/* Trigger sampler enable pulse */
	mask = HPIPE_SMAPLER_MASK;
	data = 0x1 << HPIPE_SMAPLER_OFFSET;
	reg_set(hpipe_addr + HPIPE_SAMPLER_N_PROC_CALIB_CTRL_REG, data, mask);
	mask = HPIPE_SMAPLER_MASK;
	data = 0x0 << HPIPE_SMAPLER_OFFSET;
	reg_set(hpipe_addr + HPIPE_SAMPLER_N_PROC_CALIB_CTRL_REG, data, mask);

	/* VDD Calibration Control 3 */
	mask = HPIPE_EXT_SELLV_RXSAMPL_MASK;
	data = 0x10 << HPIPE_EXT_SELLV_RXSAMPL_OFFSET;
	reg_set(hpipe_addr + HPIPE_VDD_CAL_CTRL_REG, data, mask);

	/* DFE Resolution Control */
	mask = HPIPE_DFE_RES_FORCE_MASK;
	data = 0x1 << HPIPE_DFE_RES_FORCE_OFFSET;
	reg_set(hpipe_addr + HPIPE_DFE_REG0, data, mask);

	/* DFE F3-F5 Coefficient Control */
	mask = HPIPE_DFE_F3_F5_DFE_EN_MASK;
	data = 0x0 << HPIPE_DFE_F3_F5_DFE_EN_OFFSET;
	mask |= HPIPE_DFE_F3_F5_DFE_CTRL_MASK;
	data = 0x0 << HPIPE_DFE_F3_F5_DFE_CTRL_OFFSET;
	reg_set(hpipe_addr + HPIPE_DFE_F3_F5_REG, data, mask);

	/* G3 Setting 3 */
	mask = HPIPE_G3_FFE_CAP_SEL_MASK;
	data = sata_static_values->g3_ffe_cap_sel <<
			HPIPE_G3_FFE_CAP_SEL_OFFSET;
	mask |= HPIPE_G3_FFE_RES_SEL_MASK;
	data |= sata_static_values->g3_ffe_res_sel <<
			HPIPE_G3_FFE_RES_SEL_OFFSET;
	mask |= HPIPE_G3_FFE_SETTING_FORCE_MASK;
	data |= 0x1 << HPIPE_G3_FFE_SETTING_FORCE_OFFSET;
	mask |= HPIPE_G3_FFE_DEG_RES_LEVEL_MASK;
	data |= 0x1 << HPIPE_G3_FFE_DEG_RES_LEVEL_OFFSET;
	mask |= HPIPE_G3_FFE_LOAD_RES_LEVEL_MASK;
	data |= 0x3 << HPIPE_G3_FFE_LOAD_RES_LEVEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_G3_SETTING_3_REG, data, mask);

	/* G3 Setting 4 */
	mask = HPIPE_G3_DFE_RES_MASK;
	data = sata_static_values->g3_dfe_res << HPIPE_G3_DFE_RES_OFFSET;
	reg_set(hpipe_addr + HPIPE_G3_SETTING_4_REG, data, mask);

	/* Offset Phase Control */
	mask = HPIPE_OS_PH_OFFSET_MASK;
	data = sata_static_values->align90 << HPIPE_OS_PH_OFFSET_OFFSET;
	mask |= HPIPE_OS_PH_OFFSET_FORCE_MASK;
	data |= 0x1 << HPIPE_OS_PH_OFFSET_FORCE_OFFSET;
	mask |= HPIPE_OS_PH_VALID_MASK;
	data |= 0x0 << HPIPE_OS_PH_VALID_OFFSET;
	reg_set(hpipe_addr + HPIPE_PHASE_CONTROL_REG, data, mask);
	mask = HPIPE_OS_PH_VALID_MASK;
	data = 0x1 << HPIPE_OS_PH_VALID_OFFSET;
	reg_set(hpipe_addr + HPIPE_PHASE_CONTROL_REG, data, mask);
	mask = HPIPE_OS_PH_VALID_MASK;
	data = 0x0 << HPIPE_OS_PH_VALID_OFFSET;
	reg_set(hpipe_addr + HPIPE_PHASE_CONTROL_REG, data, mask);

	/* Set G1 TX amplitude and TX post emphasis value */
	mask = HPIPE_G1_SET_0_G1_TX_AMP_MASK;
	data = sata_static_values->g1_amp << HPIPE_G1_SET_0_G1_TX_AMP_OFFSET;
	mask |= HPIPE_G1_SET_0_G1_TX_AMP_ADJ_MASK;
	data |= sata_static_values->g1_tx_amp_adj <<
			HPIPE_G1_SET_0_G1_TX_AMP_ADJ_OFFSET;
	mask |= HPIPE_G1_SET_0_G1_TX_EMPH1_MASK;
	data |= sata_static_values->g1_emph <<
			HPIPE_G1_SET_0_G1_TX_EMPH1_OFFSET;
	mask |= HPIPE_G1_SET_0_G1_TX_EMPH1_EN_MASK;
	data |= sata_static_values->g1_emph_en <<
			HPIPE_G1_SET_0_G1_TX_EMPH1_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SET_0_REG, data, mask);

	/* Set G1 emph */
	mask = HPIPE_G1_SET_2_G1_TX_EMPH0_EN_MASK;
	data = sata_static_values->g1_tx_emph_en <<
			HPIPE_G1_SET_2_G1_TX_EMPH0_EN_OFFSET;
	mask |= HPIPE_G1_SET_2_G1_TX_EMPH0_MASK;
	data |= sata_static_values->g1_tx_emph <<
			HPIPE_G1_SET_2_G1_TX_EMPH0_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SET_2_REG, data, mask);

	/* Set G2 TX amplitude and TX post emphasis value */
	mask = HPIPE_G2_SET_0_G2_TX_AMP_MASK;
	data = sata_static_values->g2_amp << HPIPE_G2_SET_0_G2_TX_AMP_OFFSET;
	mask |= HPIPE_G2_SET_0_G2_TX_AMP_ADJ_MASK;
	data |= sata_static_values->g2_tx_amp_adj <<
			HPIPE_G2_SET_0_G2_TX_AMP_ADJ_OFFSET;
	mask |= HPIPE_G2_SET_0_G2_TX_EMPH1_MASK;
	data |= sata_static_values->g2_emph <<
			HPIPE_G2_SET_0_G2_TX_EMPH1_OFFSET;
	mask |= HPIPE_G2_SET_0_G2_TX_EMPH1_EN_MASK;
	data |= sata_static_values->g2_emph_en <<
			HPIPE_G2_SET_0_G2_TX_EMPH1_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_G2_SET_0_REG, data, mask);

	/* Set G2 emph */
	mask = HPIPE_G2_SET_2_G2_TX_EMPH0_EN_MASK;
	data = sata_static_values->g2_tx_emph_en <<
			HPIPE_G2_SET_2_G2_TX_EMPH0_EN_OFFSET;
	mask |= HPIPE_G2_SET_2_G2_TX_EMPH0_MASK;
	data |= sata_static_values->g2_tx_emph <<
			HPIPE_G2_SET_2_G2_TX_EMPH0_OFFSET;
	reg_set(hpipe_addr + HPIPE_G2_SET_2_REG, data, mask);

	/* Set G3 TX amplitude and TX post emphasis value */
	mask = HPIPE_G3_SET_0_G3_TX_AMP_MASK;
	data = sata_static_values->g3_amp << HPIPE_G3_SET_0_G3_TX_AMP_OFFSET;
	mask |= HPIPE_G3_SET_0_G3_TX_AMP_ADJ_MASK;
	data |= sata_static_values->g3_tx_amp_adj <<
			HPIPE_G3_SET_0_G3_TX_AMP_ADJ_OFFSET;
	mask |= HPIPE_G3_SET_0_G3_TX_EMPH1_MASK;
	data |= sata_static_values->g3_emph <<
			HPIPE_G3_SET_0_G3_TX_EMPH1_OFFSET;
	mask |= HPIPE_G3_SET_0_G3_TX_EMPH1_EN_MASK;
	data |= sata_static_values->g3_emph_en <<
			HPIPE_G3_SET_0_G3_TX_EMPH1_EN_OFFSET;
	mask |= HPIPE_G3_SET_0_G3_TX_SLEW_RATE_SEL_MASK;
	data |= 0x4 << HPIPE_G3_SET_0_G3_TX_SLEW_RATE_SEL_OFFSET;
	mask |= HPIPE_G3_SET_0_G3_TX_SLEW_CTRL_EN_MASK;
	data |= 0x0 << HPIPE_G3_SET_0_G3_TX_SLEW_CTRL_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_G3_SET_0_REG, data, mask);

	/* Set G3 emph */
	mask = HPIPE_G3_SET_2_G3_TX_EMPH0_EN_MASK;
	data = sata_static_values->g3_tx_emph_en <<
			HPIPE_G3_SET_2_G3_TX_EMPH0_EN_OFFSET;
	mask |= HPIPE_G3_SET_2_G3_TX_EMPH0_MASK;
	data |= sata_static_values->g3_tx_emph <<
			HPIPE_G3_SET_2_G3_TX_EMPH0_OFFSET;
	reg_set(hpipe_addr + HPIPE_G3_SET_2_REG, data, mask);

	/* SERDES External Configuration 2 register */
	mask = SD_EXTERNAL_CONFIG2_SSC_ENABLE_MASK;
	data = 0x1 << SD_EXTERNAL_CONFIG2_SSC_ENABLE_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG2_REG, data, mask);

	/* DFE reset sequence */
	reg_set(hpipe_addr + HPIPE_PWR_CTR_REG,
		0x1 << HPIPE_PWR_CTR_RST_DFE_OFFSET,
		HPIPE_PWR_CTR_RST_DFE_MASK);
	reg_set(hpipe_addr + HPIPE_PWR_CTR_REG,
		0x0 << HPIPE_PWR_CTR_RST_DFE_OFFSET,
		HPIPE_PWR_CTR_RST_DFE_MASK);

	if (phy_polarity_invert != 0)
		mvebu_cp110_polarity_invert(hpipe_addr + HPIPE_SYNC_PATTERN_REG,
					    phy_polarity_invert);

	/* SW reset for interrupt logic */
	reg_set(hpipe_addr + HPIPE_PWR_CTR_REG,
		0x1 << HPIPE_PWR_CTR_SFT_RST_OFFSET,
		HPIPE_PWR_CTR_SFT_RST_MASK);
	reg_set(hpipe_addr + HPIPE_PWR_CTR_REG,
		0x0 << HPIPE_PWR_CTR_SFT_RST_OFFSET,
		HPIPE_PWR_CTR_SFT_RST_MASK);

	debug_exit();

	return ret;
}

static int mvebu_cp110_comphy_sgmii_power_on(uint64_t comphy_base,
				     uint8_t comphy_index, uint32_t comphy_mode)
{
	uintptr_t hpipe_addr, sd_ip_addr, comphy_addr, addr;
	uint32_t mask, data, sgmii_speed = COMPHY_GET_SPEED(comphy_mode);
	int ret = 0;

	debug_enter();

	hpipe_addr = HPIPE_ADDR(COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base),
				comphy_index);
	sd_ip_addr = SD_ADDR(COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base),
			     comphy_index);
	comphy_addr = COMPHY_ADDR(comphy_base, comphy_index);

	/* configure phy selector for SGMII */
	mvebu_cp110_comphy_set_phy_selector(comphy_base, comphy_index,
					    comphy_mode);

	/* Confiugre the lane */
	debug("stage: RFU configurations - hard reset comphy\n");
	/* RFU configurations - hard reset comphy */
	mask = COMMON_PHY_CFG1_PWR_UP_MASK;
	data = 0x1 << COMMON_PHY_CFG1_PWR_UP_OFFSET;
	mask |= COMMON_PHY_CFG1_PIPE_SELECT_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_PIPE_SELECT_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG1_REG, data, mask);

	/* Select Baud Rate of Comphy And PD_PLL/Tx/Rx */
	mask = SD_EXTERNAL_CONFIG0_SD_PU_PLL_MASK;
	data = 0x0 << SD_EXTERNAL_CONFIG0_SD_PU_PLL_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PHY_GEN_RX_MASK;
	mask |= SD_EXTERNAL_CONFIG0_SD_PHY_GEN_TX_MASK;

	if (sgmii_speed == COMPHY_SPEED_1_25G) {
		/* SGMII 1G, SerDes speed 1.25G */
		data |= 0x6 << SD_EXTERNAL_CONFIG0_SD_PHY_GEN_RX_OFFSET;
		data |= 0x6 << SD_EXTERNAL_CONFIG0_SD_PHY_GEN_TX_OFFSET;
	} else if (sgmii_speed == COMPHY_SPEED_3_125G) {
		/* HS SGMII (2.5G), SerDes speed 3.125G */
		data |= 0x8 << SD_EXTERNAL_CONFIG0_SD_PHY_GEN_RX_OFFSET;
		data |= 0x8 << SD_EXTERNAL_CONFIG0_SD_PHY_GEN_TX_OFFSET;
	} else {
		/* Other rates are not supported */
		ERROR("unsupported SGMII speed on comphy%d\n", comphy_index);
		return -EINVAL;
	}

	mask |= SD_EXTERNAL_CONFIG0_SD_PU_RX_MASK;
	data |= 0 << SD_EXTERNAL_CONFIG0_SD_PU_RX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PU_TX_MASK;
	data |= 0 << SD_EXTERNAL_CONFIG0_SD_PU_TX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_HALF_BUS_MODE_MASK;
	data |= 1 << SD_EXTERNAL_CONFIG0_HALF_BUS_MODE_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG0_REG, data, mask);

	/* Set hard reset */
	mask = SD_EXTERNAL_CONFIG1_RESET_IN_MASK;
	data = 0x0 << SD_EXTERNAL_CONFIG1_RESET_IN_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RESET_CORE_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG1_RESET_CORE_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RF_RESET_IN_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG1_RF_RESET_IN_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	/* Release hard reset */
	mask = SD_EXTERNAL_CONFIG1_RESET_IN_MASK;
	data = 0x1 << SD_EXTERNAL_CONFIG1_RESET_IN_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RESET_CORE_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG1_RESET_CORE_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	/* Wait 1ms - until band gap and ref clock ready */
	mdelay(1);

	/* Make sure that 40 data bits is disabled
	 * This bit is not cleared by reset
	 */
	mask = COMMON_PHY_CFG6_IF_40_SEL_MASK;
	data = 0 << COMMON_PHY_CFG6_IF_40_SEL_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG6_REG, data, mask);

	/* Start comphy Configuration */
	debug("stage: Comphy configuration\n");
	/* set reference clock */
	mask = HPIPE_MISC_REFCLK_SEL_MASK;
	data = 0x0 << HPIPE_MISC_REFCLK_SEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_MISC_REG, data, mask);
	/* Power and PLL Control */
	mask = HPIPE_PWR_PLL_REF_FREQ_MASK;
	data = 0x1 << HPIPE_PWR_PLL_REF_FREQ_OFFSET;
	mask |= HPIPE_PWR_PLL_PHY_MODE_MASK;
	data |= 0x4 << HPIPE_PWR_PLL_PHY_MODE_OFFSET;
	reg_set(hpipe_addr + HPIPE_PWR_PLL_REG, data, mask);
	/* Loopback register */
	mask = HPIPE_LOOPBACK_SEL_MASK;
	data = 0x1 << HPIPE_LOOPBACK_SEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_LOOPBACK_REG, data, mask);
	/* rx control 1 */
	mask = HPIPE_RX_CONTROL_1_RXCLK2X_SEL_MASK;
	data = 0x1 << HPIPE_RX_CONTROL_1_RXCLK2X_SEL_OFFSET;
	mask |= HPIPE_RX_CONTROL_1_CLK8T_EN_MASK;
	data |= 0x0 << HPIPE_RX_CONTROL_1_CLK8T_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_RX_CONTROL_1_REG, data, mask);
	/* DTL Control */
	mask = HPIPE_PWR_CTR_DTL_FLOOP_EN_MASK;
	data = 0x0 << HPIPE_PWR_CTR_DTL_FLOOP_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_PWR_CTR_DTL_REG, data, mask);

	/* Set analog parameters from ETP(HW) - for now use the default data */
	debug("stage: Analog parameters from ETP(HW)\n");

	reg_set(hpipe_addr + HPIPE_G1_SET_0_REG,
		0x1 << HPIPE_G1_SET_0_G1_TX_EMPH1_OFFSET,
		HPIPE_G1_SET_0_G1_TX_EMPH1_MASK);

	debug("stage: RFU configurations- Power Up PLL,Tx,Rx\n");
	/* SERDES External Configuration */
	mask = SD_EXTERNAL_CONFIG0_SD_PU_PLL_MASK;
	data = 0x1 << SD_EXTERNAL_CONFIG0_SD_PU_PLL_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PU_RX_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG0_SD_PU_RX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PU_TX_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG0_SD_PU_TX_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG0_REG, data, mask);

	ret = mvebu_cp110_comphy_is_pll_locked(comphy_base, comphy_index);
	if (ret)
		return ret;

	/* RX init */
	mask = SD_EXTERNAL_CONFIG1_RX_INIT_MASK;
	data = 0x1 << SD_EXTERNAL_CONFIG1_RX_INIT_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	/* check that RX init done */
	addr = sd_ip_addr + SD_EXTERNAL_STATUS0_REG;
	data = SD_EXTERNAL_STATUS0_RX_INIT_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask, 100, REG_32BIT);
	if (data != 0) {
		ERROR("RX init failed\n");
		ret = -ETIMEDOUT;
	}

	debug("stage: RF Reset\n");
	/* RF Reset */
	mask = SD_EXTERNAL_CONFIG1_RX_INIT_MASK;
	data = 0x0 << SD_EXTERNAL_CONFIG1_RX_INIT_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RF_RESET_IN_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG1_RF_RESET_IN_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	debug_exit();

	return ret;
}

static int mvebu_cp110_comphy_xfi_power_on(uint64_t comphy_base,
					   uint8_t comphy_index,
					   uint32_t comphy_mode)
{
	uintptr_t hpipe_addr, sd_ip_addr, comphy_addr, addr;
	uint32_t mask, data, speed = COMPHY_GET_SPEED(comphy_mode);
	int ret = 0;
	uint8_t ap_nr, cp_nr;

	debug_enter();

	mvebu_cp110_get_ap_and_cp_nr(&ap_nr, &cp_nr, comphy_base);

	if (rx_trainng_done[ap_nr][cp_nr][comphy_index]) {
		debug("Skip %s for comphy[%d][%d][%d], due to rx training\n",
		       __func__, ap_nr, cp_nr, comphy_index);
		return 0;
	}

	const struct xfi_params *xfi_static_values =
			     &xfi_static_values_tab[ap_nr][cp_nr][comphy_index];

	debug("%s: the ap_nr = %d, cp_nr = %d, comphy_index %d\n",
	      __func__, ap_nr, cp_nr, comphy_index);

	debug("g1_ffe_cap_sel= 0x%x, g1_ffe_res_sel= 0x%x, g1_dfe_res= 0x%x\n",
	      xfi_static_values->g1_ffe_cap_sel,
	      xfi_static_values->g1_ffe_res_sel,
	      xfi_static_values->g1_dfe_res);

	if (!xfi_static_values->valid) {
		ERROR("[ap%d][cp[%d][comphy:%d]: Has no valid static params\n",
		      ap_nr, cp_nr, comphy_index);
		ERROR("[ap%d][cp[%d][comphy:%d]: porting layer needs update\n",
		      ap_nr, cp_nr, comphy_index);
		return -EINVAL;
	}

	if ((speed != COMPHY_SPEED_5_15625G) &&
	     (speed != COMPHY_SPEED_10_3125G) &&
	     (speed != COMPHY_SPEED_DEFAULT)) {
		ERROR("comphy:%d: unsupported sfi/xfi speed\n", comphy_index);
		return -EINVAL;
	}

	hpipe_addr = HPIPE_ADDR(COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base),
				comphy_index);
	sd_ip_addr = SD_ADDR(COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base),
			     comphy_index);
	comphy_addr = COMPHY_ADDR(comphy_base, comphy_index);

	/* configure phy selector for XFI/SFI */
	mvebu_cp110_comphy_set_phy_selector(comphy_base, comphy_index,
					    comphy_mode);

	debug("stage: RFU configurations - hard reset comphy\n");
	/* RFU configurations - hard reset comphy */
	mask = COMMON_PHY_CFG1_PWR_UP_MASK;
	data = 0x1 << COMMON_PHY_CFG1_PWR_UP_OFFSET;
	mask |= COMMON_PHY_CFG1_PIPE_SELECT_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_PIPE_SELECT_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG1_REG, data, mask);

	/* Make sure that 40 data bits is disabled
	 * This bit is not cleared by reset
	 */
	mask = COMMON_PHY_CFG6_IF_40_SEL_MASK;
	data = 0 << COMMON_PHY_CFG6_IF_40_SEL_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG6_REG, data, mask);

	/* Select Baud Rate of Comphy And PD_PLL/Tx/Rx */
	mask = SD_EXTERNAL_CONFIG0_SD_PU_PLL_MASK;
	data = 0x0 << SD_EXTERNAL_CONFIG0_SD_PU_PLL_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PHY_GEN_RX_MASK;
	data |= 0xE << SD_EXTERNAL_CONFIG0_SD_PHY_GEN_RX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PHY_GEN_TX_MASK;
	data |= 0xE << SD_EXTERNAL_CONFIG0_SD_PHY_GEN_TX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PU_RX_MASK;
	data |= 0 << SD_EXTERNAL_CONFIG0_SD_PU_RX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PU_TX_MASK;
	data |= 0 << SD_EXTERNAL_CONFIG0_SD_PU_TX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_HALF_BUS_MODE_MASK;
	data |= 0 << SD_EXTERNAL_CONFIG0_HALF_BUS_MODE_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG0_REG, data, mask);

	/* release from hard reset */
	mask = SD_EXTERNAL_CONFIG1_RESET_IN_MASK;
	data = 0x0 << SD_EXTERNAL_CONFIG1_RESET_IN_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RESET_CORE_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG1_RESET_CORE_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RF_RESET_IN_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG1_RF_RESET_IN_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	mask = SD_EXTERNAL_CONFIG1_RESET_IN_MASK;
	data = 0x1 << SD_EXTERNAL_CONFIG1_RESET_IN_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RESET_CORE_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG1_RESET_CORE_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_TX_IDLE_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG1_TX_IDLE_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	/* Wait 1ms - until band gap and ref clock ready */
	mdelay(1);

	/*
	 * Erratum IPCE_COMPHY-1353: toggle TX_IDLE bit in
	 * addition to the PHY reset
	 */
	mask = SD_EXTERNAL_CONFIG1_TX_IDLE_MASK;
	data = 0x0U;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	/* Start comphy Configuration */
	debug("stage: Comphy configuration\n");
	/* set reference clock */
	mask = HPIPE_MISC_ICP_FORCE_MASK;
	data = (speed == COMPHY_SPEED_5_15625G) ?
		(0x0 << HPIPE_MISC_ICP_FORCE_OFFSET) :
		(0x1 << HPIPE_MISC_ICP_FORCE_OFFSET);
	mask |= HPIPE_MISC_REFCLK_SEL_MASK;
	data |= 0x0 << HPIPE_MISC_REFCLK_SEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_MISC_REG, data, mask);
	/* Power and PLL Control */
	mask = HPIPE_PWR_PLL_REF_FREQ_MASK;
	data = 0x1 << HPIPE_PWR_PLL_REF_FREQ_OFFSET;
	mask |= HPIPE_PWR_PLL_PHY_MODE_MASK;
	data |= 0x4 << HPIPE_PWR_PLL_PHY_MODE_OFFSET;
	reg_set(hpipe_addr + HPIPE_PWR_PLL_REG, data, mask);
	/* Loopback register */
	mask = HPIPE_LOOPBACK_SEL_MASK;
	data = 0x1 << HPIPE_LOOPBACK_SEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_LOOPBACK_REG, data, mask);
	/* rx control 1 */
	mask = HPIPE_RX_CONTROL_1_RXCLK2X_SEL_MASK;
	data = 0x1 << HPIPE_RX_CONTROL_1_RXCLK2X_SEL_OFFSET;
	mask |= HPIPE_RX_CONTROL_1_CLK8T_EN_MASK;
	data |= 0x1 << HPIPE_RX_CONTROL_1_CLK8T_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_RX_CONTROL_1_REG, data, mask);
	/* DTL Control */
	mask = HPIPE_PWR_CTR_DTL_FLOOP_EN_MASK;
	data = 0x1 << HPIPE_PWR_CTR_DTL_FLOOP_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_PWR_CTR_DTL_REG, data, mask);

	/* Transmitter/Receiver Speed Divider Force */
	if (speed == COMPHY_SPEED_5_15625G) {
		mask = HPIPE_SPD_DIV_FORCE_RX_SPD_DIV_MASK;
		data = 1 << HPIPE_SPD_DIV_FORCE_RX_SPD_DIV_OFFSET;
		mask |= HPIPE_SPD_DIV_FORCE_RX_SPD_DIV_FORCE_MASK;
		data |= 1 << HPIPE_SPD_DIV_FORCE_RX_SPD_DIV_FORCE_OFFSET;
		mask |= HPIPE_SPD_DIV_FORCE_TX_SPD_DIV_MASK;
		data |= 1 << HPIPE_SPD_DIV_FORCE_TX_SPD_DIV_OFFSET;
		mask |= HPIPE_SPD_DIV_FORCE_TX_SPD_DIV_FORCE_MASK;
		data |= 1 << HPIPE_SPD_DIV_FORCE_TX_SPD_DIV_FORCE_OFFSET;
	} else {
		mask = HPIPE_TXDIGCK_DIV_FORCE_MASK;
		data = 0x1 << HPIPE_TXDIGCK_DIV_FORCE_OFFSET;
	}
	reg_set(hpipe_addr + HPIPE_SPD_DIV_FORCE_REG, data, mask);

	/* Set analog parameters from ETP(HW) */
	debug("stage: Analog parameters from ETP(HW)\n");
	/* SERDES External Configuration 2 */
	mask = SD_EXTERNAL_CONFIG2_PIN_DFE_EN_MASK;
	data = 0x1 << SD_EXTERNAL_CONFIG2_PIN_DFE_EN_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG2_REG, data, mask);
	/* 0x7-DFE Resolution control */
	mask = HPIPE_DFE_RES_FORCE_MASK;
	data = 0x1 << HPIPE_DFE_RES_FORCE_OFFSET;
	reg_set(hpipe_addr + HPIPE_DFE_REG0, data, mask);
	/* 0xd-G1_Setting_0 */
	if (speed == COMPHY_SPEED_5_15625G) {
		mask = HPIPE_G1_SET_0_G1_TX_EMPH1_MASK;
		data = 0x6 << HPIPE_G1_SET_0_G1_TX_EMPH1_OFFSET;
	} else {
		mask = HPIPE_G1_SET_0_G1_TX_AMP_MASK;
		data = xfi_static_values->g1_amp <<
				HPIPE_G1_SET_0_G1_TX_AMP_OFFSET;
		mask |= HPIPE_G1_SET_0_G1_TX_EMPH1_MASK;
		data |= xfi_static_values->g1_emph <<
				HPIPE_G1_SET_0_G1_TX_EMPH1_OFFSET;

		mask |= HPIPE_G1_SET_0_G1_TX_EMPH1_EN_MASK;
		data |= xfi_static_values->g1_emph_en <<
				HPIPE_G1_SET_0_G1_TX_EMPH1_EN_OFFSET;
		mask |= HPIPE_G1_SET_0_G1_TX_AMP_ADJ_MASK;
		data |= xfi_static_values->g1_tx_amp_adj <<
				HPIPE_G1_SET_0_G1_TX_AMP_ADJ_OFFSET;
	}
	reg_set(hpipe_addr + HPIPE_G1_SET_0_REG, data, mask);
	/* Genration 1 setting 2 (G1_Setting_2) */
	mask = HPIPE_G1_SET_2_G1_TX_EMPH0_MASK;
	data = xfi_static_values->g1_tx_emph <<
				HPIPE_G1_SET_2_G1_TX_EMPH0_OFFSET;
	mask |= HPIPE_G1_SET_2_G1_TX_EMPH0_EN_MASK;
	data |= xfi_static_values->g1_tx_emph_en <<
				HPIPE_G1_SET_2_G1_TX_EMPH0_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SET_2_REG, data, mask);
	/* Transmitter Slew Rate Control register (tx_reg1) */
	mask = HPIPE_TX_REG1_TX_EMPH_RES_MASK;
	data = 0x3 << HPIPE_TX_REG1_TX_EMPH_RES_OFFSET;
	mask |= HPIPE_TX_REG1_SLC_EN_MASK;
	data |= 0x3f << HPIPE_TX_REG1_SLC_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_REG1_REG, data, mask);
	/* Impedance Calibration Control register (cal_reg1) */
	mask = HPIPE_CAL_REG_1_EXT_TXIMP_MASK;
	data = 0xe << HPIPE_CAL_REG_1_EXT_TXIMP_OFFSET;
	mask |= HPIPE_CAL_REG_1_EXT_TXIMP_EN_MASK;
	data |= 0x1 << HPIPE_CAL_REG_1_EXT_TXIMP_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_CAL_REG1_REG, data, mask);
	/* Generation 1 Setting 5 (g1_setting_5) */
	mask = HPIPE_G1_SETTING_5_G1_ICP_MASK;
	data = 0 << HPIPE_CAL_REG_1_EXT_TXIMP_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SETTING_5_REG, data, mask);

	/* 0xE-G1_Setting_1 */
	mask = HPIPE_G1_SET_1_G1_RX_DFE_EN_MASK;
	data = 0x1 << HPIPE_G1_SET_1_G1_RX_DFE_EN_OFFSET;
	if (speed == COMPHY_SPEED_5_15625G) {
		mask |= HPIPE_G1_SET_1_G1_RX_SELMUPI_MASK;
		data |= 0x1 << HPIPE_G1_SET_1_G1_RX_SELMUPI_OFFSET;
		mask |= HPIPE_G1_SET_1_G1_RX_SELMUPF_MASK;
		data |= 0x1 << HPIPE_G1_SET_1_G1_RX_SELMUPF_OFFSET;
	} else {
		mask |= HPIPE_G1_SET_1_G1_RX_SELMUPI_MASK;
		data |= xfi_static_values->g1_rx_selmupi <<
				HPIPE_G1_SET_1_G1_RX_SELMUPI_OFFSET;
		mask |= HPIPE_G1_SET_1_G1_RX_SELMUPF_MASK;
		data |= xfi_static_values->g1_rx_selmupf <<
				HPIPE_G1_SET_1_G1_RX_SELMUPF_OFFSET;
		mask |= HPIPE_G1_SET_1_G1_RX_SELMUFI_MASK;
		data |= xfi_static_values->g1_rx_selmufi <<
				HPIPE_G1_SET_1_G1_RX_SELMUFI_OFFSET;
		mask |= HPIPE_G1_SET_1_G1_RX_SELMUFF_MASK;
		data |= xfi_static_values->g1_rx_selmuff <<
				HPIPE_G1_SET_1_G1_RX_SELMUFF_OFFSET;
		mask |= HPIPE_G1_SET_1_G1_RX_DIGCK_DIV_MASK;
		data |= 0x3 << HPIPE_G1_SET_1_G1_RX_DIGCK_DIV_OFFSET;
	}
	reg_set(hpipe_addr + HPIPE_G1_SET_1_REG, data, mask);

	/* 0xA-DFE_Reg3 */
	mask = HPIPE_DFE_F3_F5_DFE_EN_MASK;
	data = 0x0 << HPIPE_DFE_F3_F5_DFE_EN_OFFSET;
	mask |= HPIPE_DFE_F3_F5_DFE_CTRL_MASK;
	data |= 0x0 << HPIPE_DFE_F3_F5_DFE_CTRL_OFFSET;
	reg_set(hpipe_addr + HPIPE_DFE_F3_F5_REG, data, mask);

	/* 0x111-G1_Setting_4 */
	mask = HPIPE_G1_SETTINGS_4_G1_DFE_RES_MASK;
	data = 0x1 << HPIPE_G1_SETTINGS_4_G1_DFE_RES_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SETTINGS_4_REG, data, mask);
	/* Genration 1 setting 3 (G1_Setting_3) */
	mask = HPIPE_G1_SETTINGS_3_G1_FBCK_SEL_MASK;
	data = 0x1 << HPIPE_G1_SETTINGS_3_G1_FBCK_SEL_OFFSET;
	if (speed == COMPHY_SPEED_5_15625G) {
		/* Force FFE (Feed Forward Equalization) to 5G */
		mask |= HPIPE_G1_SETTINGS_3_G1_FFE_CAP_SEL_MASK;
		data |= 0xf << HPIPE_G1_SETTINGS_3_G1_FFE_CAP_SEL_OFFSET;
		mask |= HPIPE_G1_SETTINGS_3_G1_FFE_RES_SEL_MASK;
		data |= 0x4 << HPIPE_G1_SETTINGS_3_G1_FFE_RES_SEL_OFFSET;
		mask |= HPIPE_G1_SETTINGS_3_G1_FFE_SETTING_FORCE_MASK;
		data |= 0x1 << HPIPE_G1_SETTINGS_3_G1_FFE_SETTING_FORCE_OFFSET;
		reg_set(hpipe_addr + HPIPE_G1_SETTINGS_3_REG, data, mask);
	} else {
		mask |= HPIPE_G1_SETTINGS_3_G1_FFE_CAP_SEL_MASK;
		data |= xfi_static_values->g1_ffe_cap_sel <<
			HPIPE_G1_SETTINGS_3_G1_FFE_CAP_SEL_OFFSET;
		mask |= HPIPE_G1_SETTINGS_3_G1_FFE_RES_SEL_MASK;
		data |= xfi_static_values->g1_ffe_res_sel <<
			HPIPE_G1_SETTINGS_3_G1_FFE_RES_SEL_OFFSET;
		mask |= HPIPE_G1_SETTINGS_3_G1_FFE_SETTING_FORCE_MASK;
		data |= 0x1 << HPIPE_G1_SETTINGS_3_G1_FFE_SETTING_FORCE_OFFSET;
		reg_set(hpipe_addr + HPIPE_G1_SETTINGS_3_REG, data, mask);

		/* Use the value from CAL_OS_PH_EXT */
		mask = HPIPE_CAL_RXCLKALIGN_90_EXT_EN_MASK;
		data = 1 << HPIPE_CAL_RXCLKALIGN_90_EXT_EN_OFFSET;
		reg_set(hpipe_addr +
			HPIPE_RX_CLK_ALIGN90_AND_TX_IDLE_CALIB_CTRL_REG,
			data, mask);

		/* Update align90 */
		mask = HPIPE_CAL_OS_PH_EXT_MASK;
		data = xfi_static_values->align90 << HPIPE_CAL_OS_PH_EXT_OFFSET;
		reg_set(hpipe_addr +
			HPIPE_RX_CLK_ALIGN90_AND_TX_IDLE_CALIB_CTRL_REG,
			data, mask);

		/* Force DFE resolution (use gen table value) */
		mask = HPIPE_DFE_RES_FORCE_MASK;
		data = 0x0 << HPIPE_DFE_RES_FORCE_OFFSET;
		reg_set(hpipe_addr + HPIPE_DFE_REG0, data, mask);

		/* 0x111-G1 DFE_Setting_4 */
		mask = HPIPE_G1_SETTINGS_4_G1_DFE_RES_MASK;
		data = xfi_static_values->g1_dfe_res <<
			HPIPE_G1_SETTINGS_4_G1_DFE_RES_OFFSET;
		reg_set(hpipe_addr + HPIPE_G1_SETTINGS_4_REG, data, mask);
	}

	/* Connfigure RX training timer */
	mask = HPIPE_RX_TRAIN_TIMER_MASK;
	data = 0x13 << HPIPE_RX_TRAIN_TIMER_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_CTRL_5_REG, data, mask);

	/* Enable TX train peak to peak hold */
	mask = HPIPE_TX_TRAIN_P2P_HOLD_MASK;
	data = 0x1 << HPIPE_TX_TRAIN_P2P_HOLD_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_CTRL_0_REG, data, mask);

	/* Configure TX preset index */
	mask = HPIPE_TX_PRESET_INDEX_MASK;
	data = 0x2 << HPIPE_TX_PRESET_INDEX_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_PRESET_INDEX_REG, data, mask);

	/* Disable pattern lock lost timeout */
	mask = HPIPE_PATTERN_LOCK_LOST_TIMEOUT_EN_MASK;
	data = 0x0 << HPIPE_PATTERN_LOCK_LOST_TIMEOUT_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_FRAME_DETECT_CTRL_3_REG, data, mask);

	/* Configure TX training pattern and TX training 16bit auto */
	mask = HPIPE_TX_TRAIN_16BIT_AUTO_EN_MASK;
	data = 0x1 << HPIPE_TX_TRAIN_16BIT_AUTO_EN_OFFSET;
	mask |= HPIPE_TX_TRAIN_PAT_SEL_MASK;
	data |= 0x1 << HPIPE_TX_TRAIN_PAT_SEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_REG, data, mask);

	/* Configure Training patten number */
	mask = HPIPE_TRAIN_PAT_NUM_MASK;
	data = 0x88 << HPIPE_TRAIN_PAT_NUM_OFFSET;
	reg_set(hpipe_addr + HPIPE_FRAME_DETECT_CTRL_0_REG, data, mask);

	/* Configure differencial manchester encoter to ethernet mode */
	mask = HPIPE_DME_ETHERNET_MODE_MASK;
	data = 0x1 << HPIPE_DME_ETHERNET_MODE_OFFSET;
	reg_set(hpipe_addr + HPIPE_DME_REG, data, mask);

	/* Configure VDD Continuous Calibration */
	mask = HPIPE_CAL_VDD_CONT_MODE_MASK;
	data = 0x1 << HPIPE_CAL_VDD_CONT_MODE_OFFSET;
	reg_set(hpipe_addr + HPIPE_VDD_CAL_0_REG, data, mask);

	/* Trigger sampler enable pulse (by toggleing the bit) */
	mask = HPIPE_RX_SAMPLER_OS_GAIN_MASK;
	data = 0x3 << HPIPE_RX_SAMPLER_OS_GAIN_OFFSET;
	mask |= HPIPE_SMAPLER_MASK;
	data |= 0x1 << HPIPE_SMAPLER_OFFSET;
	reg_set(hpipe_addr + HPIPE_SAMPLER_N_PROC_CALIB_CTRL_REG, data, mask);
	mask = HPIPE_SMAPLER_MASK;
	data = 0x0 << HPIPE_SMAPLER_OFFSET;
	reg_set(hpipe_addr + HPIPE_SAMPLER_N_PROC_CALIB_CTRL_REG, data, mask);

	/* Set External RX Regulator Control */
	mask = HPIPE_EXT_SELLV_RXSAMPL_MASK;
	data = 0x1A << HPIPE_EXT_SELLV_RXSAMPL_OFFSET;
	reg_set(hpipe_addr + HPIPE_VDD_CAL_CTRL_REG, data, mask);

	debug("stage: RFU configurations- Power Up PLL,Tx,Rx\n");
	/* SERDES External Configuration */
	mask = SD_EXTERNAL_CONFIG0_SD_PU_PLL_MASK;
	data = 0x1 << SD_EXTERNAL_CONFIG0_SD_PU_PLL_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PU_RX_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG0_SD_PU_RX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PU_TX_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG0_SD_PU_TX_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG0_REG, data, mask);

	/* check PLL rx & tx ready */
	addr = sd_ip_addr + SD_EXTERNAL_STATUS0_REG;
	data = SD_EXTERNAL_STATUS0_PLL_RX_MASK |
	       SD_EXTERNAL_STATUS0_PLL_TX_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask,
				    PLL_LOCK_TIMEOUT, REG_32BIT);
	if (data != 0) {
		if (data & SD_EXTERNAL_STATUS0_PLL_RX_MASK)
			ERROR("RX PLL is not locked\n");
		if (data & SD_EXTERNAL_STATUS0_PLL_TX_MASK)
			ERROR("TX PLL is not locked\n");

		ret = -ETIMEDOUT;
	}

	/* RX init */
	mask = SD_EXTERNAL_CONFIG1_RX_INIT_MASK;
	data = 0x1 << SD_EXTERNAL_CONFIG1_RX_INIT_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	/* check that RX init done */
	addr = sd_ip_addr + SD_EXTERNAL_STATUS0_REG;
	data = SD_EXTERNAL_STATUS0_RX_INIT_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask, 100, REG_32BIT);
	if (data != 0) {
		ERROR("RX init failed\n");
		ret = -ETIMEDOUT;
	}

	debug("stage: RF Reset\n");
	/* RF Reset */
	mask =  SD_EXTERNAL_CONFIG1_RX_INIT_MASK;
	data = 0x0 << SD_EXTERNAL_CONFIG1_RX_INIT_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RF_RESET_IN_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG1_RF_RESET_IN_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	debug_exit();

	return ret;
}

static int mvebu_cp110_comphy_pcie_power_on(uint64_t comphy_base,
				     uint8_t comphy_index, uint32_t comphy_mode)
{
	int ret = 0;
	uint32_t reg, mask, data, pcie_width;
	uint32_t clk_dir;
	uintptr_t hpipe_addr, comphy_addr, addr;
	_Bool clk_src = COMPHY_GET_CLK_SRC(comphy_mode);
	_Bool called_from_uboot = COMPHY_GET_CALLER(comphy_mode);

	/* In Armada 8K DB boards, PCIe initialization can be executed
	 * only once (PCIe reset performed during chip power on and
	 * it cannot be executed via GPIO later).
	 * This means that power on can be executed only once, so let's
	 * mark if the caller is bootloader or Linux.
	 * If bootloader -> run power on.
	 * If Linux -> exit.
	 *
	 * TODO: In MacciatoBIN, PCIe reset is connected via GPIO,
	 * so after GPIO reset is added to Linux Kernel, it can be
	 * powered-on by Linux.
	 */
	if (!called_from_uboot)
		return ret;

	hpipe_addr = HPIPE_ADDR(COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base),
				comphy_index);
	comphy_addr = COMPHY_ADDR(comphy_base, comphy_index);
	pcie_width = COMPHY_GET_PCIE_WIDTH(comphy_mode);

	debug_enter();

	spin_lock(&cp110_mac_reset_lock);

	reg = mmio_read_32(SYS_CTRL_FROM_COMPHY_ADDR(comphy_base) +
						SYS_CTRL_UINIT_SOFT_RESET_REG);
	switch (comphy_index) {
	case COMPHY_LANE0:
		reg |= PCIE_MAC_RESET_MASK_PORT0;
		break;
	case COMPHY_LANE4:
		reg |= PCIE_MAC_RESET_MASK_PORT1;
		break;
	case COMPHY_LANE5:
		reg |= PCIE_MAC_RESET_MASK_PORT2;
		break;
	}

	mmio_write_32(SYS_CTRL_FROM_COMPHY_ADDR(comphy_base) +
					    SYS_CTRL_UINIT_SOFT_RESET_REG, reg);
	spin_unlock(&cp110_mac_reset_lock);

	/* Configure PIPE selector for PCIE */
	mvebu_cp110_comphy_set_pipe_selector(comphy_base, comphy_index,
					     comphy_mode);

	/*
	 * Read SAR (Sample-At-Reset) configuration for the PCIe clock
	 * direction.
	 *
	 * SerDes Lane 4/5 got the PCIe ref-clock #1,
	 * and SerDes Lane 0 got PCIe ref-clock #0
	 */
	reg = mmio_read_32(DFX_FROM_COMPHY_ADDR(comphy_base) +
			   SAR_STATUS_0_REG);
	if (comphy_index == COMPHY_LANE4 || comphy_index == COMPHY_LANE5)
		clk_dir = (reg & SAR_RST_PCIE1_CLOCK_CONFIG_CP1_MASK) >>
					  SAR_RST_PCIE1_CLOCK_CONFIG_CP1_OFFSET;
	else
		clk_dir = (reg & SAR_RST_PCIE0_CLOCK_CONFIG_CP1_MASK) >>
					  SAR_RST_PCIE0_CLOCK_CONFIG_CP1_OFFSET;

	debug("On lane %d\n", comphy_index);
	debug("PCIe clock direction = %x\n", clk_dir);
	debug("PCIe Width = %d\n", pcie_width);

	/* enable PCIe X4 and X2 */
	if (comphy_index == COMPHY_LANE0) {
		if (pcie_width == PCIE_LNK_X4) {
			data = 0x1 << COMMON_PHY_SD_CTRL1_PCIE_X4_EN_OFFSET;
			mask = COMMON_PHY_SD_CTRL1_PCIE_X4_EN_MASK;
			reg_set(comphy_base + COMMON_PHY_SD_CTRL1,
				data, mask);
		} else if (pcie_width == PCIE_LNK_X2) {
			data = 0x1 << COMMON_PHY_SD_CTRL1_PCIE_X2_EN_OFFSET;
			mask = COMMON_PHY_SD_CTRL1_PCIE_X2_EN_MASK;
			reg_set(comphy_base + COMMON_PHY_SD_CTRL1, data, mask);
		}
	}

	/* If PCIe clock is output and clock source from SerDes lane 5,
	 * need to configure the clock-source MUX.
	 * By default, the clock source is from lane 4
	 */
	if (clk_dir && clk_src && (comphy_index == COMPHY_LANE5)) {
		data = DFX_DEV_GEN_PCIE_CLK_SRC_MUX <<
						DFX_DEV_GEN_PCIE_CLK_SRC_OFFSET;
		mask = DFX_DEV_GEN_PCIE_CLK_SRC_MASK;
		reg_set(DFX_FROM_COMPHY_ADDR(comphy_base) +
			DFX_DEV_GEN_CTRL12_REG, data, mask);
	}

	debug("stage: RFU configurations - hard reset comphy\n");
	/* RFU configurations - hard reset comphy */
	mask = COMMON_PHY_CFG1_PWR_UP_MASK;
	data = 0x1 << COMMON_PHY_CFG1_PWR_UP_OFFSET;
	mask |= COMMON_PHY_CFG1_PIPE_SELECT_MASK;
	data |= 0x1 << COMMON_PHY_CFG1_PIPE_SELECT_OFFSET;
	mask |= COMMON_PHY_CFG1_PWR_ON_RESET_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_PWR_ON_RESET_OFFSET;
	mask |= COMMON_PHY_CFG1_CORE_RSTN_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_CORE_RSTN_OFFSET;
	mask |= COMMON_PHY_PHY_MODE_MASK;
	data |= 0x0 << COMMON_PHY_PHY_MODE_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG1_REG, data, mask);

	/* release from hard reset */
	mask = COMMON_PHY_CFG1_PWR_ON_RESET_MASK;
	data = 0x1 << COMMON_PHY_CFG1_PWR_ON_RESET_OFFSET;
	mask |= COMMON_PHY_CFG1_CORE_RSTN_MASK;
	data |= 0x1 << COMMON_PHY_CFG1_CORE_RSTN_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG1_REG, data, mask);

	/* Wait 1ms - until band gap and ref clock ready */
	mdelay(1);
	/* Start comphy Configuration */
	debug("stage: Comphy configuration\n");
	/* Set PIPE soft reset */
	mask = HPIPE_RST_CLK_CTRL_PIPE_RST_MASK;
	data = 0x1 << HPIPE_RST_CLK_CTRL_PIPE_RST_OFFSET;
	/* Set PHY datapath width mode for V0 */
	mask |= HPIPE_RST_CLK_CTRL_FIXED_PCLK_MASK;
	data |= 0x1 << HPIPE_RST_CLK_CTRL_FIXED_PCLK_OFFSET;
	/* Set Data bus width USB mode for V0 */
	mask |= HPIPE_RST_CLK_CTRL_PIPE_WIDTH_MASK;
	data |= 0x0 << HPIPE_RST_CLK_CTRL_PIPE_WIDTH_OFFSET;
	/* Set CORE_CLK output frequency for 250Mhz */
	mask |= HPIPE_RST_CLK_CTRL_CORE_FREQ_SEL_MASK;
	data |= 0x0 << HPIPE_RST_CLK_CTRL_CORE_FREQ_SEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_RST_CLK_CTRL_REG, data, mask);
	/* Set PLL ready delay for 0x2 */
	data = 0x2 << HPIPE_CLK_SRC_LO_PLL_RDY_DL_OFFSET;
	mask = HPIPE_CLK_SRC_LO_PLL_RDY_DL_MASK;
	if (pcie_width != PCIE_LNK_X1) {
		data |= 0x1 << HPIPE_CLK_SRC_LO_BUNDLE_PERIOD_SEL_OFFSET;
		mask |= HPIPE_CLK_SRC_LO_BUNDLE_PERIOD_SEL_MASK;
		data |= 0x1 << HPIPE_CLK_SRC_LO_BUNDLE_PERIOD_SCALE_OFFSET;
		mask |= HPIPE_CLK_SRC_LO_BUNDLE_PERIOD_SCALE_MASK;
	}
	reg_set(hpipe_addr + HPIPE_CLK_SRC_LO_REG, data, mask);

	/* Set PIPE mode interface to PCIe3 - 0x1  & set lane order */
	data = 0x1 << HPIPE_CLK_SRC_HI_MODE_PIPE_OFFSET;
	mask = HPIPE_CLK_SRC_HI_MODE_PIPE_MASK;
	if (pcie_width != PCIE_LNK_X1) {
		mask |= HPIPE_CLK_SRC_HI_LANE_STRT_MASK;
		mask |= HPIPE_CLK_SRC_HI_LANE_MASTER_MASK;
		mask |= HPIPE_CLK_SRC_HI_LANE_BREAK_MASK;
		if (comphy_index == 0) {
			data |= 0x1 << HPIPE_CLK_SRC_HI_LANE_STRT_OFFSET;
			data |= 0x1 << HPIPE_CLK_SRC_HI_LANE_MASTER_OFFSET;
		} else if (comphy_index == (pcie_width - 1)) {
			data |= 0x1 << HPIPE_CLK_SRC_HI_LANE_BREAK_OFFSET;
		}
	}
	reg_set(hpipe_addr + HPIPE_CLK_SRC_HI_REG, data, mask);
	/* Config update polarity equalization */
	data = 0x1 << HPIPE_CFG_UPDATE_POLARITY_OFFSET;
	mask = HPIPE_CFG_UPDATE_POLARITY_MASK;
	reg_set(hpipe_addr + HPIPE_LANE_EQ_CFG1_REG, data, mask);
	/* Set PIPE version 4 to mode enable */
	data = 0x1 << HPIPE_DFE_CTRL_28_PIPE4_OFFSET;
	mask = HPIPE_DFE_CTRL_28_PIPE4_MASK;
	reg_set(hpipe_addr + HPIPE_DFE_CTRL_28_REG, data, mask);
	/* TODO: check if pcie clock is output/input - for bringup use input*/
	/* Enable PIN clock 100M_125M */
	mask = 0;
	data = 0;
	/* Only if clock is output, configure the clock-source mux */
	if (clk_dir) {
		mask |= HPIPE_MISC_CLK100M_125M_MASK;
		data |= 0x1 << HPIPE_MISC_CLK100M_125M_OFFSET;
	}
	/* Set PIN_TXDCLK_2X Clock Freq. Selection for outputs 500MHz clock */
	mask |= HPIPE_MISC_TXDCLK_2X_MASK;
	data |= 0x0 << HPIPE_MISC_TXDCLK_2X_OFFSET;
	/* Enable 500MHz Clock */
	mask |= HPIPE_MISC_CLK500_EN_MASK;
	data |= 0x1 << HPIPE_MISC_CLK500_EN_OFFSET;
	if (clk_dir) { /* output */
		/* Set reference clock comes from group 1 */
		mask |= HPIPE_MISC_REFCLK_SEL_MASK;
		data |= 0x0 << HPIPE_MISC_REFCLK_SEL_OFFSET;
	} else {
		/* Set reference clock comes from group 2 */
		mask |= HPIPE_MISC_REFCLK_SEL_MASK;
		data |= 0x1 << HPIPE_MISC_REFCLK_SEL_OFFSET;
	}
	mask |= HPIPE_MISC_ICP_FORCE_MASK;
	data |= 0x1 << HPIPE_MISC_ICP_FORCE_OFFSET;
	reg_set(hpipe_addr + HPIPE_MISC_REG, data, mask);
	if (clk_dir) { /* output */
		/* Set reference frequcency select - 0x2 for 25MHz*/
		mask = HPIPE_PWR_PLL_REF_FREQ_MASK;
		data = 0x2 << HPIPE_PWR_PLL_REF_FREQ_OFFSET;
	} else {
		/* Set reference frequcency select - 0x0 for 100MHz*/
		mask = HPIPE_PWR_PLL_REF_FREQ_MASK;
		data = 0x0 << HPIPE_PWR_PLL_REF_FREQ_OFFSET;
	}
	/* Set PHY mode to PCIe */
	mask |= HPIPE_PWR_PLL_PHY_MODE_MASK;
	data |= 0x3 << HPIPE_PWR_PLL_PHY_MODE_OFFSET;
	reg_set(hpipe_addr + HPIPE_PWR_PLL_REG, data, mask);

	/* ref clock alignment */
	if (pcie_width != PCIE_LNK_X1) {
		mask = HPIPE_LANE_ALIGN_OFF_MASK;
		data = 0x0 << HPIPE_LANE_ALIGN_OFF_OFFSET;
		reg_set(hpipe_addr + HPIPE_LANE_ALIGN_REG, data, mask);
	}

	/* Set the amount of time spent in the LoZ state - set for 0x7 only if
	 * the PCIe clock is output
	 */
	if (clk_dir)
		reg_set(hpipe_addr + HPIPE_GLOBAL_PM_CTRL,
			0x7 << HPIPE_GLOBAL_PM_RXDLOZ_WAIT_OFFSET,
			HPIPE_GLOBAL_PM_RXDLOZ_WAIT_MASK);

	/* Set Maximal PHY Generation Setting(8Gbps) */
	mask = HPIPE_INTERFACE_GEN_MAX_MASK;
	data = 0x2 << HPIPE_INTERFACE_GEN_MAX_OFFSET;
	/* Bypass frame detection and sync detection for RX DATA */
	mask |= HPIPE_INTERFACE_DET_BYPASS_MASK;
	data |= 0x1 << HPIPE_INTERFACE_DET_BYPASS_OFFSET;
	/* Set Link Train Mode (Tx training control pins are used) */
	mask |= HPIPE_INTERFACE_LINK_TRAIN_MASK;
	data |= 0x1 << HPIPE_INTERFACE_LINK_TRAIN_OFFSET;
	reg_set(hpipe_addr + HPIPE_INTERFACE_REG, data, mask);

	/* Set Idle_sync enable */
	mask = HPIPE_PCIE_IDLE_SYNC_MASK;
	data = 0x1 << HPIPE_PCIE_IDLE_SYNC_OFFSET;
	/* Select bits for PCIE Gen3(32bit) */
	mask |= HPIPE_PCIE_SEL_BITS_MASK;
	data |= 0x2 << HPIPE_PCIE_SEL_BITS_OFFSET;
	reg_set(hpipe_addr + HPIPE_PCIE_REG0, data, mask);

	/* Enable Tx_adapt_g1 */
	mask = HPIPE_TX_TRAIN_CTRL_G1_MASK;
	data = 0x1 << HPIPE_TX_TRAIN_CTRL_G1_OFFSET;
	/* Enable Tx_adapt_gn1 */
	mask |= HPIPE_TX_TRAIN_CTRL_GN1_MASK;
	data |= 0x1 << HPIPE_TX_TRAIN_CTRL_GN1_OFFSET;
	/* Disable Tx_adapt_g0 */
	mask |= HPIPE_TX_TRAIN_CTRL_G0_MASK;
	data |= 0x0 << HPIPE_TX_TRAIN_CTRL_G0_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_CTRL_REG, data, mask);

	/* Set reg_tx_train_chk_init */
	mask = HPIPE_TX_TRAIN_CHK_INIT_MASK;
	data = 0x0 << HPIPE_TX_TRAIN_CHK_INIT_OFFSET;
	/* Enable TX_COE_FM_PIN_PCIE3_EN */
	mask |= HPIPE_TX_TRAIN_COE_FM_PIN_PCIE3_MASK;
	data |= 0x1 << HPIPE_TX_TRAIN_COE_FM_PIN_PCIE3_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_REG, data, mask);

	debug("stage: TRx training parameters\n");
	/* Set Preset sweep configurations */
	mask = HPIPE_TX_TX_STATUS_CHECK_MODE_MASK;
	data = 0x1 << HPIPE_TX_STATUS_CHECK_MODE_OFFSET;
	mask |= HPIPE_TX_NUM_OF_PRESET_MASK;
	data |= 0x7 << HPIPE_TX_NUM_OF_PRESET_OFFSET;
	mask |= HPIPE_TX_SWEEP_PRESET_EN_MASK;
	data |= 0x1 << HPIPE_TX_SWEEP_PRESET_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_CTRL_11_REG, data, mask);

	/* Tx train start configuration */
	mask = HPIPE_TX_TRAIN_START_SQ_EN_MASK;
	data = 0x1 << HPIPE_TX_TRAIN_START_SQ_EN_OFFSET;
	mask |= HPIPE_TX_TRAIN_START_FRM_DET_EN_MASK;
	data |= 0x0 << HPIPE_TX_TRAIN_START_FRM_DET_EN_OFFSET;
	mask |= HPIPE_TX_TRAIN_START_FRM_LOCK_EN_MASK;
	data |= 0x0 << HPIPE_TX_TRAIN_START_FRM_LOCK_EN_OFFSET;
	mask |= HPIPE_TX_TRAIN_WAIT_TIME_EN_MASK;
	data |= 0x1 << HPIPE_TX_TRAIN_WAIT_TIME_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_CTRL_5_REG, data, mask);

	/* Enable Tx train P2P */
	mask = HPIPE_TX_TRAIN_P2P_HOLD_MASK;
	data = 0x1 << HPIPE_TX_TRAIN_P2P_HOLD_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_CTRL_0_REG, data, mask);

	/* Configure Tx train timeout */
	mask = HPIPE_TRX_TRAIN_TIMER_MASK;
	data = 0x17 << HPIPE_TRX_TRAIN_TIMER_OFFSET;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_CTRL_4_REG, data, mask);

	/* Disable G0/G1/GN1 adaptation */
	mask = HPIPE_TX_TRAIN_CTRL_G1_MASK | HPIPE_TX_TRAIN_CTRL_GN1_MASK
		| HPIPE_TX_TRAIN_CTRL_G0_OFFSET;
	data = 0;
	reg_set(hpipe_addr + HPIPE_TX_TRAIN_CTRL_REG, data, mask);

	/* Disable DTL frequency loop */
	mask = HPIPE_PWR_CTR_DTL_FLOOP_EN_MASK;
	data = 0x0 << HPIPE_PWR_CTR_DTL_FLOOP_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_PWR_CTR_DTL_REG, data, mask);

	/* Configure G3 DFE */
	mask = HPIPE_G3_DFE_RES_MASK;
	data = 0x3 << HPIPE_G3_DFE_RES_OFFSET;
	reg_set(hpipe_addr + HPIPE_G3_SETTING_4_REG, data, mask);

	/* Use TX/RX training result for DFE */
	mask = HPIPE_DFE_RES_FORCE_MASK;
	data = 0x0 << HPIPE_DFE_RES_FORCE_OFFSET;
	reg_set(hpipe_addr + HPIPE_DFE_REG0,  data, mask);

	/* Configure initial and final coefficient value for receiver */
	mask = HPIPE_G3_SET_1_G3_RX_SELMUPI_MASK;
	data = 0x1 << HPIPE_G3_SET_1_G3_RX_SELMUPI_OFFSET;

	mask |= HPIPE_G3_SET_1_G3_RX_SELMUPF_MASK;
	data |= 0x1 << HPIPE_G3_SET_1_G3_RX_SELMUPF_OFFSET;

	mask |= HPIPE_G3_SET_1_G3_SAMPLER_INPAIRX2_EN_MASK;
	data |= 0x0 << HPIPE_G3_SET_1_G3_SAMPLER_INPAIRX2_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_G3_SET_1_REG,  data, mask);

	/* Trigger sampler enable pulse */
	mask = HPIPE_SMAPLER_MASK;
	data = 0x1 << HPIPE_SMAPLER_OFFSET;
	reg_set(hpipe_addr + HPIPE_SAMPLER_N_PROC_CALIB_CTRL_REG, data, mask);
	udelay(5);
	reg_set(hpipe_addr + HPIPE_SAMPLER_N_PROC_CALIB_CTRL_REG, 0, mask);

	/* FFE resistor tuning for different bandwidth  */
	mask = HPIPE_G3_FFE_DEG_RES_LEVEL_MASK;
	data = 0x1 << HPIPE_G3_FFE_DEG_RES_LEVEL_OFFSET;
	mask |= HPIPE_G3_FFE_LOAD_RES_LEVEL_MASK;
	data |= 0x3 << HPIPE_G3_FFE_LOAD_RES_LEVEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_G3_SETTING_3_REG, data, mask);

	/* Pattern lock lost timeout disable */
	mask = HPIPE_PATTERN_LOCK_LOST_TIMEOUT_EN_MASK;
	data = 0x0 << HPIPE_PATTERN_LOCK_LOST_TIMEOUT_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_FRAME_DETECT_CTRL_3_REG, data, mask);

	/* Configure DFE adaptations */
	mask = HPIPE_CDR_RX_MAX_DFE_ADAPT_0_MASK;
	data = 0x0 << HPIPE_CDR_RX_MAX_DFE_ADAPT_0_OFFSET;
	mask |= HPIPE_CDR_RX_MAX_DFE_ADAPT_1_MASK;
	data |= 0x0 << HPIPE_CDR_RX_MAX_DFE_ADAPT_1_OFFSET;
	mask |= HPIPE_CDR_MAX_DFE_ADAPT_0_MASK;
	data |= 0x0 << HPIPE_CDR_MAX_DFE_ADAPT_0_OFFSET;
	mask |= HPIPE_CDR_MAX_DFE_ADAPT_1_MASK;
	data |= 0x1 << HPIPE_CDR_MAX_DFE_ADAPT_1_OFFSET;
	reg_set(hpipe_addr + HPIPE_CDR_CONTROL_REG, data, mask);

	mask = HPIPE_DFE_TX_MAX_DFE_ADAPT_MASK;
	data = 0x0 << HPIPE_DFE_TX_MAX_DFE_ADAPT_OFFSET;
	reg_set(hpipe_addr + HPIPE_DFE_CONTROL_REG, data, mask);

	/* Genration 2 setting 1*/
	mask = HPIPE_G2_SET_1_G2_RX_SELMUPI_MASK;
	data = 0x0 << HPIPE_G2_SET_1_G2_RX_SELMUPI_OFFSET;
	mask |= HPIPE_G2_SET_1_G2_RX_SELMUPF_MASK;
	data |= 0x1 << HPIPE_G2_SET_1_G2_RX_SELMUPF_OFFSET;
	mask |= HPIPE_G2_SET_1_G2_RX_SELMUFI_MASK;
	data |= 0x0 << HPIPE_G2_SET_1_G2_RX_SELMUFI_OFFSET;
	reg_set(hpipe_addr + HPIPE_G2_SET_1_REG, data, mask);

	/* DFE enable */
	mask = HPIPE_G2_DFE_RES_MASK;
	data = 0x3 << HPIPE_G2_DFE_RES_OFFSET;
	reg_set(hpipe_addr + HPIPE_G2_SETTINGS_4_REG, data, mask);

	/* Configure DFE Resolution */
	mask = HPIPE_LANE_CFG4_DFE_EN_SEL_MASK;
	data = 0x1 << HPIPE_LANE_CFG4_DFE_EN_SEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_CFG4_REG, data, mask);

	/* VDD calibration control */
	mask = HPIPE_EXT_SELLV_RXSAMPL_MASK;
	data = 0x16 << HPIPE_EXT_SELLV_RXSAMPL_OFFSET;
	reg_set(hpipe_addr + HPIPE_VDD_CAL_CTRL_REG, data, mask);

	/* Set PLL Charge-pump Current Control */
	mask = HPIPE_G3_SETTING_5_G3_ICP_MASK;
	data = 0x4 << HPIPE_G3_SETTING_5_G3_ICP_OFFSET;
	reg_set(hpipe_addr + HPIPE_G3_SETTING_5_REG, data, mask);

	/* Set lane rqualization remote setting */
	mask = HPIPE_LANE_CFG_FOM_DIRN_OVERRIDE_MASK;
	data = 0x1 << HPIPE_LANE_CFG_FOM_DIRN_OVERRIDE_OFFSET;
	mask |= HPIPE_LANE_CFG_FOM_ONLY_MODE_MASK;
	data |= 0x1 << HPIPE_LANE_CFG_FOM_ONLY_MODE_OFFFSET;
	mask |= HPIPE_LANE_CFG_FOM_PRESET_VECTOR_MASK;
	data |= 0x6 << HPIPE_LANE_CFG_FOM_PRESET_VECTOR_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_EQ_REMOTE_SETTING_REG, data, mask);

	mask = HPIPE_CFG_EQ_BUNDLE_DIS_MASK;
	data = 0x1 << HPIPE_CFG_EQ_BUNDLE_DIS_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_EQ_CFG2_REG, data, mask);

	debug("stage: Comphy power up\n");

	/* For PCIe X4 or X2:
	 * release from reset only after finish to configure all lanes
	 */
	if ((pcie_width == PCIE_LNK_X1) || (comphy_index == (pcie_width - 1))) {
		uint32_t i, start_lane, end_lane;

		if (pcie_width != PCIE_LNK_X1) {
			/* allows writing to all lanes in one write */
			data = 0x0;
			if (pcie_width == PCIE_LNK_X2)
				mask = COMMON_PHY_SD_CTRL1_COMPHY_0_1_PORT_MASK;
			else if (pcie_width == PCIE_LNK_X4)
				mask = COMMON_PHY_SD_CTRL1_COMPHY_0_3_PORT_MASK;
			reg_set(comphy_base + COMMON_PHY_SD_CTRL1, data, mask);
			start_lane = 0;
			end_lane = pcie_width;

			/* Release from PIPE soft reset
			 * For PCIe by4 or by2:
			 * release from soft reset all lanes - can't use
			 * read modify write
			 */
			reg_set(HPIPE_ADDR(
				COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base), 0) +
				HPIPE_RST_CLK_CTRL_REG, 0x24, 0xffffffff);
		} else {
			start_lane = comphy_index;
			end_lane = comphy_index + 1;

			/* Release from PIPE soft reset
			 * for PCIe by4 or by2:
			 * release from soft reset all lanes
			 */
			reg_set(hpipe_addr + HPIPE_RST_CLK_CTRL_REG,
				0x0 << HPIPE_RST_CLK_CTRL_PIPE_RST_OFFSET,
				HPIPE_RST_CLK_CTRL_PIPE_RST_MASK);
		}

		if (pcie_width != PCIE_LNK_X1) {
			/* disable writing to all lanes with one write */
			if (pcie_width == PCIE_LNK_X2) {
				data = (COMPHY_LANE0 <<
				COMMON_PHY_SD_CTRL1_COMPHY_0_PORT_OFFSET) |
				(COMPHY_LANE1 <<
				COMMON_PHY_SD_CTRL1_COMPHY_1_PORT_OFFSET);
				mask = COMMON_PHY_SD_CTRL1_COMPHY_0_1_PORT_MASK;
			} else if (pcie_width == PCIE_LNK_X4) {
				data = (COMPHY_LANE0 <<
				COMMON_PHY_SD_CTRL1_COMPHY_0_PORT_OFFSET) |
				(COMPHY_LANE1 <<
				COMMON_PHY_SD_CTRL1_COMPHY_1_PORT_OFFSET) |
				(COMPHY_LANE2 <<
				COMMON_PHY_SD_CTRL1_COMPHY_2_PORT_OFFSET) |
				(COMPHY_LANE3 <<
				COMMON_PHY_SD_CTRL1_COMPHY_3_PORT_OFFSET);
				mask = COMMON_PHY_SD_CTRL1_COMPHY_0_3_PORT_MASK;
			}
			reg_set(comphy_base + COMMON_PHY_SD_CTRL1,
				data, mask);
		}

		debug("stage: Check PLL\n");
		/* Read lane status */
		for (i = start_lane; i < end_lane; i++) {
			addr = HPIPE_ADDR(
				COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base), i) +
				HPIPE_LANE_STATUS1_REG;
			data = HPIPE_LANE_STATUS1_PCLK_EN_MASK;
			mask = data;
			ret = polling_with_timeout(addr, data, mask,
						   PLL_LOCK_TIMEOUT,
						   REG_32BIT);
			if (ret)
				ERROR("Failed to lock PCIE PLL\n");
		}
	}

	debug_exit();

	return ret;
}

static int mvebu_cp110_comphy_rxaui_power_on(uint64_t comphy_base,
				     uint8_t comphy_index, uint32_t comphy_mode)
{
	uintptr_t hpipe_addr, sd_ip_addr, comphy_addr, addr;
	uint32_t mask, data;
	int ret = 0;

	debug_enter();

	hpipe_addr = HPIPE_ADDR(COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base),
				comphy_index);
	comphy_addr = COMPHY_ADDR(comphy_base, comphy_index);
	sd_ip_addr = SD_ADDR(COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base),
			     comphy_index);

	/* configure phy selector for RXAUI */
	mvebu_cp110_comphy_set_phy_selector(comphy_base, comphy_index,
					    comphy_mode);

	/* RFU configurations - hard reset comphy */
	mask = COMMON_PHY_CFG1_PWR_UP_MASK;
	data = 0x1 << COMMON_PHY_CFG1_PWR_UP_OFFSET;
	mask |= COMMON_PHY_CFG1_PIPE_SELECT_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_PIPE_SELECT_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG1_REG, data, mask);

	if (comphy_index == 2) {
		reg_set(comphy_base + COMMON_PHY_SD_CTRL1,
			0x1 << COMMON_PHY_SD_CTRL1_RXAUI0_OFFSET,
			COMMON_PHY_SD_CTRL1_RXAUI0_MASK);
	}
	if (comphy_index == 4) {
		reg_set(comphy_base + COMMON_PHY_SD_CTRL1,
			0x1 << COMMON_PHY_SD_CTRL1_RXAUI1_OFFSET,
			COMMON_PHY_SD_CTRL1_RXAUI1_MASK);
	}

	/* Select Baud Rate of Comphy And PD_PLL/Tx/Rx */
	mask = SD_EXTERNAL_CONFIG0_SD_PU_PLL_MASK;
	data = 0x0 << SD_EXTERNAL_CONFIG0_SD_PU_PLL_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PHY_GEN_RX_MASK;
	data |= 0xB << SD_EXTERNAL_CONFIG0_SD_PHY_GEN_RX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PHY_GEN_TX_MASK;
	data |= 0xB << SD_EXTERNAL_CONFIG0_SD_PHY_GEN_TX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PU_RX_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG0_SD_PU_RX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PU_TX_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG0_SD_PU_TX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_HALF_BUS_MODE_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG0_HALF_BUS_MODE_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_MEDIA_MODE_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG0_MEDIA_MODE_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG0_REG, data, mask);

	/* release from hard reset */
	mask = SD_EXTERNAL_CONFIG1_RESET_IN_MASK;
	data = 0x0 << SD_EXTERNAL_CONFIG1_RESET_IN_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RESET_CORE_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG1_RESET_CORE_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RF_RESET_IN_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG1_RF_RESET_IN_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	mask = SD_EXTERNAL_CONFIG1_RESET_IN_MASK;
	data = 0x1 << SD_EXTERNAL_CONFIG1_RESET_IN_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RESET_CORE_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG1_RESET_CORE_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	/* Wait 1ms - until band gap and ref clock ready */
	mdelay(1);

	/* Start comphy Configuration */
	debug("stage: Comphy configuration\n");
	/* set reference clock */
	reg_set(hpipe_addr + HPIPE_MISC_REG,
		0x0 << HPIPE_MISC_REFCLK_SEL_OFFSET,
		HPIPE_MISC_REFCLK_SEL_MASK);
	/* Power and PLL Control */
	mask = HPIPE_PWR_PLL_REF_FREQ_MASK;
	data = 0x1 << HPIPE_PWR_PLL_REF_FREQ_OFFSET;
	mask |= HPIPE_PWR_PLL_PHY_MODE_MASK;
	data |= 0x4 << HPIPE_PWR_PLL_PHY_MODE_OFFSET;
	reg_set(hpipe_addr + HPIPE_PWR_PLL_REG, data, mask);
	/* Loopback register */
	reg_set(hpipe_addr + HPIPE_LOOPBACK_REG,
		0x1 << HPIPE_LOOPBACK_SEL_OFFSET, HPIPE_LOOPBACK_SEL_MASK);
	/* rx control 1 */
	mask = HPIPE_RX_CONTROL_1_RXCLK2X_SEL_MASK;
	data = 0x1 << HPIPE_RX_CONTROL_1_RXCLK2X_SEL_OFFSET;
	mask |= HPIPE_RX_CONTROL_1_CLK8T_EN_MASK;
	data |= 0x1 << HPIPE_RX_CONTROL_1_CLK8T_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_RX_CONTROL_1_REG, data, mask);
	/* DTL Control */
	reg_set(hpipe_addr + HPIPE_PWR_CTR_DTL_REG,
		0x0 << HPIPE_PWR_CTR_DTL_FLOOP_EN_OFFSET,
		HPIPE_PWR_CTR_DTL_FLOOP_EN_MASK);

	/* Set analog parameters from ETP(HW) */
	debug("stage: Analog parameters from ETP(HW)\n");
	/* SERDES External Configuration 2 */
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG2_REG,
		0x1 << SD_EXTERNAL_CONFIG2_PIN_DFE_EN_OFFSET,
		SD_EXTERNAL_CONFIG2_PIN_DFE_EN_MASK);
	/* 0x7-DFE Resolution control */
	reg_set(hpipe_addr + HPIPE_DFE_REG0, 0x1 << HPIPE_DFE_RES_FORCE_OFFSET,
		HPIPE_DFE_RES_FORCE_MASK);
	/* 0xd-G1_Setting_0 */
	reg_set(hpipe_addr + HPIPE_G1_SET_0_REG,
		0xd << HPIPE_G1_SET_0_G1_TX_EMPH1_OFFSET,
		HPIPE_G1_SET_0_G1_TX_EMPH1_MASK);
	/* 0xE-G1_Setting_1 */
	mask = HPIPE_G1_SET_1_G1_RX_SELMUPI_MASK;
	data = 0x1 << HPIPE_G1_SET_1_G1_RX_SELMUPI_OFFSET;
	mask |= HPIPE_G1_SET_1_G1_RX_SELMUPF_MASK;
	data |= 0x1 << HPIPE_G1_SET_1_G1_RX_SELMUPF_OFFSET;
	mask |= HPIPE_G1_SET_1_G1_RX_DFE_EN_MASK;
	data |= 0x1 << HPIPE_G1_SET_1_G1_RX_DFE_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SET_1_REG, data, mask);
	/* 0xA-DFE_Reg3 */
	mask = HPIPE_DFE_F3_F5_DFE_EN_MASK;
	data = 0x0 << HPIPE_DFE_F3_F5_DFE_EN_OFFSET;
	mask |= HPIPE_DFE_F3_F5_DFE_CTRL_MASK;
	data |= 0x0 << HPIPE_DFE_F3_F5_DFE_CTRL_OFFSET;
	reg_set(hpipe_addr + HPIPE_DFE_F3_F5_REG, data, mask);

	/* 0x111-G1_Setting_4 */
	mask = HPIPE_G1_SETTINGS_4_G1_DFE_RES_MASK;
	data = 0x1 << HPIPE_G1_SETTINGS_4_G1_DFE_RES_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SETTINGS_4_REG, data, mask);

	debug("stage: RFU configurations- Power Up PLL,Tx,Rx\n");
	/* SERDES External Configuration */
	mask = SD_EXTERNAL_CONFIG0_SD_PU_PLL_MASK;
	data = 0x1 << SD_EXTERNAL_CONFIG0_SD_PU_PLL_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PU_RX_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG0_SD_PU_RX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PU_TX_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG0_SD_PU_TX_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG0_REG, data, mask);


	/* check PLL rx & tx ready */
	addr = sd_ip_addr + SD_EXTERNAL_STATUS0_REG;
	data = SD_EXTERNAL_STATUS0_PLL_RX_MASK |
		SD_EXTERNAL_STATUS0_PLL_TX_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask, 15000, REG_32BIT);
	if (data != 0) {
		debug("Read from reg = %lx - value = 0x%x\n",
		      sd_ip_addr + SD_EXTERNAL_STATUS0_REG, data);
		ERROR("SD_EXTERNAL_STATUS0_PLL_RX is %d, -\"-_PLL_TX is %d\n",
		      (data & SD_EXTERNAL_STATUS0_PLL_RX_MASK),
		      (data & SD_EXTERNAL_STATUS0_PLL_TX_MASK));
		ret = -ETIMEDOUT;
	}

	/* RX init */
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG,
		0x1 << SD_EXTERNAL_CONFIG1_RX_INIT_OFFSET,
		SD_EXTERNAL_CONFIG1_RX_INIT_MASK);

	/* check that RX init done */
	addr = sd_ip_addr + SD_EXTERNAL_STATUS0_REG;
	data = SD_EXTERNAL_STATUS0_RX_INIT_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask, 100, REG_32BIT);
	if (data != 0) {
		debug("Read from reg = %lx - value = 0x%x\n",
		      sd_ip_addr + SD_EXTERNAL_STATUS0_REG, data);
		ERROR("SD_EXTERNAL_STATUS0_RX_INIT is 0\n");
		ret = -ETIMEDOUT;
	}

	debug("stage: RF Reset\n");
	/* RF Reset */
	mask =  SD_EXTERNAL_CONFIG1_RX_INIT_MASK;
	data = 0x0 << SD_EXTERNAL_CONFIG1_RX_INIT_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RF_RESET_IN_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG1_RF_RESET_IN_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	debug_exit();

	return ret;
}

static int mvebu_cp110_comphy_usb3_power_on(uint64_t comphy_base,
				     uint8_t comphy_index, uint32_t comphy_mode)
{
	uintptr_t hpipe_addr, comphy_addr, addr;
	uint32_t mask, data;
	uint8_t ap_nr, cp_nr, phy_polarity_invert;
	int ret = 0;

	debug_enter();

	/* Configure PIPE selector for USB3 */
	mvebu_cp110_comphy_set_pipe_selector(comphy_base, comphy_index,
					     comphy_mode);

	mvebu_cp110_get_ap_and_cp_nr(&ap_nr, &cp_nr, comphy_base);

	const struct usb_params *usb_static_values =
			&usb_static_values_tab[ap_nr][cp_nr][comphy_index];

	phy_polarity_invert = usb_static_values->polarity_invert;

	hpipe_addr = HPIPE_ADDR(COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base),
				comphy_index);
	comphy_addr = COMPHY_ADDR(comphy_base, comphy_index);

	debug("stage: RFU configurations - hard reset comphy\n");
	/* RFU configurations - hard reset comphy */
	mask = COMMON_PHY_CFG1_PWR_UP_MASK;
	data = 0x1 << COMMON_PHY_CFG1_PWR_UP_OFFSET;
	mask |= COMMON_PHY_CFG1_PIPE_SELECT_MASK;
	data |= 0x1 << COMMON_PHY_CFG1_PIPE_SELECT_OFFSET;
	mask |= COMMON_PHY_CFG1_PWR_ON_RESET_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_PWR_ON_RESET_OFFSET;
	mask |= COMMON_PHY_CFG1_CORE_RSTN_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_CORE_RSTN_OFFSET;
	mask |= COMMON_PHY_PHY_MODE_MASK;
	data |= 0x1 << COMMON_PHY_PHY_MODE_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG1_REG, data, mask);

	/* release from hard reset */
	mask = COMMON_PHY_CFG1_PWR_ON_RESET_MASK;
	data = 0x1 << COMMON_PHY_CFG1_PWR_ON_RESET_OFFSET;
	mask |= COMMON_PHY_CFG1_CORE_RSTN_MASK;
	data |= 0x1 << COMMON_PHY_CFG1_CORE_RSTN_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG1_REG, data, mask);

	/* Wait 1ms - until band gap and ref clock ready */
	mdelay(1);

	/* Start comphy Configuration */
	debug("stage: Comphy configuration\n");
	/* Set PIPE soft reset */
	mask = HPIPE_RST_CLK_CTRL_PIPE_RST_MASK;
	data = 0x1 << HPIPE_RST_CLK_CTRL_PIPE_RST_OFFSET;
	/* Set PHY datapath width mode for V0 */
	mask |= HPIPE_RST_CLK_CTRL_FIXED_PCLK_MASK;
	data |= 0x0 << HPIPE_RST_CLK_CTRL_FIXED_PCLK_OFFSET;
	/* Set Data bus width USB mode for V0 */
	mask |= HPIPE_RST_CLK_CTRL_PIPE_WIDTH_MASK;
	data |= 0x0 << HPIPE_RST_CLK_CTRL_PIPE_WIDTH_OFFSET;
	/* Set CORE_CLK output frequency for 250Mhz */
	mask |= HPIPE_RST_CLK_CTRL_CORE_FREQ_SEL_MASK;
	data |= 0x0 << HPIPE_RST_CLK_CTRL_CORE_FREQ_SEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_RST_CLK_CTRL_REG, data, mask);
	/* Set PLL ready delay for 0x2 */
	reg_set(hpipe_addr + HPIPE_CLK_SRC_LO_REG,
		0x2 << HPIPE_CLK_SRC_LO_PLL_RDY_DL_OFFSET,
		HPIPE_CLK_SRC_LO_PLL_RDY_DL_MASK);
	/* Set reference clock to come from group 1 - 25Mhz */
	reg_set(hpipe_addr + HPIPE_MISC_REG,
		0x0 << HPIPE_MISC_REFCLK_SEL_OFFSET,
		HPIPE_MISC_REFCLK_SEL_MASK);
	/* Set reference frequcency select - 0x2 */
	mask = HPIPE_PWR_PLL_REF_FREQ_MASK;
	data = 0x2 << HPIPE_PWR_PLL_REF_FREQ_OFFSET;
	/* Set PHY mode to USB - 0x5 */
	mask |= HPIPE_PWR_PLL_PHY_MODE_MASK;
	data |= 0x5 << HPIPE_PWR_PLL_PHY_MODE_OFFSET;
	reg_set(hpipe_addr + HPIPE_PWR_PLL_REG, data, mask);
	/* Set the amount of time spent in the LoZ state - set for 0x7 */
	reg_set(hpipe_addr + HPIPE_GLOBAL_PM_CTRL,
		0x7 << HPIPE_GLOBAL_PM_RXDLOZ_WAIT_OFFSET,
		HPIPE_GLOBAL_PM_RXDLOZ_WAIT_MASK);
	/* Set max PHY generation setting - 5Gbps */
	reg_set(hpipe_addr + HPIPE_INTERFACE_REG,
		0x1 << HPIPE_INTERFACE_GEN_MAX_OFFSET,
		HPIPE_INTERFACE_GEN_MAX_MASK);
	/* Set select data width 20Bit (SEL_BITS[2:0]) */
	reg_set(hpipe_addr + HPIPE_LOOPBACK_REG,
		0x1 << HPIPE_LOOPBACK_SEL_OFFSET,
		HPIPE_LOOPBACK_SEL_MASK);
	/* select de-emphasize 3.5db */
	reg_set(hpipe_addr + HPIPE_LANE_CONFIG0_REG,
		0x1 << HPIPE_LANE_CONFIG0_TXDEEMPH0_OFFSET,
		HPIPE_LANE_CONFIG0_TXDEEMPH0_MASK);
	/* override tx margining from the MAC */
	reg_set(hpipe_addr + HPIPE_TST_MODE_CTRL_REG,
		0x1 << HPIPE_TST_MODE_CTRL_MODE_MARGIN_OFFSET,
		HPIPE_TST_MODE_CTRL_MODE_MARGIN_MASK);

	/* The polarity inversion for USB was not tested due to lack of hw
	 * design which requires it. Support is added for customer needs.
	 */
	if (phy_polarity_invert)
		mvebu_cp110_polarity_invert(hpipe_addr + HPIPE_SYNC_PATTERN_REG,
					    phy_polarity_invert);

	/* Start analog parameters from ETP(HW) */
	debug("stage: Analog parameters from ETP(HW)\n");
	/* Set Pin DFE_PAT_DIS -> Bit[1]: PIN_DFE_PAT_DIS = 0x0 */
	mask = HPIPE_LANE_CFG4_DFE_CTRL_MASK;
	data = 0x1 << HPIPE_LANE_CFG4_DFE_CTRL_OFFSET;
	/* Set Override PHY DFE control pins for 0x1 */
	mask |= HPIPE_LANE_CFG4_DFE_OVER_MASK;
	data |= 0x1 << HPIPE_LANE_CFG4_DFE_OVER_OFFSET;
	/* Set Spread Spectrum Clock Enable fot 0x1 */
	mask |= HPIPE_LANE_CFG4_SSC_CTRL_MASK;
	data |= 0x1 << HPIPE_LANE_CFG4_SSC_CTRL_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_CFG4_REG, data, mask);
	/* Confifure SSC amplitude */
	mask = HPIPE_G2_TX_SSC_AMP_MASK;
	data = 0x1f << HPIPE_G2_TX_SSC_AMP_OFFSET;
	reg_set(hpipe_addr + HPIPE_G2_SET_2_REG, data, mask);
	/* End of analog parameters */

	debug("stage: Comphy power up\n");
	/* Release from PIPE soft reset */
	reg_set(hpipe_addr + HPIPE_RST_CLK_CTRL_REG,
		0x0 << HPIPE_RST_CLK_CTRL_PIPE_RST_OFFSET,
		HPIPE_RST_CLK_CTRL_PIPE_RST_MASK);

	/* wait 15ms - for comphy calibration done */
	debug("stage: Check PLL\n");
	/* Read lane status */
	addr = hpipe_addr + HPIPE_LANE_STATUS1_REG;
	data = HPIPE_LANE_STATUS1_PCLK_EN_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask, 15000, REG_32BIT);
	if (data != 0) {
		debug("Read from reg = %lx - value = 0x%x\n",
			hpipe_addr + HPIPE_LANE_STATUS1_REG, data);
		ERROR("HPIPE_LANE_STATUS1_PCLK_EN_MASK is 0\n");
		ret = -ETIMEDOUT;
	}

	debug_exit();

	return ret;
}

static void rx_pre_train(uint64_t comphy_base, uint8_t comphy_index)
{
	uintptr_t hpipe_addr;
	uint32_t mask, data;

	hpipe_addr = HPIPE_ADDR(COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base),
				comphy_index);

	debug("rx_training preparation\n\n");

	mask = HPIPE_TRX0_GAIN_TRAIN_WITH_C_MASK;
	data = (0x1 << HPIPE_TRX0_GAIN_TRAIN_WITH_C_OFF);
	mask |= HPIPE_TRX0_GAIN_TRAIN_WITH_SAMPLER_MASK;
	data |= (0x0 << HPIPE_TRX0_GAIN_TRAIN_WITH_SAMPLER_OFF);
	reg_set(hpipe_addr + HPIPE_TRX0_REG, data, mask);


	mask = HPIPE_TRX_REG2_SUMF_BOOST_TARGET_C_MASK;
	data = (0x1e << HPIPE_TRX_REG2_SUMF_BOOST_TARGET_C_OFF);
	mask |= HPIPE_TRX_REG2_SUMF_BOOST_TARGET_K_MASK;
	data |= (0x0 << HPIPE_TRX_REG2_SUMF_BOOST_TARGET_K_OFF);
	reg_set(hpipe_addr + HPIPE_TRX_REG2, data, mask);

	mask = HPIPE_TRX_REG1_MIN_BOOST_MODE_MASK;
	data = (0x1 << HPIPE_TRX_REG1_MIN_BOOST_MODE_OFF);
	reg_set(hpipe_addr + HPIPE_TRX_REG1, data, mask);

	mask = HPIPE_CRD2_CRD_MIDPOINT_SMALL_THRES_K_MASK;
	data = (0x8 << HPIPE_CRD2_CRD_MIDPOINT_SMALL_THRES_K_OFF);
	reg_set(hpipe_addr + HPIPE_CDR_CONTROL1_REG, data, mask);

	mask = HPIPE_CRD2_CRD_MIDPOINT_LARGE_THRES_K_MASK;
	data = (0x8 << HPIPE_CRD2_CRD_MIDPOINT_LARGE_THRES_K_OFF);
	reg_set(hpipe_addr + HPIPE_CDR_CONTROL2_REG, data, mask);

	mask = HPIPE_CRD_MIDPOINT_PHASE_OS_MASK;
	data = (0x0 << HPIPE_CRD_MIDPOINT_PHASE_OS_OFFSET);
	reg_set(hpipe_addr + HPIPE_CDR_CONTROL_REG, data, mask);

	mask = HPIPE_TRX_REG1_SUMFTAP_EN_MASK;
	data = (0x38 << HPIPE_TRX_REG1_SUMFTAP_EN_OFF);
	mask |= HPIPE_TRX_REG2_SUMF_BOOST_TARGET_C_MASK;
	data |= (0x1e << HPIPE_TRX_REG2_SUMF_BOOST_TARGET_C_OFF);
	reg_set(hpipe_addr + HPIPE_TRX_REG1, data, mask);
}

int mvebu_cp110_comphy_xfi_rx_training(uint64_t comphy_base,
					      uint8_t comphy_index)
{
	uint32_t mask, data, timeout;
	uint32_t g1_ffe_cap_sel, g1_ffe_res_sel, align90, g1_dfe_res;
	uintptr_t hpipe_addr;

	uint8_t ap_nr, cp_nr;

	mvebu_cp110_get_ap_and_cp_nr(&ap_nr, &cp_nr, comphy_base);

	hpipe_addr = HPIPE_ADDR(COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base),
				comphy_index);

	debug_enter();

	rx_pre_train(comphy_base, comphy_index);

	debug("Preparation for rx_training\n\n");

	/* Use the FFE table */
	mask = HPIPE_G1_SETTINGS_3_G1_FFE_SETTING_FORCE_MASK;
	data = 0 << HPIPE_G1_SETTINGS_3_G1_FFE_SETTING_FORCE_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SETTINGS_3_REG, data, mask);

	/* Use auto-calibration value */
	mask = HPIPE_CAL_RXCLKALIGN_90_EXT_EN_MASK;
	data = 0 << HPIPE_CAL_RXCLKALIGN_90_EXT_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_RX_CLK_ALIGN90_AND_TX_IDLE_CALIB_CTRL_REG,
		data, mask);

	/* Use Tx/Rx training results */
	mask = HPIPE_DFE_RES_FORCE_MASK;
	data = 0 << HPIPE_DFE_RES_FORCE_OFFSET;
	reg_set(hpipe_addr + HPIPE_DFE_REG0, data, mask);

	debug("Enable RX training\n\n");

	mask = HPIPE_TRX_RX_TRAIN_EN_MASK;
	data = 0x1 << HPIPE_TRX_RX_TRAIN_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_TRX_TRAIN_CTRL_0_REG, data, mask);

	/* Check the result of RX training */
	timeout = RX_TRAINING_TIMEOUT;
	mask = HPIPE_INTERRUPT_TRX_TRAIN_DONE_OFFSET |
		HPIPE_INTERRUPT_DFE_DONE_INT_OFFSET |
		HPIPE_INTERRUPT_RX_TRAIN_COMPLETE_INT_MASK;
	while (timeout) {
		data = mmio_read_32(hpipe_addr + HPIPE_INTERRUPT_1_REGISTER);
		if (data & mask)
			break;
		mdelay(1);
		timeout--;
	}

	debug("RX training result: interrupt reg 0x%lx = 0x%x\n\n",
	       hpipe_addr + HPIPE_INTERRUPT_1_REGISTER, data);

	if (timeout == 0 || data & HPIPE_TRX_TRAIN_TIME_OUT_INT_MASK) {
		ERROR("Rx training timeout...\n");
		return -ETIMEDOUT;
	}

	if (data & HPIPE_TRX_TRAIN_FAILED_MASK) {
		ERROR("Rx training failed...\n");
		return -EINVAL;
	}

	mask = HPIPE_TRX_RX_TRAIN_EN_MASK;
	data = 0x0 << HPIPE_TRX_RX_TRAIN_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_TRX_TRAIN_CTRL_0_REG, data, mask);

	debug("Training done, reading results...\n\n");

	mask = HPIPE_ADAPTED_FFE_ADAPTED_FFE_RES_MASK;
	g1_ffe_res_sel = ((mmio_read_32(hpipe_addr +
			   HPIPE_ADAPTED_FFE_CAPACITOR_COUNTER_CTRL_REG)
			   & mask) >> HPIPE_ADAPTED_FFE_ADAPTED_FFE_RES_OFFSET);

	mask = HPIPE_ADAPTED_FFE_ADAPTED_FFE_CAP_MASK;
	g1_ffe_cap_sel = ((mmio_read_32(hpipe_addr +
			   HPIPE_ADAPTED_FFE_CAPACITOR_COUNTER_CTRL_REG)
			   & mask) >> HPIPE_ADAPTED_FFE_ADAPTED_FFE_CAP_OFFSET);

	mask = HPIPE_DATA_PHASE_ADAPTED_OS_PH_MASK;
	align90 = ((mmio_read_32(hpipe_addr + HPIPE_DATA_PHASE_OFF_CTRL_REG)
		    & mask) >> HPIPE_DATA_PHASE_ADAPTED_OS_PH_OFFSET);

	mask = HPIPE_ADAPTED_DFE_RES_MASK;
	g1_dfe_res = ((mmio_read_32(hpipe_addr +
		       HPIPE_ADAPTED_DFE_COEFFICIENT_1_REG)
		       & mask) >> HPIPE_ADAPTED_DFE_RES_OFFSET);

	debug("================================================\n");
	debug("Switching to static configuration:\n");
	debug("FFE_RES = 0x%x FFE_CAP = 0x%x align90 = 0x%x g1_dfe_res 0x%x\n",
	       g1_ffe_res_sel, g1_ffe_cap_sel, align90, g1_dfe_res);
	debug("Result after training: 0x%lx= 0x%x, 0x%lx= 0x%x, 0x%lx = 0x%x\n",
	      (hpipe_addr + HPIPE_ADAPTED_FFE_CAPACITOR_COUNTER_CTRL_REG),
	       mmio_read_32(hpipe_addr +
			    HPIPE_ADAPTED_FFE_CAPACITOR_COUNTER_CTRL_REG),
			    (hpipe_addr + HPIPE_DATA_PHASE_OFF_CTRL_REG),
	       mmio_read_32(hpipe_addr + HPIPE_DATA_PHASE_OFF_CTRL_REG),
			    (hpipe_addr + HPIPE_ADAPTED_DFE_COEFFICIENT_1_REG),
	       mmio_read_32(hpipe_addr + HPIPE_ADAPTED_DFE_COEFFICIENT_1_REG));
	debug("================================================\n");

	/* Update FFE_RES */
	mask = HPIPE_G1_SETTINGS_3_G1_FFE_RES_SEL_MASK;
	data = g1_ffe_res_sel << HPIPE_G1_SETTINGS_3_G1_FFE_RES_SEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SETTINGS_3_REG, data, mask);

	/* Update FFE_CAP */
	mask = HPIPE_G1_SETTINGS_3_G1_FFE_CAP_SEL_MASK;
	data = g1_ffe_cap_sel << HPIPE_G1_SETTINGS_3_G1_FFE_CAP_SEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SETTINGS_3_REG, data, mask);

	/* Bypass the FFE table settings and use the FFE settings directly from
	 * registers FFE_RES_SEL and FFE_CAP_SEL
	 */
	mask = HPIPE_G1_SETTINGS_3_G1_FFE_SETTING_FORCE_MASK;
	data = 1 << HPIPE_G1_SETTINGS_3_G1_FFE_SETTING_FORCE_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SETTINGS_3_REG, data, mask);

	/* Force DFE resolution (use gen table value) */
	mask = HPIPE_DFE_RES_FORCE_MASK;
	data = 0x1 << HPIPE_DFE_RES_FORCE_OFFSET;
	reg_set(hpipe_addr + HPIPE_DFE_REG0, data, mask);

	/* 0x111-G1 DFE_Setting_4 */
	mask = HPIPE_G1_SETTINGS_4_G1_DFE_RES_MASK;
	data = g1_dfe_res << HPIPE_G1_SETTINGS_4_G1_DFE_RES_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SETTINGS_4_REG, data, mask);

	printf("########################################################\n");
	printf("# To use trained values update the ATF sources:\n");
	printf("# plat/marvell/armada/a8k/<board_type>/board/phy-porting-layer.h ");
	printf("file\n# with new values as below (for appropriate AP nr %d",
	       ap_nr);
	printf("and CP nr: %d comphy_index %d\n\n",
	       cp_nr, comphy_index);
	printf("static struct xfi_params xfi_static_values_tab[AP_NUM]");
	printf("[CP_NUM][MAX_LANE_NR] = {\n");
	printf("\t...\n");
	printf("\t.g1_ffe_res_sel = 0x%x,\n", g1_ffe_res_sel);
	printf("\t.g1_ffe_cap_sel = 0x%x,\n", g1_ffe_cap_sel);
	printf("\t.align90 = 0x%x,\n", align90);
	printf("\t.g1_dfe_res = 0x%x\n", g1_dfe_res);
	printf("\t...\n");
	printf("};\n\n");
	printf("########################################################\n");

	rx_trainng_done[ap_nr][cp_nr][comphy_index] = 1;

	return 0;
}

/* During AP the proper mode is auto-negotiated and the mac, pcs and serdes
 * configuration are done by the firmware loaded to the MG's CM3 for appropriate
 * negotiated mode. Therefore there is no need to configure the mac, pcs and
 * serdes from u-boot. The only thing that need to be setup is powering up
 * the comphy, which is done through Common PHY<n> Configuration 1 Register
 * (CP0: 0xF2441000, CP1: 0xF4441000). This step can't be done by MG's CM3,
 * since it doesn't have an access to this register-set (but it has access to
 * the network registers like: MG, AP, MAC, PCS, Serdes etc.)
 */
static int mvebu_cp110_comphy_ap_power_on(uint64_t comphy_base,
					  uint8_t comphy_index,
					  uint32_t comphy_mode)
{
	uint32_t mask, data;
	uint8_t ap_nr, cp_nr;
	uintptr_t comphy_addr = comphy_addr =
				COMPHY_ADDR(comphy_base, comphy_index);

	/* configure phy selector for XFI/SFI */
	mvebu_cp110_comphy_set_phy_selector(comphy_base, comphy_index,
					    comphy_mode);
	debug_enter();
	debug("stage: RFU configurations - hard reset comphy\n");
	/* RFU configurations - hard reset comphy */
	mask = COMMON_PHY_CFG1_PWR_UP_MASK;
	data = 0x1 << COMMON_PHY_CFG1_PWR_UP_OFFSET;
	mask |= COMMON_PHY_CFG1_PIPE_SELECT_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_PIPE_SELECT_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG1_REG, data, mask);
	debug_exit();

	/* Start AP Firmware */
	mvebu_cp110_get_ap_and_cp_nr(&ap_nr, &cp_nr, comphy_base);
	mg_start_ap_fw(cp_nr, comphy_index);

	return 0;
}

/*
 * This function allows to reset the digital synchronizers between
 * the MAC and the PHY, it is required when the MAC changes its state.
 */
int mvebu_cp110_comphy_digital_reset(uint64_t comphy_base,
				     uint8_t comphy_index,
				     uint32_t comphy_mode, uint32_t command)
{
	int mode = COMPHY_GET_MODE(comphy_mode);
	uintptr_t sd_ip_addr;
	uint32_t mask, data;

	sd_ip_addr = SD_ADDR(COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base),
			     comphy_index);

	switch (mode) {
	case (COMPHY_SGMII_MODE):
	case (COMPHY_HS_SGMII_MODE):
	case (COMPHY_XFI_MODE):
	case (COMPHY_SFI_MODE):
	case (COMPHY_RXAUI_MODE):
		mask = SD_EXTERNAL_CONFIG1_RF_RESET_IN_MASK;
		data = ((command == COMPHY_COMMAND_DIGITAL_PWR_OFF) ?
			0x0 : 0x1) << SD_EXTERNAL_CONFIG1_RF_RESET_IN_OFFSET;
		reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);
		break;
	default:
		ERROR("comphy%d: Digital PWR ON/OFF is not supported\n",
			comphy_index);
			return -EINVAL;
	}

	return 0;
}

int mvebu_cp110_comphy_power_on(uint64_t comphy_base, uint8_t comphy_index,
				uint64_t comphy_mode)
{
	int mode = COMPHY_GET_MODE(comphy_mode);
	int err = 0;

	debug_enter();

	switch (mode) {
	case(COMPHY_SATA_MODE):
		err = mvebu_cp110_comphy_sata_power_on(comphy_base,
						       comphy_index,
						       comphy_mode);
		break;
	case(COMPHY_SGMII_MODE):
	case(COMPHY_HS_SGMII_MODE):
		err = mvebu_cp110_comphy_sgmii_power_on(comphy_base,
							comphy_index,
							comphy_mode);
		break;
	/* From comphy perspective, XFI and SFI are the same */
	case (COMPHY_XFI_MODE):
	case (COMPHY_SFI_MODE):
		err = mvebu_cp110_comphy_xfi_power_on(comphy_base,
						      comphy_index,
						      comphy_mode);
		break;
	case (COMPHY_PCIE_MODE):
		err = mvebu_cp110_comphy_pcie_power_on(comphy_base,
						       comphy_index,
						       comphy_mode);
		break;
	case (COMPHY_RXAUI_MODE):
		err = mvebu_cp110_comphy_rxaui_power_on(comphy_base,
							comphy_index,
							comphy_mode);
		break;
	case (COMPHY_USB3H_MODE):
	case (COMPHY_USB3D_MODE):
		err = mvebu_cp110_comphy_usb3_power_on(comphy_base,
						       comphy_index,
						       comphy_mode);
		break;
	case (COMPHY_AP_MODE):
		err = mvebu_cp110_comphy_ap_power_on(comphy_base, comphy_index,
						     comphy_mode);
		break;
	default:
		ERROR("comphy%d: unsupported comphy mode\n", comphy_index);
		err = -EINVAL;
		break;
	}

	debug_exit();

	return err;
}

int mvebu_cp110_comphy_power_off(uint64_t comphy_base, uint8_t comphy_index,
				 uint64_t comphy_mode)
{
	uintptr_t sd_ip_addr, comphy_ip_addr;
	uint32_t mask, data;
	uint8_t ap_nr, cp_nr;
	_Bool called_from_uboot = COMPHY_GET_CALLER(comphy_mode);

	debug_enter();

	/* Power-off might happen because of 2 things:
	 *	1. Bootloader turns off unconnected lanes
	 *	2. Linux turns off all lanes during boot
	 *	   (and then reconfigure it).
	 *
	 * For PCIe, there's a problem:
	 * In Armada 8K DB boards, PCIe initialization can be executed
	 * only once (PCIe reset performed during chip power on and
	 * it cannot be executed via GPIO later) so a lane configured to
	 * PCIe should not be powered off by Linux.
	 *
	 * So, check 2 things:
	 *	1. Is Linux called for power-off?
	 *	2. Is the comphy configured to PCIe?
	 * If the answer is YES for both 1 and 2, skip the power-off.
	 *
	 * TODO: In MacciatoBIN, PCIe reset is connected via GPIO,
	 * so after GPIO reset is added to Linux Kernel, it can be
	 * powered-off.
	 */
	if (!called_from_uboot) {
		data = mmio_read_32(comphy_base +
				    COMMON_SELECTOR_PIPE_REG_OFFSET);
		data >>= (COMMON_SELECTOR_COMPHYN_FIELD_WIDTH * comphy_index);
		data &= COMMON_SELECTOR_COMPHY_MASK;
		if (data == COMMON_SELECTOR_PIPE_COMPHY_PCIE)
			return 0;
	}

	mvebu_cp110_get_ap_and_cp_nr(&ap_nr, &cp_nr, comphy_base);

	if (rx_trainng_done[ap_nr][cp_nr][comphy_index]) {
		debug("Skip %s for comphy[%d][%d][%d], due to rx training\n",
		       __func__, ap_nr, cp_nr, comphy_index);
		return 0;
	}

	sd_ip_addr = SD_ADDR(COMPHY_PIPE_FROM_COMPHY_ADDR(comphy_base),
			     comphy_index);
	comphy_ip_addr = COMPHY_ADDR(comphy_base, comphy_index);

	/* Hard reset the comphy, for Ethernet modes and Sata */
	mask = SD_EXTERNAL_CONFIG1_RESET_IN_MASK;
	data = 0x0 << SD_EXTERNAL_CONFIG1_RESET_IN_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RESET_CORE_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG1_RESET_CORE_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RF_RESET_IN_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG1_RF_RESET_IN_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	/* PCIe reset */
	spin_lock(&cp110_mac_reset_lock);

	/* The mvebu_cp110_comphy_power_off will be called only from Linux (to
	 * override settings done by bootloader) and it will be relevant only
	 * to PCIe (called before check if to skip pcie power off or not).
	 */
	data = mmio_read_32(SYS_CTRL_FROM_COMPHY_ADDR(comphy_base) +
						 SYS_CTRL_UINIT_SOFT_RESET_REG);
	switch (comphy_index) {
	case COMPHY_LANE0:
		data &= ~PCIE_MAC_RESET_MASK_PORT0;
		break;
	case COMPHY_LANE4:
		data &= ~PCIE_MAC_RESET_MASK_PORT1;
		break;
	case COMPHY_LANE5:
		data &= ~PCIE_MAC_RESET_MASK_PORT2;
		break;
	}

	mmio_write_32(SYS_CTRL_FROM_COMPHY_ADDR(comphy_base) +
					   SYS_CTRL_UINIT_SOFT_RESET_REG, data);
	spin_unlock(&cp110_mac_reset_lock);

	/* Hard reset the comphy, for PCIe and usb3 */
	mask = COMMON_PHY_CFG1_PWR_ON_RESET_MASK;
	data = 0x0 << COMMON_PHY_CFG1_PWR_ON_RESET_OFFSET;
	mask |= COMMON_PHY_CFG1_CORE_RSTN_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_CORE_RSTN_OFFSET;
	reg_set(comphy_ip_addr + COMMON_PHY_CFG1_REG, data, mask);

	/* Clear comphy PHY and PIPE selector, can't rely on previous config. */
	mvebu_cp110_comphy_clr_phy_selector(comphy_base, comphy_index);
	mvebu_cp110_comphy_clr_pipe_selector(comphy_base, comphy_index);

	debug_exit();

	return 0;
}

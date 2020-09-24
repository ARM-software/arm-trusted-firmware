/*
 * Copyright (c) 2019 - 2021, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <mdio.h>
#include <platform_usb.h>
#include <sr_utils.h>
#include "sr_usb.h"
#include <usbh_xhci_regs.h>

static uint32_t usb_func = USB3_DRD | USB3H_USB2DRD;

static void usb_pm_rescal_init(void)
{
	uint32_t data;
	uint32_t try;

	mmio_setbits_32(CDRU_MISC_RESET_CONTROL, CDRU_PM_RESET_N_R);
	/* release reset */
	mmio_setbits_32(CDRU_CHIP_TOP_SPARE_REG0, RESCAL_I_RSTB);
	udelay(10U);
	/* power up */
	mmio_setbits_32(CDRU_CHIP_TOP_SPARE_REG0,
			RESCAL_I_RSTB | RESCAL_I_PWRDNB);
	try = 1000U;
	do {
		udelay(1U);
		data = mmio_read_32(CDRU_CHIP_TOP_SPARE_REG1);
		try--;
	} while ((data & RESCAL_I_PWRDNB) == 0x0U && (try != 0U));

	if (try == 0U) {
		ERROR("CDRU_CHIP_TOP_SPARE_REG1: 0x%x\n", data);
	}

	INFO("USB and PM Rescal Init done..\n");
}

const unsigned int xhc_portsc_reg_offset[MAX_USB_PORTS] = {
	XHC_PORTSC1_OFFSET,
	XHC_PORTSC2_OFFSET,
	XHC_PORTSC3_OFFSET,
};

static void usb3h_usb2drd_init(void)
{
	uint32_t val;

	INFO("USB3H + USB 2DRD init\n");
	mmio_clrbits_32(USB3H_U3PHY_CTRL, POR_RESET);
	val = mmio_read_32(USB3H_PWR_CTRL);
	val &= ~(0x3U << POWER_CTRL_OVRD);
	val |= (1U << POWER_CTRL_OVRD);
	mmio_write_32(USB3H_PWR_CTRL, val);
	mmio_setbits_32(USB3H_U3PHY_CTRL, PHY_RESET);
	/* Phy to come out of reset */
	udelay(2U);
	mmio_clrbits_32(USB3H_U3PHY_CTRL, MDIO_RESET);

	/* MDIO in reset */
	udelay(2U);
	mmio_setbits_32(USB3H_U3PHY_CTRL, MDIO_RESET);

	/* After MDIO reset release */
	udelay(2U);

	/* USB 3.0 phy Analog Block Initialization */
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_MDIO_BLOCK_BASE_REG,
			USB3_PHY_ANA_BLOCK_BASE);
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_ANA_REG0, 0x4646U);
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_ANA_REG1, 0x80c9U);
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_ANA_REG2, 0x88a6U);
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_ANA_REG5, 0x7c12U);
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_ANA_REG8, 0x1d07U);
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_ANA_REG11, 0x25cU);

	/* USB 3.0 phy RXPMD Block initialization*/
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_MDIO_BLOCK_BASE_REG,
			USB3_PHY_RXPMD_BLOCK_BASE);
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_RXPMD_REG1, 0x4052U);
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_RXPMD_REG2, 0x4cU);
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_RXPMD_REG5, 0x7U);
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_RXPMD_REG7, 0x173U);

	/* USB 3.0 phy AEQ Block initialization*/
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_MDIO_BLOCK_BASE_REG,
			USB3_PHY_AEQ_BLOCK_BASE);
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_AEQ_REG1, 0x3000U);
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_AEQ_REG3, 0x2c70U);

	/* USB 3.0 phy TXPMD Block initialization*/
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_MDIO_BLOCK_BASE_REG,
			USB3_PHY_TXPMD_BLOCK_BASE);
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_TXPMD_REG1, 0x100fU);
	mdio_write(MDIO_BUS_ID, USB3H_PHY_ID, USB3_PHY_TXPMD_REG2, 0x238cU);
}

static void usb3drd_init(void)
{
	uint32_t val;

	INFO("USB3DRD init\n");
	mmio_clrbits_32(DRDU3_U3PHY_CTRL, POR_RESET);
	val = mmio_read_32(DRDU3_PWR_CTRL);
	val &= ~(0x3U << POWER_CTRL_OVRD);
	val |= (1U << POWER_CTRL_OVRD);
	mmio_write_32(DRDU3_PWR_CTRL, val);
	mmio_setbits_32(DRDU3_U3PHY_CTRL, PHY_RESET);
	/* Phy to come out of reset */
	udelay(2U);
	mmio_clrbits_32(DRDU3_U3PHY_CTRL, MDIO_RESET);

	/* MDIO in reset */
	udelay(2U);
	mmio_setbits_32(DRDU3_U3PHY_CTRL, MDIO_RESET);

	/* After MDIO reset release */
	udelay(2U);

	/* USB 3.0 DRD phy Analog Block Initialization */
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_MDIO_BLOCK_BASE_REG,
			USB3_PHY_ANA_BLOCK_BASE);
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_ANA_REG0, 0x4646U);
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_ANA_REG1, 0x80c9U);
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_ANA_REG2, 0x88a6U);
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_ANA_REG5, 0x7c12U);
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_ANA_REG8, 0x1d07U);
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_ANA_REG11, 0x25cU);

	/* USB 3.0 DRD phy RXPMD Block initialization*/
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_MDIO_BLOCK_BASE_REG,
			USB3_PHY_RXPMD_BLOCK_BASE);
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_RXPMD_REG1, 0x4052U);
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_RXPMD_REG2, 0x4cU);
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_RXPMD_REG5, 0x7U);
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_RXPMD_REG7, 0x173U);

	/* USB 3.0 DRD phy AEQ Block initialization*/
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_MDIO_BLOCK_BASE_REG,
			USB3_PHY_AEQ_BLOCK_BASE);
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_AEQ_REG1, 0x3000U);
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_AEQ_REG3, 0x2c70U);

	/* USB 3.0 DRD phy TXPMD Block initialization*/
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_MDIO_BLOCK_BASE_REG,
			USB3_PHY_TXPMD_BLOCK_BASE);
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_TXPMD_REG1, 0x100fU);
	mdio_write(MDIO_BUS_ID, USB3DRD_PHY_ID, USB3_PHY_TXPMD_REG2, 0x238cU);
}

static void usb3_phy_init(void)
{
	usb_pm_rescal_init();

	if ((usb_func & USB3H_USB2DRD) != 0U) {
		usb3h_usb2drd_init();
	}

	if ((usb_func & USB3_DRD) != 0U) {
		usb3drd_init();
	}
}

#ifdef USB_DMA_COHERENT
void usb_enable_coherence(void)
{
	if (usb_func & USB3H_USB2DRD) {
		mmio_setbits_32(USB3H_SOFT_RESET_CTRL,
				USB3H_XHC_AXI_SOFT_RST_N);
		mmio_setbits_32(DRDU2_SOFT_RESET_CTRL,
				DRDU2_BDC_AXI_SOFT_RST_N);
		mmio_setbits_32(USB3H_U3PHY_CTRL, USB3H_U3SOFT_RST_N);
		mmio_setbits_32(DRDU2_PHY_CTRL, DRDU2_U2SOFT_RST_N);

		mmio_clrsetbits_32(DRD2U3H_XHC_REGS_AXIWRA,
				   (USBAXIWR_UA_MASK | USBAXIWR_SA_MASK),
				   (USBAXIWR_UA_VAL | USBAXIWR_SA_VAL));

		mmio_clrsetbits_32(DRD2U3H_XHC_REGS_AXIRDA,
				   (USBAXIRD_UA_MASK | USBAXIRD_SA_MASK),
				   (USBAXIRD_UA_VAL | USBAXIRD_SA_VAL));

		mmio_clrsetbits_32(DRDU2D_BDC_REGS_AXIWRA,
				   (USBAXIWR_UA_MASK | USBAXIWR_SA_MASK),
				   (USBAXIWR_UA_VAL | USBAXIWR_SA_VAL));

		mmio_clrsetbits_32(DRDU2D_BDC_REGS_AXIRDA,
				   (USBAXIRD_UA_MASK | USBAXIRD_SA_MASK),
				   (USBAXIRD_UA_VAL | USBAXIRD_SA_VAL));

	}

	if (usb_func & USB3_DRD) {
		mmio_setbits_32(DRDU3_SOFT_RESET_CTRL,
				(DRDU3_XHC_AXI_SOFT_RST_N |
				DRDU3_BDC_AXI_SOFT_RST_N));
		mmio_setbits_32(DRDU3_U3PHY_CTRL,
				(DRDU3_U3XHC_SOFT_RST_N |
				DRDU3_U3BDC_SOFT_RST_N));

		mmio_clrsetbits_32(DRDU3H_XHC_REGS_AXIWRA,
				   (USBAXIWR_UA_MASK | USBAXIWR_SA_MASK),
				   (USBAXIWR_UA_VAL | USBAXIWR_SA_VAL));

		mmio_clrsetbits_32(DRDU3H_XHC_REGS_AXIRDA,
				   (USBAXIRD_UA_MASK | USBAXIRD_SA_MASK),
				   (USBAXIRD_UA_VAL | USBAXIRD_SA_VAL));

		mmio_clrsetbits_32(DRDU3D_BDC_REGS_AXIWRA,
				   (USBAXIWR_UA_MASK | USBAXIWR_SA_MASK),
				   (USBAXIWR_UA_VAL | USBAXIWR_SA_VAL));

		mmio_clrsetbits_32(DRDU3D_BDC_REGS_AXIRDA,
				   (USBAXIRD_UA_MASK | USBAXIRD_SA_MASK),
				   (USBAXIRD_UA_VAL | USBAXIRD_SA_VAL));
	}
}
#endif

void xhci_phy_init(void)
{
	uint32_t val;

	INFO("usb init start\n");
	mmio_setbits_32(CDRU_MISC_CLK_ENABLE_CONTROL,
			CDRU_MISC_CLK_USBSS);

	mmio_setbits_32(CDRU_MISC_RESET_CONTROL, CDRU_USBSS_RESET_N);

	if (usb_func & USB3_DRD) {
		VERBOSE(" - configure stream_id = 0x6800 for DRDU3\n");
		val = SR_SID_VAL(0x3U, 0x1U, 0x0U) << ICFG_USB_SID_SHIFT;
		mmio_write_32(ICFG_DRDU3_SID_CTRL + ICFG_USB_SID_AWADDR_OFFSET,
				val);
		mmio_write_32(ICFG_DRDU3_SID_CTRL + ICFG_USB_SID_ARADDR_OFFSET,
				val);

		/*
		 * DRDU3 Device USB Space, DRDU3 Host USB Space,
		 * DRDU3 SS Config
		 */
		mmio_setbits_32(USBIC_GPV_SECURITY10,
				USBIC_GPV_SECURITY10_FIELD);
	}

	if (usb_func & USB3H_USB2DRD) {
		VERBOSE(" - configure stream_id = 0x6801 for USB3H\n");
		val = SR_SID_VAL(0x3U, 0x1U, 0x1U) << ICFG_USB_SID_SHIFT;
		mmio_write_32(ICFG_USB3H_SID_CTRL + ICFG_USB_SID_AWADDR_OFFSET,
				val);
		mmio_write_32(ICFG_USB3H_SID_CTRL + ICFG_USB_SID_ARADDR_OFFSET,
				val);

		VERBOSE(" - configure stream_id = 0x6802 for DRDU2\n");
		val = SR_SID_VAL(0x3U, 0x1U, 0x2U) << ICFG_USB_SID_SHIFT;
		mmio_write_32(ICFG_DRDU2_SID_CTRL + ICFG_USB_SID_AWADDR_OFFSET,
				val);
		mmio_write_32(ICFG_DRDU2_SID_CTRL + ICFG_USB_SID_ARADDR_OFFSET,
				val);

		/* DRDU2 APB Bridge:DRDU2 USB Device, USB3H SS Config */
		mmio_setbits_32(USBIC_GPV_SECURITY1, USBIC_GPV_SECURITY1_FIELD);

		/*
		 * USB3H APB Bridge:DRDU2 Host + USB3 Host USB Space,
		 * USB3H SS Config
		 */
		mmio_setbits_32(USBIC_GPV_SECURITY2, USBIC_GPV_SECURITY2_FIELD);
	}

	/* Configure Host masters as non-Secure */
	mmio_setbits_32(USBSS_TZPCDECPROT0set, USBSS_TZPCDECPROT0);

	/* CCN Slave on USBIC */
	mmio_setbits_32(USBIC_GPV_SECURITY0, USBIC_GPV_SECURITY0_FIELD);

	/* SLAVE_8:IDM Register Space */
	mmio_setbits_32(USBIC_GPV_SECURITY4, USBIC_GPV_SECURITY4_FIELD);

	usb3_phy_init();
#ifdef USB_DMA_COHERENT
	usb_enable_coherence();
#endif

	usb_device_init(usb_func);

	INFO("PLAT USB: init done.\n");
}

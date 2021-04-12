/*
 * Copyright (c) 2019 - 2021, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_usb.h>
#include <usb_phy.h>

#define USB_PHY_ALREADY_STARTED	(-2)
#define USB_MAX_DEVICES		 2
#define USB3H_USB2DRD_PHY	 0
#define USB3_DRD_PHY		 1

/* Common bit fields for all the USB2 phy */
#define USB2_PHY_ISO		DRDU2_U2PHY_ISO
#define USB2_AFE_PLL_PWRDWNB	DRDU2_U2AFE_PLL_PWRDWNB
#define USB2_AFE_BG_PWRDWNB	DRDU2_U2AFE_BG_PWRDWNB
#define USB2_AFE_LDO_PWRDWNB	DRDU2_U2AFE_LDO_PWRDWNB
#define USB2_CTRL_CORERDY	DRDU2_U2CTRL_CORERDY

#define USB2_PHY_PCTL_MASK	DRDU2_U2PHY_PCTL_MASK
#define USB2_PHY_PCTL_OFFSET	DRDU2_U2PHY_PCTL_OFFSET
#define USB2_PHY_PCTL_VAL	U2PHY_PCTL_VAL

#define USB2_PLL_RESETB		DRDU2_U2PLL_RESETB
#define USB2_PHY_RESETB		DRDU2_U2PHY_RESETB

static usb_phy_port_t usb_phy_port[2U][MAX_NR_PORTS];

static usb_phy_t usb_phy_info[2U] = {
	{DRDU2_U2PLL_NDIV_FRAC, USB3H_PIPE_CTRL, 0U, USB3H_DRDU2_PHY},
	{0U, 0U, DRDU3_PIPE_CTRL, DRDU3_PHY}
};

typedef struct {
	void *pcd_id;
} usb_platform_dev;

/* index 0: USB3H + USB2 DRD, 1: USB3 DRD */
static usb_platform_dev xhci_devices_configs[USB_MAX_DEVICES] = {
	{&usb_phy_info[0U]},
	{&usb_phy_info[1U]}
};

static int32_t pll_lock_check(uint32_t address, uint32_t bit)
{
	uint32_t retry;
	uint32_t data;

	retry = PLL_LOCK_RETRY_COUNT;
	do {
		data = mmio_read_32(address);
		if ((data & bit) != 0U) {
			return 0;
		}
		udelay(1);
	} while (--retry != 0);

	ERROR("%s(): FAIL (0x%08x)\n", __func__, address);
	return -1;
}

/*
 * USB2 PHY using external FSM bringup sequence
 * Total #3 USB2 phys. All phys has the same
 * bringup sequence. Register bit fields for
 * some of the PHY's are different.
 * Bit fields which are different are passed using
 * struct u2_phy_ext_fsm with bit-fields and register addr.
 */

static void u2_phy_ext_fsm_power_on(struct u2_phy_ext_fsm *u2_phy)
{
	mmio_setbits_32(u2_phy->phy_ctrl_reg, USB2_PHY_ISO);
	/* Delay as per external FSM spec */
	udelay(10U);

	mmio_setbits_32(u2_phy->phy_ctrl_reg, u2_phy->phy_iddq);
	/* Delay as per external FSM spec */
	udelay(10U);

	mmio_clrbits_32(u2_phy->phy_ctrl_reg,
			(USB2_AFE_BG_PWRDWNB |
			 USB2_AFE_PLL_PWRDWNB |
			 USB2_AFE_LDO_PWRDWNB |
			 USB2_CTRL_CORERDY));

	mmio_clrsetbits_32(u2_phy->phy_ctrl_reg,
			   (USB2_PHY_PCTL_MASK << USB2_PHY_PCTL_OFFSET),
			   (USB2_PHY_PCTL_VAL << USB2_PHY_PCTL_OFFSET));
	/* Delay as per external FSM spec */
	udelay(160U);

	mmio_setbits_32(u2_phy->phy_ctrl_reg, USB2_CTRL_CORERDY);
	/* Delay as per external FSM spec */
	udelay(50U);

	mmio_setbits_32(u2_phy->phy_ctrl_reg, USB2_AFE_BG_PWRDWNB);
	/* Delay as per external FSM spec */
	udelay(200U);

	mmio_setbits_32(u2_phy->pwr_ctrl_reg, u2_phy->pwr_onin);
	mmio_setbits_32(u2_phy->phy_ctrl_reg, USB2_AFE_LDO_PWRDWNB);
	/* Delay as per external FSM spec */
	udelay(10U);

	mmio_setbits_32(u2_phy->pwr_ctrl_reg, u2_phy->pwr_okin);
	/* Delay as per external FSM spec */
	udelay(10U);

	mmio_setbits_32(u2_phy->phy_ctrl_reg, USB2_AFE_PLL_PWRDWNB);
	/* Delay as per external FSM spec */
	udelay(10U);

	mmio_clrbits_32(u2_phy->phy_ctrl_reg, USB2_PHY_ISO);
	/* Delay as per external FSM spec */
	udelay(10U);
	mmio_clrbits_32(u2_phy->phy_ctrl_reg, u2_phy->phy_iddq);
	/* Delay as per external FSM spec */
	udelay(1U);

	mmio_setbits_32(u2_phy->pll_ctrl_reg, USB2_PLL_RESETB);
	mmio_setbits_32(u2_phy->phy_ctrl_reg, USB2_PHY_RESETB);

}

static int32_t usb3h_u2_phy_power_on(uint32_t base)
{
	int32_t status;
	struct u2_phy_ext_fsm u2_phy;

	u2_phy.pll_ctrl_reg = base + USB3H_U2PLL_CTRL;
	u2_phy.phy_ctrl_reg = base + USB3H_U2PHY_CTRL;
	u2_phy.phy_iddq = USB3H_U2PHY_IDDQ;
	u2_phy.pwr_ctrl_reg = base + USB3H_PWR_CTRL;
	u2_phy.pwr_okin = USB3H_PWR_CTRL_U2PHY_DFE_SWITCH_PWROKIN;
	u2_phy.pwr_onin = USB3H_PWR_CTRL_U2PHY_DFE_SWITCH_PWRONIN;

	u2_phy_ext_fsm_power_on(&u2_phy);

	status = pll_lock_check(base + USB3H_U2PLL_CTRL, USB3H_U2PLL_LOCK);
	if (status != 0) {
		/* re-try by toggling the PLL reset */
		mmio_clrbits_32(base + USB3H_U2PLL_CTRL,
				(uint32_t)USB3H_U2PLL_RESETB);
		mmio_setbits_32(base + USB3H_U2PLL_CTRL, USB3H_U2PLL_RESETB);
		status = pll_lock_check(base + USB3H_U2PLL_CTRL,
					USB3H_U2PLL_LOCK);
		if (status != 0)
			ERROR("%s() re-try PLL lock FAIL (0x%08x)\n", __func__,
			      base + USB3H_U2PLL_CTRL);
	}

	mmio_clrsetbits_32(base + USB3H_U2PHY_CTRL,
			   (USB3H_U2PHY_PCTL_MASK << USB3H_U2PHY_PCTL_OFFSET),
			   (U2PHY_PCTL_NON_DRV_LOW << USB3H_U2PHY_PCTL_OFFSET));
	return status;
}

static int32_t usb3h_u3_phy_power_on(uint32_t base)
{
	int32_t status;

	/* Set pctl with mode and soft reset */
	mmio_clrsetbits_32(base + USB3H_U3PHY_CTRL,
			   (USB3H_U3PHY_PCTL_MASK << USB3H_U3PHY_PCTL_OFFSET),
			   (U3PHY_PCTL_VAL << USB3H_U3PHY_PCTL_OFFSET));

	mmio_clrbits_32(base + USB3H_U3PHY_PLL_CTRL,
			(uint32_t) USB3H_U3SSPLL_SUSPEND_EN);
	mmio_setbits_32(base + USB3H_U3PHY_PLL_CTRL, USB3H_U3PLL_SEQ_START);
	mmio_setbits_32(base + USB3H_U3PHY_PLL_CTRL, USB3H_U3PLL_RESETB);

	/* Time to stabilize the PLL Control */
	mdelay(1U);

	status = pll_lock_check(base + USB3H_U3PHY_PLL_CTRL,
				USB3H_U3PLL_SS_LOCK);

	return status;
}

static int32_t drdu3_u2_phy_power_on(uint32_t base)
{
	int32_t status;
	struct u2_phy_ext_fsm u2_phy;

	u2_phy.pll_ctrl_reg = base + DRDU3_U2PLL_CTRL;
	u2_phy.phy_ctrl_reg = base + DRDU3_U2PHY_CTRL;
	u2_phy.phy_iddq = DRDU3_U2PHY_IDDQ;
	u2_phy.pwr_ctrl_reg = base + DRDU3_PWR_CTRL;
	u2_phy.pwr_okin = DRDU3_U2PHY_DFE_SWITCH_PWROKIN;
	u2_phy.pwr_onin = DRDU3_U2PHY_DFE_SWITCH_PWRONIN;

	u2_phy_ext_fsm_power_on(&u2_phy);

	status = pll_lock_check(base + DRDU3_U2PLL_CTRL, DRDU3_U2PLL_LOCK);
	if (status != 0) {
		/* re-try by toggling the PLL reset */
		mmio_clrbits_32(base + DRDU3_U2PLL_CTRL,
				(uint32_t)DRDU2_U2PLL_RESETB);
		mmio_setbits_32(base + DRDU3_U2PLL_CTRL, DRDU3_U2PLL_RESETB);

		status = pll_lock_check(base + DRDU3_U2PLL_CTRL,
					DRDU3_U2PLL_LOCK);
		if (status != 0) {
			ERROR("%s() re-try PLL lock FAIL (0x%08x)\n", __func__,
			      base + DRDU3_U2PLL_CTRL);
		}
	}
	mmio_clrsetbits_32(base + DRDU3_U2PHY_CTRL,
			   (DRDU3_U2PHY_PCTL_MASK << DRDU3_U2PHY_PCTL_OFFSET),
			   (U2PHY_PCTL_NON_DRV_LOW << DRDU3_U2PHY_PCTL_OFFSET));

	return status;
}

static int32_t drdu3_u3_phy_power_on(uint32_t base)
{
	int32_t status;

	/* Set pctl with mode and soft reset */
	mmio_clrsetbits_32(base + DRDU3_U3PHY_CTRL,
			   (DRDU3_U3PHY_PCTL_MASK << DRDU3_U3PHY_PCTL_OFFSET),
			   (U3PHY_PCTL_VAL << DRDU3_U3PHY_PCTL_OFFSET));

	mmio_clrbits_32(base + DRDU3_U3PHY_PLL_CTRL,
			(uint32_t) DRDU3_U3SSPLL_SUSPEND_EN);
	mmio_setbits_32(base + DRDU3_U3PHY_PLL_CTRL, DRDU3_U3PLL_SEQ_START);
	mmio_setbits_32(base + DRDU3_U3PHY_PLL_CTRL, DRDU3_U3PLL_RESETB);

	/* Time to stabilize the PLL Control */
	mdelay(1U);

	status = pll_lock_check(base + DRDU3_U3PHY_PLL_CTRL,
				DRDU3_U3PLL_SS_LOCK);

	return status;
}

static int32_t drdu2_u2_phy_power_on(uint32_t base)
{
	int32_t status;
	struct u2_phy_ext_fsm u2_phy;

	u2_phy.pll_ctrl_reg = base + DRDU2_U2PLL_CTRL;
	u2_phy.phy_ctrl_reg = base + DRDU2_PHY_CTRL;
	u2_phy.phy_iddq = DRDU2_U2IDDQ;
	u2_phy.pwr_ctrl_reg = base + DRDU2_PWR_CTRL;
	u2_phy.pwr_okin = DRDU2_U2PHY_DFE_SWITCH_PWROKIN_I;
	u2_phy.pwr_onin = DRDU2_U2PHY_DFE_SWITCH_PWRONIN_I;

	u2_phy_ext_fsm_power_on(&u2_phy);

	status = pll_lock_check(base + DRDU2_U2PLL_CTRL, DRDU2_U2PLL_LOCK);
	if (status != 0) {
		/* re-try by toggling the PLL reset */
		mmio_clrbits_32(base + DRDU2_U2PLL_CTRL,
				(uint32_t)DRDU2_U2PLL_RESETB);
		mmio_setbits_32(base + DRDU2_U2PLL_CTRL, DRDU2_U2PLL_RESETB);

		status = pll_lock_check(base + DRDU2_U2PLL_CTRL,
					DRDU2_U2PLL_LOCK);
		if (status != 0)
			ERROR("%s() re-try PLL lock FAIL (0x%08x)\n", __func__,
			      base + DRDU2_U2PLL_CTRL);
	}
	mmio_clrsetbits_32(base + DRDU2_PHY_CTRL,
			   (DRDU2_U2PHY_PCTL_MASK << DRDU2_U2PHY_PCTL_OFFSET),
			   (U2PHY_PCTL_NON_DRV_LOW << DRDU2_U2PHY_PCTL_OFFSET));

	return status;
}

void u3h_u2drd_phy_reset(usb_phy_port_t *phy_port)
{
	usb_phy_t *phy = phy_port->p;

	switch (phy_port->port_id) {
	case USB3HS_PORT:
		mmio_clrbits_32(phy->usb3hreg + USB3H_U2PHY_CTRL,
				(uint32_t) USB3H_U2CTRL_CORERDY);
		mmio_setbits_32(phy->usb3hreg + USB3H_U2PHY_CTRL,
				USB3H_U2CTRL_CORERDY);
		break;
	case DRDU2_PORT:
		mmio_clrbits_32(phy->drdu2reg + DRDU2_PHY_CTRL,
				(uint32_t) DRDU2_U2CTRL_CORERDY);
		mmio_setbits_32(phy->drdu2reg + DRDU2_PHY_CTRL,
				DRDU2_U2CTRL_CORERDY);
		break;
	}
}

void u3drd_phy_reset(usb_phy_port_t *phy_port)
{
	usb_phy_t *phy = phy_port->p;

	if (phy_port->port_id == DRD3HS_PORT) {
		mmio_clrbits_32(phy->drdu3reg + DRDU3_U2PHY_CTRL,
				(uint32_t) DRDU3_U2CTRL_CORERDY);
		mmio_setbits_32(phy->drdu3reg + DRDU3_U2PHY_CTRL,
				DRDU3_U2CTRL_CORERDY);
	}
}

static int32_t u3h_u2drd_phy_power_on(usb_phy_port_t *phy_port)
{
	usb_phy_t *phy = phy_port->p;
	int32_t status;

	switch (phy_port->port_id) {
	case USB3SS_PORT:
		mmio_clrbits_32(phy->usb3hreg + USB3H_PHY_PWR_CTRL,
				(uint32_t) USB3H_DISABLE_USB30_P0);
		status = usb3h_u3_phy_power_on(phy->usb3hreg);
		if (status != 0) {
			goto err_usb3h_phy_on;
		}
		break;
	case USB3HS_PORT:
		mmio_clrbits_32(phy->usb3hreg + USB3H_PHY_PWR_CTRL,
				(uint32_t) USB3H_DISABLE_EUSB_P1);
		mmio_setbits_32(AXI_DEBUG_CTRL,
				AXI_DBG_CTRL_SSPHY_DRD_MODE_DISABLE);
		mmio_setbits_32(USB3H_DEBUG_CTRL,
				USB3H_DBG_CTRL_SSPHY_DRD_MODE_DISABLE);

		mmio_clrbits_32(phy->usb3hreg + USB3H_PWR_CTRL,
				USB3H_PWR_CTRL_U2PHY_DFE_SWITCH_PWRONIN);
		/* Delay as per external FSM spec */
		udelay(10U);
		mmio_clrbits_32(phy->usb3hreg + USB3H_PWR_CTRL,
				USB3H_PWR_CTRL_U2PHY_DFE_SWITCH_PWROKIN);
		status = usb3h_u2_phy_power_on(phy->usb3hreg);
		if (status != 0) {
			goto err_usb3h_phy_on;
		}
		break;
	case DRDU2_PORT:
		mmio_clrbits_32(phy->usb3hreg + USB3H_PHY_PWR_CTRL,
				(uint32_t) USB3H_DISABLE_EUSB_P0);
		mmio_setbits_32(AXI_DEBUG_CTRL,
				AXI_DBG_CTRL_SSPHY_DRD_MODE_DISABLE);
		mmio_setbits_32(USB3H_DEBUG_CTRL,
				USB3H_DBG_CTRL_SSPHY_DRD_MODE_DISABLE);

		mmio_clrbits_32(phy->usb3hreg + DRDU2_PWR_CTRL,
				DRDU2_U2PHY_DFE_SWITCH_PWRONIN_I);
		/* Delay as per external FSM spec */
		udelay(10U);
		mmio_clrbits_32(phy->usb3hreg + DRDU2_PWR_CTRL,
				DRDU2_U2PHY_DFE_SWITCH_PWROKIN_I);

		status = drdu2_u2_phy_power_on(phy->drdu2reg);
		if (status != 0) {
			mmio_setbits_32(phy->usb3hreg + USB3H_PHY_PWR_CTRL,
					USB3H_DISABLE_EUSB_P0);
			goto err_drdu2_phy_on;
		}
		break;
	}

	/* Device Mode */
	if (phy_port->port_id == DRDU2_PORT) {
		mmio_write_32(phy->drdu2reg + DRDU2_SOFT_RESET_CTRL,
			      DRDU2_BDC_AXI_SOFT_RST_N);
		mmio_setbits_32(phy->drdu2reg + DRDU2_PHY_CTRL,
				DRDU2_U2SOFT_RST_N);
	}
	/* Host Mode */
	mmio_write_32(phy->usb3hreg + USB3H_SOFT_RESET_CTRL,
		      USB3H_XHC_AXI_SOFT_RST_N);
	mmio_setbits_32(phy->usb3hreg + USB3H_U3PHY_CTRL, USB3H_U3SOFT_RST_N);

	return 0U;
 err_usb3h_phy_on:mmio_setbits_32(phy->usb3hreg + USB3H_PHY_PWR_CTRL,
			(USB3H_DISABLE_EUSB_P1 |
			 USB3H_DISABLE_USB30_P0));
 err_drdu2_phy_on:

	return status;
}

static int32_t u3drd_phy_power_on(usb_phy_port_t *phy_port)
{
	usb_phy_t *phy = phy_port->p;
	int32_t status;

	switch (phy_port->port_id) {
	case DRD3SS_PORT:
		mmio_clrbits_32(phy->drdu3reg + DRDU3_PHY_PWR_CTRL,
				(uint32_t) DRDU3_DISABLE_USB30_P0);

		status = drdu3_u3_phy_power_on(phy->drdu3reg);
		if (status != 0) {
			goto err_drdu3_phy_on;
		}
		break;
	case DRD3HS_PORT:
		mmio_clrbits_32(phy->drdu3reg + DRDU3_PHY_PWR_CTRL,
				(uint32_t) DRDU3_DISABLE_EUSB_P0);
		mmio_setbits_32(AXI_DEBUG_CTRL,
				AXI_DBG_CTRL_SSPHY_DRD_MODE_DISABLE);
		mmio_setbits_32(USB3H_DEBUG_CTRL,
				USB3H_DBG_CTRL_SSPHY_DRD_MODE_DISABLE);

		mmio_clrbits_32(phy->drdu3reg + DRDU3_PWR_CTRL,
				DRDU3_U2PHY_DFE_SWITCH_PWRONIN);
		/* Delay as per external FSM spec */
		udelay(10U);
		mmio_clrbits_32(phy->drdu3reg + DRDU3_PWR_CTRL,
				DRDU3_U2PHY_DFE_SWITCH_PWROKIN);

		status = drdu3_u2_phy_power_on(phy->drdu3reg);
		if (status != 0) {
			goto err_drdu3_phy_on;
		}

		/* Host Mode */
		mmio_setbits_32(phy->drdu3reg + DRDU3_SOFT_RESET_CTRL,
				DRDU3_XHC_AXI_SOFT_RST_N);
		mmio_setbits_32(phy->drdu3reg + DRDU3_U3PHY_CTRL,
				DRDU3_U3XHC_SOFT_RST_N);
		/* Device Mode */
		mmio_setbits_32(phy->drdu3reg + DRDU3_SOFT_RESET_CTRL,
				DRDU3_BDC_AXI_SOFT_RST_N);
		mmio_setbits_32(phy->drdu3reg + DRDU3_U3PHY_CTRL,
				DRDU3_U3BDC_SOFT_RST_N);
		break;
	}

	return 0U;
 err_drdu3_phy_on:mmio_setbits_32(phy->drdu3reg + DRDU3_PHY_PWR_CTRL,
			(DRDU3_DISABLE_EUSB_P0 |
			 DRDU3_DISABLE_USB30_P0));

	return status;
}

static void u3h_u2drd_phy_power_off(usb_phy_port_t *phy_port)
{
	usb_phy_t *p = phy_port->p;

	switch (phy_port->port_id) {
	case USB3SS_PORT:
		mmio_setbits_32(p->usb3hreg + USB3H_PHY_PWR_CTRL,
				USB3H_DISABLE_USB30_P0);
		break;
	case USB3HS_PORT:
		mmio_setbits_32(p->usb3hreg + USB3H_PHY_PWR_CTRL,
				USB3H_DISABLE_EUSB_P1);
		break;
	case DRDU2_PORT:
		mmio_setbits_32(p->usb3hreg + USB3H_PHY_PWR_CTRL,
				USB3H_DISABLE_EUSB_P0);
		break;
	}
}

static void u3drd_phy_power_off(usb_phy_port_t *phy_port)
{
	usb_phy_t *p = phy_port->p;

	switch (phy_port->port_id) {
	case DRD3SS_PORT:
		mmio_setbits_32(p->drdu3reg + DRDU3_PHY_PWR_CTRL,
				DRDU3_DISABLE_USB30_P0);
		break;
	case DRD3HS_PORT:
		mmio_setbits_32(p->drdu3reg + DRDU3_PHY_PWR_CTRL,
				DRDU3_DISABLE_EUSB_P0);
		break;
	}
}

int32_t usb_info_fill(usb_phy_t *phy_info)
{
	int32_t index;

	if (phy_info->initialized != 0U) {
		return USB_PHY_ALREADY_STARTED;
	}

	if (phy_info->phy_id == USB3H_DRDU2_PHY) {
		phy_info->phy_port = usb_phy_port[USB3H_DRDU2_PHY - 1U];
		phy_info->ports_enabled = 0x7U;
	} else {
		phy_info->phy_port = usb_phy_port[DRDU3_PHY - 1U];
		phy_info->ports_enabled = 0x3U;
	}

	for (index = MAX_NR_PORTS - 1U; index > -1; index--) {
		phy_info->phy_port[index].enabled = (phy_info->ports_enabled
						     >> index) & 0x1U;
		phy_info->phy_port[index].p = phy_info;
		phy_info->phy_port[index].port_id = index;
	}

	return 0U;
}

int32_t usb_phy_init(usb_platform_dev *device)
{
	int32_t status;
	usb_phy_t *phy_info;
	uint32_t index;

	phy_info = (usb_phy_t *)device->pcd_id;

	status = usb_info_fill(phy_info);
	if (status != 0) {
		return (status == USB_PHY_ALREADY_STARTED) ? 0 : status;
	}

	for (index = 0U; index < MAX_NR_PORTS; index++) {
		if (phy_info->phy_port[index].enabled != 0U) {
			switch (phy_info->phy_id) {
			case USB3H_DRDU2_PHY:
				status =
				    u3h_u2drd_phy_power_on(&phy_info->
							   phy_port[index]);
				break;
			default:
				status =
				    u3drd_phy_power_on(&phy_info->
						       phy_port[index]);
			}
		}
	}

	phy_info->initialized = !status;
	return status;
}

void usb_phy_shutdown(usb_platform_dev *device)
{
	usb_phy_t *phy_info;
	uint32_t index;

	phy_info = (usb_phy_t *)device->pcd_id;

	phy_info->initialized = 0U;

	for (index = 0U; index < MAX_NR_PORTS; index++) {
		if (phy_info->phy_port[index].enabled != 0U) {
			switch (phy_info->phy_id) {
			case USB3H_DRDU2_PHY:
				u3h_u2drd_phy_power_off(&phy_info->
							phy_port[index]);
				break;
			case DRDU3_PHY:
				u3drd_phy_power_off(&phy_info->phy_port[index]);
				break;
			default:
				INFO("%s: invalid phy id 0x%x\n", __func__,
				     phy_info->phy_id);
			}
		}
	}
}

int32_t usb_xhci_init(usb_platform_dev *device)
{
	int32_t status;

	status = usb_phy_init(device);
	if (status == USB_PHY_ALREADY_STARTED) {
		status = 0U;
	}

	return status;
}

int32_t usb_device_init(unsigned int usb_func)
{
	int32_t status;
	int32_t devices_initialized = 0U;

	if ((usb_func & USB3H_USB2DRD) != 0U) {
		status = usb_xhci_init(
				&xhci_devices_configs[USB3H_USB2DRD_PHY]);
		if (status == 0) {
			devices_initialized++;
		} else {
			ERROR("%s(): USB3H_USB2DRD init failure\n", __func__);
		}
	}

	if ((usb_func & USB3_DRD) != 0U) {
		status = usb_xhci_init(&xhci_devices_configs[USB3_DRD_PHY]);
		if (status == 0) {
			devices_initialized++;
		} else {
			ERROR("%s(): USB3_DRD init failure\n", __func__);
		}
	}

	return devices_initialized;
}

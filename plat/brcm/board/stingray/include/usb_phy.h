/*
 * Copyright (c) 2017 - 2021, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef USB_PHY_H
#define USB_PHY_H

#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <platform_def.h>

#define DRDU2_U2PLL_NDIV_FRAC_OFFSET            0x0U

#define DRDU2_U2PLL_NDIV_INT                    0x4U

#define DRDU2_U2PLL_CTRL                        0x8U
#define DRDU2_U2PLL_LOCK                        BIT(6U)
#define DRDU2_U2PLL_RESETB                      BIT(5U)
#define DRDU2_U2PLL_PDIV_MASK                   0xFU
#define DRDU2_U2PLL_PDIV_OFFSET                 1U
#define DRDU2_U2PLL_SUSPEND_EN                  BIT(0U)

#define DRDU2_PHY_CTRL                          0x0CU
#define DRDU2_U2IDDQ                            BIT(30U)
#define DRDU2_U2SOFT_RST_N                      BIT(29U)
#define DRDU2_U2PHY_ON_FLAG                     BIT(22U)
#define DRDU2_U2PHY_PCTL_MASK                   0xFFFFU
#define DRDU2_U2PHY_PCTL_OFFSET                 6U
#define DRDU2_U2PHY_RESETB                      BIT(5U)
#define DRDU2_U2PHY_ISO                         BIT(4U)
#define DRDU2_U2AFE_BG_PWRDWNB                  BIT(3U)
#define DRDU2_U2AFE_PLL_PWRDWNB                 BIT(2U)
#define DRDU2_U2AFE_LDO_PWRDWNB                 BIT(1U)
#define DRDU2_U2CTRL_CORERDY                    BIT(0U)

#define DRDU2_STRAP_CTRL                        0x18U
#define DRDU2_FORCE_HOST_MODE                   BIT(5U)
#define DRDU2_FORCE_DEVICE_MODE                 BIT(4U)
#define BDC_USB_STP_SPD_MASK                    0x7U
#define BDC_USB_STP_SPD_OFFSET                  0U

#define DRDU2_PWR_CTRL                          0x1CU
#define DRDU2_U2PHY_DFE_SWITCH_PWROKIN_I        BIT(2U)
#define DRDU2_U2PHY_DFE_SWITCH_PWRONIN_I        BIT(1U)

#define DRDU2_SOFT_RESET_CTRL                   0x20U
#define DRDU2_BDC_AXI_SOFT_RST_N                BIT(0U)

#define USB3H_U2PLL_NDIV_FRAC                   0x4U

#define USB3H_U2PLL_NDIV_INT                    0x8U

#define USB3H_U2PLL_CTRL                        0xCU
#define USB3H_U2PLL_LOCK                        BIT(6U)
#define USB3H_U2PLL_RESETB                      BIT(5U)
#define USB3H_U2PLL_PDIV_MASK                   0xFU
#define USB3H_U2PLL_PDIV_OFFSET                 1U

#define USB3H_U2PHY_CTRL                        0x10U
#define USB3H_U2PHY_ON_FLAG                     22U
#define USB3H_U2PHY_PCTL_MASK                   0xFFFFU
#define USB3H_U2PHY_PCTL_OFFSET                 6U
#define USB3H_U2PHY_IDDQ                        BIT(29U)
#define USB3H_U2PHY_RESETB                      BIT(5U)
#define USB3H_U2PHY_ISO                         BIT(4U)
#define USB3H_U2AFE_BG_PWRDWNB                  BIT(3U)
#define USB3H_U2AFE_PLL_PWRDWNB                 BIT(2U)
#define USB3H_U2AFE_LDO_PWRDWNB                 BIT(1U)
#define USB3H_U2CTRL_CORERDY                    BIT(0U)

#define USB3H_U3PHY_CTRL                        0x14U
#define USB3H_U3SOFT_RST_N                      BIT(30U)
#define USB3H_U3MDIO_RESETB_I                   BIT(29U)
#define USB3H_U3POR_RESET_I                     BIT(28U)
#define USB3H_U3PHY_PCTL_MASK                   0xFFFFU
#define USB3H_U3PHY_PCTL_OFFSET                 2U
#define USB3H_U3PHY_RESETB                      BIT(1U)

#define USB3H_U3PHY_PLL_CTRL                    0x18U
#define USB3H_U3PLL_REFCLK_MASK                 0x7U
#define USB3H_U3PLL_REFCLK_OFFSET               4U
#define USB3H_U3PLL_SS_LOCK                     BIT(3U)
#define USB3H_U3PLL_SEQ_START                   BIT(2U)
#define USB3H_U3SSPLL_SUSPEND_EN                BIT(1U)
#define USB3H_U3PLL_RESETB                      BIT(0U)

#define USB3H_PWR_CTRL                          0x28U
#define USB3H_PWR_CTRL_OVERRIDE_I_R             4U
#define USB3H_PWR_CTRL_U2PHY_DFE_SWITCH_PWROKIN BIT(11U)
#define USB3H_PWR_CTRL_U2PHY_DFE_SWITCH_PWRONIN BIT(10U)

#define USB3H_SOFT_RESET_CTRL                   0x2CU
#define USB3H_XHC_AXI_SOFT_RST_N                BIT(1U)

#define USB3H_PHY_PWR_CTRL                      0x38U
#define USB3H_DISABLE_USB30_P0                  BIT(2U)
#define USB3H_DISABLE_EUSB_P1                   BIT(1U)
#define USB3H_DISABLE_EUSB_P0                   BIT(0U)


#define DRDU3_U2PLL_NDIV_FRAC                   0x4U

#define DRDU3_U2PLL_NDIV_INT                    0x8U

#define DRDU3_U2PLL_CTRL                        0xCU
#define DRDU3_U2PLL_LOCK                        BIT(6U)
#define DRDU3_U2PLL_RESETB                      BIT(5U)
#define DRDU3_U2PLL_PDIV_MASK                   0xFU
#define DRDU3_U2PLL_PDIV_OFFSET                 1U

#define DRDU3_U2PHY_CTRL                        0x10U
#define DRDU3_U2PHY_IDDQ                        BIT(29U)
#define DRDU3_U2PHY_ON_FLAG                     BIT(22U)
#define DRDU3_U2PHY_PCTL_MASK                   0xFFFFU
#define DRDU3_U2PHY_PCTL_OFFSET                 6U
#define DRDU3_U2PHY_RESETB                      BIT(5U)
#define DRDU3_U2PHY_ISO                         BIT(4U)
#define DRDU3_U2AFE_BG_PWRDWNB                  BIT(3U)
#define DRDU3_U2AFE_PLL_PWRDWNB                 BIT(2U)
#define DRDU3_U2AFE_LDO_PWRDWNB                 BIT(1U)
#define DRDU3_U2CTRL_CORERDY                    BIT(0U)

#define DRDU3_U3PHY_CTRL                        0x14U
#define DRDU3_U3XHC_SOFT_RST_N                  BIT(31U)
#define DRDU3_U3BDC_SOFT_RST_N                  BIT(30U)
#define DRDU3_U3MDIO_RESETB_I                   BIT(29U)
#define DRDU3_U3POR_RESET_I                     BIT(28U)
#define DRDU3_U3PHY_PCTL_MASK                   0xFFFFU
#define DRDU3_U3PHY_PCTL_OFFSET                 2U
#define DRDU3_U3PHY_RESETB                      BIT(1U)

#define DRDU3_U3PHY_PLL_CTRL                    0x18U
#define DRDU3_U3PLL_REFCLK_MASK                 0x7U
#define DRDU3_U3PLL_REFCLK_OFFSET               4U
#define DRDU3_U3PLL_SS_LOCK                     BIT(3U)
#define DRDU3_U3PLL_SEQ_START                   BIT(2U)
#define DRDU3_U3SSPLL_SUSPEND_EN                BIT(1U)
#define DRDU3_U3PLL_RESETB                      BIT(0U)

#define DRDU3_STRAP_CTRL                        0x28U
#define BDC_USB_STP_SPD_MASK                    0x7U
#define BDC_USB_STP_SPD_OFFSET                  0U
#define BDC_USB_STP_SPD_SS                      0x0U
#define BDC_USB_STP_SPD_HS                      0x2U

#define DRDU3_PWR_CTRL                          0x2cU
#define DRDU3_U2PHY_DFE_SWITCH_PWROKIN          BIT(12U)
#define DRDU3_U2PHY_DFE_SWITCH_PWRONIN          BIT(11U)
#define DRDU3_PWR_CTRL_OVERRIDE_I_R             4U

#define DRDU3_SOFT_RESET_CTRL                   0x30U
#define DRDU3_XHC_AXI_SOFT_RST_N                BIT(1U)
#define DRDU3_BDC_AXI_SOFT_RST_N                BIT(0U)

#define DRDU3_PHY_PWR_CTRL                      0x3cU
#define DRDU3_DISABLE_USB30_P0                  BIT(2U)
#define DRDU3_DISABLE_EUSB_P1                   BIT(1U)
#define DRDU3_DISABLE_EUSB_P0                   BIT(0U)

#define PLL_REFCLK_PAD                          0x0U
#define PLL_REFCLK_25MHZ                        0x1U
#define PLL_REFCLK_96MHZ                        0x2U
#define PLL_REFCLK_INTERNAL                     0x3U
/* USB PLL lock time out for 10 ms */
#define PLL_LOCK_RETRY_COUNT                    10000U


#define U2PLL_NDIV_INT_VAL                      0x13U
#define U2PLL_NDIV_FRAC_VAL                     0x1005U
#define U2PLL_PDIV_VAL                          0x1U
/*
 * Using external FSM
 * BIT-3:2: device mode; mode is not effect
 * BIT-1: soft reset active low
 */
#define U2PHY_PCTL_VAL                          0x0003U
/* Non-driving signal low */
#define U2PHY_PCTL_NON_DRV_LOW                  0x0002U
#define U3PHY_PCTL_VAL                          0x0006U

#define MAX_NR_PORTS                            3U

#define USB3H_DRDU2_PHY                         1U
#define DRDU3_PHY                               2U

#define USB_HOST_MODE                           1U
#define USB_DEV_MODE                            2U

#define USB3SS_PORT                             0U
#define DRDU2_PORT                              1U
#define USB3HS_PORT                             2U

#define DRD3SS_PORT                             0U
#define DRD3HS_PORT                             1U

#define SR_USB_PHY_COUNT                        2U

#define DRDU3_PIPE_CTRL			0x68500000U
#define DRDU3H_XHC_REGS_CPLIVER		0x68501000U
#define USB3H_PIPE_CTRL			0x68510000U
#define DRD2U3H_XHC_REGS_CPLIVER	0x68511000U
#define DRDU2_U2PLL_NDIV_FRAC		0x68520000U

#define AXI_DEBUG_CTRL				0x68500038U
#define AXI_DBG_CTRL_SSPHY_DRD_MODE_DISABLE	BIT(12U)

#define USB3H_DEBUG_CTRL			0x68510034U
#define USB3H_DBG_CTRL_SSPHY_DRD_MODE_DISABLE	BIT(7U)

typedef struct _usb_phy_port usb_phy_port_t;

typedef struct {
	uint32_t drdu2reg;
	uint32_t usb3hreg;
	uint32_t drdu3reg;
	uint32_t phy_id;
	uint32_t ports_enabled;
	uint32_t initialized;
	usb_phy_port_t *phy_port;
} usb_phy_t;

struct _usb_phy_port {
	uint32_t port_id;
	uint32_t mode;
	uint32_t enabled;
	usb_phy_t *p;
};

struct u2_phy_ext_fsm {
	uint32_t pll_ctrl_reg;
	uint32_t phy_ctrl_reg;
	uint32_t phy_iddq;
	uint32_t pwr_ctrl_reg;
	uint32_t pwr_okin;
	uint32_t pwr_onin;
};

#endif /* USB_PHY_H */

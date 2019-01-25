/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <drivers/arm/nic_400.h>
#include <lib/mmio.h>
#include <plat/arm/soc/common/soc_css.h>

void soc_css_init_nic400(void)
{
	/*
	 * NIC-400 Access Control Initialization
	 *
	 * Define access privileges by setting each corresponding bit to:
	 *   0 = Secure access only
	 *   1 = Non-secure access allowed
	 */

	/*
	 * Allow non-secure access to some SOC regions, excluding UART1, which
	 * remains secure (unless CSS_NON_SECURE_UART is set).
	 * Note: This is the NIC-400 device on the SOC
	 */
	mmio_write_32(SOC_CSS_NIC400_BASE +
		NIC400_ADDR_CTRL_SECURITY_REG(SOC_CSS_NIC400_USB_EHCI), ~0);
	mmio_write_32(SOC_CSS_NIC400_BASE +
		NIC400_ADDR_CTRL_SECURITY_REG(SOC_CSS_NIC400_TLX_MASTER), ~0);
	mmio_write_32(SOC_CSS_NIC400_BASE +
		NIC400_ADDR_CTRL_SECURITY_REG(SOC_CSS_NIC400_USB_OHCI), ~0);
	mmio_write_32(SOC_CSS_NIC400_BASE +
		NIC400_ADDR_CTRL_SECURITY_REG(SOC_CSS_NIC400_PL354_SMC), ~0);
	mmio_write_32(SOC_CSS_NIC400_BASE +
		NIC400_ADDR_CTRL_SECURITY_REG(SOC_CSS_NIC400_APB4_BRIDGE), ~0);
#if  CSS_NON_SECURE_UART
	/* Configure UART for non-secure access */
	mmio_write_32(SOC_CSS_NIC400_BASE +
		NIC400_ADDR_CTRL_SECURITY_REG(SOC_CSS_NIC400_BOOTSEC_BRIDGE), ~0);
#else
	mmio_write_32(SOC_CSS_NIC400_BASE +
		NIC400_ADDR_CTRL_SECURITY_REG(SOC_CSS_NIC400_BOOTSEC_BRIDGE),
		~SOC_CSS_NIC400_BOOTSEC_BRIDGE_UART1);
#endif /* CSS_NON_SECURE_UART */

}


#define PCIE_SECURE_REG		0x3000
/* Mask uses REG and MEM access bits */
#define PCIE_SEC_ACCESS_MASK	((1 << 0) | (1 << 1))

void soc_css_init_pcie(void)
{
#if !PLAT_juno
	/*
	 * Do not initialize PCIe in emulator environment.
	 * Platform ID register not supported on Juno
	 */
	if (BOARD_CSS_GET_PLAT_TYPE(BOARD_CSS_PLAT_ID_REG_ADDR) ==
			BOARD_CSS_PLAT_TYPE_EMULATOR)
		return;
#endif /* PLAT_juno */

	/*
	 * PCIE Root Complex Security settings to enable non-secure
	 * access to config registers.
	 */
	mmio_write_32(SOC_CSS_PCIE_CONTROL_BASE + PCIE_SECURE_REG,
			PCIE_SEC_ACCESS_MASK);
}

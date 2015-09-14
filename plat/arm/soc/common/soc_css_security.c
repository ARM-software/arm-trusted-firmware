/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#include <board_css_def.h>
#include <mmio.h>
#include <nic_400.h>
#include <platform_def.h>
#include <soc_css_def.h>

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
	 * remains secure.
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
	mmio_write_32(SOC_CSS_NIC400_BASE +
		NIC400_ADDR_CTRL_SECURITY_REG(SOC_CSS_NIC400_BOOTSEC_BRIDGE),
		~SOC_CSS_NIC400_BOOTSEC_BRIDGE_UART1);

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

/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <assert.h>
#include <arch_helpers.h>
#include <platform.h>
#include <bl1.h>
#include <console.h>
#include <cci400.h>
#include <tzc400.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted RAM
 ******************************************************************************/
extern unsigned long __COHERENT_RAM_START__;
extern unsigned long __COHERENT_RAM_END__;

extern unsigned long __BL1_RAM_START__;
extern unsigned long __BL1_RAM_END__;

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL1_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL1_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

#define BL1_RAM_BASE (unsigned long)(&__BL1_RAM_START__)
#define BL1_RAM_LIMIT (unsigned long)(&__BL1_RAM_END__)


/* Data structure which holds the extents of the trusted RAM for BL1 */
static meminfo bl1_tzram_layout;

meminfo *bl1_plat_sec_mem_layout(void)
{
	return &bl1_tzram_layout;
}

/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	const unsigned long bl1_ram_base = BL1_RAM_BASE;
	const unsigned long bl1_ram_limit = BL1_RAM_LIMIT;
	const unsigned long tzram_limit = TZRAM_BASE + TZRAM_SIZE;

	/*
	 * Calculate how much ram is BL1 using & how much remains free.
	 * This also includes a rudimentary mechanism to detect whether
	 * the BL1 data is loaded at the top or bottom of memory.
	 * TODO: add support for discontigous chunks of free ram if
	 *       needed. Might need dynamic memory allocation support
	 *       et al.
	 */
	bl1_tzram_layout.total_base = TZRAM_BASE;
	bl1_tzram_layout.total_size = TZRAM_SIZE;

	if (bl1_ram_limit == tzram_limit) {
		/* BL1 has been loaded at the top of memory. */
		bl1_tzram_layout.free_base = TZRAM_BASE;
		bl1_tzram_layout.free_size = bl1_ram_base - TZRAM_BASE;
	} else {
		/* BL1 has been loaded at the bottom of memory. */
		bl1_tzram_layout.free_base = bl1_ram_limit;
		bl1_tzram_layout.free_size =
			tzram_limit - bl1_ram_limit;
	}

	/* Initialize the console */
	console_init(PL011_UART0_BASE);
}


/*
 * Address of slave 'n' security setting in the NIC-400 address region
 * control
 * TODO: Ideally this macro should be moved in a "nic-400.h" header file but
 * it would be the only thing in there so it's not worth it at the moment.
 */
#define NIC400_ADDR_CTRL_SECURITY_REG(n)	(0x8 + (n) * 4)

static void init_nic400(void)
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
	mmio_write_32(SOC_NIC400_BASE +
		      NIC400_ADDR_CTRL_SECURITY_REG(SOC_NIC400_USB_EHCI), ~0);
	mmio_write_32(SOC_NIC400_BASE +
		      NIC400_ADDR_CTRL_SECURITY_REG(SOC_NIC400_TLX_MASTER), ~0);
	mmio_write_32(SOC_NIC400_BASE +
		      NIC400_ADDR_CTRL_SECURITY_REG(SOC_NIC400_USB_OHCI), ~0);
	mmio_write_32(SOC_NIC400_BASE +
		      NIC400_ADDR_CTRL_SECURITY_REG(SOC_NIC400_PL354_SMC), ~0);
	mmio_write_32(SOC_NIC400_BASE +
		      NIC400_ADDR_CTRL_SECURITY_REG(SOC_NIC400_APB4_BRIDGE), ~0);
	mmio_write_32(SOC_NIC400_BASE +
		      NIC400_ADDR_CTRL_SECURITY_REG(SOC_NIC400_BOOTSEC_BRIDGE),
		      ~SOC_NIC400_BOOTSEC_BRIDGE_UART1);

	/*
	 * Allow non-secure access to some CSS regions.
	 * Note: This is the NIC-400 device on the CSS
	 */
	mmio_write_32(CSS_NIC400_BASE +
		      NIC400_ADDR_CTRL_SECURITY_REG(CSS_NIC400_SLAVE_BOOTSECURE),
		      ~0);
}


static void init_tzc400(void)
{
	/* Enable all filter units available */
	mmio_write_32(TZC400_BASE + TZC400_GATE_KEEPER_REG, 0x0000000f);

	/*
	 * Secure read and write are enabled for region 0, and the background
	 * region (region 0) is enabled for all four filter units
	 */
	mmio_write_32(TZC400_BASE + TZC400_REGION_ATTRIBUTES_0_REG, 0xc0000000);

	/*
	 * Enable Non-secure read/write accesses for the Soc Devices from the
	 * Non-Secure World
	 */
	mmio_write_32(TZC400_BASE + TZC400_REGION_ID_ACCESS_0_REG,
		(TZC400_NSAID_RD_RW << TZC400_NSAID_CCI400)	|
		(TZC400_NSAID_RD_RW << TZC400_NSAID_PCIE)	|
		(TZC400_NSAID_RD_RW << TZC400_NSAID_HDLCD0)	|
		(TZC400_NSAID_RD_RW << TZC400_NSAID_HDLCD1)	|
		(TZC400_NSAID_RD_RW << TZC400_NSAID_USB)	|
		(TZC400_NSAID_RD_RW << TZC400_NSAID_DMA330)	|
		(TZC400_NSAID_RD_RW << TZC400_NSAID_THINLINKS)	|
		(TZC400_NSAID_RD_RW << TZC400_NSAID_AP)		|
		(TZC400_NSAID_RD_RW << TZC400_NSAID_GPU)	|
		(TZC400_NSAID_RD_RW << TZC400_NSAID_SCP)	|
		(TZC400_NSAID_RD_RW << TZC400_NSAID_CORESIGHT)
		);
}

#define PCIE_SECURE_REG		0x3000
#define PCIE_SEC_ACCESS_MASK	((1 << 0) | (1 << 1)) /* REG and MEM access bits */

static void init_pcie(void)
{
	/*
	 * PCIE Root Complex Security settings to enable non-secure
	 * access to config registers.
	 */
	mmio_write_32(PCIE_CONTROL_BASE + PCIE_SECURE_REG, PCIE_SEC_ACCESS_MASK);
}


/*******************************************************************************
 * Function which will perform any remaining platform-specific setup that can
 * occur after the MMU and data cache have been enabled.
 ******************************************************************************/
void bl1_platform_setup(void)
{
	init_nic400();
	init_tzc400();
	init_pcie();

	/* Initialise the IO layer and register platform IO devices */
	io_setup();

	/* Enable and initialize the System level generic timer */
	mmio_write_32(SYS_CNTCTL_BASE + CNTCR_OFF, CNTCR_EN);
}


/*******************************************************************************
 * Perform the very early platform specific architecture setup here. At the
 * moment this only does basic initialization. Later architectural setup
 * (bl1_arch_setup()) does not do anything platform specific.
 ******************************************************************************/
void bl1_plat_arch_setup(void)
{
	/*
	 * Enable CCI-400 for this cluster. No need
	 * for locks as no other cpu is active at the
	 * moment
	 */
	cci_enable_coherency(read_mpidr());

	configure_mmu(&bl1_tzram_layout,
			TZROM_BASE,
			TZROM_BASE + TZROM_SIZE,
			BL1_COHERENT_RAM_BASE,
			BL1_COHERENT_RAM_LIMIT);
}

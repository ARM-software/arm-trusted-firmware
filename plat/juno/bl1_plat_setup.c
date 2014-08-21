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

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <cci400.h>
#include <console.h>
#include <debug.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <tzc400.h>
#include "../../bl1/bl1_private.h"
#include "juno_def.h"
#include "juno_private.h"

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted RAM
 ******************************************************************************/
extern unsigned long __COHERENT_RAM_START__;
extern unsigned long __COHERENT_RAM_END__;

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL1_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL1_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

/* Data structure which holds the extents of the trusted RAM for BL1 */
static meminfo_t bl1_tzram_layout;

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return &bl1_tzram_layout;
}

/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	const size_t bl1_size = BL1_RAM_LIMIT - BL1_RAM_BASE;

	/* Initialize the console to provide early debug support */
	console_init(PL011_UART2_BASE, PL011_UART2_CLK_IN_HZ, PL011_BAUDRATE);

	/*
	 * Enable CCI-400 for this cluster. No need for locks as no other cpu is
	 * active at the moment
	 */
	cci_init(CCI400_BASE,
		 CCI400_SL_IFACE3_CLUSTER_IX,
		 CCI400_SL_IFACE4_CLUSTER_IX);
	cci_enable_cluster_coherency(read_mpidr());

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_tzram_layout.total_base = TZRAM_BASE;
	bl1_tzram_layout.total_size = TZRAM_SIZE;

	/* Calculate how much RAM BL1 is using and how much remains free */
	bl1_tzram_layout.free_base = TZRAM_BASE;
	bl1_tzram_layout.free_size = TZRAM_SIZE;
	reserve_mem(&bl1_tzram_layout.free_base,
		    &bl1_tzram_layout.free_size,
		    BL1_RAM_BASE,
		    bl1_size);

	INFO("BL1: 0x%lx - 0x%lx [size = %u]\n", BL1_RAM_BASE, BL1_RAM_LIMIT,
	     bl1_size);
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
	mmio_write_32(TZC400_BASE + GATE_KEEPER_OFF, 0x0000000f);

	/*
	 * Secure read and write are enabled for region 0, and the background
	 * region (region 0) is enabled for all four filter units
	 */
	mmio_write_32(TZC400_BASE + REGION_ATTRIBUTES_OFF, 0xc0000000);

	/*
	 * Enable Non-secure read/write accesses for the Soc Devices from the
	 * Non-Secure World
	 */
	mmio_write_32(TZC400_BASE + REGION_ID_ACCESS_OFF,
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_CCI400)	|
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_PCIE)	|
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_HDLCD0)	|
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_HDLCD1)	|
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_USB)	|
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_DMA330)	|
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_THINLINKS)	|
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_AP)		|
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_GPU)	|
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_SCP)	|
		TZC_REGION_ACCESS_RDWR(TZC400_NSAID_CORESIGHT)
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
	mmio_write_32(SYS_CNTCTL_BASE + CNTCR_OFF, CNTCR_FCREQ(0) | CNTCR_EN);
}


/*******************************************************************************
 * Perform the very early platform specific architecture setup here. At the
 * moment this only does basic initialization. Later architectural setup
 * (bl1_arch_setup()) does not do anything platform specific.
 ******************************************************************************/
void bl1_plat_arch_setup(void)
{
	configure_mmu_el3(bl1_tzram_layout.total_base,
			  bl1_tzram_layout.total_size,
			  TZROM_BASE,
			  TZROM_BASE + TZROM_SIZE,
			  BL1_COHERENT_RAM_BASE,
			  BL1_COHERENT_RAM_LIMIT);
}

/*******************************************************************************
 * Before calling this function BL2 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL2 and set SPSR and security state.
 * On Juno we are only setting the security state, entrypoint
 ******************************************************************************/
void bl1_plat_set_bl2_ep_info(image_info_t *bl2_image,
			      entry_point_info_t *bl2_ep)
{
	SET_SECURITY_STATE(bl2_ep->h.attr, SECURE);
	bl2_ep->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
}

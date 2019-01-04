/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <bl31/interrupt_mgmt.h>
#include <drivers/console.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include "rpi3_hw.h"
#include "rpi3_private.h"

#define MAP_DEVICE0	MAP_REGION_FLAT(DEVICE0_BASE,			\
					DEVICE0_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_SHARED_RAM	MAP_REGION_FLAT(SHARED_RAM_BASE,		\
					SHARED_RAM_SIZE,		\
					MT_DEVICE | MT_RW | MT_SECURE)

#ifdef RPI3_PRELOADED_DTB_BASE
#define MAP_NS_DTB	MAP_REGION_FLAT(RPI3_PRELOADED_DTB_BASE, 0x10000, \
					MT_MEMORY | MT_RW | MT_NS)
#endif

#define MAP_NS_DRAM0	MAP_REGION_FLAT(NS_DRAM0_BASE, NS_DRAM0_SIZE,	\
					MT_MEMORY | MT_RW | MT_NS)

#define MAP_FIP		MAP_REGION_FLAT(PLAT_RPI3_FIP_BASE,		\
					PLAT_RPI3_FIP_MAX_SIZE,		\
					MT_MEMORY | MT_RO | MT_NS)

#define MAP_BL32_MEM	MAP_REGION_FLAT(BL32_MEM_BASE, BL32_MEM_SIZE,	\
					MT_MEMORY | MT_RW | MT_SECURE)

#ifdef SPD_opteed
#define MAP_OPTEE_PAGEABLE	MAP_REGION_FLAT(		\
				RPI3_OPTEE_PAGEABLE_LOAD_BASE,	\
				RPI3_OPTEE_PAGEABLE_LOAD_SIZE,	\
				MT_MEMORY | MT_RW | MT_SECURE)
#endif

/*
 * Table of regions for various BL stages to map using the MMU.
 */
#ifdef IMAGE_BL1
static const mmap_region_t plat_rpi3_mmap[] = {
	MAP_SHARED_RAM,
	MAP_DEVICE0,
	MAP_FIP,
#ifdef SPD_opteed
	MAP_OPTEE_PAGEABLE,
#endif
	{0}
};
#endif

#ifdef IMAGE_BL2
static const mmap_region_t plat_rpi3_mmap[] = {
	MAP_SHARED_RAM,
	MAP_DEVICE0,
	MAP_FIP,
	MAP_NS_DRAM0,
#ifdef BL32_BASE
	MAP_BL32_MEM,
#endif
	{0}
};
#endif

#ifdef IMAGE_BL31
static const mmap_region_t plat_rpi3_mmap[] = {
	MAP_SHARED_RAM,
	MAP_DEVICE0,
#ifdef RPI3_PRELOADED_DTB_BASE
	MAP_NS_DTB,
#endif
#ifdef BL32_BASE
	MAP_BL32_MEM,
#endif
	{0}
};
#endif

/*******************************************************************************
 * Function that sets up the console
 ******************************************************************************/
static console_16550_t rpi3_console;

void rpi3_console_init(void)
{
	int console_scope = CONSOLE_FLAG_BOOT;
#if RPI3_RUNTIME_UART != -1
	console_scope |= CONSOLE_FLAG_RUNTIME;
#endif
	int rc = console_16550_register(PLAT_RPI3_UART_BASE,
					PLAT_RPI3_UART_CLK_IN_HZ,
					PLAT_RPI3_UART_BAUDRATE,
					&rpi3_console);
	if (rc == 0) {
		/*
		 * The crash console doesn't use the multi console API, it uses
		 * the core console functions directly. It is safe to call panic
		 * and let it print debug information.
		 */
		panic();
	}

	console_set_scope(&rpi3_console.console, console_scope);
}

/*******************************************************************************
 * Function that sets up the translation tables.
 ******************************************************************************/
void rpi3_setup_page_tables(uintptr_t total_base, size_t total_size,
			    uintptr_t code_start, uintptr_t code_limit,
			    uintptr_t rodata_start, uintptr_t rodata_limit
#if USE_COHERENT_MEM
			    , uintptr_t coh_start, uintptr_t coh_limit
#endif
			    )
{
	/*
	 * Map the Trusted SRAM with appropriate memory attributes.
	 * Subsequent mappings will adjust the attributes for specific regions.
	 */
	VERBOSE("Trusted SRAM seen by this BL image: %p - %p\n",
		(void *) total_base, (void *) (total_base + total_size));
	mmap_add_region(total_base, total_base,
			total_size,
			MT_MEMORY | MT_RW | MT_SECURE);

	/* Re-map the code section */
	VERBOSE("Code region: %p - %p\n",
		(void *) code_start, (void *) code_limit);
	mmap_add_region(code_start, code_start,
			code_limit - code_start,
			MT_CODE | MT_SECURE);

	/* Re-map the read-only data section */
	VERBOSE("Read-only data region: %p - %p\n",
		(void *) rodata_start, (void *) rodata_limit);
	mmap_add_region(rodata_start, rodata_start,
			rodata_limit - rodata_start,
			MT_RO_DATA | MT_SECURE);

#if USE_COHERENT_MEM
	/* Re-map the coherent memory region */
	VERBOSE("Coherent region: %p - %p\n",
		(void *) coh_start, (void *) coh_limit);
	mmap_add_region(coh_start, coh_start,
			coh_limit - coh_start,
			MT_DEVICE | MT_RW | MT_SECURE);
#endif

	mmap_add(plat_rpi3_mmap);

	init_xlat_tables();
}

/*******************************************************************************
 * Return entrypoint of BL33.
 ******************************************************************************/
uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	return PLAT_RPI3_NS_IMAGE_OFFSET;
#endif
}

/*******************************************************************************
 * Gets SPSR for BL32 entry
 ******************************************************************************/
uint32_t rpi3_get_spsr_for_bl32_entry(void)
{
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL32 image.
	 */
	return 0;
}

/*******************************************************************************
 * Gets SPSR for BL33 entry
 ******************************************************************************/
uint32_t rpi3_get_spsr_for_bl33_entry(void)
{
#if RPI3_BL33_IN_AARCH32
	INFO("BL33 will boot in Non-secure AArch32 Hypervisor mode\n");
	return SPSR_MODE32(MODE32_hyp, SPSR_T_ARM, SPSR_E_LITTLE,
			   DISABLE_ALL_EXCEPTIONS);
#else
	return SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
#endif
}

unsigned int plat_get_syscnt_freq2(void)
{
	return SYS_COUNTER_FREQ_IN_TICKS;
}

uint32_t plat_ic_get_pending_interrupt_type(void)
{
	ERROR("rpi3: Interrupt routed to EL3.\n");
	return INTR_TYPE_INVAL;
}

uint32_t plat_interrupt_type_to_line(uint32_t type, uint32_t security_state)
{
	assert((type == INTR_TYPE_S_EL1) || (type == INTR_TYPE_EL3) ||
	       (type == INTR_TYPE_NS));

	assert(sec_state_is_valid(security_state));

	/* Non-secure interrupts are signalled on the IRQ line always. */
	if (type == INTR_TYPE_NS)
		return __builtin_ctz(SCR_IRQ_BIT);

	/* Secure interrupts are signalled on the FIQ line always. */
	return  __builtin_ctz(SCR_FIQ_BIT);
}

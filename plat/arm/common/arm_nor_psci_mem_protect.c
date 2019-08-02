/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/cfi/v2m_flash.h>
#include <lib/psci/psci.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <plat/arm/common/plat_arm.h>

/*
 * DRAM1 is used also to load the NS boot loader. For this reason we
 * cannot clear the full DRAM1, because in that case we would clear
 * the NS images (especially for RESET_TO_BL31 and RESET_TO_SPMIN cases).
 * For this reason we reserve 64 MB for the NS images and protect the RAM
 * until the end of DRAM1.
 * We limit the size of DRAM2 to 1 GB to avoid big delays while booting
 */
#define DRAM1_NS_IMAGE_LIMIT  (PLAT_ARM_NS_IMAGE_BASE + (32 << TWO_MB_SHIFT))
#define DRAM1_PROTECTED_SIZE  (ARM_NS_DRAM1_END+1u - DRAM1_NS_IMAGE_LIMIT)

static mem_region_t arm_ram_ranges[] = {
	{DRAM1_NS_IMAGE_LIMIT, DRAM1_PROTECTED_SIZE},
#ifdef __aarch64__
	{ARM_DRAM2_BASE, 1u << ONE_GB_SHIFT},
#endif
};

/*******************************************************************************
 * Function that reads the content of the memory protect variable that
 * enables clearing of non secure memory when system boots. This variable
 * should be stored in a secure NVRAM.
 ******************************************************************************/
int arm_psci_read_mem_protect(int *enabled)
{
	int tmp;

	tmp = *(int *) PLAT_ARM_MEM_PROT_ADDR;
	*enabled = (tmp == 1) ? 1 : 0;
	return 0;
}

/*******************************************************************************
 * Function that writes the content of the memory protect variable that
 * enables overwritten of non secure memory when system boots.
 ******************************************************************************/
int arm_nor_psci_write_mem_protect(int val)
{
	unsigned long enable = (val != 0) ? 1UL : 0UL;

	if (nor_unlock(PLAT_ARM_MEM_PROT_ADDR) != 0) {
		ERROR("unlocking memory protect variable\n");
		return -1;
	}

	if (enable == 1UL) {
		/*
		 * If we want to write a value different than 0
		 * then we have to erase the full block because
		 * otherwise we cannot ensure that the value programmed
		 * into the flash is going to be the same than the value
		 * requested by the caller
		 */
		if (nor_erase(PLAT_ARM_MEM_PROT_ADDR) != 0) {
			ERROR("erasing block containing memory protect variable\n");
			return -1;
		}
	}

	if (nor_word_program(PLAT_ARM_MEM_PROT_ADDR, enable) != 0) {
		ERROR("programming memory protection variable\n");
		return -1;
	}
	return 0;
}

/*******************************************************************************
 * Function used for required psci operations performed when
 * system boots
 ******************************************************************************/
/*
 * PLAT_MEM_PROTECT_VA_FRAME is a address specifically
 * selected in a way that is not needed an additional
 * translation table for memprotect. It happens because
 * we use a chunk of size 2MB and it means that it can
 * be mapped in a level 2 table and the level 2 table
 * for 0xc0000000 is already used and the entry for
 * 0xc0000000 is not used.
 */
#if defined(PLAT_XLAT_TABLES_DYNAMIC)
void arm_nor_psci_do_dyn_mem_protect(void)
{
	int enable;

	arm_psci_read_mem_protect(&enable);
	if (enable == 0)
		return;

	INFO("PSCI: Overwriting non secure memory\n");
	clear_map_dyn_mem_regions(arm_ram_ranges,
				  ARRAY_SIZE(arm_ram_ranges),
				  PLAT_ARM_MEM_PROTEC_VA_FRAME,
				  1 << TWO_MB_SHIFT);
}
#endif

/*******************************************************************************
 * Function used for required psci operations performed when
 * system boots and dynamic memory is not used.
 ******************************************************************************/
void arm_nor_psci_do_static_mem_protect(void)
{
	int enable;

	(void) arm_psci_read_mem_protect(&enable);
	if (enable == 0)
		return;

	INFO("PSCI: Overwriting non secure memory\n");
	clear_mem_regions(arm_ram_ranges,
			  ARRAY_SIZE(arm_ram_ranges));
	(void) arm_nor_psci_write_mem_protect(0);
}

/*******************************************************************************
 * Function that checks if a region is protected by the memory protect
 * mechanism
 ******************************************************************************/
int arm_psci_mem_protect_chk(uintptr_t base, u_register_t length)
{
	return mem_region_in_array_chk(arm_ram_ranges,
				       ARRAY_SIZE(arm_ram_ranges),
				       base, length);
}

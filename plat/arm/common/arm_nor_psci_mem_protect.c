/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <mmio.h>
#include <norflash.h>
#include <plat_arm.h>
#include <platform_def.h>
#include <psci.h>
#include <utils.h>

mem_region_t arm_ram_ranges[] = {
	{ARM_NS_DRAM1_BASE, ARM_NS_DRAM1_SIZE},
#ifdef AARCH64
	{ARM_DRAM2_BASE, ARM_DRAM2_SIZE},
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
	*enabled = (tmp == 1);
	return 0;
}

/*******************************************************************************
 * Function that writes the content of the memory protect variable that
 * enables overwritten of non secure memory when system boots.
 ******************************************************************************/
int arm_nor_psci_write_mem_protect(int val)
{
	int enable = (val != 0);

	if (nor_unlock(PLAT_ARM_MEM_PROT_ADDR) != 0) {
		ERROR("unlocking memory protect variable\n");
		return -1;
	}

	if (enable) {
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
void arm_nor_psci_do_mem_protect(void)
{
	int enable;

	arm_psci_read_mem_protect(&enable);
	if (!enable)
		return;
	INFO("PSCI: Overwritting non secure memory\n");
	clear_mem_regions(arm_ram_ranges, ARRAY_SIZE(arm_ram_ranges));
	arm_nor_psci_write_mem_protect(0);
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

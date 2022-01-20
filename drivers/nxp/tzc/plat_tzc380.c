/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <plat_tzc380.h>

#pragma weak populate_tzc380_reg_list

#ifdef DEFAULT_TZASC_CONFIG
/*
 * Typical Memory map of DRAM0
 *    |-----------NXP_NS_DRAM_ADDR ( = NXP_DRAM0_ADDR)----------|
 *    |								|
 *    |								|
 *    |			Non-SECURE REGION			|
 *    |								|
 *    |								|
 *    |								|
 *    |------- (NXP_NS_DRAM_ADDR + NXP_NS_DRAM_SIZE - 1) -------|
 *    |-----------------NXP_SECURE_DRAM_ADDR--------------------|
 *    |								|
 *    |								|
 *    |								|
 *    |			SECURE REGION (= 64MB)			|
 *    |								|
 *    |								|
 *    |								|
 *    |--- (NXP_SECURE_DRAM_ADDR + NXP_SECURE_DRAM_SIZE - 1)----|
 *    |-----------------NXP_SP_SHRD_DRAM_ADDR-------------------|
 *    |								|
 *    |	       Secure EL1 Payload SHARED REGION (= 2MB)         |
 *    |								|
 *    |-----------(NXP_DRAM0_ADDR + NXP_DRAM0_SIZE - 1)---------|
 *
 *
 *
 * Typical Memory map of DRAM1
 *    |---------------------NXP_DRAM1_ADDR----------------------|
 *    |								|
 *    |								|
 *    |			Non-SECURE REGION			|
 *    |								|
 *    |								|
 *    |---(NXP_DRAM1_ADDR + Dynamically calculated Size - 1) ---|
 *
 *
 * Typical Memory map of DRAM2
 *    |---------------------NXP_DRAM2_ADDR----------------------|
 *    |								|
 *    |								|
 *    |			Non-SECURE REGION			|
 *    |								|
 *    |								|
 *    |---(NXP_DRAM2_ADDR + Dynamically calculated Size - 1) ---|
 */

/*****************************************************************************
 * This function sets up access permissions on memory regions
 *
 * Input:
 *	tzc380_reg_list	: TZC380 Region List
 *	dram_idx	: DRAM index
 *	list_idx	: TZC380 Region List Index
 *	dram_start_addr	: Start address of DRAM at dram_idx.
 *	dram_size	: Size of DRAM at dram_idx.
 *	secure_dram_sz	: Secure DRAM Size
 *	shrd_dram_sz	: Shared DRAM Size
 *
 * Out:
 *	list_idx	: last populated index + 1
 *
 ****************************************************************************/
int populate_tzc380_reg_list(struct tzc380_reg *tzc380_reg_list,
			     int dram_idx, int list_idx,
			     uint64_t dram_start_addr,
			     uint64_t dram_size,
			     uint32_t secure_dram_sz,
			     uint32_t shrd_dram_sz)
{
	/* Region 0: Default region marked as Non-Secure */
	if (list_idx == 0) {
		tzc380_reg_list[list_idx].secure = TZC_ATTR_SP_NS_RW;
		tzc380_reg_list[list_idx].enabled = TZC_ATTR_REGION_DISABLE;
		tzc380_reg_list[list_idx].addr = UL(0x0);
		tzc380_reg_list[list_idx].size = 0x0;
		tzc380_reg_list[list_idx].sub_mask = 0x0; /* all enabled */
		list_idx++;
	}
	/* Continue with list entries for index > 0 */
	if (dram_idx == 0) {
		/* TZC Region 1 on DRAM0 for Secure Memory*/
		tzc380_reg_list[list_idx].secure = TZC_ATTR_SP_S_RW;
		tzc380_reg_list[list_idx].enabled = TZC_ATTR_REGION_ENABLE;
		tzc380_reg_list[list_idx].addr = dram_start_addr + dram_size;
		tzc380_reg_list[list_idx].size = secure_dram_sz;
		tzc380_reg_list[list_idx].sub_mask = 0x0; /* all enabled */
		list_idx++;

		/* TZC Region 2 on DRAM0 for Shared Memory*/
		tzc380_reg_list[list_idx].secure = TZC_ATTR_SP_S_RW;
		tzc380_reg_list[list_idx].enabled = TZC_ATTR_REGION_ENABLE;
		tzc380_reg_list[list_idx].addr = dram_start_addr + dram_size + secure_dram_sz;
		tzc380_reg_list[list_idx].size = shrd_dram_sz;
		tzc380_reg_list[list_idx].sub_mask = 0x0; /* all enabled */
		list_idx++;

	}

	return list_idx;
}
#else
int populate_tzc380_reg_list(struct tzc380_reg *tzc380_reg_list,
			     int dram_idx, int list_idx,
			     uint64_t dram_start_addr,
			     uint64_t dram_size,
			     uint32_t secure_dram_sz,
			     uint32_t shrd_dram_sz)
{
	ERROR("tzc380_reg_list used is not a default list\n");
	ERROR("%s needs to be over-written.\n", __func__);
	return 0;
}
#endif	/* DEFAULT_TZASC_CONFIG */


void mem_access_setup(uintptr_t base, uint32_t total_regions,
			struct tzc380_reg *tzc380_reg_list)
{
	uint32_t indx = 0;
	unsigned int attr_value;

	VERBOSE("Configuring TrustZone Controller tzc380\n");

	tzc380_init(base);

	tzc380_set_action(TZC_ACTION_NONE);

	for (indx = 0; indx < total_regions; indx++) {
		attr_value = tzc380_reg_list[indx].secure |
			TZC_ATTR_SUBREG_DIS(tzc380_reg_list[indx].sub_mask) |
			TZC_ATTR_REGION_SIZE(tzc380_reg_list[indx].size) |
			tzc380_reg_list[indx].enabled;

		tzc380_configure_region(indx, tzc380_reg_list[indx].addr,
				attr_value);
	}

	tzc380_set_action(TZC_ACTION_ERR);
}

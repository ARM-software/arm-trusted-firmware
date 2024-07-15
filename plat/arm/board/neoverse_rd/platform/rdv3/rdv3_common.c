/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/gic600_multichip.h>
#include <drivers/arm/rse_comms.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <nrd_plat.h>
#include <rdv3_mhuv3.h>
#include <rdv3_rse_comms.h>

unsigned int plat_arm_nrd_get_platform_id(void)
{
	return mmio_read_32(SID_REG_BASE + SID_SYSTEM_ID_OFFSET) &
	       SID_SYSTEM_ID_PART_NUM_MASK;
}

unsigned int plat_arm_nrd_get_config_id(void)
{
	return mmio_read_32(SID_REG_BASE + SID_SYSTEM_CFG_OFFSET);
}

unsigned int plat_arm_nrd_get_multi_chip_mode(void)
{
	return (mmio_read_32(SID_REG_BASE + SID_NODE_ID_OFFSET) &
		SID_MULTI_CHIP_MODE_MASK) >> SID_MULTI_CHIP_MODE_SHIFT;
}

/*
 * Get a pointer to the RMM-EL3 shared buffer and return it
 * through the pointer passed as parameter.
 *
 * This function returns the size of the shared buffer.
 */
size_t plat_rmmd_get_el3_rmm_shared_mem(uintptr_t *shared)
{
	*shared = (uintptr_t)RMM_SHARED_BASE;

	return (size_t)RMM_SHARED_SIZE;
}

int plat_rmmd_load_manifest(struct rmm_manifest *manifest)
{
	uint64_t checksum, num_banks, num_consoles;
	struct ns_dram_bank *bank_ptr;
	struct console_info *console_ptr;

	assert(manifest != NULL);

	/* DRAM Bank-1 and Bank-2 */
	num_banks = 2;
	assert(num_banks <= ARM_DRAM_NUM_BANKS);

	/* Set number of consoles */
	num_consoles = NRD_CSS_RMM_CONSOLE_COUNT;

	manifest->version = RMMD_MANIFEST_VERSION;
	manifest->padding = 0U; /* RES0 */
	manifest->plat_data = (uintptr_t)NULL;
	manifest->plat_dram.num_banks = num_banks;
	manifest->plat_console.num_consoles = num_consoles;

	/*
	 * Boot Manifest structure illustration, with two dram banks and
	 * a single console.
	 *
	 * +----------------------------------------+
	 * | offset |     field      |    comment   |
	 * +--------+----------------+--------------+
	 * |   0    |    version     |  0x00000003  |
	 * +--------+----------------+--------------+
	 * |   4    |    padding     |  0x00000000  |
	 * +--------+----------------+--------------+
	 * |   8    |   plat_data    |     NULL     |
	 * +--------+----------------+--------------+
	 * |   16   |   num_banks    |              |
	 * +--------+----------------+              |
	 * |   24   |     banks      |   plat_dram  |
	 * +--------+----------------+              |
	 * |   32   |    checksum    |              |
	 * +--------+----------------+--------------+
	 * |   40   |  num_consoles  |              |
	 * +--------+----------------+              |
	 * |   48   |    consoles    | plat_console |
	 * +--------+----------------+              |
	 * |   56   |    checksum    |              |
	 * +--------+----------------+--------------+
	 * |   64   |     base 0     |              |
	 * +--------+----------------+    bank[0]   |
	 * |   72   |     size 0     |              |
	 * +--------+----------------+--------------+
	 * |   80   |     base 1     |              |
	 * +--------+----------------+    bank[1]   |
	 * |   88   |     size 1     |              |
	 * +--------+----------------+--------------+
	 * |   96   |     base       |              |
	 * +--------+----------------+              |
	 * |   104  |   map_pages    |              |
	 * +--------+----------------+              |
	 * |   112  |     name       |              |
	 * +--------+----------------+  consoles[0] |
	 * |   120  |   clk_in_hz    |              |
	 * +--------+----------------+              |
	 * |   128  |   baud_rate    |              |
	 * +--------+----------------+              |
	 * |   136  |     flags      |              |
	 * +--------+----------------+--------------+
	 */

	bank_ptr = (struct ns_dram_bank *)
			(((uintptr_t)manifest) + sizeof(*manifest));
	console_ptr = (struct console_info *)
			((uintptr_t)bank_ptr + (num_banks * sizeof(*bank_ptr)));

	manifest->plat_dram.banks = bank_ptr;
	manifest->plat_console.consoles = console_ptr;

	/* Ensure the manifest is not larger than the shared buffer */
	assert((sizeof(struct rmm_manifest) +
		(sizeof(struct console_info) *
		manifest->plat_console.num_consoles) +
		(sizeof(struct ns_dram_bank) * manifest->plat_dram.num_banks))
		<= ARM_EL3_RMM_SHARED_SIZE);

	/* Calculate checksum of plat_dram structure */
	checksum = num_banks + (uint64_t)bank_ptr;

	/* Store FVP DRAM banks data in Boot Manifest */
	bank_ptr[0].base = ARM_NS_DRAM1_BASE;
	bank_ptr[0].size = ARM_NS_DRAM1_SIZE;

	bank_ptr[1].base = ARM_DRAM2_BASE;
	bank_ptr[1].size = ARM_DRAM2_SIZE;

	/* Update checksum */
	checksum += bank_ptr[0].base + bank_ptr[0].size + bank_ptr[1].base +
		bank_ptr[1].size;

	/* Checksum must be 0 */
	manifest->plat_dram.checksum = ~checksum + 1UL;

	/* Calculate the checksum of the plat_consoles structure */
	checksum = num_consoles + (uint64_t)console_ptr;

	/* Zero out the console info struct */
	memset((void *)console_ptr, '\0',
		sizeof(struct console_info) * num_consoles);

	console_ptr[0].map_pages = 1;
	console_ptr[0].base = NRD_CSS_RMM_CONSOLE_BASE;
	console_ptr[0].clk_in_hz = NRD_CSS_RMM_CONSOLE_CLK_IN_HZ;
	console_ptr[0].baud_rate = NRD_CSS_RMM_CONSOLE_BAUD;

	strlcpy(console_ptr[0].name, NRD_CSS_RMM_CONSOLE_NAME,
		sizeof(console_ptr[0].name));

	/* Update checksum */
	checksum += console_ptr[0].base + console_ptr[0].map_pages +
		console_ptr[0].clk_in_hz + console_ptr[0].baud_rate;

	/* Checksum must be 0 */
	manifest->plat_console.checksum = ~checksum + 1UL;

	return 0;
}

int plat_rse_comms_init(void)
{
	uintptr_t snd_base, rcv_base;

	/* Get sender and receiver frames for AP-RSE communication */
	mhu_v3_get_secure_device_base(&snd_base, true);
	mhu_v3_get_secure_device_base(&rcv_base, false);

	VERBOSE("Initializing the rse_comms now\n");
	/* Initialize the communication channel between AP and RSE */
	return rse_comms_init(snd_base, rcv_base);
}

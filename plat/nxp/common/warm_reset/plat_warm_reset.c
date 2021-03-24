/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <errno.h>

#include <common/debug.h>
#include <ddr.h>
#ifndef NXP_COINED_BB
#include <flash_info.h>
#include <fspi.h>
#include <fspi_api.h>
#endif
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#ifdef NXP_COINED_BB
#include <snvs.h>
#endif

#include <plat_nv_storage.h>
#include "plat_warm_rst.h"
#include "platform_def.h"

#if defined(IMAGE_BL2)

uint32_t is_warm_boot(void)
{
	uint32_t ret = mmio_read_32(NXP_RESET_ADDR + RST_RSTRQSR1_OFFSET)
				& ~(RSTRQSR1_SWRR);

	const nv_app_data_t *nv_app_data = get_nv_data();

	if (ret == 0U) {
		INFO("Not a SW(Warm) triggered reset.\n");
		return 0U;
	}

	ret = (nv_app_data->warm_rst_flag == WARM_BOOT_SUCCESS) ? 1 : 0;

	if (ret != 0U) {
		INFO("Warm Reset was triggered..\n");
	} else {
		INFO("Warm Reset was not triggered..\n");
	}

	return ret;
}

#endif

#if defined(IMAGE_BL31)
int prep_n_execute_warm_reset(void)
{
#ifdef NXP_COINED_BB
#if !TRUSTED_BOARD_BOOT
	snvs_disable_zeroize_lp_gpr();
#endif
#else
	int ret;
	uint8_t warm_reset = WARM_BOOT_SUCCESS;

	ret = fspi_init(NXP_FLEXSPI_ADDR, NXP_FLEXSPI_FLASH_ADDR);

	if (ret != 0) {
		ERROR("Failed to initialized driver flexspi-nor.\n");
		ERROR("exiting warm-reset request.\n");
		return PSCI_E_INTERN_FAIL;
	}

	/* Sector starting from NV_STORAGE_BASE_ADDR is already
	 * erased for writing.
	 */

#if (ERLY_WRM_RST_FLG_FLSH_UPDT)
	ret = xspi_write((uint32_t)NV_STORAGE_BASE_ADDR,
			 &warm_reset,
			 sizeof(warm_reset));
#else
	/* Preparation for writing the Warm reset flag. */
	ret = xspi_wren((uint32_t)NV_STORAGE_BASE_ADDR);

	/* IP Control Register0 - SF Address to be read */
	fspi_out32((NXP_FLEXSPI_ADDR + FSPI_IPCR0),
		   (uint32_t) NV_STORAGE_BASE_ADDR);

	while ((fspi_in32(NXP_FLEXSPI_ADDR + FSPI_INTR) &
		FSPI_INTR_IPTXWE_MASK) == 0) {
		;
	}
	/* Write TX FIFO Data Register */
	fspi_out32(NXP_FLEXSPI_ADDR + FSPI_TFDR, (uint32_t) warm_reset);

	fspi_out32(NXP_FLEXSPI_ADDR + FSPI_INTR, FSPI_INTR_IPTXWE);

	/* IP Control Register1 - SEQID_WRITE operation, Size = 1 Byte */
	fspi_out32(NXP_FLEXSPI_ADDR + FSPI_IPCR1,
		   (uint32_t)(FSPI_WRITE_SEQ_ID << FSPI_IPCR1_ISEQID_SHIFT) |
		   (uint16_t) sizeof(warm_reset));

	/* Trigger XSPI-IP-Write cmd only if:
	 *  - Putting DDR in-self refresh mode is successfully.
	 *    to complete the writing of the warm-reset flag
	 *    to flash.
	 *
	 * This code is as part of assembly.
	 */
#endif
#endif
	INFO("Doing DDR Self refresh.\n");
	_soc_sys_warm_reset();

	/* Expected behaviour is to do the power cycle */
	while (1 != 0)
		;

	return -1;
}
#endif

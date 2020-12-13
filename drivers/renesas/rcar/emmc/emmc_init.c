/*
 * Copyright (c) 2015-2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <lib/mmio.h>

#include "emmc_config.h"
#include "emmc_hal.h"
#include "emmc_std.h"
#include "emmc_registers.h"
#include "emmc_def.h"
#include "rcar_private.h"

st_mmc_base mmc_drv_obj;

EMMC_ERROR_CODE rcar_emmc_memcard_power(uint8_t mode)
{

	if (mode == TRUE) {
		/* power on (Vcc&Vccq is always power on) */
		mmc_drv_obj.card_power_enable = TRUE;
	} else {
		/* power off (Vcc&Vccq is always power on) */
		mmc_drv_obj.card_power_enable = FALSE;
		mmc_drv_obj.mount = FALSE;
		mmc_drv_obj.selected = FALSE;
	}

	return EMMC_SUCCESS;
}
static inline void emmc_set_retry_count(uint32_t retry)
{
	mmc_drv_obj.retries_after_fail = retry;
}

static inline void emmc_set_data_timeout(uint32_t data_timeout)
{
	mmc_drv_obj.data_timeout = data_timeout;
}

static void emmc_memset(uint8_t *buff, uint8_t data, uint32_t cnt)
{
	if (buff == NULL) {
		return;
	}

	while (cnt > 0) {
		*buff++ = data;
		cnt--;
	}
}

static void emmc_driver_config(void)
{
	emmc_set_retry_count(EMMC_RETRY_COUNT);
	emmc_set_data_timeout(EMMC_RW_DATA_TIMEOUT);
}

static void emmc_drv_init(void)
{
	emmc_memset((uint8_t *) (&mmc_drv_obj), 0, sizeof(st_mmc_base));
	mmc_drv_obj.card_present = HAL_MEMCARD_CARD_IS_IN;
	mmc_drv_obj.data_timeout = EMMC_RW_DATA_TIMEOUT;
	mmc_drv_obj.bus_width = HAL_MEMCARD_DATA_WIDTH_1_BIT;
}

static EMMC_ERROR_CODE emmc_dev_finalize(void)
{
	EMMC_ERROR_CODE result;
	uint32_t dataL;

	/*
	 * MMC power off
	 * the power supply of eMMC device is always turning on.
	 * RST_n : Hi --> Low level.
	 */
	result = rcar_emmc_memcard_power(FALSE);

	/* host controller reset */
	SETR_32(SD_INFO1, 0x00000000U);		/* all interrupt clear */
	SETR_32(SD_INFO2, SD_INFO2_CLEAR);	/* all interrupt clear */
	SETR_32(SD_INFO1_MASK, 0x00000000U);	/* all interrupt disable */
	SETR_32(SD_INFO2_MASK, SD_INFO2_CLEAR);	/* all interrupt disable */
	SETR_32(SD_CLK_CTRL, 0x00000000U);	/* MMC clock stop */

	dataL = mmio_read_32(CPG_SMSTPCR3);
	if ((dataL & CPG_MSTP_MMC) == 0U) {
		dataL |= (CPG_MSTP_MMC);
		mmio_write_32(CPG_CPGWPR, (~dataL));
		mmio_write_32(CPG_SMSTPCR3, dataL);
	}

	return result;
}

static EMMC_ERROR_CODE emmc_dev_init(void)
{
	/* Enable clock supply to eMMC. */
	mstpcr_write(CPG_SMSTPCR3, CPG_MSTPSR3, CPG_MSTP_MMC);

	/* Set SD clock */
	mmio_write_32(CPG_CPGWPR, ~((uint32_t) (BIT9 | BIT0)));	/* SD phy 200MHz */

	/* Stop SDnH clock & SDn=200MHz */
	mmio_write_32(CPG_SDxCKCR, (BIT9 | BIT0));

	/* MMCIF initialize */
	SETR_32(SD_INFO1, 0x00000000U);		/* all interrupt clear */
	SETR_32(SD_INFO2, SD_INFO2_CLEAR);	/* all interrupt clear */
	SETR_32(SD_INFO1_MASK, 0x00000000U);	/* all interrupt disable */
	SETR_32(SD_INFO2_MASK, SD_INFO2_CLEAR);	/* all interrupt disable */

	SETR_32(HOST_MODE, 0x00000000U);	/* SD_BUF access width = 64-bit */
	SETR_32(SD_OPTION, 0x0000C0EEU);	/* Bus width = 1bit, timeout=MAX */
	SETR_32(SD_CLK_CTRL, 0x00000000U);	/* Disable Automatic Control & Clock Output */

	return EMMC_SUCCESS;
}

static EMMC_ERROR_CODE emmc_reset_controller(void)
{
	EMMC_ERROR_CODE result;

	/* initialize mmc driver */
	emmc_drv_init();

	/* initialize H/W */
	result = emmc_dev_init();
	if (result == EMMC_SUCCESS) {
		mmc_drv_obj.initialize = TRUE;
	}

	return result;

}

EMMC_ERROR_CODE emmc_terminate(void)
{
	EMMC_ERROR_CODE result;

	result = emmc_dev_finalize();

	emmc_memset((uint8_t *) (&mmc_drv_obj), 0, sizeof(st_mmc_base));

	return result;
}

EMMC_ERROR_CODE rcar_emmc_init(void)
{
	EMMC_ERROR_CODE result;

	result = emmc_reset_controller();
	if (result == EMMC_SUCCESS) {
		emmc_driver_config();
	}

	return result;
}

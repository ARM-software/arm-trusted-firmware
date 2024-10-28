/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/cadence/cdns_combo_phy.h>
#include <drivers/cadence/cdns_sdmmc.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils.h>

#include "sdmmc.h"
#include "socfpga_mailbox.h"
#include "wdt/watchdog.h"

static const struct mmc_ops *ops;
static unsigned int mmc_ocr_value;
static unsigned int mmc_flags;
static unsigned int rca;

extern const struct mmc_ops cdns_sdmmc_ops;
extern struct cdns_sdmmc_params cdns_params;
extern struct cdns_sdmmc_combo_phy sdmmc_combo_phy_reg;
extern struct cdns_sdmmc_sdhc sdmmc_sdhc_reg;

bool is_cmd23_enabled(void)
{
	return ((mmc_flags & MMC_FLAG_CMD23) != 0U);
}

int sdmmc_send_cmd(unsigned int idx, unsigned int arg,
			unsigned int r_type, unsigned int *r_data)
{
	struct mmc_cmd cmd;
	int ret;

	zeromem(&cmd, sizeof(struct mmc_cmd));

	cmd.cmd_idx = idx;
	cmd.cmd_arg = arg;
	cmd.resp_type = r_type;

	ret = ops->send_cmd(&cmd);

	if ((ret == 0) && (r_data != NULL)) {
		int i;

		for (i = 0; i < 4; i++) {
			*r_data = cmd.resp_data[i];
			r_data++;
		}
	}

	if (ret != 0) {
		VERBOSE("Send command %u error: %d\n", idx, ret);
	}

	return ret;
}

int sdmmc_device_state(void)
{
	int retries = DEFAULT_SDMMC_MAX_RETRIES;
	unsigned int resp_data[4];

	do {
		int ret;

		if (retries == 0) {
			ERROR("CMD13 failed after %d retries\n",
			      DEFAULT_SDMMC_MAX_RETRIES);
			return -EIO;
		}

		ret = sdmmc_send_cmd(MMC_CMD(13), rca << RCA_SHIFT_OFFSET,
				   MMC_RESPONSE_R1, &resp_data[0]);
		if (ret != 0) {
			retries--;
			continue;
		}

		if ((resp_data[0] & STATUS_SWITCH_ERROR) != 0U) {
			return -EIO;
		}

		retries--;
	} while ((resp_data[0] & STATUS_READY_FOR_DATA) == 0U);

	return MMC_GET_STATE(resp_data[0]);
}

size_t sdmmc_read_blocks(int lba, uintptr_t buf, size_t size)
{
	mmc_read_blocks(lba, buf, size);

	/* Restart watchdog for reading each chunk byte */
	watchdog_sw_rst();

	return size;
}

size_t sdmmc_write_blocks(int lba, const uintptr_t buf, size_t size)
{
	int ret;
	unsigned int cmd_idx, cmd_arg;

	assert((ops != NULL) &&
	       (ops->write != NULL) &&
	       (size != 0U) &&
	       ((buf & MMC_BLOCK_MASK) == 0U) &&
	       ((size & MMC_BLOCK_MASK) == 0U));

	ret = ops->prepare(lba, buf, size);
	if (ret != 0) {
		return 0;
	}

	if (is_cmd23_enabled()) {
		/* Set block count */
		ret = sdmmc_send_cmd(MMC_CMD(23), size / MMC_BLOCK_SIZE,
				   MMC_RESPONSE_R1, NULL);
		if (ret != 0) {
			return 0;
		}

		cmd_idx = MMC_CMD(25);
	} else {
		if (size > MMC_BLOCK_SIZE) {
			cmd_idx = MMC_CMD(25);
		} else {
			cmd_idx = MMC_CMD(24);
		}
	}

	if ((mmc_ocr_value & OCR_ACCESS_MODE_MASK) == OCR_BYTE_MODE) {
		cmd_arg = lba * MMC_BLOCK_SIZE;
	} else {
		cmd_arg = lba;
	}

	ret = sdmmc_send_cmd(cmd_idx, cmd_arg, MMC_RESPONSE_R1, NULL);
	if (ret != 0) {
		return 0;
	}

	ret = ops->write(lba, buf, size);
	if (ret != 0) {
		return 0;
	}

	/* Wait buffer empty */
	do {
		ret = sdmmc_device_state();
		if (ret < 0) {
			return 0;
		}
	} while ((ret != MMC_STATE_TRAN) && (ret != MMC_STATE_RCV));

	if (!is_cmd23_enabled() && (size > MMC_BLOCK_SIZE)) {
		ret = sdmmc_send_cmd(MMC_CMD(12), 0, MMC_RESPONSE_R1B, NULL);
		if (ret != 0) {
			return 0;
		}
	}

	return size;
}

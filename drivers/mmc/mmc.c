/*
 * Defines a simple and generic interface to access eMMC device.
 *
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
#include <debug.h>
#include <errno.h>
#include <mmc.h>
#include <string.h>

static const mmc_ops_t *ops;
static unsigned int mmc_ocr_value;
static mmc_csd_t mmc_csd;

static int mmc_device_state(void)
{
	mmc_cmd_t cmd;
	int ret;

	do {
		memset(&cmd, 0, sizeof(mmc_cmd_t));
		cmd.cmd_idx = MMC_CMD13;
		cmd.cmd_arg = MMC_FIX_RCA << RCA_SHIFT_OFFSET;
		cmd.resp_type = MMC_RESPONSE_R1;
		ret = ops->send_cmd(&cmd);
		assert(ret == 0);
		assert((cmd.resp_data[0] & STATUS_SWITCH_ERROR) == 0);
	} while ((cmd.resp_data[0] & STATUS_READY_FOR_DATA) == 0);
	return MMC_GET_STATE(cmd.resp_data[0]);
}

static void mmc_set_ext_csd(unsigned int ext_cmd, unsigned int value)
{
	mmc_cmd_t cmd;
	int ret, state;

	memset(&cmd, 0, sizeof(mmc_cmd_t));
	cmd.cmd_idx = MMC_CMD6;
	cmd.cmd_arg = EXTCSD_WRITE_BYTES | EXTCSD_CMD(ext_cmd) |
		      EXTCSD_VALUE(value) | 1;
	ret = ops->send_cmd(&cmd);
	assert(ret == 0);

	/* wait to exit PRG state */
	do {
		state = mmc_device_state();
	} while (state == MMC_STATE_PRG);
}

static void mmc_set_ios(int clk, int bus_width)
{
	int ret;

	/* set IO speed & IO bus width */
	if (mmc_csd.spec_vers == 4)
		mmc_set_ext_csd(CMD_EXTCSD_BUS_WIDTH, bus_width);
	ret = ops->set_ios(clk, bus_width);
	assert(ret == 0);
}

static int mmc_enumerate(int clk, int bus_width)
{
	mmc_cmd_t cmd;
	int ret, state;

	ops->init();

	/* CMD0: reset to IDLE */
	memset(&cmd, 0, sizeof(mmc_cmd_t));
	cmd.cmd_idx = MMC_CMD0;
	ret = ops->send_cmd(&cmd);
	assert(ret == 0);

	while (1) {
		/* CMD1: get OCR register */
		memset(&cmd, 0, sizeof(mmc_cmd_t));
		cmd.cmd_idx = MMC_CMD1;
		cmd.cmd_arg = OCR_SECTOR_MODE | OCR_VDD_MIN_2V7 |
			      OCR_VDD_MIN_1V7;
		cmd.resp_type = MMC_RESPONSE_R3;
		ret = ops->send_cmd(&cmd);
		assert(ret == 0);
		mmc_ocr_value = cmd.resp_data[0];
		if (mmc_ocr_value & OCR_POWERUP)
			break;
	}

	/* CMD2: Card Identification */
	memset(&cmd, 0, sizeof(mmc_cmd_t));
	cmd.cmd_idx = MMC_CMD2;
	cmd.resp_type = MMC_RESPONSE_R2;
	ret = ops->send_cmd(&cmd);
	assert(ret == 0);

	/* CMD3: Set Relative Address */
	memset(&cmd, 0, sizeof(mmc_cmd_t));
	cmd.cmd_idx = MMC_CMD3;
	cmd.cmd_arg = MMC_FIX_RCA << RCA_SHIFT_OFFSET;
	cmd.resp_type = MMC_RESPONSE_R1;
	ret = ops->send_cmd(&cmd);
	assert(ret == 0);

	/* CMD9: CSD Register */
	memset(&cmd, 0, sizeof(mmc_cmd_t));
	cmd.cmd_idx = MMC_CMD9;
	cmd.cmd_arg = MMC_FIX_RCA << RCA_SHIFT_OFFSET;
	cmd.resp_type = MMC_RESPONSE_R2;
	ret = ops->send_cmd(&cmd);
	assert(ret == 0);
	memcpy(&mmc_csd, &cmd.resp_data, sizeof(cmd.resp_data));

	/* CMD7: Select Card */
	memset(&cmd, 0, sizeof(mmc_cmd_t));
	cmd.cmd_idx = MMC_CMD7;
	cmd.cmd_arg = MMC_FIX_RCA << RCA_SHIFT_OFFSET;
	cmd.resp_type = MMC_RESPONSE_R1;
	ret = ops->send_cmd(&cmd);
	assert(ret == 0);
	/* wait to TRAN state */
	do {
		state = mmc_device_state();
	} while (state != MMC_STATE_TRAN);

	mmc_set_ios(clk, bus_width);
	return ret;
}

size_t mmc_read_blocks(int lba, uintptr_t buf, size_t size)
{
	mmc_cmd_t cmd;
	int ret;

	assert((ops != 0) &&
	       (ops->read != 0) &&
	       ((buf & MMC_BLOCK_MASK) == 0) &&
	       ((size & MMC_BLOCK_MASK) == 0));

	inv_dcache_range(buf, size);
	ret = ops->prepare(lba, buf, size);
	assert(ret == 0);

	memset(&cmd, 0, sizeof(mmc_cmd_t));
	if (size > MMC_BLOCK_SIZE)
		cmd.cmd_idx = MMC_CMD18;
	else
		cmd.cmd_idx = MMC_CMD17;
	if ((mmc_ocr_value & OCR_ACCESS_MODE_MASK) == OCR_BYTE_MODE)
		cmd.cmd_arg = lba * MMC_BLOCK_SIZE;
	else
		cmd.cmd_arg = lba;
	cmd.resp_type = MMC_RESPONSE_R1;
	ret = ops->send_cmd(&cmd);
	assert(ret == 0);

	ret = ops->read(lba, buf, size);
	assert(ret == 0);

	/* wait buffer empty */
	mmc_device_state();

	if (size > MMC_BLOCK_SIZE) {
		memset(&cmd, 0, sizeof(mmc_cmd_t));
		cmd.cmd_idx = MMC_CMD12;
		ret = ops->send_cmd(&cmd);
		assert(ret == 0);
	}

	return size;
}

size_t mmc_write_blocks(int lba, uintptr_t buf, size_t size)
{
	mmc_cmd_t cmd;
	int ret;

	assert((ops != 0) &&
	       (ops->write != 0) &&
	       ((buf & MMC_BLOCK_MASK) == 0) &&
	       ((size & MMC_BLOCK_MASK) == 0));

	clean_dcache_range(buf, size);
	ret = ops->prepare(lba, buf, size);
	assert(ret == 0);

	memset(&cmd, 0, sizeof(mmc_cmd_t));
	if (size > MMC_BLOCK_SIZE)
		cmd.cmd_idx = MMC_CMD25;
	else
		cmd.cmd_idx = MMC_CMD24;
	if ((mmc_ocr_value & OCR_ACCESS_MODE_MASK) == OCR_BYTE_MODE)
		cmd.cmd_arg = lba * MMC_BLOCK_SIZE;
	else
		cmd.cmd_arg = lba;
	cmd.resp_type = MMC_RESPONSE_R1;
	ret = ops->send_cmd(&cmd);
	assert(ret == 0);

	ret = ops->write(lba, buf, size);
	assert(ret == 0);

	/* wait buffer empty */
	mmc_device_state();

	if (size > MMC_BLOCK_SIZE) {
		memset(&cmd, 0, sizeof(mmc_cmd_t));
		cmd.cmd_idx = MMC_CMD12;
		ret = ops->send_cmd(&cmd);
		assert(ret == 0);
	}
	return size;
}

size_t mmc_erase_blocks(int lba, size_t size)
{
	mmc_cmd_t cmd;
	int ret, state;

	assert(ops != 0);
	assert((size != 0) && ((size % MMC_BLOCK_SIZE) == 0));

	memset(&cmd, 0, sizeof(mmc_cmd_t));
	cmd.cmd_idx = MMC_CMD35;
	cmd.cmd_arg = lba;
	cmd.resp_type = MMC_RESPONSE_R1;
	ret = ops->send_cmd(&cmd);
	assert(ret == 0);

	memset(&cmd, 0, sizeof(mmc_cmd_t));
	cmd.cmd_idx = MMC_CMD36;
	cmd.cmd_arg = lba + (size / MMC_BLOCK_SIZE) - 1;
	cmd.resp_type = MMC_RESPONSE_R1;
	ret = ops->send_cmd(&cmd);
	assert(ret == 0);

	memset(&cmd, 0, sizeof(mmc_cmd_t));
	cmd.cmd_idx = MMC_CMD38;
	cmd.resp_type = MMC_RESPONSE_R1B;
	ret = ops->send_cmd(&cmd);
	assert(ret == 0);

	/* wait to TRAN state */
	do {
		state = mmc_device_state();
	} while (state != MMC_STATE_TRAN);

	return size;
}

static inline void mmc_rpmb_enable(void)
{
	mmc_set_ext_csd(CMD_EXTCSD_PARTITION_CONFIG,
			PART_CFG_BOOT_PARTITION1_ENABLE |
			PART_CFG_PARTITION1_ACCESS);
}

static inline void mmc_rpmb_disable(void)
{
	mmc_set_ext_csd(CMD_EXTCSD_PARTITION_CONFIG,
			PART_CFG_BOOT_PARTITION1_ENABLE);
}

size_t mmc_rpmb_read_blocks(int lba, uintptr_t buf, size_t size)
{
	size_t size_read;

	mmc_rpmb_enable();
	size_read = mmc_read_blocks(lba, buf, size);
	mmc_rpmb_disable();
	return size_read;
}

size_t mmc_rpmb_write_blocks(int lba, uintptr_t buf, size_t size)
{
	size_t size_written;

	mmc_rpmb_enable();
	size_written = mmc_write_blocks(lba, buf, size);
	mmc_rpmb_disable();
	return size_written;
}

size_t mmc_rpmb_erase_blocks(int lba, size_t size)
{
	size_t size_erased;

	mmc_rpmb_enable();
	size_erased = mmc_erase_blocks(lba, size);
	mmc_rpmb_disable();
	return size_erased;
}

void mmc_init(const mmc_ops_t *ops_ptr, int clk, int width)
{
	assert((ops_ptr != 0) &&
	       (ops_ptr->init != 0) &&
	       (ops_ptr->send_cmd != 0) &&
	       (ops_ptr->set_ios != 0) &&
	       (ops_ptr->prepare != 0) &&
	       (ops_ptr->read != 0) &&
	       (ops_ptr->write != 0) &&
	       (clk != 0) &&
	       ((width == MMC_BUS_WIDTH_1) ||
		(width == MMC_BUS_WIDTH_4) ||
		(width == MMC_BUS_WIDTH_8)));
	ops = ops_ptr;

	mmc_enumerate(clk, width);
}

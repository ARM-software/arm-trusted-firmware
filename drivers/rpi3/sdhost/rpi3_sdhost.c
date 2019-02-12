/*
 * Copyright (c) 2019, Linaro Limited
 * Copyright (c) 2019, Ying-Chun Liu (PaulLiu) <paul.liu@linaro.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <drivers/delay_timer.h>
#include <drivers/rpi3/sdhost/rpi3_sdhost.h>
#include <drivers/mmc.h>
#include <drivers/rpi3/gpio/rpi3_gpio.h>
#include <errno.h>
#include <string.h>

static void rpi3_sdhost_initialize(void);
static int rpi3_sdhost_send_cmd(struct mmc_cmd *cmd);
static int rpi3_sdhost_set_ios(unsigned int clk, unsigned int width);
static int rpi3_sdhost_prepare(int lba, uintptr_t buf, size_t size);
static int rpi3_sdhost_read(int lba, uintptr_t buf, size_t size);
static int rpi3_sdhost_write(int lba, uintptr_t buf, size_t size);

static const struct mmc_ops rpi3_sdhost_ops = {
	.init		= rpi3_sdhost_initialize,
	.send_cmd	= rpi3_sdhost_send_cmd,
	.set_ios	= rpi3_sdhost_set_ios,
	.prepare	= rpi3_sdhost_prepare,
	.read		= rpi3_sdhost_read,
	.write		= rpi3_sdhost_write,
};

static struct rpi3_sdhost_params rpi3_sdhost_params;

/**
 * Wait for command being processed.
 *
 * This function waits the command being processed. It compares
 * the ENABLE flag of the HC_COMMAND register. When ENABLE flag disappeared
 * it means the command is processed by the SDHOST.
 * The timeout is currently 1000*100 us = 100 ms.
 *
 * @return 0: command finished. 1: command timed out.
 */
static int rpi3_sdhost_waitcommand(void)
{
	uintptr_t reg_base = rpi3_sdhost_params.reg_base;

	volatile int timeout = 1000;

	while ((mmio_read_32(reg_base + HC_COMMAND) & HC_CMD_ENABLE)
	       && (--timeout > 0)) {
		udelay(100);
	}

	return ((timeout > 0) ? 0 : (-(ETIMEDOUT)));
}

/**
 * Send the command and argument to the SDHOST
 *
 * This function will wait for the previous command finished. And then
 * clear any error status of previous command. And then
 * send out the command and args. The command will be turned on the ENABLE
 * flag before sending out.
 */
static void send_command_raw(unsigned int cmd, unsigned int arg)
{
	unsigned int status;
	uintptr_t reg_base = rpi3_sdhost_params.reg_base;

	/* wait for previous command finish */
	rpi3_sdhost_waitcommand();

	/* clean error status */
	status = mmio_read_32(reg_base + HC_HOSTSTATUS);
	if (status & HC_HSTST_MASK_ERROR_ALL)
		mmio_write_32(reg_base + HC_HOSTSTATUS, status);

	/* recording the command */
	rpi3_sdhost_params.current_cmd = cmd & HC_CMD_COMMAND_MASK;

	/* send the argument and command */
	mmio_write_32(reg_base + HC_ARGUMENT, arg);
	mmio_write_32(reg_base + HC_COMMAND, cmd | HC_CMD_ENABLE);
}

/**
 * Send the command and argument to the SDHOST, decorated with control
 * flags.
 *
 * This function will use send_command_raw to send the commands to SDHOST.
 * But before sending it will decorate the command with control flags specific
 * to SDHOST.
 */
static void send_command_decorated(unsigned int cmd, unsigned int arg)
{
	unsigned int cmd_flags = 0;

	switch (cmd & HC_CMD_COMMAND_MASK) {
	case MMC_CMD(0):
		cmd_flags |= HC_CMD_RESPONSE_NONE;
		break;
	case MMC_ACMD(51):
		cmd_flags |= HC_CMD_READ;
		break;
	case MMC_CMD(8):
	case MMC_CMD(11):
	case MMC_CMD(17):
	case MMC_CMD(18):
		cmd_flags |= HC_CMD_READ;
		break;
	case MMC_CMD(20):
	case MMC_CMD(24):
	case MMC_CMD(25):
		cmd_flags |= HC_CMD_WRITE;
		break;
	case MMC_CMD(12):
		cmd_flags |= HC_CMD_BUSY;
		break;
	default:
		break;
	}
	send_command_raw(cmd | cmd_flags, arg);
}

/**
 * drains the FIFO on DATA port
 *
 * This function drains any data left in the DATA port.
 */
static void rpi3_drain_fifo(void)
{
	uintptr_t reg_base = rpi3_sdhost_params.reg_base;
	volatile int timeout = 100000;

	rpi3_sdhost_waitcommand();

	while (mmio_read_32(reg_base + HC_HOSTSTATUS) & HC_HSTST_HAVEDATA) {
		mmio_read_32(reg_base + HC_DATAPORT);
		udelay(100);
	}

	while (1) {
		uint32_t edm, fsm;

		edm = mmio_read_32(reg_base + HC_DEBUG);
		fsm = edm & HC_DBG_FSM_MASK;

		if ((fsm == HC_DBG_FSM_IDENTMODE) ||
		    (fsm == HC_DBG_FSM_DATAMODE))
			break;

		if ((fsm == HC_DBG_FSM_READWAIT) ||
		    (fsm == HC_DBG_FSM_WRITESTART1) ||
		    (fsm == HC_DBG_FSM_READDATA)) {
			mmio_write_32(reg_base + HC_DEBUG,
				      edm | HC_DBG_FORCE_DATA_MODE);
			break;
		}

		if (--timeout <= 0) {
			ERROR("rpi3_sdhost: %s cannot recover stat\n",
			      __func__);
			return;
		}
	}
}

/**
 * Dump SDHOST registers
 */
static void rpi3_sdhost_print_regs(void)
{
	uintptr_t reg_base = rpi3_sdhost_params.reg_base;

	INFO("rpi3_sdhost: HC_COMMAND:        0x%08x\n",
	     mmio_read_32(reg_base + HC_COMMAND));
	INFO("rpi3_sdhost: HC_ARGUMENT:       0x%08x\n",
	     mmio_read_32(reg_base + HC_ARGUMENT));
	INFO("rpi3_sdhost: HC_TIMEOUTCOUNTER: 0x%08x\n",
	     mmio_read_32(reg_base + HC_TIMEOUTCOUNTER));
	INFO("rpi3_sdhost: HC_CLOCKDIVISOR:   0x%08x\n",
	     mmio_read_32(reg_base + HC_CLOCKDIVISOR));
	INFO("rpi3_sdhost: HC_RESPONSE_0:     0x%08x\n",
	     mmio_read_32(reg_base + HC_RESPONSE_0));
	INFO("rpi3_sdhost: HC_RESPONSE_1:     0x%08x\n",
	     mmio_read_32(reg_base + HC_RESPONSE_1));
	INFO("rpi3_sdhost: HC_RESPONSE_2:     0x%08x\n",
	     mmio_read_32(reg_base + HC_RESPONSE_2));
	INFO("rpi3_sdhost: HC_RESPONSE_3:     0x%08x\n",
	     mmio_read_32(reg_base + HC_RESPONSE_3));
	INFO("rpi3_sdhost: HC_HOSTSTATUS:     0x%08x\n",
	     mmio_read_32(reg_base + HC_HOSTSTATUS));
	INFO("rpi3_sdhost: HC_POWER:          0x%08x\n",
	     mmio_read_32(reg_base + HC_POWER));
	INFO("rpi3_sdhost: HC_DEBUG:          0x%08x\n",
	     mmio_read_32(reg_base + HC_DEBUG));
	INFO("rpi3_sdhost: HC_HOSTCONFIG:     0x%08x\n",
	     mmio_read_32(reg_base + HC_HOSTCONFIG));
	INFO("rpi3_sdhost: HC_BLOCKSIZE:      0x%08x\n",
	     mmio_read_32(reg_base + HC_BLOCKSIZE));
	INFO("rpi3_sdhost: HC_BLOCKCOUNT:     0x%08x\n",
	     mmio_read_32(reg_base + HC_BLOCKCOUNT));
}

/**
 * Reset SDHOST
 */
static void rpi3_sdhost_reset(void)
{
	uintptr_t reg_base = rpi3_sdhost_params.reg_base;
	unsigned int dbg;
	uint32_t tmp1;

	mmio_write_32(reg_base + HC_POWER, 0);
	mmio_write_32(reg_base + HC_COMMAND, 0);
	mmio_write_32(reg_base + HC_ARGUMENT, 0);

	mmio_write_32(reg_base + HC_TIMEOUTCOUNTER, HC_TIMEOUT_DEFAULT);
	mmio_write_32(reg_base + HC_CLOCKDIVISOR, 0);
	mmio_write_32(reg_base + HC_HOSTSTATUS, HC_HSTST_RESET);
	mmio_write_32(reg_base + HC_HOSTCONFIG, 0);
	mmio_write_32(reg_base + HC_BLOCKSIZE, 0);
	mmio_write_32(reg_base + HC_BLOCKCOUNT, 0);

	dbg = mmio_read_32(reg_base + HC_DEBUG);
	dbg &= ~((HC_DBG_FIFO_THRESH_MASK << HC_DBG_FIFO_THRESH_READ_SHIFT) |
		 (HC_DBG_FIFO_THRESH_MASK << HC_DBG_FIFO_THRESH_WRITE_SHIFT));
	dbg |= (HC_FIFO_THRESH_READ << HC_DBG_FIFO_THRESH_READ_SHIFT) |
		(HC_FIFO_THRESH_WRITE << HC_DBG_FIFO_THRESH_WRITE_SHIFT);
	mmio_write_32(reg_base + HC_DEBUG, dbg);
	mdelay(250);
	mmio_write_32(reg_base + HC_POWER, 1);
	mdelay(250);
	rpi3_sdhost_params.clk_rate = 0;

	mmio_write_32(reg_base + HC_CLOCKDIVISOR, HC_CLOCKDIVISOR_MAXVAL);
	tmp1 = mmio_read_32(reg_base + HC_HOSTCONFIG);
	mmio_write_32(reg_base + HC_HOSTCONFIG, tmp1 | HC_HSTCF_INT_BUSY);
}

static void rpi3_sdhost_initialize(void)
{
	uintptr_t reg_base = rpi3_sdhost_params.reg_base;

	assert((rpi3_sdhost_params.reg_base & MMC_BLOCK_MASK) == 0);

	rpi3_sdhost_reset();

	mmio_write_32(reg_base + HC_CLOCKDIVISOR, HC_CLOCKDIVISOR_PREFERVAL);
	udelay(300);
}

static int rpi3_sdhost_send_cmd(struct mmc_cmd *cmd)
{
	uintptr_t reg_base = rpi3_sdhost_params.reg_base;
	int err = 0;
	uint32_t cmd_idx;
	uint32_t cmd_arg;
	uint32_t cmd_flags = 0;
	uint32_t intmask;

	/* Wait for the command done */
	err = rpi3_sdhost_waitcommand();
	if (err != 0) {
		WARN("previous command not done yet\n");
		return err;
	}

	cmd_idx = cmd->cmd_idx & HC_CMD_COMMAND_MASK;

	cmd_arg = cmd->cmd_arg;
	if (cmd_idx == MMC_ACMD(51)) {
		/* if previous cmd send to SDHOST is not MMC_CMD(55).
		 * It means this MMC_ACMD(51) is a resend.
		 * And we must also resend MMC_CMD(55) in this case
		 */
		if (rpi3_sdhost_params.current_cmd != MMC_CMD(55)) {
			send_command_decorated(
				MMC_CMD(55),
				rpi3_sdhost_params.sdcard_rca <<
				RCA_SHIFT_OFFSET);
			rpi3_sdhost_params.mmc_app_cmd = 1;
			rpi3_sdhost_waitcommand();

			/* Also we need to call prepare to clean the buffer */
			rpi3_sdhost_prepare(0, (uintptr_t)NULL, 8);
		}
	}

	/* We ignore MMC_CMD(12) sending from the TF-A's MMC driver
	 * because we send MMC_CMD(12) by ourselves.
	 */
	if (cmd_idx == MMC_CMD(12))
		return 0;

	if ((cmd->resp_type & MMC_RSP_136) &&
	    (cmd->resp_type & MMC_RSP_BUSY)) {
		ERROR("rpi3_sdhost: unsupported response type!\n");
		return -(EOPNOTSUPP);
	}

	if (cmd->resp_type & MMC_RSP_48 && cmd->resp_type != MMC_RESPONSE_R2) {
		/* 48-bit command
		 * We don't need to set any flags here because it is default.
		 */
	} else if (cmd->resp_type & MMC_RSP_136) {
		/* 136-bit command */
		cmd_flags |= HC_CMD_RESPONSE_LONG;
	} else {
		/* no respond command */
		cmd_flags |= HC_CMD_RESPONSE_NONE;
	}

	rpi3_sdhost_params.cmdbusy = 0;
	if (cmd->resp_type & MMC_RSP_BUSY) {
		cmd_flags |= HC_CMD_BUSY;
		rpi3_sdhost_params.cmdbusy = 1;
	}

	if (rpi3_sdhost_params.mmc_app_cmd) {
		switch (cmd_idx) {
		case MMC_ACMD(41):
			if (cmd_arg == OCR_HCS)
				cmd_arg |= OCR_3_3_3_4;
			break;
		default:
			break;
		}
		rpi3_sdhost_params.mmc_app_cmd = 0;
	}

	if (cmd_idx == MMC_CMD(55))
		rpi3_sdhost_params.mmc_app_cmd = 1;

	send_command_decorated(cmd_idx | cmd_flags, cmd_arg);

	intmask = mmio_read_32(reg_base + HC_HOSTSTATUS);
	if (rpi3_sdhost_params.cmdbusy && (intmask & HC_HSTST_INT_BUSY)) {
		mmio_write_32(reg_base + HC_HOSTSTATUS, HC_HSTST_INT_BUSY);
		rpi3_sdhost_params.cmdbusy = 0;
	}

	if (!(cmd_flags & HC_CMD_RESPONSE_NONE)) {
		err = rpi3_sdhost_waitcommand();
		if (err != 0)
			ERROR("rpi3_sdhost: cmd cannot be finished\n");
	}

	cmd->resp_data[0] = mmio_read_32(reg_base + HC_RESPONSE_0);
	cmd->resp_data[1] = mmio_read_32(reg_base + HC_RESPONSE_1);
	cmd->resp_data[2] = mmio_read_32(reg_base + HC_RESPONSE_2);
	cmd->resp_data[3] = mmio_read_32(reg_base + HC_RESPONSE_3);

	if (mmio_read_32(reg_base + HC_COMMAND) & HC_CMD_FAILED) {
		uint32_t sdhsts = mmio_read_32(reg_base + HC_HOSTSTATUS);

		mmio_write_32(reg_base + HC_HOSTSTATUS,
			      HC_HSTST_MASK_ERROR_ALL);

		/*
		 * If the command SEND_OP_COND returns with CRC7 error,
		 * it can be considered as having completed successfully.
		 */
		if (!(sdhsts & HC_HSTST_ERROR_CRC7)
		    || (cmd_idx != MMC_CMD(1))) {
			if (sdhsts & HC_HSTST_TIMEOUT_CMD) {
				ERROR("rpi3_sdhost: timeout status 0x%x\n",
				      sdhsts);
				err = -(ETIMEDOUT);
			} else {
				ERROR("rpi3_sdhost: unknown err, cmd = 0x%x\n",
				      mmio_read_32(reg_base + HC_COMMAND));
				ERROR("rpi3_sdhost status: 0x%x\n", sdhsts);
				err = -(EILSEQ);
			}
		}
	}

	if ((!err) && (cmd_idx == MMC_CMD(3))) {
		/* we keep the RCA in case to send MMC_CMD(55) ourselves */
		rpi3_sdhost_params.sdcard_rca = (cmd->resp_data[0]
						 & 0xFFFF0000U) >> 16;
	}

	return err;
}

static int rpi3_sdhost_set_clock(unsigned int clk)
{
	uintptr_t reg_base = rpi3_sdhost_params.reg_base;
	uint32_t max_clk = 250000000;
	uint32_t div;

	if (clk < 100000) {
		mmio_write_32(reg_base + HC_CLOCKDIVISOR,
			      HC_CLOCKDIVISOR_MAXVAL);
		return 0;
	}

	div = max_clk / clk;
	if (div < 2)
		div = 2;

	if ((max_clk / div) > clk)
		div++;

	div -= 2;
	if (div > HC_CLOCKDIVISOR_MAXVAL)
		div = HC_CLOCKDIVISOR_MAXVAL;

	rpi3_sdhost_params.clk_rate = max_clk / (div + 2);
	rpi3_sdhost_params.ns_per_fifo_word = (1000000000 /
					       rpi3_sdhost_params.clk_rate)
		* 8;

	mmio_write_32(reg_base + HC_CLOCKDIVISOR, div);
	return 0;
}

static int rpi3_sdhost_set_ios(unsigned int clk, unsigned int width)
{
	uintptr_t reg_base = rpi3_sdhost_params.reg_base;
	uint32_t tmp1;

	rpi3_sdhost_set_clock(clk);
	VERBOSE("rpi3_sdhost: Changing clock to %dHz for data mode\n", clk);

	if (width != MMC_BUS_WIDTH_4 && width != MMC_BUS_WIDTH_1) {
		ERROR("rpi3_sdhost: width %d not supported\n", width);
		return -(EOPNOTSUPP);
	}
	rpi3_sdhost_params.bus_width = width;

	tmp1 = mmio_read_32(reg_base + HC_HOSTCONFIG);
	tmp1 &= ~(HC_HSTCF_EXTBUS_4BIT);
	if (rpi3_sdhost_params.bus_width == MMC_BUS_WIDTH_4)
		tmp1 |= HC_HSTCF_EXTBUS_4BIT;

	mmio_write_32(reg_base + HC_HOSTCONFIG, tmp1);
	tmp1 = mmio_read_32(reg_base + HC_HOSTCONFIG);
	mmio_write_32(reg_base + HC_HOSTCONFIG, tmp1 |
		      HC_HSTCF_SLOW_CARD | HC_HSTCF_INTBUS_WIDE);

	return 0;
}

static int rpi3_sdhost_prepare(int lba, uintptr_t buf, size_t size)
{
	uintptr_t reg_base = rpi3_sdhost_params.reg_base;
	size_t blocks;
	size_t blocksize;

	if (size < 512) {
		blocksize = size;
		blocks = 1;
	} else {
		blocksize = 512;
		blocks = size / blocksize;
		if (size % blocksize != 0)
			blocks++;
	}

	rpi3_drain_fifo();

	mmio_write_32(reg_base + HC_BLOCKSIZE, blocksize);
	mmio_write_32(reg_base + HC_BLOCKCOUNT, blocks);
	udelay(100);
	return 0;
}

static int rpi3_sdhost_read(int lba, uintptr_t buf, size_t size)
{
	int err = 0;
	uint32_t *buf1 = ((uint32_t *) buf);
	uintptr_t reg_base = rpi3_sdhost_params.reg_base;
	int timeout = 100000;
	int remaining_words = 0;

	for (int i = 0; i < size / 4; i++) {
		volatile int t = timeout;
		uint32_t hsts_err;

		while ((mmio_read_32(reg_base + HC_HOSTSTATUS)
			& HC_HSTST_HAVEDATA) == 0) {
			if (t == 0) {
				ERROR("rpi3_sdhost: fifo timeout after %dus\n",
				      timeout);
				err = -(ETIMEDOUT);
				break;
			}
			t--;
			udelay(10);
		}
		if (t == 0)
			break;

		uint32_t data = mmio_read_32(reg_base + HC_DATAPORT);

		hsts_err = mmio_read_32(reg_base + HC_HOSTSTATUS)
			& HC_HSTST_MASK_ERROR_ALL;
		if (hsts_err) {
			ERROR("rpi3_sdhost: transfer FIFO word %d: 0x%x\n",
			      i,
			      mmio_read_32(reg_base + HC_HOSTSTATUS));
			rpi3_sdhost_print_regs();

			err = -(EILSEQ);

			/* clean the error status */
			mmio_write_32(reg_base + HC_HOSTSTATUS, hsts_err);
		}

		if (buf1)
			buf1[i] = data;

		/* speeding up if the remaining words are still a lot */
		remaining_words = (mmio_read_32(reg_base + HC_DEBUG) >> 4)
			& HC_DBG_FIFO_THRESH_MASK;
		if (remaining_words >= 7)
			continue;

		/* delay. slowing down the read process */
		udelay(100);
	}

	/* We decide to stop by ourselves.
	 * It is because MMC_CMD(18) -> MMC_CMD(13) -> MMC_CMD(12)
	 * doesn't work for RPi3 SDHost.
	 */
	if (rpi3_sdhost_params.current_cmd == MMC_CMD(18))
		send_command_decorated(MMC_CMD(12), 0);

	return err;
}

static int rpi3_sdhost_write(int lba, uintptr_t buf, size_t size)
{
	uint32_t *buf1 = ((uint32_t *) buf);
	uintptr_t reg_base = rpi3_sdhost_params.reg_base;
	int err = 0;
	int remaining_words = 0;

	for (int i = 0; i < size / 4; i++) {
		uint32_t hsts_err;
		uint32_t data = buf1[i];
		uint32_t dbg;
		uint32_t fsm_state;

		mmio_write_32(reg_base + HC_DATAPORT, data);

		dbg = mmio_read_32(reg_base + HC_DEBUG);
		fsm_state = dbg & HC_DBG_FSM_MASK;
		if (fsm_state != HC_DBG_FSM_WRITEDATA
		    && fsm_state != HC_DBG_FSM_WRITESTART1
		    && fsm_state != HC_DBG_FSM_WRITESTART2
		    && fsm_state != HC_DBG_FSM_WRITECRC
		    && fsm_state != HC_DBG_FSM_WRITEWAIT1
		    && fsm_state != HC_DBG_FSM_WRITEWAIT2) {
			hsts_err = mmio_read_32(reg_base + HC_HOSTSTATUS)
				& HC_HSTST_MASK_ERROR_ALL;
			if (hsts_err)
				err = -(EILSEQ);
		}

		/* speeding up if the remaining words are not many */
		remaining_words = (mmio_read_32(reg_base + HC_DEBUG) >> 4)
			& HC_DBG_FIFO_THRESH_MASK;
		if (remaining_words <= 4)
			continue;

		udelay(100);
	}

	/* We decide to stop by ourselves.
	 * It is because MMC_CMD(25) -> MMC_CMD(13) -> MMC_CMD(12)
	 * doesn't work for RPi3 SDHost.
	 */
	if (rpi3_sdhost_params.current_cmd == MMC_CMD(25))
		send_command_decorated(MMC_CMD(12), 0);

	return err;
}

void rpi3_sdhost_init(struct rpi3_sdhost_params *params,
		    struct mmc_device_info *mmc_dev_info)
{
	assert((params != 0) &&
	       ((params->reg_base & MMC_BLOCK_MASK) == 0));

	memcpy(&rpi3_sdhost_params, params, sizeof(struct rpi3_sdhost_params));

	/* backup GPIO 48 to 53 configurations */
	for (int i = 48; i <= 53; i++) {
		rpi3_sdhost_params.gpio48_pinselect[i - 48]
			= rpi3_gpio_get_select(i);
		VERBOSE("rpi3_sdhost: Original GPIO state %d: %d\n",
			i,
			rpi3_sdhost_params.gpio48_pinselect[i - 48]);
	}

	/* setting pull resistors for 48 to 53.
	 * It is debatable to set SD_CLK to UP or NONE. We massively
	 * tested different brands of SD Cards and found NONE works
	 * most stable.
	 *
	 * GPIO 48 (SD_CLK) to GPIO_PULL_NONE
	 * GPIO 49 (SD_CMD) to GPIO_PULL_UP
	 * GPIO 50 (SD_D0)  to GPIO_PULL_UP
	 * GPIO 51 (SD_D1)  to GPIO_PULL_UP
	 * GPIO 52 (SD_D2)  to GPIO_PULL_UP
	 * GPIO 53 (SD_D3)  to GPIO_PULL_UP
	 */
	gpio_set_pull(48, GPIO_PULL_NONE);
	for (int i = 49; i <= 53; i++)
		gpio_set_pull(i, GPIO_PULL_UP);

	/* Set pin 48-53 to alt-0. It means route SDHOST to card slot */
	for (int i = 48; i <= 53; i++)
		rpi3_gpio_set_select(i, RPI3_GPIO_FUNC_ALT0);

	mmc_init(&rpi3_sdhost_ops, params->clk_rate, params->bus_width,
		 params->flags, mmc_dev_info);
}

void rpi3_sdhost_stop(void)
{
	uintptr_t reg_base = rpi3_sdhost_params.reg_base;

	VERBOSE("rpi3_sdhost: Shutting down: drain FIFO out\n");
	rpi3_drain_fifo();

	VERBOSE("rpi3_sdhost: Shutting down: slowing down the clock\n");
	mmio_write_32(reg_base+HC_CLOCKDIVISOR, HC_CLOCKDIVISOR_SLOWVAL);
	udelay(500);

	VERBOSE("rpi3_sdhost: Shutting down: put SDHost into idle state\n");
	send_command_decorated(MMC_CMD(0), 0);
	udelay(500);

	mmio_write_32(reg_base + HC_COMMAND, 0);
	mmio_write_32(reg_base + HC_ARGUMENT, 0);
	mmio_write_32(reg_base + HC_TIMEOUTCOUNTER, HC_TIMEOUT_IDLE);
	mmio_write_32(reg_base + HC_CLOCKDIVISOR, HC_CLOCKDIVISOR_STOPVAL);

	udelay(100);

	mmio_write_32(reg_base + HC_POWER, 0);
	mmio_write_32(reg_base + HC_HOSTCONFIG, 0);
	mmio_write_32(reg_base + HC_BLOCKSIZE, 0x400);
	mmio_write_32(reg_base + HC_BLOCKCOUNT, 0);
	mmio_write_32(reg_base + HC_HOSTSTATUS, 0x7f8);

	mmio_write_32(reg_base + HC_COMMAND, 0);
	mmio_write_32(reg_base + HC_ARGUMENT, 0);

	udelay(100);

	/* Restore the pinmux to original state */
	for (int i = 48; i <= 53; i++) {
		rpi3_gpio_set_select(i,
				     rpi3_sdhost_params.gpio48_pinselect[i-48]);
	}

	/* Reset the pull resistors before entering BL33.
	 * GPIO 48 (SD_CLK) to GPIO_PULL_UP
	 * GPIO 49 (SD_CMD) to GPIO_PULL_UP
	 * GPIO 50 (SD_D0)  to GPIO_PULL_UP
	 * GPIO 51 (SD_D1)  to GPIO_PULL_UP
	 * GPIO 52 (SD_D2)  to GPIO_PULL_UP
	 * GPIO 53 (SD_D3)  to GPIO_PULL_UP
	 */
	for (int i = 48; i <= 53; i++)
		gpio_set_pull(i, GPIO_PULL_UP);
}

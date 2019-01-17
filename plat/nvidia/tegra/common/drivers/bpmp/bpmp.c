/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bpmp.h>
#include <common/debug.h>
#include <delay_timer.h>
#include <errno.h>
#include <mmio.h>
#include <platform.h>
#include <stdbool.h>
#include <string.h>
#include <tegra_def.h>

#define BPMP_TIMEOUT_10US	10

static uint32_t channel_base[NR_CHANNELS];
static uint32_t bpmp_init_state = BPMP_INIT_PENDING;

static uint32_t channel_field(unsigned int ch)
{
	return mmio_read_32(TEGRA_RES_SEMA_BASE + STA_OFFSET) & CH_MASK(ch);
}

static bool master_free(unsigned int ch)
{
	return channel_field(ch) == MA_FREE(ch);
}

static bool master_acked(unsigned int ch)
{
	return channel_field(ch) == MA_ACKD(ch);
}

static void signal_slave(unsigned int ch)
{
	mmio_write_32(TEGRA_RES_SEMA_BASE + CLR_OFFSET, CH_MASK(ch));
}

static void free_master(unsigned int ch)
{
	mmio_write_32(TEGRA_RES_SEMA_BASE + CLR_OFFSET,
		      MA_ACKD(ch) ^ MA_FREE(ch));
}

/* should be called with local irqs disabled */
int32_t tegra_bpmp_send_receive_atomic(int mrq, const void *ob_data, int ob_sz,
		void *ib_data, int ib_sz)
{
	unsigned int ch = (unsigned int)plat_my_core_pos();
	mb_data_t *p = (mb_data_t *)(uintptr_t)channel_base[ch];
	int32_t ret = -ETIMEDOUT, timeout = 0;

	if (bpmp_init_state == BPMP_INIT_COMPLETE) {

		/* loop until BPMP is free */
		for (timeout = 0; timeout < BPMP_TIMEOUT_10US; timeout++) {
			if (master_free(ch) == true) {
				break;
			}

			udelay(1);
		}

		if (timeout != BPMP_TIMEOUT_10US) {

			/* generate the command struct */
			p->code = mrq;
			p->flags = DO_ACK;
			(void)memcpy((void *)p->data, ob_data, (size_t)ob_sz);

			/* signal command ready to the BPMP */
			signal_slave(ch);
			mmio_write_32(TEGRA_PRI_ICTLR_BASE + CPU_IEP_FIR_SET,
				      (1UL << INT_SHR_SEM_OUTBOX_FULL));

			/* loop until the command is executed */
			for (timeout = 0; timeout < BPMP_TIMEOUT_10US; timeout++) {
				if (master_acked(ch) == true) {
					break;
				}

				udelay(1);
			}

			if (timeout != BPMP_TIMEOUT_10US) {

				/* get the command response */
				(void)memcpy(ib_data, (const void *)p->data,
					     (size_t)ib_sz);

				/* return error code */
				ret = p->code;

				/* free this channel */
				free_master(ch);
			}
		}

	} else {
		/* return error code */
		ret = -EINVAL;
	}

	if (timeout == BPMP_TIMEOUT_10US) {
		ERROR("Timed out waiting for bpmp's response");
	}

	return ret;
}

int tegra_bpmp_init(void)
{
	uint32_t val, base;
	unsigned int ch;
	int ret = 0;

	if (bpmp_init_state != BPMP_INIT_COMPLETE) {

		/* check if the bpmp processor is alive. */
		val = mmio_read_32(TEGRA_RES_SEMA_BASE + STA_OFFSET);
		if (val != SIGN_OF_LIFE) {
			ERROR("BPMP precessor not available\n");
			ret = -ENOTSUP;
		}

		/* check if clock for the atomics block is enabled */
		val = mmio_read_32(TEGRA_CAR_RESET_BASE + TEGRA_CLK_ENB_V);
		if ((val & CAR_ENABLE_ATOMICS) == 0) {
			ERROR("Clock to the atomics block is disabled\n");
		}

		/* check if the atomics block is out of reset */
		val = mmio_read_32(TEGRA_CAR_RESET_BASE + TEGRA_RST_DEV_CLR_V);
		if ((val & CAR_ENABLE_ATOMICS) == CAR_ENABLE_ATOMICS) {
			ERROR("Reset to the atomics block is asserted\n");
		}

		/* base address to get the result from Atomics */
		base = TEGRA_ATOMICS_BASE + RESULT0_REG_OFFSET;

		/* channel area is setup by BPMP before signaling handshake */
		for (ch = 0; ch < NR_CHANNELS; ch++) {

			/* issue command to get the channel base address */
			mmio_write_32(base, (ch << TRIGGER_ID_SHIFT) |
				      ATOMIC_CMD_GET);

			/* get the base address for the channel */
			channel_base[ch] = mmio_read_32(base);

			/* increment result register offset */
			base += 4UL;
		}

		/* mark state as "initialized" */
		if (ret == 0)
			bpmp_init_state = BPMP_INIT_COMPLETE;

		/* the channel values have to be visible across all cpus */
		flush_dcache_range((uint64_t)channel_base, sizeof(channel_base));
		flush_dcache_range((uint64_t)&bpmp_init_state,
				   sizeof(bpmp_init_state));

		INFO("%s: done\n", __func__);
	}

	return ret;
}

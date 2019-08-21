/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <sspm_reg.h>
#include <mtk_mcdi.h>

static inline uint32_t mcdi_mbox_read(uint32_t id)
{
	return mmio_read_32(SSPM_MBOX_3_BASE + (id << 2));
}

static inline void mcdi_mbox_write(uint32_t id, uint32_t val)
{
	mmio_write_32(SSPM_MBOX_3_BASE + (id << 2), val);
}

void sspm_set_bootaddr(uint32_t bootaddr)
{
	mcdi_mbox_write(MCDI_MBOX_BOOTADDR, bootaddr);
}

void sspm_cluster_pwr_off_notify(uint32_t cluster)
{
	mcdi_mbox_write(MCDI_MBOX_CLUSTER_0_ATF_ACTION_DONE + cluster, 1);
}

void sspm_cluster_pwr_on_notify(uint32_t cluster)
{
	mcdi_mbox_write(MCDI_MBOX_CLUSTER_0_ATF_ACTION_DONE + cluster, 0);
}

void sspm_standbywfi_irq_enable(uint32_t cpu_idx)
{
	mmio_write_32(SSPM_CFGREG_ACAO_INT_SET, STANDBYWFI_EN(cpu_idx));
}

uint32_t mcdi_avail_cpu_mask_read(void)
{
	return mcdi_mbox_read(MCDI_MBOX_AVAIL_CPU_MASK);
}

uint32_t mcdi_avail_cpu_mask_write(uint32_t mask)
{
	mcdi_mbox_write(MCDI_MBOX_AVAIL_CPU_MASK, mask);

	return mask;
}

uint32_t mcdi_avail_cpu_mask_set(uint32_t mask)
{
	uint32_t m;

	m = mcdi_mbox_read(MCDI_MBOX_AVAIL_CPU_MASK);
	m |= mask;
	mcdi_mbox_write(MCDI_MBOX_AVAIL_CPU_MASK, m);

	return m;
}

uint32_t mcdi_avail_cpu_mask_clr(uint32_t mask)
{
	uint32_t m;

	m = mcdi_mbox_read(MCDI_MBOX_AVAIL_CPU_MASK);
	m &= ~mask;
	mcdi_mbox_write(MCDI_MBOX_AVAIL_CPU_MASK, m);

	return m;
}

uint32_t mcdi_cpu_cluster_pwr_stat_read(void)
{
	return mcdi_mbox_read(MCDI_MBOX_CPU_CLUSTER_PWR_STAT);
}

#define PAUSE_BIT		1
#define CLUSTER_OFF_OFS		20
#define CPU_OFF_OFS		24
#define CLUSTER_ON_OFS		4
#define CPU_ON_OFS		8

static uint32_t target_mask(int cluster, int cpu_idx, bool on)
{
	uint32_t t = 0;

	if (on) {
		if (cluster >= 0)
			t |= BIT(cluster + CLUSTER_ON_OFS);

		if (cpu_idx >= 0)
			t |= BIT(cpu_idx + CPU_ON_OFS);
	} else {
		if (cluster >= 0)
			t |= BIT(cluster + CLUSTER_OFF_OFS);

		if (cpu_idx >= 0)
			t |= BIT(cpu_idx + CPU_OFF_OFS);
	}

	return t;
}

void mcdi_pause_clr(int cluster, int cpu_idx, bool on)
{
	uint32_t tgt = target_mask(cluster, cpu_idx, on);
	uint32_t m = mcdi_mbox_read(MCDI_MBOX_PAUSE_ACTION);

	m &= ~tgt;
	mcdi_mbox_write(MCDI_MBOX_PAUSE_ACTION, m);
}

void mcdi_pause_set(int cluster, int cpu_idx, bool on)
{
	uint32_t tgt = target_mask(cluster, cpu_idx, on);
	uint32_t m = mcdi_mbox_read(MCDI_MBOX_PAUSE_ACTION);
	uint32_t tgtn = target_mask(-1, cpu_idx, !on);

	/* request on and off at the same time to ensure it can be paused */
	m |= tgt | tgtn;
	mcdi_mbox_write(MCDI_MBOX_PAUSE_ACTION, m);

	/* wait pause_ack */
	while (!mcdi_mbox_read(MCDI_MBOX_PAUSE_ACK))
		;

	/* clear non-requested operation */
	m &= ~tgtn;
	mcdi_mbox_write(MCDI_MBOX_PAUSE_ACTION, m);
}

void mcdi_pause(void)
{
	uint32_t m = mcdi_mbox_read(MCDI_MBOX_PAUSE_ACTION) | BIT(PAUSE_BIT);

	mcdi_mbox_write(MCDI_MBOX_PAUSE_ACTION, m);

	/* wait pause_ack */
	while (!mcdi_mbox_read(MCDI_MBOX_PAUSE_ACK))
		;
}

void mcdi_unpause(void)
{
	uint32_t m = mcdi_mbox_read(MCDI_MBOX_PAUSE_ACTION) & ~BIT(PAUSE_BIT);

	mcdi_mbox_write(MCDI_MBOX_PAUSE_ACTION, m);
}

void mcdi_hotplug_wait_ack(int cluster, int cpu_idx, bool on)
{
	uint32_t tgt = target_mask(cluster, cpu_idx, on);
	uint32_t ack = mcdi_mbox_read(MCDI_MBOX_HP_ACK);

	/* wait until ack */
	while (!(ack & tgt))
		ack = mcdi_mbox_read(MCDI_MBOX_HP_ACK);
}

void mcdi_hotplug_clr(int cluster, int cpu_idx, bool on)
{
	uint32_t tgt = target_mask(cluster, cpu_idx, on);
	uint32_t tgt_cpu = target_mask(-1, cpu_idx, on);
	uint32_t cmd = mcdi_mbox_read(MCDI_MBOX_HP_CMD);
	uint32_t ack = mcdi_mbox_read(MCDI_MBOX_HP_ACK);

	if (!(cmd & tgt))
		return;

	/* wait until ack */
	while (!(ack & tgt_cpu))
		ack = mcdi_mbox_read(MCDI_MBOX_HP_ACK);

	cmd &= ~tgt;
	mcdi_mbox_write(MCDI_MBOX_HP_CMD, cmd);
}

void mcdi_hotplug_set(int cluster, int cpu_idx, bool on)
{
	uint32_t tgt = target_mask(cluster, cpu_idx, on);
	uint32_t tgt_cpu = target_mask(-1, cpu_idx, on);
	uint32_t cmd = mcdi_mbox_read(MCDI_MBOX_HP_CMD);
	uint32_t ack = mcdi_mbox_read(MCDI_MBOX_HP_ACK);

	if ((cmd & tgt) == tgt)
		return;

	/* wait until ack clear */
	while (ack & tgt_cpu)
		ack = mcdi_mbox_read(MCDI_MBOX_HP_ACK);

	cmd |= tgt;
	mcdi_mbox_write(MCDI_MBOX_HP_CMD, cmd);
}

bool check_mcdi_ctl_stat(void)
{
	uint32_t clk_regs[] = {0x100010ac, 0x100010c8};
	uint32_t clk_mask[] = {0x00028000, 0x00000018};
	uint32_t tgt = target_mask(0, 0, true);
	uint32_t m;
	int i;

	/* check clk status */
	for (i = 0; i < ARRAY_SIZE(clk_regs); i++) {
		if (mmio_read_32(clk_regs[i]) & clk_mask[i]) {
			WARN("mcdi: clk check fail.\n");
			return false;
		}
	}

	/* check mcdi cmd handling */
	m = mcdi_mbox_read(MCDI_MBOX_PAUSE_ACTION) | BIT(PAUSE_BIT);
	mcdi_mbox_write(MCDI_MBOX_PAUSE_ACTION, m);

	i = 500;
	while (!mcdi_mbox_read(MCDI_MBOX_PAUSE_ACK) && --i > 0)
		udelay(10);

	m = mcdi_mbox_read(MCDI_MBOX_PAUSE_ACTION) & ~BIT(PAUSE_BIT);
	mcdi_mbox_write(MCDI_MBOX_PAUSE_ACTION, m);

	if (i == 0) {
		WARN("mcdi: pause_action fail.\n");
		return false;
	}

	/* check mcdi cmd handling */
	if (mcdi_mbox_read(MCDI_MBOX_HP_CMD) ||
			mcdi_mbox_read(MCDI_MBOX_HP_ACK)) {
		WARN("mcdi: hp_cmd fail.\n");
		return false;
	}

	mcdi_mbox_write(MCDI_MBOX_HP_CMD, tgt);

	i = 500;
	while ((mcdi_mbox_read(MCDI_MBOX_HP_ACK) & tgt) != tgt && --i > 0)
		udelay(10);

	mcdi_mbox_write(MCDI_MBOX_HP_CMD, 0);

	if (i == 0) {
		WARN("mcdi: hp_ack fail.\n");
		return false;
	}

	return true;
}

void mcdi_init(void)
{
	mcdi_avail_cpu_mask_write(0x01); /* cpu0 default on */
}

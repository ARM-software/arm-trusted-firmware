/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <drivers/mminfra_public.h>
#include <mminfra.h>
#include <mtk_mmap_pool.h>

static const mmap_region_t mminfra_plat_mmap[] MTK_MMAP_SECTION = {
	MAP_REGION_FLAT(MMINFRA_HW_VOTER_BASE, PAGE_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	{0}
};
DECLARE_MTK_MMAP_REGIONS(mminfra_plat_mmap);

static struct mtk_mminfra_pwr_ctrl mminfra_pwr_ctrl = {
	.hw_voter = {
		.base = MMINFRA_HW_VOTER_BASE,
		.set_ofs = 0x104,
		.clr_ofs = 0x108,
		.en_ofs = 0x100,
		.en_shift = 0x1,
		.done_bits = VLP_AO_RSVD6,
	},
	.hw_sema = {
		.base = SPM_BASE,
		.offset = SPM_SEMAPHORE_M1,
		.offset_all = {
			SPM_SEMAPHORE_M0,
			SPM_SEMAPHORE_M1,
			SPM_SEMAPHORE_M2,
			SPM_SEMAPHORE_M3,
			SPM_SEMAPHORE_M4,
			SPM_SEMAPHORE_M5,
			SPM_SEMAPHORE_M6,
			SPM_SEMAPHORE_M7,
		},
		.set_val = SPM_SEMA_MMINFRA,
	},
	.active = true,
	.ref_cnt = 0,
};

static int spm_semaphore_get(uint32_t base, uint32_t set_val)
{
	int  cnt = SEMA_RETRY_CNT;
	uint32_t val;

	val = mmio_read_32(base);
	if ((val & set_val) == set_val) {
		mminfra_err("hw_sem was already got, base:0x%x=0x%x, set_val:0x%x\n",
			    base, val, set_val);
		return -1;
	}

	while (cnt > 0) {
		mmio_write_32(base, set_val);
		udelay(10);
		if ((mmio_read_32(base) & set_val) == set_val)
			return 0;
		cnt--;
	}

	mminfra_err("timeout! base:0x%x, set_val:0x%x\n", base, set_val);
	return -1;
}

static int spm_semaphore_release(uint32_t base, uint32_t set_val)
{
	int cnt = SEMA_RETRY_CNT;
	uint32_t val;

	val = mmio_read_32(base);
	if ((val & set_val) != set_val) {
		mminfra_err("hw_sem was already released, base:0x%x=0x%x, set_val:0x%x\n",
			    base, val, set_val);
		return -1;
	}
	do {
		mmio_write_32(base, set_val);
		udelay(10);
		if (cnt-- < 0) {
			if ((mmio_read_32(base) & set_val) != set_val)
				return 0;
			mminfra_err("timeout! base:0x%x, set_val:0x%x\n", base, set_val);
			return -1;
		}
	} while ((mmio_read_32(base) & set_val) == set_val);

	return 0;
}

static int mminfra_hw_sema_ctrl(struct mminfra_hw_sema *hw_sema, bool is_get)
{
	int i, ret;

	if (!hw_sema)
		return 0;

	if (is_get)
		ret = spm_semaphore_get(hw_sema->base + hw_sema->offset, hw_sema->set_val);
	else
		ret = spm_semaphore_release(hw_sema->base + hw_sema->offset, hw_sema->set_val);

	if (ret)
		for (i = 0; i < SPM_SEMA_MMINFRA_NR; i++)
			mminfra_err("0x%x=0x%x\n", hw_sema->base + hw_sema->offset_all[i],
				    mmio_read_32(hw_sema->base + hw_sema->offset_all[i]));

	return ret;
}

static bool is_mminfra_ready(struct mminfra_hw_voter *hw_voter)
{
	if (!hw_voter)
		return false;

	return !!(mmio_read_32(hw_voter->done_bits) & MMINFRA_DONE);
}

static int mminfra_hwv_power_ctrl(struct mminfra_hw_voter *hw_voter, bool is_on)
{
	uint32_t vote_ofs, vote_mask, vote_ack;
	uint32_t val = 0, cnt;

	vote_mask = BIT(hw_voter->en_shift);
	vote_ofs = is_on ? hw_voter->set_ofs : hw_voter->clr_ofs;
	vote_ack = is_on ? vote_mask : 0x0;

	/* Vote on off */
	cnt = 0;
	do {
		mmio_write_32(hw_voter->base + vote_ofs, vote_mask);
		udelay(MTK_POLL_HWV_VOTE_US);
		val = mmio_read_32(hw_voter->base + hw_voter->en_ofs);
		if ((val & vote_mask) == vote_ack)
			break;

		if (cnt > MTK_POLL_HWV_VOTE_CNT) {
			mminfra_err("vote mminfra timeout, is_on:%d, 0x%x=0x%x\n",
				    is_on, hw_voter->base + hw_voter->en_ofs, val);
			return -1;
		}
		cnt++;
	} while (1);

	if (!is_on)
		return 0;

	/* Confirm done bits */
	cnt = 0;
	while (cnt < MTK_POLL_DONE_RETRY) {
		if (is_mminfra_ready(hw_voter))
			return 0;
		udelay(MTK_POLL_DONE_DELAY_US);
		cnt++;
	}

	mminfra_err("polling mminfra done timeout, 0x%x=0x%x\n",
		    hw_voter->done_bits, val);
	return -1;
}

int mminfra_get_if_in_use(void)
{
	int ret, is_on = MMINFRA_RET_POWER_OFF;

	if (!mminfra_pwr_ctrl.active) {
		mminfra_err("not ready\n");
		return MMINFRA_RET_POWER_OFF;
	}

	spin_lock(&mminfra_pwr_ctrl.lock);
	if (mminfra_pwr_ctrl.ref_cnt > 0) {
		mminfra_pwr_ctrl.ref_cnt++;
		is_on = MMINFRA_RET_POWER_ON;
		spin_unlock(&mminfra_pwr_ctrl.lock);
		return is_on;
	}

	ret = mminfra_hw_sema_ctrl(&mminfra_pwr_ctrl.hw_sema, true);
	if (ret)
		goto err;

	/* Check if mminfra is in use */
	if (is_mminfra_ready(&mminfra_pwr_ctrl.hw_voter)) {
		ret = mminfra_hwv_power_ctrl(&mminfra_pwr_ctrl.hw_voter, true);
		if (ret) {
			mminfra_err("vote for mminfra fail, ret=%d\n", ret);
			goto err;
		}
		mminfra_pwr_ctrl.ref_cnt++;
		is_on = MMINFRA_RET_POWER_ON;
	} else {
		is_on = MMINFRA_RET_POWER_OFF;
	}

	ret = mminfra_hw_sema_ctrl(&mminfra_pwr_ctrl.hw_sema, false);
	if (ret)
		goto err;
	ret = is_on; /* Return power is on or off. */
err:
	spin_unlock(&mminfra_pwr_ctrl.lock);
	return ret;
}

int mminfra_put(void)
{
	if (!mminfra_pwr_ctrl.active) {
		mminfra_err("not ready\n");
		return 0;
	}

	spin_lock(&mminfra_pwr_ctrl.lock);
	mminfra_pwr_ctrl.ref_cnt--;
	if (mminfra_pwr_ctrl.ref_cnt > 0)
		goto out;

	mminfra_hwv_power_ctrl(&mminfra_pwr_ctrl.hw_voter, false);
out:
	spin_unlock(&mminfra_pwr_ctrl.lock);
	return 0;
}

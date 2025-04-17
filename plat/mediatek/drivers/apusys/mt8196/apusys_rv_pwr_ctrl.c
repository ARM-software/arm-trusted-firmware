/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include "apusys_power.h"
#include "apusys_rv.h"
#include "apusys_rv_pwr_ctrl.h"

#define RPC_POWER_OFF_TIMEOUT_CNT	(100000) /* 100ms */

static int wait_for_state_ready(uint32_t reg, uint32_t mask, uint32_t expect,
				uint32_t retry_times, uint32_t set_reg, uint32_t set_val)
{
	uint32_t count = 0;

	while ((mmio_read_32(reg) & mask) != expect) {
		/*
		 * If retry_times == HW_SEM_NO_WAIT, it is just for checking if the hardware
		 * semaphore can be locked or not. The purpose is for SMMU to check NPU power
		 * status. Hence, just returning -EBUSY is okay. There is no need to show any
		 * ERROR message here.
		 */
		if (retry_times == HW_SEM_NO_WAIT) {
			return -EBUSY;
		} else if (count > retry_times) {
			ERROR("%s: timed out, reg = %x, mask = %x, expect = %x\n",
			       __func__, reg, mask, expect);
			return -EBUSY;
		}
		count += 1;

		if (set_reg)
			mmio_write_32(set_reg, set_val);
		udelay(1);
	}

	return 0;
}

int apu_hw_sema_ctl_per_mbox(uint32_t sem_ctrl_addr, uint32_t sem_sta_addr,
			     uint8_t usr_bit, enum apu_hw_sem_op ctl, uint32_t timeout,
			     uint8_t bypass)
{
	int ret;
	uint8_t ctl_bit = 0;

	if (ctl == HW_SEM_GET)
		ctl_bit = 0x1;
	else if (ctl == HW_SEM_PUT)
		ctl_bit = 0x2;
	else
		return -EINVAL;

	/* return fail if semaphore is currently not held by this user */
	if (ctl == HW_SEM_PUT && ((mmio_read_32(sem_sta_addr) & BIT(usr_bit)) == 0)
	    && !bypass) {
		ERROR("%s release error: usr_bit:%d ctl:%d (sem_addr(0x%08x) = 0x%08x)\n",
		       __func__, usr_bit, ctl, sem_sta_addr, mmio_read_32(sem_sta_addr));
		return -EINVAL;
	}

	mmio_write_32(sem_ctrl_addr, ctl_bit);

	if (ctl == HW_SEM_PUT)
		return 0;

	ret = wait_for_state_ready(sem_sta_addr, BIT(usr_bit), BIT(usr_bit), timeout,
				   sem_ctrl_addr, ctl_bit);
	if (ret)
		return ret;

	return 0;
}

int apusys_rv_pwr_ctrl(enum APU_PWR_OP op)
{
	int ret;
	uint32_t global_ref_cnt;

	ret = apu_hw_sema_ctl_per_mbox(APU_MBOX(APU_HW_SEM_SYS_APMCU) + APU_MBOX_SEMA0_CTRL,
				       APU_MBOX(APU_HW_SEM_SYS_APMCU) + APU_MBOX_SEMA0_STA,
				       APU_HW_SEM_SYS_APMCU, HW_SEM_GET, HW_SEM_TIMEOUT, 0);

	if (ret) {
		ERROR("%s(%d): sem acquire timeout\n", __func__, op);
		return ret;
	}

	global_ref_cnt = mmio_read_32(APU_MBOX(APU_HW_SEM_SYS_APMCU) + APU_MBOX_DUMMY);

	if (global_ref_cnt > 2) {
		ERROR("%s: global_ref_cnt(%d) > 2\n", __func__, global_ref_cnt);
	} else if (op == APU_PWR_OFF) {
		global_ref_cnt--;
		mmio_write_32(APU_MBOX(APU_HW_SEM_SYS_APMCU) + APU_MBOX_DUMMY, global_ref_cnt);
		if (global_ref_cnt == 0)
			mmio_write_32(APU_MBOX_WKUP_CFG(11), 0);
	} else if (op == APU_PWR_ON) {
		global_ref_cnt++;
		mmio_write_32(APU_MBOX(APU_HW_SEM_SYS_APMCU) + APU_MBOX_DUMMY, global_ref_cnt);
		if (global_ref_cnt == 1)
			mmio_write_32(APU_MBOX_WKUP_CFG(11), 1);
	}

	ret = apu_hw_sema_ctl_per_mbox(APU_MBOX(APU_HW_SEM_SYS_APMCU) + APU_MBOX_SEMA0_CTRL,
				       APU_MBOX(APU_HW_SEM_SYS_APMCU) + APU_MBOX_SEMA0_STA,
				       APU_HW_SEM_SYS_APMCU, HW_SEM_PUT, HW_SEM_TIMEOUT, 0);

	if (ret)
		ERROR("%s(%d): sem release timeout\n", __func__, op);

	return ret;
}

int rv_iommu_hw_sem_trylock(void)
{
	return apu_hw_sema_ctl_per_mbox(APU_MBOX(APU_HW_SEM_SYS_APMCU) + APU_MBOX_SEMA1_CTRL,
					APU_MBOX(APU_HW_SEM_SYS_APMCU) + APU_MBOX_SEMA1_STA,
					APU_HW_SEM_SYS_APMCU, HW_SEM_GET, HW_SEM_NO_WAIT, 0);
}

int rv_iommu_hw_sem_unlock(void)
{
	return apu_hw_sema_ctl_per_mbox(APU_MBOX(APU_HW_SEM_SYS_APMCU) + APU_MBOX_SEMA1_CTRL,
					APU_MBOX(APU_HW_SEM_SYS_APMCU) + APU_MBOX_SEMA1_STA,
					APU_HW_SEM_SYS_APMCU, HW_SEM_PUT, HW_SEM_NO_WAIT, 0);
}

int apu_hw_sema_ctl(uint32_t sem_addr, uint8_t usr_bit, uint8_t ctl, uint32_t timeout,
		    uint8_t bypass)
{
	int ret;
	uint8_t ctl_bit = 0;

	if (ctl == HW_SEM_GET)
		ctl_bit = usr_bit;
	else if (ctl == HW_SEM_PUT)
		ctl_bit = usr_bit + HW_SEM_PUT_BIT_SHIFT;
	else
		return -EINVAL;

	if (ctl == HW_SEM_PUT && ((mmio_read_32(sem_addr) & BIT(ctl_bit)) == 0) && !bypass) {
		ERROR("%s release error: usr_bit:%d ctl:%d (sem_addr(0x%08x) = 0x%08x)\n",
		       __func__, usr_bit, ctl, sem_addr, mmio_read_32(sem_addr));
		return -EINVAL;
	}

	mmio_write_32(sem_addr, BIT(ctl_bit));

	if (ctl == HW_SEM_PUT)
		goto end;

	ret = wait_for_state_ready(sem_addr, BIT(ctl_bit), BIT(ctl_bit), timeout,
				   sem_addr, BIT(ctl_bit));
	if (ret)
		return ret;

end:
	VERBOSE("%s: sem_addr = 0x%x, usr_bit: %d, ctl: %d, sem_addr = 0x%08x\n",
		 __func__, sem_addr, usr_bit, ctl, mmio_read_32(sem_addr));

	return 0;
}

int apusys_infra_dcm_setup(void)
{
	mmio_write_32(APU_REG_AO_GLUE_CONFG,
		      mmio_read_32(APU_REG_AO_GLUE_CONFG) | BIT(24) | BIT(26));

	return 0;
}

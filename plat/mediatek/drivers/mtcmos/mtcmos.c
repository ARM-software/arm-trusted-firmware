/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <platform_def.h>

#include <mtcmos.h>

#define SPM_PROJECT_CODE	0xB16

#define PWR_RST_B	BIT(0)
#define PWR_ISO		BIT(1)
#define PWR_ON		BIT(2)
#define PWR_ON_2ND	BIT(3)
#define PWR_CLK_DIS	BIT(4)
#define PWR_ACK		BIT(30)
#define PWR_ACK_2ND	BIT(31)

#define SRAM_PDN		BIT(8)
#define SRAM_PDN_ACK		BIT(12)

#define MTCMOS_TIMEOUT_US	500

#define MTCMOS_ETIMEDOUT	25

static spinlock_t mtcmos_ctrl_lock;

static int mtcmos_wait_for_state(uint32_t reg, uint32_t mask, bool is_set)
{
	uint32_t retry = MTCMOS_TIMEOUT_US;
	uint32_t expect = is_set ? mask : 0;

	do {
		if ((mmio_read_32(reg) & mask) == expect)
			return 0;
		udelay(1);
		retry--;
	} while (retry);

	ERROR("%s(0x%x, 0x%x, %d) timeout, reg_val=0x%x\n",
	      __func__, reg, mask, is_set, mmio_read_32(reg));

	return -MTCMOS_ETIMEDOUT;
}


static int spm_mtcmos_ctrl_bus_prot(const struct bus_protect *bp_table, uint32_t bp_steps)
{
	int i;

	for (i = 0; i < bp_steps; i++) {
		mmio_write_32(bp_table[i].en_addr, bp_table[i].mask);
		if (bp_table[i].rdy_addr) {
			if (mtcmos_wait_for_state(bp_table[i].rdy_addr, bp_table[i].mask, true))
				return -MTCMOS_ETIMEDOUT;
		}
	}
	return 0;
}


static int spm_mtcmos_ctrl(enum mtcmos_state state, uintptr_t reg,
			   bool has_sram, int *rtff_save_flag,
			   const struct bus_protect *bp_table, uint32_t bp_steps)
{
	int ret = 0;

	spin_lock(&mtcmos_ctrl_lock);

	mmio_write_32(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | BIT(0));

	if (state == STA_POWER_DOWN) {
		ret = spm_mtcmos_ctrl_bus_prot(bp_table, bp_steps);
		if (ret)
			goto exit;

		if (has_sram) {
			mmio_setbits_32(reg, SRAM_PDN);
			ret = mtcmos_wait_for_state(reg, SRAM_PDN_ACK, true);
			if (ret)
				goto exit;
		}

		mmio_setbits_32(reg, RTFF_CLK_DIS);
		mmio_setbits_32(reg, RTFF_SAVE);
		mmio_clrbits_32(reg, RTFF_SAVE);
		mmio_clrbits_32(reg, RTFF_CLK_DIS);
#ifdef RTFF_SAVE_FLAG
		mmio_setbits_32(reg, RTFF_SAVE_FLAG);
#endif
		*rtff_save_flag = 1;


		mmio_setbits_32(reg, PWR_ISO);
		mmio_setbits_32(reg, PWR_CLK_DIS);
		mmio_clrbits_32(reg, PWR_RST_B);

		mmio_clrbits_32(reg, PWR_ON);
		ret = mtcmos_wait_for_state(reg, PWR_ACK, false);
		if (ret)
			goto exit;

		mmio_clrbits_32(reg, PWR_ON_2ND);
		ret = mtcmos_wait_for_state(reg, PWR_ACK_2ND, false);
		if (ret)
			goto exit;
	} else if (state == STA_POWER_ON) {
		mmio_setbits_32(reg, PWR_ON);
		ret = mtcmos_wait_for_state(reg, PWR_ACK, true);
		if (ret)
			goto exit;

		udelay(50);

		mmio_setbits_32(reg, PWR_ON_2ND);
		ret = mtcmos_wait_for_state(reg, PWR_ACK_2ND, true);
		if (ret)
			goto exit;

		mmio_clrbits_32(reg, PWR_CLK_DIS);
		mmio_clrbits_32(reg, PWR_ISO);
		udelay(10);
		mmio_setbits_32(reg, PWR_RST_B);


		if (*rtff_save_flag == 1) {
			mmio_setbits_32(reg, RTFF_CLK_DIS);
			mmio_clrbits_32(reg, RTFF_NRESTORE);
			mmio_setbits_32(reg, RTFF_NRESTORE);
			mmio_clrbits_32(reg, RTFF_CLK_DIS);
#ifdef RTFF_SAVE_FLAG
			mmio_clrbits_32(reg, RTFF_SAVE_FLAG);
#endif
			*rtff_save_flag = 0;
		}

		if (has_sram) {
			mmio_clrbits_32(reg, SRAM_PDN);
			ret = mtcmos_wait_for_state(reg, SRAM_PDN_ACK, false);
			if (ret)
				goto exit;
		}

		spm_mtcmos_ctrl_bus_prot(bp_table, bp_steps);
	}

exit:
	spin_unlock(&mtcmos_ctrl_lock);
	return ret;
}

int spm_mtcmos_ctrl_ufs0(enum mtcmos_state state)
{
	static int rtff_save_flag;
	const struct bus_protect *bp_table;
	uint32_t bp_steps;

	if (state == STA_POWER_DOWN) {
		bp_table = &ufs0_bus_prot_set_table[0];
		bp_steps = ARRAY_SIZE(ufs0_bus_prot_set_table);
	} else {
		bp_table = &ufs0_bus_prot_clr_table[0];
		bp_steps = ARRAY_SIZE(ufs0_bus_prot_clr_table);
	}

	return spm_mtcmos_ctrl(state, UFS0_PWR_CON, false, &rtff_save_flag,
			       bp_table, bp_steps);
}

int spm_mtcmos_ctrl_ufs0_phy(enum mtcmos_state state)
{
	static int rtff_save_flag;
	const struct bus_protect *bp_table;
	uint32_t bp_steps;

	if (state == STA_POWER_DOWN) {
		bp_table = &ufs0_phy_bus_prot_set_table[0];
		bp_steps = ARRAY_SIZE(ufs0_phy_bus_prot_set_table);
	} else {
		bp_table = &ufs0_phy_bus_prot_clr_table[0];
		bp_steps = ARRAY_SIZE(ufs0_phy_bus_prot_clr_table);
	}

	return spm_mtcmos_ctrl(state, UFS0_PHY_PWR_CON, true, &rtff_save_flag,
			       bp_table, bp_steps);
}

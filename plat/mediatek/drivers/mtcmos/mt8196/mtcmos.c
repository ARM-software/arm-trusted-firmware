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
#define RTFF_SAVE	BIT(24)
#define RTFF_NRESTORE	BIT(25)
#define RTFF_CLK_DIS	BIT(26)
#define RTFF_SAVE_FLAG	BIT(27)
#define PWR_ACK		BIT(30)
#define PWR_ACK_2ND	BIT(31)

#define UFS0_SRAM_PDN		BIT(8)
#define UFS0_SRAM_PDN_ACK	BIT(12)

#define POWERON_CONFIG_EN	(SPM_BASE + 0x0)
#define UFS0_PWR_CON		(SPM_BASE + 0xE2C)
#define UFS0_PHY_PWR_CON	(SPM_BASE + 0xE30)

#define SPM_BUS_PROTECT_EN_SET		(SPM_BASE + 0x90DC)
#define SPM_BUS_PROTECT_EN_CLR		(SPM_BASE + 0x90E0)
#define SPM_BUS_PROTECT_CG_EN_SET	(SPM_BASE + 0x90F4)
#define SPM_BUS_PROTECT_CG_EN_CLR	(SPM_BASE + 0x90F8)
#define SPM_BUS_PROTECT_RDY_STA		(SPM_BASE + 0x9208)

#define UFS0_PROT_STEP1_MASK		BIT(11)
#define UFS0_PHY_PROT_STEP1_MASK	BIT(12)

enum {
	RELEASE_BUS_PROTECT,
	SET_BUS_PROTECT
};

#define MTCMOS_TIMEOUT_US	500

#define ETIMEDOUT	25

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

	return -ETIMEDOUT;
}


static int spm_mtcmos_ctrl_bus_prot(int state, uint32_t mask)
{
	mmio_write_32(SPM_BUS_PROTECT_CG_EN_SET, mask);

	if (state == SET_BUS_PROTECT) {
		mmio_write_32(SPM_BUS_PROTECT_EN_SET, mask);
		if (mtcmos_wait_for_state(SPM_BUS_PROTECT_RDY_STA, mask,
					  true))
			return -ETIMEDOUT;
	} else if (state == RELEASE_BUS_PROTECT) {
		mmio_write_32(SPM_BUS_PROTECT_EN_CLR, mask);
	}

	mmio_write_32(SPM_BUS_PROTECT_CG_EN_CLR, mask);

	return 0;
}

int spm_mtcmos_ctrl(enum mtcmos_state state, uintptr_t reg, uint32_t mask)
{
	int ret = 0;

	spin_lock(&mtcmos_ctrl_lock);

	mmio_write_32(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | BIT(0));

	if (state == STA_POWER_DOWN) {
		ret = spm_mtcmos_ctrl_bus_prot(SET_BUS_PROTECT, mask);
		if (ret)
			goto exit;

		if (reg == UFS0_PWR_CON) {
			mmio_setbits_32(reg, UFS0_SRAM_PDN);
			ret = mtcmos_wait_for_state(reg, UFS0_SRAM_PDN_ACK,
						    true);
			if (ret)
				goto exit;
		}

		mmio_setbits_32(reg, RTFF_CLK_DIS);
		mmio_setbits_32(reg, RTFF_SAVE);
		mmio_clrbits_32(reg, RTFF_SAVE);
		mmio_clrbits_32(reg, RTFF_CLK_DIS);
		mmio_setbits_32(reg, RTFF_SAVE_FLAG);

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

		if ((mmio_read_32(reg) & RTFF_SAVE_FLAG) == RTFF_SAVE_FLAG) {
			mmio_setbits_32(reg, RTFF_CLK_DIS);
			mmio_clrbits_32(reg, RTFF_NRESTORE);
			mmio_setbits_32(reg, RTFF_NRESTORE);
			mmio_clrbits_32(reg, RTFF_CLK_DIS);
		}

		if (reg == UFS0_PWR_CON) {
			mmio_clrbits_32(UFS0_PWR_CON, UFS0_SRAM_PDN);
			ret = mtcmos_wait_for_state(UFS0_PWR_CON,
						    UFS0_SRAM_PDN_ACK,
						    false);
			if (ret)
				goto exit;
		}

		spm_mtcmos_ctrl_bus_prot(RELEASE_BUS_PROTECT, mask);
	}

exit:
	spin_unlock(&mtcmos_ctrl_lock);
	return ret;
}

int spm_mtcmos_ctrl_ufs0(enum mtcmos_state state)
{
	return spm_mtcmos_ctrl(state, UFS0_PWR_CON, UFS0_PROT_STEP1_MASK);
}

int spm_mtcmos_ctrl_ufs0_phy(enum mtcmos_state state)
{
	return spm_mtcmos_ctrl(state, UFS0_PHY_PWR_CON,
			       UFS0_PHY_PROT_STEP1_MASK);
}

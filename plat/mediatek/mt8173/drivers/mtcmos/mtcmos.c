/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <mt8173_def.h>
#include <mtcmos.h>
#include <spm.h>
#include <spm_mcdi.h>

enum {
	SRAM_ISOINT_B	= 1U << 6,
	SRAM_CKISO	= 1U << 5,
	PWR_CLK_DIS	= 1U << 4,
	PWR_ON_2ND	= 1U << 3,
	PWR_ON		= 1U << 2,
	PWR_ISO		= 1U << 1,
	PWR_RST_B	= 1U << 0
};

enum {
	L1_PDN_ACK	= 1U << 8,
	L1_PDN		= 1U << 0
};

enum {
	LITTLE_CPU3	= 1U << 12,
	LITTLE_CPU2	= 1U << 11,
	LITTLE_CPU1	= 1U << 10,
};

enum {
	SRAM_PDN           = 0xf << 8,
	DIS_SRAM_ACK       = 0x1 << 12,
	AUD_SRAM_ACK       = 0xf << 12,
};

enum {
	DIS_PWR_STA_MASK   = 0x1 << 3,
	AUD_PWR_STA_MASK   = 0x1 << 24,
};

#define SPM_VDE_PWR_CON				0x0210
#define SPM_MFG_PWR_CON				0x0214
#define SPM_VEN_PWR_CON				0x0230
#define SPM_ISP_PWR_CON				0x0238
#define SPM_DIS_PWR_CON				0x023c
#define SPM_VEN2_PWR_CON			0x0298
#define SPM_AUDIO_PWR_CON			0x029c
#define SPM_MFG_2D_PWR_CON			0x02c0
#define SPM_MFG_ASYNC_PWR_CON			0x02c4
#define SPM_USB_PWR_CON				0x02cc

#define MTCMOS_CTRL_SUCCESS			0
#define MTCMOS_CTRL_ERROR			-1

#define MTCMOS_CTRL_EN				(0x1 << 18)

#define VDE_PWR_ON				0
#define VEN_PWR_ON				1
#define ISP_PWR_ON				2
#define DIS_PWR_ON				3
#define VEN2_PWR_ON				4
#define AUDIO_PWR_ON				5
#define MFG_ASYNC_PWR_ON			6
#define MFG_2D_PWR_ON				7
#define MFG_PWR_ON				8
#define USB_PWR_ON				9

#define VDE_PWR_OFF				10
#define VEN_PWR_OFF				11
#define ISP_PWR_OFF				12
#define DIS_PWR_OFF				13
#define VEN2_PWR_OFF				14
#define AUDIO_PWR_OFF				15
#define MFG_ASYNC_PWR_OFF			16
#define MFG_2D_PWR_OFF				17
#define MFG_PWR_OFF				18
#define USB_PWR_OFF				19

#define VDE_PWR_CON_PWR_STA			7
#define VEN_PWR_CON_PWR_STA			21
#define ISP_PWR_CON_PWR_STA			5
#define DIS_PWR_CON_PWR_STA			3
#define VEN2_PWR_CON_PWR_STA			20
#define AUDIO_PWR_CON_PWR_STA			24
#define MFG_ASYNC_PWR_CON_PWR_STA		23
#define MFG_2D_PWR_CON_PWR_STA			22
#define MFG_PWR_CON_PWR_STA			4
#define USB_PWR_CON_PWR_STA			25

/*
 * Timeout if the ack is not signled after 1 second.
 * According to designer, one mtcmos operation should be done
 * around 10us.
 */
#define MTCMOS_ACK_POLLING_MAX_COUNT			10000
#define MTCMOS_ACK_POLLING_INTERVAL			10

static void mtcmos_ctrl_little_off(unsigned int linear_id)
{
	uint32_t reg_pwr_con;
	uint32_t reg_l1_pdn;
	uint32_t bit_cpu;

	switch (linear_id) {
	case 1:
		reg_pwr_con = SPM_CA7_CPU1_PWR_CON;
		reg_l1_pdn = SPM_CA7_CPU1_L1_PDN;
		bit_cpu = LITTLE_CPU1;
		break;
	case 2:
		reg_pwr_con = SPM_CA7_CPU2_PWR_CON;
		reg_l1_pdn = SPM_CA7_CPU2_L1_PDN;
		bit_cpu = LITTLE_CPU2;
		break;
	case 3:
		reg_pwr_con = SPM_CA7_CPU3_PWR_CON;
		reg_l1_pdn = SPM_CA7_CPU3_L1_PDN;
		bit_cpu = LITTLE_CPU3;
		break;
	default:
		/* should never come to here */
		return;
	}

	/* enable register control */
	mmio_write_32(SPM_POWERON_CONFIG_SET,
			(SPM_PROJECT_CODE << 16) | (1U << 0));

	mmio_setbits_32(reg_pwr_con, PWR_ISO);
	mmio_setbits_32(reg_pwr_con, SRAM_CKISO);
	mmio_clrbits_32(reg_pwr_con, SRAM_ISOINT_B);
	mmio_setbits_32(reg_l1_pdn, L1_PDN);

	while (!(mmio_read_32(reg_l1_pdn) & L1_PDN_ACK))
		continue;

	mmio_clrbits_32(reg_pwr_con, PWR_RST_B);
	mmio_setbits_32(reg_pwr_con, PWR_CLK_DIS);
	mmio_clrbits_32(reg_pwr_con, PWR_ON);
	mmio_clrbits_32(reg_pwr_con, PWR_ON_2ND);

	while ((mmio_read_32(SPM_PWR_STATUS) & bit_cpu) ||
	       (mmio_read_32(SPM_PWR_STATUS_2ND) & bit_cpu))
		continue;
}

void mtcmos_little_cpu_off(void)
{
	/* turn off little cpu 1 - 3 */
	mtcmos_ctrl_little_off(1);
	mtcmos_ctrl_little_off(2);
	mtcmos_ctrl_little_off(3);
}

uint32_t wait_mtcmos_ack(uint32_t on, uint32_t pwr_ctrl, uint32_t spm_pwr_sta)
{
	int i = 0;
	uint32_t cmp, pwr_sta, pwr_sta_2nd;

	while (1) {
		cmp = mmio_read_32(SPM_PCM_PASR_DPD_3) & pwr_ctrl;
		pwr_sta = (mmio_read_32(SPM_PWR_STATUS) >> spm_pwr_sta) & 1;
		pwr_sta_2nd =
			(mmio_read_32(SPM_PWR_STATUS_2ND) >> spm_pwr_sta) & 1;
		if (cmp && (pwr_sta == on) && (pwr_sta_2nd == on)) {
			mmio_write_32(SPM_PCM_RESERVE2, 0);
			return MTCMOS_CTRL_SUCCESS;
		}
		udelay(MTCMOS_ACK_POLLING_INTERVAL);
		i++;
		if (i > MTCMOS_ACK_POLLING_MAX_COUNT) {
			INFO("MTCMOS control failed(%d), SPM_PWR_STA(%d),\n"
				"SPM_PCM_RESERVE=0x%x,SPM_PCM_RESERVE2=0x%x,\n"
				"SPM_PWR_STATUS=0x%x,SPM_PWR_STATUS_2ND=0x%x\n"
				"SPM_PCM_PASR_DPD_3 = 0x%x\n",
				on, spm_pwr_sta, mmio_read_32(SPM_PCM_RESERVE),
				mmio_read_32(SPM_PCM_RESERVE2),
				mmio_read_32(SPM_PWR_STATUS),
				mmio_read_32(SPM_PWR_STATUS_2ND),
				mmio_read_32(SPM_PCM_PASR_DPD_3));
			mmio_write_32(SPM_PCM_RESERVE2, 0);
			return MTCMOS_CTRL_ERROR;
		}
	}
}

uint32_t mtcmos_non_cpu_ctrl(uint32_t on, uint32_t mtcmos_num)
{
	uint32_t ret = MTCMOS_CTRL_SUCCESS;
	uint32_t power_on;
	uint32_t power_off;
	uint32_t power_ctrl;
	uint32_t power_status;

	spm_lock_get();
	spm_mcdi_prepare_for_mtcmos();
	mmio_setbits_32(SPM_PCM_RESERVE, MTCMOS_CTRL_EN);

	switch (mtcmos_num) {
	case SPM_VDE_PWR_CON:
		power_on = VDE_PWR_ON;
		power_off = VDE_PWR_OFF;
		power_status = VDE_PWR_CON_PWR_STA;
		break;
	case SPM_MFG_PWR_CON:
		power_on = MFG_PWR_ON;
		power_off = MFG_PWR_OFF;
		power_status = MFG_PWR_CON_PWR_STA;
		break;
	case SPM_VEN_PWR_CON:
		power_on = VEN_PWR_ON;
		power_off = VEN_PWR_OFF;
		power_status = VEN_PWR_CON_PWR_STA;
		break;
	case SPM_ISP_PWR_CON:
		power_on = ISP_PWR_ON;
		power_off = ISP_PWR_OFF;
		power_status = ISP_PWR_CON_PWR_STA;
		break;
	case SPM_DIS_PWR_CON:
		power_on = DIS_PWR_ON;
		power_off = DIS_PWR_OFF;
		power_status = DIS_PWR_CON_PWR_STA;
		break;
	case SPM_VEN2_PWR_CON:
		power_on = VEN2_PWR_ON;
		power_off = VEN2_PWR_OFF;
		power_status = VEN2_PWR_CON_PWR_STA;
		break;
	case SPM_AUDIO_PWR_CON:
		power_on = AUDIO_PWR_ON;
		power_off = AUDIO_PWR_OFF;
		power_status = AUDIO_PWR_CON_PWR_STA;
		break;
	case SPM_MFG_2D_PWR_CON:
		power_on = MFG_2D_PWR_ON;
		power_off = MFG_2D_PWR_OFF;
		power_status = MFG_2D_PWR_CON_PWR_STA;
		break;
	case SPM_MFG_ASYNC_PWR_CON:
		power_on = MFG_ASYNC_PWR_ON;
		power_off = MFG_ASYNC_PWR_OFF;
		power_status = MFG_ASYNC_PWR_CON_PWR_STA;
		break;
	case SPM_USB_PWR_CON:
		power_on = USB_PWR_ON;
		power_off = USB_PWR_OFF;
		power_status = USB_PWR_CON_PWR_STA;
		break;
	default:
		ret = MTCMOS_CTRL_ERROR;
		INFO("No mapping MTCMOS(%d), ret = %d\n", mtcmos_num, ret);
		break;
	}
	if (ret == MTCMOS_CTRL_SUCCESS) {
		power_ctrl = on ? (1 << power_on) : (1 << power_off);
		mmio_setbits_32(SPM_PCM_RESERVE2, power_ctrl);
		ret = wait_mtcmos_ack(on, power_ctrl, power_status);
		VERBOSE("0x%x(%d), PWR_STATUS(0x%x), ret(%d)\n",
			power_ctrl, on, mmio_read_32(SPM_PWR_STATUS), ret);
	}

	mmio_clrbits_32(SPM_PCM_RESERVE, MTCMOS_CTRL_EN);
	spm_lock_release();

	return ret;
}

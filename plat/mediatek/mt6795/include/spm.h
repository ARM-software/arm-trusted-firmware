/*
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

#ifndef __SPM_H__
#define __SPM_H__

#define SPM_POWERON_CONFIG_SET			(SPM_BASE + 0x000)
#define SPM_POWER_ON_VAL0			(SPM_BASE + 0x010)
#define SPM_POWER_ON_VAL1			(SPM_BASE + 0x014)
#define SPM_CLK_SETTLE				(SPM_BASE + 0x100)
#define SPM_CA7_CPU1_PWR_CON			(SPM_BASE + 0x218)
#define SPM_CA7_CPU2_PWR_CON			(SPM_BASE + 0x21c)
#define SPM_CA7_CPU3_PWR_CON			(SPM_BASE + 0x220)
#define SPM_CA7_CPU1_L1_PDN			(SPM_BASE + 0x264)
#define SPM_CA7_CPU2_L1_PDN			(SPM_BASE + 0x26c)
#define SPM_CA7_CPU3_L1_PDN			(SPM_BASE + 0x274)
#define SPM_MD32_SRAM_CON			(SPM_BASE + 0x2c8)
#define SPM_PCM_CON0				(SPM_BASE + 0x310)
#define SPM_PCM_CON1				(SPM_BASE + 0x314)
#define SPM_PCM_IM_PTR				(SPM_BASE + 0x318)
#define SPM_PCM_IM_LEN				(SPM_BASE + 0x31c)
#define SPM_PCM_REG_DATA_INI			(SPM_BASE + 0x320)
#define SPM_PCM_EVENT_VECTOR0			(SPM_BASE + 0x340)
#define SPM_PCM_EVENT_VECTOR1			(SPM_BASE + 0x344)
#define SPM_PCM_EVENT_VECTOR2			(SPM_BASE + 0x348)
#define SPM_PCM_EVENT_VECTOR3			(SPM_BASE + 0x34c)
#define SPM_PCM_MAS_PAUSE_MASK			(SPM_BASE + 0x354)
#define SPM_PCM_PWR_IO_EN			(SPM_BASE + 0x358)
#define SPM_PCM_TIMER_VAL			(SPM_BASE + 0x35c)
#define SPM_PCM_TIMER_OUT			(SPM_BASE + 0x360)
#define SPM_PCM_REG0_DATA			(SPM_BASE + 0x380)
#define SPM_PCM_REG1_DATA			(SPM_BASE + 0x384)
#define SPM_PCM_REG2_DATA			(SPM_BASE + 0x388)
#define SPM_PCM_REG3_DATA			(SPM_BASE + 0x38c)
#define SPM_PCM_REG4_DATA			(SPM_BASE + 0x390)
#define SPM_PCM_REG5_DATA			(SPM_BASE + 0x394)
#define SPM_PCM_REG6_DATA			(SPM_BASE + 0x398)
#define SPM_PCM_REG7_DATA			(SPM_BASE + 0x39c)
#define SPM_PCM_REG8_DATA			(SPM_BASE + 0x3a0)
#define SPM_PCM_REG9_DATA			(SPM_BASE + 0x3a4)
#define SPM_PCM_REG10_DATA			(SPM_BASE + 0x3a8)
#define SPM_PCM_REG11_DATA			(SPM_BASE + 0x3ac)
#define SPM_PCM_REG12_DATA			(SPM_BASE + 0x3b0)
#define SPM_PCM_REG13_DATA			(SPM_BASE + 0x3b4)
#define SPM_PCM_REG14_DATA			(SPM_BASE + 0x3b8)
#define SPM_PCM_REG15_DATA			(SPM_BASE + 0x3bc)
#define SPM_PCM_EVENT_REG_STA			(SPM_BASE + 0x3c0)
#define SPM_PCM_FSM_STA				(SPM_BASE + 0x3c4)
#define SPM_PCM_IM_HOST_RW_PTR			(SPM_BASE + 0x3c8)
#define SPM_PCM_IM_HOST_RW_DAT			(SPM_BASE + 0x3cc)
#define SPM_PCM_EVENT_VECTOR4			(SPM_BASE + 0x3d0)
#define SPM_PCM_EVENT_VECTOR5			(SPM_BASE + 0x3d4)
#define SPM_PCM_EVENT_VECTOR6			(SPM_BASE + 0x3d8)
#define SPM_PCM_EVENT_VECTOR7			(SPM_BASE + 0x3dc)
#define SPM_PCM_SW_INT_SET			(SPM_BASE + 0x3e0)
#define SPM_PCM_SW_INT_CLEAR			(SPM_BASE + 0x3e4)
#define SPM_CLK_CON				(SPM_BASE + 0x400)
#define SPM_SLEEP_PTPOD2_CON			(SPM_BASE + 0x408)
#define SPM_APMCU_PWRCTL			(SPM_BASE + 0x600)
#define SPM_AP_DVFS_CON_SET			(SPM_BASE + 0x604)
#define SPM_AP_STANBY_CON			(SPM_BASE + 0x608)
#define SPM_PWR_STATUS				(SPM_BASE + 0x60c)
#define SPM_PWR_STATUS_2ND			(SPM_BASE + 0x610)
#define SPM_AP_BSI_REQ				(SPM_BASE + 0x614)
#define SPM_SLEEP_TIMER_STA			(SPM_BASE + 0x720)
#define SPM_SLEEP_WAKEUP_EVENT_MASK		(SPM_BASE + 0x810)
#define SPM_SLEEP_CPU_WAKEUP_EVENT		(SPM_BASE + 0x814)
#define SPM_SLEEP_MD32_WAKEUP_EVENT_MASK	(SPM_BASE + 0x818)
#define SPM_PCM_WDT_TIMER_VAL			(SPM_BASE + 0x824)
#define SPM_PCM_WDT_TIMER_OUT			(SPM_BASE + 0x828)
#define SPM_PCM_MD32_MAILBOX			(SPM_BASE + 0x830)
#define SPM_PCM_MD32_IRQ			(SPM_BASE + 0x834)
#define SPM_SLEEP_ISR_MASK			(SPM_BASE + 0x900)
#define SPM_SLEEP_ISR_STATUS			(SPM_BASE + 0x904)
#define SPM_SLEEP_ISR_RAW_STA			(SPM_BASE + 0x910)
#define SPM_SLEEP_MD32_ISR_RAW_STA		(SPM_BASE + 0x914)
#define SPM_SLEEP_WAKEUP_MISC			(SPM_BASE + 0x918)
#define SPM_SLEEP_BUS_PROTECT_RDY		(SPM_BASE + 0x91c)
#define SPM_SLEEP_SUBSYS_IDLE_STA		(SPM_BASE + 0x920)
#define SPM_PCM_RESERVE				(SPM_BASE + 0xb00)
#define SPM_PCM_RESERVE2			(SPM_BASE + 0xb04)
#define SPM_PCM_FLAGS				(SPM_BASE + 0xb08)
#define SPM_PCM_SRC_REQ				(SPM_BASE + 0xb0c)
#define SPM_PCM_DEBUG_CON			(SPM_BASE + 0xb20)
#define SPM_CA7_CPU0_IRQ_MASK			(SPM_BASE + 0xb30)
#define SPM_CA7_CPU1_IRQ_MASK			(SPM_BASE + 0xb34)
#define SPM_CA7_CPU2_IRQ_MASK			(SPM_BASE + 0xb38)
#define SPM_CA7_CPU3_IRQ_MASK			(SPM_BASE + 0xb3c)
#define SPM_CA15_CPU0_IRQ_MASK			(SPM_BASE + 0xb40)
#define SPM_CA15_CPU1_IRQ_MASK			(SPM_BASE + 0xb44)
#define SPM_CA15_CPU2_IRQ_MASK			(SPM_BASE + 0xb48)
#define SPM_CA15_CPU3_IRQ_MASK			(SPM_BASE + 0xb4c)
#define SPM_PCM_PASR_DPD_0			(SPM_BASE + 0xb60)
#define SPM_PCM_PASR_DPD_1			(SPM_BASE + 0xb64)
#define SPM_PCM_PASR_DPD_2			(SPM_BASE + 0xb68)
#define SPM_PCM_PASR_DPD_3			(SPM_BASE + 0xb6c)
#define SPM_SLEEP_CA7_WFI0_EN			(SPM_BASE + 0xf00)
#define SPM_SLEEP_CA7_WFI1_EN			(SPM_BASE + 0xf04)
#define SPM_SLEEP_CA7_WFI2_EN			(SPM_BASE + 0xf08)
#define SPM_SLEEP_CA7_WFI3_EN			(SPM_BASE + 0xf0c)
#define SPM_SLEEP_CA15_WFI0_EN			(SPM_BASE + 0xf10)
#define SPM_SLEEP_CA15_WFI1_EN			(SPM_BASE + 0xf14)
#define SPM_SLEEP_CA15_WFI2_EN			(SPM_BASE + 0xf18)
#define SPM_SLEEP_CA15_WFI3_EN			(SPM_BASE + 0xf1c)

#define SPM_PROJECT_CODE	0xb16

#define SPM_REGWR_EN		(1U << 0)
#define SPM_REGWR_CFG_KEY	(SPM_PROJECT_CODE << 16)

#define SPM_CPU_PDN_DIS		(1U << 0)
#define SPM_INFRA_PDN_DIS	(1U << 1)
#define SPM_DDRPHY_PDN_DIS	(1U << 2)
#define SPM_DUALVCORE_PDN_DIS	(1U << 3)
#define SPM_PASR_DIS		(1U << 4)
#define SPM_DPD_DIS		(1U << 5)
#define SPM_SODI_DIS		(1U << 6)
#define SPM_MEMPLL_RESET	(1U << 7)
#define SPM_MAINPLL_PDN_DIS	(1U << 8)
#define SPM_CPU_DVS_DIS		(1U << 9)
#define SPM_CPU_DORMANT		(1U << 10)
#define SPM_EXT_VSEL_GPIO103	(1U << 11)
#define SPM_DDR_HIGH_SPEED	(1U << 12)
#define SPM_OPT			(1U << 13)

#define POWER_ON_VAL1_DEF	0x01011820
#define PCM_FSM_STA_DEF		0x48490
#define PCM_END_FSM_STA_DEF	0x08490
#define PCM_END_FSM_STA_MASK	0x3fff0
#define PCM_HANDSHAKE_SEND1	0xbeefbeef

#define PCM_WDT_TIMEOUT		(30 * 32768)
#define PCM_TIMER_MAX		(0xffffffff - PCM_WDT_TIMEOUT)

#define CON0_PCM_KICK		(1U << 0)
#define CON0_IM_KICK		(1U << 1)
#define CON0_IM_SLEEP_DVS	(1U << 3)
#define CON0_PCM_SW_RESET	(1U << 15)
#define CON0_CFG_KEY		(SPM_PROJECT_CODE << 16)

#define CON1_IM_SLAVE		(1U << 0)
#define CON1_MIF_APBEN		(1U << 3)
#define CON1_PCM_TIMER_EN	(1U << 5)
#define CON1_IM_NONRP_EN	(1U << 6)
#define CON1_PCM_WDT_EN		(1U << 8)
#define CON1_PCM_WDT_WAKE_MODE	(1U << 9)
#define CON1_SPM_SRAM_SLP_B	(1U << 10)
#define CON1_SPM_SRAM_ISO_B	(1U << 11)
#define CON1_EVENT_LOCK_EN	(1U << 12)
#define CON1_CFG_KEY		(SPM_PROJECT_CODE << 16)

#define PCM_PWRIO_EN_R0		(1U << 0)
#define PCM_PWRIO_EN_R7		(1U << 7)
#define PCM_RF_SYNC_R0		(1U << 16)
#define PCM_RF_SYNC_R2		(1U << 18)
#define PCM_RF_SYNC_R6		(1U << 22)
#define PCM_RF_SYNC_R7		(1U << 23)

#define CC_SYSCLK0_EN_0		(1U << 0)
#define CC_SYSCLK0_EN_1		(1U << 1)
#define CC_SYSCLK1_EN_0		(1U << 2)
#define CC_SYSCLK1_EN_1		(1U << 3)
#define CC_SYSSETTLE_SEL	(1U << 4)
#define CC_LOCK_INFRA_DCM	(1U << 5)
#define CC_SRCLKENA_MASK_0	(1U << 6)
#define CC_CXO32K_RM_EN_MD1	(1U << 9)
#define CC_CXO32K_RM_EN_MD2	(1U << 10)
#define CC_CLKSQ1_SEL		(1U << 12)
#define CC_DISABLE_DORM_PWR	(1U << 14)
#define CC_MD32_DCM_EN		(1U << 18)

#define WFI_OP_AND		1
#define WFI_OP_OR		0

#define WAKE_MISC_PCM_TIMER	(1U << 19)
#define WAKE_MISC_CPU_WAKE	(1U << 20)

/* define WAKE_SRC_XXX */
#define WAKE_SRC_SPM_MERGE	(1 << 0)
#define WAKE_SRC_KP		(1 << 2)
#define WAKE_SRC_WDT		(1 << 3)
#define WAKE_SRC_GPT		(1 << 4)
#define WAKE_SRC_EINT		(1 << 6)
#define WAKE_SRC_LOW_BAT	(1 << 9)
#define WAKE_SRC_MD32		(1 << 10)
#define WAKE_SRC_USB_CD		(1 << 14)
#define WAKE_SRC_USB_PDN	(1 << 15)
#define WAKE_SRC_AFE		(1 << 20)
#define WAKE_SRC_THERM		(1 << 21)
#define WAKE_SRC_SYSPWREQ	(1 << 24)
#define WAKE_SRC_SEJ		(1 << 27)
#define WAKE_SRC_ALL_MD32	(1 << 28)
#define WAKE_SRC_CPU_IRQ	(1 << 29)

#endif /* __SPM_H__ */

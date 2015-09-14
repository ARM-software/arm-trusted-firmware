/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <bakery_lock.h>
#include <debug.h>
#include <mmio.h>
#include <mt8173_def.h>
#include <spm.h>
#include <spm_suspend.h>

/*
 * System Power Manager (SPM) is a hardware module, which controls cpu or
 * system power for different power scenarios using different firmware, i.e.,
 * - spm_hotplug.c for cpu power control in cpu hotplug flow.
 * - spm_mcdi.c for cpu power control in cpu idle power saving state.
 * - spm_suspend.c for system power control in system suspend scenario.
 *
 * This file provide utility functions common to hotplug, mcdi(idle), suspend
 * power scenarios. A bakery lock (software lock) is incoporated to protect
 * certain critical sections to avoid kicking different SPM firmware
 * concurrently.
 */

#define SPM_SYSCLK_SETTLE       128	/* 3.9ms */

#if DEBUG
static int spm_dormant_sta = CPU_DORMANT_RESET;
#endif

DEFINE_BAKERY_LOCK(spm_lock);

static int spm_hotplug_ready __attribute__ ((section("tzfw_coherent_mem")));
static int spm_mcdi_ready __attribute__ ((section("tzfw_coherent_mem")));
static int spm_suspend_ready __attribute__ ((section("tzfw_coherent_mem")));

void spm_lock_init(void)
{
	bakery_lock_init(&spm_lock);
}

void spm_lock_get(void)
{
	bakery_lock_get(&spm_lock);
}

void spm_lock_release(void)
{
	bakery_lock_release(&spm_lock);
}

int is_mcdi_ready(void)
{
	return spm_mcdi_ready;
}

int is_hotplug_ready(void)
{
	return spm_hotplug_ready;
}

int is_suspend_ready(void)
{
	return spm_suspend_ready;
}

void set_mcdi_ready(void)
{
	spm_mcdi_ready = 1;
	spm_hotplug_ready = 0;
	spm_suspend_ready = 0;
}

void set_hotplug_ready(void)
{
	spm_mcdi_ready = 0;
	spm_hotplug_ready = 1;
	spm_suspend_ready = 0;
}

void set_suspend_ready(void)
{
	spm_mcdi_ready = 0;
	spm_hotplug_ready = 0;
	spm_suspend_ready = 1;
}

void clear_all_ready(void)
{
	spm_mcdi_ready = 0;
	spm_hotplug_ready = 0;
	spm_suspend_ready = 0;
}

void spm_register_init(void)
{
	mmio_write_32(SPM_POWERON_CONFIG_SET, SPM_REGWR_CFG_KEY | SPM_REGWR_EN);

	mmio_write_32(SPM_POWER_ON_VAL0, 0);
	mmio_write_32(SPM_POWER_ON_VAL1, POWER_ON_VAL1_DEF);
	mmio_write_32(SPM_PCM_PWR_IO_EN, 0);

	mmio_write_32(SPM_PCM_CON0, CON0_CFG_KEY | CON0_PCM_SW_RESET);
	mmio_write_32(SPM_PCM_CON0, CON0_CFG_KEY);
	if (mmio_read_32(SPM_PCM_FSM_STA) != PCM_FSM_STA_DEF)
		WARN("PCM reset failed\n");

	mmio_write_32(SPM_PCM_CON0, CON0_CFG_KEY | CON0_IM_SLEEP_DVS);
	mmio_write_32(SPM_PCM_CON1, CON1_CFG_KEY | CON1_EVENT_LOCK_EN |
		CON1_SPM_SRAM_ISO_B | CON1_SPM_SRAM_SLP_B | CON1_MIF_APBEN);
	mmio_write_32(SPM_PCM_IM_PTR, 0);
	mmio_write_32(SPM_PCM_IM_LEN, 0);

	mmio_write_32(SPM_CLK_CON, CC_SYSCLK0_EN_1 | CC_SYSCLK0_EN_0 |
		CC_SYSCLK1_EN_0 | CC_SRCLKENA_MASK_0 | CC_CLKSQ1_SEL |
		CC_CXO32K_RM_EN_MD2 | CC_CXO32K_RM_EN_MD1 | CC_MD32_DCM_EN);

	mmio_write_32(SPM_SLEEP_ISR_MASK, 0xff0c);
	mmio_write_32(SPM_SLEEP_ISR_STATUS, 0xc);
	mmio_write_32(SPM_PCM_SW_INT_CLEAR, 0xff);
	mmio_write_32(SPM_MD32_SRAM_CON, 0xff0);
}

void spm_reset_and_init_pcm(void)
{
	unsigned int con1;
	int i = 0;

	mmio_write_32(SPM_PCM_CON0, CON0_CFG_KEY | CON0_PCM_SW_RESET);
	mmio_write_32(SPM_PCM_CON0, CON0_CFG_KEY);
	while (mmio_read_32(SPM_PCM_FSM_STA) != PCM_FSM_STA_DEF) {
		i++;
		if (i > 1000) {
			i = 0;
			WARN("PCM reset failed\n");
			break;
		}
	}

	mmio_write_32(SPM_PCM_CON0, CON0_CFG_KEY | CON0_IM_SLEEP_DVS);

	con1 = mmio_read_32(SPM_PCM_CON1) &
		(CON1_PCM_WDT_WAKE_MODE | CON1_PCM_WDT_EN);
	mmio_write_32(SPM_PCM_CON1, con1 | CON1_CFG_KEY | CON1_EVENT_LOCK_EN |
		CON1_SPM_SRAM_ISO_B | CON1_SPM_SRAM_SLP_B |
		CON1_IM_NONRP_EN | CON1_MIF_APBEN);
}

void spm_init_pcm_register(void)
{
	mmio_write_32(SPM_PCM_REG_DATA_INI, mmio_read_32(SPM_POWER_ON_VAL0));
	mmio_write_32(SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R0);
	mmio_write_32(SPM_PCM_PWR_IO_EN, 0);

	mmio_write_32(SPM_PCM_REG_DATA_INI, mmio_read_32(SPM_POWER_ON_VAL1));
	mmio_write_32(SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R7);
	mmio_write_32(SPM_PCM_PWR_IO_EN, 0);
}

void spm_set_power_control(const struct pwr_ctrl *pwrctrl)
{
	mmio_write_32(SPM_AP_STANBY_CON, (!pwrctrl->md32_req_mask << 21) |
					 (!pwrctrl->mfg_req_mask << 17) |
					 (!pwrctrl->disp_req_mask << 16) |
					 (!!pwrctrl->mcusys_idle_mask << 7) |
					 (!!pwrctrl->ca15top_idle_mask << 6) |
					 (!!pwrctrl->ca7top_idle_mask << 5) |
					 (!!pwrctrl->wfi_op << 4));
	mmio_write_32(SPM_PCM_SRC_REQ, (!!pwrctrl->pcm_apsrc_req << 0));
	mmio_write_32(SPM_PCM_PASR_DPD_2, 0);

	mmio_clrsetbits_32(SPM_CLK_CON, CC_SRCLKENA_MASK_0,
		(pwrctrl->srclkenai_mask ? CC_SRCLKENA_MASK_0 : 0));

	mmio_write_32(SPM_SLEEP_CA15_WFI0_EN, !!pwrctrl->ca15_wfi0_en);
	mmio_write_32(SPM_SLEEP_CA15_WFI1_EN, !!pwrctrl->ca15_wfi1_en);
	mmio_write_32(SPM_SLEEP_CA15_WFI2_EN, !!pwrctrl->ca15_wfi2_en);
	mmio_write_32(SPM_SLEEP_CA15_WFI3_EN, !!pwrctrl->ca15_wfi3_en);
	mmio_write_32(SPM_SLEEP_CA7_WFI0_EN, !!pwrctrl->ca7_wfi0_en);
	mmio_write_32(SPM_SLEEP_CA7_WFI1_EN, !!pwrctrl->ca7_wfi1_en);
	mmio_write_32(SPM_SLEEP_CA7_WFI2_EN, !!pwrctrl->ca7_wfi2_en);
	mmio_write_32(SPM_SLEEP_CA7_WFI3_EN, !!pwrctrl->ca7_wfi3_en);
}

void spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl)
{
	unsigned int val, mask;

	if (pwrctrl->timer_val_cust == 0)
		val = pwrctrl->timer_val ? pwrctrl->timer_val : PCM_TIMER_MAX;
	else
		val = pwrctrl->timer_val_cust;

	mmio_write_32(SPM_PCM_TIMER_VAL, val);
	mmio_setbits_32(SPM_PCM_CON1, CON1_CFG_KEY);

	if (pwrctrl->wake_src_cust == 0)
		mask = pwrctrl->wake_src;
	else
		mask = pwrctrl->wake_src_cust;

	if (pwrctrl->syspwreq_mask)
		mask &= ~WAKE_SRC_SYSPWREQ;

	mmio_write_32(SPM_SLEEP_WAKEUP_EVENT_MASK, ~mask);
	mmio_write_32(SPM_SLEEP_ISR_MASK, 0xfe04);
}

void spm_get_wakeup_status(struct wake_status *wakesta)
{
	wakesta->assert_pc = mmio_read_32(SPM_PCM_REG_DATA_INI);
	wakesta->r12 = mmio_read_32(SPM_PCM_REG12_DATA);
	wakesta->raw_sta = mmio_read_32(SPM_SLEEP_ISR_RAW_STA);
	wakesta->wake_misc = mmio_read_32(SPM_SLEEP_WAKEUP_MISC);
	wakesta->timer_out = mmio_read_32(SPM_PCM_TIMER_OUT);
	wakesta->r13 = mmio_read_32(SPM_PCM_REG13_DATA);
	wakesta->idle_sta = mmio_read_32(SPM_SLEEP_SUBSYS_IDLE_STA);
	wakesta->debug_flag = mmio_read_32(SPM_PCM_PASR_DPD_3);
	wakesta->event_reg = mmio_read_32(SPM_PCM_EVENT_REG_STA);
	wakesta->isr = mmio_read_32(SPM_SLEEP_ISR_STATUS);
}

void spm_init_event_vector(const struct pcm_desc *pcmdesc)
{
	/* init event vector register */
	mmio_write_32(SPM_PCM_EVENT_VECTOR0, pcmdesc->vec0);
	mmio_write_32(SPM_PCM_EVENT_VECTOR1, pcmdesc->vec1);
	mmio_write_32(SPM_PCM_EVENT_VECTOR2, pcmdesc->vec2);
	mmio_write_32(SPM_PCM_EVENT_VECTOR3, pcmdesc->vec3);
	mmio_write_32(SPM_PCM_EVENT_VECTOR4, pcmdesc->vec4);
	mmio_write_32(SPM_PCM_EVENT_VECTOR5, pcmdesc->vec5);
	mmio_write_32(SPM_PCM_EVENT_VECTOR6, pcmdesc->vec6);
	mmio_write_32(SPM_PCM_EVENT_VECTOR7, pcmdesc->vec7);

	/* event vector will be enabled by PCM itself */
}

void spm_kick_im_to_fetch(const struct pcm_desc *pcmdesc)
{
	unsigned int ptr = 0, len, con0;

	ptr = (unsigned int)(unsigned long)(pcmdesc->base);
	len = pcmdesc->size - 1;
	if (mmio_read_32(SPM_PCM_IM_PTR) != ptr ||
	    mmio_read_32(SPM_PCM_IM_LEN) != len ||
	    pcmdesc->sess > 2) {
		mmio_write_32(SPM_PCM_IM_PTR, ptr);
		mmio_write_32(SPM_PCM_IM_LEN, len);
	} else {
		mmio_setbits_32(SPM_PCM_CON1, CON1_CFG_KEY | CON1_IM_SLAVE);
	}

	/* kick IM to fetch (only toggle IM_KICK) */
	con0 = mmio_read_32(SPM_PCM_CON0) & ~(CON0_IM_KICK | CON0_PCM_KICK);
	mmio_write_32(SPM_PCM_CON0, con0 | CON0_CFG_KEY | CON0_IM_KICK);
	mmio_write_32(SPM_PCM_CON0, con0 | CON0_CFG_KEY);

	/* kick IM to fetch (only toggle PCM_KICK) */
	con0 = mmio_read_32(SPM_PCM_CON0) & ~(CON0_IM_KICK | CON0_PCM_KICK);
	mmio_write_32(SPM_PCM_CON0, con0 | CON0_CFG_KEY | CON0_PCM_KICK);
	mmio_write_32(SPM_PCM_CON0, con0 | CON0_CFG_KEY);
}

void spm_set_sysclk_settle(void)
{
	mmio_write_32(SPM_CLK_SETTLE, SPM_SYSCLK_SETTLE);

	INFO("settle = %u\n", mmio_read_32(SPM_CLK_SETTLE));
}

void spm_kick_pcm_to_run(struct pwr_ctrl *pwrctrl)
{
	unsigned int con1;

	con1 = mmio_read_32(SPM_PCM_CON1) &
		~(CON1_PCM_WDT_WAKE_MODE | CON1_PCM_WDT_EN);

	mmio_write_32(SPM_PCM_CON1, CON1_CFG_KEY | con1);

	if (mmio_read_32(SPM_PCM_TIMER_VAL) > PCM_TIMER_MAX)
		mmio_write_32(SPM_PCM_TIMER_VAL, PCM_TIMER_MAX);

	mmio_write_32(SPM_PCM_WDT_TIMER_VAL,
		mmio_read_32(SPM_PCM_TIMER_VAL) + PCM_WDT_TIMEOUT);

	mmio_write_32(SPM_PCM_CON1, con1 | CON1_CFG_KEY | CON1_PCM_WDT_EN);
	mmio_write_32(SPM_PCM_PASR_DPD_0, 0);

	mmio_write_32(SPM_PCM_MAS_PAUSE_MASK, 0xffffffff);
	mmio_write_32(SPM_PCM_REG_DATA_INI, 0);
	mmio_clrbits_32(SPM_CLK_CON, CC_DISABLE_DORM_PWR);

	mmio_write_32(SPM_PCM_FLAGS, pwrctrl->pcm_flags);

	mmio_clrsetbits_32(SPM_CLK_CON, CC_LOCK_INFRA_DCM,
		(pwrctrl->infra_dcm_lock ? CC_LOCK_INFRA_DCM : 0));

	mmio_write_32(SPM_PCM_PWR_IO_EN,
		(pwrctrl->r0_ctrl_en ? PCM_PWRIO_EN_R0 : 0) |
		(pwrctrl->r7_ctrl_en ? PCM_PWRIO_EN_R7 : 0));
}

void spm_clean_after_wakeup(void)
{
	mmio_clrsetbits_32(SPM_PCM_CON1, CON1_PCM_WDT_EN, CON1_CFG_KEY);

	mmio_write_32(SPM_PCM_PWR_IO_EN, 0);
	mmio_write_32(SPM_SLEEP_CPU_WAKEUP_EVENT, 0);
	mmio_clrsetbits_32(SPM_PCM_CON1, CON1_PCM_TIMER_EN, CON1_CFG_KEY);

	mmio_write_32(SPM_SLEEP_WAKEUP_EVENT_MASK, ~0);
	mmio_write_32(SPM_SLEEP_ISR_MASK, 0xFF0C);
	mmio_write_32(SPM_SLEEP_ISR_STATUS, 0xC);
	mmio_write_32(SPM_PCM_SW_INT_CLEAR, 0xFF);
}

enum wake_reason_t spm_output_wake_reason(struct wake_status *wakesta)
{
	enum wake_reason_t wr;
	int i;

	wr = WR_UNKNOWN;

	if (wakesta->assert_pc != 0) {
		ERROR("PCM ASSERT AT %u, r12=0x%x, r13=0x%x, debug_flag=0x%x\n",
		      wakesta->assert_pc, wakesta->r12, wakesta->r13,
		      wakesta->debug_flag);
		return WR_PCM_ASSERT;
	}

	if (wakesta->r12 & WAKE_SRC_SPM_MERGE) {
		if (wakesta->wake_misc & WAKE_MISC_PCM_TIMER)
			wr = WR_PCM_TIMER;
		if (wakesta->wake_misc & WAKE_MISC_CPU_WAKE)
			wr = WR_WAKE_SRC;
	}

	for (i = 1; i < 32; i++) {
		if (wakesta->r12 & (1U << i))
			wr = WR_WAKE_SRC;
	}

	if ((wakesta->event_reg & 0x100000) == 0) {
		INFO("pcm sleep abort!\n");
		wr = WR_PCM_ABORT;
	}

	INFO("timer_out = %u, r12 = 0x%x, r13 = 0x%x, debug_flag = 0x%x\n",
	     wakesta->timer_out, wakesta->r12, wakesta->r13,
	     wakesta->debug_flag);

	INFO("raw_sta = 0x%x, idle_sta = 0x%x, event_reg = 0x%x, isr = 0x%x\n",
	     wakesta->raw_sta, wakesta->idle_sta, wakesta->event_reg,
	     wakesta->isr);

	INFO("dormant state = %d\n", spm_dormant_sta);
	return wr;
}

void spm_boot_init(void)
{
	/* Only CPU0 is online during boot, initialize cpu online reserve bit */
	mmio_write_32(SPM_PCM_RESERVE, 0xFE);
	spm_lock_init();
	spm_register_init();
}

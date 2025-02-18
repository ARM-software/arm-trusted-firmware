/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <rtc.h>

/* RTC register offsets */
#define RTC_PID                                                  (0x00)
#define RTC_SUB_S_CNT                                            (0x04)
#define RTC_S_CNT_LSW                                            (0x08)
#define RTC_S_CNT_MSW                                            (0x0C)
#define RTC_COMP                                                 (0x10)
#define RTC_OFF_ON_S_CNT_LSW                                     (0x18)
#define RTC_OFF_ON_S_CNT_MSW                                     (0x1C)
#define RTC_ON_OFF_S_CNT_LSW                                     (0x20)
#define RTC_ON_OFF_S_CNT_MSW                                     (0x24)
#define RTC_DEBOUNCE                                             (0x28)
#define RTC_ANALOG                                               (0x2C)
#define RTC_SCRATCH0                                             (0x30)
#define RTC_SCRATCH1                                             (0x34)
#define RTC_SCRATCH2                                             (0x38)
#define RTC_SCRATCH3                                             (0x3C)
#define RTC_SCRATCH4                                             (0x40)
#define RTC_SCRATCH5                                             (0x44)
#define RTC_SCRATCH6                                             (0x48)
#define RTC_SCRATCH7                                             (0x4C)
#define RTC_GENRAL_CTL                                           (0x50)
#define RTC_IRQSTATUS_RAW_SYS                                    (0x54)
#define RTC_IRQSTATUS_SYS                                        (0x58)
#define RTC_IRQENABLE_SET_SYS                                    (0x5C)
#define RTC_IRQENABLE_CLR_SYS                                    (0x60)
#define RTC_SYNCPEND                                             (0x68)
#define RTC_KICK0                                                (0x70)
#define RTC_KICK1                                                (0x74)
#define RTC_LFXOSC_CTRL                                          (0x80)
#define RTC_LFXOSC_TRIM                                          (0x84)
#define RTC_GP_SCRATCH0                                          (0x1000)
#define RTC_GP_SCRATCH1                                          (0x1004)
#define RTC_GP_SCRATCH2                                          (0x1008)
#define RTC_GP_SCRATCH3                                          (0x100C)
#define RTC_GP_SCRATCH4                                          (0x1010)
#define RTC_GP_SCRATCH5                                          (0x1014)
#define RTC_GP_SCRATCH6                                          (0x1018)
#define RTC_GP_SCRATCH7                                          (0x101C)

#define WKUP_CTRL_CLK_32K_RC_CLKSEL				(0x100)
#define WKUP_CTRL_CLK_32K_RC_CLKSEL_LFOSC0_CLKOUT		(0x3)
#define WKUP_CTRL_CLK_32K_RC_CLKSEL_CLK32K_RC			(0x0)
#define RELOAD_FROM_BBD						BIT(31)

/* TODO: Recheck the timeout value */
#define K3_RTC_TIMEOUT		10000U

void lpm_rtc_read_time(struct rtc_time *rtc)
{
	if (rtc != NULL) {
		rtc->sub_sec = mmio_read_32(K3_RTC_BASE + RTC_SUB_S_CNT);
		rtc->sec_lo = mmio_read_32(K3_RTC_BASE + RTC_S_CNT_LSW);
		rtc->sec_hi = mmio_read_32(K3_RTC_BASE + RTC_S_CNT_MSW);
	} else {
		/* do nothing */
	}
}

static void rtc_lock(void)
{
	uint32_t i = 0U;

	/* Lock RTC MMRs */
	/* TODO: do proper timeout implementation */
	mmio_write_32(K3_RTC_BASE + RTC_KICK0, 0);
	while ((i++ < K3_RTC_TIMEOUT) && ((mmio_read_32(K3_RTC_BASE + RTC_GENRAL_CTL) & BIT(23)) != 0U)) {
	}
}

static void wait_for_write_pend(void)
{
	uint32_t i = 0U;

	while ((i++ < K3_RTC_TIMEOUT) && ((mmio_read_32(K3_RTC_BASE + RTC_SYNCPEND) & BIT(0)) != 0U)) {
	}
}

static void rtc_unlock(void)
{
	uint32_t i = 0U;

	/* Unlock RTC MMRs */
	mmio_write_32(K3_RTC_BASE + RTC_KICK0, 0x83E70B13);
	mmio_write_32(K3_RTC_BASE + RTC_KICK1, 0x95A4F1E0);
	while ((i++ < K3_RTC_TIMEOUT) && ((mmio_read_32(K3_RTC_BASE + RTC_GENRAL_CTL) & BIT(23))) == 0U) {
	}
}

void rtc_init(void)
{
	uint32_t ctrl;

	/* Select RTC clock */
	mmio_write_32(WKUP_CTRL_MMR_SEC_2_BASE + WKUP_CTRL_CLK_32K_RC_CLKSEL, WKUP_CTRL_CLK_32K_RC_CLKSEL_LFOSC0_CLKOUT);

	/* Configure RTC analog MMRs */
	rtc_unlock();
	mmio_write_32(K3_RTC_BASE + RTC_ANALOG, 0x0);
	mmio_write_32(K3_RTC_BASE + RTC_LFXOSC_CTRL, 0x0);
	mmio_write_32(K3_RTC_BASE + RTC_LFXOSC_TRIM, 0x00121203);
	rtc_lock();
	wait_for_write_pend();

	/* Enable 32K OSC dependency */
	rtc_unlock();
	ctrl = mmio_read_32(K3_RTC_BASE + RTC_GENRAL_CTL);
	ctrl &= ~0x400000;
	ctrl |= 0x200000;
	mmio_write_32(K3_RTC_BASE + RTC_GENRAL_CTL, ctrl);
	rtc_lock();
	wait_for_write_pend();

	mmio_write_32(WKUP_CTRL_MMR_SEC_2_BASE + WKUP_CTRL_CLK_32K_RC_CLKSEL, WKUP_CTRL_CLK_32K_RC_CLKSEL_CLK32K_RC);

}

void rtc_suspend(void)
{
	uint32_t ctrl;

	/* Configure wake up source polarity and enable pmic power off control */
	rtc_unlock();
	ctrl = mmio_read_32(K3_RTC_BASE + RTC_GENRAL_CTL);
	ctrl |= 0x10040;
	mmio_write_32(K3_RTC_BASE + RTC_GENRAL_CTL, ctrl);
	/* Enable all wake up interrupt */
	ctrl = mmio_read_32(K3_RTC_BASE + RTC_IRQENABLE_SET_SYS);
	ctrl |= 0x1C;
	mmio_write_32(K3_RTC_BASE + RTC_IRQENABLE_SET_SYS, ctrl);
	rtc_lock();

	/* Enable all wake up source and issue a OFF event */
	rtc_unlock();
	ctrl = mmio_read_32(K3_RTC_BASE + RTC_GENRAL_CTL);
	ctrl |= 0x20007;
	mmio_write_32(K3_RTC_BASE + RTC_GENRAL_CTL, ctrl);

	/* Wait for read, write to finish */
	wait_for_write_pend();
}

void rtc_resume(void)
{
	uint32_t intr_src;
	uint32_t ctrl;
	uint32_t i = 0U;

	/* Explicitly clear SW_OFF on rtc_cd side */
	ctrl = mmio_read_32(K3_RTC_BASE + RTC_GENRAL_CTL);
	ctrl = ctrl & (~(1 << 17));
	mmio_write_32(K3_RTC_BASE + RTC_GENRAL_CTL, ctrl);

	/* flush write; this causes a "write error" but removes SW_OFF condition */
	wait_for_write_pend();

	/* Lock RTC */
	mmio_write_32(K3_RTC_BASE + RTC_KICK0, 0);

	/* resync of lock status takes a 32k clock cycle because of O32K_OSC_DEP_EN */
	wait_for_write_pend();

	/* read the IRQ source */
	ctrl = mmio_read_32(K3_RTC_BASE + RTC_IRQSTATUS_RAW_SYS);

	/* Clear write error condition */
	rtc_unlock();
	mmio_write_32(K3_RTC_BASE + RTC_SYNCPEND, 0x00000008);
	/* Disable wake up interrupts */
	mmio_write_32(K3_RTC_BASE + RTC_IRQENABLE_CLR_SYS, 0x1F);
	rtc_lock();
	wait_for_write_pend();

	/* Read RTC's interrupt register to check the wake up source */
	intr_src = mmio_read_32(K3_RTC_BASE + RTC_IRQSTATUS_RAW_SYS);
	ERROR("Wake up interrupt 0x%lx\n", (unsigned long)intr_src);

	/* Clear wake up en */
	ctrl = mmio_read_32(K3_RTC_BASE + RTC_GENRAL_CTL);
	ctrl = ctrl & (~(0xF));
	rtc_unlock();
	mmio_write_32(K3_RTC_BASE + RTC_GENRAL_CTL, ctrl);
	rtc_lock();
	wait_for_write_pend();

	while ((i++ < K3_RTC_TIMEOUT) && ((mmio_read_32(K3_RTC_BASE + RTC_SYNCPEND) & BIT(2)) == 0U)) {
	}
	i = 0;
	while ((i++ < K3_RTC_TIMEOUT) && ((mmio_read_32(K3_RTC_BASE + RTC_SYNCPEND) & BIT(2)) != 0U)) {
	}

	/* Clear wake up interrupt */
	rtc_unlock();
	mmio_write_32(K3_RTC_BASE + RTC_IRQSTATUS_SYS, intr_src);
	rtc_lock();
	wait_for_write_pend();

	ctrl = mmio_read_32(K3_RTC_BASE + RTC_SYNCPEND);
	ctrl = ctrl | RELOAD_FROM_BBD;
	rtc_unlock();
	mmio_write_32(K3_RTC_BASE + RTC_SYNCPEND, ctrl);
	wait_for_write_pend();
}

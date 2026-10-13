/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdbool.h>

#include "../drivers/arm/gic/v3/gicv3_private.h"

#include <drivers/arm/gicv3.h>
#include <drivers/delay_timer.h>
#include <drivers/nxp/trdc/imx_trdc.h>

#include <plat_imx8.h>
#include <platform_def.h>
#include <pwr_ctrl.h>

bool has_wakeup_irq;
struct plat_gic_ctx imx_gicv3_ctx;
bool gpio_wakeup;

static uint32_t gpio_ctrl_offset[GPIO_CTRL_REG_NUM] = {
	0xc, 0x10, 0x14, 0x18,
	0x1c, 0x40, 0x54, 0x58
};

/* Context save/restore for wdog3-5 in wakeupmix */
static uint32_t wdog_val[3][2];

/*
 * NIC GPV QoS register offsets:
 *   Rd QoS = base + 0x100, Wr QoS = base + 0x104
 */
#define NIC_GPV_QOS_RD(port)	((port) + U(0x100))
#define NIC_GPV_QOS_WR(port)	((port) + U(0x104))

void nicmix_qos_init(void)
{
	mmio_write_32(AIPS4_BASE + 0x10, 0x44);
	mmio_write_32(AIPS4_BASE + 0x14, 0x330033);
	mmio_write_32(AIPS4_BASE + 0x18, 0x330033);
	mmio_write_32(AIPS4_BASE + 0x1c, 0x330033);
	mmio_write_32(AIPS4_BASE + 0x20, 0x440044);
}

void wakeupmix_qos_init(void)
{
	static const uintptr_t wakeup_gpv_ports[] = {
		AIPS3_BASE + U(0x46000),
		AIPS3_BASE + U(0x47000),
		AIPS3_BASE + U(0x42000),
		AIPS3_BASE + U(0x43000),
		AIPS3_BASE + U(0x45000),
		NIC_MAIN_GPV_BASE + U(0x47000),
		NIC_MAIN_GPV_BASE + U(0x45000),
	};
	unsigned int i;

	for (i = 0U; i < ARRAY_SIZE(wakeup_gpv_ports); i++) {
		mmio_write_32(NIC_GPV_QOS_RD(wakeup_gpv_ports[i]), 0x4);
		mmio_write_32(NIC_GPV_QOS_WR(wakeup_gpv_ports[i]), 0x4);
	}
}

bool is_wakeup_source(unsigned int irq)
{
	uint32_t val;

	val = mmio_read_32(CM_SLICE(CPU_A55_PLAT) + CM_IRQ_WAKEUP_MASKx(IRQ_REG_IDX(irq)));
	return !(val & IRQ_BIT_MASK(irq));
}

void imx_set_sys_wakeup(unsigned int last_core, bool pdn)
{
	unsigned int i;
	uint32_t irq_mask;
	uintptr_t gicd_base = PLAT_GICD_BASE;

	if (pdn) {
		/*
		 * If NICMIX power down, need to switch the primary core & cluster wakeup
		 * source to GPC as GIC will be power down.
		 */
		gpc_select_wakeup_raw_irq(CPU_A55C0);
		gpc_select_wakeup_raw_irq(CPU_A55_PLAT);
	} else {
		/* Switch to GIC wakeup source for last_core and cluster */
		gpc_select_wakeup_gic(CPU_A55C0);
		gpc_select_wakeup_gic(CPU_A55_PLAT);
	}

	/* Set the GPC IMRs based on GIC IRQ mask setting */
	for (i = 0; i < IMR_NUM; i++) {
		if (pdn) {
			/* Set the wakeup irq base GIC */
			irq_mask = ~gicd_read_isenabler(gicd_base, 32 * (i + 1));
		} else {
			irq_mask = 0xFFFFFFFF;
		}

		/*
		 * If any of the non gpio is enabled, that means wakeupmix
		 * should be keep on to make sure these irqs can wakeup system
		 * successfully.
		 */
		if (~irq_mask & wakeupmix_irq_mask[i]) {
			has_wakeup_irq = true;
		}
		/* Set the mask into core & cluster GPC IMR */
		gpc_set_irq_mask(CPU_A55C0, i, irq_mask);
		gpc_set_irq_mask(CPU_A55_PLAT, i, irq_mask);
	}
}

void nicmix_pwr_down(unsigned int core_id)
{
	/* Enable the handshake between sentinel & NICMIX */
	mmio_setbits_32(BLK_CTRL_S_BASE + HW_LP_HANDHSK, BIT(11));

	/* Switch wakeup axi, hsio & nic to 24M when NICMIX power down */
	clock_root[1] = mmio_read_32(CCM_ROOT_SLICE(WAKEUP_AXI_ROOT));
	clock_root[2] = mmio_read_32(CCM_ROOT_SLICE(HSIO_CLK_ROOT));
	clock_root[3] = mmio_read_32(CCM_ROOT_SLICE(NIC_CLK_ROOT));
	mmio_clrbits_32(CCM_ROOT_SLICE(WAKEUP_AXI_ROOT), ROOT_MUX_MASK);
	mmio_clrbits_32(CCM_ROOT_SLICE(HSIO_CLK_ROOT), ROOT_MUX_MASK);
	mmio_clrbits_32(CCM_ROOT_SLICE(NIC_CLK_ROOT), ROOT_MUX_MASK);

	/* NICMIX */
	src_mix_set_lpm(SRC_NIC, 0x3, CM_MODE_WAIT);
	src_authen_config(SRC_NIC, 0x8, 0x1);

	/* NICMIX OCRAM memory retention */
	src_mem_lpm_en(SRC_NIC_MEM, MEM_OFF);
	/* OCRAM MEM */
	src_mem_lpm_en(SRC_NIC_OCRAM, MEM_RETN);

	/* Save the gic context */
	plat_gic_save(core_id, &imx_gicv3_ctx);
	imx_set_sys_wakeup(core_id, true);
}

void nicmix_pwr_up(unsigned int core_id)
{
	mmio_setbits_32(CCM_ROOT_SLICE(WAKEUP_AXI_ROOT), clock_root[1] & ROOT_MUX_MASK);
	mmio_setbits_32(CCM_ROOT_SLICE(HSIO_CLK_ROOT), clock_root[2] & ROOT_MUX_MASK);
	mmio_setbits_32(CCM_ROOT_SLICE(NIC_CLK_ROOT), clock_root[3] & ROOT_MUX_MASK);

	/* Keep nicmix on when exit from system suspend */
	src_mix_set_lpm(SRC_NIC, 0x3, CM_MODE_SUSPEND);
	trdc_n_reinit();
#if defined(PLAT_imx93)
	nicmix_qos_init();
#endif
	plat_gic_restore(core_id, &imx_gicv3_ctx);
	imx_set_sys_wakeup(core_id, false);
}

void set_gpio_secure(bool ns)
{
	/* GPIO2-4 */
	/* GPIO2 TRDC_W MBC0 MEM 1 */
	trdc_mbc_blk_config(0x42460000, 0, 3, 1, 0x0, ns, 0);
	/* GPIO3 TRDC_W MBC0 MEM 2 */
	trdc_mbc_blk_config(0x42460000, 0, 3, 2, 0x0, ns, 0);
	/* GPIO4 TRDC_W MBC1 MEM 3 */
	trdc_mbc_blk_config(0x42460000, 1, 3, 3, 0x0, ns, 0);
}

void gpio_save(struct gpio_ctx *ctx, int port_num)
{
	unsigned int i, j;

	/* Enable GPIO secure access */
	set_gpio_secure(true);

	for (i = 0; i < port_num; i++) {
		/* Save the port control setting */
		for (j = 0; j < GPIO_CTRL_REG_NUM; j++) {
			if (j < 4) {
				ctx->port_ctrl[j] = mmio_read_32(ctx->base + gpio_ctrl_offset[j]);
				/*
				 * Clear the permission setting to read the GPIO
				 * non-secure world setting.
				 */
				mmio_write_32(ctx->base + gpio_ctrl_offset[j], 0x0);
			} else {
				ctx->port_ctrl[j] = mmio_read_32(ctx->base + gpio_ctrl_offset[j]);
			}
		}
		/* Save the gpio icr setting */
		for (j = 0; j < ctx->pin_num; j++) {
			ctx->gpio_icr[j] = mmio_read_32(ctx->base + 0x80 + j * 4);

			/* Check if any gpio irq is enabled as wakeup source */
			if (ctx->gpio_icr[j]) {
				gpio_wakeup = true;
			}
		}

		/* Permission config restore back */
		for (j = 0; j < 4; j++) {
			mmio_write_32(ctx->base + gpio_ctrl_offset[j], ctx->port_ctrl[j]);
		}

		ctx++;
	}
	/* Disable GPIO secure access */
	set_gpio_secure(false);
}

void gpio_restore(struct gpio_ctx *ctx, int port_num)
{
	unsigned int i, j;

	set_gpio_secure(true);

	for (i = 0; i < port_num; i++) {
		for (j = 0; j < ctx->pin_num; j++) {
			mmio_write_32(ctx->base + 0x80 + j * 4, ctx->gpio_icr[j]);
		}

		for (j = 4; j < GPIO_CTRL_REG_NUM; j++) {
			mmio_write_32(ctx->base + gpio_ctrl_offset[j], ctx->port_ctrl[j]);
		}

		/* Permission config restore last */
		for (j = 0; j < 4; j++) {
			mmio_write_32(ctx->base + gpio_ctrl_offset[j], ctx->port_ctrl[j]);
		}

		ctx++;
	}

	set_gpio_secure(false);
}

void wdog_save(uintptr_t base, uint32_t index)
{
	/* Enable wdog clock */
	mmio_write_32(LPCG(WDOG3_LPCG + index), 0x1);

	/* Save the CS & TOVAL register */
	wdog_val[index][0] = mmio_read_32(base);
	wdog_val[index][1] = mmio_read_32(base + 0x8);

	mmio_write_32(LPCG(WDOG3_LPCG + index), 0x0);
}

void wdog_restore(uintptr_t base, uint32_t index)
{
	uint32_t cs, toval;
	unsigned int timeout;

	mmio_write_32(LPCG(WDOG3_LPCG + index), 0x1);

	cs = mmio_read_32(base);
	toval = mmio_read_32(base + 0x8);

	if (cs == wdog_val[index][0] &&
	    toval == wdog_val[index][1]) {
		mmio_write_32(LPCG(WDOG3_LPCG + index), 0x0);
		return;
	}

	mmio_write_32(base, wdog_val[index][0]);
	mmio_write_32(base + 0x8, wdog_val[index][1]);

	timeout = 10000U;
	while ((mmio_read_32(base) & BIT(11)) && (--timeout > 0U)) {
		udelay(1);
	}

	timeout = 10000U;
	while (!(mmio_read_32(base) & BIT(10)) && (--timeout > 0U)) {
		udelay(1);
	}

	mmio_write_32(LPCG(WDOG3_LPCG + index), 0x0);
}

void wakeupmix_pwr_down(void)
{
	wdog_save(WDOG3_BASE, 0);
	wdog_save(WDOG4_BASE, 1);
	wdog_save(WDOG5_BASE, 2);
	gpio_save(wakeupmix_gpio_ctx, 3);
	if (!(gpio_wakeup || has_wakeup_irq)) {
		/* M33 root need to switch to 24M OSC when wakeupmix power down */
		clock_root[0] = mmio_read_32(CCM_ROOT_SLICE(M33_ROOT));
		mmio_clrbits_32(CCM_ROOT_SLICE(M33_ROOT), ROOT_MUX_MASK);

		/* Wakeup mix controlled by A55 cluster power down: domain3 only */
		src_mix_set_lpm(SRC_WKUP, 0x3, CM_MODE_WAIT);
		src_authen_config(SRC_WKUP, 0x8, 0x1);
		/* Wakeupmix mem off */
		src_mem_lpm_en(SRC_WKUP_MEM, MEM_OFF);
		/* Enable the handshake between sentinel & wakeupmix */
		mmio_setbits_32(BLK_CTRL_S_BASE + HW_LP_HANDHSK, BIT(9));
	}
}

void wakeupmix_pwr_up(void)
{
	if (!(gpio_wakeup || has_wakeup_irq)) {
		mmio_setbits_32(CCM_ROOT_SLICE(M33_ROOT), clock_root[0] & ROOT_MUX_MASK);
		/* Keep wakeupmix on when exit from system suspend */
		src_mix_set_lpm(SRC_WKUP, 0x3, CM_MODE_SUSPEND);
		mmio_clrbits_32(SRC_BASE + 0xc00 + 0x4, BIT(2));
		trdc_w_reinit();
#if defined(PLAT_imx93)
		wakeupmix_qos_init();
#endif
		gpio_restore(wakeupmix_gpio_ctx, 3);
		wdog_restore(WDOG3_BASE, 0);
		wdog_restore(WDOG4_BASE, 1);
		wdog_restore(WDOG5_BASE, 2);
	}

	/*
	 * After wakeup, revert back to 'false', so next time
	 * evaluation for wakeupmix on/off can work well.
	 */
	has_wakeup_irq = false;
	gpio_wakeup = false;
}

void pll_pwr_down(bool enter)
{
	unsigned int i;

	if (enter) {
		/* Switch the ARM/SYS PLLs to hw_ctrl(bit 16) in PLL CTRL reg */
		mmio_setbits_32(ARM_PLL_BASE, PLL_HW_CTRL_EN);
		mmio_setbits_32(SYS_PLL_BASE, PLL_HW_CTRL_EN);
		mmio_setbits_32(SYS_PLL_DFS_0, PLL_HW_CTRL_EN);
		mmio_setbits_32(SYS_PLL_DFS_1, PLL_HW_CTRL_EN);
		mmio_setbits_32(SYS_PLL_DFS_2, PLL_HW_CTRL_EN);

		/* LPM setting for PLL */
		for (i = 1U; i <= OSCPLL_NUM; i++) {
			mmio_setbits_32(OSCPLL_CHAN(i) + OSCPLL_LPM0,
					OSCPLL_LPM_DOMAIN_MODE(0x1, 0x3));
			mmio_setbits_32(OSCPLL_CHAN(i) + OSCPLL_LPM_AUTH, BIT(2));
		}
	} else {
		mmio_clrbits_32(ARM_PLL_BASE, PLL_HW_CTRL_EN);
		mmio_clrbits_32(SYS_PLL_BASE, PLL_HW_CTRL_EN);
		mmio_clrbits_32(SYS_PLL_DFS_0, PLL_HW_CTRL_EN);
		mmio_clrbits_32(SYS_PLL_DFS_1, PLL_HW_CTRL_EN);
		mmio_clrbits_32(SYS_PLL_DFS_2, PLL_HW_CTRL_EN);

		for (i = 1U; i <= OSCPLL_NUM; i++) {
			mmio_clrbits_32(OSCPLL_CHAN(i) + OSCPLL_LPM_AUTH, BIT(2));
		}
	}
}

/* Do the necessary GPC, SRC, BLK_CTRL_S init */
void pwr_sys_init(void)
{
	unsigned int cpu;

	/*
	 * Assigned A55 cluster to 3, m33 to 2, A55 CORE0 & CORE1 to 0/1.
	 * domain0/1 only used for trigger LPM of themselves. A55 cluster & M33's
	 * domain assignment should be align with the TRDC DID.
	 */
	gpc_assign_domains(0x3102);

	/* CA55 core0/1 config */
	for (cpu = CPU_A55C0; cpu <= CPU_A55_PLAT; cpu++) {
		/* Clear the cpu sleep hold */
		gpc_clear_cpu_sleep_hold(cpu);
		/* Use gic wakeup source by default */
		gpc_select_wakeup_gic(cpu);
		/*
		 * Ignore A55 core0/1's LPM trigger for system sleep.
		 * normally, for A55 side, only the A55 cluster(plat)
		 * domain will be used to trigger the system wide low
		 * power mode transition.
		 */
		if (cpu != CPU_A55_PLAT) {
			gpc_force_cpu_suspend(cpu);
		}
	}

	/* Boot core(A55C0) */
	src_mem_lpm_en(SRC_A55P0_MEM, MEM_OFF);
	/* For A55 core, only need to be on in RUN mode */
	src_mix_set_lpm(SRC_A55C0, 0x0, CM_MODE_WAIT);
	/* Set CNT_MODE=0 to reduce unnecessary latency */
	src_ack_cnt_mode(SRC_A55C0, 0x0);
	/* Whitelist: 0x1 for domain 0 only */
	src_authen_config(SRC_A55C0, 0x1, 0x1);

	/* A55 cluster */
	gpc_select_wakeup_gic(CPU_A55_PLAT);
	gpc_clear_cpu_sleep_hold(CPU_A55_PLAT);

	/* SCU MEM must be OFF when A55 PLAT OFF */
	src_mem_lpm_en(SRC_A55_SCU_MEM, MEM_OFF);
	/* L3 memory in retention by default */
	src_mem_lpm_en(SRC_A55_L3_MEM, MEM_RETN);

	src_mix_set_lpm(SRC_A55P, 0x3, 0x1);
	/* Set CNT_MODE=0 to reduce unnecessary latency */
	src_ack_cnt_mode(SRC_A55P, 0x0);
	/* Whitelist: 0x8 for domain 3 only */
	src_authen_config(SRC_A55P, 0x8, 0x1);

	/* enable the HW LP handshake between S401 & A55 cluster */
	mmio_setbits_32(BLK_CTRL_S_BASE + HW_LP_HANDHSK, BIT(5));

	/* Set the PMIC STBY OFF delay to 1.5ms by default */
	mmio_clrsetbits_32(GPC_GLOBAL_BASE + PMIC_STBY_ACK_CTRL,
			   PMIC_STBY_OFF_DELAY_MASK,
			   PMIC_STBY_OFF_DELAY(PMIC_STBY_OFF_DELAY_1_5MS));

	/* Enable S401 clock gating LP handshake */
	mmio_setbits_32(BLK_CTRL_S_BASE + HW_LP_HANDHSK, BIT(24) | BIT(23));
	mmio_setbits_32(LPCG(3) + 0x10, BIT(13) | BIT(12));
	mmio_setbits_32(LPCG(3) + 0x30, BIT(2));

	/* Init wakeupmix and nocmix to always on by default */
	mmio_write_32(SRC_SLICE(SRC_WKUP) + SRC_LPM_SETTING1, 0x44444444);
	mmio_write_32(SRC_SLICE(SRC_WKUP) + SRC_LPM_SETTING2, 0x44444444);
	mmio_write_32(SRC_SLICE(SRC_NIC) + SRC_LPM_SETTING1, 0x44444444);
	mmio_write_32(SRC_SLICE(SRC_NIC) + SRC_LPM_SETTING2, 0x44444444);
}

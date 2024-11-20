/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../drivers/arm/gic/v3/gicv3_private.h"

#include <drivers/arm/gic.h>
#include <lib/mmio.h>
#include <scmi_imx9.h>

#include <ele_api.h>
#include <imx9_sys_sleep.h>
#include <imx_scmi_client.h>
#include <plat_imx8.h>

#define IRQ_MASK(x)	((x) / 32U)
#define IRQ_SHIFT(x)	(1U << (x) % 32U)

static uint32_t irq_mask[IMR_NUM] = { 0x0 };
static struct scmi_per_lpm_config per_lpm[PER_NUM];

static const uint32_t gpio_ctrl_offset[GPIO_CTRL_REG_NUM] = {
	 0xc, 0x10, 0x14, 0x18, 0x1c, 0x40, 0x54, 0x58
};

/* for GIC context save/restore if NIC lost power */
struct plat_gic_ctx imx_gicv3_ctx;

bool has_netc_irq;
static uint32_t wakeup_mark_count;
static bool gpio_wakeup;
bool keep_wakeupmix_on;

#if HAS_XSPI_SUPPORT && !IMX_CRRM
static uint32_t xspi_mto[2];

static void xspi_save(void)
{
	/* Save the XSPI MTO register */
	xspi_mto[0]  = mmio_read_32(XSPI1_BASE + XSPI_MTO);
#if XSPI2_BASE
	xspi_mto[1]  = mmio_read_32(XSPI2_BASE + XSPI_MTO);
#endif
}

static void xspi_restore(void)
{
	/* request the GMID first */
	ele_release_gmid();
	mmio_write_32(XSPI1_BASE + XSPI_MTO, xspi_mto[0]);
#if XSPI2_BASE
	mmio_write_32(XSPI2_BASE + XSPI_MTO, xspi_mto[1]);
#endif
}
#endif

static void gpio_save(struct gpio_ctx *ctx)
{
	for (uint32_t i = 0U; i < GPIO_CTRL_REG_NUM; i++) {
		/* First 4 regs for permission */
		if (i < 4U) {
			ctx->port_ctrl[i] = mmio_read_32(ctx->base + gpio_ctrl_offset[i]);
			/* Clear the permission to read the gpio non-secure setting. */
			mmio_write_32(ctx->base + gpio_ctrl_offset[i], 0x0);
		} else {
			ctx->port_ctrl[i] = mmio_read_32(ctx->base + gpio_ctrl_offset[i]);
		}
	}

	/* Save the gpio icr */
	for (uint32_t i = 0U; i < ctx->pin_num; i++) {
		ctx->gpio_icr[i] = mmio_read_32(ctx->base + 0x80 + i * 4U);
		/* Mark if any gpio pin is used as wakeup irq */
		if (ctx->gpio_icr[i]) {
			gpio_wakeup = true;
		}
	}

	/* Restore the gpio permission */
	for (uint32_t i = 0U; i < 4U; i++) {
		mmio_write_32(ctx->base + gpio_ctrl_offset[i], ctx->port_ctrl[i]);
	}
}
static void gpio_restore(struct gpio_ctx *ctx)
{
	/* Clear the gpio permission */
	for (uint32_t i = 0U; i < 4U; i++) {
		mmio_write_32(ctx->base + gpio_ctrl_offset[i], 0x0);
	}

	for (uint32_t i = 0U; i < ctx->pin_num; i++) {
		mmio_write_32(ctx->base + 0x80 + i * 4U, ctx->gpio_icr[i]);
	}

	for (uint32_t i = 4U; i < GPIO_CTRL_REG_NUM; i++)
		mmio_write_32(ctx->base + gpio_ctrl_offset[i], ctx->port_ctrl[i]);

	/* Permission config retore last */
	for (uint32_t i = 0U; i < 4U; i++) {
		mmio_write_32(ctx->base + gpio_ctrl_offset[i], ctx->port_ctrl[i]);
	}

	gpio_wakeup = false;
}

static void wdog_save(struct wdog_ctx *wdog)
{
	wdog->regs[0] = mmio_read_32(wdog->base);
	wdog->regs[1] = mmio_read_32(wdog->base + 0x8);
}

static void wdog_restore(struct wdog_ctx *wdog)
{
	uint32_t cs, toval;

	cs = mmio_read_32(wdog->base);
	toval = mmio_read_32(wdog->base + 0x8);

	/* Wdog does not lost context, no need to restore */
	if (cs == wdog->regs[0] && toval == wdog->regs[1]) {
		return;
	}

	/* Reconfig the CS */
	mmio_write_32(wdog->base, wdog->regs[0]);
	/* Set the tiemout value */
	mmio_write_32(wdog->base + 0x8, wdog->regs[1]);

	/* Wait for the lock status */
	while ((mmio_read_32(wdog->base) & BIT(11))) {
		;
	}

	/* Wait for the config done */
	while (!(mmio_read_32(wdog->base) & BIT(10))) {
		;
	}
}

static inline bool active_wakeup_irq(uint32_t irq)
{
	return !(irq_mask[IRQ_MASK(irq)] & IRQ_SHIFT(irq));
}

/*
 * For peripherals like CANs, GPIOs & UARTs that need to support
 * async wakeup when clock is gated, LPCGs of these IPs need to be
 * changed to CPU LPM controlled, and for CANs &UARTs, we also need
 * to make sure its ROOT clock slice is enabled.
 */
static void peripheral_qchannel_hsk(bool en)
{
	uint32_t num_hsks = 0U;

	for (uint32_t i = 0U; i < ARRAY_SIZE(per_hsk_cfg); i++) {
		/*
		 * We assume the per_hsk_cfg array valid entry ends
		 * if wakeup_irq = 0
		 */
		if (per_hsk_cfg[i].wakeup_irq == 0U) {
			break;
		}

		if (active_wakeup_irq(per_hsk_cfg[i].wakeup_irq)) {
			per_lpm[num_hsks].perId = per_hsk_cfg[i].per_idx;
			per_lpm[num_hsks].lpmSetting = en ? SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP :
							    SCMI_CPU_PD_LPM_ON_ALWAYS;
			num_hsks++;
		}
	}

	scmi_per_lpm_mode_set(imx9_scmi_handle, IMX9_SCMI_CPU_A55P,
			      num_hsks, per_lpm);
}

void imx_set_sys_wakeup(uint32_t last_core, bool pdn)
{
	uintptr_t gicd_base = PLAT_GICD_BASE;
	uint32_t mask;

	/* Clear the wakeup and netc irq enabled flags */
	wakeup_mark_count = 0;
	has_netc_irq = false;

	/* Set the GPC IMRs based on GIC IRQ mask setting */
	for (uint32_t i = 0U; i < IMR_NUM; i++) {
		if (pdn) {
			/* set the wakeup irq based on GIC */
			irq_mask[i] =
				~gicd_read_isenabler(gicd_base, 32 * (i + 1));
		} else {
			irq_mask[i] = 0xFFFFFFFF;
		}

		mask = ~irq_mask[i] & wakeup_irq_mask[i];

		if (!mask) {
			continue;
		}

		/* If mask is not zero, increase the mark_count */
		wakeup_mark_count++;

		if (i == IRQ_MASK(NETC_IREC_PCI_INT_X0) &&
		    (mask & IRQ_SHIFT(NETC_IREC_PCI_INT_X0))) {
			/*
			 * If only this NETC interrupt in the mask, no need
			 * enable wakeupmix wakeup
			 */
			if (mask == IRQ_SHIFT(NETC_IREC_PCI_INT_X0)) {
				wakeup_mark_count--;
			}
			has_netc_irq = true;
		}
	}

	/* Set IRQ wakeup mask for the last core & cluster */
	scmi_core_Irq_wake_set(imx9_scmi_handle, IMX9_SCMI_CPU_A55P,
			       0, IMR_NUM, irq_mask);

	scmi_core_Irq_wake_set(imx9_scmi_handle, SCMI_CPU_A55_ID(last_core),
			       0, IMR_NUM, irq_mask);

	/* Configure low power wakeup source interface */
	peripheral_qchannel_hsk(pdn);
}

void imx9_sys_sleep_prepare(uint32_t core_id)
{
	/* Save the gic context */
	plat_gic_save(core_id, &imx_gicv3_ctx);

	/* Save contex of gpios in wakeupmix */
	for (uint32_t i = 0U; i < GPIO_NUM; i++) {
		gpio_save(&gpios[i]);
	}

	/* Save wdog3/4 ctx */
	for (uint32_t i = 0U; i < WDOG_NUM; i++) {
		wdog_save(&wdogs[i]);
	}

#if HAS_XSPI_SUPPORT && !IMX_CRRM
	xspi_save();
#endif
	imx_set_sys_wakeup(core_id, true);

	keep_wakeupmix_on = gpio_wakeup || wakeup_mark_count;

#if IMX_CRRM
	/* Keep XSPI always on to avoid setting lost */
	keep_wakeupmix_on = true;
#endif
}

void imx9_sys_sleep_unprepare(uint32_t core_id)
{
	/* Restore the gic context */
	plat_gic_restore(core_id, &imx_gicv3_ctx);

#if HAS_XSPI_SUPPORT && !IMX_CRRM
	xspi_restore();
#endif
	/* Restore contex of gpios in wakeupmix */
	for (uint32_t i = 0U; i < GPIO_NUM; i++) {
		gpio_restore(&gpios[i]);
	}

	/* Restore wdog3/4 ctx */
	for (uint32_t i = 0U; i < WDOG_NUM; i++) {
		wdog_restore(&wdogs[i]);
	}

	imx_set_sys_wakeup(core_id, false);
}

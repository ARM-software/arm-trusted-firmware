/*
 * Copyright (c) 2017-2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <libfdt.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32_iwdg.h>
#include <drivers/st/stm32mp_clkfunc.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

/* IWDG registers offsets */
#define IWDG_KR_OFFSET		0x00U

/* Registers values */
#define IWDG_KR_RELOAD_KEY	0xAAAA

struct stm32_iwdg_instance {
	uintptr_t base;
	unsigned long clock;
	uint8_t flags;
	int num_irq;
};

static struct stm32_iwdg_instance stm32_iwdg[IWDG_MAX_INSTANCE];

static int stm32_iwdg_get_dt_node(struct dt_node_info *info, int offset)
{
	int node;

	node = dt_get_node(info, offset, DT_IWDG_COMPAT);
	if (node < 0) {
		if (offset == -1) {
			VERBOSE("%s: No IDWG found\n", __func__);
		}
		return -FDT_ERR_NOTFOUND;
	}

	return node;
}

void stm32_iwdg_refresh(void)
{
	uint8_t i;

	for (i = 0U; i < IWDG_MAX_INSTANCE; i++) {
		struct stm32_iwdg_instance *iwdg = &stm32_iwdg[i];

		/* 0x00000000 is not a valid address for IWDG peripherals */
		if (iwdg->base != 0U) {
			clk_enable(iwdg->clock);

			mmio_write_32(iwdg->base + IWDG_KR_OFFSET,
				      IWDG_KR_RELOAD_KEY);

			clk_disable(iwdg->clock);
		}
	}
}

int stm32_iwdg_init(void)
{
	int node = -1;
	struct dt_node_info dt_info;
	void *fdt;
	uint32_t __unused count = 0;

	if (fdt_get_address(&fdt) == 0) {
		panic();
	}

	for (node = stm32_iwdg_get_dt_node(&dt_info, node);
	     node != -FDT_ERR_NOTFOUND;
	     node = stm32_iwdg_get_dt_node(&dt_info, node)) {
		struct stm32_iwdg_instance *iwdg;
		uint32_t hw_init;
		uint32_t idx;

		count++;

		idx = stm32_iwdg_get_instance(dt_info.base);
		iwdg = &stm32_iwdg[idx];
		iwdg->base = dt_info.base;
		iwdg->clock = (unsigned long)dt_info.clock;

		/* DT can specify low power cases */
		if (fdt_getprop(fdt, node, "stm32,enable-on-stop", NULL) ==
		    NULL) {
			iwdg->flags |= IWDG_DISABLE_ON_STOP;
		}

		if (fdt_getprop(fdt, node, "stm32,enable-on-standby", NULL) ==
		    NULL) {
			iwdg->flags |= IWDG_DISABLE_ON_STANDBY;
		}

		/* Explicit list of supported bit flags */
		hw_init = stm32_iwdg_get_otp_config(idx);

		if ((hw_init & IWDG_HW_ENABLED) != 0) {
			if (dt_info.status == DT_DISABLED) {
				ERROR("OTP enabled but iwdg%u DT-disabled\n",
				      idx + 1U);
				panic();
			}
			iwdg->flags |= IWDG_HW_ENABLED;
		}

		if (dt_info.status == DT_DISABLED) {
			zeromem((void *)iwdg,
				sizeof(struct stm32_iwdg_instance));
			continue;
		}

		if ((hw_init & IWDG_DISABLE_ON_STOP) != 0) {
			iwdg->flags |= IWDG_DISABLE_ON_STOP;
		}

		if ((hw_init & IWDG_DISABLE_ON_STANDBY) != 0) {
			iwdg->flags |= IWDG_DISABLE_ON_STANDBY;
		}

		VERBOSE("IWDG%u found, %ssecure\n", idx + 1U,
			((dt_info.status & DT_NON_SECURE) != 0) ?
			"non-" : "");

		if ((dt_info.status & DT_NON_SECURE) != 0) {
			stm32mp_register_non_secure_periph_iomem(iwdg->base);
		} else {
			stm32mp_register_secure_periph_iomem(iwdg->base);
		}

#if defined(IMAGE_BL2)
		if (stm32_iwdg_shadow_update(idx, iwdg->flags) != BSEC_OK) {
			return -1;
		}
#endif
	}

	VERBOSE("%u IWDG instance%s found\n", count, (count > 1U) ? "s" : "");

	return 0;
}

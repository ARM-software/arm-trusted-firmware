/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX9_SYS_SLEEP_H
#define IMX9_SYS_SLEEP_H

#include <stdbool.h>
#include <stdint.h>

#include <lib/utils_def.h>

#include <platform_def.h>

#define GPIO_CTRL_REG_NUM	U(8)
#define GPIO_PIN_MAX_NUM	U(32)
#define GPIO_CTX(addr, num)	\
	{ .base = (addr), .pin_num = (num), }

struct gpio_ctx {
	/* gpio base */
	uintptr_t base;
	/* port control */
	uint32_t port_ctrl[GPIO_CTRL_REG_NUM];
	/* GPIO ICR, Max 32 */
	uint32_t pin_num;
	uint32_t gpio_icr[GPIO_PIN_MAX_NUM];
};

#define WDOG_CTX(addr)		\
	{ .base = (addr), }

struct wdog_ctx {
	/* wdog base */
	uintptr_t base;
	uint32_t regs[2];
};

#define PER_HSK_CFG(idx, irq)	\
	{ .per_idx = (idx, wakeup_irq = (irq), }

struct per_hsk_cfg {
	const uint32_t per_idx;
	const uint32_t wakeup_irq;
};

extern struct gpio_ctx gpios[GPIO_NUM];
extern struct wdog_ctx wdogs[WDOG_NUM];
extern struct per_hsk_cfg per_hsk_cfg[PER_NUM];
extern uint32_t wakeup_irq_mask[IMR_NUM];
extern bool keep_wakeupmix_on;
extern bool has_netc_irq;

void imx9_sys_sleep_prepare(uint32_t core_id);
void imx9_sys_sleep_unprepare(uint32_t core_id);

#endif /* IMX9_SYS_SLEEP_H */

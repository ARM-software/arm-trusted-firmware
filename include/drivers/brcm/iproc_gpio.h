/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IPROC_GPIO_H
#define IPROC_GPIO_H

#ifdef USE_GPIO
void iproc_gpio_init(uintptr_t base, int nr_gpios, uintptr_t pinmux_base,
		     uintptr_t pinconf_base);
#else
static void iproc_gpio_init(uintptr_t base, int nr_gpios, uintptr_t pinmux_base,
			    uintptr_t pinconf_base)
{
}
#endif /* IPROC_GPIO */

#endif /* IPROC_GPIO_H */

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

#ifndef __BOARD_DRIVER_EXT_GPIO_MT6391_H__
#define __BOARD_DRIVER_EXT_GPIO_MT6391_H__

#include <stdint.h>

/*
 * PMIC GPIO REGISTER DEFINITION
 */
enum {
	MT6391_GPIO_DIR_BASE = 0xC000,
	MT6391_GPIO_PULLEN_BASE = 0xC020,
	MT6391_GPIO_PULLSEL_BASE = 0xC040,
	MT6391_GPIO_DOUT_BASE = 0xC080,
	MT6391_GPIO_DIN_BASE = 0xC0A0,
	MT6391_GPIO_MODE_BASE = 0xC0C0,
};

enum mt6391_pull_enable {
	MT6391_GPIO_PULL_DISABLE = 0,
	MT6391_GPIO_PULL_ENABLE = 1,
};

enum mt6391_pull_select {
	MT6391_GPIO_PULL_DOWN = 0,
	MT6391_GPIO_PULL_UP = 1,
};

/*
 * PMIC GPIO Exported Function
 */
int mt6391_gpio_get(uint32_t gpio);
void mt6391_gpio_set(uint32_t gpio, int value);
void mt6391_gpio_input_pulldown(uint32_t gpio);
void mt6391_gpio_input_pullup(uint32_t gpio);
void mt6391_gpio_input(uint32_t gpio);
void mt6391_gpio_output(uint32_t gpio, int value);
void mt6391_gpio_set_pull(uint32_t gpio, enum mt6391_pull_enable enable,
			  enum mt6391_pull_select select);
void mt6391_gpio_set_mode(uint32_t gpio, int mode);

#endif /* __BOARD_DRIVER_EXT_GPIO_MT6391_H__ */

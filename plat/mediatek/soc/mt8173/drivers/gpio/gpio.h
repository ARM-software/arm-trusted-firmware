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
#ifndef __PLAT_DRIVER_GPIO_H__
#define __PLAT_DRIVER_GPIO_H__

#include <stdint.h>
#include <mt8173_def.h>

enum pull_enable {
	GPIO_PULL_DISABLE = 0,
	GPIO_PULL_ENABLE = 1,
};

enum pull_select {
	GPIO_PULL_DOWN = 0,
	GPIO_PULL_UP = 1,
};

struct val_regs {
	uint16_t val;
	uint16_t align1;
	uint16_t set;
	uint16_t align2;
	uint16_t rst;
	uint16_t align3[3];
};

struct gpio_regs {
	struct val_regs dir[9];
	uint8_t rsv00[112];
	struct val_regs pullen[9];
	uint8_t rsv01[112];
	struct val_regs pullsel[9];
	uint8_t rsv02[112];
	uint8_t rsv03[256];
	struct val_regs dout[9];
	uint8_t rsv04[112];
	struct val_regs din[9];
	uint8_t rsv05[112];
	struct val_regs mode[27];
	uint8_t rsv06[336];
	struct val_regs ies[3];
	struct val_regs smt[3];
	uint8_t rsv07[160];
	struct val_regs tdsel[8];
	struct val_regs rdsel[6];
	uint8_t rsv08[32];
	struct val_regs drv_mode[10];
	uint8_t rsv09[96];
	struct val_regs msdc_rsv0[11];
	struct val_regs msdc2_ctrl5;
	struct val_regs msdc_rsv1[12];
	uint8_t rsv10[64];
	struct val_regs exmd_ctrl[1];
	uint8_t rsv11[48];
	struct val_regs kpad_ctrl[2];
	struct val_regs hsic_ctrl[4];
};

static struct gpio_regs *const mt8173_gpio = (void *)(GPIO_BASE);

int gpio_get(uint32_t gpio);
void gpio_set(uint32_t gpio, int output);
void gpio_input_pulldown(uint32_t gpio);
void gpio_input_pullup(uint32_t gpio);
void gpio_input(uint32_t gpio);
void gpio_output(uint32_t gpio, int value);
void gpio_set_pull(uint32_t gpio, enum pull_enable enable,
		   enum pull_select select);
void gpio_set_mode(uint32_t gpio, int mode);

#endif /* __PLAT_DRIVER_GPIO_H__ */

/*
 * Copyright (c) 2014-2016, Linaro Ltd and Contributors. All rights reserved.
 * Copyright (c) 2014-2016, Hisilicon Ltd and Contributors. All rights reserved.
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

#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdint.h>

#define GPIO_DIR_OUT		0
#define GPIO_DIR_IN		1

#define GPIO_LEVEL_LOW		0
#define GPIO_LEVEL_HIGH		1

typedef struct gpio_ops {
	int (*set_direction)(uint32_t gpio, uint32_t direction);
	int (*get_value)(uint32_t gpio);
	int (*set_value)(uint32_t gpio, uint32_t value);
} gpio_ops_t;

int gpio_set_direction(uint32_t gpio, uint32_t direction);
int gpio_get_value(uint32_t gpio);
int gpio_set_value(uint32_t gpio, uint32_t value);
void gpio_init(const gpio_ops_t *ops);

#endif	/* __GPIO_H__ */

/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
 *
 * GPIO -- General Purpose Input/Output
 *
 * Defines a simple and generic interface to access GPIO device.
 *
 */

#include <assert.h>
#include <errno.h>
#include <gpio.h>

/*
 * The gpio implementation
 */
static const gpio_ops_t *ops;

int gpio_get_direction(int gpio)
{
	assert(ops);
	assert(ops->get_direction != 0);
	assert(gpio >= 0);

	return ops->get_direction(gpio);
}

void gpio_set_direction(int gpio, int direction)
{
	assert(ops);
	assert(ops->set_direction != 0);
	assert((direction == GPIO_DIR_OUT) || (direction == GPIO_DIR_IN));
	assert(gpio >= 0);

	ops->set_direction(gpio, direction);
}

int gpio_get_value(int gpio)
{
	assert(ops);
	assert(ops->get_value != 0);
	assert(gpio >= 0);

	return ops->get_value(gpio);
}

void gpio_set_value(int gpio, int value)
{
	assert(ops);
	assert(ops->set_value != 0);
	assert((value == GPIO_LEVEL_LOW) || (value == GPIO_LEVEL_HIGH));
	assert(gpio >= 0);

	ops->set_value(gpio, value);
}

void gpio_set_pull(int gpio, int pull)
{
	assert(ops);
	assert(ops->set_pull != 0);
	assert((pull == GPIO_PULL_NONE) || (pull == GPIO_PULL_UP) ||
	       (pull == GPIO_PULL_DOWN));
	assert(gpio >= 0);

	ops->set_pull(gpio, pull);
}

int gpio_get_pull(int gpio)
{
	assert(ops);
	assert(ops->get_pull != 0);
	assert(gpio >= 0);

	return ops->get_pull(gpio);
}

/*
 * Initialize the gpio. The fields in the provided gpio
 * ops pointer must be valid.
 */
void gpio_init(const gpio_ops_t *ops_ptr)
{
	assert(ops_ptr != 0  &&
	       (ops_ptr->get_direction != 0) &&
	       (ops_ptr->set_direction != 0) &&
	       (ops_ptr->get_value != 0) &&
	       (ops_ptr->set_value != 0));

	ops = ops_ptr;
}

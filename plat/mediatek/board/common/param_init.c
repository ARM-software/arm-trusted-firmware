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
#include <assert.h>
#include <bl_common.h>
#include <board_gpio.h>
#include <da9212.h>
#include <debug.h>
#include <mt6311.h>
#include <mtk_board_func.h>
#include <param_init.h>

static struct board_gpio power_notify;
static struct {
	const int device_id;
	int (*init)(void *sub_param);
	void (*ctrl)(int enable);
} bucks[] = {
#if DA9212
	{ BUCK_DA9212, da9212_init, da9212_ctrl },
#endif
#if MT6311
	{ BUCK_MT6311, mt6311_init, mt6311_ctrl },
#endif
	{ BUCK_DUMMY, NULL, NULL },
};

void cluster1_buck_init(uint32_t sub_type, void *sub_param)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(bucks); ++i)
		if (bucks[i].device_id == sub_type)
			if (!bucks[i].init(sub_param)) {
				set_cluster1_buck_ctrl_callback(bucks[i].ctrl);
				break;
			}
}

static void notify_sys_off(void)
{
	if (power_notify.output)
		power_notify.output(power_notify.pin, power_notify.polarity);
}

void sys_off_init(uint32_t sub_type, void *sub_param)
{
	struct sys_off_params *p = (struct sys_off_params *)sub_param;

	fill_board_gpio(&p->gpio_info, &power_notify);
	set_sys_off_callback(&notify_sys_off);
}

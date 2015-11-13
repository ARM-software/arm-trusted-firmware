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
#include <da9212.h>
#include <debug.h>
#include <mt6311.h>
#include <mtk_board_func.h>
#include <plat_params.h>
#include <string.h>

#if MEDIATEK_MT6311
static struct bl31_mt6311_param param_mt6311;
#endif
#if MEDIATEK_DA9212
static struct bl31_da9212_param param_da9212;
#endif

static struct bl31_plat_param *bl31_params_head;

void params_early_setup(void *plat_param_from_bl2)
{
	struct bl31_plat_param *param;
	struct bl31_plat_param *bl2_param;
	size_t size;

	/* keep plat parameters for later processing */
	bl2_param = (struct bl31_plat_param *)plat_param_from_bl2;
	while (bl2_param) {
		switch (bl2_param->type) {
#if MEDIATEK_MT6311
		case PARAM_CLUSTER1_MT6311:
			size = sizeof(struct bl31_mt6311_param);
			param = (struct bl31_plat_param *)&param_mt6311;
			break;
#endif
#if MEDIATEK_DA9212
		case PARAM_CLUSTER1_DA9212:
			size = sizeof(struct bl31_da9212_param);
			param = (struct bl31_plat_param *)&param_da9212;
			break;
#endif
		default:
			assert(0);
			return; /* don't continue if unexpected type found */
		}
		memcpy((void *)param, (void *)bl2_param, size);
		param->next = bl31_params_head;
		bl31_params_head = param;
		bl2_param = bl2_param->next;
	}
}

void params_setup(void)
{
	struct bl31_plat_param *head = bl31_params_head;

	while (head) {
		switch (head->type) {
#if MEDIATEK_MT6311
		case PARAM_CLUSTER1_MT6311:
			if (!mt6311_init((struct bl31_mt6311_param *)head))
				set_cluster1_buck_ctrl_callback(&mt6311_ctrl);
			break;
#endif
#if MEDIATEK_DA9212
		case PARAM_CLUSTER1_DA9212:
			if (!da9212_init((struct bl31_da9212_param *)head))
				set_cluster1_buck_ctrl_callback(&da9212_ctrl);
			break;
#endif
		default:
			INFO("%s:unknown bl31_plat_param type:%lx\n",
				__func__, head->type);
			break;
		}
		head = head->next;
	}
}

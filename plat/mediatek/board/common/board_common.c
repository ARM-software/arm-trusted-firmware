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
#include <board_params.h>
#include <debug.h>

static struct bl31_plat_params plat_params;

static void board_param_init(uint32_t type, uint32_t sub_type, void *sub_param)
{
	switch (type) {
	case PARAM_CLUSTER1_BUCK:
		cluster1_buck_init(sub_type, sub_param);
		break;
	case PARAM_SYS_OFF:
		sys_off_init(sub_type, sub_param);
		break;
	default:
		INFO("%s:unhandled param type:%x\n", __func__, type);
		break;
	}
}

void board_early_setup(struct bl31_plat_params *params)
{
	/* just keep the parameters from earlier boot stage */
	plat_params = *params;
}

void board_setup(void)
{
	struct board_param *bp;
	int i;

	bp = plat_params.param_body;
	for (i = 0; i < plat_params.num_of_param; i++) {
		assert(bp != NULL);
		board_param_init(bp->type, bp->sub_type, &bp->sub_param);
		bp = bp->next_board_param;
	}
}

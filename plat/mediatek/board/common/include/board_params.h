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
#ifndef __BOARD_PARAMS_H__
#define __BOARD_PARAMS_H__

#include <board_gpio.h>
#include <sys/types.h>
#include <param_init.h>

/*
 * struct bl31_plat_params and board_param are used for earlier boot stage to
 * pass board specific parameters to Mediatek ARM TF platform code. The picture
 * below shows how board specific parameters are kept in structure.
 * struct bl31_plat_params is passed to ARM TF via register x1 (We consider
 * !RESET_TO_BL31 case only now).
 * The member of bl31_plat_params:
 * - bl31_magic: the magic number used to validate parameter passed from earlier
 *               boot stage
 * - num_of_param: how many board params to be handled
 * - param_body: pointer to the board_param struct
 *
 * struct board_param is kept in memory. The member of board_param:
 * - type: param type (e.g. PARAM_EXT_BUCK, PARAM_SYS_OFF)
 * - sub_type: sub type of param type (e.g. DA9212 or MT6311 of PARAM_EXT_BUCK)
 * - next_board_param: pointer to next board param if any, or NULL
 * - sub_param: place holder for the struct of specific sub_type param
 *
 *
 *  bl31_plat_params                     board_param
 *  +--------------+   +------> +--------------------------+  <-- shared memory
 *  | bl31_magic   |   |        | type: PARAM_CLUSTER1_BUCK|
 *  +--------------+   |        +--------------------------+
 *  | num_of_param |   |        |   sub_type: BUCK_DA9212  |
 *  +--------------+   |        +--------------------------+
 *  | param_body   |---+   +----|     next_board_param     |
 *  +--------------+       |    +--------------------------+
 *                         |    | sub_param: da9212_params |
 *                         +--> +--------------------------+
 *                              |    type: PARAM_SYS_OFF   |
 *                              +--------------------------+
 *                              |    sub_type: 0 (not used)|
 *                              +--------------------------+
 *                     NULL <---|     next_board_param     |
 *                              +--------------------------+
 *                              | sub_param: sys_off_params|
 *                              +--------------------------+
 *
 * Note: The SCTLR_EL3.A bit (Alignment fault check enable) of ARM TF is set,
 * so be sure to insert padding data to sub_params struct to make it multiple
 * of 64-bit. Then, all the start address of following board_param could be
 * aligned on 64-bit. Otherwise, we will have alignment fault during accessing
 * board_param data member.
 */

/* param type */
enum {
	PARAM_CLUSTER1_BUCK = 0,	/* param about cluster1 buck */
	PARAM_SYS_OFF = 1,		/* param about system off */
};

struct board_param {
	uint32_t type;
	uint32_t sub_type;
	struct board_param *next_board_param;
	void *sub_param; /* place holder */
};

struct bl31_plat_params {
	uint64_t bl31_magic;
	uint32_t num_of_param;
	struct board_param *param_body;
};

/* PARAM_CLUSTER1_BUCK sub type param */
/* buck sub_type */
enum {
	BUCK_DUMMY = -1,
	BUCK_DA9212 = 0,
	BUCK_MT6311 = 1,
};

struct da9212_params {
	uint32_t i2c_bus;
	struct board_gpio_info gpio_ic_en;
	struct board_gpio_info gpio_en_a;
	struct board_gpio_info gpio_en_b;
};

struct mt6311_params {
	uint32_t i2c_bus;
	uint32_t padding; /* padding struct to multiple of 64-bit */
};

/* PARAM_SYS_OFF sub type param */
struct sys_off_params {
	struct board_gpio_info gpio_info;
	uint32_t padding; /* padding struct to multiple of 64-bit */
};

/*******************************************************************************
 * functions provided to soc
 ******************************************************************************/

/* keep bl31_plat_params from earlier boot stage for later use */
void board_early_setup(struct bl31_plat_params *params);

/* board related initialization */
void board_setup(void);

#endif /* __BOARD_PARAMS_H__ */

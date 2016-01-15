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
#ifndef __PMIC_MT6311_H__
#define __PMIC_MT6311_H__

#include <plat_params.h>

enum {
	MT6311_CID = 0x0,
	MT6311_SWCID = 0x1,
	MT6311_GPIO_MODE = 0x04,
	MT6311_TOP_RST_CON = 0x15,
	MT6311_TOP_INT_CON = 0x18,
	MT6311_STRUP_CON5 = 0x1F,
	MT6311_EFUSE_DOUT_56_63 = 0x40,
	MT6311_EFUSE_DOUT_64_71 = 0x41,
	MT6311_BUCK_ALL_CON23 = 0x69,
	MT6311_STRUP_ANA_CON1 = 0x6D,
	MT6311_STRUP_ANA_CON2 = 0x6E,
	MT6311_VDVFS1_ANA_CON10 = 0x84,
	MT6311_VDVFS11_CON7 = 0x88,
	MT6311_VDVFS11_CON9 = 0x8A,
	MT6311_VDVFS11_CON10 = 0x8B,
	MT6311_VDVFS11_CON11 = 0x8C,
	MT6311_VDVFS11_CON12 = 0x8D,
	MT6311_VDVFS11_CON13 = 0x8E,
	MT6311_VDVFS11_CON14 = 0x8F,
	MT6311_VDVFS11_CON19 = 0x94,
	MT6311_LDO_CON3 = 0xCF,
};

enum {
	MT6311_E1_CID_CODE = 0x0110,
	MT6311_E2_CID_CODE = 0x0120,
	MT6311_E3_CID_CODE = 0x0130,
};

/* initialize mt6311 */
int mt6311_init(struct bl31_mt6311_param *param);

/* enable or disable mt6311 */
void mt6311_ctrl(int enable);

#endif /* __PMIC_MT6311_H__ */

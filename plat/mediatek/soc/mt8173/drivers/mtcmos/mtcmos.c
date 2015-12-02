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
#include <mmio.h>
#include <mt8173_def.h>
#include <mtcmos.h>
#include <spm.h>

enum {
	SRAM_ISOINT_B	= 1U << 6,
	SRAM_CKISO	= 1U << 5,
	PWR_CLK_DIS	= 1U << 4,
	PWR_ON_2ND	= 1U << 3,
	PWR_ON		= 1U << 2,
	PWR_ISO		= 1U << 1,
	PWR_RST_B	= 1U << 0
};

enum {
	L1_PDN_ACK	= 1U << 8,
	L1_PDN		= 1U << 0
};

enum {
	LITTLE_CPU3	= 1U << 12,
	LITTLE_CPU2	= 1U << 11,
	LITTLE_CPU1	= 1U << 10,
};

enum {
	SRAM_PDN           = 0xf << 8,
	DIS_SRAM_ACK       = 0x1 << 12,
	AUD_SRAM_ACK       = 0xf << 12,
};

enum {
	DIS_PWR_STA_MASK   = 0x1 << 3,
	AUD_PWR_STA_MASK   = 0x1 << 24,
};

static void mtcmos_ctrl_little_off(unsigned int linear_id)
{
	uint32_t reg_pwr_con;
	uint32_t reg_l1_pdn;
	uint32_t bit_cpu;

	switch (linear_id) {
	case 1:
		reg_pwr_con = SPM_CA7_CPU1_PWR_CON;
		reg_l1_pdn = SPM_CA7_CPU1_L1_PDN;
		bit_cpu = LITTLE_CPU1;
		break;
	case 2:
		reg_pwr_con = SPM_CA7_CPU2_PWR_CON;
		reg_l1_pdn = SPM_CA7_CPU2_L1_PDN;
		bit_cpu = LITTLE_CPU2;
		break;
	case 3:
		reg_pwr_con = SPM_CA7_CPU3_PWR_CON;
		reg_l1_pdn = SPM_CA7_CPU3_L1_PDN;
		bit_cpu = LITTLE_CPU3;
		break;
	default:
		/* should never come to here */
		return;
	}

	/* enable register control */
	mmio_write_32(SPM_POWERON_CONFIG_SET,
			(SPM_PROJECT_CODE << 16) | (1U << 0));

	mmio_setbits_32(reg_pwr_con, PWR_ISO);
	mmio_setbits_32(reg_pwr_con, SRAM_CKISO);
	mmio_clrbits_32(reg_pwr_con, SRAM_ISOINT_B);
	mmio_setbits_32(reg_l1_pdn, L1_PDN);

	while (!(mmio_read_32(reg_l1_pdn) & L1_PDN_ACK))
		continue;

	mmio_clrbits_32(reg_pwr_con, PWR_RST_B);
	mmio_setbits_32(reg_pwr_con, PWR_CLK_DIS);
	mmio_clrbits_32(reg_pwr_con, PWR_ON);
	mmio_clrbits_32(reg_pwr_con, PWR_ON_2ND);

	while ((mmio_read_32(SPM_PWR_STATUS) & bit_cpu) ||
	       (mmio_read_32(SPM_PWR_STATUS_2ND) & bit_cpu))
		continue;
}

void mtcmos_little_cpu_off(void)
{
	/* turn off little cpu 1 - 3 */
	mtcmos_ctrl_little_off(1);
	mtcmos_ctrl_little_off(2);
	mtcmos_ctrl_little_off(3);
}

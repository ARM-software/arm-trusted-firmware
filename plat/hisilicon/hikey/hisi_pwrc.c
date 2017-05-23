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
 */

#include <debug.h>
#include <mmio.h>
#include <hisi_ipc.h>
#include <hisi_pwrc.h>
#include <hisi_sram_map.h>
#include <hi6220_regs_acpu.h>
#include <hi6220_regs_ao.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <platform_def.h>

#define CLUSTER_CORE_COUNT		(4)
#define CLUSTER_CORE_MASK		((1 << CLUSTER_CORE_COUNT) - 1)

void hisi_pwrc_set_core_bx_addr(unsigned int core, unsigned int cluster,
				uintptr_t entry_point)
{
	uintptr_t *core_entry = (uintptr_t *)PWRCTRL_ACPU_ASM_D_ARM_PARA_AD;
	unsigned int i;

	if (!core_entry) {
		INFO("%s: core entry point is null!\n", __func__);
		return;
	}

	i = cluster * CLUSTER_CORE_COUNT + core;
	mmio_write_64((uintptr_t)(core_entry + i), entry_point);
}

void hisi_pwrc_set_cluster_wfi(unsigned int cluster)
{
	unsigned int reg = 0;

	if (cluster == 0) {
		reg = mmio_read_32(ACPU_SC_SNOOP_PWD);
		reg |= PD_DETECT_START0;
		mmio_write_32(ACPU_SC_SNOOP_PWD, reg);
	} else if (cluster == 1) {
		reg = mmio_read_32(ACPU_SC_SNOOP_PWD);
		reg |= PD_DETECT_START1;
		mmio_write_32(ACPU_SC_SNOOP_PWD, reg);
	}
}

int hisi_pwrc_setup(void)
{
	unsigned int reg, sec_entrypoint;
	extern char pm_asm_code[], pm_asm_code_end[];
	extern char v7_asm[], v7_asm_end[];

	sec_entrypoint = PWRCTRL_ACPU_ASM_CODE_BASE;
	mmio_write_32(ACPU_SC_CPUx_RVBARADDR(0), sec_entrypoint >> 2);
	mmio_write_32(ACPU_SC_CPUx_RVBARADDR(1), sec_entrypoint >> 2);
	mmio_write_32(ACPU_SC_CPUx_RVBARADDR(2), sec_entrypoint >> 2);
	mmio_write_32(ACPU_SC_CPUx_RVBARADDR(3), sec_entrypoint >> 2);
	mmio_write_32(ACPU_SC_CPUx_RVBARADDR(4), sec_entrypoint >> 2);
	mmio_write_32(ACPU_SC_CPUx_RVBARADDR(5), sec_entrypoint >> 2);
	mmio_write_32(ACPU_SC_CPUx_RVBARADDR(6), sec_entrypoint >> 2);
	mmio_write_32(ACPU_SC_CPUx_RVBARADDR(7), sec_entrypoint >> 2);

	memset((void *)PWRCTRL_ACPU_ASM_SPACE_ADDR, 0, 0x400);
	memcpy((void *)PWRCTRL_ACPU_ASM_SPACE_ADDR, (void *)v7_asm,
	       v7_asm_end - v7_asm);

	memcpy((void *)PWRCTRL_ACPU_ASM_CODE_BASE, (void *)pm_asm_code,
	       pm_asm_code_end - pm_asm_code);

	reg = mmio_read_32(AO_SC_SYS_CTRL1);
	reg |= AO_SC_SYS_CTRL1_REMAP_SRAM_AARM |
	       AO_SC_SYS_CTRL1_REMAP_SRAM_AARM_MSK;
	mmio_write_32(AO_SC_SYS_CTRL1, reg);

	return 0;
}

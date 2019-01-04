/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <platform_def.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include <hi6220_regs_acpu.h>
#include <hi6220_regs_ao.h>
#include <hisi_ipc.h>
#include <hisi_pwrc.h>
#include <hisi_sram_map.h>

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

void hisi_pwrc_enable_debug(unsigned int core, unsigned int cluster)
{
	unsigned int val, enable;

	enable = 1U << (core + PDBGUP_CLUSTER1_SHIFT * cluster);

	/* Enable debug module */
	val = mmio_read_32(ACPU_SC_PDBGUP_MBIST);
	mmio_write_32(ACPU_SC_PDBGUP_MBIST, val | enable);
	do {
		/* RAW barrier */
		val = mmio_read_32(ACPU_SC_PDBGUP_MBIST);
	} while (!(val & enable));
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
	/* Remap SRAM address for ACPU */
	reg |= AO_SC_SYS_CTRL1_REMAP_SRAM_AARM |
	       AO_SC_SYS_CTRL1_REMAP_SRAM_AARM_MSK;

	/* Enable reset signal for watchdog */
	reg |= AO_SC_SYS_CTRL1_AARM_WD_RST_CFG |
	       AO_SC_SYS_CTRL1_AARM_WD_RST_CFG_MSK;
	mmio_write_32(AO_SC_SYS_CTRL1, reg);

	return 0;
}

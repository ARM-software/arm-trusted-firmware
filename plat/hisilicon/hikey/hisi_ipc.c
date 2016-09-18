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

#include <arch_helpers.h>
#include <hisi_ipc.h>
#include <hisi_sram_map.h>
#include <mmio.h>
#include <platform_def.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define BIT(x)	(0x1 << (x))

static int  ipc_init = 0;

static unsigned int cpu_ipc_num[PLATFORM_CLUSTER_COUNT][PLATFORM_CORE_COUNT_PER_CLUSTER] = {
	{
		HISI_IPC_MCU_INT_SRC_ACPU0_PD,
		HISI_IPC_MCU_INT_SRC_ACPU1_PD,
		HISI_IPC_MCU_INT_SRC_ACPU2_PD,
		HISI_IPC_MCU_INT_SRC_ACPU3_PD,
	},
	{
		HISI_IPC_MCU_INT_SRC_ACPU4_PD,
		HISI_IPC_MCU_INT_SRC_ACPU5_PD,
		HISI_IPC_MCU_INT_SRC_ACPU6_PD,
		HISI_IPC_MCU_INT_SRC_ACPU7_PD,
	}
};

int hisi_cpus_pd_in_cluster_besides_curr(unsigned int cpu,
					 unsigned int cluster)
{
	unsigned int val = 0, cpu_val = 0;
	int i;

	val = mmio_read_32(ACPU_CORE_POWERDOWN_FLAGS_ADDR);
	val = val >> (cluster * 16);

	for (i = 0; i < PLATFORM_CORE_COUNT_PER_CLUSTER; i++) {

		if (cpu == i)
			continue;

		cpu_val = (val >> (i * 4)) & 0xF;
		if (cpu_val == 0x8)
			return 0;
        }

	return 1;
}

int hisi_cpus_powered_off_besides_curr(unsigned int cpu)
{
	unsigned int val;

	val = mmio_read_32(ACPU_CORE_POWERDOWN_FLAGS_ADDR);
	return (val == (0x8 << (cpu * 4)));
}

static void hisi_ipc_send(unsigned int ipc_num)
{
	if (!ipc_init) {
		printf("error ipc base is null!!!\n");
		return;
	}

	mmio_write_32(HISI_IPC_CPU_RAW_INT_ADDR, 1 << ipc_num);
}

void hisi_ipc_spin_lock(unsigned int signal)
{
	unsigned int hs_ctrl;

	if (signal >= HISI_IPC_INT_SRC_NUM)
		return;

	do {
		hs_ctrl = mmio_read_32(HISI_IPC_ACPU_CTRL(signal));
	} while (hs_ctrl);
}

void hisi_ipc_spin_unlock(unsigned int signal)
{
	if (signal >= HISI_IPC_INT_SRC_NUM)
		return;

	mmio_write_32(HISI_IPC_ACPU_CTRL(signal), 0);
}

void hisi_ipc_cpu_on_off(unsigned int cpu, unsigned int cluster,
			 unsigned int mode)
{
	unsigned int val = 0;
	unsigned int offset;

	if (mode == HISI_IPC_PM_ON)
		offset = cluster * 16 + cpu * 4;
	else
		offset = cluster * 16 + cpu * 4 + 1;

	hisi_ipc_spin_lock(HISI_IPC_SEM_CPUIDLE);
	val = mmio_read_32(ACPU_CORE_POWERDOWN_FLAGS_ADDR);
	val |= (0x01 << offset);
	mmio_write_32(ACPU_CORE_POWERDOWN_FLAGS_ADDR, val);
	isb();
	dsb();
	hisi_ipc_spin_unlock(HISI_IPC_SEM_CPUIDLE);

	hisi_ipc_send(cpu_ipc_num[cluster][cpu]);
}

void hisi_ipc_cpu_on(unsigned int cpu, unsigned int cluster)
{
	hisi_ipc_cpu_on_off(cpu, cluster, HISI_IPC_PM_ON);
}

void hisi_ipc_cpu_off(unsigned int cpu, unsigned int cluster)
{
	hisi_ipc_cpu_on_off(cpu, cluster, HISI_IPC_PM_OFF);
}

void hisi_ipc_cluster_on_off(unsigned int cpu, unsigned int cluster,
			     unsigned int mode)
{
	unsigned int val = 0;
	unsigned int offset;

	if (mode == HISI_IPC_PM_ON)
		offset = cluster * 4;
	else
		offset = cluster * 4 + 1;

	hisi_ipc_spin_lock(HISI_IPC_SEM_CPUIDLE);
	val = mmio_read_32(ACPU_CLUSTER_POWERDOWN_FLAGS_ADDR);
	val |= (0x01 << offset);
	mmio_write_32(ACPU_CLUSTER_POWERDOWN_FLAGS_ADDR, val);
	isb();
	dsb();
	hisi_ipc_spin_unlock(HISI_IPC_SEM_CPUIDLE);

	hisi_ipc_send(cpu_ipc_num[cluster][cpu]);
}

void hisi_ipc_cluster_on(unsigned int cpu, unsigned int cluster)
{
	hisi_ipc_cluster_on_off(cpu, cluster, HISI_IPC_PM_ON);
}

void hisi_ipc_cluster_off(unsigned int cpu, unsigned int cluster)
{
	hisi_ipc_cluster_on_off(cpu, cluster, HISI_IPC_PM_OFF);
}

void hisi_ipc_cpu_suspend(unsigned int cpu, unsigned int cluster)
{
	unsigned int val = 0;
	unsigned int offset;

	offset = cluster * 16 + cpu * 4 + 2;

	hisi_ipc_spin_lock(HISI_IPC_SEM_CPUIDLE);
	val = mmio_read_32(ACPU_CORE_POWERDOWN_FLAGS_ADDR);
	val |= (0x01 << offset);
	mmio_write_32(ACPU_CORE_POWERDOWN_FLAGS_ADDR, val);
	hisi_ipc_spin_unlock(HISI_IPC_SEM_CPUIDLE);

	hisi_ipc_send(cpu_ipc_num[cluster][cpu]);
}

void hisi_ipc_cluster_suspend(unsigned int cpu, unsigned int cluster)
{
	unsigned int val;
	unsigned int offset;

	offset = cluster * 4 + 1;

	hisi_ipc_spin_lock(HISI_IPC_SEM_CPUIDLE);
	if (hisi_cpus_pd_in_cluster_besides_curr(cpu, cluster)) {
		val = mmio_read_32(ACPU_CLUSTER_POWERDOWN_FLAGS_ADDR);
		val |= (0x01 << offset);
		mmio_write_32(ACPU_CLUSTER_POWERDOWN_FLAGS_ADDR, val);
	}
	hisi_ipc_spin_unlock(HISI_IPC_SEM_CPUIDLE);

	hisi_ipc_send(cpu_ipc_num[cluster][cpu]);
}

void hisi_ipc_psci_system_off(void)
{
	hisi_ipc_send(HISI_IPC_MCU_INT_SRC_ACPU_PD);
}

int hisi_ipc_init(void)
{
	ipc_init = 1;

	mmio_write_32(ACPU_CORE_POWERDOWN_FLAGS_ADDR, 0x8);
	mmio_write_32(ACPU_CLUSTER_POWERDOWN_FLAGS_ADDR, 0x8);
	return 0;
}

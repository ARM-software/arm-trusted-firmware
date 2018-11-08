/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HISI_IPC_H
#define HISI_IPC_H

#define HISI_IPC_CORE_ACPU		0x0

#define HISI_IPC_MCU_INT_SRC_ACPU0_PD	10
#define HISI_IPC_MCU_INT_SRC_ACPU1_PD	11
#define HISI_IPC_MCU_INT_SRC_ACPU2_PD	12
#define HISI_IPC_MCU_INT_SRC_ACPU3_PD	13
#define HISI_IPC_MCU_INT_SRC_ACPU_PD	16
#define HISI_IPC_MCU_INT_SRC_ACPU4_PD	26
#define HISI_IPC_MCU_INT_SRC_ACPU5_PD	27
#define HISI_IPC_MCU_INT_SRC_ACPU6_PD	28
#define HISI_IPC_MCU_INT_SRC_ACPU7_PD	29

#define HISI_IPC_SEM_CPUIDLE		27
#define HISI_IPC_INT_SRC_NUM		32

#define HISI_IPC_PM_ON			0
#define HISI_IPC_PM_OFF			1

#define HISI_IPC_OK			(0)
#define HISI_IPC_ERROR			(-1)

#define HISI_IPC_BASE_ADDR		(0xF7510000)
#define HISI_IPC_CPU_RAW_INT_ADDR	(0xF7510420)
#define HISI_IPC_ACPU_CTRL(i)		(0xF7510800 + (i << 3))

void hisi_ipc_spin_lock(unsigned int signal);
void hisi_ipc_spin_unlock(unsigned int signal);
void hisi_ipc_cpu_on(unsigned int cpu, unsigned int cluster);
void hisi_ipc_cpu_off(unsigned int cpu, unsigned int cluster);
void hisi_ipc_cpu_suspend(unsigned int cpu, unsigned int cluster);
void hisi_ipc_cluster_on(unsigned int cpu, unsigned int cluster);
void hisi_ipc_cluster_off(unsigned int cpu, unsigned int cluster);
void hisi_ipc_cluster_suspend(unsigned int cpu, unsigned int cluster);
void hisi_ipc_psci_system_off(void);
int hisi_ipc_init(void);

#endif /* HISI_IPC_H */

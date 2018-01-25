/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CCI_400_H__
#define __CCI_400_H__

/**************************************************************
 * THIS DRIVER IS DEPRECATED. Please use the driver in cci.h
 **************************************************************/
#if PLAT_COMPAT >= 201512
#error " The CCI-400 specific driver is deprecated."
#endif


/* Slave interface offsets from PERIPHBASE */
#define SLAVE_IFACE4_OFFSET		0x5000
#define SLAVE_IFACE3_OFFSET		0x4000
#define SLAVE_IFACE2_OFFSET		0x3000
#define SLAVE_IFACE1_OFFSET		0x2000
#define SLAVE_IFACE0_OFFSET		0x1000
#define SLAVE_IFACE_OFFSET(index)	SLAVE_IFACE0_OFFSET +	\
					(0x1000 * (index))

/* Control and ID register offsets */
#define CTRL_OVERRIDE_REG		0x0
#define SPEC_CTRL_REG			0x4
#define SECURE_ACCESS_REG		0x8
#define STATUS_REG			0xc
#define IMPRECISE_ERR_REG		0x10
#define PERFMON_CTRL_REG		0x100

/* Slave interface register offsets */
#define SNOOP_CTRL_REG			0x0
#define SH_OVERRIDE_REG			0x4
#define READ_CHNL_QOS_VAL_OVERRIDE_REG	0x100
#define WRITE_CHNL_QOS_VAL_OVERRIDE_REG	0x104
#define QOS_CTRL_REG			0x10c
#define MAX_OT_REG			0x110
#define TARGET_LATENCY_REG		0x130
#define LATENCY_REGULATION_REG		0x134
#define QOS_RANGE_REG			0x138

/* Snoop Control register bit definitions */
#define DVM_EN_BIT			(1 << 1)
#define SNOOP_EN_BIT			(1 << 0)

/* Status register bit definitions */
#define CHANGE_PENDING_BIT		(1 << 0)

#ifndef __ASSEMBLY__

#include <stdint.h>

/* Function declarations */

/*
 * The CCI-400 driver must be initialized with the base address of the
 * CCI-400 device in the platform memory map, and the cluster indices for
 * the CCI-400 slave interfaces 3 and 4 respectively. These are the fully
 * coherent ACE slave interfaces of CCI-400.
 * The cluster indices must either be 0 or 1, corresponding to the level 1
 * affinity instance of the mpidr representing the cluster. A negative cluster
 * index indicates that no cluster is present on that slave interface.
 */
void cci_init(uintptr_t cci_base,
		int slave_iface3_cluster_ix,
		int slave_iface4_cluster_ix) __deprecated;

void cci_enable_cluster_coherency(unsigned long mpidr) __deprecated;
void cci_disable_cluster_coherency(unsigned long mpidr) __deprecated;

#endif /* __ASSEMBLY__ */
#endif /* __CCI_400_H__ */

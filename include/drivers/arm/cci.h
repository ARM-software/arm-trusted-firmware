/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CCI_H__
#define __CCI_H__

/* Slave interface offsets from PERIPHBASE */
#define SLAVE_IFACE6_OFFSET		0x7000
#define SLAVE_IFACE5_OFFSET		0x6000
#define SLAVE_IFACE4_OFFSET		0x5000
#define SLAVE_IFACE3_OFFSET		0x4000
#define SLAVE_IFACE2_OFFSET		0x3000
#define SLAVE_IFACE1_OFFSET		0x2000
#define SLAVE_IFACE0_OFFSET		0x1000
#define SLAVE_IFACE_OFFSET(index)	(SLAVE_IFACE0_OFFSET +	\
					(0x1000 * (index)))

/* Slave interface event and count register offsets from PERIPHBASE */
#define EVENT_SELECT7_OFFSET		0x80000
#define EVENT_SELECT6_OFFSET		0x70000
#define EVENT_SELECT5_OFFSET		0x60000
#define EVENT_SELECT4_OFFSET		0x50000
#define EVENT_SELECT3_OFFSET		0x40000
#define EVENT_SELECT2_OFFSET		0x30000
#define EVENT_SELECT1_OFFSET		0x20000
#define EVENT_SELECT0_OFFSET		0x10000
#define EVENT_OFFSET(index)		(EVENT_SELECT0_OFFSET +	\
					(0x10000 * (index)))

/* Control and ID register offsets */
#define CTRL_OVERRIDE_REG		0x0
#define SECURE_ACCESS_REG		0x8
#define STATUS_REG			0xc
#define IMPRECISE_ERR_REG		0x10
#define PERFMON_CTRL_REG		0x100
#define IFACE_MON_CTRL_REG		0x104

/* Component and peripheral ID registers */
#define PERIPHERAL_ID0			0xFE0
#define PERIPHERAL_ID1			0xFE4
#define PERIPHERAL_ID2			0xFE8
#define PERIPHERAL_ID3			0xFEC
#define PERIPHERAL_ID4			0xFD0
#define PERIPHERAL_ID5			0xFD4
#define PERIPHERAL_ID6			0xFD8
#define PERIPHERAL_ID7			0xFDC

#define COMPONENT_ID0			0xFF0
#define COMPONENT_ID1			0xFF4
#define COMPONENT_ID2			0xFF8
#define COMPONENT_ID3			0xFFC
#define COMPONENT_ID4			0x1000
#define COMPONENT_ID5			0x1004
#define COMPONENT_ID6			0x1008
#define COMPONENT_ID7			0x100C

/* Slave interface register offsets */
#define SNOOP_CTRL_REG			0x0
#define SH_OVERRIDE_REG			0x4
#define READ_CHNL_QOS_VAL_OVERRIDE_REG	0x100
#define WRITE_CHNL_QOS_VAL_OVERRIDE_REG	0x104
#define MAX_OT_REG			0x110

/* Snoop Control register bit definitions */
#define DVM_EN_BIT			(1 << 1)
#define SNOOP_EN_BIT			(1 << 0)
#define SUPPORT_SNOOPS			(1 << 30)
#define SUPPORT_DVM			(1 << 31)

/* Status register bit definitions */
#define CHANGE_PENDING_BIT		(1 << 0)

/* Event and count register offsets */
#define EVENT_SELECT_REG		0x0
#define EVENT_COUNT_REG			0x4
#define COUNT_CNTRL_REG			0x8
#define COUNT_OVERFLOW_REG		0xC

/* Slave interface monitor registers */
#define INT_MON_REG_SI0			0x90000
#define INT_MON_REG_SI1			0x90004
#define INT_MON_REG_SI2			0x90008
#define INT_MON_REG_SI3			0x9000C
#define INT_MON_REG_SI4			0x90010
#define INT_MON_REG_SI5			0x90014
#define INT_MON_REG_SI6			0x90018

/* Master interface monitor registers */
#define INT_MON_REG_MI0			0x90100
#define INT_MON_REG_MI1			0x90104
#define INT_MON_REG_MI2			0x90108
#define INT_MON_REG_MI3			0x9010c
#define INT_MON_REG_MI4			0x90110
#define INT_MON_REG_MI5			0x90114

#define SLAVE_IF_UNUSED			-1

#ifndef __ASSEMBLY__

#include <stdint.h>

/* Function declarations */

/*
 * The ARM CCI driver needs the following:
 * 1. Base address of the CCI product
 * 2. An array  of map between AMBA 4 master ids and ACE/ACE lite slave
 *    interfaces.
 * 3. Size of the array.
 *
 * SLAVE_IF_UNUSED should be used in the map to represent no AMBA 4 master exists
 * for that interface.
 */
void cci_init(uintptr_t base, const int *map, unsigned int num_cci_masters);

void cci_enable_snoop_dvm_reqs(unsigned int master_id);
void cci_disable_snoop_dvm_reqs(unsigned int master_id);

#endif /* __ASSEMBLY__ */
#endif /* __CCI_H__ */

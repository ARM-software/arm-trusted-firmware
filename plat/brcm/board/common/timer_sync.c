/*
 * Copyright (c) 2015 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include <platform_def.h>
#include <timer_sync.h>

/*******************************************************************************
 * Defines related to time sync and satelite timers
 ******************************************************************************/
#define TIME_SYNC_WR_ENA	((uint32_t)0xACCE55 << 8)
#define IHOST_STA_TMR_CTRL	0x1800
#define IHOST_SAT_TMR_INC_L	0x1814
#define IHOST_SAT_TMR_INC_H	0x1818

#define SAT_TMR_CYCLE_DELAY	2
#define SAT_TMR_32BIT_WRAP_VAL	(BIT_64(32) - SAT_TMR_CYCLE_DELAY)

void ihost_enable_satellite_timer(unsigned int cluster_id)
{
	uintptr_t ihost_base;
	uint32_t time_lx, time_h;
	uintptr_t ihost_enable;

	VERBOSE("Program iHost%u satellite timer\n", cluster_id);
	ihost_base = IHOST0_BASE + cluster_id * IHOST_ADDR_SPACE;

	/* this read starts the satellite timer counting from 0 */
	ihost_enable = CENTRAL_TIMER_GET_IHOST_ENA_BASE + cluster_id * 4;
	time_lx = mmio_read_32(ihost_enable);

	/*
	 * Increment the satellite timer by the central timer plus 2
	 * to accommodate for a 1 cycle delay through NOC
	 * plus counter starting from 0.
	 */
	mmio_write_32(ihost_base + IHOST_SAT_TMR_INC_L,
		      time_lx + SAT_TMR_CYCLE_DELAY);

	/*
	 * Read the latched upper data, if lx will wrap by adding 2 to it
	 * we need to handle the wrap
	 */
	time_h = mmio_read_32(CENTRAL_TIMER_GET_H);
	if (time_lx >= SAT_TMR_32BIT_WRAP_VAL)
		mmio_write_32(ihost_base + IHOST_SAT_TMR_INC_H, time_h + 1);
	else
		mmio_write_32(ihost_base + IHOST_SAT_TMR_INC_H, time_h);
}

void brcm_timer_sync_init(void)
{
	unsigned int cluster_id;

	/* Get the Time Sync module out of reset */
	mmio_setbits_32(CDRU_MISC_RESET_CONTROL,
			BIT(CDRU_MISC_RESET_CONTROL_TS_RESET_N));

	/* Deassert the Central Timer TIMER_EN signal for all module */
	mmio_write_32(CENTRAL_TIMER_SAT_TMR_ENA, TIME_SYNC_WR_ENA);

	/* enables/programs iHost0 satellite timer*/
	cluster_id = MPIDR_AFFLVL1_VAL(read_mpidr());
	ihost_enable_satellite_timer(cluster_id);
}

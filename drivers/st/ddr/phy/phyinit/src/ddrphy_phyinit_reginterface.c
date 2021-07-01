/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * This file provides a group of functions that are used to track PHY register
 * writes by intercepting io_write16 function calls.  Once the registers are
 * tracked, their value can be saved at a given time spot, and restored later
 * as required. This implementation is useful to capture any PHY register
 * programing in any function during PHY initialization.
 */

#include <stdint.h>

#include <common/debug.h>

#include <ddrphy_phyinit.h>

#include <lib/mmio.h>

#include <platform_def.h>

/*
 * MAX_NUM_RET_REGS default Max number of retention registers.
 *
 * This define is only used by the PhyInit Register interface to define the max
 * amount of registered that can be saved. The user may increase this variable
 * as desired if a larger number of registers need to be restored.
 */
#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
#define MAX_NUM_RET_REGS	129
#else /* STM32MP_LPDDR4_TYPE */
#define MAX_NUM_RET_REGS	283
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */

/*
 * Array of Address/value pairs used to store register values for the purpose
 * of retention restore.
 */
#define RETREG_AREA	(MAX_NUM_RET_REGS + 1) * sizeof(struct reg_addr_val)
#define RETREG_BASE	RETRAM_BASE + RETRAM_SIZE - RETREG_AREA

static int *retregsize = (int *)(RETREG_BASE);
static struct reg_addr_val *retreglist = (struct reg_addr_val *)(RETREG_BASE + sizeof(int));

static int numregsaved; /* Current Number of registers saved. */
static int tracken = 1; /* Enabled tracking of registers */

/*
 * Tags a register if tracking is enabled in the register
 * interface.
 *
 * During PhyInit registers writes, keeps track of address
 * for the purpose of restoring the PHY register state during PHY
 * retention exit process. Tracking can be turned on/off via the
 * ddrphy_phyinit_reginterface STARTTRACK, STOPTRACK instructions. By
 * default tracking is always turned on.
 *
 * \return 0 on success.
 */
int ddrphy_phyinit_trackreg(uint32_t adr)
{
	int regindx = 0;

	/* Return if tracking is disabled */
	if (tracken == 0) {
		return 0;
	}

	/* Search register address within the array */
	for (regindx = 0; regindx < numregsaved; regindx++) {
		if (retreglist[regindx].address == adr) {
			/* Register found */
			return 0;
		}
	}

	/* Register not found, so add it */
	if (numregsaved > MAX_NUM_RET_REGS) {
		ERROR("numregsaved > MAX_NUM_RET_REGS\n");
		VERBOSE("[ddrphy_phyinit_reginterface:%s]\n", __func__);
		VERBOSE("Max Number of Restore Registers reached: %d.\n", numregsaved);
		VERBOSE("Please recompile PhyInit with larger MAX_NUM_RET_REG value.\n");
		return -1;
	}

	retreglist[regindx].address = adr;
	numregsaved++;

	return 0;
}

/*
 * Register interface function used to track, save and restore retention registers.
 *
 * ### Usage
 * Register tracking is enabled by calling:
 *
 *  \code
 *  ddrphy_phyinit_reginterface(STARTTRACK,0,0);
 *  \endcode
 *
 * from this point on any call to mmio_write_16() in
 * return will be capture by the register interface via a call to
 * ddrphy_phyinit_trackreg(). Tracking is disabled by calling:
 *
 *  \code
 *  ddrphy_phyinit_reginterface(STOPTRACK,0,0);
 *  \endcode
 *
 * On calling this function, register write via mmio_write_16 are no longer tracked until a
 * STARTTRACK call is made. Once all the register write are complete, SAVEREGS
 * command can be issue to save register values into the internal data array of
 * the register interface. Upon retention exit RESTOREREGS are command can be
 * used to issue register write commands to the PHY based on values stored in
 * the array.
 *  \code
 *   ddrphy_phyinit_reginterface(SAVEREGS,0,0);
 *   ddrphy_phyinit_reginterface(RESTOREREGS,0,0);
 *  \endcode
 * \return 0 on success.
 */
int ddrphy_phyinit_reginterface(enum reginstr myreginstr, uint32_t adr, uint16_t dat)
{
	if (myreginstr == SAVEREGS) {
		int regindx;

		/*
		 * Go through all the tracked registers, issue a register read and place
		 * the result in the data structure for future recovery.
		 */
		for (regindx = 0; regindx < numregsaved; regindx++) {
			uint16_t data;

			data = mmio_read_16((uintptr_t)(DDRPHYC_BASE +
							(4U * retreglist[regindx].address)));
			retreglist[regindx].value = data;
		}

		*retregsize = numregsaved;

		return 0;
	} else if (myreginstr == RESTOREREGS) {
		int regindx;

		/*
		 * Write PHY registers based on Address, Data value pairs stores in
		 * retreglist.
		 */
		for (regindx = 0; regindx < *retregsize; regindx++) {
			mmio_write_16((uintptr_t)
				      (DDRPHYC_BASE + (4U * retreglist[regindx].address)),
				      retreglist[regindx].value);
		}

		return 0;
	} else if (myreginstr == STARTTRACK) {
		/* Enable tracking */
		tracken = 1;
		return 0;
	} else if (myreginstr == STOPTRACK) {
		/* Disable tracking */
		tracken = 0;
		return 0;
	} else {
		return -1;
	}
}

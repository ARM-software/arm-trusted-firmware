/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <common/debug.h>
#include <mmio.h>

void erratum_a050426(void)
{
	uint32_t i, val3, val4;

	/*
	 * Part of this Errata is implemented in RCW and SCRATCHRW5
	 * register is updated to hold Errata number.
	 * Validate whether RCW has already included required changes
	 */
	if (mmio_read_32(0x01e00210) != 0x00050426) {
		ERROR("%s: Invalid RCW : ERR050426 not implemented\n", __func__);
	}

	/* Enable BIST to access Internal memory locations */
	val3 = mmio_read_32(0x700117E60);
	mmio_write_32(0x700117E60, (val3 | 0x80000001));
	val4 = mmio_read_32(0x700117E90);
	mmio_write_32(0x700117E90, (val4 & 0xFFDFFFFF));

	/* wriop Internal Memory.*/
	for (i = 0U; i < 4U; i++) {
		mmio_write_32(0x706312000 + (i * 4), 0x55555555);
		mmio_write_32(0x706312400 + (i * 4), 0x55555555);
		mmio_write_32(0x706312800 + (i * 4), 0x55555555);
		mmio_write_32(0x706314000 + (i * 4), 0x55555555);
		mmio_write_32(0x706314400 + (i * 4), 0x55555555);
		mmio_write_32(0x706314800 + (i * 4), 0x55555555);
		mmio_write_32(0x706314c00 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 3U; i++) {
		mmio_write_32(0x706316000 + (i * 4), 0x55555555);
		mmio_write_32(0x706320000 + (i * 4), 0x55555555);
		mmio_write_32(0x706320400 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 2U; i++) {
		mmio_write_32(0x70640a000 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 3U; i++) {
		mmio_write_32(0x706518000 + (i * 4), 0x55555555);
		mmio_write_32(0x706519000 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 4U; i++) {
		mmio_write_32(0x706522000 + (i * 4), 0x55555555);
		mmio_write_32(0x706522800 + (i * 4), 0x55555555);
		mmio_write_32(0x706523000 + (i * 4), 0x55555555);
		mmio_write_32(0x706523800 + (i * 4), 0x55555555);
		mmio_write_32(0x706524000 + (i * 4), 0x55555555);
		mmio_write_32(0x706524800 + (i * 4), 0x55555555);
		mmio_write_32(0x706608000 + (i * 4), 0x55555555);
		mmio_write_32(0x706608800 + (i * 4), 0x55555555);
		mmio_write_32(0x706609000 + (i * 4), 0x55555555);
		mmio_write_32(0x706609800 + (i * 4), 0x55555555);
		mmio_write_32(0x70660a000 + (i * 4), 0x55555555);
		mmio_write_32(0x70660a800 + (i * 4), 0x55555555);
		mmio_write_32(0x70660b000 + (i * 4), 0x55555555);
		mmio_write_32(0x70660b800 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 3U; i++) {
		mmio_write_32(0x70660c000 + (i * 4), 0x55555555);
		mmio_write_32(0x70660c800 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 2U; i++) {
		mmio_write_32(0x706718000 + (i * 4), 0x55555555);
		mmio_write_32(0x706718800 + (i * 4), 0x55555555);
	}
	mmio_write_32(0x706b0a000, 0x55555555);

	for (i = 0U; i < 4U; i++) {
		mmio_write_32(0x706b0e000 + (i * 4), 0x55555555);
		mmio_write_32(0x706b0e800 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 2U; i++) {
		mmio_write_32(0x706b10000 + (i * 4), 0x55555555);
		mmio_write_32(0x706b10400 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 4U; i++) {
		mmio_write_32(0x706b14000 + (i * 4), 0x55555555);
		mmio_write_32(0x706b14800 + (i * 4), 0x55555555);
		mmio_write_32(0x706b15000 + (i * 4), 0x55555555);
		mmio_write_32(0x706b15800 + (i * 4), 0x55555555);
	}
	mmio_write_32(0x706e12000, 0x55555555);

	for (i = 0U; i < 4U; i++) {
		mmio_write_32(0x706e14000 + (i * 4), 0x55555555);
		mmio_write_32(0x706e14800 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 2U; i++) {
		mmio_write_32(0x706e16000 + (i * 4), 0x55555555);
		mmio_write_32(0x706e16400 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 3U; i++) {
		mmio_write_32(0x706e1a000 + (i * 4), 0x55555555);
		mmio_write_32(0x706e1a800 + (i * 4), 0x55555555);
		mmio_write_32(0x706e1b000 + (i * 4), 0x55555555);
		mmio_write_32(0x706e1b800 + (i * 4), 0x55555555);
		mmio_write_32(0x706e1c000 + (i * 4), 0x55555555);
		mmio_write_32(0x706e1c800 + (i * 4), 0x55555555);
		mmio_write_32(0x706e1e000 + (i * 4), 0x55555555);
		mmio_write_32(0x706e1e800 + (i * 4), 0x55555555);
		mmio_write_32(0x706e1f000 + (i * 4), 0x55555555);
		mmio_write_32(0x706e1f800 + (i * 4), 0x55555555);
		mmio_write_32(0x706e20000 + (i * 4), 0x55555555);
		mmio_write_32(0x706e20800 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 4U; i++) {
		mmio_write_32(0x707108000 + (i * 4), 0x55555555);
		mmio_write_32(0x707109000 + (i * 4), 0x55555555);
		mmio_write_32(0x70710a000 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 2U; i++) {
		mmio_write_32(0x70711c000 + (i * 4), 0x55555555);
		mmio_write_32(0x70711c800 + (i * 4), 0x55555555);
		mmio_write_32(0x70711d000 + (i * 4), 0x55555555);
		mmio_write_32(0x70711d800 + (i * 4), 0x55555555);
		mmio_write_32(0x70711e000 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 4U; i++) {
		mmio_write_32(0x707120000 + (i * 4), 0x55555555);
		mmio_write_32(0x707121000 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 3U; i++) {
		mmio_write_32(0x707122000 + (i * 4), 0x55555555);
		mmio_write_32(0x70725a000 + (i * 4), 0x55555555);
		mmio_write_32(0x70725b000 + (i * 4), 0x55555555);
		mmio_write_32(0x70725c000 + (i * 4), 0x55555555);
		mmio_write_32(0x70725e000 + (i * 4), 0x55555555);
		mmio_write_32(0x70725e400 + (i * 4), 0x55555555);
		mmio_write_32(0x70725e800 + (i * 4), 0x55555555);
		mmio_write_32(0x70725ec00 + (i * 4), 0x55555555);
		mmio_write_32(0x70725f000 + (i * 4), 0x55555555);
		mmio_write_32(0x70725f400 + (i * 4), 0x55555555);
		mmio_write_32(0x707340000 + (i * 4), 0x55555555);
		mmio_write_32(0x707346000 + (i * 4), 0x55555555);
		mmio_write_32(0x707484000 + (i * 4), 0x55555555);
		mmio_write_32(0x70748a000 + (i * 4), 0x55555555);
		mmio_write_32(0x70748b000 + (i * 4), 0x55555555);
		mmio_write_32(0x70748c000 + (i * 4), 0x55555555);
		mmio_write_32(0x70748d000 + (i * 4), 0x55555555);
	}

	/* EDMA Internal Memory.*/
	for (i = 0U; i < 5U; i++) {
		mmio_write_32(0x70a208000 + (i * 4), 0x55555555);
		mmio_write_32(0x70a208800 + (i * 4), 0x55555555);
		mmio_write_32(0x70a209000 + (i * 4), 0x55555555);
		mmio_write_32(0x70a209800 + (i * 4), 0x55555555);
	}

	/* QDMA Internal Memory.*/
	for (i = 0U; i < 5U; i++) {
		mmio_write_32(0x70b008000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b00c000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b010000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b014000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b018000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b018400 + (i * 4), 0x55555555);
		mmio_write_32(0x70b01a000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b01a400 + (i * 4), 0x55555555);
		mmio_write_32(0x70b01c000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b01d000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b01e000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b01e800 + (i * 4), 0x55555555);
		mmio_write_32(0x70b01f000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b01f800 + (i * 4), 0x55555555);
		mmio_write_32(0x70b020000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b020400 + (i * 4), 0x55555555);
		mmio_write_32(0x70b020800 + (i * 4), 0x55555555);
		mmio_write_32(0x70b020c00 + (i * 4), 0x55555555);
		mmio_write_32(0x70b022000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b022400 + (i * 4), 0x55555555);
		mmio_write_32(0x70b024000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b024800 + (i * 4), 0x55555555);
		mmio_write_32(0x70b025000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b025800 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 4U; i++) {
		mmio_write_32(0x70b026000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b026200 + (i * 4), 0x55555555);
	}
	for (i = 0U; i < 5U; i++) {
		mmio_write_32(0x70b028000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b028800 + (i * 4), 0x55555555);
		mmio_write_32(0x70b029000 + (i * 4), 0x55555555);
		mmio_write_32(0x70b029800 + (i * 4), 0x55555555);
	}

	/* Disable BIST */
	mmio_write_32(0x700117E60, val3);
	mmio_write_32(0x700117E90, val4);
}

/*
 * Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2024, Mario Bălănică <mariobalanica02@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * The RPi has a single nonstandard PCI config region. It is broken into two
 * pieces, the root port config registers and a window to a single device's
 * config space which can move between devices. There isn't (yet) an
 * authoritative public document on this since the available BCM2711 reference
 * notes that there is a PCIe root port in the memory map but doesn't describe
 * it. Given that it's not ECAM compliant yet reasonably simple, it makes for
 * an excellent example of the PCI SMCCC interface.
 *
 * The PCI SMCCC interface is described in DEN0115 available from:
 * https://developer.arm.com/documentation/den0115/latest
 */

#include <assert.h>
#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/pmf/pmf.h>
#include <lib/runtime_instr.h>
#include <services/pci_svc.h>
#include <services/sdei.h>
#include <services/std_svc.h>
#include <smccc_helpers.h>

#include <lib/mmio.h>

#define PCIE_MISC_PCIE_STATUS	0x4068
#define PCIE_EXT_CFG_INDEX	0x9000
#define PCIE_EXT_CFG_DATA	0x8000
#define	PCIE_EXT_CFG_BDF_SHIFT	12

#define INVALID_PCI_ADDR	0xFFFFFFFF

static spinlock_t pci_lock;

static uint64_t pcie_rc_bases[] = { RPI_PCIE_RC_BASES };

static uint64_t pci_segment_lib_get_base(uint32_t address, uint32_t offset)
{
	uint64_t base;
	uint32_t seg, bus, dev, fun;

	seg = PCI_ADDR_SEG(address);

	if (seg >= ARRAY_SIZE(pcie_rc_bases)) {
		return INVALID_PCI_ADDR;
	}

	/* The root port is at the base of the PCIe register space */
	base = pcie_rc_bases[seg];

	bus = PCI_ADDR_BUS(address);
	dev = PCI_ADDR_DEV(address);
	fun = PCI_ADDR_FUN(address);

	/* There can only be the root port on bus 0 */
	if ((bus == 0U) && ((dev > 0U) || (fun > 0U))) {
		return INVALID_PCI_ADDR;
	}

	/* There can only be one device on bus 1 */
	if ((bus == 1U) && (dev > 0U)) {
		return INVALID_PCI_ADDR;
	}

	if (bus > 0) {
#if RPI_PCIE_ECAM_SERROR_QUIRK
		uint32_t status = mmio_read_32(base + PCIE_MISC_PCIE_STATUS);

		/* Assure link up before accessing downstream of root port */
		if ((status & 0x30) == 0U) {
			return INVALID_PCI_ADDR;
		}
#endif
		/*
		 * Device function is mapped at CFG_DATA, a 4 KB window
		 * movable by writing its B/D/F location to CFG_INDEX.
		 */
		mmio_write_32(base + PCIE_EXT_CFG_INDEX, address << PCIE_EXT_CFG_BDF_SHIFT);
		base += PCIE_EXT_CFG_DATA;
	}

	return base + (offset & PCI_OFFSET_MASK);
}

/**
 * pci_read_config() - Performs a config space read at addr
 * @addr: 32-bit, segment, BDF of requested function encoded per DEN0115
 * @off:  register offset of function described by @addr to read
 * @sz:	  size of read (8,16,32) bits.
 * @val:  returned zero extended value read from config space
 *
 * sz bits of PCI config space is read at addr:offset, and the value
 * is returned in val. Invalid segment/offset values return failure.
 * Reads to valid functions that don't exist return INVALID_PCI_ADDR
 * as is specified by PCI for requests that aren't completed by EPs.
 * The boilerplate in pci_svc.c tends to do basic segment, off
 * and sz validation. This routine should avoid duplicating those
 * checks.
 *
 * This function maps directly to the PCI_READ function in DEN0115
 * where detailed requirements may be found.
 *
 * Return: SMC_PCI_CALL_SUCCESS with val set
 *	   SMC_PCI_CALL_INVAL_PARAM, on parameter error
 */
uint32_t pci_read_config(uint32_t addr, uint32_t off, uint32_t sz, uint32_t *val)
{
	uint32_t ret = SMC_PCI_CALL_SUCCESS;
	uint64_t base;

	spin_lock(&pci_lock);
	base = pci_segment_lib_get_base(addr, off);

	if (base == INVALID_PCI_ADDR) {
		*val = base;
	} else {
		switch (sz) {
		case SMC_PCI_SZ_8BIT:
			*val = mmio_read_8(base);
			break;
		case SMC_PCI_SZ_16BIT:
			*val = mmio_read_16(base);
			break;
		case SMC_PCI_SZ_32BIT:
			*val = mmio_read_32(base);
			break;
		default: /* should be unreachable */
			*val = 0U;
			ret = SMC_PCI_CALL_INVAL_PARAM;
		}
	}
	spin_unlock(&pci_lock);
	return ret;
}

/**
 * pci_write_config() - Performs a config space write at addr
 * @addr: 32-bit, segment, BDF of requested function encoded per DEN0115
 * @off:  register offset of function described by @addr to write
 * @sz:	  size of write (8,16,32) bits.
 * @val:  value to be written
 *
 * sz bits of PCI config space is written at addr:offset. Invalid
 * segment/BDF values return failure. Writes to valid functions
 * without valid EPs are ignored, as is specified by PCI.
 * The boilerplate in pci_svc.c tends to do basic segment, off
 * and sz validation, so it shouldn't need to be repeated here.
 *
 * This function maps directly to the PCI_WRITE function in DEN0115
 * where detailed requirements may be found.
 *
 * Return: SMC_PCI_CALL_SUCCESS
 *	   SMC_PCI_CALL_INVAL_PARAM, on parameter error
 */
uint32_t pci_write_config(uint32_t addr, uint32_t off, uint32_t sz, uint32_t val)
{
	uint32_t ret = SMC_PCI_CALL_SUCCESS;
	uint64_t base;

	spin_lock(&pci_lock);
	base = pci_segment_lib_get_base(addr, off);

	if (base != INVALID_PCI_ADDR) {
		switch (sz) {
		case SMC_PCI_SZ_8BIT:
			mmio_write_8(base, val);
			break;
		case SMC_PCI_SZ_16BIT:
			mmio_write_16(base, val);
			break;
		case SMC_PCI_SZ_32BIT:
			mmio_write_32(base, val);
			break;
		default: /* should be unreachable */
			ret = SMC_PCI_CALL_INVAL_PARAM;
		}
	}
	spin_unlock(&pci_lock);
	return ret;
}

/**
 * pci_get_bus_for_seg() - returns the start->end bus range for a segment
 * @seg:  segment being queried
 * @bus_range:	returned bus begin + (end << 8)
 * @nseg: returns next segment in this machine or 0 for end
 *
 * pci_get_bus_for_seg is called to check if a given segment is
 * valid on this machine. If it is valid, then its bus ranges are
 * returned along with the next valid segment on the machine. If
 * this is the last segment, then nseg must be 0.
 *
 * This function maps directly to the PCI_GET_SEG_INFO function
 * in DEN0115 where detailed requirements may be found.
 *
 * Return: SMC_PCI_CALL_SUCCESS, and appropriate bus_range and nseg
 *	   SMC_PCI_CALL_NOT_IMPL, if the segment is invalid
 */
uint32_t pci_get_bus_for_seg(uint32_t seg, uint32_t *bus_range, uint32_t *nseg)
{
	uint32_t ret = SMC_PCI_CALL_SUCCESS;
	uint32_t rc_count = ARRAY_SIZE(pcie_rc_bases);

	*nseg = (seg < rc_count - 1U) ? seg + 1U : 0U;

	if (seg < rc_count) {
		*bus_range = 0U + (0xFF << 8); /* start 0, end 255 */
	} else {
		*bus_range = 0U;
		ret = SMC_PCI_CALL_NOT_IMPL;
	}
	return ret;
}

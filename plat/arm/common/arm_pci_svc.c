/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <services/pci_svc.h>

uint32_t pci_read_config(uint32_t addr, uint32_t off, uint32_t sz, uint32_t *val)
{
	uint32_t ret = SMC_PCI_CALL_SUCCESS;

	(void)addr;
	(void)off;
	(void)sz;
	(void)val;

	return ret;
}

uint32_t pci_write_config(uint32_t addr, uint32_t off, uint32_t sz, uint32_t val)
{
	uint32_t ret = SMC_PCI_CALL_SUCCESS;

	(void)addr;
	(void)off;
	(void)sz;
	(void)val;

	return ret;
}

uint32_t pci_get_bus_for_seg(uint32_t seg, uint32_t *bus_range, uint32_t *nseg)
{
	uint32_t ret = SMC_PCI_CALL_SUCCESS;

	(void)seg;
	(void)bus_range;
	(void)nseg;

	return ret;
}

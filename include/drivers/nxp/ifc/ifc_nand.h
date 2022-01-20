/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IFC_NAND_H
#define IFC_NAND_H

#define NXP_IFC_SRAM_BUFFER_SIZE	UL(0x100000)	/* 1M */

int ifc_nand_init(uintptr_t *block_dev_spec,
			uintptr_t ifc_region_addr,
			uintptr_t ifc_register_addr,
			size_t ifc_sram_size,
			uintptr_t ifc_nand_blk_offset,
			size_t ifc_nand_blk_size);

#endif /*IFC_NAND_H*/

/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef FLEXSPI_NOR_H
#define FLEXSPI_NOR_H

int flexspi_nor_io_setup(uintptr_t nxp_flexspi_flash_addr,
			 size_t nxp_flexspi_flash_size,
			 uint32_t fspi_base_reg_addr);

#endif /*	FLEXSPI_NOR_H	*/

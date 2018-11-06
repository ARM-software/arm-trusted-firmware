/*
 * Copyright (C) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef IMX_HAB_H
#define IMX_HAB_H

#include <imx_hab_arch.h>
#include <imx_regs.h>

#define HAB_ROM_VECTOR_BASE\
	(BOOTROM_BASE + HAB_CALLBACK_OFFSET)
/*
 * Section 4.5 of the High Assurance Boot Version 4 Application Programming
 * Interface Reference Manual defines the ROM Vector table as coming after a 4
 * byte header
 *
 * A series of function pointers are enumerated at fixed addresses, which are
 * described below
 */
#define HAB_ROM_VECTOR_TABLE_ENTRY		(HAB_ROM_VECTOR_BASE + 0x04)
#define HAB_ROM_VECTOR_TABLE_EXIT		(HAB_ROM_VECTOR_BASE + 0x08)
#define HAB_ROM_VECTOR_TABLE_CHECK_TARGET	(HAB_ROM_VECTOR_BASE + 0x0C)
#define HAB_ROM_VECTOR_TABLE_AUTHENTICATE_IMAGE	(HAB_ROM_VECTOR_BASE + 0x10)
#define HAB_ROM_VECTOR_TABLE_RUN_DCD		(HAB_ROM_VECTOR_BASE + 0x14)
#define HAB_ROM_VECTOR_TABLE_RUN_CSF		(HAB_ROM_VECTOR_BASE + 0x18)
#define HAB_ROM_VECTOR_TABLE_ASSERT		(HAB_ROM_VECTOR_BASE + 0x1C)
#define HAB_ROM_VECTOR_TABLE_REPORT_EVENT	(HAB_ROM_VECTOR_BASE + 0x20)
#define HAB_ROM_VECTOR_TABLE_REPORT_STATUS	(HAB_ROM_VECTOR_BASE + 0x24)
#define HAB_ROM_VECTOR_TABLE_FAILSAFE		(HAB_ROM_VECTOR_BASE + 0x28)

#endif /* IMX_HAB_H */

/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPM_SHIM_PRIVATE_H
#define SPM_SHIM_PRIVATE_H

#include <stdint.h>

#include <lib/utils_def.h>

/* Assembly source */
IMPORT_SYM(uintptr_t, spm_shim_exceptions_ptr,		SPM_SHIM_EXCEPTIONS_PTR);

/* Linker symbols */
IMPORT_SYM(uintptr_t, __SPM_SHIM_EXCEPTIONS_START__,	SPM_SHIM_EXCEPTIONS_START);
IMPORT_SYM(uintptr_t, __SPM_SHIM_EXCEPTIONS_END__,	SPM_SHIM_EXCEPTIONS_END);

/* Definitions */

#define SPM_SHIM_EXCEPTIONS_SIZE	\
	(SPM_SHIM_EXCEPTIONS_END - SPM_SHIM_EXCEPTIONS_START)

/*
 * Use the smallest virtual address space size allowed in ARMv8.0 for
 * compatibility.
 */
#define SPM_SHIM_XLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 25)
#define SPM_SHIM_MMAP_REGIONS	1
#define SPM_SHIM_XLAT_TABLES	1

#endif /* SPM_SHIM_PRIVATE_H */

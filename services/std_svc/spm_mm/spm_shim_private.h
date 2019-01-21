/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
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

#endif /* SPM_SHIM_PRIVATE_H */

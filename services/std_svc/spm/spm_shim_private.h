/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SPM_SHIM_PRIVATE__
#define __SPM_SHIM_PRIVATE__

#include <types.h>

/* Assembly source */
extern uintptr_t spm_shim_exceptions_ptr;

/* Linker symbols */
extern uintptr_t __SPM_SHIM_EXCEPTIONS_START__;
extern uintptr_t __SPM_SHIM_EXCEPTIONS_END__;

/* Definitions */
#define SPM_SHIM_EXCEPTIONS_PTR		(uintptr_t)(&spm_shim_exceptions_ptr)

#define SPM_SHIM_EXCEPTIONS_START	\
	(uintptr_t)(&__SPM_SHIM_EXCEPTIONS_START__)
#define SPM_SHIM_EXCEPTIONS_END		\
	(uintptr_t)(&__SPM_SHIM_EXCEPTIONS_END__)
#define SPM_SHIM_EXCEPTIONS_SIZE	\
	(SPM_SHIM_EXCEPTIONS_END - SPM_SHIM_EXCEPTIONS_START)

#endif /* __SPM_SHIM_PRIVATE__ */

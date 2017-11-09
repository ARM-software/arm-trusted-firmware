/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SECURE_PARTITION_H__
#define __SECURE_PARTITION_H__

#include <bl_common.h>
#include <types.h>
#include <utils_def.h>

/* Linker symbols */
extern uintptr_t __SP_IMAGE_XLAT_TABLES_START__;
extern uintptr_t __SP_IMAGE_XLAT_TABLES_END__;

/* Definitions */
#define SP_IMAGE_XLAT_TABLES_START	\
	(uintptr_t)(&__SP_IMAGE_XLAT_TABLES_START__)
#define SP_IMAGE_XLAT_TABLES_END	\
	(uintptr_t)(&__SP_IMAGE_XLAT_TABLES_END__)
#define SP_IMAGE_XLAT_TABLES_SIZE	\
	(SP_IMAGE_XLAT_TABLES_END - SP_IMAGE_XLAT_TABLES_START)

/*
 * Flags used by the secure_partition_mp_info structure to describe the
 * characteristics of a cpu. Only a single flag is defined at the moment to
 * indicate the primary cpu.
 */
#define MP_INFO_FLAG_PRIMARY_CPU	U(0x00000001)

/*
 * This structure is used to provide information required to initialise a S-EL0
 * partition.
 */
typedef struct secure_partition_mp_info {
	u_register_t		mpidr;
	unsigned int		linear_id;
	unsigned int		flags;
} secure_partition_mp_info_t;

typedef struct secure_partition_boot_info {
	param_header_t		h;
	uintptr_t		sp_mem_base;
	uintptr_t		sp_mem_limit;
	uintptr_t		sp_image_base;
	uintptr_t		sp_stack_base;
	uintptr_t		sp_heap_base;
	uintptr_t		sp_ns_comm_buf_base;
	uintptr_t		sp_shared_buf_base;
	size_t			sp_image_size;
	size_t			sp_pcpu_stack_size;
	size_t			sp_heap_size;
	size_t			sp_ns_comm_buf_size;
	size_t			sp_shared_buf_size;
	unsigned int		num_sp_mem_regions;
	unsigned int		num_cpus;
	secure_partition_mp_info_t	*mp_info;
} secure_partition_boot_info_t;

/* Setup function for secure partitions context. */

void secure_partition_setup(void);

#endif /* __SECURE_PARTITION_H__ */

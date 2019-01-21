/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SECURE_PARTITION_H
#define SECURE_PARTITION_H

#if SPM_MM

#include <stdint.h>

#include <lib/utils_def.h>

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
	uint64_t		mpidr;
	uint32_t		linear_id;
	uint32_t		flags;
} secure_partition_mp_info_t;

typedef struct secure_partition_boot_info {
	param_header_t		h;
	uint64_t		sp_mem_base;
	uint64_t		sp_mem_limit;
	uint64_t		sp_image_base;
	uint64_t		sp_stack_base;
	uint64_t		sp_heap_base;
	uint64_t		sp_ns_comm_buf_base;
	uint64_t		sp_shared_buf_base;
	uint64_t		sp_image_size;
	uint64_t		sp_pcpu_stack_size;
	uint64_t		sp_heap_size;
	uint64_t		sp_ns_comm_buf_size;
	uint64_t		sp_shared_buf_size;
	uint32_t		num_sp_mem_regions;
	uint32_t		num_cpus;
	secure_partition_mp_info_t	*mp_info;
} secure_partition_boot_info_t;

#endif /* SPM_MM */

#endif /* SECURE_PARTITION_H */

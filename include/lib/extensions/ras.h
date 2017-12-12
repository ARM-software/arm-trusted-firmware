/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RAS_COMMON__
#define __RAS_COMMON__

#define ERR_HANDLER_VERSION	1

/* Error record access mechanism */
#define ERR_ACCESS_SYSREG	0
#define ERR_ACCESS_MEMMAP	1

/*
 * Register all error records on the platform.
 *
 * This macro must be used in the same file as the array of error record info
 * are declared. Only then would ARRAY_SIZE() yield a meaningful value.
 */
#define REGISTER_ERR_RECORD_INFO(_info) \
	const struct ras_system_data ras_system_data = { \
		.info = _info, \
		.num_info = ARRAY_SIZE(_info), \
	}

/* Error record info iterator */
#define for_each_err_record_info(_i, _info) \
	for (_i = 0, _info = ras_system_data.info; \
		_i < ras_system_data.num_info; \
		_i++, _info++)

#define _ERR_RECORD_COMMON(_probe, _handler, _aux) \
	.probe = _probe, \
	.handler = _handler, \
	.aux_data = _aux,

#define ERR_RECORD_SYSREG_V1(_idx_start, _num_idx, _probe, _handler, _aux) \
	{ \
		.version = 1, \
		.sysreg.idx_start = _idx_start, \
		.sysreg.num_idx = _num_idx, \
		.access = ERR_ACCESS_SYSREG, \
		_ERR_RECORD_COMMON(_probe, _handler, _aux) \
	}

#define ERR_RECORD_MEMMAP_V1(_base_addr, _size_num_k, _probe, _handler, _aux) \
	{ \
		.version = 1, \
		.memmap.base_addr = _base_addr, \
		.memmap.size_num_k = _size_num_k, \
		.access = ERR_ACCESS_MEMMAP, \
		_ERR_RECORD_COMMON(_probe, _handler, _aux) \
	}

/*
 * Macro to be used to name and declare an array of RAS interrupts along with
 * their handlers.
 *
 * This macro must be used in the same file as the array of interrupts are
 * declared. Only then would ARRAY_SIZE() yield a meaningful value. Also, the
 * array is expected to be sorted in the increasing order of interrupt number.
 */
#define REGISTER_RAS_INTERRUPTS(_array) \
	const struct ras_interrupt_data ras_interrupt_data = { \
		.intrs = _array, \
		.num_intrs = ARRAY_SIZE(_array), \
	}

#ifndef __ASSEMBLY__

#include <assert.h>
#include <ras_arch.h>

struct err_record_info;

typedef struct ras_interrupt {
	/* Interrupt number, and the associated error record info */
	unsigned int intr_number;
	struct err_record_info *info;
	void *cookie;
} ras_interrupt_t;

/* Function to probe a error record group for error */
typedef int (*err_record_probe_t)(const struct err_record_info *info,
		int *probe_data);

/* Data passed to error record group handler */
struct err_handler_data {
	/* Info passed on from top-level exception handler */
	uint64_t flags;
	void *cookie;
	void *handle;

	/* Data strcuture version */
	unsigned int version;

	/* Reason for EA: one the ERROR_* constants */
	unsigned int ea_reason;

	/*
	 * For EAs recevied at vector, the value read from ESR; for an EA
	 * synchronized by ESB, the value of DISR.
	 */
	uint32_t syndrome;

	/* For errors signalled via. interrupt, the raw interrupt ID; otherwise, 0. */
	unsigned int interrupt;
};

/* Function to handle error from an error record group */
typedef int (*err_record_handler_t)(const struct err_record_info *info,
		int probe_data, const struct err_handler_data *const data);

/* Error record information */
struct err_record_info {
	/* Function to probe error record group for errors */
	err_record_probe_t probe;

	/* Function to handle error record group errors */
	err_record_handler_t handler;

	/* Opaque group-specific data */
	void *aux_data;

	/* Additional information for Standard Error Records */
	union {
		struct {
			/*
			 * For a group accessed via. memory-mapped register,
			 * base address of the page hosting error records, and
			 * the size of the record group.
			 */
			uintptr_t base_addr;

			/* Size of group in number of KBs */
			unsigned int size_num_k;
		} memmap;

		struct {
			/*
			 * For error records accessed via. system register, index of
			 * the error record.
			 */
			unsigned int idx_start;
			unsigned int num_idx;
		} sysreg;
	};

	/* Data structure version */
	uint16_t version;

	/* Error record access mechanism */
	unsigned int access:1;
};

struct ras_system_data {
	struct err_record_info *info;
	size_t num_info;
};

struct ras_interrupt_data {
	ras_interrupt_t *intrs;
	size_t num_intrs;
};

extern const struct ras_system_data ras_system_data;
extern const struct ras_interrupt_data ras_interrupt_data;


/*
 * Helper functions to probe memory-mapped and system registers implemented in
 * Standard Error Record format
 */
static inline int ras_err_ser_probe_memmap(const struct err_record_info *info,
		int *probe_data)
{
	assert(info->version == ERR_HANDLER_VERSION);

	return ser_probe_memmap(info->memmap.base_addr, info->memmap.size_num_k,
		probe_data);
}

static inline int ras_err_ser_probe_sysreg(const struct err_record_info *info,
		int *probe_data)
{
	assert(info->version == ERR_HANDLER_VERSION);

	return ser_probe_sysreg(info->sysreg.idx_start, info->sysreg.num_idx,
			probe_data);
}

int ras_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		void *handle, uint64_t flags);
void ras_init(void);

#endif /* __ASSEMBLY__ */
#endif /* __RAS_COMMON__ */

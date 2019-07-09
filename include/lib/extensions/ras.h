/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RAS_H
#define RAS_H

#define ERR_HANDLER_VERSION	1U

/* Error record access mechanism */
#define ERR_ACCESS_SYSREG	0
#define ERR_ACCESS_MEMMAP	1

/*
 * Register all error records on the platform.
 *
 * This macro must be used in the same file as the array of error record info
 * are declared. Only then would ARRAY_SIZE() yield a meaningful value.
 */
#define REGISTER_ERR_RECORD_INFO(_records) \
	const struct err_record_mapping err_record_mappings = { \
		.err_records = (_records), \
		.num_err_records = ARRAY_SIZE(_records), \
	}

/* Error record info iterator */
#define for_each_err_record_info(_i, _info) \
	for ((_i) = 0, (_info) = err_record_mappings.err_records; \
		(_i) < err_record_mappings.num_err_records; \
		(_i)++, (_info)++)

#define ERR_RECORD_COMMON_(_probe, _handler, _aux) \
	.probe = _probe, \
	.handler = _handler, \
	.aux_data = _aux,

#define ERR_RECORD_SYSREG_V1(_idx_start, _num_idx, _probe, _handler, _aux) \
	{ \
		.version = 1, \
		.sysreg.idx_start = _idx_start, \
		.sysreg.num_idx = _num_idx, \
		.access = ERR_ACCESS_SYSREG, \
		ERR_RECORD_COMMON_(_probe, _handler, _aux) \
	}

#define ERR_RECORD_MEMMAP_V1(_base_addr, _size_num_k, _probe, _handler, _aux) \
	{ \
		.version = 1, \
		.memmap.base_addr = _base_addr, \
		.memmap.size_num_k = _size_num_k, \
		.access = ERR_ACCESS_MEMMAP, \
		ERR_RECORD_COMMON_(_probe, _handler, _aux) \
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
	const struct ras_interrupt_mapping ras_interrupt_mappings = { \
		.intrs = (_array), \
		.num_intrs = ARRAY_SIZE(_array), \
	}

#ifndef __ASSEMBLER__

#include <assert.h>

#include <lib/extensions/ras_arch.h>

struct err_record_info;

struct ras_interrupt {
	/* Interrupt number, and the associated error record info */
	unsigned int intr_number;
	struct err_record_info *err_record;
	void *cookie;
};

/* Function to probe a error record group for error */
typedef int (*err_record_probe_t)(const struct err_record_info *info,
		int *probe_data);

/* Data passed to error record group handler */
struct err_handler_data {
	/* Info passed on from top-level exception handler */
	uint64_t flags;
	void *cookie;
	void *handle;

	/* Data structure version */
	unsigned int version;

	/* Reason for EA: one the ERROR_* constants */
	unsigned int ea_reason;

	/*
	 * For EAs received at vector, the value read from ESR; for an EA
	 * synchronized by ESB, the value of DISR.
	 */
	uint32_t syndrome;

	/* For errors signalled via interrupt, the raw interrupt ID; otherwise, 0. */
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
			 * For a group accessed via memory-mapped register,
			 * base address of the page hosting error records, and
			 * the size of the record group.
			 */
			uintptr_t base_addr;

			/* Size of group in number of KBs */
			unsigned int size_num_k;
		} memmap;

		struct {
			/*
			 * For error records accessed via system register, index of
			 * the error record.
			 */
			unsigned int idx_start;
			unsigned int num_idx;
		} sysreg;
	};

	/* Data structure version */
	unsigned int version;

	/* Error record access mechanism */
	unsigned int access:1;
};

struct err_record_mapping {
	struct err_record_info *err_records;
	size_t num_err_records;
};

struct ras_interrupt_mapping {
	struct ras_interrupt *intrs;
	size_t num_intrs;
};

extern const struct err_record_mapping err_record_mappings;
extern const struct ras_interrupt_mapping ras_interrupt_mappings;


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

#endif /* __ASSEMBLER__ */

#endif /* RAS_H */

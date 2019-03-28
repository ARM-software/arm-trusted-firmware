/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * The profiler stores the timestamps captured during cold boot to the shared
 * memory for the non-secure world. The non-secure world driver parses the
 * shared memory block and writes the contents to a file on the device, which
 * can be later extracted for analysis.
 *
 * Profiler memory map
 *
 * TOP     ---------------------------      ---
 *            Trusted OS timestamps         3KB
 *         ---------------------------      ---
 *         Trusted Firmware timestamps      1KB
 * BASE    ---------------------------      ---
 *
 ******************************************************************************/

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <profiler.h>
#include <stdbool.h>
#include <string.h>

static uint64_t shmem_base_addr;

#define MAX_PROFILER_RECORDS	U(16)
#define TAG_LEN_BYTES		U(56)

/*******************************************************************************
 * Profiler entry format
 ******************************************************************************/
typedef struct {
	/* text explaining the timestamp location in code */
	uint8_t tag[TAG_LEN_BYTES];
	/* timestamp value */
	uint64_t timestamp;
} profiler_rec_t;

static profiler_rec_t *head, *cur, *tail;
static uint32_t tmr;
static bool is_shmem_buf_mapped;

/*******************************************************************************
 * Initialise the profiling library
 ******************************************************************************/
void boot_profiler_init(uint64_t shmem_base, uint32_t tmr_base)
{
	uint64_t shmem_end_base;

	assert(shmem_base != ULL(0));
	assert(tmr_base != U(0));

	/* store the buffer address */
	shmem_base_addr = shmem_base;

	/* calculate the base address of the last record */
	shmem_end_base = shmem_base + (sizeof(profiler_rec_t) *
			 (MAX_PROFILER_RECORDS - U(1)));

	/* calculate the head, tail and cur values */
	head = (profiler_rec_t *)shmem_base;
	tail = (profiler_rec_t *)shmem_end_base;
	cur = head;

	/* timer used to get the current timestamp */
	tmr = tmr_base;
}

/*******************************************************************************
 * Add tag and timestamp to profiler
 ******************************************************************************/
void boot_profiler_add_record(const char *str)
{
	unsigned int len;

	/* calculate the length of the tag */
	if (((unsigned int)strlen(str) + U(1)) > TAG_LEN_BYTES) {
		len = TAG_LEN_BYTES;
	} else {
		len = (unsigned int)strlen(str) + U(1);
	}

	if (head != NULL) {

		/*
		 * The profiler runs with/without MMU enabled. Check
		 * if MMU is enabled and memmap the shmem buffer, in
		 * case it is.
		 */
		if ((!is_shmem_buf_mapped) &&
		    ((read_sctlr_el3() & SCTLR_M_BIT) != U(0))) {

			(void)mmap_add_dynamic_region(shmem_base_addr,
					shmem_base_addr,
					PROFILER_SIZE_BYTES,
					(MT_NS | MT_RW | MT_EXECUTE_NEVER));

			is_shmem_buf_mapped = true;
		}

		/* write the tag and timestamp to buffer */
		(void)snprintf((char *)cur->tag, len, "%s", str);
		cur->timestamp = mmio_read_32(tmr);

		/* start from head if we reached the end */
		if (cur == tail) {
			cur = head;
		} else {
			cur++;
		}
	}
}

/*******************************************************************************
 * Deinint the profiler
 ******************************************************************************/
void boot_profiler_deinit(void)
{
	if (shmem_base_addr != ULL(0)) {

		/* clean up resources */
		cur = NULL;
		head = NULL;
		tail = NULL;

		/* flush the shmem for it to be visible to the NS world */
		flush_dcache_range(shmem_base_addr, PROFILER_SIZE_BYTES);

		/* unmap the shmem buffer */
		if (is_shmem_buf_mapped) {
			(void)mmap_remove_dynamic_region(shmem_base_addr,
					PROFILER_SIZE_BYTES);
		}
	}
}

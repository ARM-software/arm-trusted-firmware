/*
 * Copyright (c) 2017-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Runtime firmware routines to report errata status for the current CPU. */

#include <assert.h>
#include <stdbool.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/cpus/cpu_ops.h>
#include <lib/cpus/errata.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/spinlock.h>

#ifdef IMAGE_BL1
# define BL_STRING	"BL1"
#elif defined(__aarch64__) && defined(IMAGE_BL31)
# define BL_STRING	"BL31"
#elif !defined(__aarch64__) && defined(IMAGE_BL32)
# define BL_STRING	"BL32"
#elif defined(IMAGE_BL2) && RESET_TO_BL2
# define BL_STRING "BL2"
#else
# error This image should not be printing errata status
#endif

/* Errata format: BL stage, CPU, errata ID, message */
#define ERRATA_FORMAT	"%s: %s: CPU workaround for %s was %s\n"

#define CVE_FORMAT	"%s: %s: CPU workaround for CVE %u_%04u was %s\n"
#define ERRATUM_FORMAT	"%s: %s: CPU workaround for erratum %u was %s\n"


static __unused void print_status(int status, char *cpu_str, uint16_t cve, uint32_t id)
{
	if (status == ERRATA_MISSING) {
		if (cve) {
			WARN(CVE_FORMAT, BL_STRING, cpu_str, cve, id, "missing!");
		} else {
			WARN(ERRATUM_FORMAT, BL_STRING, cpu_str, id, "missing!");
		}
	} else if (status == ERRATA_APPLIES) {
		if (cve) {
			INFO(CVE_FORMAT, BL_STRING, cpu_str, cve, id, "applied");
		}  else {
			INFO(ERRATUM_FORMAT, BL_STRING, cpu_str, id, "applied");
		}
	} else {
		if (cve) {
			VERBOSE(CVE_FORMAT, BL_STRING, cpu_str, cve, id, "not applicable");
		}  else {
			VERBOSE(ERRATUM_FORMAT, BL_STRING, cpu_str, id, "not applicable");
		}
	}
}

#if !REPORT_ERRATA
void print_errata_status(void) {}
#else /* !REPORT_ERRATA */
/*
 * New errata status message printer
 */
static void generic_errata_report(struct cpu_ops *cpu_ops)
{
	struct erratum_entry *entry = cpu_ops->errata_list_start;
	struct erratum_entry *end = cpu_ops->errata_list_end;
	long rev_var = cpu_get_rev_var();

	for (; entry != end; entry += 1) {
		uint64_t status = entry->check_func(rev_var);

		assert(entry->id != 0);

		/*
		 * Errata workaround has not been compiled in. If the errata
		 * would have applied had it been compiled in, print its status
		 * as missing.
		 */
		if (status == ERRATA_APPLIES && entry->chosen == 0) {
			status = ERRATA_MISSING;
		}

		print_status(status, cpu_ops->cpu_str, entry->cve, entry->id);
	}
}

/*
 * Function to print errata status for the calling CPU (and others of the same
 * type). Must be called only:
 *   - when MMU and data caches are enabled;
 *   - after cpu_ops have been initialized in per-CPU data.
 */
void print_errata_status(void)
{
#ifdef IMAGE_BL1
	struct cpu_ops *cpu_ops = get_cpu_ops_ptr();
#else
	struct cpu_ops *cpu_ops = get_cpu_data(cpu_ops_ptr);
#endif

	assert(cpu_ops != NULL);

#ifndef IMAGE_BL1
	/*
	 * Try to acquire the lock. The first CPU of each type to do so will
	 * report errata. The others need not do anything. Never release the
	 * lock as there are no other users.
	 */
	if (!spin_trylock(cpu_ops->errata_reported)) {
		return;
	}
#endif

	generic_errata_report(cpu_ops);
}
#endif /* !REPORT_ERRATA */

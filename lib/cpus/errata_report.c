/*
 * Copyright (c) 2017-2024, Arm Limited and Contributors. All rights reserved.
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

#define CVE_FORMAT	"%s: %s: CPU workaround for CVE %u_%u was %s\n"
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
 * The order checking function is hidden behind the FEATURE_DETECTION flag to
 * save space. This functionality is only useful on development and platform
 * bringup builds, when FEATURE_DETECTION should be used anyway
 */
void generic_errata_report(void)
{
	struct cpu_ops *cpu_ops = get_cpu_ops_ptr();
	struct erratum_entry *entry = cpu_ops->errata_list_start;
	struct erratum_entry *end = cpu_ops->errata_list_end;
	long rev_var = cpu_get_rev_var();
#if FEATURE_DETECTION
	uint32_t last_erratum_id = 0;
	uint16_t last_cve_yr = 0;
	bool check_cve = false;
	bool failed = false;
#endif /* FEATURE_DETECTION */

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

#if FEATURE_DETECTION
		if (entry->cve) {
			if (last_cve_yr > entry->cve ||
			   (last_cve_yr == entry->cve && last_erratum_id >= entry->id)) {
				ERROR("CVE %u_%u was out of order!\n",
				      entry->cve, entry->id);
				failed = true;
			}
			check_cve = true;
			last_cve_yr = entry->cve;
		} else {
			if (last_erratum_id >= entry->id || check_cve) {
				ERROR("Erratum %u was out of order!\n",
				      entry->id);
				failed = true;
			}
		}
		last_erratum_id = entry->id;
#endif /* FEATURE_DETECTION */
	}

#if FEATURE_DETECTION
	/*
	 * enforce errata and CVEs are in ascending order and that CVEs are
	 * after errata
	 */
	assert(!failed);
#endif /* FEATURE_DETECTION */
}

/*
 * Returns whether errata needs to be reported. Passed arguments are private to
 * a CPU type.
 */
static __unused int errata_needs_reporting(spinlock_t *lock, uint32_t *reported)
{
	bool report_now;

	/* If already reported, return false. */
	if (*reported != 0U)
		return 0;

	/*
	 * Acquire lock. Determine whether status needs reporting, and then mark
	 * report status to true.
	 */
	spin_lock(lock);
	report_now = (*reported == 0U);
	if (report_now)
		*reported = 1;
	spin_unlock(lock);

	return report_now;
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
	generic_errata_report();
#else /* IMAGE_BL1 */
	struct cpu_ops *cpu_ops = (void *) get_cpu_data(cpu_ops_ptr);

	assert(cpu_ops != NULL);

	if (errata_needs_reporting(cpu_ops->errata_lock, cpu_ops->errata_reported)) {
		generic_errata_report();
	}
#endif /* IMAGE_BL1 */
}
#endif /* !REPORT_ERRATA */

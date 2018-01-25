/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Runtime firmware routines to report errata status for the current CPU. */

#include <arch_helpers.h>
#include <assert.h>
#include <cpu_data.h>
#include <debug.h>
#include <errata_report.h>
#include <spinlock.h>
#include <utils.h>

#ifdef IMAGE_BL1
# define BL_STRING	"BL1"
#elif defined(AARCH64) && defined(IMAGE_BL31)
# define BL_STRING	"BL31"
#elif defined(AARCH32) && defined(IMAGE_BL32)
# define BL_STRING	"BL32"
#elif defined(IMAGE_BL2) && BL2_AT_EL3
# define BL_STRING "BL2"
#else
# error This image should not be printing errata status
#endif

/* Errata format: BL stage, CPU, errata ID, message */
#define ERRATA_FORMAT	"%s: %s: CPU workaround for %s was %s\n"

/*
 * Returns whether errata needs to be reported. Passed arguments are private to
 * a CPU type.
 */
int errata_needs_reporting(spinlock_t *lock, uint32_t *reported)
{
	int report_now;

	/* If already reported, return false. */
	if (*reported)
		return 0;

	/*
	 * Acquire lock. Determine whether status needs reporting, and then mark
	 * report status to true.
	 */
	spin_lock(lock);
	report_now = !(*reported);
	if (report_now)
		*reported = 1;
	spin_unlock(lock);

	return report_now;
}

/*
 * Print errata status message.
 *
 * Unknown: WARN
 * Missing: WARN
 * Applied: INFO
 * Not applied: VERBOSE
 */
void errata_print_msg(unsigned int status, const char *cpu, const char *id)
{
	/* Errata status strings */
	static const char *const errata_status_str[] = {
		[ERRATA_NOT_APPLIES] = "not applied",
		[ERRATA_APPLIES] = "applied",
		[ERRATA_MISSING] = "missing!"
	};
	static const char *const __unused bl_str = BL_STRING;
	const char *msg __unused;


	assert(status < ARRAY_SIZE(errata_status_str));
	assert(cpu);
	assert(id);

	msg = errata_status_str[status];

	switch (status) {
	case ERRATA_NOT_APPLIES:
		VERBOSE(ERRATA_FORMAT, bl_str, cpu, id, msg);
		break;

	case ERRATA_APPLIES:
		INFO(ERRATA_FORMAT, bl_str, cpu, id, msg);
		break;

	case ERRATA_MISSING:
		WARN(ERRATA_FORMAT, bl_str, cpu, id, msg);
		break;

	default:
		WARN(ERRATA_FORMAT, bl_str, cpu, id, "unknown");
		break;
	}
}

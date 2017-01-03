/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
#else
# error This image should not be printing errata status
#endif

/* Errata format: BL stage, CPU, errata ID, message */
#define ERRATA_FORMAT	"%s: %s: errata workaround for %s was %s\n"

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
void errata_print_msg(int status, const char *cpu, const char *id)
{
	/* Errata status strings */
	static const char *const errata_status_str[] = {
		[ERRATA_NOT_APPLIES] = "not applied",
		[ERRATA_APPLIES] = "applied",
		[ERRATA_MISSING] = "missing!"
	};
	static const char *const __unused bl_str = BL_STRING;
	const char *msg __unused;


	assert(status >= 0 && status < ARRAY_SIZE(errata_status_str));
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

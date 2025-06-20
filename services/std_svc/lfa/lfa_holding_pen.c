/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <common/debug.h>
#include <lib/psci/psci_lib.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>
#include <services/lfa_holding_pen.h>

#include <platform_def.h>

static spinlock_t holding_lock;
static spinlock_t activation_lock;
static uint32_t activation_count;
static enum lfa_retc activation_status;

/**
 * lfa_holding_start - Called by each active CPU to coordinate live activation.
 *
 * Note that only CPUs that are active at the time of activation will
 * participate in CPU rendezvous.
 *
 * This function is invoked by each CPU participating in the LFA Activate
 * process. It increments the shared activation count under `activation_lock`
 * to track how many CPUs have entered the activation phase.
 *
 * The first CPU to enter acquires the `holding_lock`, which ensures
 * serialization during the wait and activation phases. This lock is
 * released only after the last CPU completes the activation.
 *
 * The function returns `true` only for the last CPU to enter, allowing it
 * to proceed with performing the live firmware activation. All other CPUs
 * receive `false` and will wait in `lfa_holding_wait()` until activation
 * is complete.
 *
 * @return `true` for the last CPU, `false` for all others.
 */
bool lfa_holding_start(void)
{
	bool status;
	unsigned int no_of_cpus;

	spin_lock(&activation_lock);

	if (activation_count == 0U) {
		/* First CPU locks holding lock */
		spin_lock(&holding_lock);
	}

	activation_count += 1U;

	no_of_cpus = psci_num_cpus_running_on_safe(plat_my_core_pos());
	status = (activation_count == no_of_cpus);
	if (!status) {
		VERBOSE("Hold, %d CPU left\n",
			 PLATFORM_CORE_COUNT - activation_count);
	}

	spin_unlock(&activation_lock);

	return status;
}

/**
 * lfa_holding_wait - CPUs wait until activation is completed by the last CPU.
 *
 * All CPUs are serialized using `holding_lock`, which is initially acquired
 * by the first CPU in `lfa_holding_start()` and only released by the last
 * CPU through `lfa_holding_release()`. This ensures that no two CPUs enter
 * the critical section at the same time during the wait phase. Once the
 * last CPU completes activation, each CPU decrements the activation count
 * and returns the final activation status,  which was set by the last CPU
 * to complete the activation process.
 *
 * @return Activation status set by the last CPU.
 */
enum lfa_retc lfa_holding_wait(void)
{
	spin_lock(&holding_lock);
	activation_count -= 1U;
	spin_unlock(&holding_lock);
	return activation_status;
}

/**
 * lfa_holding_release - Called by the last CPU to complete activation.
 *
 * This function is used by the last participating CPU after it completes
 * live firmware activation. It updates the shared activation status and
 * resets the activation count. Finally, it releases the `holding_lock` to
 * allow other CPUs that were waiting in `lfa_holding_wait()` to proceed.
 *
 * @param status Activation status to be shared with other CPUs.
 */
void lfa_holding_release(enum lfa_retc status)
{
	activation_count = 0U;
	activation_status = status;
	spin_unlock(&holding_lock);
}

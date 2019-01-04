/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/el3_runtime/pubsub.h>

/*
 * This file defines a list of pubsub events, declared using
 * REGISTER_PUBSUB_EVENT() macro.
 */

/*
 * Event published after a CPU has been powered up and finished its
 * initialization.
 */
REGISTER_PUBSUB_EVENT(psci_cpu_on_finish);

/*
 * These events are published before/after a CPU has been powered down/up
 * via the PSCI CPU SUSPEND API.
 */
REGISTER_PUBSUB_EVENT(psci_suspend_pwrdown_start);
REGISTER_PUBSUB_EVENT(psci_suspend_pwrdown_finish);

#ifdef AARCH64
/*
 * These events are published by the AArch64 context management framework
 * after the secure context is restored/saved via
 * cm_el1_sysregs_context_{restore,save}() API.
 */
REGISTER_PUBSUB_EVENT(cm_entering_secure_world);
REGISTER_PUBSUB_EVENT(cm_exited_secure_world);

/*
 * These events are published by the AArch64 context management framework
 * after the normal context is restored/saved via
 * cm_el1_sysregs_context_{restore,save}() API.
 */
REGISTER_PUBSUB_EVENT(cm_entering_normal_world);
REGISTER_PUBSUB_EVENT(cm_exited_normal_world);
#endif /* AARCH64 */

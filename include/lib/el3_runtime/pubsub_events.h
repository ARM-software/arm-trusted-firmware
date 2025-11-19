/*
 * Copyright (c) 2017-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/el3_runtime/pubsub.h>

/*
 * This file defines a list of pubsub events, declared using
 * REGISTER_PUBSUB_EVENT() macro.
 */

/*
 * psci_cpu_on_finish is published after CPU power-up has completed.
 * psci_cpu_off_start is published during CPU-off handling, before the
 * CPU is powered down and before its affinity state is updated to OFF.
 */
REGISTER_PUBSUB_EVENT(psci_cpu_on_finish);
REGISTER_PUBSUB_EVENT(psci_cpu_off_start);

/*
 * These events are published before/after a CPU has been powered down/up
 * via the PSCI CPU SUSPEND API.
 */
REGISTER_PUBSUB_EVENT(psci_suspend_pwrdown_start);
REGISTER_PUBSUB_EVENT(psci_suspend_pwrdown_finish);

#ifdef __aarch64__
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
#endif /* __aarch64__ */

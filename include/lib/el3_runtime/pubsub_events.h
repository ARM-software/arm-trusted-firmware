/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pubsub.h>

/*
 * This file defines a list of pubsub events, declared using
 * REGISTER_PUBSUB_EVENT() macro.
 */

/*
 * Event published after a CPU has been powered up and finished its
 * initialization.
 */
REGISTER_PUBSUB_EVENT(psci_cpu_on_finish);

/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MORELLO_PRIVATE_H
#define MORELLO_PRIVATE_H

#include <lib/psci/psci.h>

void morello_pwr_domain_off(const psci_power_state_t *target_state);

#endif /* MORELLO_PRIVATE_H */

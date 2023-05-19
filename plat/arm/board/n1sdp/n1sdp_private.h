/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef N1SDP_PRIVATE_H
#define N1SDP_PRIVATE_H

#include <lib/psci/psci.h>

void n1sdp_pwr_domain_off(const psci_power_state_t *target_state);

#endif /* N1SDP_PRIVATE_H */

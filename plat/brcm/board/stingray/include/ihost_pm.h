/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IHOST_PM
#define IHOST_PM

#include <stdint.h>

#define CLUSTER_POWER_ON	0x1
#define CLUSTER_POWER_OFF	0x0

void ihost_power_on_cluster(u_register_t mpidr);
void ihost_power_on_secondary_core(u_register_t mpidr, uint64_t rvbar);
void ihost_enable_satellite_timer(unsigned int cluster_id);

#endif

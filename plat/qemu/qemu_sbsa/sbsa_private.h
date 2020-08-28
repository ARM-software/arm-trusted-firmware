/*
 * Copyright (c) 2020, Nuvia Inc
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SBSA_PRIVATE_H
#define SBSA_PRIVATE_H

#include <stdint.h>

unsigned int plat_qemu_calc_core_pos(u_register_t mpidr);

void qemu_pwr_gic_on_finish(void);
void qemu_pwr_gic_off(void);

#endif /* SBSA_PRIVATE_H */

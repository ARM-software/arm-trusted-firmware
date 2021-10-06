/*
 * Copyright (c) 2020-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MEASURED_BOOT_H
#define MEASURED_BOOT_H

#include <stdint.h>

#include <drivers/measured_boot/event_log.h>

/* Functions' declarations */
void measured_boot_init(void);
void measured_boot_finish(void);

#endif /* MEASURED_BOOT_H */

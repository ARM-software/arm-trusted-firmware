/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MICRO_DELAY_H
#define MICRO_DELAY_H

#define TMU3_MEASUREMENT	(0)

#ifndef __ASSEMBLY__
#include <stdint.h>
void rcar_micro_delay(uint32_t count_us);

#if (TMU3_MEASUREMENT == 1)
void tmu3_start(void);
void tmu3_init(void);
void tmu3_stop(void);

uint32_t tcnt3_snapshot(void);
#endif

#endif

#endif /* MICRO_DELAY_H */

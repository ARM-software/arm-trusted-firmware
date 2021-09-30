/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XLAT_MPU_H
#define XLAT_MPU_H

#ifndef __ASSEMBLER__

#include <lib/cassert.h>

#define XLAT_TABLES_LIB_V2	1

void enable_mpu_el2(unsigned int flags);
void enable_mpu_direct_el2(unsigned int flags);

/*
 * Function to wipe clean and disable all MPU regions.  This function expects
 * that the MPU has already been turned off, and caching concerns addressed,
 * but it nevertheless also explicitly turns off the MPU.
 */
void clear_all_mpu_regions(void);

#endif /* __ASSEMBLER__ */
#endif /* XLAT_MPU_H */

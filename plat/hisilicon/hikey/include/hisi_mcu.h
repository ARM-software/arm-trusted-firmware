/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HISI_MCU_H
#define HISI_MCU_H

#include <stdint.h>

extern void hisi_mcu_enable_sram(void);
extern void hisi_mcu_start_run(void);
extern int hisi_mcu_load_image(uintptr_t image_base, uint32_t image_size);

#endif /* HISI_MCU_H */

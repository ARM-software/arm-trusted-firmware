/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ras.h>

struct ras_interrupt arm_ras_interrupts[] = {
};

struct err_record_info arm_err_records[] = {
};

REGISTER_ERR_RECORD_INFO(arm_err_records);
REGISTER_RAS_INTERRUPTS(arm_ras_interrupts);

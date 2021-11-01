/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/extensions/ras.h>

struct ras_interrupt fvp_ras_interrupts[] = {
};

struct err_record_info fvp_err_records[] = {
};

REGISTER_ERR_RECORD_INFO(fvp_err_records);
REGISTER_RAS_INTERRUPTS(fvp_ras_interrupts);

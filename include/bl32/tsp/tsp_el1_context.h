/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TSP_EL1_CONTEXT_H
#define TSP_EL1_CONTEXT_H

#define TSP_CORRUPT_EL1_REGS        1
#define TSP_RESTORE_EL1_REGS        0

/* Public helper function to handle EL1 ctx registers at S-EL1(TSP) */
void modify_el1_ctx_regs(const bool modify_option);

#endif /* TSP_EL1_CONTEXT_H */

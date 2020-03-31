/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SMCCC_DEF_H
#define SMCCC_DEF_H

/* Defines used to retrieve ARM SOC revision */
#define ARM_SOC_CONTINUATION_CODE	U(0x4)
#define ARM_SOC_IDENTIFICATION_CODE	U(0x3B)
#define ARM_SOC_CONTINUATION_SHIFT	U(24)
#define ARM_SOC_IDENTIFICATION_SHIFT	U(16)

#endif /* SMCCC_DEF_H */

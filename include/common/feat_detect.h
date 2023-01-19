/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FEAT_DETECT_H
#define FEAT_DETECT_H

/* Function Prototypes */
void detect_arch_features(void);

/* Macro Definitions */
#define FEAT_STATE_DISABLED	0
#define FEAT_STATE_ALWAYS	1
#define FEAT_STATE_CHECK	2

#endif /* FEAT_DETECT_H */

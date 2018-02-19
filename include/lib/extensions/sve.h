/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SVE_H__
#define __SVE_H__

int sve_supported(void);
void sve_enable(int el2_unused);

#endif /* __SVE_H__ */

/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL31_LFA_H
#define BL31_LFA_H

#include <services/lfa_component_desc.h>

struct lfa_component_ops *get_bl31_activator(void);

#endif /* BL31_LFA_H */

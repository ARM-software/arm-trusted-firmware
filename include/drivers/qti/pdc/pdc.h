/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PDC_H
#define PDC_H

#ifdef QTI_PDC_ENABLED
void qti_pdc_init(void);
#else
static inline void qti_pdc_init(void) {}
#endif

#endif /* PDC_H */

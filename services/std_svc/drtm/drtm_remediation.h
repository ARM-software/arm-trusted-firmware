/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 */
#ifndef DRTM_REMEDIATION_H
#define DRTM_REMEDIATION_H

uint64_t drtm_set_error(uint64_t x1, void *ctx);
uint64_t drtm_get_error(void *ctx);

void drtm_enter_remediation(uint64_t error_code, const char *error_str);

#endif /* DRTM_REMEDIATION_H */

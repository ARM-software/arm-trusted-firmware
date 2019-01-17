/*
 * Copyright (c) 2016-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BSEC_SVC_H
#define BSEC_SVC_H

#include <stdint.h>

/* version of this service */
/* must be increase at each structure modification */
#define BSEC_SERVICE_VERSION		0x01U

uint32_t bsec_main(uint32_t x1, uint32_t x2, uint32_t x3,
		   uint32_t *ret_otp_value);

#endif /* BSEC_SVC_H */

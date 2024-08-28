
/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/tpm/tpm2_chip.h>

/*
 * TPM timeout values
 * Reference: TCG PC Client Platform TPM Profile (PTP) Specification v1.05
 */
struct tpm_chip_data tpm_chip_data = {
	.locality = -1,
	.timeout_msec_a = 750,
	.timeout_msec_b = 2000,
	.timeout_msec_c = 200,
	.timeout_msec_d = 30,
	.address = 0,
};

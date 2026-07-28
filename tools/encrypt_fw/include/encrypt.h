/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019, Linaro Limited. All rights reserved.
 * Author: Sumit Garg <sumit.garg@linaro.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ENCRYPT_H
#define ENCRYPT_H

/* Supported key algorithms */
enum {
	KEY_ALG_GCM,		/* AES-GCM (default) */
	KEY_ALG_CCM		/* AES-CCM */
};

#endif /* ENCRYPT_H */

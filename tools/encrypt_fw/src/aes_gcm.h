/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AES_GCM_H
#define AES_GCM_H

int gcm_encrypt(unsigned short fw_enc_status, const char *key_string,
		const char *nonce_string, const char *ip_name,
		const char *op_name);

#endif /* AES_GCM_H */

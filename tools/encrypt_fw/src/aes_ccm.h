/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AES_CCM_H
#define AES_CCM_H

int ccm_encrypt(unsigned short fw_enc_status, const char *key_string,
		const char *nonce_string, const char *input_file_path,
		const char *output_file_path);

#endif /* AES_CCM_H */

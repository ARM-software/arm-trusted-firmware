/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TBBR_COT_COMMON_H
#define TBBR_COT_COMMON_H

#include <drivers/auth/auth_mod.h>

extern unsigned char tb_fw_hash_buf[HASH_DER_LEN];
extern unsigned char scp_fw_hash_buf[HASH_DER_LEN];
extern unsigned char nt_world_bl_hash_buf[HASH_DER_LEN];

extern auth_param_type_desc_t trusted_nv_ctr;
extern auth_param_type_desc_t subject_pk;
extern auth_param_type_desc_t sig;
extern auth_param_type_desc_t sig_alg;
extern auth_param_type_desc_t raw_data;

extern auth_param_type_desc_t tb_fw_hash;
extern auth_param_type_desc_t tb_fw_config_hash;
extern auth_param_type_desc_t fw_config_hash;

extern const auth_img_desc_t trusted_boot_fw_cert;
extern const auth_img_desc_t hw_config;

#endif /* TBBR_COT_COMMON_H */

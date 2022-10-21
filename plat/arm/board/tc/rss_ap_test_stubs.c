/*
 * Copyright (c) 2022, Arm Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include <delegated_attestation.h>
#include <measured_boot.h>
#include <psa/error.h>


psa_status_t
tfm_measured_boot_extend_measurement(uint8_t index,
				     const uint8_t *signer_id,
				     size_t signer_id_size,
				     const uint8_t *version,
				     size_t version_size,
				     uint32_t measurement_algo,
				     const uint8_t *sw_type,
				     size_t sw_type_size,
				     const uint8_t *measurement_value,
				     size_t measurement_value_size,
				     bool lock_measurement)
{
	return rss_measured_boot_extend_measurement(index,
						    signer_id,
						    signer_id_size,
						    version,
						    version_size,
						    measurement_algo,
						    sw_type,
						    sw_type_size,
						    measurement_value,
						    measurement_value_size,
						    lock_measurement);
}

psa_status_t
tfm_measured_boot_read_measurement(uint8_t index,
				   uint8_t *signer_id,
				   size_t signer_id_size,
				   size_t *signer_id_len,
				   uint8_t *version,
				   size_t version_size,
				   size_t *version_len,
				   uint32_t *measurement_algo,
				   uint8_t *sw_type,
				   size_t sw_type_size,
				   size_t *sw_type_len,
				   uint8_t *measurement_value,
				   size_t measurement_value_size,
				   size_t *measurement_value_len,
				   bool *is_locked)
{
	return rss_measured_boot_read_measurement(index,
						  signer_id,
						  signer_id_size,
						  signer_id_len,
						  version,
						  version_size,
						  version_len,
						  measurement_algo,
						  sw_type,
						  sw_type_size,
						  sw_type_len,
						  measurement_value,
						  measurement_value_size,
						  measurement_value_len,
						  is_locked);
}

psa_status_t
tfm_delegated_attest_get_token(const uint8_t *dak_pub_hash,
			       size_t         dak_pub_hash_size,
			       uint8_t       *token_buf,
			       size_t         token_buf_size,
			       size_t        *token_size)
{
	return rss_delegated_attest_get_token(dak_pub_hash,
					      dak_pub_hash_size,
					      token_buf,
					      token_buf_size,
					      token_size);
}

psa_status_t
tfm_delegated_attest_get_delegated_key(uint8_t   ecc_curve,
				       uint32_t  key_bits,
				       uint8_t  *key_buf,
				       size_t    key_buf_size,
				       size_t   *key_size,
				       uint32_t  hash_algo)
{
	return rss_delegated_attest_get_delegated_key(ecc_curve,
						      key_bits,
						      key_buf,
						      key_buf_size,
						      key_size,
						      hash_algo);
}

int tfm_log_printf(const char *fmt, ...)
{
	int count;
	va_list ap;

	va_start(ap, fmt);
	count = vprintf(fmt, ap);
	va_end(ap);

	return count;
}

void printf_set_color(int color_id)
{
	(void)color_id;
}

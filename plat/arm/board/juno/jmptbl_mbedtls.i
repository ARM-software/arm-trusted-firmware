#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Platform-specific ROMLIB MbedTLS functions can be added here.
# During the build process, this file is appended to jmptbl.i
# if MbedTLS support is required.
#
# Format:
# lib   function        [patch]
# Example:
# mbedtls mbedtls_asn1_get_alg
# mbedtls mbedtls_asn1_get_alg_null patch

mbedtls mbedtls_asn1_get_alg
mbedtls mbedtls_asn1_get_alg_null
mbedtls mbedtls_asn1_get_bitstring_null
mbedtls mbedtls_asn1_get_bool
mbedtls mbedtls_asn1_get_int
mbedtls mbedtls_asn1_get_len
mbedtls mbedtls_asn1_get_tag
mbedtls mbedtls_free
mbedtls mbedtls_md
mbedtls mbedtls_md_get_size
mbedtls mbedtls_memory_buffer_alloc_init
mbedtls mbedtls_oid_get_md_alg
mbedtls mbedtls_oid_get_numeric_string
mbedtls mbedtls_oid_get_pk_alg
mbedtls mbedtls_oid_get_sig_alg
mbedtls mbedtls_pk_free
mbedtls mbedtls_pk_init
mbedtls mbedtls_pk_parse_subpubkey
mbedtls mbedtls_pk_verify_ext
mbedtls mbedtls_platform_set_snprintf
mbedtls mbedtls_x509_get_rsassa_pss_params
mbedtls mbedtls_x509_get_sig_alg
mbedtls mbedtls_md_info_from_type

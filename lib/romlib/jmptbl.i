#
# Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Format:
# lib	function	[patch]
# Add "patch" at the end of the line to patch a function. For example:
# mbedtls	mbedtls_memory_buffer_alloc_init	patch
# Holes can be introduced in the table by using a special keyword "reserved".
# Example:
# reserved	reserved
# The jump table will contain an invalid instruction instead of branch

rom	rom_lib_init
fdt	fdt_getprop_namelen
fdt	fdt_setprop_inplace
fdt	fdt_check_header
fdt	fdt_node_offset_by_compatible
fdt     fdt_setprop_inplace_namelen_partial
mbedtls	mbedtls_asn1_get_alg
mbedtls	mbedtls_asn1_get_alg_null
mbedtls	mbedtls_asn1_get_bitstring_null
mbedtls	mbedtls_asn1_get_bool
mbedtls	mbedtls_asn1_get_int
mbedtls	mbedtls_asn1_get_tag
mbedtls	mbedtls_free
mbedtls	mbedtls_md
mbedtls	mbedtls_md_get_size
mbedtls	mbedtls_memory_buffer_alloc_init
mbedtls	mbedtls_oid_get_md_alg
mbedtls	mbedtls_oid_get_numeric_string
mbedtls	mbedtls_oid_get_pk_alg
mbedtls	mbedtls_oid_get_sig_alg
mbedtls	mbedtls_pk_free
mbedtls	mbedtls_pk_init
mbedtls	mbedtls_pk_parse_subpubkey
mbedtls	mbedtls_pk_verify_ext
mbedtls	mbedtls_platform_set_snprintf
mbedtls	mbedtls_x509_get_rsassa_pss_params
mbedtls	mbedtls_x509_get_sig_alg
mbedtls	mbedtls_md_info_from_type
c	exit
c	atexit

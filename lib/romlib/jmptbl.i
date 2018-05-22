#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

0	rom	rom_lib_init
1	fdt	fdt_getprop_namelen
2	fdt	fdt_setprop_inplace
3	fdt	fdt_check_header
4	fdt	fdt_node_offset_by_compatible
5	mbedtls	mbedtls_asn1_get_alg
6	mbedtls	mbedtls_asn1_get_alg_null
7	mbedtls	mbedtls_asn1_get_bitstring_null
8	mbedtls	mbedtls_asn1_get_bool
9	mbedtls	mbedtls_asn1_get_int
10	mbedtls	mbedtls_asn1_get_tag
11	mbedtls	mbedtls_free
12	mbedtls	mbedtls_md
13	mbedtls	mbedtls_md_get_size
14	mbedtls	mbedtls_memory_buffer_alloc_init
15	mbedtls	mbedtls_oid_get_md_alg
16	mbedtls	mbedtls_oid_get_numeric_string
17	mbedtls	mbedtls_oid_get_pk_alg
18	mbedtls	mbedtls_oid_get_sig_alg
19	mbedtls	mbedtls_pk_free
20	mbedtls	mbedtls_pk_init
21	mbedtls	mbedtls_pk_parse_subpubkey
22	mbedtls	mbedtls_pk_verify_ext
23	mbedtls	mbedtls_platform_set_snprintf
24	mbedtls	mbedtls_x509_get_rsassa_pss_params
25	mbedtls	mbedtls_x509_get_sig_alg
26	mbedtls	mbedtls_md_info_from_type
27	c	exit
28	c	atexit

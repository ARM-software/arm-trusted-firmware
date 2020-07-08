#
# Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Platform specific romlib functions can be added or included here.
# The index in the output file will be generated cumulatively in the same
# order as it is given in this file.
# Output file can be found at: $BUILD_DIR/jmptbl.i
#
# Format:
# lib	function	[patch]
# Example:
# rom	rom_lib_init
# fdt	fdt_getprop_namelen	patch

rom     rom_lib_init
fdt     fdt_getprop
fdt     fdt_get_property
fdt     fdt_getprop_namelen
fdt     fdt_setprop_inplace
fdt     fdt_check_header
fdt     fdt_node_offset_by_compatible
fdt     fdt_setprop_inplace_namelen_partial
fdt     fdt_first_subnode
fdt     fdt_next_subnode
fdt     fdt_parent_offset
fdt     fdt_stringlist_search
fdt     fdt_get_alias_namelen
fdt     fdt_path_offset
fdt     fdt_path_offset_namelen
fdt     fdt_address_cells
fdt     fdt_size_cells
fdt     fdt_get_name
fdt     fdt_get_alias
fdt     fdt_node_offset_by_phandle
mbedtls mbedtls_asn1_get_alg
mbedtls mbedtls_asn1_get_alg_null
mbedtls mbedtls_asn1_get_bitstring_null
mbedtls mbedtls_asn1_get_bool
mbedtls mbedtls_asn1_get_int
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
c       exit
c       atexit

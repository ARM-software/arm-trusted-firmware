#
# Copyright (c) 2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Platform-specific ROMLIB MbedTLS PSA Crypto functions can be added here.
# During the build process, this file is appended to jmptbl.i
# if MbedTLS support is required and PSA Crypto is supported.
#
# Format:
# lib   function        [patch]
# Example:
# mbedtls psa_crypto_init

mbedtls mbedtls_pk_get_psa_attributes
mbedtls mbedtls_pk_import_into_psa
mbedtls psa_crypto_init
mbedtls psa_destroy_key
mbedtls psa_verify_message

#
# Copyright (c) 2015-2026, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include drivers/auth/mbedtls/mbedtls_common.mk

ifeq (${PSA_CRYPTO},1)
	MBEDTLS_SOURCES	+=	drivers/auth/mbedtls/mbedtls_psa_x509_parser.c
else
	MBEDTLS_SOURCES	+=	drivers/auth/mbedtls/mbedtls_x509_parser.c
endif

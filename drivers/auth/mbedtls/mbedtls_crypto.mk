#
# Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include drivers/auth/mbedtls/mbedtls_common.mk

ifeq (${PSA_CRYPTO},1)
	# Some of the PSA functions are declared in multiple header files
	# that triggers this warning.
	TF_CFLAGS      	+=     	-Wno-error=redundant-decls
	MBEDTLS_SOURCES +=	drivers/auth/mbedtls/mbedtls_psa_crypto.c
else
	MBEDTLS_SOURCES +=	drivers/auth/mbedtls/mbedtls_crypto.c
endif

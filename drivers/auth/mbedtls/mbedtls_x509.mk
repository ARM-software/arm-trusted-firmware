#
# Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include drivers/auth/mbedtls/mbedtls_common.mk

MBEDTLS_X509_SOURCES	:=	drivers/auth/mbedtls/mbedtls_x509_parser.c	\
				$(addprefix ${MBEDTLS_DIR}/library/,		\
				x509.c 						\
				x509_crt.c 					\
				)

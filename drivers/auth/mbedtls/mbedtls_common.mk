#
# Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifneq (${MBEDTLS_COMMON_MK},1)
MBEDTLS_COMMON_MK	:=	1

# MBEDTLS_DIR must be set to the mbed TLS main directory (it must contain
# the 'include' and 'library' subdirectories).
ifeq (${MBEDTLS_DIR},)
  $(error Error: MBEDTLS_DIR not set)
endif

INCLUDES		+=	-I${MBEDTLS_DIR}/include		\
				-Iinclude/drivers/auth/mbedtls

# Specify mbed TLS configuration file
MBEDTLS_CONFIG_FILE	:=	"<mbedtls_config.h>"
$(eval $(call add_define,MBEDTLS_CONFIG_FILE))

MBEDTLS_COMMON_SOURCES	:=	drivers/auth/mbedtls/mbedtls_common.c	\
				$(addprefix ${MBEDTLS_DIR}/library/,	\
				asn1parse.c 				\
				asn1write.c 				\
				memory_buffer_alloc.c			\
				oid.c 					\
				platform.c 				\
				)

endif

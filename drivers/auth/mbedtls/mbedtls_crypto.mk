#
# Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include drivers/auth/mbedtls/mbedtls_common.mk

# The platform may define the variable 'MBEDTLS_KEY_ALG' to select the key
# algorithm to use. Default algorithm is RSA.
ifeq (${MBEDTLS_KEY_ALG},)
    MBEDTLS_KEY_ALG		:=	rsa
endif

MBEDTLS_CRYPTO_SOURCES		:=	drivers/auth/mbedtls/mbedtls_crypto.c	\
					$(addprefix ${MBEDTLS_DIR}/library/,	\
					bignum.c				\
					md.c					\
					md_wrap.c				\
					pk.c 					\
					pk_wrap.c 				\
					pkparse.c 				\
					pkwrite.c 				\
					sha256.c				\
					)

# Key algorithm specific files
ifeq (${MBEDTLS_KEY_ALG},ecdsa)
    MBEDTLS_CRYPTO_SOURCES	+=	$(addprefix ${MBEDTLS_DIR}/library/,	\
    					ecdsa.c					\
    					ecp_curves.c				\
    					ecp.c					\
    					)
    TBBR_KEY_ALG_ID		:=	TBBR_ECDSA
else ifeq (${MBEDTLS_KEY_ALG},rsa)
    MBEDTLS_CRYPTO_SOURCES	+=	$(addprefix ${MBEDTLS_DIR}/library/,	\
    					rsa.c					\
    					)
    TBBR_KEY_ALG_ID		:=	TBBR_RSA
else
    $(error "MBEDTLS_KEY_ALG=${MBEDTLS_KEY_ALG} not supported on mbed TLS")
endif

# Needs to be set to drive mbed TLS configuration correctly
$(eval $(call add_define,TBBR_KEY_ALG_ID))

BL1_SOURCES			+=	${MBEDTLS_CRYPTO_SOURCES}
BL2_SOURCES			+=	${MBEDTLS_CRYPTO_SOURCES}

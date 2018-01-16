#
# Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include drivers/auth/mbedtls/mbedtls_common.mk

# The platform may define the variable 'TF_MBEDTLS_KEY_ALG' to select the key
# algorithm to use. If the variable is not defined, select it based on algorithm
# used for key generation `KEY_ALG`. If `KEY_ALG` is not defined or is
# defined to `rsa`/`rsa_1_5`, then set the variable to `rsa`.
ifeq (${TF_MBEDTLS_KEY_ALG},)
    ifeq (${KEY_ALG}, ecdsa)
        TF_MBEDTLS_KEY_ALG		:=	ecdsa
    else
        TF_MBEDTLS_KEY_ALG		:=	rsa
    endif
endif

# If MBEDTLS_KEY_ALG build flag is defined use it to set TF_MBEDTLS_KEY_ALG for
# backward compatibility
ifdef MBEDTLS_KEY_ALG
    ifeq (${ERROR_DEPRECATED},1)
        $(error "MBEDTLS_KEY_ALG is deprecated. Please use the new build flag TF_MBEDTLS_KEY_ALG")
    endif
    $(warning "MBEDTLS_KEY_ALG is deprecated. Please use the new build flag TF_MBEDTLS_KEY_ALG")
    TF_MBEDTLS_KEY_ALG	:= ${MBEDTLS_KEY_ALG}
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
					)

ifeq (${HASH_ALG}, sha384)
    MBEDTLS_CRYPTO_SOURCES  += \
					$(addprefix ${MBEDTLS_DIR}/library/,	\
						sha256.c            \
						sha512.c            \
					)
    TF_MBEDTLS_HASH_ALG_ID	:=	TF_MBEDTLS_SHA384
else ifeq (${HASH_ALG}, sha512)
    MBEDTLS_CRYPTO_SOURCES  += \
					$(addprefix ${MBEDTLS_DIR}/library/,	\
						sha256.c            \
						sha512.c            \
					)
    TF_MBEDTLS_HASH_ALG_ID	:=	TF_MBEDTLS_SHA512
else
    MBEDTLS_CRYPTO_SOURCES  += \
					$(addprefix ${MBEDTLS_DIR}/library/,	\
						sha256.c            \
					)
    TF_MBEDTLS_HASH_ALG_ID	:=	TF_MBEDTLS_SHA256
endif

# Key algorithm specific files
MBEDTLS_ECDSA_CRYPTO_SOURCES	+=	$(addprefix ${MBEDTLS_DIR}/library/,	\
					ecdsa.c					\
					ecp_curves.c				\
					ecp.c					\
					)

MBEDTLS_RSA_CRYPTO_SOURCES	+=	$(addprefix ${MBEDTLS_DIR}/library/,	\
					rsa.c					\
					)

ifeq (${TF_MBEDTLS_KEY_ALG},ecdsa)
    MBEDTLS_CRYPTO_SOURCES	+=	$(MBEDTLS_ECDSA_CRYPTO_SOURCES)
    TF_MBEDTLS_KEY_ALG_ID	:=	TF_MBEDTLS_ECDSA
else ifeq (${TF_MBEDTLS_KEY_ALG},rsa)
    MBEDTLS_CRYPTO_SOURCES	+=	$(MBEDTLS_RSA_CRYPTO_SOURCES)
    TF_MBEDTLS_KEY_ALG_ID	:=	TF_MBEDTLS_RSA
else ifeq (${TF_MBEDTLS_KEY_ALG},rsa+ecdsa)
    MBEDTLS_CRYPTO_SOURCES	+=	$(MBEDTLS_ECDSA_CRYPTO_SOURCES)
    MBEDTLS_CRYPTO_SOURCES	+=	$(MBEDTLS_RSA_CRYPTO_SOURCES)
    TF_MBEDTLS_KEY_ALG_ID	:=	TF_MBEDTLS_RSA_AND_ECDSA
else
    $(error "TF_MBEDTLS_KEY_ALG=${TF_MBEDTLS_KEY_ALG} not supported on mbed TLS")
endif

# Needs to be set to drive mbed TLS configuration correctly
$(eval $(call add_define,TF_MBEDTLS_KEY_ALG_ID))
$(eval $(call add_define,TF_MBEDTLS_HASH_ALG_ID))

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

MBEDTLS_SOURCES	+=		drivers/auth/mbedtls/mbedtls_common.c


LIBMBEDTLS_SRCS		:= $(addprefix ${MBEDTLS_DIR}/library/,	\
					asn1parse.c 				\
					asn1write.c 				\
					memory_buffer_alloc.c			\
					oid.c 					\
					platform.c 				\
					platform_util.c				\
					bignum.c				\
					md.c					\
					md_wrap.c				\
					pk.c 					\
					pk_wrap.c 				\
					pkparse.c 				\
					pkwrite.c 				\
					sha256.c            			\
					sha512.c            			\
					ecdsa.c					\
					ecp_curves.c				\
					ecp.c					\
					rsa.c					\
					rsa_internal.c				\
					x509.c 					\
					x509_crt.c 				\
					)

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

ifeq (${HASH_ALG}, sha384)
    TF_MBEDTLS_HASH_ALG_ID	:=	TF_MBEDTLS_SHA384
else ifeq (${HASH_ALG}, sha512)
   TF_MBEDTLS_HASH_ALG_ID	:=	TF_MBEDTLS_SHA512
else
    TF_MBEDTLS_HASH_ALG_ID	:=	TF_MBEDTLS_SHA256
endif

ifeq (${TF_MBEDTLS_KEY_ALG},ecdsa)
    TF_MBEDTLS_KEY_ALG_ID	:=	TF_MBEDTLS_ECDSA
else ifeq (${TF_MBEDTLS_KEY_ALG},rsa)
    TF_MBEDTLS_KEY_ALG_ID	:=	TF_MBEDTLS_RSA
else ifeq (${TF_MBEDTLS_KEY_ALG},rsa+ecdsa)
    TF_MBEDTLS_KEY_ALG_ID	:=	TF_MBEDTLS_RSA_AND_ECDSA
else
    $(error "TF_MBEDTLS_KEY_ALG=${TF_MBEDTLS_KEY_ALG} not supported on mbed TLS")
endif

# Needs to be set to drive mbed TLS configuration correctly
$(eval $(call add_define,TF_MBEDTLS_KEY_ALG_ID))
$(eval $(call add_define,TF_MBEDTLS_HASH_ALG_ID))


$(eval $(call MAKE_LIB,mbedtls))

endif

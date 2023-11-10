#
# Copyright (c) 2015-2023, Arm Limited. All rights reserved.
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

MBEDTLS_INC		=	-I${MBEDTLS_DIR}/include

MBEDTLS_MAJOR=$(shell grep -hP "define MBEDTLS_VERSION_MAJOR" ${MBEDTLS_DIR}/include/mbedtls/*.h | grep -oe '\([0-9.]*\)')
MBEDTLS_MINOR=$(shell grep -hP "define MBEDTLS_VERSION_MINOR" ${MBEDTLS_DIR}/include/mbedtls/*.h | grep -oe '\([0-9.]*\)')
$(info MBEDTLS_VERSION_MAJOR is [${MBEDTLS_MAJOR}] MBEDTLS_VERSION_MINOR is [${MBEDTLS_MINOR}])

# Specify mbed TLS configuration file
ifeq (${MBEDTLS_MAJOR}, 2)
        $(info Deprecation Notice: Please migrate to Mbedtls version 3.x (refer to TF-A documentation for the exact version number))
	MBEDTLS_CONFIG_FILE             ?=	"<drivers/auth/mbedtls/mbedtls_config-2.h>"
else ifeq (${MBEDTLS_MAJOR}, 3)
	ifeq (${PSA_CRYPTO},1)
		MBEDTLS_CONFIG_FILE     ?=      "<drivers/auth/mbedtls/psa_mbedtls_config.h>"
	else
		MBEDTLS_CONFIG_FILE	?=	"<drivers/auth/mbedtls/mbedtls_config-3.h>"
	endif
endif

$(eval $(call add_define,MBEDTLS_CONFIG_FILE))

MBEDTLS_SOURCES	+=		drivers/auth/mbedtls/mbedtls_common.c

LIBMBEDTLS_SRCS		+= $(addprefix ${MBEDTLS_DIR}/library/,		\
					aes.c 				\
					asn1parse.c 			\
					asn1write.c 			\
					cipher.c 			\
					cipher_wrap.c 			\
					constant_time.c			\
					memory_buffer_alloc.c		\
					oid.c 				\
					platform.c 			\
					platform_util.c			\
					bignum.c			\
					gcm.c 				\
					md.c				\
					pk.c 				\
					pk_wrap.c 			\
					pkparse.c 			\
					pkwrite.c 			\
					sha256.c            		\
					sha512.c            		\
					ecdsa.c				\
					ecp_curves.c			\
					ecp.c				\
					rsa.c				\
					x509.c 				\
					x509_crt.c 			\
					)

ifeq (${MBEDTLS_MAJOR}, 2)
	LIBMBEDTLS_SRCS +=  $(addprefix ${MBEDTLS_DIR}/library/,	\
						rsa_internal.c		\
						)
else ifeq (${MBEDTLS_MAJOR}, 3)
	LIBMBEDTLS_SRCS +=  $(addprefix ${MBEDTLS_DIR}/library/,	\
						bignum_core.c		\
						rsa_alt_helpers.c	\
						hash_info.c		\
						)

	# Currently on Mbedtls-3 there is outstanding bug due to usage
	# of redundant declaration[1], So disable redundant-decls
	# compilation flag to avoid compilation error when compiling with
	# Mbedtls-3.
	# [1]: https://github.com/Mbed-TLS/mbedtls/issues/6910
	LIBMBEDTLS_CFLAGS += -Wno-error=redundant-decls
endif

ifeq (${PSA_CRYPTO},1)
LIBMBEDTLS_SRCS         += $(addprefix ${MBEDTLS_DIR}/library/,    	\
					psa_crypto.c                   	\
					psa_crypto_client.c            	\
					psa_crypto_driver_wrappers.c   	\
					psa_crypto_hash.c              	\
					psa_crypto_rsa.c               	\
					psa_crypto_ecp.c               	\
					psa_crypto_slot_management.c   	\
					)
endif

# The platform may define the variable 'TF_MBEDTLS_KEY_ALG' to select the key
# algorithm to use. If the variable is not defined, select it based on
# algorithm used for key generation `KEY_ALG`. If `KEY_ALG` is not defined,
# then it is set to `rsa`.
ifeq (${TF_MBEDTLS_KEY_ALG},)
    ifeq (${KEY_ALG}, ecdsa)
        TF_MBEDTLS_KEY_ALG		:=	ecdsa
    else
        TF_MBEDTLS_KEY_ALG		:=	rsa
    endif
endif

ifeq (${TF_MBEDTLS_KEY_SIZE},)
    ifneq ($(findstring rsa,${TF_MBEDTLS_KEY_ALG}),)
        ifeq (${KEY_SIZE},)
            TF_MBEDTLS_KEY_SIZE		:=	2048
        else ifneq ($(filter $(KEY_SIZE), 1024 2048 3072 4096),)
            TF_MBEDTLS_KEY_SIZE		:=	${KEY_SIZE}
        else
            $(error "Invalid value for KEY_SIZE: ${KEY_SIZE}")
        endif
    else ifneq ($(findstring ecdsa,${TF_MBEDTLS_KEY_ALG}),)
        ifeq (${KEY_SIZE},)
            TF_MBEDTLS_KEY_SIZE		:=	256
        else ifneq ($(filter $(KEY_SIZE), 256 384),)
            TF_MBEDTLS_KEY_SIZE		:=	${KEY_SIZE}
        else
            $(error "Invalid value for KEY_SIZE: ${KEY_SIZE}")
        endif
    endif
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

ifeq (${DECRYPTION_SUPPORT}, aes_gcm)
    TF_MBEDTLS_USE_AES_GCM	:=	1
else
    TF_MBEDTLS_USE_AES_GCM	:=	0
endif

# Needs to be set to drive mbed TLS configuration correctly
$(eval $(call add_defines,\
    $(sort \
        TF_MBEDTLS_KEY_ALG_ID \
        TF_MBEDTLS_KEY_SIZE \
        TF_MBEDTLS_HASH_ALG_ID \
        TF_MBEDTLS_USE_AES_GCM \
)))

$(eval $(call MAKE_LIB,mbedtls))

endif

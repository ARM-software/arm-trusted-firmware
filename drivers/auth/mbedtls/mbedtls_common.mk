#
# Copyright (c) 2015-2026, Arm Limited. All rights reserved.
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

MBEDTLS_NAME		:= mbedtls

MBEDTLS_BUILD_DIR	:= $(BUILD_PLAT)/lib$(MBEDTLS_NAME)
MBEDTLS_INSTALL_DIR	:= $(BUILD_PLAT)/$(MBEDTLS_NAME)-install
MBEDTLS_TARGETS		:= $(MBEDTLS_INSTALL_DIR)/lib/libmbedx509.a \
                           $(MBEDTLS_INSTALL_DIR)/lib/libmbedcrypto.a

MBEDTLS_BUILD_TYPE	:= MinSizeRel

MBEDTLS_LIBS		:= $(MBEDTLS_TARGETS)
MBEDTLS_INCLUDE_DIRS	:= $(MBEDTLS_INSTALL_DIR)/include

MBEDTLS_MAJOR=$(shell grep -hP "define MBEDTLS_VERSION_MAJOR" ${MBEDTLS_DIR}/include/mbedtls/*.h | grep -oe '\([0-9.]*\)')
MBEDTLS_MINOR=$(shell grep -hP "define MBEDTLS_VERSION_MINOR" ${MBEDTLS_DIR}/include/mbedtls/*.h | grep -oe '\([0-9.]*\)')
$(info MBEDTLS_VERSION_MAJOR is [${MBEDTLS_MAJOR}] MBEDTLS_VERSION_MINOR is [${MBEDTLS_MINOR}])

ifneq (${MBEDTLS_MAJOR}, 3)
  $(error Error: TF-A only supports MbedTLS versions > 3.x)
endif

# Specify mbed TLS configuration file
ifeq (${PSA_CRYPTO},1)
  MBEDTLS_CONFIG_FILE    ?=    "<drivers/auth/mbedtls/default_psa_mbedtls_config.h>"
else
  MBEDTLS_CONFIG_FILE    ?=    "<drivers/auth/mbedtls/default_mbedtls_config.h>"
endif

$(eval $(call add_define,MBEDTLS_CONFIG_FILE))

MBEDTLS_SOURCES	+=		drivers/auth/mbedtls/mbedtls_common.c

LIBMBEDTLS_CFLAGS ?= $(filter-out -I%,$(TF_CFLAGS))
LIBMBEDTLS_CFLAGS += $(patsubst %,-I%,$(call include-dirs,$(TF_CFLAGS)))

# This is a temporary workaround due to changes in the locations of helper
# function declarations in Mbed-TLS version 3.6.4
# TODO: remove this once the related Mbedt-TLS issue is resolved
LIBMBEDTLS_CFLAGS	+=	-Wno-error=redundant-decls

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

ifeq (${MBOOT_EL_HASH_ALG}, sha256)
    $(eval $(call add_define,TF_MBEDTLS_MBOOT_USE_SHA256))
else ifeq (${MBOOT_EL_HASH_ALG}, sha384)
    $(eval $(call add_define,TF_MBEDTLS_MBOOT_USE_SHA384))
else ifeq (${MBOOT_EL_HASH_ALG}, sha512)
    $(eval $(call add_define,TF_MBEDTLS_MBOOT_USE_SHA512))
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

TF_MBEDTLS_USE_AES_CCM	:=	0
TF_MBEDTLS_USE_AES_GCM	:=	0

ifeq (${DECRYPTION_SUPPORT}, aes_ccm)
    TF_MBEDTLS_USE_AES_CCM	:=	1
else ifeq (${DECRYPTION_SUPPORT}, aes_gcm)
    TF_MBEDTLS_USE_AES_GCM	:=	1
endif

# Needs to be set to drive mbed TLS configuration correctly

MBEDTLS_DEFINES := \
        -DTF_MBEDTLS_KEY_ALG_ID=$(TF_MBEDTLS_KEY_ALG_ID) \
        -DTF_MBEDTLS_KEY_SIZE=$(TF_MBEDTLS_KEY_SIZE) \
        -DTF_MBEDTLS_HASH_ALG_ID=$(TF_MBEDTLS_HASH_ALG_ID) \
        -DTF_MBEDTLS_USE_AES_CCM=$(TF_MBEDTLS_USE_AES_CCM) \
        -DTF_MBEDTLS_USE_AES_GCM=$(TF_MBEDTLS_USE_AES_GCM)

$(call add_defines,\
    $(sort \
        TF_MBEDTLS_KEY_ALG_ID \
        TF_MBEDTLS_KEY_SIZE \
        TF_MBEDTLS_HASH_ALG_ID \
        TF_MBEDTLS_USE_AES_CCM \
        TF_MBEDTLS_USE_AES_GCM \
))

LIBMBEDTLS_CFLAGS += $(MBEDTLS_DEFINES)

ifeq ($(filter 1,$(ENABLE_FEAT_CRYPTO)),1)
    REMOVED_CFLAGS		:=	-nostdinc -mgeneral-regs-only
endif #(ENABLE_FEAT_CRYPTO)

FILTERED_LIBMBEDTLS_CFLAGS = $(filter-out $(REMOVED_CFLAGS),$(LIBMBEDTLS_CFLAGS))

$(MBEDTLS_INSTALL_DIR)/% $(MBEDTLS_INSTALL_DIR)/%/: $(MBEDTLS_TARGETS) ;
$(MBEDTLS_TARGETS) $(MBEDTLS_INSTALL_DIR)/ &: $(BUILD_PLAT)/lib/libc.a
	$(s)echo "  CM      $@"
	$(q)cmake -S $(MBEDTLS_DIR) -B $(MBEDTLS_BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=$(MBEDTLS_BUILD_TYPE) \
		-DCMAKE_SYSTEM_NAME=Generic \
		-DCMAKE_SYSTEM_VERSION= \
		-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
		-DCMAKE_C_COMPILER=$(call shell-quote,$(call shell-join,$($(ARCH)-cc-program),;)) \
		$(if $($(ARCH)-cc-wrapper),-DCMAKE_C_COMPILER_LAUNCHER=$(call shell-quote,$(call shell-join,$($(ARCH)-cc-wrapper),;))) \
		-DCMAKE_C_FLAGS=$(call escape-shell,$(FILTERED_LIBMBEDTLS_CFLAGS)) \
		-DENABLE_TESTING=OFF \
		-DENABLE_PROGRAMS=OFF \
		$(if $(V),, --log-level=ERROR) > /dev/null
	$(q)cmake --build $(MBEDTLS_BUILD_DIR) -- $(if $(V),,-s) > /dev/null
	$(q)cmake --install $(MBEDTLS_BUILD_DIR) \
		--prefix $(MBEDTLS_INSTALL_DIR) \
		--config $(MBEDTLS_BUILD_TYPE) > /dev/null
endif

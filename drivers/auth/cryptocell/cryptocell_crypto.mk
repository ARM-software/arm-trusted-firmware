#
# Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include drivers/auth/mbedtls/mbedtls_common.mk

# The algorithm is RSA when using Cryptocell crypto driver
TF_MBEDTLS_KEY_ALG_ID		:=	TF_MBEDTLS_RSA

# Needs to be set to drive mbed TLS configuration correctly
$(eval $(call add_define,TF_MBEDTLS_KEY_ALG_ID))

$(eval $(call add_define,KEY_SIZE))

# CCSBROM_LIB_PATH must be set to the Cryptocell SBROM library path
ifeq (${CCSBROM_LIB_PATH},)
  $(error Error: CCSBROM_LIB_PATH not set)
endif

CRYPTOCELL_VERSION ?= 712
ifeq (${CRYPTOCELL_VERSION},712)
  CCSBROM_LIB_FILENAME := cc_712sbromx509
else
  $(error Error: CRYPTOCELL_VERSION set to invalid version)
endif

CRYPTOCELL_SRC_DIR	:= drivers/auth/cryptocell/${CRYPTOCELL_VERSION}/

CRYPTOCELL_SOURCES	:= ${CRYPTOCELL_SRC_DIR}/cryptocell_crypto.c \
			   ${CRYPTOCELL_SRC_DIR}/cryptocell_plat_helpers.c

TF_LDFLAGS		+= -L$(CCSBROM_LIB_PATH)
LDLIBS			+= -l$(CCSBROM_LIB_FILENAME)

BL1_SOURCES		+= ${CRYPTOCELL_SOURCES}
BL2_SOURCES		+= ${CRYPTOCELL_SOURCES}

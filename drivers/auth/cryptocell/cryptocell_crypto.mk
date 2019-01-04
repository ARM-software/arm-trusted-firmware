#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include drivers/auth/mbedtls/mbedtls_common.mk

# The algorithm is RSA when using Cryptocell crypto driver
TF_MBEDTLS_KEY_ALG_ID		:=	TF_MBEDTLS_RSA

# Needs to be set to drive mbed TLS configuration correctly
$(eval $(call add_define,TF_MBEDTLS_KEY_ALG_ID))

# CCSBROM_LIB_PATH must be set to the Cryptocell SBROM library path
ifeq (${CCSBROM_LIB_PATH},)
  $(error Error: CCSBROM_LIB_PATH not set)
endif

TF_LDFLAGS		+= -L$(CCSBROM_LIB_PATH)
LDLIBS			+= -lcc_712sbromx509

CRYPTOCELL_SOURCES	:=	drivers/auth/cryptocell/cryptocell_crypto.c

BL1_SOURCES		+=	${CRYPTOCELL_SOURCES}
BL2_SOURCES		+=	${CRYPTOCELL_SOURCES}

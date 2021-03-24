#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

ifeq (${ADD_CAAM},)

ADD_CAAM		:= 1
CAAM_DRIVER_PATH	:= drivers/nxp/crypto/caam

CAAM_DRIVER_SOURCES	+=  $(wildcard $(CAAM_DRIVER_PATH)/src/*.c)

PLAT_INCLUDES		+= -I$(CAAM_DRIVER_PATH)/include

ifeq (${BL_COMM_CRYPTO_NEEDED},yes)
BL_COMMON_SOURCES	+= ${CAAM_DRIVER_SOURCES}
else
ifeq (${BL2_CRYPTO_NEEDED},yes)
BL2_SOURCES		+= ${CAAM_DRIVER_SOURCES}
endif
ifeq (${BL31_CRYPTO_NEEDED},yes)
BL31_SOURCES		+= ${CAAM_DRIVER_SOURCES}
endif
endif

endif

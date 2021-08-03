#
# Copyright 2020-2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

ifeq (${ADD_CAAM},)

ADD_CAAM		:= 1

CAAM_DRIVER_SOURCES	+=  $(wildcard $(PLAT_DRIVERS_PATH)/crypto/caam/src/*.c)

PLAT_INCLUDES		+= -I$(PLAT_DRIVERS_INCLUDE_PATH)/crypto/caam

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

#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

SEC_DRIVERS_PATH	:=	drivers/nxp/crypto/caam

ifeq (${TRUSTED_BOARD_BOOT},1)
AUTH_SOURCES +=  $(wildcard $(SEC_DRIVERS_PATH)/src/auth/*.c)
endif

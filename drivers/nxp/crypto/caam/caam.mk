#
# Copyright 2018-2019 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

SEC_DRIVERS_PATH	:=	drivers/nxp/crypto/caam

SEC_SOURCES +=  $(wildcard $(SEC_DRIVERS_PATH)/src/*.c)

PLAT_INCLUDES			+= -I$(SEC_DRIVERS_PATH)/include

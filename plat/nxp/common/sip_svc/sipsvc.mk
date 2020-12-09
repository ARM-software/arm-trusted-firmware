#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#
#------------------------------------------------------------------------------
#
# Select the SIP SVC files
#
# -----------------------------------------------------------------------------

ifeq (${ADD_SIPSVC},)

ADD_SIPSVC		:= 1

PLAT_SIPSVC_PATH	:= $(PLAT_COMMON_PATH)/sip_svc

SIPSVC_SOURCES		:= ${PLAT_SIPSVC_PATH}/sip_svc.c \
			   ${PLAT_SIPSVC_PATH}/$(ARCH)/sipsvc.S

PLAT_INCLUDES		+=	-I${PLAT_SIPSVC_PATH}/include

ifeq (${BL_COMM_SIPSVC_NEEDED},yes)
BL_COMMON_SOURCES	+= ${SIPSVC_SOURCES}
else
ifeq (${BL2_SIPSVC_NEEDED},yes)
BL2_SOURCES		+= ${SIPSVC_SOURCES}
endif
ifeq (${BL31_SIPSVC_NEEDED},yes)
BL31_SOURCES		+= ${SIPSVC_SOURCES}
endif
endif
endif
# -----------------------------------------------------------------------------

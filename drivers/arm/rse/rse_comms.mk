#
# Copyright (c) 2022-2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

$(warning "RSE driver is an experimental feature")

RSE_COMMS_SOURCES	:=	$(addprefix drivers/arm/rse/,			\
					rse_comms.c				\
					rse_comms_protocol.c			\
					rse_comms_protocol_embed.c		\
					rse_comms_protocol_pointer_access.c	\
				)

# Default to MHUv2 if PLAT_MHU_VERSION undefined
PLAT_MHU_VERSION ?= 2

ifeq (${PLAT_MHU_VERSION}, 3)
RSE_COMMS_SOURCES	+=	$(addprefix drivers/arm/mhu/,			\
					mhu_v3_x.c				\
					mhu_wrapper_v3_x.c			\
				)
else ifeq (${PLAT_MHU_VERSION}, 2)
RSE_COMMS_SOURCES	+=	$(addprefix drivers/arm/mhu/,			\
					mhu_v2_x.c				\
					mhu_wrapper_v2_x.c			\
				)
else
$(error Unsupported MHU version)
endif

PLAT_INCLUDES		+=	-Idrivers/arm/rse		\
				-Idrivers/arm/mhu		\
				-Iinclude/lib/psa

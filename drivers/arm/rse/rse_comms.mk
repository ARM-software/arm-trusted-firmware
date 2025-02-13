#
# Copyright (c) 2022-2025, Arm Limited and Contributors. All rights reserved.
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

# Default to MHUv2 if PLAT_MHU undefined
PLAT_MHU ?= MHUv2

ifneq (${PLAT_MHU}, NO_MHU)
ifeq (${PLAT_MHU}, MHUv3)
RSE_COMMS_SOURCES	+=	$(addprefix drivers/arm/mhu/,			\
					mhu_v3_x.c				\
					mhu_wrapper_v3_x.c			\
				)
else ifeq (${PLAT_MHU}, MHUv2)
RSE_COMMS_SOURCES	+=	$(addprefix drivers/arm/mhu/,			\
					mhu_v2_x.c				\
					mhu_wrapper_v2_x.c			\
				)
else
$(error Unsupported MHU version)
endif

RSE_COMMS_SOURCES	+=	$(addprefix drivers/arm/rse/,			\
					rse_comms_mhu.c				\
				)

PLAT_INCLUDES		+=	-Idrivers/arm/mhu
endif

PLAT_INCLUDES		+=	-Idrivers/arm/rse		\
				-Iinclude/lib/psa

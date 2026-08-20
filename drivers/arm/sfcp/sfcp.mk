#
# Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

$(warning "SFCP is an experimental feature")

SFCP_SOURCES	:=	$(addprefix drivers/arm/sfcp/sfcp_core/,			\
					sfcp_helpers.c				\
					sfcp.c			\
					sfcp_link_hal.c		\
					sfcp_random.c	\
					sfcp_encryption_stub.c	\
				)

SFCP_SOURCES += $(addprefix drivers/arm/sfcp/sfcp_psa/,			\
					sfcp_psa_call/sfcp_psa_call.c				\
					sfcp_psa_protocol/sfcp_psa_protocol.c	\
					sfcp_psa_protocol/sfcp_psa_protocol_pointer_access.c	\
					sfcp_psa_protocol/sfcp_psa_protocol_embed.c	\
				)

# Default to MHUv2 if PLAT_MHU undefined
PLAT_MHU ?= MHUv2

ifneq (${PLAT_MHU}, NO_MHU)
ifeq (${PLAT_MHU}, MHUv3)
SFCP_SOURCES	+=	$(addprefix drivers/arm/mhu/,			\
					mhu_v3_x.c				\
				)

$(call add_defines, \
	PLAT_MHU_VERSION=3	\
)

else ifeq (${PLAT_MHU}, MHUv2)
SFCP_SOURCES	+=	$(addprefix drivers/arm/mhu/,			\
					mhu_v2_x.c				\
				)

$(call add_defines, \
	PLAT_MHU_VERSION=2	\
)

else
$(error Unsupported MHU version)
endif

PLAT_INCLUDES		+=	-Idrivers/arm/mhu
endif

PLAT_INCLUDES		+=	-Iinclude/lib/psa		\
						-Idrivers/arm/sfcp/sfcp_core	\
						-Idrivers/arm/sfcp/sfcp_psa/sfcp_psa_protocol

$(call add_defines, \
	SFCP_PROTOCOL_EMBED_ENABLED	\
	SFCP_PROTOCOL_POINTER_ACCESS_ENABLED	\
)

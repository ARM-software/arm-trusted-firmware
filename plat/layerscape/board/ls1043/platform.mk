#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# indicate the reset vector address can be programmed
PROGRAMMABLE_RESET_ADDRESS	:=	1
USE_COHERENT_MEM		:=	0
RESET_TO_BL31			:=	0
ENABLE_STACK_PROTECTOR		:=	0
LS1043_GIC_SOURCES		:=	drivers/arm/gic/common/gic_common.c	\
					drivers/arm/gic/v2/gicv2_main.c		\
					drivers/arm/gic/v2/gicv2_helpers.c	\
					plat/common/plat_gicv2.c		\
					plat/layerscape/board/ls1043/ls_gic.c


LS1043_INTERCONNECT_SOURCES	:= 	drivers/arm/cci/cci.c

LS1043_SECURITY_SOURCES 	:=	plat/layerscape/common/ls_tzc380.c	\
					plat/layerscape/board/ls1043/ls1043_security.c

PLAT_INCLUDES			:=	-Iplat/layerscape/board/ls1043/include   \
					-Iplat/layerscape/common/include	\

PLAT_BL_COMMON_SOURCES		:=	plat/layerscape/common/aarch64/ls_console.S

LS1043_CPU_LIBS			:=	lib/cpus/${ARCH}/aem_generic.S

LS1043_CPU_LIBS			+=	lib/cpus/aarch64/cortex_a53.S

BL1_SOURCES			+= 	plat/layerscape/board/ls1043/ls1043_bl1_setup.c		\
					plat/layerscape/board/ls1043/ls1043_err.c			\
					drivers/delay_timer/delay_timer.c \

BL1_SOURCES     		+=	plat/layerscape/board/ls1043/${ARCH}/ls1043_helpers.S \
					${LS1043_CPU_LIBS}					\
					${LS1043_INTERCONNECT_SOURCES}		\
					$(LS1043_SECURITY_SOURCES)


BL2_SOURCES			+=	drivers/delay_timer/delay_timer.c		\
					plat/layerscape/board/ls1043/ls1043_bl2_setup.c		\
					plat/layerscape/board/ls1043/ls1043_err.c			\
					${LS1043_SECURITY_SOURCES}


BL31_SOURCES			+=	plat/layerscape/board/ls1043/ls1043_bl31_setup.c		\
					plat/layerscape/board/ls1043/ls1043_topology.c		\
					plat/layerscape/board/ls1043/aarch64/ls1043_helpers.S	\
					plat/layerscape/board/ls1043/ls1043_psci.c		\
					drivers/delay_timer/delay_timer.c		\
					${LS1043_CPU_LIBS}					\
					${LS1043_GIC_SOURCES}				\
					${LS1043_INTERCONNECT_SOURCES}			\
					${LS1043_SECURITY_SOURCES}

# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_855873		:=	1

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES		+=	plat/layerscape/board/ls1043/ls1043_stack_protector.c
endif

ifeq (${ARCH},aarch32)
    NEED_BL32 := yes
endif

include plat/layerscape/common/ls_common.mk

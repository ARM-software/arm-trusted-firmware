#
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Making sure the diphda platform type is specified
ifeq ($(filter ${TARGET_PLATFORM}, fpga fvp),)
	$(error TARGET_PLATFORM must be fpga or fvp)
endif

DIPHDA_CPU_LIBS	+=lib/cpus/aarch64/cortex_a35.S

PLAT_INCLUDES		:=	-Iplat/arm/board/diphda/common/include	\
				-Iplat/arm/board/diphda/include		\
				-Iinclude/plat/arm/common			\
				-Iinclude/plat/arm/css/common/aarch64


DIPHDA_FW_NVCTR_VAL	:=	255
TFW_NVCTR_VAL		:=	${DIPHDA_FW_NVCTR_VAL}
NTFW_NVCTR_VAL		:=	${DIPHDA_FW_NVCTR_VAL}

override NEED_BL1	:=	no

override NEED_BL2	:=	yes
FIP_BL2_ARGS := tb-fw

override NEED_BL2U	:=	no
override NEED_BL31	:=	yes
NEED_BL32		:=	yes
override NEED_BL33	:=	yes

# Include GICv2 driver files
include drivers/arm/gic/v2/gicv2.mk

DIPHDA_GIC_SOURCES	:=	${GICV2_SOURCES}			\
				plat/common/plat_gicv2.c		\
				plat/arm/common/arm_gicv2.c


BL2_SOURCES		+=	plat/arm/board/diphda/common/diphda_security.c		\
				plat/arm/board/diphda/common/diphda_err.c		\
				plat/arm/board/diphda/common/diphda_trusted_boot.c	\
				lib/utils/mem_region.c					\
				plat/arm/board/diphda/common/diphda_helpers.S		\
				plat/arm/board/diphda/common/diphda_plat.c		\
				plat/arm/board/diphda/common/diphda_bl2_mem_params_desc.c \
				${DIPHDA_CPU_LIBS}					\


BL31_SOURCES	+=	drivers/cfi/v2m/v2m_flash.c				\
			lib/utils/mem_region.c					\
			plat/arm/board/diphda/common/diphda_helpers.S		\
			plat/arm/board/diphda/common/diphda_topology.c		\
			plat/arm/board/diphda/common/diphda_security.c		\
			plat/arm/board/diphda/common/diphda_plat.c		\
			plat/arm/board/diphda/common/diphda_pm.c		\
			${DIPHDA_CPU_LIBS}					\
			${DIPHDA_GIC_SOURCES}

ifneq (${ENABLE_STACK_PROTECTOR},0)
	ifneq (${ENABLE_STACK_PROTECTOR},none)
		DIPHDA_SECURITY_SOURCES := plat/arm/board/diphda/common/diphda_stack_protector.c
		BL2_SOURCES += ${DIPHDA_SECURITY_SOURCES}
		BL31_SOURCES += ${DIPHDA_SECURITY_SOURCES}
	endif
endif

FDT_SOURCES		+=	plat/arm/board/diphda/common/fdts/diphda_spmc_manifest.dts
DIPHDA_TOS_FW_CONFIG	:=	${BUILD_PLAT}/fdts/diphda_spmc_manifest.dtb

# Add the SPMC manifest to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${DIPHDA_TOS_FW_CONFIG},--tos-fw-config,${DIPHDA_TOS_FW_CONFIG}))

# Adding TARGET_PLATFORM as a GCC define (-D option)
$(eval $(call add_define,TARGET_PLATFORM_$(call uppercase,${TARGET_PLATFORM})))

# Adding DIPHDA_FW_NVCTR_VAL as a GCC define (-D option)
$(eval $(call add_define,DIPHDA_FW_NVCTR_VAL))

include plat/arm/common/arm_common.mk
include plat/arm/board/common/board_common.mk

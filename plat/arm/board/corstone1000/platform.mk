#
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Making sure the corstone1000 platform type is specified
ifeq ($(filter ${TARGET_PLATFORM}, fpga fvp),)
	$(error TARGET_PLATFORM must be fpga or fvp)
endif

CORSTONE1000_CPU_LIBS	+=lib/cpus/aarch64/cortex_a35.S

PLAT_INCLUDES		:=	-Iplat/arm/board/corstone1000/common/include	\
				-Iplat/arm/board/corstone1000/include		\
				-Iinclude/plat/arm/common			\
				-Iinclude/plat/arm/css/common/aarch64


CORSTONE1000_FW_NVCTR_VAL	:=	255
TFW_NVCTR_VAL		:=	${CORSTONE1000_FW_NVCTR_VAL}
NTFW_NVCTR_VAL		:=	${CORSTONE1000_FW_NVCTR_VAL}

override NEED_BL1	:=	no

override NEED_BL2	:=	yes
FIP_BL2_ARGS := tb-fw

override NEED_BL2U	:=	no
override NEED_BL31	:=	yes
NEED_BL32		:=	yes
override NEED_BL33	:=	yes

# Include GICv2 driver files
include drivers/arm/gic/v2/gicv2.mk

CORSTONE1000_GIC_SOURCES	:=	${GICV2_SOURCES}			\
				plat/common/plat_gicv2.c		\
				plat/arm/common/arm_gicv2.c


BL2_SOURCES		+=	plat/arm/board/corstone1000/common/corstone1000_security.c		\
				plat/arm/board/corstone1000/common/corstone1000_err.c		\
				plat/arm/board/corstone1000/common/corstone1000_trusted_boot.c	\
				lib/utils/mem_region.c					\
				plat/arm/board/corstone1000/common/corstone1000_helpers.S		\
				plat/arm/board/corstone1000/common/corstone1000_plat.c		\
				plat/arm/board/corstone1000/common/corstone1000_bl2_mem_params_desc.c \
				${CORSTONE1000_CPU_LIBS}					\


BL31_SOURCES	+=	drivers/cfi/v2m/v2m_flash.c				\
			lib/utils/mem_region.c					\
			plat/arm/board/corstone1000/common/corstone1000_helpers.S		\
			plat/arm/board/corstone1000/common/corstone1000_topology.c		\
			plat/arm/board/corstone1000/common/corstone1000_security.c		\
			plat/arm/board/corstone1000/common/corstone1000_plat.c		\
			plat/arm/board/corstone1000/common/corstone1000_pm.c		\
			${CORSTONE1000_CPU_LIBS}					\
			${CORSTONE1000_GIC_SOURCES}

ifneq (${ENABLE_STACK_PROTECTOR},0)
	ifneq (${ENABLE_STACK_PROTECTOR},none)
		CORSTONE1000_SECURITY_SOURCES := plat/arm/board/corstone1000/common/corstone1000_stack_protector.c
		BL2_SOURCES += ${CORSTONE1000_SECURITY_SOURCES}
		BL31_SOURCES += ${CORSTONE1000_SECURITY_SOURCES}
	endif
endif

FDT_SOURCES		+=	plat/arm/board/corstone1000/common/fdts/corstone1000_spmc_manifest.dts
CORSTONE1000_TOS_FW_CONFIG	:=	${BUILD_PLAT}/fdts/corstone1000_spmc_manifest.dtb

# Add the SPMC manifest to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${CORSTONE1000_TOS_FW_CONFIG},--tos-fw-config,${CORSTONE1000_TOS_FW_CONFIG}))

# Adding TARGET_PLATFORM as a GCC define (-D option)
$(eval $(call add_define,TARGET_PLATFORM_$(call uppercase,${TARGET_PLATFORM})))

# Adding CORSTONE1000_FW_NVCTR_VAL as a GCC define (-D option)
$(eval $(call add_define,CORSTONE1000_FW_NVCTR_VAL))

include plat/arm/common/arm_common.mk
include plat/arm/board/common/board_common.mk

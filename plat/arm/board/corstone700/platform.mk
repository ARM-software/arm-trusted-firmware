#
# Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Making sure the corstone700 platform type is specified
ifeq ($(filter ${TARGET_PLATFORM}, fpga fvp),)
        $(error TARGET_PLATFORM must be fpga or fvp)
endif

CORSTONE700_CPU_LIBS	+=	lib/cpus/aarch32/cortex_a32.S

BL32_SOURCES		+=	plat/arm/common/aarch32/arm_helpers.S	\
				plat/arm/common/arm_console.c	\
				plat/arm/common/arm_common.c	\
				lib/xlat_tables/aarch32/xlat_tables.c	\
				lib/xlat_tables/xlat_tables_common.c	\
				${CORSTONE700_CPU_LIBS}	\
				plat/arm/board/corstone700/common/drivers/mhu/mhu.c

PLAT_INCLUDES		:=	-Iplat/arm/board/corstone700/common/include	\
				-Iinclude/plat/arm/common	\
				-Iplat/arm/board/corstone700/common/drivers/mhu

NEED_BL32		:=	yes

# Include GICv2 driver files
include drivers/arm/gic/v2/gicv2.mk

CORSTONE700_GIC_SOURCES	:=	${GICV2_SOURCES}			\
				plat/common/plat_gicv2.c		\
				plat/arm/common/arm_gicv2.c

# BL1/BL2 Image not a part of the capsule Image for Corstone700
override NEED_BL1	:=	no
override NEED_BL2	:=	no
override NEED_BL2U	:=	no
override NEED_BL33	:=	yes

#TFA for Corstone700 starts from BL32
override RESET_TO_SP_MIN	:=	1

#Device tree
CORSTONE700_HW_CONFIG_DTS	:=	fdts/corstone700_${TARGET_PLATFORM}.dts
CORSTONE700_HW_CONFIG		:=	${BUILD_PLAT}/fdts/corstone700_${TARGET_PLATFORM}.dtb
FDT_SOURCES			+=	${CORSTONE700_HW_CONFIG_DTS}
$(eval CORSTONE700_HW_CONFIG	:=	${BUILD_PLAT}/$(patsubst %.dts,%.dtb,$(CORSTONE700_HW_CONFIG_DTS)))

# Add the HW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${CORSTONE700_HW_CONFIG},--hw-config,${CORSTONE700_HW_CONFIG}))

# Check for Linux kernel as a BL33 image by default
$(eval $(call add_define,ARM_LINUX_KERNEL_AS_BL33))
  ifndef ARM_PRELOADED_DTB_BASE
    $(error "ARM_PRELOADED_DTB_BASE must be set if ARM_LINUX_KERNEL_AS_BL33 is used.")
  endif
  $(eval $(call add_define,ARM_PRELOADED_DTB_BASE))

# Adding TARGET_PLATFORM as a GCC define (-D option)
$(eval $(call add_define,TARGET_PLATFORM_$(call uppercase,${TARGET_PLATFORM})))

include plat/arm/board/common/board_common.mk

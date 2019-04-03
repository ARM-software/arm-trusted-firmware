#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/xlat_tables_v2/xlat_tables.mk

DOIMAGEPATH		?=	tools/meson
DOIMAGETOOL		?=	${DOIMAGEPATH}/doimage

PLAT_INCLUDES		:=	-Iinclude/drivers/meson/		\
				-Iinclude/drivers/meson/gxl		\
				-Iplat/meson/gxl/include

GXBB_GIC_SOURCES	:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c

PLAT_BL_COMMON_SOURCES	:=	drivers/meson/console/aarch64/meson_console.S \
				plat/meson/gxl/gxl_common.c		\
				plat/meson/gxl/gxl_topology.c		\
				${XLAT_TABLES_LIB_SRCS}

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				plat/common/plat_psci_common.c		\
				plat/meson/gxl/aarch64/gxl_helpers.S	\
				plat/meson/gxl/gxl_bl31_setup.c		\
				plat/meson/gxl/gxl_efuse.c		\
				plat/meson/gxl/gxl_mhu.c		\
				plat/meson/gxl/gxl_pm.c			\
				plat/meson/gxl/gxl_scpi.c		\
				plat/meson/gxl/gxl_sip_svc.c		\
				plat/meson/gxl/gxl_thermal.c		\
				drivers/meson/gxl/crypto/sha_dma.c	\
				${GXBB_GIC_SOURCES}

# Tune compiler for Cortex-A53
ifeq ($(notdir $(CC)),armclang)
    TF_CFLAGS_aarch64	+=	-mcpu=cortex-a53
else ifneq ($(findstring clang,$(notdir $(CC))),)
    TF_CFLAGS_aarch64	+=	-mcpu=cortex-a53
else
    TF_CFLAGS_aarch64	+=	-mtune=cortex-a53
endif

# Build config flags
# ------------------

# Enable all errata workarounds for Cortex-A53
ERRATA_A53_855873		:= 1
ERRATA_A53_819472		:= 1
ERRATA_A53_824069		:= 1
ERRATA_A53_827319		:= 1

WORKAROUND_CVE_2017_5715	:= 0

# Have different sections for code and rodata
SEPARATE_CODE_AND_RODATA	:= 1

# Use Coherent memory
USE_COHERENT_MEM		:= 1

# Use multi console API
MULTI_CONSOLE_API		:= 1

# Verify build config
# -------------------

ifneq (${MULTI_CONSOLE_API}, 1)
  $(error Error: gxl needs MULTI_CONSOLE_API=1)
endif

ifneq (${RESET_TO_BL31}, 0)
  $(error Error: gxl needs RESET_TO_BL31=0)
endif

ifeq (${ARCH},aarch32)
  $(error Error: AArch32 not supported on gxl)
endif

all: ${BUILD_PLAT}/bl31.img
distclean realclean clean: cleanimage

cleanimage:
	${Q}${MAKE} -C ${DOIMAGEPATH} clean

${DOIMAGETOOL}:
	${Q}${MAKE} -C ${DOIMAGEPATH}

${BUILD_PLAT}/bl31.img: ${BUILD_PLAT}/bl31.bin ${DOIMAGETOOL}
	${DOIMAGETOOL} ${BUILD_PLAT}/bl31.bin ${BUILD_PLAT}/bl31.img


#
# Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/xlat_tables_v2/xlat_tables.mk

AML_PLAT		:=	plat/amlogic
AML_PLAT_SOC		:=	${AML_PLAT}/${PLAT}
AML_PLAT_COMMON		:=	${AML_PLAT}/common

DOIMAGEPATH		?=	tools/amlogic
DOIMAGETOOL		?=	${DOIMAGEPATH}/doimage

PLAT_INCLUDES		:=	-Iinclude/drivers/amlogic/			\
				-I${AML_PLAT_SOC}/include			\
				-I${AML_PLAT_COMMON}/include

GIC_SOURCES		:=	drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v2/gicv2_main.c			\
				drivers/arm/gic/v2/gicv2_helpers.c		\
				plat/common/plat_gicv2.c

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S			\
				plat/common/plat_psci_common.c			\
				drivers/amlogic/console/aarch64/meson_console.S	\
				${AML_PLAT_SOC}/${PLAT}_bl31_setup.c		\
				${AML_PLAT_SOC}/${PLAT}_pm.c			\
				${AML_PLAT_SOC}/${PLAT}_common.c		\
				${AML_PLAT_COMMON}/aarch64/aml_helpers.S	\
				${AML_PLAT_COMMON}/aml_efuse.c			\
				${AML_PLAT_COMMON}/aml_mhu.c			\
				${AML_PLAT_COMMON}/aml_scpi.c			\
				${AML_PLAT_COMMON}/aml_sip_svc.c		\
				${AML_PLAT_COMMON}/aml_thermal.c		\
				${AML_PLAT_COMMON}/aml_topology.c		\
				${AML_PLAT_COMMON}/aml_console.c		\
				drivers/amlogic/crypto/sha_dma.c		\
				${XLAT_TABLES_LIB_SRCS}				\
				${GIC_SOURCES}

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

# Verify build config
# -------------------

ifneq (${RESET_TO_BL31}, 0)
  $(error Error: ${PLAT} needs RESET_TO_BL31=0)
endif

ifeq (${ARCH},aarch32)
  $(error Error: AArch32 not supported on ${PLAT})
endif

all: ${BUILD_PLAT}/bl31.img
distclean realclean clean: cleanimage

cleanimage:
	${Q}${MAKE} -C ${DOIMAGEPATH} clean

${DOIMAGETOOL}:
	${Q}${MAKE} -C ${DOIMAGEPATH}

${BUILD_PLAT}/bl31.img: ${BUILD_PLAT}/bl31.bin ${DOIMAGETOOL}
	${DOIMAGETOOL} ${BUILD_PLAT}/bl31.bin ${BUILD_PLAT}/bl31.img


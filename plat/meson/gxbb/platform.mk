#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/xlat_tables_v2/xlat_tables.mk

PLAT_INCLUDES		:=	-Iplat/meson/gxbb/include

GXBB_GIC_SOURCES	:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c

PLAT_BL_COMMON_SOURCES	:=	drivers/meson/console/aarch64/meson_console.S \
				plat/meson/gxbb/gxbb_common.c		\
				plat/meson/gxbb/gxbb_topology.c		\
				${XLAT_TABLES_LIB_SRCS}

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				plat/common/plat_psci_common.c		\
				plat/meson/gxbb/aarch64/gxbb_helpers.S	\
				plat/meson/gxbb/gxbb_bl31_setup.c	\
				plat/meson/gxbb/gxbb_efuse.c		\
				plat/meson/gxbb/gxbb_mhu.c		\
				plat/meson/gxbb/gxbb_pm.c		\
				plat/meson/gxbb/gxbb_scpi.c		\
				plat/meson/gxbb/gxbb_sip_svc.c		\
				plat/meson/gxbb/gxbb_thermal.c		\
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
ERRATA_A53_826319		:= 1
ERRATA_A53_835769		:= 1
ERRATA_A53_836870		:= 1
ERRATA_A53_843419		:= 1
ERRATA_A53_855873		:= 1

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
  $(error Error: gxbb needs MULTI_CONSOLE_API=1)
endif

ifneq (${RESET_TO_BL31}, 0)
  $(error Error: gxbb needs RESET_TO_BL31=0)
endif

ifeq (${ARCH},aarch32)
  $(error Error: AArch32 not supported on gxbb)
endif

#
# Copyright (c) 2021, Stephan Gerhold <stephan@gerhold.net>
#
# SPDX-License-Identifier: BSD-3-Clause
#

include drivers/arm/gic/v2/gicv2.mk
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES	:= ${XLAT_TABLES_LIB_SRCS}

PLAT_INCLUDES	:=	-Iinclude/plat/arm/common/${ARCH}		\
			-Iplat/qti/msm8916/include

BL31_SOURCES	+=	${GICV2_SOURCES}				\
			drivers/delay_timer/delay_timer.c		\
			drivers/delay_timer/generic_delay_timer.c	\
			lib/cpus/${ARCH}/cortex_a53.S			\
			plat/common/plat_gicv2.c			\
			plat/common/plat_psci_common.c			\
			plat/qti/msm8916/msm8916_bl31_setup.c		\
			plat/qti/msm8916/msm8916_cpu_boot.c		\
			plat/qti/msm8916/msm8916_gicv2.c		\
			plat/qti/msm8916/msm8916_pm.c			\
			plat/qti/msm8916/msm8916_topology.c		\
			plat/qti/msm8916/${ARCH}/msm8916_helpers.S	\
			plat/qti/msm8916/${ARCH}/uartdm_console.S

# Only BL31 is supported at the moment and is entered on a single CPU
RESET_TO_BL31			:= 1
COLD_BOOT_SINGLE_CPU		:= 1

# Build config flags
# ------------------
BL31_BASE			?= 0x86500000
BL32_BASE			?= 0x86000000
PRELOADED_BL33_BASE		?= 0x8f600000

# Have different sections for code and rodata
SEPARATE_CODE_AND_RODATA	:= 1

# Single cluster
WARMBOOT_ENABLE_DCACHE_EARLY	:= 1

# Disable features unsupported in ARMv8.0
ENABLE_AMU			:= 0
ENABLE_SPE_FOR_LOWER_ELS	:= 0
ENABLE_SVE_FOR_NS		:= 0

# MSM8916 uses ARM Cortex-A53 r0p0 so likely all the errata apply
ERRATA_A53_819472		:= 1
ERRATA_A53_824069		:= 1
ERRATA_A53_826319		:= 1
ERRATA_A53_827319		:= 1
ERRATA_A53_835769		:= 1
ERRATA_A53_836870		:= 1
ERRATA_A53_843419		:= 1
ERRATA_A53_855873		:= 0	# Workaround works only for >= r0p3
ERRATA_A53_1530924		:= 1

$(eval $(call add_define,BL31_BASE))
$(eval $(call add_define,BL32_BASE))

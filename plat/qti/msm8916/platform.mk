#
# Copyright (c) 2021-2023, Stephan Gerhold <stephan@gerhold.net>
#
# SPDX-License-Identifier: BSD-3-Clause
#

include drivers/arm/gic/v2/gicv2.mk
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES	:=	${GICV2_SOURCES}				\
				${XLAT_TABLES_LIB_SRCS}				\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				plat/common/plat_gicv2.c			\
				plat/qti/msm8916/msm8916_gicv2.c		\
				plat/qti/msm8916/msm8916_setup.c		\
				plat/qti/msm8916/${ARCH}/msm8916_helpers.S	\
				plat/qti/msm8916/${ARCH}/uartdm_console.S

MSM8916_CPU		:=	$(if ${ARM_CORTEX_A7},cortex_a7,cortex_a53)
MSM8916_PM_SOURCES	:=	drivers/arm/cci/cci.c				\
				lib/cpus/${ARCH}/${MSM8916_CPU}.S		\
				plat/common/plat_psci_common.c			\
				plat/qti/msm8916/msm8916_config.c		\
				plat/qti/msm8916/msm8916_cpu_boot.c		\
				plat/qti/msm8916/msm8916_pm.c			\
				plat/qti/msm8916/msm8916_topology.c

BL31_SOURCES		+=	${MSM8916_PM_SOURCES}				\
				plat/qti/msm8916/msm8916_bl31_setup.c

PLAT_INCLUDES		:=	-Iplat/qti/msm8916/include

ifeq (${ARCH},aarch64)
# arm_macros.S exists only on aarch64 currently
PLAT_INCLUDES		+=	-Iinclude/plat/arm/common/${ARCH}
endif

# Only BL31 is supported at the moment and is entered on a single CPU
RESET_TO_BL31			:= 1
COLD_BOOT_SINGLE_CPU		:= 1

# Have different sections for code and rodata
SEPARATE_CODE_AND_RODATA	:= 1

# Single cluster
WARMBOOT_ENABLE_DCACHE_EARLY	:= 1

# Disable features unsupported in ARMv8.0
ENABLE_SPE_FOR_NS		:= 0
ENABLE_SVE_FOR_NS		:= 0

# Disable workarounds unnecessary for Cortex-A7/A53
WORKAROUND_CVE_2017_5715	:= 0
WORKAROUND_CVE_2022_23960	:= 0

ifeq (${MSM8916_CPU},cortex_a53)
# The Cortex-A53 revision varies depending on the SoC revision.
# msm8916 uses r0p0, msm8939 uses r0p1 or r0p4. Enable all errata
# and rely on the runtime detection to apply them only if needed.
ERRATA_A53_819472		:= 1
ERRATA_A53_824069		:= 1
ERRATA_A53_826319		:= 1
ERRATA_A53_827319		:= 1
ERRATA_A53_835769		:= 1
ERRATA_A53_836870		:= 1
ERRATA_A53_843419		:= 1
ERRATA_A53_855873		:= 1
ERRATA_A53_1530924		:= 1
endif

# Build config flags
# ------------------
BL31_BASE			?= 0x86500000
PRELOADED_BL33_BASE		?= 0x8f600000

ifeq (${ARCH},aarch64)
    BL32_BASE			?= BL31_LIMIT
    $(eval $(call add_define,BL31_BASE))
else
    ifeq (${AARCH32_SP},none)
	$(error Variable AARCH32_SP has to be set for AArch32)
    endif
    # There is no BL31 on aarch32, so reuse its location for BL32
    BL32_BASE			?= $(BL31_BASE)
endif
$(eval $(call add_define,BL32_BASE))

# UART number to use for TF-A output during early boot
QTI_UART_NUM			?= 2
$(eval $(call assert_numeric,QTI_UART_NUM))
$(eval $(call add_define,QTI_UART_NUM))

# Set to 1 on the command line to keep using UART after early boot.
# Requires reserving the UART and related clocks inside the normal world.
QTI_RUNTIME_UART		?= 0
$(eval $(call assert_boolean,QTI_RUNTIME_UART))
$(eval $(call add_define,QTI_RUNTIME_UART))

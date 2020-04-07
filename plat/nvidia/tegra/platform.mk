#
# Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
# Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

SOC_DIR			:=	plat/nvidia/tegra/soc/${TARGET_SOC}

# dump the state on crash console
CRASH_REPORTING		:=	1
$(eval $(call add_define,CRASH_REPORTING))

# enable assert() for release/debug builds
ENABLE_ASSERTIONS	:=	1
PLAT_LOG_LEVEL_ASSERT	:=	50
$(eval $(call add_define,PLAT_LOG_LEVEL_ASSERT))

# enable dynamic memory mapping
PLAT_XLAT_TABLES_DYNAMIC :=	1
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))

# Enable exception handling at EL3
EL3_EXCEPTION_HANDLING	:=	1
GICV2_G0_FOR_EL3	:=	1

# Enable PSCI v1.0 extended state ID format
PSCI_EXTENDED_STATE_ID	:=	1

# code and read-only data should be put on separate memory pages
SEPARATE_CODE_AND_RODATA :=	1

# do not use coherent memory
USE_COHERENT_MEM	:=	0

# enable D-cache early during CPU warmboot
WARMBOOT_ENABLE_DCACHE_EARLY := 1

# remove the standard libc
OVERRIDE_LIBC		:=	1

# Flag to enable WDT FIQ interrupt handling for Tegra SoCs
# prior to Tegra186
ENABLE_TEGRA_WDT_LEGACY_FIQ_HANDLING	?= 0

# Flag to allow relocation of BL32 image to TZDRAM during boot
RELOCATE_BL32_IMAGE		?= 0

# Enable stack protection
ENABLE_STACK_PROTECTOR		:=	strong

# Enable SDEI
SDEI_SUPPORT			:= 1

# modify BUILD_PLAT to point to SoC specific build directory
BUILD_PLAT	:=	${BUILD_BASE}/${PLAT}/${TARGET_SOC}/${BUILD_TYPE}

include plat/nvidia/tegra/common/tegra_common.mk
include ${SOC_DIR}/platform_${TARGET_SOC}.mk

$(eval $(call add_define,ENABLE_TEGRA_WDT_LEGACY_FIQ_HANDLING))
$(eval $(call add_define,RELOCATE_BL32_IMAGE))

# platform cflags (enable signed comparisons, disable stdlib)
TF_CFLAGS	+= -nostdlib

# override with necessary libc files for the Tegra platform
override LIBC_SRCS :=	$(addprefix lib/libc/,		\
			aarch64/setjmp.S		\
			assert.c			\
			memchr.c			\
			memcmp.c			\
			memcpy.c			\
			memmove.c			\
			memset.c			\
			printf.c			\
			putchar.c			\
			strrchr.c			\
			strlen.c			\
			snprintf.c)

INCLUDES	+=	-Iinclude/lib/libc		\
			-Iinclude/lib/libc/$(ARCH)	\

ifneq ($(findstring armlink,$(notdir $(LD))),)
# o suppress warnings for section mismatches, undefined symbols
# o use only those libraries that are specified in the input file
#   list to resolve references
# o create a static callgraph of functions
# o resolve undefined symbols to el3_panic
# o include only required sections
TF_LDFLAGS	+= --diag_suppress=L6314,L6332 --no_scanlib --callgraph
TF_LDFLAGS	+= --keep="*(__pubsub*)" --keep="*(rt_svc_descs*)" --keep="*(*cpu_ops)"
ifeq (${ENABLE_PMF},1)
TF_LDFLAGS	+= --keep="*(*pmf_svc_descs*)"
endif
endif

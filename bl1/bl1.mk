#
# Copyright (c) 2013-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL1_SOURCES		+=	bl1/${ARCH}/bl1_arch_setup.c		\
				bl1/${ARCH}/bl1_context_mgmt.c		\
				bl1/${ARCH}/bl1_entrypoint.S		\
				bl1/${ARCH}/bl1_exceptions.S		\
				bl1/bl1_main.c				\
				lib/cpus/${ARCH}/cpu_helpers.S		\
				lib/cpus/errata_report.c		\
				lib/el3_runtime/${ARCH}/context_mgmt.c	\
				plat/common/plat_bl1_common.c		\
				plat/common/${ARCH}/platform_up_stack.S \
				${MBEDTLS_SOURCES}

ifeq (${ARCH},aarch64)
BL1_SOURCES		+=	lib/el3_runtime/aarch64/context.S	\
				lib/cpus/errata_common.c
endif

ifeq (${TRUSTED_BOARD_BOOT},1)
BL1_SOURCES		+=	bl1/bl1_fwu.c
endif

ifeq (${ENABLE_PMF},1)
BL1_SOURCES		+=	lib/pmf/pmf_main.c
endif

BL1_DEFAULT_LINKER_SCRIPT_SOURCE := bl1/bl1.ld.S

# CRYPTO_SUPPORT
NEED_AUTH := $(if $(filter 1,$(TRUSTED_BOARD_BOOT)),1,)
NEED_HASH := $(if $(filter 1,$(MEASURED_BOOT) $(DRTM_SUPPORT)),1,)
$(eval $(call set_crypto_support,NEED_AUTH,NEED_HASH))

# BL1_CPPFLAGS
$(eval BL1_CPPFLAGS += $(call make_defines, \
    $(sort \
        CRYPTO_SUPPORT \
)))

# Numeric_Flags
$(eval $(call assert_numerics,\
    $(sort \
	CRYPTO_SUPPORT \
)))

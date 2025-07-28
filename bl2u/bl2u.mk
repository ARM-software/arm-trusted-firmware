#
# Copyright (c) 2015-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL2U_SOURCES		+=	bl2u/bl2u_main.c			\
				bl2u/${ARCH}/bl2u_entrypoint.S		\
				plat/common/${ARCH}/platform_up_stack.S

ifeq (${ARCH},aarch64)
BL2U_SOURCES		+=	common/aarch64/early_exceptions.S
endif

BL2U_DEFAULT_LINKER_SCRIPT_SOURCE := bl2u/bl2u.ld.S

# CRYPTO_SUPPORT
NEED_AUTH := $(if $(filter 1,$(TRUSTED_BOARD_BOOT)),1,)
NEED_HASH := $(if $(filter 1,$(MEASURED_BOOT) $(DRTM_SUPPORT)),1,)
$(eval $(call set_crypto_support,NEED_AUTH,NEED_HASH))

# BL2U_CPPFLAGS
$(eval BL2U_CPPFLAGS += $(call make_defines, \
    $(sort \
        CRYPTO_SUPPORT \
)))

# Numeric_Flags
$(eval $(call assert_numerics,\
    $(sort \
	CRYPTO_SUPPORT \
)))

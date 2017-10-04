#
# Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES		+=	-Iinclude/plat/arm/board/common/			\
				-Iinclude/plat/arm/board/common/drivers

PLAT_BL_COMMON_SOURCES	+=	drivers/arm/pl011/${ARCH}/pl011_console.S		\
				plat/arm/board/common/${ARCH}/board_arm_helpers.S

BL1_SOURCES		+=	plat/arm/board/common/drivers/norflash/norflash.c

BL2_SOURCES		+=	lib/utils/mem_region.c					\
				plat/arm/common/arm_nor_psci_mem_protect.c		\
				plat/arm/board/common/drivers/norflash/norflash.c

BL31_SOURCES		+=	lib/utils/mem_region.c					\
				plat/arm/board/common/drivers/norflash/norflash.c	\
				plat/arm/common/arm_nor_psci_mem_protect.c

ifneq (${TRUSTED_BOARD_BOOT},0)
  ifneq (${ARM_CRYPTOCELL_INTEG}, 1)
    # ROTPK hash location
    ifeq (${ARM_ROTPK_LOCATION}, regs)
        ARM_ROTPK_LOCATION_ID = ARM_ROTPK_REGS_ID
    else ifeq (${ARM_ROTPK_LOCATION}, devel_rsa)
        KEY_ALG := rsa
        ARM_ROTPK_LOCATION_ID = ARM_ROTPK_DEVEL_RSA_ID
    else ifeq (${ARM_ROTPK_LOCATION}, devel_ecdsa)
        KEY_ALG := ecdsa
        ARM_ROTPK_LOCATION_ID = ARM_ROTPK_DEVEL_ECDSA_ID
    else
        $(error "Unsupported ARM_ROTPK_LOCATION value")
    endif
    $(eval $(call add_define,ARM_ROTPK_LOCATION_ID))

    # Certificate NV-Counters. Use values corresponding to tied off values in
    # ARM development platforms
    TFW_NVCTR_VAL	?=	31
    NTFW_NVCTR_VAL	?=	223
  else
    # Certificate NV-Counters when CryptoCell is integrated. For development
    # platforms we set the counter to first valid value.
    TFW_NVCTR_VAL	?=	0
    NTFW_NVCTR_VAL	?=	0
  endif
    BL1_SOURCES		+=	plat/arm/board/common/board_arm_trusted_boot.c
    BL2_SOURCES		+=	plat/arm/board/common/board_arm_trusted_boot.c
endif

# This flag controls whether memory usage needs to be optimised
ARM_BOARD_OPTIMISE_MEM	?=	0

# Process flags
$(eval $(call assert_boolean,ARM_BOARD_OPTIMISE_MEM))
$(eval $(call add_define,ARM_BOARD_OPTIMISE_MEM))

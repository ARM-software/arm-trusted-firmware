#
# Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# Neither the name of ARM nor the names of its contributors may be used
# to endorse or promote products derived from this software without specific
# prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

ifeq (${ARCH}, aarch64)
  # On ARM standard platorms, the TSP can execute from Trusted SRAM, Trusted
  # DRAM (if available) or the TZC secured area of DRAM.
  # Trusted SRAM is the default.

  ARM_TSP_RAM_LOCATION	:=	tsram
  ifeq (${ARM_TSP_RAM_LOCATION}, tsram)
    ARM_TSP_RAM_LOCATION_ID = ARM_TRUSTED_SRAM_ID
  else ifeq (${ARM_TSP_RAM_LOCATION}, tdram)
    ARM_TSP_RAM_LOCATION_ID = ARM_TRUSTED_DRAM_ID
  else ifeq (${ARM_TSP_RAM_LOCATION}, dram)
    ARM_TSP_RAM_LOCATION_ID = ARM_DRAM_ID
  else
    $(error "Unsupported ARM_TSP_RAM_LOCATION value")
  endif

  # Process flags
  $(eval $(call add_define,ARM_TSP_RAM_LOCATION_ID))

  # Process ARM_BL31_IN_DRAM flag
  ARM_BL31_IN_DRAM		:=	0
  $(eval $(call assert_boolean,ARM_BL31_IN_DRAM))
  $(eval $(call add_define,ARM_BL31_IN_DRAM))
endif

# For the original power-state parameter format, the State-ID can be encoded
# according to the recommended encoding or zero. This flag determines which
# State-ID encoding to be parsed.
ARM_RECOM_STATE_ID_ENC := 0

# If the PSCI_EXTENDED_STATE_ID is set, then the recommended state ID need to
# be used. Else throw a build error.
ifeq (${PSCI_EXTENDED_STATE_ID}, 1)
  ifeq (${ARM_RECOM_STATE_ID_ENC}, 0)
    $(error "Incompatible STATE_ID build option specified")
  endif
endif

# Process ARM_RECOM_STATE_ID_ENC flag
$(eval $(call assert_boolean,ARM_RECOM_STATE_ID_ENC))
$(eval $(call add_define,ARM_RECOM_STATE_ID_ENC))

# Process ARM_DISABLE_TRUSTED_WDOG flag
# By default, Trusted Watchdog is always enabled unless SPIN_ON_BL1_EXIT is set
ARM_DISABLE_TRUSTED_WDOG	:=	0
ifeq (${SPIN_ON_BL1_EXIT}, 1)
ARM_DISABLE_TRUSTED_WDOG	:=	1
endif
$(eval $(call assert_boolean,ARM_DISABLE_TRUSTED_WDOG))
$(eval $(call add_define,ARM_DISABLE_TRUSTED_WDOG))

# Process ARM_CONFIG_CNTACR
ARM_CONFIG_CNTACR		:=	1
$(eval $(call assert_boolean,ARM_CONFIG_CNTACR))
$(eval $(call add_define,ARM_CONFIG_CNTACR))

# Process ARM_BL31_IN_DRAM flag
ARM_BL31_IN_DRAM		:=	0
$(eval $(call assert_boolean,ARM_BL31_IN_DRAM))
$(eval $(call add_define,ARM_BL31_IN_DRAM))

# Enable PSCI_STAT_COUNT/RESIDENCY APIs on ARM platforms
ENABLE_PSCI_STAT		:=	1

# On ARM platforms, separate the code and read-only data sections to allow
# mapping the former as executable and the latter as execute-never.
SEPARATE_CODE_AND_RODATA	:=	1


PLAT_INCLUDES		+=	-Iinclude/common/tbbr				\
				-Iinclude/plat/arm/common

ifeq (${ARCH}, aarch64)
PLAT_INCLUDES		+=	-Iinclude/plat/arm/common/aarch64
endif

PLAT_BL_COMMON_SOURCES	+=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/${ARCH}/xlat_tables.c		\
				plat/arm/common/${ARCH}/arm_helpers.S		\
				plat/arm/common/arm_common.c			\
				plat/common/${ARCH}/plat_common.c

BL1_SOURCES		+=	drivers/arm/sp805/sp805.c			\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/arm/common/arm_bl1_setup.c			\
				plat/arm/common/arm_io_storage.c		\
				plat/common/${ARCH}/platform_up_stack.S
ifdef EL3_PAYLOAD_BASE
# Need the arm_program_trusted_mailbox() function to release secondary CPUs from
# their holding pen
BL1_SOURCES		+=	plat/arm/common/arm_pm.c
endif

BL2_SOURCES		+=	drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/arm/common/arm_bl2_setup.c			\
				plat/arm/common/arm_io_storage.c		\
				plat/common/${ARCH}/platform_up_stack.S
ifeq (${LOAD_IMAGE_V2},1)
BL2_SOURCES		+=	plat/arm/common/${ARCH}/arm_bl2_mem_params_desc.c\
				plat/arm/common/arm_image_load.c		\
				common/desc_image_load.c
endif

BL2U_SOURCES		+=	plat/arm/common/arm_bl2u_setup.c		\
				plat/common/aarch64/platform_up_stack.S

BL31_SOURCES		+=	plat/arm/common/arm_bl31_setup.c		\
				plat/arm/common/arm_pm.c			\
				plat/arm/common/arm_topology.c			\
				plat/common/aarch64/platform_mp_stack.S		\
				plat/common/plat_psci_common.c

ifneq (${TRUSTED_BOARD_BOOT},0)

    # By default, ARM platforms use RSA keys
    KEY_ALG		:=	rsa

    # Include common TBB sources
    AUTH_SOURCES	:=	drivers/auth/auth_mod.c				\
				drivers/auth/crypto_mod.c			\
				drivers/auth/img_parser_mod.c			\
				drivers/auth/tbbr/tbbr_cot.c			\

    PLAT_INCLUDES	+=	-Iinclude/bl1/tbbr

    BL1_SOURCES		+=	${AUTH_SOURCES}					\
				bl1/tbbr/tbbr_img_desc.c			\
				plat/arm/common/arm_bl1_fwu.c

    BL2_SOURCES		+=	${AUTH_SOURCES}

    $(eval $(call FWU_FIP_ADD_IMG,NS_BL2U,--fwu))

    MBEDTLS_KEY_ALG	:=	${KEY_ALG}

    # We expect to locate the *.mk files under the directories specified below
    CRYPTO_LIB_MK := drivers/auth/mbedtls/mbedtls_crypto.mk
    IMG_PARSER_LIB_MK := drivers/auth/mbedtls/mbedtls_x509.mk

    $(info Including ${CRYPTO_LIB_MK})
    include ${CRYPTO_LIB_MK}

    $(info Including ${IMG_PARSER_LIB_MK})
    include ${IMG_PARSER_LIB_MK}

endif

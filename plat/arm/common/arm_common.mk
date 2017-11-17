#
# Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${ARCH}, aarch64)
  # On ARM standard platorms, the TSP can execute from Trusted SRAM, Trusted
  # DRAM (if available) or the TZC secured area of DRAM.
  # TZC secured DRAM is the default.

  ARM_TSP_RAM_LOCATION	?=	dram

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
  # Process ARM_BL31_IN_DRAM flag
  ARM_BL31_IN_DRAM		:=	0
  $(eval $(call assert_boolean,ARM_BL31_IN_DRAM))
  $(eval $(call add_define,ARM_BL31_IN_DRAM))
else
  ARM_TSP_RAM_LOCATION_ID = ARM_TRUSTED_SRAM_ID
endif

$(eval $(call add_define,ARM_TSP_RAM_LOCATION_ID))


# For the original power-state parameter format, the State-ID can be encoded
# according to the recommended encoding or zero. This flag determines which
# State-ID encoding to be parsed.
ARM_RECOM_STATE_ID_ENC := 0

# If the PSCI_EXTENDED_STATE_ID is set, then ARM_RECOM_STATE_ID_ENC need to
# be set. Else throw a build error.
ifeq (${PSCI_EXTENDED_STATE_ID}, 1)
  ifeq (${ARM_RECOM_STATE_ID_ENC}, 0)
    $(error Build option ARM_RECOM_STATE_ID_ENC needs to be set if \
            PSCI_EXTENDED_STATE_ID is set for ARM platforms)
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

# Process ARM_PLAT_MT flag
ARM_PLAT_MT			:=	0
$(eval $(call assert_boolean,ARM_PLAT_MT))
$(eval $(call add_define,ARM_PLAT_MT))

# Use translation tables library v2 by default
ARM_XLAT_TABLES_LIB_V1		:=	0
$(eval $(call assert_boolean,ARM_XLAT_TABLES_LIB_V1))
$(eval $(call add_define,ARM_XLAT_TABLES_LIB_V1))

# Use an implementation of SHA-256 with a smaller memory footprint but reduced
# speed.
$(eval $(call add_define,MBEDTLS_SHA256_SMALLER))

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call FIP_ADD_IMG,BL32_EXTRA1,--tos-fw-extra1))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call FIP_ADD_IMG,BL32_EXTRA2,--tos-fw-extra2))
endif

# Enable PSCI_STAT_COUNT/RESIDENCY APIs on ARM platforms
ENABLE_PSCI_STAT		:=	1
ENABLE_PMF			:=	1

# On ARM platforms, separate the code and read-only data sections to allow
# mapping the former as executable and the latter as execute-never.
SEPARATE_CODE_AND_RODATA	:=	1

# Enable new version of image loading on ARM platforms
LOAD_IMAGE_V2			:=	1

# Use generic OID definition (tbbr_oid.h)
USE_TBBR_DEFS			:=	1

# Disable ARM Cryptocell by default
ARM_CRYPTOCELL_INTEG		:=	0
$(eval $(call assert_boolean,ARM_CRYPTOCELL_INTEG))
$(eval $(call add_define,ARM_CRYPTOCELL_INTEG))

PLAT_INCLUDES		+=	-Iinclude/common/tbbr				\
				-Iinclude/plat/arm/common

ifeq (${ARCH}, aarch64)
PLAT_INCLUDES		+=	-Iinclude/plat/arm/common/aarch64
endif

PLAT_BL_COMMON_SOURCES	+=	plat/arm/common/${ARCH}/arm_helpers.S		\
				plat/arm/common/arm_common.c

ifeq (${ARM_XLAT_TABLES_LIB_V1}, 1)
PLAT_BL_COMMON_SOURCES	+=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/${ARCH}/xlat_tables.c
else
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}
endif

BL1_SOURCES		+=	drivers/arm/sp805/sp805.c			\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/arm/common/arm_bl1_setup.c			\
				plat/arm/common/arm_io_storage.c
ifdef EL3_PAYLOAD_BASE
# Need the arm_program_trusted_mailbox() function to release secondary CPUs from
# their holding pen
BL1_SOURCES		+=	plat/arm/common/arm_pm.c
endif

BL2_SOURCES		+=	drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/arm/common/arm_bl2_setup.c			\
				plat/arm/common/arm_io_storage.c

ifeq (${BL2_AT_EL3},1)
BL2_SOURCES		+=	plat/arm/common/arm_bl2_el3_setup.c
endif

ifeq (${LOAD_IMAGE_V2},1)
# Because BL1/BL2 execute in AArch64 mode but BL32 in AArch32 we need to use
# the AArch32 descriptors.
ifeq (${JUNO_AARCH32_EL3_RUNTIME},1)
BL2_SOURCES		+=	plat/arm/common/aarch32/arm_bl2_mem_params_desc.c
else
BL2_SOURCES		+=	plat/arm/common/${ARCH}/arm_bl2_mem_params_desc.c
endif
BL2_SOURCES		+=	plat/arm/common/arm_image_load.c		\
				common/desc_image_load.c
ifeq (${SPD},opteed)
BL2_SOURCES		+=	lib/optee/optee_utils.c
endif
endif

BL2U_SOURCES		+=	drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				plat/arm/common/arm_bl2u_setup.c

BL31_SOURCES		+=	plat/arm/common/arm_bl31_setup.c		\
				plat/arm/common/arm_pm.c			\
				plat/arm/common/arm_topology.c			\
				plat/arm/common/execution_state_switch.c	\
				plat/common/plat_psci_common.c

ifeq (${ENABLE_PMF}, 1)
BL31_SOURCES		+=	plat/arm/common/arm_sip_svc.c			\
				lib/pmf/pmf_smc.c
endif

ifeq (${EL3_EXCEPTION_HANDLING},1)
BL31_SOURCES		+=	plat/arm/common/aarch64/arm_ehf.c
endif

ifeq (${SDEI_SUPPORT},1)
BL31_SOURCES		+=	plat/arm/common/aarch64/arm_sdei.c
endif

ifneq (${TRUSTED_BOARD_BOOT},0)

    # Include common TBB sources
    AUTH_SOURCES	:=	drivers/auth/auth_mod.c				\
				drivers/auth/crypto_mod.c			\
				drivers/auth/img_parser_mod.c			\
				drivers/auth/tbbr/tbbr_cot.c			\

    PLAT_INCLUDES	+=	-Iinclude/bl1/tbbr

    BL1_SOURCES		+=	${AUTH_SOURCES}					\
				bl1/tbbr/tbbr_img_desc.c			\
				plat/arm/common/arm_bl1_fwu.c			\
				plat/common/tbbr/plat_tbbr.c

    BL2_SOURCES		+=	${AUTH_SOURCES}					\
				plat/common/tbbr/plat_tbbr.c

    $(eval $(call FWU_FIP_ADD_IMG,NS_BL2U,--fwu))

    # We expect to locate the *.mk files under the directories specified below
ifeq (${ARM_CRYPTOCELL_INTEG},0)
    CRYPTO_LIB_MK := drivers/auth/mbedtls/mbedtls_crypto.mk
else
    CRYPTO_LIB_MK := drivers/auth/cryptocell/cryptocell_crypto.mk
endif
    IMG_PARSER_LIB_MK := drivers/auth/mbedtls/mbedtls_x509.mk

    $(info Including ${CRYPTO_LIB_MK})
    include ${CRYPTO_LIB_MK}

    $(info Including ${IMG_PARSER_LIB_MK})
    include ${IMG_PARSER_LIB_MK}

endif

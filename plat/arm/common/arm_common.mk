#
# Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include common/fdt_wrappers.mk

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
# By default, Trusted Watchdog is always enabled unless
# SPIN_ON_BL1_EXIT or ENABLE_RME is set
ARM_DISABLE_TRUSTED_WDOG	:=	0
ifneq ($(filter 1,${SPIN_ON_BL1_EXIT} ${ENABLE_RME}),)
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

# As per CCA security model, all root firmware must execute from on-chip secure
# memory. This means we must not run BL31 from TZC-protected DRAM.
ifeq (${ARM_BL31_IN_DRAM},1)
  ifeq (${ENABLE_RME},1)
    $(error "BL31 must not run from DRAM on RME-systems. Please set ARM_BL31_IN_DRAM to 0")
  endif
endif

# Process ARM_PLAT_MT flag
ARM_PLAT_MT			:=	0
$(eval $(call assert_boolean,ARM_PLAT_MT))
$(eval $(call add_define,ARM_PLAT_MT))

# Use translation tables library v2 by default
ARM_XLAT_TABLES_LIB_V1		:=	0
$(eval $(call assert_boolean,ARM_XLAT_TABLES_LIB_V1))
$(eval $(call add_define,ARM_XLAT_TABLES_LIB_V1))

# Don't have the Linux kernel as a BL33 image by default
ARM_LINUX_KERNEL_AS_BL33	:=	0
$(eval $(call assert_boolean,ARM_LINUX_KERNEL_AS_BL33))
$(eval $(call add_define,ARM_LINUX_KERNEL_AS_BL33))

ifeq (${ARM_LINUX_KERNEL_AS_BL33},1)
  ifneq (${ARCH},aarch64)
    ifneq (${RESET_TO_SP_MIN},1)
      $(error "ARM_LINUX_KERNEL_AS_BL33 is only available if RESET_TO_SP_MIN=1.")
    endif
  endif
  ifndef PRELOADED_BL33_BASE
    $(error "PRELOADED_BL33_BASE must be set if ARM_LINUX_KERNEL_AS_BL33 is used.")
  endif
  ifeq (${RESET_TO_BL31},1)
    ifndef ARM_PRELOADED_DTB_BASE
      $(error "ARM_PRELOADED_DTB_BASE must be set if ARM_LINUX_KERNEL_AS_BL33 is
       used with RESET_TO_BL31.")
    endif
    $(eval $(call add_define,ARM_PRELOADED_DTB_BASE))
  endif
endif

# Arm Ethos-N NPU SiP service
ARM_ETHOSN_NPU_DRIVER			:=	0
$(eval $(call assert_boolean,ARM_ETHOSN_NPU_DRIVER))
$(eval $(call add_define,ARM_ETHOSN_NPU_DRIVER))

# Use an implementation of SHA-256 with a smaller memory footprint but reduced
# speed.
$(eval $(call add_define,MBEDTLS_SHA256_SMALLER))

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call TOOL_ADD_IMG,bl32_extra1,--tos-fw-extra1))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call TOOL_ADD_IMG,bl32_extra2,--tos-fw-extra2))
endif

# Enable PSCI_STAT_COUNT/RESIDENCY APIs on ARM platforms
ENABLE_PSCI_STAT		:=	1
ENABLE_PMF			:=	1

# Override the standard libc with optimised libc_asm
OVERRIDE_LIBC			:=	1
ifeq (${OVERRIDE_LIBC},1)
    include lib/libc/libc_asm.mk
endif

# On ARM platforms, separate the code and read-only data sections to allow
# mapping the former as executable and the latter as execute-never.
SEPARATE_CODE_AND_RODATA	:=	1

# On ARM platforms, disable SEPARATE_NOBITS_REGION by default. Both PROGBITS
# and NOBITS sections of BL31 image are adjacent to each other and loaded
# into Trusted SRAM.
SEPARATE_NOBITS_REGION		:=	0

# In order to support SEPARATE_NOBITS_REGION for Arm platforms, we need to load
# BL31 PROGBITS into secure DRAM space and BL31 NOBITS into SRAM. Hence mandate
# the build to require that ARM_BL31_IN_DRAM is enabled as well.
ifeq ($(SEPARATE_NOBITS_REGION),1)
    ifneq ($(ARM_BL31_IN_DRAM),1)
         $(error For SEPARATE_NOBITS_REGION, ARM_BL31_IN_DRAM must be enabled)
    endif
    ifneq ($(RECLAIM_INIT_CODE),0)
          $(error For SEPARATE_NOBITS_REGION, RECLAIM_INIT_CODE cannot be supported)
    endif
endif

# Disable ARM Cryptocell by default
ARM_CRYPTOCELL_INTEG		:=	0
$(eval $(call assert_boolean,ARM_CRYPTOCELL_INTEG))
$(eval $(call add_define,ARM_CRYPTOCELL_INTEG))

# Enable PIE support for RESET_TO_BL31/RESET_TO_SP_MIN case
ifneq ($(filter 1,${RESET_TO_BL31} ${RESET_TO_SP_MIN}),)
	ENABLE_PIE			:=	1
endif

# CryptoCell integration relies on coherent buffers for passing data from
# the AP CPU to the CryptoCell
ifeq (${ARM_CRYPTOCELL_INTEG},1)
    ifeq (${USE_COHERENT_MEM},0)
        $(error "ARM_CRYPTOCELL_INTEG needs USE_COHERENT_MEM to be set.")
    endif
endif

# Disable GPT parser support, use FIP image by default
ARM_GPT_SUPPORT			:=	0
$(eval $(call assert_boolean,ARM_GPT_SUPPORT))
$(eval $(call add_define,ARM_GPT_SUPPORT))

# Include necessary sources to parse GPT image
ifeq (${ARM_GPT_SUPPORT}, 1)
  BL2_SOURCES	+=	drivers/partition/gpt.c		\
			drivers/partition/partition.c
endif

# Enable CRC instructions via extension for ARMv8-A CPUs.
# For ARMv8.1-A, and onwards CRC instructions are default enabled.
# Enable HW computed CRC support unconditionally in BL2 component.
ifeq (${ARM_ARCH_MINOR},0)
  BL2_CPPFLAGS += -march=armv8-a+crc
endif

ifeq ($(PSA_FWU_SUPPORT),1)
    # GPT support is recommended as per PSA FWU specification hence
    # PSA FWU implementation is tightly coupled with GPT support,
    # and it does not support other formats.
    ifneq ($(ARM_GPT_SUPPORT),1)
      $(error For PSA_FWU_SUPPORT, ARM_GPT_SUPPORT must be enabled)
    endif
    FWU_MK := drivers/fwu/fwu.mk
    $(info Including ${FWU_MK})
    include ${FWU_MK}
endif

ifeq (${ARCH}, aarch64)
PLAT_INCLUDES		+=	-Iinclude/plat/arm/common/aarch64
endif

PLAT_BL_COMMON_SOURCES	+=	plat/arm/common/${ARCH}/arm_helpers.S		\
				plat/arm/common/arm_common.c			\
				plat/arm/common/arm_console.c

ifeq (${ARM_XLAT_TABLES_LIB_V1}, 1)
PLAT_BL_COMMON_SOURCES 	+=	lib/xlat_tables/xlat_tables_common.c	      \
				lib/xlat_tables/${ARCH}/xlat_tables.c
else
ifeq (${XLAT_MPU_LIB_V1}, 1)
include lib/xlat_mpu/xlat_mpu.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_MPU_LIB_V1_SRCS}
else
include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=      ${XLAT_TABLES_LIB_SRCS}
endif
endif

ARM_IO_SOURCES		+=	plat/arm/common/arm_io_storage.c		\
				plat/arm/common/fconf/arm_fconf_io.c
ifeq (${SPD},spmd)
    ifeq (${BL2_ENABLE_SP_LOAD},1)
         ARM_IO_SOURCES		+=	plat/arm/common/fconf/arm_fconf_sp.c
    endif
endif

BL1_SOURCES		+=	drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/arm/common/arm_bl1_setup.c			\
				plat/arm/common/arm_err.c			\
				${ARM_IO_SOURCES}

ifdef EL3_PAYLOAD_BASE
# Need the plat_arm_program_trusted_mailbox() function to release secondary CPUs from
# their holding pen
BL1_SOURCES		+=	plat/arm/common/arm_pm.c
endif

BL2_SOURCES		+=	drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/arm/common/arm_bl2_setup.c			\
				plat/arm/common/arm_err.c			\
				common/tf_crc32.c				\
				${ARM_IO_SOURCES}

# Firmware Configuration Framework sources
include lib/fconf/fconf.mk

BL1_SOURCES		+=	${FCONF_SOURCES} ${FCONF_DYN_SOURCES}
BL2_SOURCES		+=	${FCONF_SOURCES} ${FCONF_DYN_SOURCES}

# Add `libfdt` and Arm common helpers required for Dynamic Config
include lib/libfdt/libfdt.mk

DYN_CFG_SOURCES		+=	plat/arm/common/arm_dyn_cfg.c		\
				plat/arm/common/arm_dyn_cfg_helpers.c	\
				common/uuid.c

DYN_CFG_SOURCES		+=	${FDT_WRAPPERS_SOURCES}

BL1_SOURCES		+=	${DYN_CFG_SOURCES}
BL2_SOURCES		+=	${DYN_CFG_SOURCES}

ifeq (${BL2_AT_EL3},1)
BL2_SOURCES		+=	plat/arm/common/arm_bl2_el3_setup.c
endif

# Because BL1/BL2 execute in AArch64 mode but BL32 in AArch32 we need to use
# the AArch32 descriptors.
ifeq (${JUNO_AARCH32_EL3_RUNTIME},1)
BL2_SOURCES		+=	plat/arm/common/aarch32/arm_bl2_mem_params_desc.c
else
ifneq (${PLAT}, corstone1000)
BL2_SOURCES		+=	plat/arm/common/${ARCH}/arm_bl2_mem_params_desc.c
endif
endif
BL2_SOURCES		+=	plat/arm/common/arm_image_load.c		\
				common/desc_image_load.c
ifeq (${SPD},opteed)
BL2_SOURCES		+=	lib/optee/optee_utils.c
endif

BL2U_SOURCES		+=	drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				plat/arm/common/arm_bl2u_setup.c

BL31_SOURCES		+=	plat/arm/common/arm_bl31_setup.c		\
				plat/arm/common/arm_pm.c			\
				plat/arm/common/arm_topology.c			\
				plat/common/plat_psci_common.c

ifneq ($(filter 1,${ENABLE_PMF} ${ARM_ETHOSN_NPU_DRIVER}),)
ARM_SVC_HANDLER_SRCS :=

ifeq (${ENABLE_PMF},1)
ARM_SVC_HANDLER_SRCS	+=	lib/pmf/pmf_smc.c
endif

ifeq (${ARM_ETHOSN_NPU_DRIVER},1)
ARM_SVC_HANDLER_SRCS	+=	plat/arm/common/fconf/fconf_ethosn_getter.c	\
				drivers/delay_timer/delay_timer.c		\
				drivers/arm/ethosn/ethosn_smc.c
endif

ifeq (${ARCH}, aarch64)
BL31_SOURCES		+=	plat/arm/common/aarch64/execution_state_switch.c\
				plat/arm/common/arm_sip_svc.c			\
				${ARM_SVC_HANDLER_SRCS}
else
BL32_SOURCES		+=	plat/arm/common/arm_sip_svc.c			\
				${ARM_SVC_HANDLER_SRCS}
endif
endif

ifeq (${EL3_EXCEPTION_HANDLING},1)
BL31_SOURCES		+=	plat/common/aarch64/plat_ehf.c
endif

ifeq (${SDEI_SUPPORT},1)
BL31_SOURCES		+=	plat/arm/common/aarch64/arm_sdei.c
ifeq (${SDEI_IN_FCONF},1)
BL31_SOURCES		+=	plat/arm/common/fconf/fconf_sdei_getter.c
endif
endif

# RAS sources
ifeq (${RAS_EXTENSION},1)
BL31_SOURCES		+=	lib/extensions/ras/std_err_record.c		\
				lib/extensions/ras/ras_common.c
endif

# Pointer Authentication sources
ifeq (${ENABLE_PAUTH}, 1)
PLAT_BL_COMMON_SOURCES	+=	plat/arm/common/aarch64/arm_pauth.c	\
				lib/extensions/pauth/pauth_helpers.S
endif

ifeq (${SPD},spmd)
BL31_SOURCES		+=	plat/common/plat_spmd_manifest.c	\
				common/uuid.c				\
				${LIBFDT_SRCS}

BL31_SOURCES		+=	${FDT_WRAPPERS_SOURCES}
endif

ifneq (${TRUSTED_BOARD_BOOT},0)

    # Include common TBB sources
    AUTH_SOURCES 	:= 	drivers/auth/auth_mod.c	\
				drivers/auth/img_parser_mod.c

    # Include the selected chain of trust sources.
    ifeq (${COT},tbbr)
            BL1_SOURCES	+=	drivers/auth/tbbr/tbbr_cot_common.c		\
				drivers/auth/tbbr/tbbr_cot_bl1.c
        ifneq (${COT_DESC_IN_DTB},0)
            BL2_SOURCES	+=	lib/fconf/fconf_cot_getter.c
        else
            BL2_SOURCES	+=	drivers/auth/tbbr/tbbr_cot_common.c	\
				drivers/auth/tbbr/tbbr_cot_bl2.c
        endif
    else ifeq (${COT},dualroot)
        AUTH_SOURCES	+=	drivers/auth/dualroot/cot.c
    else ifeq (${COT},cca)
        AUTH_SOURCES	+=	drivers/auth/cca/cot.c
    else
        $(error Unknown chain of trust ${COT})
    endif

    BL1_SOURCES		+=	${AUTH_SOURCES}					\
				bl1/tbbr/tbbr_img_desc.c			\
				plat/arm/common/arm_bl1_fwu.c			\
				plat/common/tbbr/plat_tbbr.c

    BL2_SOURCES		+=	${AUTH_SOURCES}					\
				plat/common/tbbr/plat_tbbr.c

    $(eval $(call TOOL_ADD_IMG,ns_bl2u,--fwu,FWU_))

    IMG_PARSER_LIB_MK := drivers/auth/mbedtls/mbedtls_x509.mk

    $(info Including ${IMG_PARSER_LIB_MK})
    include ${IMG_PARSER_LIB_MK}
endif

# Include Measured Boot makefile before any Crypto library makefile.
# Crypto library makefile may need default definitions of Measured Boot build
# flags present in Measured Boot makefile.
ifeq (${MEASURED_BOOT},1)
    MEASURED_BOOT_MK := drivers/measured_boot/event_log/event_log.mk
    $(info Including ${MEASURED_BOOT_MK})
    include ${MEASURED_BOOT_MK}

    ifneq (${MBOOT_EL_HASH_ALG}, sha256)
        $(eval $(call add_define,TF_MBEDTLS_MBOOT_USE_SHA512))
    endif

    BL1_SOURCES		+= 	${EVENT_LOG_SOURCES}
    BL2_SOURCES		+= 	${EVENT_LOG_SOURCES}
endif

ifneq ($(filter 1,${MEASURED_BOOT} ${TRUSTED_BOARD_BOOT}),)
    CRYPTO_SOURCES	:=	drivers/auth/crypto_mod.c 	\
				lib/fconf/fconf_tbbr_getter.c
    BL1_SOURCES		+=	${CRYPTO_SOURCES}
    BL2_SOURCES		+=	${CRYPTO_SOURCES}

    # We expect to locate the *.mk files under the directories specified below
    ifeq (${ARM_CRYPTOCELL_INTEG},0)
        CRYPTO_LIB_MK := drivers/auth/mbedtls/mbedtls_crypto.mk
    else
        CRYPTO_LIB_MK := drivers/auth/cryptocell/cryptocell_crypto.mk
    endif

    $(info Including ${CRYPTO_LIB_MK})
    include ${CRYPTO_LIB_MK}
endif

ifeq (${RECLAIM_INIT_CODE}, 1)
    ifeq (${ARM_XLAT_TABLES_LIB_V1}, 1)
        $(error "To reclaim init code xlat tables v2 must be used")
    endif
endif

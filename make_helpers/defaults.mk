#
# Copyright (c) 2016-2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Default, static values for build variables, listed in alphabetic order.
# Dependencies between build options, if any, are handled in the top-level
# Makefile, after this file is included. This ensures that the former is better
# poised to handle dependencies, as all build variables would have a default
# value by then.

# Warning level to give to the compiler
W				:= 0

# Use T32 by default
AARCH32_INSTRUCTION_SET		:= T32

# The AArch32 Secure Payload to be built as BL32 image
AARCH32_SP			:= none

# The Target build architecture. Supported values are: aarch64, aarch32.
ARCH				:= aarch64

# ARM Architecture feature modifiers: none by default
ARM_ARCH_FEATURE		:= none

# ARM Architecture major and minor versions: 8.0 by default.
ARM_ARCH_MAJOR			:= 8
ARM_ARCH_MINOR			:= 0

# Base commit to perform code check on
BASE_COMMIT			:= origin/master

# Execute BL2 at EL3
RESET_TO_BL2			:= 0

# Only use SP packages if SP layout JSON is defined
BL2_ENABLE_SP_LOAD		:= 0

# BL2 image is stored in XIP memory, for now, this option is only supported
# when RESET_TO_BL2 is 1.
BL2_IN_XIP_MEM			:= 0

# Do dcache invalidate upon BL2 entry at EL3
BL2_INV_DCACHE			:= 1

# Select the branch protection features to use.
BRANCH_PROTECTION		:= 0

# By default, consider that the platform may release several CPUs out of reset.
# The platform Makefile is free to override this value.
COLD_BOOT_SINGLE_CPU		:= 0

# Flag to compile in coreboot support code. Exclude by default. The coreboot
# Makefile system will set this when compiling TF as part of a coreboot image.
COREBOOT			:= 0

# For Chain of Trust
CREATE_KEYS			:= 1

# Build flag to include AArch32 registers in cpu context save and restore during
# world switch. This flag must be set to 0 for AArch64-only platforms.
CTX_INCLUDE_AARCH32_REGS	:= 1

# Include FP registers in cpu context
CTX_INCLUDE_FPREGS		:= 0

# Include SVE registers in cpu context
CTX_INCLUDE_SVE_REGS		:= 0

# Debug build
DEBUG				:= 0

# By default disable authenticated decryption support.
DECRYPTION_SUPPORT		:= none

# Build platform
DEFAULT_PLAT			:= fvp

# Disable the generation of the binary image (ELF only).
DISABLE_BIN_GENERATION		:= 0

# Enable capability to disable authentication dynamically. Only meant for
# development platforms.
DYN_DISABLE_AUTH		:= 0

# Enable the SIMD crypto extension feature. The flags suppose to be in
# arch_features.mk but since mbedtls_common.mk is included before arch_features.mk,
# so this flag has to be defined here.
ENABLE_FEAT_CRYPTO		:= 0

# Enable the SIMD SHA3 crypto extension feature.
ENABLE_FEAT_CRYPTO_SHA3		:= 0

# Enable the Maximum Power Mitigation Mechanism on supporting cores.
ENABLE_MPMM			:= 0

# Flag to Enable Position Independant support (PIE)
ENABLE_PIE			:= 0

# Flag to enable Performance Measurement Framework
ENABLE_PMF			:= 0

# Flag to enable PSCI STATs functionality
ENABLE_PSCI_STAT		:= 0

# Flag to enable runtime instrumentation using PMF
ENABLE_RUNTIME_INSTRUMENTATION	:= 0

# Flag to enable stack corruption protection
ENABLE_STACK_PROTECTOR		:= 0

# Flag to enable exception handling in EL3
EL3_EXCEPTION_HANDLING		:= 0

# Flag to include all errata for all CPUs TF-A implements workarounds for
# Its supposed to be used only for testing.
ENABLE_ERRATA_ALL		:= 0

# By default BL31 encryption disabled
ENCRYPT_BL31			:= 0

# By default BL32 encryption disabled
ENCRYPT_BL32			:= 0

# Default dummy firmware encryption key
ENC_KEY	:= 1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef

# Default dummy nonce for firmware encryption
ENC_NONCE			:= 1234567890abcdef12345678

# Build flag to treat usage of deprecated platform and framework APIs as error.
ERROR_DEPRECATED		:= 0

# Fault injection support
FAULT_INJECTION_SUPPORT		:= 0

# Flag to enable architectural features detection mechanism
FEATURE_DETECTION		:= 0

# Byte alignment that each component in FIP is aligned to
FIP_ALIGN			:= 0

# Default FIP file name
FIP_NAME			:= fip.bin

# Default FWU_FIP file name
FWU_FIP_NAME			:= fwu_fip.bin

# Default BL2 FIP file name
BL2_FIP_NAME			:= bl2_fip.bin

# By default firmware encryption with SSK
FW_ENC_STATUS			:= 0

# For Chain of Trust
GENERATE_COT			:= 0

# Default number of 512 blocks per bitlock
RME_GPT_BITLOCK_BLOCK		:= 1

# Default maximum size of GPT contiguous block
RME_GPT_MAX_BLOCK		:= 512

# Hint platform interrupt control layer that Group 0 interrupts are for EL3. By
# default, they are for Secure EL1.
GICV2_G0_FOR_EL3		:= 0

# Generic implementation of a GICvX driver
USE_GIC_DRIVER			:= 0

# Route NS External Aborts to EL3. Disabled by default; External Aborts are handled
# by lower ELs.
HANDLE_EA_EL3_FIRST_NS		:= 0

# Enable Handoff protocol using transfer lists
TRANSFER_LIST			:= 0

# Enable HOB list to generate boot information
HOB_LIST			:= 0

# Enables support for the gcc compiler option "-mharden-sls=all".
# By default, disables all SLS hardening.
HARDEN_SLS			:= 0

# Secure hash algorithm flag, accepts 3 values: sha256, sha384 and sha512.
# The default value is sha256.
HASH_ALG			:= sha256

# Whether system coherency is managed in hardware, without explicit software
# operations.
HW_ASSISTED_COHERENCY		:= 0

# Flag to enable trapping of implementation defined sytem registers
IMPDEF_SYSREG_TRAP		:= 0

# Set the default algorithm for the generation of Trusted Board Boot keys
KEY_ALG				:= rsa

# Set the default key size in case KEY_ALG is rsa
ifeq ($(KEY_ALG),rsa)
KEY_SIZE			:= 2048
endif

# Option to build TF with Measured Boot support
MEASURED_BOOT			:= 0

# Option to build TF with Discrete TPM support
DISCRETE_TPM			:= 0

# Option to enable the DICE Protection Environmnet as a Measured Boot backend
DICE_PROTECTION_ENVIRONMENT	:=0

# NS timer register save and restore (deprecated)
NS_TIMER_SWITCH			:= 0

# Include lib/libc in the final image
OVERRIDE_LIBC			:= 0

# Build PL011 UART driver in minimal generic UART mode
PL011_GENERIC_UART		:= 0

# By default, consider that the platform's reset address is not programmable.
# The platform Makefile is free to override this value.
PROGRAMMABLE_RESET_ADDRESS	:= 0

# Flag used to choose the power state format: Extended State-ID or Original
PSCI_EXTENDED_STATE_ID		:= 0

# Enable PSCI OS-initiated mode support
PSCI_OS_INIT_MODE		:= 0

# SMCCC_ARCH_FEATURE_AVAILABILITY support
ARCH_FEATURE_AVAILABILITY	:= 0

# By default, BL1 acts as the reset handler, not BL31
RESET_TO_BL31			:= 0

# For Chain of Trust
SAVE_KEYS			:= 0

# Software Delegated Exception support
SDEI_SUPPORT			:= 0

# Number of UUIDs allowed for a physical partition
SPMC_AT_EL3_PARTITION_MAX_UUIDS := 4

# True Random Number firmware Interface support
TRNG_SUPPORT			:= 0

# Check to see if Errata ABI is supported
ERRATA_ABI_SUPPORT		:= 0

# Check to enable Errata ABI for platforms with non-arm interconnect
ERRATA_NON_ARM_INTERCONNECT	:= 0

# SMCCC PCI support
SMC_PCI_SUPPORT			:= 0

# Whether code and read-only data should be put on separate memory pages. The
# platform Makefile is free to override this value.
SEPARATE_CODE_AND_RODATA	:= 0

# Put NOBITS sections (.bss, stacks, page tables, and coherent memory) in a
# separate memory region, which may be discontiguous from the rest of BL31.
SEPARATE_NOBITS_REGION		:= 0

# Put BL2 NOLOAD sections (.bss, stacks, page tables) in a separate memory
# region, platform Makefile is free to override this value.
SEPARATE_BL2_NOLOAD_REGION	:= 0

# Put RW DATA sections (.rwdata) in a separate memory region, which may be
# discontiguous from the rest of BL31.
SEPARATE_RWDATA_REGION		:= 0

# Put SIMD context data structures in a separate memory region. Platforms
# have the choice to put it outside of default BSS region of EL3 firmware.
SEPARATE_SIMD_SECTION		:= 0

# If the BL31 image initialisation code is recalimed after use for the secondary
# cores stack
RECLAIM_INIT_CODE		:= 0

# SPD choice
SPD				:= none

# Enable the Management Mode (MM)-based Secure Partition Manager implementation
SPM_MM				:= 0

# Use the FF-A SPMC implementation in EL3.
SPMC_AT_EL3			:= 0

# Enable SEL0 SP when SPMC is enabled at EL3
SPMC_AT_EL3_SEL0_SP		:=0

# Use SPM at S-EL2 as a default config for SPMD
SPMD_SPM_AT_SEL2		:= 1

# Flag to introduce an infinite loop in BL1 just before it exits into the next
# image. This is meant to help debugging the post-BL2 phase.
SPIN_ON_BL1_EXIT		:= 0

# Flags to build TF with Trusted Boot support
TRUSTED_BOARD_BOOT		:= 0

# Build option to choose whether Trusted Firmware uses Coherent memory or not.
USE_COHERENT_MEM		:= 1

# Build option to add debugfs support
USE_DEBUGFS			:= 0

# Build option to enable passing the FDT in x0 to BL33, following the kernel
# convention.
USE_KERNEL_DT_CONVENTION	:= 0

# Build option to fconf based io
ARM_IO_IN_DTB			:= 0

# Build option to support SDEI through fconf
SDEI_IN_FCONF			:= 0

# Build option to support Secure Interrupt descriptors through fconf
SEC_INT_DESC_IN_FCONF		:= 0

# Build option to choose whether Trusted Firmware uses library at ROM
USE_ROMLIB			:= 0

# Build option to choose whether the xlat tables of BL images can be read-only.
# Note that this only serves as a higher level option to PLAT_RO_XLAT_TABLES,
# which is the per BL-image option that actually enables the read-only tables
# API. The reason for having this additional option is to have a common high
# level makefile where we can check for incompatible features/build options.
ALLOW_RO_XLAT_TABLES		:= 0

# Chain of trust.
COT				:= tbbr

# Use tbbr_oid.h instead of platform_oid.h
USE_TBBR_DEFS			:= 1

# Whether to enable D-Cache early during warm boot. This is usually
# applicable for platforms wherein interconnect programming is not
# required to enable cache coherency after warm reset (eg: single cluster
# platforms).
WARMBOOT_ENABLE_DCACHE_EARLY	:= 0

# Default SVE vector length to maximum architected value
SVE_VECTOR_LEN			:= 2048

SANITIZE_UB := off

# Enable Link Time Optimization
ENABLE_LTO			:= 0

# This option will include EL2 registers in cpu context save and restore during
# EL2 firmware entry/exit. Internal flag not meant for direct setting.
# Use SPD=spmd and SPMD_SPM_AT_SEL2=1 or ENABLE_RMM=1 to enable
# CTX_INCLUDE_EL2_REGS.
CTX_INCLUDE_EL2_REGS		:= 0

# Select workaround for AT speculative behaviour.
ERRATA_SPECULATIVE_AT		:= 0

# Trap RAS error record access from Non secure
RAS_TRAP_NS_ERR_REC_ACCESS	:= 0

# Build option to create cot descriptors using fconf
COT_DESC_IN_DTB			:= 0

# Build option to provide OpenSSL directory path
OPENSSL_DIR			:= /usr

# Select the openssl binary provided in OPENSSL_DIR variable
ifeq ("$(wildcard ${OPENSSL_DIR}/bin)", "")
    OPENSSL_BIN_PATH = ${OPENSSL_DIR}/apps
else
    OPENSSL_BIN_PATH = ${OPENSSL_DIR}/bin
endif

# Build option to use the SP804 timer instead of the generic one
USE_SP804_TIMER			:= 0

# Build option to define number of firmware banks, used in firmware update
# metadata structure.
NR_OF_FW_BANKS			:= 2

# Build option to define number of images in firmware bank, used in firmware
# update metadata structure.
NR_OF_IMAGES_IN_FW_BANK		:= 1

# Disable Firmware update support by default
PSA_FWU_SUPPORT			:= 0

# Enable image description in FWU metadata by default when PSA_FWU_SUPPORT
# is enabled.
ifeq ($(PSA_FWU_SUPPORT),1)
PSA_FWU_METADATA_FW_STORE_DESC	:= 1
else
PSA_FWU_METADATA_FW_STORE_DESC	:= 0
endif

# Dynamic Root of Trust for Measurement support
DRTM_SUPPORT			:= 0

# Check platform if cache management operations should be performed.
# Disabled by default.
CONDITIONAL_CMO			:= 0

# By default, disable SPMD Logical partitions
ENABLE_SPMD_LP			:= 0

# By default, disable PSA crypto (use MbedTLS legacy crypto API).
PSA_CRYPTO			:= 0

# getc() support from the console(s).
# Disabled by default because it constitutes an attack vector into TF-A. It
# should only be enabled if there is a use case for it.
ENABLE_CONSOLE_GETC		:= 0

# Build option to disable EL2 when it is not used.
# Most platforms switch from EL3 to NS-EL2 and hence the unused NS-EL2
# functions must be enabled by platforms if they require it.
# Disabled by default.
INIT_UNUSED_NS_EL2		:= 0

# Disable including MPAM EL2 registers in context by default since currently
# it's only enabled for NS world
CTX_INCLUDE_MPAM_REGS		:= 0

# Enable context memory usage reporting during BL31 setup.
PLATFORM_REPORT_CTX_MEM_USE	:= 0

# Request a custom addition to the BL31 linker script
PLAT_EXTRA_LD_SCRIPT		:= 0

# Enable early console
EARLY_CONSOLE			:= 0

# Allow platforms to save/restore DSU PMU registers over a power cycle.
# Disabled by default and must be enabled by individual platforms.
PRESERVE_DSU_PMU_REGS		:= 0

# Flag to enable an RME payload
ENABLE_RMM			:= 0

# Enable RMMD to forward attestation requests from RMM to EL3.
RMMD_ENABLE_EL3_TOKEN_SIGN	:= 0

# Enable RMMD to program and manage IDE Keys at the PCIe Root Port(RP).
# This flag is temporary and it is expected once the interface is
# finalized, this flag will be removed.
RMMD_ENABLE_IDE_KEY_PROG	:= 0

# Enable RMM v1.x compatibility mode
RMM_V1_COMPAT			:= 0

# Live firmware activation support
LFA_SUPPORT			:= 0
ENABLE_LFA_BL31			:= 0

# Enable support for arm DSU driver.
USE_DSU_DRIVER			:= 0

# Define the separation of BL2 flag, by default it is disabled.
SEPARATE_BL2_FIP		:=	0

# Disable NUMA awareness for per-CPU framework by default. Platforms should
# enable this feature by setting PLATFORM_NODE_COUNT > 1
PLATFORM_NODE_COUNT		:= 1

# Support for live activation of SPs managed by S-EL2 SPMC
SUPPORT_SP_LIVE_ACTIVATION	:= 0

# Negative I/O test: intentionally report a short read for a selected
# image_id. Test/CI only. Do not enable in production builds.
TEST_IO_SHORT_READ_FI		:= 0
TEST_IO_SHORT_READ_FI_IMAGE_ID	:= 0

# Enable the FIRME interface.
FIRME_SUPPORT			:= 0

# FIRME IDE KM support.
FIRME_SUPPORT_IDE_KM		:= 0

# Flag to enable the spinlock implementation variant using the FEAT_LSE
# compare-and-swap instruction.
USE_SPINLOCK_CAS		?=	0

#----
# 8.1
#----

# Flag to enable access to Privileged Access Never bit of PSTATE.
ENABLE_FEAT_PAN			?=	0

# Flag to enable Virtualization Host Extensions.
ENABLE_FEAT_VHE			?=	0

#----
# 8.2
#----

# Enable RAS Support.
ENABLE_FEAT_RAS			?=	0

#----
# 8.3
#----

# Flag to enable Pointer Authentication. Internal flag not meant for
# direct setting. Use BRANCH_PROTECTION to enable PAUTH.
ENABLE_PAUTH			?=	0

# FEAT_PAUTH_LR is an optional architectural feature, so this flag must be set
# manually in addition to the BRANCH_PROTECTION flag which is used for other
# branch protection and pointer authentication features.
ENABLE_FEAT_PAUTH_LR		?=	0

# Include pointer authentication (ARMv8.3-PAuth) registers in cpu context. This
# must be set to 1 if the platform wants to use this feature in the Secure
# world. It is not necessary for use in the Non-secure world.
CTX_INCLUDE_PAUTH_REGS		?=	0


#----
# 8.4
#----

# Flag to enable Secure EL-2 feature.
ENABLE_FEAT_SEL2		?=	0

# By default, disable trace filter control register access to lower non-secure
# exception levels, i.e. NS-EL2, or NS-EL1 if NS-EL2 is implemented, but
# trace filter control register access is unused if FEAT_TRF is implemented.
ENABLE_TRF_FOR_NS		?=	0

# Flag to enable Data Independent Timing instructions.
ENABLE_FEAT_DIT			?=	0

#----
# 8.5
#----

# Flag to enable Branch Target Identification.
# Internal flag not meant for direct setting.
# Use BRANCH_PROTECTION to enable BTI.
ENABLE_BTI			?=	0

# Flag to enable access to the Random Number Generator registers.
ENABLE_FEAT_RNG			?=	0

# Flag to enable Speculation Barrier Instruction.
ENABLE_FEAT_SB			?=	0

#----
# 8.6
#----

# Flag to enable access to the CNTPOFF_EL2 register.
ENABLE_FEAT_ECV			?=	0

# Flag to enable access to the HDFGRTR_EL2 register.
ENABLE_FEAT_FGT			?=	0

#----
# 8.7
#----

# Flag to enable access to the HCRX_EL2 register by setting SCR_EL3.HXEn.
ENABLE_FEAT_HCX			?=	0

#----
# 8.8
#----

# Flag to enable FEAT_MOPS (Standardization of Memory operations)
# when INIT_UNUSED_NS_EL2 = 1
ENABLE_FEAT_MOPS		?=	0

#----
# 8.9
#----

# Flag to enable access to TCR2 (FEAT_TCR2).
ENABLE_FEAT_TCR2		?=	0

# Flag to enable access to SCTLR2 (FEAT_SCTLR2).
ENABLE_FEAT_SCTLR2		?=	0

#
################################################################################
# Optional Features defaulted to 0 or 2, if they are not enabled from
# build option. Can also be disabled or enabled by platform if needed.
################################################################################
#

#----
# 8.0
#----

# Flag to enable support for clrbhb instruction.
ENABLE_FEAT_CLRBHB			?=	0

# Flag to enable CSV2_2 extension.
ENABLE_FEAT_CSV2_2			?=	0

# Flag to enable CSV2_3 extension. FEAT_CSV2_3 enables access to the
# SCXTNUM_ELx register.
ENABLE_FEAT_CSV2_3			?=	0

# By default, disable access of trace system registers from NS lower
# ELs  i.e. NS-EL2, or NS-EL1 if NS-EL2 implemented but unused if
# system register trace is implemented. This feature is available if
# trace unit such as ETMv4.x, This feature is OPTIONAL and is only
# permitted in Armv8 implementations.
ENABLE_SYS_REG_TRACE_FOR_NS		?=	0

#----
# 8.2
#----

# Build option to enable/disable the Statistical Profiling Extension,
# keep it enabled by default for AArch64.
ifeq (${ARCH},aarch64)
       ENABLE_SPE_FOR_NS		?=	2
else ifeq (${ARCH},aarch32)
       ENABLE_SPE_FOR_NS		:=	0
endif

# Enable SVE for non-secure world by default.
ifeq (${ARCH},aarch64)
       ENABLE_SVE_FOR_NS		?=	2
# SVE is only supported on AArch64 so disable it on AArch32.
else ifeq (${ARCH},aarch32)
       ENABLE_SVE_FOR_NS		:=	0
endif

#----
# 8.4
#----

# Feature flags for supporting Activity monitor extensions.
ENABLE_FEAT_AMU				?=	0
ENABLE_AMU_AUXILIARY_COUNTERS		?=	0
AMU_RESTRICT_COUNTERS			?=	1

# Build option to enable MPAM for lower ELs.
# Enabling it by default
ifeq (${ARCH},aarch64)
        ENABLE_FEAT_MPAM		?=	2
else ifeq (${ARCH},aarch32)
        ENABLE_FEAT_MPAM		:=	0
endif

# Include nested virtualization control (Armv8.4-NV) registers in cpu context.
# This must be set to 1 if architecture implements Nested Virtualization
# Extension and platform wants to use this feature in the Secure world.
CTX_INCLUDE_NEVE_REGS			?=	0

#----
# 8.5
#----

# Flag to enable support for EL3 trapping of reads of the RNDR and RNDRRS
# registers, by setting SCR_EL3.TRNDR.
ENABLE_FEAT_RNG_TRAP			?=	0

# Enable FEAT_MTE2. This must be set to 1 if the platform wants
# to use this feature and is enabled at ELX.
ENABLE_FEAT_MTE2		        ?=	0

#----
# 8.6
#----

# Flag to enable AMUv1p1 extension.
ENABLE_FEAT_AMUv1p1			?=	0

# Flag to enable delayed trapping of WFE instruction (FEAT_TWED).
ENABLE_FEAT_TWED			?=	0

# In v8.6+ platforms with delayed trapping of WFE being supported
# via FEAT_TWED, this flag takes the delay value to be set in the
# SCR_EL3.TWEDEL(4bit) field, when FEAT_TWED is implemented.
# By default it takes 0, and need to be updated by the platforms.
TWED_DELAY				?=	0

# Disable MTPMU if FEAT_MTPMU is supported.
DISABLE_MTPMU				?=	0

# Flag to enable FEAT_FGT2 (Fine Granular Traps 2)
ENABLE_FEAT_FGT2			?=	0

# LoadStore64Bytes extension using the ACCDATA_EL1 system register
ENABLE_FEAT_LS64_ACCDATA		?=	0

#----
# 8.8
#----

# Flag to enable FEAT_THE (Translation Hardening Extension)
ENABLE_FEAT_THE				?=	0

#----
# 8.9
#----

# Flag to enable access to Stage 2 Permission Indirection (FEAT_S2PIE).
ENABLE_FEAT_S2PIE			?=	0

# Flag to enable access to Stage 1 Permission Indirection (FEAT_S1PIE).
ENABLE_FEAT_S1PIE			?=	0

# Flag to enable access to Stage 2 Permission Overlay (FEAT_S2POE).
ENABLE_FEAT_S2POE			?=	0

# Flag to enable access to Stage 1 Permission Overlay (FEAT_S1POE).
ENABLE_FEAT_S1POE			?=	0

# Flag to enable access to Arm v8.9 Debug extension
ENABLE_FEAT_DEBUGV8P9			?=	0

# AIE extension using the (A)MAIR2 system registers
ENABLE_FEAT_AIE				?=	0

# PFAR extension using the PFAR system registers
ENABLE_FEAT_PFAR			?=	0

# Enable asynchronous Device and Normal Error Exceptions.
ENABLE_FEAT_AxERR			?=	0

#-------------------------------------------------------------
# Non-standard feature
#-------------------------------------------------------------
ENABLE_FEAT_MORELLO			?=	0

#----
# 9.0
#----

# Scalable Matrix Extension for non-secure world.
ENABLE_SME_FOR_NS			?=	0

# Scalable Vector Extension for secure world.
ENABLE_SVE_FOR_SWD			?=	0

# By default, disable access of trace buffer control registers from NS
# lower ELs  i.e. NS-EL2, or NS-EL1 if NS-EL2 implemented but unused
# if FEAT_TRBE is implemented.
# Note FEAT_TRBE is only supported on AArch64 - therefore do not enable in
# AArch32.
ifeq (${ARCH},aarch64)
        ENABLE_TRBE_FOR_NS		?=	0
else ifeq (${ARCH},aarch32)
        ifneq ($(or $(ENABLE_TRBE_FOR_NS),0),0)
               $(error ENABLE_TRBE_FOR_NS is not supported for AArch32)
        else
               ENABLE_TRBE_FOR_NS 	:=	0
        endif
endif

# Flag that enables hardware injection of undefined exceptions
ENABLE_FEAT_UINJ			?=	0

#----
# 9.2
#----

# Flag to enable Realm Management Extension (FEAT_RME).
ENABLE_FEAT_RME				?=	0

# Scalable Matrix Extension version 2 for non-secure world.
ENABLE_SME2_FOR_NS			?=	0

# Scalable Matrix Extension for secure world.
ENABLE_SME_FOR_SWD			?=	0

# By default, disable access to branch record buffer control registers from NS
# lower ELs i.e. NS-EL2, or NS-EL1 if NS-EL2 implemented but unused
# if FEAT_BRBE is implemented.
ENABLE_BRBE_FOR_NS			?=	0

# Flag to enable Branch Recording at EL3
ENABLE_FEAT_BRBEV1P1			?=	0

# Flag to enable Floating point exception Mode Register Feature (FEAT_FPMR)
ENABLE_FEAT_FPMR			?=	0

# Flag to enable Memory Encryption Contexts (FEAT_MEC).
ENABLE_FEAT_MEC				?=	0

#----
# 9.3
#----
# Flag to enable access to Arm v9.3 FEAT_D128 extension
ENABLE_FEAT_D128			?=	0

# Flag to enable access to GICv5 CPU interface extension (FEAT_GCIE)
ENABLE_FEAT_GCIE			?=	0

# Enables access to PE-side MPAM bandwidth controls (FEAT_MPAM_PE_BW_CTRL)
ENABLE_FEAT_MPAM_PE_BW_CTRL		?=	0

# Flag to enable Exception-based Event Profiling (FEAT_EBEP)
ENABLE_FEAT_EBEP			?=	0

#----
#9.4
#----

# Flag to enable FEAT_RME_GDI
ENABLE_FEAT_RME_GDI			?=	0

# Flag to enable access to Guarded Control Stack (FEAT_GCS).
ENABLE_FEAT_GCS				?=	0

# Flag to enable Fine Grained Write Traps (FEAT_FGWTE3) for EL3.
ENABLE_FEAT_FGWTE3			?=	0

# Flag to enable checked pointer arithmetic (FEAT_CPA2) for EL3.
# We don't have a flag for FEAT_CPA since that has no effect on software
ENABLE_FEAT_CPA2			?=	0

# Flag to enable Enhanced Software Step Extension (FEAT_STEP2)
ENABLE_FEAT_STEP2			?=	0

# Flag to enable Hardware Dirty state tracking structure (FEAT_HDBSS).
ENABLE_FEAT_HDBSS			?=	0

# Flag to enable Hardening Address and Context Debug Banked State (FEAT_HACDBS).
ENABLE_FEAT_HACDBS			?=	0

# Flag to enable SPEv1p5 support (FEAT_SPE_EXC and FEAT_SPE_nVM)
ENABLE_FEAT_SPEV1P5			?=	0

#----
#9.6
#----

# Flag to enable trapping of ID registers to EL3
ENABLE_FEAT_IDTE3                       ?=      0

# Flag to enable EL1 control register aliases and bitmasks (FEAT_SRMASK).
ENABLE_FEAT_SRMASK			?=	0
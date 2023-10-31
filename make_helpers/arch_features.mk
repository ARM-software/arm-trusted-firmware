#
# Copyright (c) 2022-2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# This file lists all of the architectural features, and initializes
# and enables them based on the configured architecture version.

# This file follows the following format:
#   - Enable mandatory feature if applicable to an Arch Version.
#   - By default disable any mandatory features if they have not been defined yet.
#   - Disable or enable any optional feature this would be enabled/disabled if needed by platform.

#
################################################################################
# Enable Mandatory features based on Arch versions.
################################################################################
#

# Enable the features which are mandatory from ARCH version 8.1 and upwards.
ifeq "8.1" "$(word 1, $(sort 8.1 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_PAN				:=	1
ENABLE_FEAT_VHE				:=	1
endif

# Enable the features which are mandatory from ARCH version 8.2 and upwards.
ifeq "8.2" "$(word 1, $(sort 8.2 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_RAS				:=	1
endif

# Enable the features which are mandatory from ARCH version 8.4 and upwards.
ifeq "8.4" "$(word 1, $(sort 8.4 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_SEL2			:=	1
ENABLE_TRF_FOR_NS			:=	1
ENABLE_FEAT_DIT				:=	1
endif

# Enable the features which are mandatory from ARCH version 8.5 and upwards.
ifeq "8.5" "$(word 1, $(sort 8.5 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_RNG				:=	1
ENABLE_FEAT_SB				:=	1

# Enable Memory tagging, Branch Target Identification for aarch64 only.
ifeq ($(ARCH), aarch64)
	mem_tag_arch_support		:= 	yes
endif #(ARCH=aarch64)

endif

# Enable the features which are mandatory from ARCH version 8.6 and upwards.
ifeq "8.6" "$(word 1, $(sort 8.6 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_ECV				:=	1
ENABLE_FEAT_FGT				:=	1
endif

# Enable the features which are mandatory from ARCH version 8.7 and upwards.
ifeq "8.7" "$(word 1, $(sort 8.7 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_HCX				:=	1
endif

# Enable the features which are mandatory from ARCH version 8.9 and upwards.
ifeq "8.9" "$(word 1, $(sort 8.9 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_TCR2			:=	1
endif

#
################################################################################
# Set mandatory features by default to zero.
################################################################################
#

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
# 8.9
#----

# Flag to enable access to TCR2 (FEAT_TCR2).
ENABLE_FEAT_TCR2		?=	0

#
################################################################################
# Optional Features defaulted to 0 or 2, if they are not enabled from
# build option. Can also be disabled or enabled by platform if needed.
################################################################################
#

#----
# 8.0
#----

# Flag to enable CSV2_2 extension.
ENABLE_FEAT_CSV2_2			?=	0

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
       ifneq ($(or $(ENABLE_SPE_FOR_NS),0),0)
              $(error ENABLE_SPE_FOR_NS is not supported for AArch32)
       else
              ENABLE_SPE_FOR_NS		:=	0
       endif
endif

# Enable SVE for non-secure world by default.
ifeq (${ARCH},aarch64)
       ENABLE_SVE_FOR_NS		?=	2
# SVE is only supported on AArch64 so disable it on AArch32.
else ifeq (${ARCH},aarch32)
       ifneq ($(or $(ENABLE_SVE_FOR_NS),0),0)
              $(error ENABLE_SVE_FOR_NS is not supported for AArch32)
       else
              ENABLE_SVE_FOR_NS 	:=	0
       endif
endif

#----
# 8.4
#----

# Feature flags for supporting Activity monitor extensions.
ENABLE_FEAT_AMU				?=	0
ENABLE_AMU_AUXILIARY_COUNTERS		?=	0
ENABLE_AMU_FCONF			?=	0
AMU_RESTRICT_COUNTERS			?=	0

# Build option to enable MPAM for lower ELs.
# Enabling it by default
ifeq (${ARCH},aarch64)
        ENABLE_FEAT_MPAM		?=	2
else ifeq (${ARCH},aarch32)
        ifneq ($(or $(ENABLE_FEAT_MPAM),0),0)
                $(error ENABLE_FEAT_MPAM is not supported for AArch32)
        else
                ENABLE_FEAT_MPAM	:=	0
        endif
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

# Include Memory Tagging Extension registers in cpu context. This must be set
# to 1 if the platform wants to use this feature in the Secure world and MTE is
# enabled at ELX.
CTX_INCLUDE_MTE_REGS			?=	0

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

#----
# 8.9
#----

# Flag to enable NoTagAccess memory region attribute for stage 2 of translation.
ENABLE_FEAT_MTE_PERM			?=	0

# Flag to enable access to Stage 2 Permission Indirection (FEAT_S2PIE).
ENABLE_FEAT_S2PIE			?=	0

# Flag to enable access to Stage 1 Permission Indirection (FEAT_S1PIE).
ENABLE_FEAT_S1PIE			?=	0

# Flag to enable access to Stage 2 Permission Overlay (FEAT_S2POE).
ENABLE_FEAT_S2POE			?=	0

# Flag to enable access to Stage 1 Permission Overlay (FEAT_S1POE).
ENABLE_FEAT_S1POE			?=	0

#----
# 9.0
#----

# Flag to enable Realm Management Extension (FEAT_RME).
ENABLE_RME				?=	0

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

#----
# 9.2
#----

# Scalable Matrix Extension version 2 for non-secure world.
ENABLE_SME2_FOR_NS			?=	0

# Scalable Matrix Extension for secure world.
ENABLE_SME_FOR_SWD			?=	0

# By default, disable access to branch record buffer control registers from NS
# lower ELs i.e. NS-EL2, or NS-EL1 if NS-EL2 implemented but unused
# if FEAT_BRBE is implemented.
ENABLE_BRBE_FOR_NS			?=	0

#----
#9.4
#----

# Flag to enable access to Guarded Control Stack (FEAT_GCS).
ENABLE_FEAT_GCS				?=	0

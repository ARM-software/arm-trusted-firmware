#
# Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Default, static values for build variables, listed in alphabetic order.
# Dependencies between build options, if any, are handled in the top-level
# Makefile, after this file is included. This ensures that the former is better
# poised to handle dependencies, as all build variables would have a default
# value by then.

# The AArch32 Secure Payload to be built as BL32 image
AARCH32_SP			:= none

# The Target build architecture. Supported values are: aarch64, aarch32.
ARCH				:= aarch64

# ARM Architecture major and minor versions: 8.0 by default.
ARM_ARCH_MAJOR			:= 8
ARM_ARCH_MINOR			:= 0

# Determine the version of ARM GIC architecture to use for interrupt management
# in EL3. The platform port can change this value if needed.
ARM_GIC_ARCH			:= 2

# Base commit to perform code check on
BASE_COMMIT			:= origin/master

# Execute BL2 at EL3
BL2_AT_EL3			:= 0

# By default, consider that the platform may release several CPUs out of reset.
# The platform Makefile is free to override this value.
COLD_BOOT_SINGLE_CPU		:= 0

# For Chain of Trust
CREATE_KEYS			:= 1

# Build flag to include AArch32 registers in cpu context save and restore during
# world switch. This flag must be set to 0 for AArch64-only platforms.
CTX_INCLUDE_AARCH32_REGS	:= 1

# Include FP registers in cpu context
CTX_INCLUDE_FPREGS		:= 0

# Debug build
DEBUG				:= 0

# Build platform
DEFAULT_PLAT			:= fvp

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

# Build flag to treat usage of deprecated platform and framework APIs as error.
ERROR_DEPRECATED		:= 0

# Byte alignment that each component in FIP is aligned to
FIP_ALIGN			:= 0

# Default FIP file name
FIP_NAME			:= fip.bin

# Default FWU_FIP file name
FWU_FIP_NAME			:= fwu_fip.bin

# For Chain of Trust
GENERATE_COT			:= 0

# Hint platform interrupt control layer that Group 0 interrupts are for EL3. By
# default, they are for Secure EL1.
GICV2_G0_FOR_EL3		:= 0

# Whether system coherency is managed in hardware, without explicit software
# operations.
HW_ASSISTED_COHERENCY		:= 0

# Set the default algorithm for the generation of Trusted Board Boot keys
KEY_ALG				:= rsa

# Flag to enable new version of image loading
LOAD_IMAGE_V2			:= 0

# NS timer register save and restore
NS_TIMER_SWITCH			:= 0

# Build PL011 UART driver in minimal generic UART mode
PL011_GENERIC_UART		:= 0

# By default, consider that the platform's reset address is not programmable.
# The platform Makefile is free to override this value.
PROGRAMMABLE_RESET_ADDRESS	:= 0

# Flag used to choose the power state format viz Extended State-ID or the
# Original format.
PSCI_EXTENDED_STATE_ID		:= 0

# By default, BL1 acts as the reset handler, not BL31
RESET_TO_BL31			:= 0

# For Chain of Trust
SAVE_KEYS			:= 0

# Software Delegated Exception support
SDEI_SUPPORT            	:= 0

# Whether code and read-only data should be put on separate memory pages. The
# platform Makefile is free to override this value.
SEPARATE_CODE_AND_RODATA	:= 0

# SPD choice
SPD				:= none

# For including the Secure Partition Manager
ENABLE_SPM			:= 0

# Flag to introduce an infinite loop in BL1 just before it exits into the next
# image. This is meant to help debugging the post-BL2 phase.
SPIN_ON_BL1_EXIT		:= 0

# Flags to build TF with Trusted Boot support
TRUSTED_BOARD_BOOT		:= 0

# Build option to choose whether Trusted firmware uses Coherent memory or not.
USE_COHERENT_MEM		:= 1

# Use tbbr_oid.h instead of platform_oid.h
USE_TBBR_DEFS			= $(ERROR_DEPRECATED)

# Build verbosity
V				:= 0

# Whether to enable D-Cache early during warm boot. This is usually
# applicable for platforms wherein interconnect programming is not
# required to enable cache coherency after warm reset (eg: single cluster
# platforms).
WARMBOOT_ENABLE_DCACHE_EARLY	:= 0

# Build option to enable/disable the Statistical Profiling Extensions
ENABLE_SPE_FOR_LOWER_ELS	:= 1

# SPE is only supported on AArch64 so disable it on AArch32.
ifeq (${ARCH},aarch32)
    override ENABLE_SPE_FOR_LOWER_ELS := 0
endif

ENABLE_AMU			:= 0

# By default, enable Scalable Vector Extension if implemented for Non-secure
# lower ELs
# Note SVE is only supported on AArch64 - therefore do not enable in AArch32
ifneq (${ARCH},aarch32)
    ENABLE_SVE_FOR_NS		:= 1
else
    override ENABLE_SVE_FOR_NS	:= 0
endif

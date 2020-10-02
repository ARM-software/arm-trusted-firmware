#
# Copyright (c) 2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

GCC_V_OUTPUT		:=	$(if $($(ARCH)-cc),$(shell $($(ARCH)-cc) -v 2>&1))
PIE_FOUND		:=	$(findstring --enable-default-pie,${GCC_V_OUTPUT})

ifneq (${DEBUG}, 0)
	cflags-common		+=	-g -gdwarf-4
endif #(Debug)

ifeq (${AARCH32_INSTRUCTION_SET},A32)
	cflags-aarch32		:=	-marm
else ifeq (${AARCH32_INSTRUCTION_SET},T32)
	cflags-aarch32		:=	-mthumb
endif #(AARCH32_INSTRUCTION_SET)

cflags-aarch32			+=	-mno-unaligned-access
cflags-aarch64			:=	-mgeneral-regs-only -mstrict-align

cflags-common			+=	$(cflags-$(ARCH))

##############################################################################
# WARNINGS Configuration
###############################################################################
# General warnings
WARNING0		:=	-Wall -Wmissing-include-dirs -Wunused	\
				-Wdisabled-optimization -Wvla -Wshadow	\
				-Wredundant-decls
# stricter warnings
WARNING0		+=	-Wextra -Wno-trigraphs
# too verbose for generic build
WARNING0		+=	-Wno-missing-field-initializers \
				-Wno-type-limits -Wno-sign-compare \
# on clang this flag gets reset if -Wextra is set after it. No difference on gcc
WARNING0		+=	-Wno-unused-parameter

# Additional warnings
# Level 1 - infrequent warnings we should have none of
# full -Wextra
WARNING1 := $(WARNING0)
WARNING1 += -Wsign-compare
WARNING1 += -Wtype-limits
WARNING1 += -Wmissing-field-initializers

# Level 2 - problematic warnings that we want
# zlib, compiler-rt, coreboot, and mbdedtls blow up with these
# TODO: disable just for them and move into default build
WARNING2 := $(WARNING1)
WARNING2 += -Wold-style-definition
WARNING2 += -Wmissing-prototypes
WARNING2 += -Wmissing-format-attribute
# TF-A aims to comply with this eventually. Effort too large at present
WARNING2 += -Wundef
# currently very involved and many platforms set this off
WARNING2 += -Wunused-const-variable=2

# Level 3 - very pedantic, frequently ignored
WARNING3 := $(WARNING2)
WARNING3 += -Wbad-function-cast
WARNING3 += -Waggregate-return
WARNING3 += -Wnested-externs
WARNING3 += -Wcast-align
WARNING3 += -Wcast-qual
WARNING3 += -Wconversion
WARNING3 += -Wpacked
WARNING3 += -Wpointer-arith
WARNING3 += -Wswitch-default

cflags-common	+=	$(WARNING$(W))
ifneq (${E},0)
	cflags-common	+= -Werror
endif #(E)

# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105523
TF_CFLAGS_MIN_PAGE_SIZE	:=	$(call cc_option, --param=min-pagesize=0)
ifeq ($(HARDEN_SLS), 1)
        TF_CFLAGS_MHARDEN_SLS	:=      $(call cc_option, -mharden-sls=all)
endif

LTO_CFLAGS		:=	$(if $(call bool,$(ENABLE_LTO)),-flto)

# Compiler specific warnings
cc-flags-gnu-gcc	+=	-Wunused-but-set-variable -Wmaybe-uninitialized	\
				-Wpacked-bitfield-compat -Wshift-overflow=2 \
				-Wlogical-op $(TF_CFLAGS_MIN_PAGE_SIZE) $(TF_CFLAGS_MHARDEN_SLS)
cc-flags-llvm-clang	+=	-Wshift-overflow -Wshift-sign-overflow \
				-Wlogical-op-parentheses
# TODO: some day GCC will be able to LTO __builtin functions (i.e. the libc and
# with it all libs). When this happens, this should become generic. This can
# also happen when GCC14 is the oldest reasonable version we support, then this
# can work with -ffat-lto-objects.
cc-flags-llvm-clang	+=	$(LTO_CFLAGS)

# arm-clang has the same flags
cc-flags-arm-clang	+=	$(cc-flags-llvm-clang)

cflags-common		+=	${DEFINES} ${INCLUDES} ${MBEDTLS_INC} -nostdinc

cflags-common		+=	-ffunction-sections -fdata-sections		\
				-ffreestanding -fno-common			\
				-Os -std=gnu99

ifneq (${BP_OPTION},none)
	cflags-common	+=	-mbranch-protection=${BP_OPTION}
endif #(BP_OPTION)

ifeq (${SANITIZE_UB},on)
	cflags-common	+=	-fsanitize=undefined -fno-sanitize-recover
endif #(${SANITIZE_UB},on)

ifeq (${SANITIZE_UB},trap)
	cflags-common	+=	-fsanitize=undefined -fno-sanitize-recover	\
				-fsanitize-undefined-trap-on-error
endif #(${SANITIZE_UB},trap)

ifeq (${ERROR_DEPRECATED},0)
        cflags-common	+= 	-Wno-error=deprecated-declarations
        cflags-common	+= 	-Wno-error=cpp
endif #(!ERROR_DEPRECATED)

################################################################################
# Platform specific Makefile might provide us ARCH_MAJOR/MINOR use that to come
# up with appropriate march values for compiler.
################################################################################
include ${MAKE_HELPERS_DIRECTORY}march.mk
ifeq (${ARM_ARCH_MAJOR},7)
include make_helpers/armv7-a-cpus.mk
endif

ifneq ($(ENABLE_FEAT_MORELLO),0)
        TF_CFLAGS	+=	-march=morello
else
        cflags-common	+=	$(march-directive)
endif

ifneq ($(PIE_FOUND),)
        cflags-common	+=	-fno-PIE
endif

ifeq ($(ENABLE_LTO),1)
ifeq ($($(ARCH)-ld-id),gnu-gcc)
        cflags-common	+=	-flto-partition=one
endif
endif

cflags-common		+=	$(TF_CFLAGS_$(ARCH))
cflags-common		+=	$(CPPFLAGS) $(CFLAGS) # some platforms set these
TF_CFLAGS		+=	$(cflags-common)
TF_CFLAGS		+=	$(target-flag-$(ARCH)-$($(ARCH)-cc-id))
TF_CFLAGS		+=	$(cc-flags-$($(ARCH)-cc-id))

# it's logical to give the same flags to the linker when it's invoked through
# the compiler. This is requied for LTO to work correctly
ifeq ($($(ARCH)-ld-id),$($(ARCH)-cc-id))
        TF_LDFLAGS	+= 	$(cflags-common)
        TF_LDFLAGS	+=	$(cc-flags-$($(ARCH)-ld-id))
        TF_LDFLAGS	+=	$(LTO_CFLAGS)
endif

TF_LDFLAGS		+= 	$(target-flag-$(ARCH)-$($(ARCH)-ld-id))

ASFLAGS			+=	-Wa,--fatal-warnings
TF_LDFLAGS		+=	-z noexecstack

# LD = armlink
ifeq ($($(ARCH)-ld-id),arm-link)
	TF_LDFLAGS		+=	--diag_error=warning --lto_level=O1
	TF_LDFLAGS		+=	--remove --info=unused,unusedsymbols

# LD = gcc or clang
else
        ifeq ($($(ARCH)-ld-id),llvm-clang)
                ldflags-common		+=	-fuse-ld=lld
        endif

        ldflags-common		:=	$(call ld_option,--no-warn-rwx-segments)
        # ld.lld reports section type mismatch warnings,
        # so don't add --fatal-warnings to it.
        ifneq ($($(ARCH)-ld-id),$(filter $($(ARCH)-ld-id),llvm-clang llvm-lld))
                ldflags-common	+=	$(call ld_prefix,--fatal-warnings)
        endif
        ldflags-common		+=	$(call ld_prefix,--gc-sections)
        ldflags-common		+=	-z common-page-size=4096 # Configure page size constants
        ldflags-common		+=	-z max-page-size=4096
        ldflags-common		+=	$(call ld_prefix,--build-id=none)
        ldflags-common		+=	$(call ld_option,--sort-section=alignment)

        ifeq ($(ENABLE_LTO),1)
                ldflags-common	+=	-fuse-linker-plugin
        endif #(ENABLE_LTO)

        ldflags-common		+= 	-nostdlib

        ifneq ($(call bool,$(USE_ROMLIB)),)
                ldflags-common	+= @${BUILD_PLAT}/romlib/romlib.ldflags
        endif
endif

# Errata build flags
ifneq ($(call bool,$(ERRATA_A53_843419)),)
ldflags-aarch64		+= $(call ld_option,--fix-cortex-a53-843419)
else
# GCC automatically adds fix-cortex-a53-843419 flag when used to link
# which breaks some builds, so disable if errata fix is not explicitly enabled
ldflags-aarch64		+= $(call ld_option,--no-fix-cortex-a53-843419)
endif

ifneq ($(call bool,$(ERRATA_A53_835769)),)
cflags-aarch64		+= -mfix-cortex-a53-835769
ldflags-aarch64		+= $(call ld_option,--fix-cortex-a53-835769)
endif

ifneq ($(PIE_FOUND),)
ifeq ($($(ARCH)-ld-id),gnu-gcc)
        ldflags-common	+=	-no-pie
endif
endif #(PIE_FOUND)
TF_LDFLAGS	+=	$(ldflags-common)
TF_LDFLAGS	+=	$(ldflags-$(ARCH))

PIE_LDFLAGS	+=	$(call ld_prefix,-pie)
PIE_LDFLAGS	+=	$(call ld_prefix,--no-dynamic-linker)

ifeq ($(ENABLE_PIE),1)
	ifeq ($(RESET_TO_BL2),1)
		ifneq ($(BL2_IN_XIP_MEM),1)
			BL2_CPPFLAGS	+=	-fpie
			BL2_CFLAGS	+=	-fpie
			BL2_LDFLAGS	+=	$(PIE_LDFLAGS)
		endif #(BL2_IN_XIP_MEM)
	endif #(RESET_TO_BL2)
	BL31_CPPFLAGS	+=	-fpie
	BL31_CFLAGS 	+=	-fpie
	BL31_LDFLAGS	+=	$(PIE_LDFLAGS)

	BL32_CPPFLAGS	+=	-fpie
	BL32_CFLAGS	+=	-fpie
	BL32_LDFLAGS	+=	$(PIE_LDFLAGS)
endif #(ENABLE_PIE)

BL1_CPPFLAGS  += -DREPORT_ERRATA=${DEBUG}
BL31_CPPFLAGS += -DREPORT_ERRATA=${DEBUG}
BL32_CPPFLAGS += -DREPORT_ERRATA=${DEBUG}

BL1_CPPFLAGS += -DIMAGE_AT_EL3
ifeq ($(RESET_TO_BL2),1)
	BL2_CPPFLAGS += -DIMAGE_AT_EL3
else
	BL2_CPPFLAGS += -DIMAGE_AT_EL1
endif #(RESET_TO_BL2)

ifeq (${ARCH},aarch64)
	BL2U_CPPFLAGS += -DIMAGE_AT_EL1
	BL31_CPPFLAGS += -DIMAGE_AT_EL3
	BL32_CPPFLAGS += -DIMAGE_AT_EL1
else
	BL32_CPPFLAGS += -DIMAGE_AT_EL3
endif

ifeq (${SPD},spmd)
	ifeq ($(findstring optee_sp,$(ARM_SPMC_MANIFEST_DTS)),optee_sp)
		DTC_CPPFLAGS	+=	-DOPTEE_SP_FW_CONFIG
	endif

	ifeq ($(findstring trusty_sp,$(ARM_SPMC_MANIFEST_DTS)),trusty_sp)
		DTC_CPPFLAGS	+=	-DTRUSTY_SP_FW_CONFIG
	endif

	ifeq ($(TS_SP_FW_CONFIG),1)
		DTC_CPPFLAGS	+=	-DTS_SP_FW_CONFIG
	endif

	ifneq ($(ARM_BL2_SP_LIST_DTS),)
		DTC_CPPFLAGS += -DARM_BL2_SP_LIST_DTS=$(ARM_BL2_SP_LIST_DTS)
	endif
endif


DTC_FLAGS		+=	-I dts -O dtb
DTC_CPPFLAGS		+=	-Ifdts -undef


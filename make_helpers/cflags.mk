#
# Copyright (c) 2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

GCC_V_OUTPUT		:=	$(if $($(ARCH)-cc),$(shell $($(ARCH)-cc) -v 2>&1))
PIE_FOUND		:=	$(findstring --enable-default-pie,${GCC_V_OUTPUT})

################################################################################
# Compiler Configuration based on ARCH_MAJOR and ARCH_MINOR flags
################################################################################
ifeq (${ARM_ARCH_MAJOR},7)
	target32-directive	= 	-target arm-none-eabi
# Will set march-directive from platform configuration
else
	target32-directive	= 	-target armv8a-none-eabi
endif #(ARM_ARCH_MAJOR)

ifneq ($(filter %-clang,$($(ARCH)-cc-id)),)
	ifeq ($($(ARCH)-cc-id),arm-clang)
		TF_CFLAGS_aarch32	:=	-target arm-arm-none-eabi
		TF_CFLAGS_aarch64	:=	-target aarch64-arm-none-eabi
	else
		TF_CFLAGS_aarch32	=	$(target32-directive)
		TF_CFLAGS_aarch64	:=	-target aarch64-elf
	endif
endif #(clang)

# Process Debug flag
ifneq (${DEBUG}, 0)
	TF_CFLAGS	+=	-g -gdwarf-4
endif #(Debug)

ifeq (${AARCH32_INSTRUCTION_SET},A32)
	TF_CFLAGS_aarch32	+=	-marm
else ifeq (${AARCH32_INSTRUCTION_SET},T32)
	TF_CFLAGS_aarch32	+=	-mthumb
endif #(AARCH32_INSTRUCTION_SET)

TF_CFLAGS_aarch32	+=	-mno-unaligned-access
TF_CFLAGS_aarch64	+=	-mgeneral-regs-only -mstrict-align

##############################################################################
# WARNINGS Configuration
###############################################################################
# General warnings
WARNINGS		:=	-Wall -Wmissing-include-dirs -Wunused	\
				-Wdisabled-optimization -Wvla -Wshadow	\
				-Wredundant-decls
# stricter warnings
WARNINGS		+=	-Wextra -Wno-trigraphs
# too verbose for generic build
WARNINGS		+=	-Wno-missing-field-initializers \
				-Wno-type-limits -Wno-sign-compare \
# on clang this flag gets reset if -Wextra is set after it. No difference on gcc
WARNINGS		+=	-Wno-unused-parameter

# Additional warnings
# Level 1 - infrequent warnings we should have none of
# full -Wextra
WARNING1 += -Wsign-compare
WARNING1 += -Wtype-limits
WARNING1 += -Wmissing-field-initializers

# Level 2 - problematic warnings that we want
# zlib, compiler-rt, coreboot, and mbdedtls blow up with these
# TODO: disable just for them and move into default build
WARNING2 += -Wold-style-definition
WARNING2 += -Wmissing-prototypes
WARNING2 += -Wmissing-format-attribute
# TF-A aims to comply with this eventually. Effort too large at present
WARNING2 += -Wundef
# currently very involved and many platforms set this off
WARNING2 += -Wunused-const-variable=2

# Level 3 - very pedantic, frequently ignored
WARNING3 := -Wbad-function-cast
WARNING3 += -Waggregate-return
WARNING3 += -Wnested-externs
WARNING3 += -Wcast-align
WARNING3 += -Wcast-qual
WARNING3 += -Wconversion
WARNING3 += -Wpacked
WARNING3 += -Wpointer-arith
WARNING3 += -Wswitch-default

ifeq (${W},1)
	WARNINGS += $(WARNING1)
else ifeq (${W},2)
	WARNINGS += $(WARNING1) $(WARNING2)
else ifeq (${W},3)
	WARNINGS += $(WARNING1) $(WARNING2) $(WARNING3)
endif #(W)

ifneq (${E},0)
	ERRORS := -Werror
endif #(E)

# Compiler specific warnings
ifeq ($(filter %-clang,$($(ARCH)-cc-id)),)
# not using clang
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105523
TF_CFLAGS_MIN_PAGE_SIZE	:=	$(call cc_option, --param=min-pagesize=0)
TF_CFLAGS		+=	$(TF_CFLAGS_MIN_PAGE_SIZE)

ifeq ($(HARDEN_SLS), 1)
        TF_CFLAGS_MHARDEN_SLS	:=      $(call cc_option, -mharden-sls=all)
        TF_CFLAGS_aarch64	+=      $(TF_CFLAGS_MHARDEN_SLS)
endif

WARNINGS	+=		-Wunused-but-set-variable -Wmaybe-uninitialized	\
				-Wpacked-bitfield-compat -Wshift-overflow=2 \
				-Wlogical-op

else
# using clang
WARNINGS	+=		-Wshift-overflow -Wshift-sign-overflow \
				-Wlogical-op-parentheses
endif #(Clang Warning)

CPPFLAGS		=	${DEFINES} ${INCLUDES} ${MBEDTLS_INC} -nostdinc	\
				$(ERRORS) $(WARNINGS)


TF_CFLAGS		+=	-ffunction-sections -fdata-sections		\
				-ffreestanding -fno-common			\
				-Os -std=gnu99

ifneq (${BP_OPTION},none)
	TF_CFLAGS_aarch64	+=	-mbranch-protection=${BP_OPTION}
endif #(BP_OPTION)

ifeq (${SANITIZE_UB},on)
	TF_CFLAGS	+=	-fsanitize=undefined -fno-sanitize-recover
endif #(${SANITIZE_UB},on)

ifeq (${SANITIZE_UB},trap)
	TF_CFLAGS	+=	-fsanitize=undefined -fno-sanitize-recover	\
				-fsanitize-undefined-trap-on-error
endif #(${SANITIZE_UB},trap)

ifeq ($($(ARCH)-cc-id),gnu-gcc)
	# Enable LTO only for aarch64
	LTO_CFLAGS	=	$(if $(filter-out 0,$(ENABLE_LTO)),-flto)
endif #(gnu-gcc)

ifeq (${ERROR_DEPRECATED},0)
# Check if deprecated declarations and cpp warnings should be treated as error or not.
ifneq ($(filter %-clang,$($(ARCH)-cc-id)),)
    CPPFLAGS		+= 	-Wno-error=deprecated-declarations
else
    CPPFLAGS		+= 	-Wno-error=deprecated-declarations -Wno-error=cpp
endif
endif #(!ERROR_DEPRECATED)

################################################################################
# Platform specific Makefile might provide us ARCH_MAJOR/MINOR use that to come
# up with appropriate march values for compiler.
################################################################################
include ${MAKE_HELPERS_DIRECTORY}march.mk
ifeq (${ARM_ARCH_MAJOR},7)
include make_helpers/armv7-a-cpus.mk
endif

TF_CFLAGS	+=	$(march-directive)

ifneq ($(PIE_FOUND),)
	TF_CFLAGS	+=	-fno-PIE
endif

TF_CFLAGS		+=	$(CPPFLAGS) $(TF_CFLAGS_$(ARCH))
TF_CFLAGS		+=	$(CFLAGS)
ASFLAGS			+=	-Wa,--fatal-warnings
TF_LDFLAGS		+=	-z noexecstack

# LD = armlink
ifeq ($($(ARCH)-ld-id),arm-link)
	TF_LDFLAGS		+=	--diag_error=warning --lto_level=O1
	TF_LDFLAGS		+=	--remove --info=unused,unusedsymbols
	TF_LDFLAGS		+=	$(TF_LDFLAGS_$(ARCH))

# LD = gcc (used when GCC LTO is enabled)
else ifeq ($($(ARCH)-ld-id),gnu-gcc)
	# Pass ld options with Wl or Xlinker switches
	TF_LDFLAGS		+=	$(call ld_option,-Xlinker --no-warn-rwx-segments)
	TF_LDFLAGS		+=	-Wl,--fatal-warnings -O1
	TF_LDFLAGS		+=	-Wl,--gc-sections

	TF_LDFLAGS		+=	-Wl,-z,common-page-size=4096 #Configure page size constants
	TF_LDFLAGS		+=	-Wl,-z,max-page-size=4096
	TF_LDFLAGS		+=	-Wl,--build-id=none

	ifeq ($(ENABLE_LTO),1)
		TF_LDFLAGS	+=	-flto -fuse-linker-plugin
		TF_LDFLAGS      +=	-flto-partition=one
	endif #(ENABLE_LTO)

# GCC automatically adds fix-cortex-a53-843419 flag when used to link
# which breaks some builds, so disable if errata fix is not explicitly enabled
	ifeq (${ARCH},aarch64)
		ifneq (${ERRATA_A53_843419},1)
			TF_LDFLAGS	+= 	-mno-fix-cortex-a53-843419
		endif
	endif
	TF_LDFLAGS		+= 	-nostdlib
	TF_LDFLAGS		+=	$(subst --,-Xlinker --,$(TF_LDFLAGS_$(ARCH)))

# LD = gcc-ld (ld) or llvm-ld (ld.lld) or other
else
# With ld.bfd version 2.39 and newer new warnings are added. Skip those since we
# are not loaded by a elf loader.
	TF_LDFLAGS		+=	$(call ld_option, --no-warn-rwx-segments)
	TF_LDFLAGS		+=	-O1
	TF_LDFLAGS		+=	--gc-sections

	TF_LDFLAGS		+=	-z common-page-size=4096 # Configure page size constants
	TF_LDFLAGS		+=	-z max-page-size=4096
	TF_LDFLAGS		+=	--build-id=none

# ld.lld doesn't recognize the errata flags,
# therefore don't add those in that case.
# ld.lld reports section type mismatch warnings,
# therefore don't add --fatal-warnings to it.
	ifneq ($($(ARCH)-ld-id),llvm-lld)
		TF_LDFLAGS	+=	$(TF_LDFLAGS_$(ARCH)) --fatal-warnings
	endif

endif #(LD = armlink)

ifneq ($(PIE_FOUND),)
ifeq ($($(ARCH)-ld-id),gnu-gcc)
	TF_LDFLAGS	+=	-no-pie
endif
endif #(PIE_FOUND)

ifeq ($($(ARCH)-ld-id),gnu-gcc)
	PIE_LDFLAGS	+=	-Wl,-pie -Wl,--no-dynamic-linker
else
	PIE_LDFLAGS	+=	-pie --no-dynamic-linker
endif

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


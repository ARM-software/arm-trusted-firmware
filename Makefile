#
# Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

# Decrease the verbosity of the make script
# can be made verbose by passing V=1 at the make command line
ifdef V
  KBUILD_VERBOSE = ${V}
else
  KBUILD_VERBOSE = 0
endif

# Checkpatch ignores
CHECK_IGNORE		=	--ignore COMPLEX_MACRO

CHECKPATCH_ARGS		=	--no-tree --no-signoff ${CHECK_IGNORE}
CHECKCODE_ARGS		=	--no-patch --no-tree --no-signoff ${CHECK_IGNORE}

ifeq "${KBUILD_VERBOSE}" "0"
	Q=@
	CHECKCODE_ARGS	+=	--no-summary --terse
else
	Q=
endif

DEBUG	?= 0

ifneq (${DEBUG}, 0)
	BUILD_TYPE	:=	debug
else
	BUILD_TYPE	:=	release
endif

BL_COMMON_SOURCES	:=	misc_helpers.S		\
				cache_helpers.S		\
				tlb_helpers.S		\
				std.c			\
				bl_common.c		\
				platform_helpers.S	\
				io_storage.c

ARCH 			?=	aarch64

# By default, build all platforms available
PLAT			?=	fvp

BUILD_BASE		:=	./build
BUILD_PLAT		:=	${BUILD_BASE}/${PLAT}/${BUILD_TYPE}

PLATFORMS		:=	$(shell ls -I common plat/)
HELP_PLATFORMS		:=	$(shell echo ${PLATFORMS} | sed 's/ /|/g')

ifeq (${PLAT},)
  $(error "Error: Unknown platform. Please use PLAT=<platform name> to specify the platform.")
endif
ifeq ($(findstring ${PLAT},${PLATFORMS} all),)
  $(error "Error: Invalid platform. The following platforms are available: ${PLATFORMS}")
endif

include plat/${PLAT}/platform.mk

ifdef BL1_SOURCES
BL1_PRESENT := bl1
include bl1/bl1.mk
endif

ifdef BL2_SOURCES
BL2_PRESENT := bl2
include bl2/bl2.mk
endif

ifdef BL31_SOURCES
BL31_PRESENT := bl31
include bl31/bl31.mk
endif

all: msg_start $(BL1_PRESENT) $(BL2_PRESENT) $(BL31_PRESENT)

msg_start:
	@echo "Building ${PLAT}"

${PLATFORMS}:
	${MAKE} PLAT=$@ all

.PHONY:			all msg_start ${PLATFORMS} dump clean realclean distclean cscope locate-checkpatch checkcodebase checkpatch fiptool fip locate-bl33
.SUFFIXES:


INCLUDES		+=	-Ilib/include/			\
				-Idrivers/io			\
				-Iinclude/${ARCH}/		\
				-Iinclude/			\
				-Iarch/system/gic		\
				-Icommon/psci			\
				-Iinclude/stdlib		\
				-Iinclude/stdlib/sys		\
				-Iplat/${PLAT}			\
				${PLAT_INCLUDES}

ASFLAGS			+= 	-nostdinc -ffreestanding -Wa,--fatal-warnings	\
				-mgeneral-regs-only -D__ASSEMBLY__ ${INCLUDES}	\
				-DDEBUG=${DEBUG}
CFLAGS			:= 	-nostdinc -pedantic -ffreestanding -Wall	\
				-Werror -mgeneral-regs-only -std=c99 -c -Os	\
				-DDEBUG=${DEBUG} ${INCLUDES} ${CFLAGS}

LDFLAGS			+=	--fatal-warnings -O1


vpath %.ld.S bl1:bl2:bl31
vpath %.c bl1:bl2:bl31
vpath %.c bl1/${ARCH}:bl2/${ARCH}:bl31/${ARCH}
vpath %.S bl1/${ARCH}:bl2/${ARCH}:bl31/${ARCH}


ifneq (${DEBUG}, 0)
#CFLAGS			+= 	-g -O0
CFLAGS			+= 	-g
# -save-temps -fverbose-asm
ASFLAGS			+= 	-g -Wa,--gdwarf-2
else
CFLAGS			+=	-DNDEBUG=1
endif


CC			:=	${CROSS_COMPILE}gcc
CPP			:=	${CROSS_COMPILE}cpp
AS			:=	${CROSS_COMPILE}gcc
AR			:=	${CROSS_COMPILE}ar
LD			:=	${CROSS_COMPILE}ld
OC			:=	${CROSS_COMPILE}objcopy
OD			:=	${CROSS_COMPILE}objdump
NM			:=	${CROSS_COMPILE}nm
PP			:=	${CROSS_COMPILE}gcc -E ${CFLAGS}

BASE_COMMIT		?=	origin/master

# Variables for use with Firmware Image Package
FIPTOOLPATH		?=	tools/fip_create
FIPTOOL		?=	${FIPTOOLPATH}/fip_create
fiptool:		${FIPTOOL}
fip:			${BUILD_PLAT}/fip.bin

ifeq (${PLAT},all)
  ifeq (${MAKECMDGOALS},clean)
    $(error "Please select a platform with PLAT=<platform>. You can use 'make distclean' to clean up all platform builds")
  endif
endif

locate-checkpatch:
ifndef CHECKPATCH
	$(error "Please set CHECKPATCH to point to the Linux checkpatch.pl file, eg: CHECKPATCH=../linux/script/checkpatch.pl")
else
ifeq (,$(wildcard ${CHECKPATCH}))
	$(error "The file CHECKPATCH points to cannot be found, use eg: CHECKPATCH=../linux/script/checkpatch.pl")
endif
endif

locate-bl33:
ifndef BL33
	$(error "Please set BL33 to point to the Normal World binary, eg: BL33=../uefi/FVP_AARCH64_EFI.fd")
else
ifeq (,$(wildcard ${BL33}))
	$(error "The file BL33 points to cannot be found (${BL33})")
endif
endif


clean:
			@echo "  CLEAN"
			${Q}rm -rf ${BUILD_PLAT}
			${Q}${MAKE} -C ${FIPTOOLPATH} clean

realclean distclean:
			@echo "  REALCLEAN"
			${Q}rm -rf ${BUILD_BASE}
			${Q}rm -f ${CURDIR}/cscope.*
			${Q}${MAKE} -C ${FIPTOOLPATH} clean

dump:
			@echo "  OBJDUMP"
			${Q}${OD} -d ${BUILD_BL1}/bl1.elf > ${BUILD_BL1}/bl1.dump
			${Q}${OD} -d ${BUILD_BL2}/bl2.elf > ${BUILD_BL2}/bl2.dump
			${Q}${OD} -d ${BUILD_BL31}/bl31.elf > ${BUILD_BL31}/bl31.dump

checkcodebase:		locate-checkpatch
			@echo "  CHECKING STYLE"
			@if test -d .git ; then	\
				git ls-files | while read GIT_FILE ; do ${CHECKPATCH} ${CHECKCODE_ARGS} -f $$GIT_FILE ; done ;	\
			 else			\
				 find . -type f -not -iwholename "*.git*" -not -iwholename "*build*" -exec ${CHECKPATCH} ${CHECKCODE_ARGS} -f {} \; ;	\
			 fi

checkpatch:		locate-checkpatch
			@echo "  CHECKING STYLE"
			@git format-patch --stdout ${BASE_COMMIT} | ${CHECKPATCH} ${CHECKPATCH_ARGS} - || true

${FIPTOOL}:
			@echo "  BUILDING FIRMWARE IMAGE PACKAGE TOOL $@"
			@echo
			${Q}${MAKE} -C ${FIPTOOLPATH}
			@echo

define match_goals
$(strip $(foreach goal,$(1),$(filter $(goal),$(MAKECMDGOALS))))
endef


CLEANING := $(call match_goals,clean realclean distclean)


define MAKE_C

$(eval OBJ := $(1)/$(patsubst %.c,%.o,$(notdir $(2))))
$(eval PREREQUISITES := $(patsubst %.o,%.d,$(OBJ)))

$(OBJ) : $(2)
	@echo "  CC      $$<"
	$$(Q)$$(CC) $$(CFLAGS) -c $$< -o $$@


$(PREREQUISITES) : $(2)
	@echo "  DEPS    $$@"
	@mkdir -p $(1)
	$$(Q)$$(CC) $$(CFLAGS) -M -MT $(OBJ) -MF $$@ $$<

ifeq "$(CLEANING)" ""
-include $(PREREQUISITES)
endif

endef


define MAKE_S

$(eval OBJ := $(1)/$(patsubst %.S,%.o,$(notdir $(2))))
$(eval PREREQUISITES := $(patsubst %.o,%.d,$(OBJ)))

$(OBJ) : $(2)
	@echo "  AS      $$<"
	$$(Q)$$(AS) $$(ASFLAGS) -c $$< -o $$@

$(PREREQUISITES) : $(2)
	@echo "  DEPS    $$@"
	@mkdir -p $(1)
	$$(Q)$$(AS) $$(ASFLAGS) -M -MT $(OBJ) -MF $$@ $$<

ifeq "$(CLEANING)" ""
-include $(PREREQUISITES)
endif

endef


define MAKE_OBJS
	$(eval C_OBJS := $(filter %.c,$(2)))
	$(eval REMAIN := $(filter-out %.c,$(2)))
	$(eval $(foreach obj,$(C_OBJS),$(call MAKE_C,$(1),$(obj))))

	$(eval S_OBJS := $(filter %.S,$(REMAIN)))
	$(eval REMAIN := $(filter-out %.S,$(REMAIN)))
	$(eval $(foreach obj,$(S_OBJS),$(call MAKE_S,$(1),$(obj))))

	$(and $(REMAIN),$(error Unexpected source files present: $(REMAIN)))
endef


# NOTE: The line continuation '\' is required in the next define otherwise we
# end up with a line-feed characer at the end of the last c filename.
# Also bare this issue in mind if extending the list of supported filetypes.
define SOURCES_TO_OBJS
	$(notdir $(patsubst %.c,%.o,$(filter %.c,$(1)))) \
	$(notdir $(patsubst %.S,%.o,$(filter %.S,$(1))))
endef

define MAKE_BL
	$(eval BUILD_DIR  := ${BUILD_PLAT}/bl$(1))
	$(eval SOURCES    := $(BL$(1)_SOURCES) $(BL_COMMON_SOURCES) $(PLAT_BL_COMMON_SOURCES))
	$(eval OBJS       := $(addprefix $(BUILD_DIR)/,$(call SOURCES_TO_OBJS,$(SOURCES))))
	$(eval LINKERFILE := $(BUILD_DIR)/bl$(1).ld)
	$(eval MAPFILE    := $(BUILD_DIR)/bl$(1).map)
	$(eval ELF        := $(BUILD_DIR)/bl$(1).elf)
	$(eval BIN        := $(BUILD_PLAT)/bl$(1).bin)

	$(eval $(call MAKE_OBJS,$(BUILD_DIR),$(SOURCES)))

$(BUILD_DIR) :
	$$(Q)mkdir -p "$$@"

$(LINKERFILE) : bl$(1)/bl$(1).ld.S
	@echo "  PP      $$<"
	$$(Q)$$(AS) $$(ASFLAGS) -P -E $$< -o $$@

$(ELF) : $(OBJS) $(LINKERFILE)
	@echo "  LD      $$@"
	@echo 'const char build_message[] = "Built : "__TIME__", "__DATE__;' | \
		$$(CC) $$(CFLAGS) -xc - -o $(BUILD_DIR)\build_message.o
	$$(Q)$$(LD) -o $$@ $$(LDFLAGS) -Map=$(MAPFILE) --script $(LINKERFILE) \
					--entry=$(BL$(1)_ENTRY_POINT) $(OBJS) \
					$(BUILD_DIR)\build_message.o

$(BIN) : $(ELF)
	@echo "  BIN     $$@"
	$$(Q)$$(OC) -O binary $$< $$@
	@echo
	@echo "Built $$@ successfully"
	@echo

.PHONY: bl$(1)
bl$(1) : $(BUILD_DIR) $(BIN)

endef


ifneq "$(call match_goals,all $(BL1_PRESENT))" ""
$(eval $(call MAKE_BL,1))
endif

ifneq "$(call match_goals,all $(BL2_PRESENT))" ""
$(eval $(call MAKE_BL,2))
endif

ifneq "$(call match_goals,all $(BL31_PRESENT))" ""
$(eval $(call MAKE_BL,31))
endif


${BUILD_PLAT}/fip.bin:	bl2 bl31 locate-bl33 ${FIPTOOL}
			@echo " CREATE FIRMWARE IMAGE PACKAGE $@"
			@echo
			${Q}${FIPTOOL} --dump \
				--bl2 ${BUILD_PLAT}/bl2.bin \
				--bl31 ${BUILD_PLAT}/bl31.bin \
				--bl33 ${BL33} \
				$@
			@echo


cscope:
	@echo "  CSCOPE"
	${Q}find ${CURDIR} -name "*.[chsS]" > cscope.files
	${Q}cscope -b -q -k

help:
	@echo "usage: ${MAKE} PLAT=<all|${HELP_PLATFORMS}> <all|bl1|bl2|bl31|distclean|clean|checkcodebase|checkpatch|dump>"
	@echo ""
	@echo "PLAT is used to specify which platform you wish to build."
	@echo ""
	@echo "Supported Targets:"
	@echo "  all            Build the BL1, BL2 and BL31 binaries"
	@echo "  bl1            Build the BL1 binary"
	@echo "  bl2            Build the BL2 binary"
	@echo "  bl31           Build the BL31 binary"
	@echo "  checkcodebase  Check the coding style of the entire source tree"
	@echo "  checkpatch     Check the coding style on changes in the current"
	@echo "                 branch against BASE_COMMIT (default origin/master)"
	@echo "  clean          Clean the build for the selected platform"
	@echo "  cscope         Generate cscope index"
	@echo "  distclean      Remove all build artifacts for all platforms"
	@echo "  dump           Generate object file dumps"
	@echo "  fiptool        Build the Firmware Image Package(FIP) creation tool"
	@echo ""
	@echo "note: most build targets require PLAT to be set to a specific platform."
	@echo ""
	@echo "example: build all targets for the FVP platform:"
	@echo "  CROSS_COMPILE=aarch64-none-elf- make PLAT=fvp all"

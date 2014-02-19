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

export Q

DEBUG	?= 0

ifneq (${DEBUG}, 0)
	BUILD_TYPE	:=	debug
else
	BUILD_TYPE	:=	release
endif

BL_COMMON_OBJS		:=	misc_helpers.o		\
				cache_helpers.o		\
				tlb_helpers.o		\
				xlat_helpers.o		\
				std.o			\
				bl_common.o		\
				platform_helpers.o	\
				io_storage.o

ARCH 			?=	aarch64

# By default, build all platforms available
PLAT			?=	all
# By default, build no SPD component
SPD			?=	none

BUILD_BASE		:=	./build
BUILD_PLAT		:=	${BUILD_BASE}/${PLAT}/${BUILD_TYPE}
BUILD_BL1		:=	${BUILD_PLAT}/bl1
BUILD_BL2		:=	${BUILD_PLAT}/bl2
BUILD_BL31		:=	${BUILD_PLAT}/bl31
BUILD_DIRS		:=	${BUILD_BL1} ${BUILD_BL2} ${BUILD_BL31}

PLATFORMS		:=	$(shell ls -I common plat/)
SPDS			:=	$(shell ls -I none services/spd)
HELP_PLATFORMS		:=	$(shell echo ${PLATFORMS} | sed 's/ /|/g')

ifeq (${PLAT},)
  $(error "Error: Unknown platform. Please use PLAT=<platform name> to specify the platform.")
endif
ifeq ($(findstring ${PLAT},${PLATFORMS} all),)
  $(error "Error: Invalid platform. The following platforms are available: ${PLATFORMS}")
endif

ifeq (${PLAT},all)
all: ${PLATFORMS}
else
all: msg_start bl1 bl2 bl31 fip
endif

msg_start:
	@echo "Building ${PLAT}"

${PLATFORMS}:
	${MAKE} PLAT=$@ all

ifneq (${PLAT},all)
  $(info Including ${PLAT}/platform.mk)
  include plat/${PLAT}/platform.mk
  $(info Including bl1.mk)
  include bl1/bl1.mk
  $(info Including bl2.mk)
  include bl2/bl2.mk
  $(info Including bl31.mk)
  include bl31/bl31.mk
endif

# Include SPD Makefile if one has been specified
ifneq (${SPD},none)
  # We expect to locate an spd.mk under the specified SPD directory
  SPD_MAKE		:=	$(shell m="services/spd/${SPD}/${SPD}.mk"; [ -f "$$m" ] && echo "$$m")

  ifeq (${SPD_MAKE},)
    $(error Error: No services/spd/${SPD}/${SPD}.mk located)
  endif
  $(info Including ${SPD_MAKE})
  include ${SPD_MAKE}

  # If there's BL32 companion for the chosen SPD, and the SPD wants to build the
  # BL2 from source, we expect that the SPD's Makefile would set NEED_BL32
  # variable to "yes"
endif

.PHONY:			all msg_start ${PLATFORMS} dump clean realclean distclean bl1 bl2 bl31 cscope locate-checkpatch checkcodebase checkpatch fiptool fip locate-bl33
.SUFFIXES:


BL1_OBJS		:= 	$(addprefix ${BUILD_BL1}/,${BL1_OBJS} ${BL_COMMON_OBJS} ${PLAT_BL_COMMON_OBJS})
BL2_OBJS		:= 	$(addprefix ${BUILD_BL2}/,${BL2_OBJS} ${BL_COMMON_OBJS} ${PLAT_BL_COMMON_OBJS})
BL31_OBJS		:= 	$(addprefix ${BUILD_BL31}/,${BL31_OBJS} ${BL_COMMON_OBJS} ${PLAT_BL_COMMON_OBJS} ${SPD_OBJS})
BL1_MAPFILE		:= 	$(addprefix ${BUILD_BL1}/,${BL1_MAPFILE})
BL2_MAPFILE		:= 	$(addprefix ${BUILD_BL2}/,${BL2_MAPFILE})
BL31_MAPFILE		:= 	$(addprefix ${BUILD_BL31}/,${BL31_MAPFILE})
BL1_LINKERFILE		:= 	$(addprefix ${BUILD_BL1}/,${BL1_LINKERFILE})
BL2_LINKERFILE		:= 	$(addprefix ${BUILD_BL2}/,${BL2_LINKERFILE})
BL31_LINKERFILE		:= 	$(addprefix ${BUILD_BL31}/,${BL31_LINKERFILE})

INCLUDES		+=	-Ilib/include/			\
				-Idrivers/io			\
				-Iinclude/${ARCH}/		\
				-Iinclude/			\
				-Iarch/system/gic		\
				-Iservices/psci			\
				-Iinclude/stdlib		\
				-Iinclude/stdlib/sys		\
				-Iplat/${PLAT}			\
				${PLAT_INCLUDES}		\
				${SPD_INCLUDES}

ASFLAGS			+= 	-nostdinc -ffreestanding -Wa,--fatal-warnings	\
				-mgeneral-regs-only -D__ASSEMBLY__ ${INCLUDES}	\
				-DDEBUG=${DEBUG}
CFLAGS			:= 	-nostdinc -pedantic -ffreestanding -Wall	\
				-Werror -mgeneral-regs-only -std=c99 -c -Os	\
				-DDEBUG=${DEBUG} ${INCLUDES} ${CFLAGS}

LDFLAGS			+=	--fatal-warnings -O1
BL1_LDFLAGS		:=	-Map=${BL1_MAPFILE} --script ${BL1_LINKERFILE} --entry=${BL1_ENTRY_POINT}
BL2_LDFLAGS		:=	-Map=${BL2_MAPFILE} --script ${BL2_LINKERFILE} --entry=${BL2_ENTRY_POINT}
BL31_LDFLAGS		:=	-Map=${BL31_MAPFILE} --script ${BL31_LINKERFILE} --entry=${BL31_ENTRY_POINT}


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


bl1:			${BUILD_BL1} ${BUILD_PLAT}/bl1.bin
FIP_DEPS		+= ${BUILD_PLAT}/bl1.bin

bl2:			${BUILD_BL2} ${BUILD_PLAT}/bl2.bin
FIP_DEPS		+= ${BUILD_PLAT}/bl2.bin

bl31:			${BUILD_BL31} ${BUILD_PLAT}/bl31.bin
FIP_DEPS		+= ${BUILD_PLAT}/bl31.bin

BASE_COMMIT		?=	origin/master

# Variables for use with Firmware Image Package
FIPTOOLPATH		?=	tools/fip_create
FIPTOOL			?=	${FIPTOOLPATH}/fip_create
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
FIP_DEPS		+= ${BL33}
endif


# If BL32 needs to be built, provide necessary build rules and targets
ifeq (${NEED_BL32},yes)
BUILD_BL32		:=	${BUILD_PLAT}/bl32
BUILD_DIRS		+=	${BUILD_BL32}

BL32_OBJS		:=	$(addprefix ${BUILD_BL32}/,${BL32_OBJS})
BL32_MAPFILE		:=	$(addprefix ${BUILD_BL32}/,${BL32_MAPFILE})
BL32_LINKERFILE	:=	$(addprefix ${BUILD_BL32}/,${BL32_LINKERFILE})
BL32_LDFLAGS		:=	-Map=${BL32_MAPFILE} --script ${BL32_LINKERFILE} --entry=${BL32_ENTRY_POINT}

bl32:			${BUILD_BL32} ${BUILD_PLAT}/bl32.bin
all:			bl32
dump:			bl32_dump
.PHONY:			bl32

# Add BL32 image to FIP's input image list
FIP_DEPS		:= bl32
FIP_ARGS		:= --bl32 ${BUILD_PLAT}/bl32.bin

${BUILD_BL32}/%.o:	%.S
			@echo "  AS      $<"
			${Q}${AS} ${ASFLAGS} -c $< -o $@

${BUILD_BL32}/%.o:	%.c
			@echo "  CC      $<"
			${Q}${CC} ${CFLAGS} -c $< -o $@

${BUILD_BL32}/%.ld:	%.ld.S
			@echo "  PP      $<"
			${Q}${AS} ${ASFLAGS} -P -E $< -o $@

${BUILD_BL32}/bl32.elf:	${BL32_OBJS} ${BL32_LINKERFILE}
			@echo "  LD      $@"
			${Q}${LD} -o $@ ${LDFLAGS} ${BL32_LDFLAGS} ${BL32_OBJS}

${BUILD_PLAT}/bl32.bin:	${BUILD_BL32}/bl32.elf
			@echo "  BIN     $@"
			${Q}${OC} -O binary $< $@
			@echo
			@echo "Built $@ successfully"
			@echo

bl32_dump:
	${Q}${OD} -d ${BUILD_BL32}/bl32.elf > ${BUILD_BL32}/bl32.dump
endif


clean:
			@echo "  CLEAN"
			${Q}rm -rf ${BUILD_PLAT}
			${Q}${MAKE} --no-print-directory -C ${FIPTOOLPATH} clean

realclean distclean:
			@echo "  REALCLEAN"
			${Q}rm -rf ${BUILD_BASE}
			${Q}rm -f ${CURDIR}/cscope.*
			${Q}${MAKE} --no-print-directory -C ${FIPTOOLPATH} clean

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
			${Q}${MAKE} --no-print-directory -C ${FIPTOOLPATH}
			@echo
			@echo "Built $@ successfully"
			@echo

${BUILD_DIRS}:
			${Q}mkdir -p "$@"


${BUILD_BL1}/%.o:	%.S
			@echo "  AS      $<"
			${Q}${AS} ${ASFLAGS} -c $< -o $@

${BUILD_BL2}/%.o:	%.S
			@echo "  AS      $<"
			${Q}${AS} ${ASFLAGS} -c $< -o $@

${BUILD_BL31}/%.o:	%.S
			@echo "  AS      $<"
			${Q}${AS} ${ASFLAGS} -c $< -o $@

${BUILD_BL1}/%.o:	%.c
			@echo "  CC      $<"
			${Q}${CC} ${CFLAGS} -c $< -o $@

${BUILD_BL2}/%.o:	%.c
			@echo "  CC      $<"
			${Q}${CC} ${CFLAGS} -c $< -o $@

${BUILD_BL31}/%.o:	%.c
			@echo "  CC      $<"
			${Q}${CC} ${CFLAGS} -c $< -o $@

${BUILD_BL1}/%.ld:	%.ld.S
			@echo "  PP      $<"
			${Q}${AS} ${ASFLAGS} -P -E $< -o $@

${BUILD_BL2}/%.ld:	%.ld.S
			@echo "  PP      $<"
			${Q}${AS} ${ASFLAGS} -P -E $< -o $@

${BUILD_BL31}/%.ld:	%.ld.S
			@echo "  PP      $<"
			${Q}${AS} ${ASFLAGS} -P -E $< -o $@


${BUILD_BL1}/bl1.elf:	${BL1_OBJS} ${BL1_LINKERFILE}
			@echo "  LD      $@"
			${Q}${LD} -o $@ ${LDFLAGS} ${BL1_LDFLAGS} ${BL1_OBJS}

${BUILD_BL2}/bl2.elf:	${BL2_OBJS} ${BL2_LINKERFILE}
			@echo "  LD      $@"
			${Q}${LD} -o $@ ${LDFLAGS} ${BL2_LDFLAGS} ${BL2_OBJS}

${BUILD_BL31}/bl31.elf:	${BL31_OBJS} ${BL31_LINKERFILE}
			@echo "  LD      $@"
			${Q}${LD} -o $@ ${LDFLAGS} ${BL31_LDFLAGS} ${BL31_OBJS}

${BUILD_PLAT}/bl1.bin:	${BUILD_BL1}/bl1.elf
			@echo "  BIN     $@"
			${Q}${OC} -O binary $< $@
			@echo
			@echo "Built $@ successfully"
			@echo

${BUILD_PLAT}/bl2.bin:	${BUILD_BL2}/bl2.elf
			@echo "  BIN     $@"
			${Q}${OC} -O binary $< $@
			@echo
			@echo "Built $@ successfully"
			@echo

${BUILD_PLAT}/bl31.bin:	${BUILD_BL31}/bl31.elf
			@echo "  BIN     $@"
			${Q}${OC} -O binary $< $@
			@echo
			@echo "Built $@ successfully"
			@echo

${BUILD_PLAT}/fip.bin:	locate-bl33 ${FIP_DEPS} ${FIPTOOL}
			${Q}${FIPTOOL} --dump \
				--bl2 ${BUILD_PLAT}/bl2.bin \
				--bl31 ${BUILD_PLAT}/bl31.bin \
				--bl33 ${BL33} \
				${FIP_ARGS} \
				$@
			@echo
			@echo "Built $@ successfully"
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
	@echo "  all            Build the BL1, BL31 binaries"
	@echo "  bl1            Build the BL1 binary"
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

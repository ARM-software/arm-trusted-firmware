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

ifeq "${KBUILD_VERBOSE}" "0"
	Q=@
else
	Q=
endif

DEBUG	?= 0

ifneq (${DEBUG}, 0)
	BUILD_TYPE	:=	debug
else
	BUILD_TYPE	:=	release
endif

BL_COMMON_OBJS		:=	misc_helpers.o cache_helpers.o tlb_helpers.o		\
				std.o bl_common.o platform_helpers.o
ARCH 			?=	aarch64

# By default, build all platforms available
PLAT			?=	all

BUILD_BASE		:=	./build
BUILD_PLAT		:=	${BUILD_BASE}/${PLAT}/${BUILD_TYPE}
BUILD_BL1		:=	${BUILD_PLAT}/bl1
BUILD_BL2		:=	${BUILD_PLAT}/bl2
BUILD_BL31		:=	${BUILD_PLAT}/bl31
BUILD_DIRS		:=	${BUILD_BL1} ${BUILD_BL2} ${BUILD_BL31}

PLATFORMS		:=	$(shell ls -I common plat/)
ifeq (${PLAT},)
  $(error "Error: Unknown platform. Please use PLAT=<platform name> to specify the platform.")
endif
ifeq ($(findstring ${PLAT},${PLATFORMS} all),)
  $(error "Error: Invalid platform. The following platforms are available: ${PLATFORMS}")
endif

ifeq (${PLAT},all)
all: ${PLATFORMS}
else
all: msg_start bl1 bl2 bl31
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

.PHONY:			all msg_start ${PLATFORMS} dump clean realclean distclean bl1 bl2 bl31
.SUFFIXES:


BL1_OBJS		:= 	$(addprefix ${BUILD_BL1}/,${BL1_OBJS} ${BL_COMMON_OBJS} ${PLAT_BL_COMMON_OBJS})
BL2_OBJS		:= 	$(addprefix ${BUILD_BL2}/,${BL2_OBJS} ${BL_COMMON_OBJS} ${PLAT_BL_COMMON_OBJS})
BL31_OBJS		:= 	$(addprefix ${BUILD_BL31}/,${BL31_OBJS} ${BL_COMMON_OBJS} ${PLAT_BL_COMMON_OBJS})
BL1_MAPFILE		:= 	$(addprefix ${BUILD_BL1}/,${BL1_MAPFILE})
BL2_MAPFILE		:= 	$(addprefix ${BUILD_BL2}/,${BL2_MAPFILE})
BL31_MAPFILE		:= 	$(addprefix ${BUILD_BL31}/,${BL31_MAPFILE})
BL1_LINKERFILE		:= 	$(addprefix ${BUILD_BL1}/,${BL1_LINKERFILE})
BL2_LINKERFILE		:= 	$(addprefix ${BUILD_BL2}/,${BL2_LINKERFILE})
BL31_LINKERFILE		:= 	$(addprefix ${BUILD_BL31}/,${BL31_LINKERFILE})

INCLUDES		+=	-Ilib/include/ -Iinclude/${ARCH}/ -Iinclude/	\
				-Iarch/system/gic -Icommon/psci			\
				-Iinclude/stdlib -Iinclude/stdlib/sys		\
				-Iplat/${PLAT} ${PLAT_INCLUDES}

ASFLAGS			+= 	-nostdinc -ffreestanding -Wa,--fatal-warnings	\
				-mgeneral-regs-only -D__ASSEMBLY__ ${INCLUDES}
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
bl2:			${BUILD_BL2} ${BUILD_PLAT}/bl2.bin
bl31:			${BUILD_BL31} ${BUILD_PLAT}/bl31.bin

ifeq (${PLAT},all)
  ifeq (${MAKECMDGOALS},clean)
    $(error "Please select a platform with PLAT=<platform>. You can use 'make distclean' to clean up all platform builds")
  endif
endif
clean:
			@echo "  CLEAN"
			${Q}rm -rf ${BUILD_PLAT}

realclean distclean:
			@echo "  REALCLEAN"
			${Q}rm -rf ${BUILD_BASE}

dump:
			@echo "  OBJDUMP"
			${Q}${OD} -d ${BUILD_BL1}/bl1.elf > ${BUILD_BL1}/bl1.dump
			${Q}${OD} -d ${BUILD_BL2}/bl2.elf > ${BUILD_BL2}/bl2.dump
			${Q}${OD} -d ${BUILD_BL31}/bl31.elf > ${BUILD_BL31}/bl31.dump

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

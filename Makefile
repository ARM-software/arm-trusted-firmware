#
# Copyright (c) 2013, ARM Limited. All rights reserved.
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
  KBUILD_VERBOSE = $(V)
else
  KBUILD_VERBOSE = 0
endif

ifeq "$(KBUILD_VERBOSE)" "0"
	Q=@
else
	Q=
endif

DEBUG	?= 0
BL_COMMON_OBJS		=	misc_helpers.o cache_helpers.o tlb_helpers.o		\
				semihosting_call.o mmio.o pl011.o semihosting.o		\
				std.o bl_common.o platform_helpers.o sysreg_helpers.o

ARCH 			:=	aarch64

all: $(patsubst %,%.bin,bl1 bl2 bl31) ;


#$(info $(filter bl2.%, $(MAKECMDGOALS)))
#$(info $(filter bl1.%, $(MAKECMDGOALS)))
#$(info $(MAKECMDGOALS))

$(info Including bl1.mk)
include bl1/bl1.mk

$(info Including bl2.mk)
include bl2/bl2.mk

$(info Including bl31.mk)
include bl31/bl31.mk

OBJS			+= 	$(BL_COMMON_OBJS)

INCLUDES		+=	-Ilib/include/ -Iinclude/aarch64/ -Iinclude/	\
				-Idrivers/arm/interconnect/cci-400/		\
				-Idrivers/arm/peripherals/pl011/ 		\
				-Iplat/fvp -Idrivers/power			\
				-Iarch/system/gic -Icommon/psci

ASFLAGS			+= 	 -D__ASSEMBLY__ $(INCLUDES)
CFLAGS			:= 	-Wall -std=c99 -c -Os -DDEBUG=$(DEBUG) $(INCLUDES) ${CFLAGS}

LDFLAGS			+=	-O1
BL1_LDFLAGS		:=	-Map=$(BL1_MAPFILE) --script $(BL1_LINKERFILE) --entry=$(BL1_ENTRY_POINT)
BL2_LDFLAGS		:=	-Map=$(BL2_MAPFILE) --script $(BL2_LINKERFILE) --entry=$(BL2_ENTRY_POINT)
BL31_LDFLAGS		:=	-Map=$(BL31_MAPFILE) --script $(BL31_LINKERFILE) --entry=$(BL31_ENTRY_POINT)


vpath %.ld.S bl1:bl2:bl31
vpath %.c bl1:bl2:bl31
vpath %.c bl1/${ARCH}:bl2/${ARCH}:bl31/${ARCH}
vpath %.S bl1/${ARCH}:bl2/${ARCH}:bl31/${ARCH}


ifneq ($(DEBUG), 0)
#CFLAGS			+= 	-g -O0
CFLAGS			+= 	-g
# -save-temps -fverbose-asm
ASFLAGS			+= 	-g -Wa,--gdwarf-2
endif


CC			=	$(CROSS_COMPILE)gcc
CPP			=	$(CROSS_COMPILE)cpp
AS			=	$(CROSS_COMPILE)gcc
AR			=	$(CROSS_COMPILE)ar
LD			=	$(CROSS_COMPILE)ld
OC			=	$(CROSS_COMPILE)objcopy
OD			=	$(CROSS_COMPILE)objdump
NM			=	$(CROSS_COMPILE)nm
PP			=	$(CROSS_COMPILE)gcc -E $(CFLAGS)


distclean: clean
			@echo "  DISTCLEAN"
			$(Q)rm -rf *.zi
			$(Q)rm -rf *.dump
			$(Q)rm -rf *.bin
			$(Q)rm -f *.axf
			$(Q)rm -f *.i *.s
			$(Q)rm -f *.ar
			$(Q)rm -f *.map
			$(Q)rm -f *.scf
			$(Q)rm -f *.txt
			$(Q)rm -f *.elf
			$(Q)rm -rf *.bin
			$(Q)rm -f $(LISTFILE)

clean:
			@echo "  CLEAN"
			$(Q)rm -f *.o *.ld

.PHONY:			dump

dump:
			@echo "  OBJDUMP"
			$(OD) -d bl1.elf > bl1.dump
			$(OD) -d bl2.elf > bl2.dump
			$(OD) -d bl31.elf > bl31.dump

%.o:			%.S
			@echo "  AS      $<"
			$(Q)$(AS) $(ASFLAGS) -c $< -o $@

%.o:			%.c
			@echo "  CC      $<"
			$(Q)$(CC) $(CFLAGS) -c $< -o $@

%.ld:			%.ld.S
			@echo "  LDS      $<"
			$(Q)$(AS) $(ASFLAGS) -P -E $< -o $@


bl1.elf:		$(OBJS) $(BL1_OBJS) bl1.ld
			@echo "  LD      $@"
			$(Q)$(LD) -o $@ $(LDFLAGS) $(BL1_LDFLAGS) $(OBJS) $(BL1_OBJS)
			@echo "Built $@ successfully"
			@echo

bl2.elf:		$(OBJS) $(BL2_OBJS) bl2.ld
			@echo "  LD      $@"
			$(Q)$(LD) -o $@ $(LDFLAGS) $(BL2_LDFLAGS) $(OBJS) $(BL2_OBJS)
			@echo "Built $@ successfully"
			@echo

bl31.elf:		$(OBJS) $(BL31_OBJS) bl31.ld
			@echo "  LD      $@"
			$(Q)$(LD) -o $@ $(LDFLAGS) $(BL31_LDFLAGS) $(OBJS) $(BL31_OBJS)
			@echo "Built $@ successfully"
			@echo

%.bin:			%.elf
			$(OC) -O binary $< $@

#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

# Cross Compile
M0_CROSS_COMPILE ?= arm-none-eabi-

# Build architecture
ARCH		:= cortex-m0

# Build platform
PLAT_M0		?= rk3399m0

ifeq (${V},0)
	Q=@
else
	Q=
endif
export Q

.SUFFIXES:

INCLUDES		+= -Iinclude/

# NOTE: Add C source files here
C_SOURCES		:= src/startup.c \
			   src/main.c

# Flags definition
CFLAGS			:= -g
ASFLAGS			:= -g -Wa,--gdwarf-2

ASFLAGS			+= -mcpu=$(ARCH) -mthumb -Wall -ffunction-sections -O3
CFLAGS			+= -mcpu=$(ARCH) -mthumb -Wall -ffunction-sections -O3

LDFLAGS			:= -mcpu=$(ARCH) -mthumb -g -nostartfiles -O3
LDFLAGS			+= -Wl,--gc-sections -Wl,--build-id=none

# Cross tool
CC			:= ${M0_CROSS_COMPILE}gcc
CPP			:= ${M0_CROSS_COMPILE}cpp
AS			:= ${M0_CROSS_COMPILE}gcc
AR			:= ${M0_CROSS_COMPILE}ar
LD			:= ${M0_CROSS_COMPILE}ld
OC			:= ${M0_CROSS_COMPILE}objcopy
OD			:= ${M0_CROSS_COMPILE}objdump
NM			:= ${M0_CROSS_COMPILE}nm
PP			:= ${M0_CROSS_COMPILE}gcc -E ${CFLAGS}

# NOTE: The line continuation '\' is required in the next define otherwise we
# end up with a line-feed characer at the end of the last c filename.
# Also bare this issue in mind if extending the list of supported filetypes.
define SOURCES_TO_OBJS
	$(notdir $(patsubst %.c,%.o,$(filter %.c,$(1)))) \
	$(notdir $(patsubst %.S,%.o,$(filter %.S,$(1))))
endef

SOURCES 		:= $(C_SOURCES)
OBJS 			:= $(addprefix $(BUILD)/,$(call SOURCES_TO_OBJS,$(SOURCES)))
LINKERFILE		:= src/rk3399m0.ld
MAPFILE			:= $(BUILD)/$(PLAT_M0).map
ELF 			:= $(BUILD)/$(PLAT_M0).elf
BIN 			:= $(BUILD)/$(PLAT_M0).bin

# Function definition related compilation
define MAKE_C
$(eval OBJ := $(1)/$(patsubst %.c,%.o,$(notdir $(2))))
-include $(patsubst %.o,%.d,$(OBJ))

$(OBJ) : $(2)
	@echo "  CC      $$<"
	$$(Q)$$(CC) $$(CFLAGS) $$(INCLUDES) -MMD -MT $$@ -c $$< -o $$@
endef

define MAKE_S
$(eval OBJ := $(1)/$(patsubst %.S,%.o,$(notdir $(2))))

$(OBJ) : $(2)
	@echo "  AS      $$<"
	$$(Q)$$(AS) $$(ASFLAGS) -c $$< -o $$@
endef

define MAKE_OBJS
	$(eval C_OBJS := $(filter %.c,$(2)))
	$(eval REMAIN := $(filter-out %.c,$(2)))
	$(eval $(foreach obj,$(C_OBJS),$(call MAKE_C,$(1),$(obj),$(3))))

	$(eval S_OBJS := $(filter %.S,$(REMAIN)))
	$(eval REMAIN := $(filter-out %.S,$(REMAIN)))
	$(eval $(foreach obj,$(S_OBJS),$(call MAKE_S,$(1),$(obj),$(3))))

	$(and $(REMAIN),$(error Unexpected source files present: $(REMAIN)))
endef

.PHONY: all
all: $(BIN)

$(ELF) : $(OBJS) $(LINKERFILE)
	@echo "  LD      $@"
	$(Q)$(CC) -o $@ $(LDFLAGS) -Wl,-Map=$(MAPFILE) -Wl,-T$(LINKERFILE) \
		$(OBJS)

$(BIN) : $(ELF)
	@echo "  BIN     $@"
	$(Q)$(OC) -O binary $< $@

$(eval $(call MAKE_OBJS,$(BUILD),$(SOURCES),$(1)))

#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
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

INCLUDES		+= -Iinclude/ \
			   -I../../include/shared/

# NOTE: Add C source files here
C_SOURCES		:= src/startup.c \
			   src/main.c	\
			   src/suspend.c \
			   src/dram.c	\
			   src/stopwatch.c

# Flags definition
COMMON_FLAGS		:= -g -mcpu=$(ARCH) -mthumb -Wall -O3 -nostdlib -mfloat-abi=soft
CFLAGS			:= -ffunction-sections -fdata-sections -fomit-frame-pointer -fno-common
ASFLAGS			:= -Wa,--gdwarf-2
LDFLAGS			:= -Wl,--gc-sections -Wl,--build-id=none

# Cross tool
CC			:= ${M0_CROSS_COMPILE}gcc
CPP			:= ${M0_CROSS_COMPILE}cpp
AR			:= ${M0_CROSS_COMPILE}ar
OC			:= ${M0_CROSS_COMPILE}objcopy
OD			:= ${M0_CROSS_COMPILE}objdump
NM			:= ${M0_CROSS_COMPILE}nm

# NOTE: The line continuation '\' is required in the next define otherwise we
# end up with a line-feed characer at the end of the last c filename.
# Also bare this issue in mind if extending the list of supported filetypes.
define SOURCES_TO_OBJS
	$(notdir $(patsubst %.c,%.o,$(filter %.c,$(1)))) \
	$(notdir $(patsubst %.S,%.o,$(filter %.S,$(1))))
endef

SOURCES 		:= $(C_SOURCES)
OBJS 			:= $(addprefix $(BUILD)/,$(call SOURCES_TO_OBJS,$(SOURCES)))
LINKERFILE		:= $(BUILD)/$(PLAT_M0).ld
MAPFILE			:= $(BUILD)/$(PLAT_M0).map
ELF 			:= $(BUILD)/$(PLAT_M0).elf
BIN 			:= $(BUILD)/$(PLAT_M0).bin
LINKERFILE_SRC		:= src/$(PLAT_M0).ld.S

# Function definition related compilation
define MAKE_C
$(eval OBJ := $(1)/$(patsubst %.c,%.o,$(notdir $(2))))
-include $(patsubst %.o,%.d,$(OBJ))

$(OBJ) : $(2)
	@echo "  CC      $$<"
	$$(Q)$$(CC) $$(COMMON_FLAGS) $$(CFLAGS) $$(INCLUDES) -MMD -MT $$@ -c $$< -o $$@
endef

define MAKE_S
$(eval OBJ := $(1)/$(patsubst %.S,%.o,$(notdir $(2))))

$(OBJ) : $(2)
	@echo "  AS      $$<"
	$$(Q)$$(CC) -x assembler-with-cpp $$(COMMON_FLAGS) $$(ASFLAGS) -c $$< -o $$@
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

.DEFAULT_GOAL := $(BIN)

$(LINKERFILE): $(LINKERFILE_SRC)
	$(CC) $(COMMON_FLAGS) $(INCLUDES) -P -E -D__LINKER__ -MMD -MF $@.d -MT $@ -o $@ $<
-include $(LINKERFILE).d

$(ELF) : $(OBJS) $(LINKERFILE)
	@echo "  LD      $@"
	$(Q)$(CC) -o $@ $(COMMON_FLAGS) $(LDFLAGS) -Wl,-Map=$(MAPFILE) -Wl,-T$(LINKERFILE) $(OBJS)

$(BIN) : $(ELF)
	@echo "  BIN     $@"
	$(Q)$(OC) -O binary $< $@

$(eval $(call MAKE_OBJS,$(BUILD),$(SOURCES),$(1)))

#
# Copyright (c) 2016-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include ../../../../../make_helpers/common.mk
include ../../../../../make_helpers/build_macros.mk
include ../../../../../make_helpers/toolchain.mk

# Build architecture
ARCH		:= rk3399-m0

# Build platform
PLAT_M0		?= rk3399m0
PLAT_M0_PMU	?= rk3399m0pmu

INCLUDES		+= -Iinclude/ \
			   -I../../include/shared/

# NOTE: Add C source files here
C_SOURCES_COMMON	:= src/startup.c
C_SOURCES		:= src/dram.c	\
			   src/stopwatch.c
C_SOURCES_PMU		:= src/suspend.c

# Flags definition
COMMON_FLAGS		:= -g -mcpu=cortex-m0 -mthumb -Wall -O3 -nostdlib -mfloat-abi=soft
CFLAGS			:= -ffunction-sections -fdata-sections -fomit-frame-pointer -fno-common
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105523
CFLAGS			+= $(call cc_option, --param=min-pagesize=0)
ASFLAGS			:= -Wa,--gdwarf-2 -Wa,--fatal-warnings
LDFLAGS			:= -Wl,--gc-sections -Wl,--build-id=none -Wl,--fatal-warnings -z noexecstack
LDFLAGS			+= $(call ld_option,--no-warn-rwx-segments)

# NOTE: The line continuation '\' is required in the next define otherwise we
# end up with a line-feed characer at the end of the last c filename.
# Also bare this issue in mind if extending the list of supported filetypes.
define SOURCES_TO_OBJS
	$(notdir $(patsubst %.c,%.o,$(filter %.c,$(1)))) \
	$(notdir $(patsubst %.S,%.o,$(filter %.S,$(1))))
endef

SOURCES_COMMON		:= $(C_SOURCES_COMMON)
SOURCES 		:= $(C_SOURCES)
SOURCES_PMU		:= $(C_SOURCES_PMU)
OBJS_COMMON		:= $(addprefix $(BUILD)/,$(call SOURCES_TO_OBJS,$(SOURCES_COMMON)))
OBJS 			:= $(addprefix $(BUILD)/,$(call SOURCES_TO_OBJS,$(SOURCES)))
OBJS_PMU 		:= $(addprefix $(BUILD)/,$(call SOURCES_TO_OBJS,$(SOURCES_PMU)))
LINKERFILE		:= $(BUILD)/$(PLAT_M0).ld
MAPFILE			:= $(BUILD)/$(PLAT_M0).map
MAPFILE_PMU		:= $(BUILD)/$(PLAT_M0_PMU).map
ELF 			:= $(BUILD)/$(PLAT_M0).elf
ELF_PMU			:= $(BUILD)/$(PLAT_M0_PMU).elf
BIN 			:= $(BUILD)/$(PLAT_M0).bin
BIN_PMU			:= $(BUILD)/$(PLAT_M0_PMU).bin
LINKERFILE_SRC		:= src/$(PLAT_M0).ld.S

# Function definition related compilation
define MAKE_C
$(eval OBJ := $(1)/$(patsubst %.c,%.o,$(notdir $(2))))
-include $(patsubst %.o,%.d,$(OBJ))

$(OBJ) : $(2)
	$(s)echo "  CC      $$<"
	$$(q)$($(ARCH)-cc) $$(COMMON_FLAGS) $$(CFLAGS) $$(INCLUDES) -MMD -MT $$@ -c $$< -o $$@
endef

define MAKE_S
$(eval OBJ := $(1)/$(patsubst %.S,%.o,$(notdir $(2))))

$(OBJ) : $(2)
	$(s)echo "  AS      $$<"
	$$(q)$($(ARCH)-as) -x assembler-with-cpp $$(COMMON_FLAGS) $$(ASFLAGS) -c $$< -o $$@
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
all: $(BIN) $(BIN_PMU)

.DEFAULT_GOAL := all

$(LINKERFILE): $(LINKERFILE_SRC)
	$(q)$($(ARCH)-cc) $(COMMON_FLAGS) $(INCLUDES) -P -E -D__LINKER__ -MMD -MF $@.d -MT $@ -o $@ $<
-include $(LINKERFILE).d

$(ELF) : $(OBJS) $(OBJS_COMMON) $(LINKERFILE)
	$(s)echo "  LD      $@"
	$(q)$($(ARCH)-ld) -o $@ $(COMMON_FLAGS) $(LDFLAGS) -Wl,-Map=$(MAPFILE) -Wl,-T$(LINKERFILE) $(OBJS) $(OBJS_COMMON)

$(BIN) $(BIN_PMU): %.bin: %.elf
	$(s)echo "  BIN     $@"
	$(q)$($(ARCH)-oc) -O binary $< $@

$(ELF_PMU) : $(OBJS_COMMON) $(OBJS_PMU) $(LINKERFILE)
	$(s)echo "  LD      $@"
	$(q)$($(ARCH)-ld) -o $@ $(COMMON_FLAGS) $(LDFLAGS) -Wl,-Map=$(MAPFILE_PMU) -Wl,-T$(LINKERFILE) $(OBJS_PMU) $(OBJS_COMMON)

$(eval $(call MAKE_OBJS,$(BUILD),$(SOURCES_COMMON),$(1)))
$(eval $(call MAKE_OBJS,$(BUILD),$(SOURCES),$(1)))
$(eval $(call MAKE_OBJS,$(BUILD),$(SOURCES_PMU),$(1)))

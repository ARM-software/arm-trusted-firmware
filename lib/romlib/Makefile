#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

AS          = $(CROSS_COMPILE)as
LD          = $(CROSS_COMPILE)ld
OC          = $(CROSS_COMPILE)objcopy
CPP         = $(CROSS_COMPILE)cpp
BUILD_DIR   = ../../$(BUILD_PLAT)/romlib
LIB_DIR     = ../../$(BUILD_PLAT)/lib
WRAPPER_DIR = ../../$(BUILD_PLAT)/libwrapper
LIBS        = -lmbedtls -lfdt -lc
INC         = $(INCLUDES:-I%=-I../../%)
PPFLAGS     = $(INC) $(DEFINES) -P -D__ASSEMBLY__ -D__LINKER__ -MD -MP -MT $(BUILD_DIR)/romlib.ld
OBJS        = $(BUILD_DIR)/jmptbl.o $(BUILD_DIR)/init.o

V ?= 0
ifeq ($(V),0)
  Q := @
else
  Q :=
endif

ifeq ($(DEBUG),1)
   CFLAGS  := -g
   LDFLAGS := -g
endif


.PHONY: all clean distclean

all: $(BUILD_DIR)/romlib.bin $(LIB_DIR)/libwrappers.a

%.o: %.s
	@echo "  AS      $@"
	$(Q)$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.s
	@echo "  AS      $@"
	$(Q)$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/romlib.ld: romlib.ld.S
	@echo "  PP      $@"
	$(Q)$(CPP) $(PPFLAGS) -o $@ romlib.ld.S

$(BUILD_DIR)/romlib.elf: $(OBJS) $(BUILD_DIR)/romlib.ld
	@echo "  LD      $@"
	$(Q)$(LD) -T $(BUILD_DIR)/romlib.ld -L$(LIB_DIR) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(BUILD_DIR)/romlib.bin: $(BUILD_DIR)/romlib.elf
	@echo "  BIN     $@"
	$(Q)$(OC) -O binary $(BUILD_DIR)/romlib.elf $@

$(WRAPPER_DIR)/jmpvar.s: $(BUILD_DIR)/romlib.elf
	@echo "  VAR     $@"
	$(Q)./genvar.sh -o $@ $(BUILD_DIR)/romlib.elf

$(LIB_DIR)/libwrappers.a: jmptbl.i $(WRAPPER_DIR)/jmpvar.o
	@echo "  AR      $@"
	$(Q)./genwrappers.sh -b $(WRAPPER_DIR) -o $@ jmptbl.i

$(BUILD_DIR)/jmptbl.s: jmptbl.i
	@echo "  TBL     $@"
	$(Q)./gentbl.sh -o $@ jmptbl.i

clean:
	@rm -f $(BUILD_DIR)/*

-include $(BUILD_DIR)/romlib.d

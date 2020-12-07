#
# Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

###################################################
# makefile
###################################################

#output file name
FILE_NAME_SA0 = bootparam_sa0
FILE_NAME_SA6 = cert_header_sa6

OUTPUT_FILE_SA0 = $(FILE_NAME_SA0).elf
OUTPUT_FILE_SA6 = $(FILE_NAME_SA6).elf

#object file name
OBJ_FILE_SA0 = sa0.o
OBJ_FILE_SA6 = sa6.o

#linker script name
MEMORY_DEF_SA0 = sa0.ld.S
MEMORY_DEF_SA6 = sa6.ld.S

###################################################
# Convenience function for adding build definitions
# $(eval $(call add_define,FOO)) will have:
# -DFOO if $(FOO) is empty; -DFOO=$(FOO) otherwise
define add_define
DEFINES += -D$(1)$(if $(value $(1)),=$(value $(1)),)
endef

# Process RCAR_SA0_SIZE flag
ifndef RCAR_SA0_SIZE
RCAR_SA0_SIZE := 1
else
ifeq (${RCAR_SA0_SIZE},0)
RCAR_SA0_SIZE := 0
else
RCAR_SA0_SIZE := 1
endif
endif
$(eval $(call add_define,RCAR_SA0_SIZE))

# Process RCAR_SA6_TYPE flag
ifndef RCAR_SA6_TYPE
RCAR_SA6_TYPE := 0
else
ifeq (${RCAR_SA6_TYPE},0)
RCAR_SA6_TYPE := 0
else
RCAR_SA6_TYPE := 1
endif
endif
$(eval $(call add_define,RCAR_SA6_TYPE))

RCAR_VMA_ADJUST_ADDR := 0xE6320000
$(eval $(call add_define,RCAR_VMA_ADJUST_ADDR))


###################################################

#c compiler
CC = $(CROSS_COMPILE)gcc
CFLAGS += ${DEFINES}
CFLAGS += -nostdinc \
	  -I../../../include/lib/libc \
	  -I../../../include/lib/libc/aarch64

#Linker
LD = $(CROSS_COMPILE)ld

#objcopy
objcopy = $(CROSS_COMPILE)objcopy

#clean
CL = rm -f

###################################################
.SUFFIXES : .s .c .o

###################################################
# command

.PHONY: all
all: $(OUTPUT_FILE_SA0) $(OUTPUT_FILE_SA6)
###################################################
# Linker
###################################################
$(OUTPUT_FILE_SA0) : $(MEMORY_DEF_SA0) $(OBJ_FILE_SA0)
	$(LD) $(OBJ_FILE_SA0)		 	\
	-T $(MEMORY_DEF_SA0)			\
	-o $(OUTPUT_FILE_SA0)			\
	-Map $(FILE_NAME_SA0).map 		\

	$(objcopy) -O srec --adjust-vma=$(RCAR_VMA_ADJUST_ADDR) --srec-forceS3  $(OUTPUT_FILE_SA0) $(FILE_NAME_SA0).srec
	$(objcopy) -O binary --adjust-vma=$(RCAR_VMA_ADJUST_ADDR) --srec-forceS3  $(OUTPUT_FILE_SA0) $(FILE_NAME_SA0).bin

$(OUTPUT_FILE_SA6) : $(MEMORY_DEF_SA6) $(OBJ_FILE_SA6)
	$(LD) $(OBJ_FILE_SA6)		 	\
	-T $(MEMORY_DEF_SA6)			\
	-o $(OUTPUT_FILE_SA6)			\
	-Map $(FILE_NAME_SA6).map 		\

	$(objcopy) -O srec --adjust-vma=$(RCAR_VMA_ADJUST_ADDR) --srec-forceS3  $(OUTPUT_FILE_SA6) $(FILE_NAME_SA6).srec
	$(objcopy) -O binary --adjust-vma=$(RCAR_VMA_ADJUST_ADDR) --srec-forceS3  $(OUTPUT_FILE_SA6) $(FILE_NAME_SA6).bin

###################################################
# Compile
###################################################

%.o:../%.c
	$(CC) -c -I $< -o $@

.PHONY: clean
clean:
	$(CL)  *.bin *.map *.srec *.elf *.o

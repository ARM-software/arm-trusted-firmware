#
# Copyright (C) 2018 Marvell International Ltd.
#
# SPDX-License-Identifier:     BSD-3-Clause
# https://spdx.org/licenses

PROJECT = doimage
OBJECTS = doimage.o

HOSTCCFLAGS = -Wall -Werror
ifeq (${DEBUG},1)
  HOSTCCFLAGS += -g -O0 -DDEBUG
else
  HOSTCCFLAGS += -O2
endif

ifeq (${MARVELL_SECURE_BOOT},1)
DOIMAGE_CC_FLAGS := -DCONFIG_MVEBU_SECURE_BOOT
DOIMAGE_LD_FLAGS := -lconfig -lmbedtls -lmbedcrypto -lmbedx509
endif

HOSTCCFLAGS += ${DOIMAGE_CC_FLAGS}

# Make soft links and include from local directory otherwise wrong headers
# could get pulled in from firmware tree.
INCLUDE_PATHS = -I.

HOSTCC ?= gcc
RM := rm -rf

.PHONY: all clean

all: ${PROJECT}

${PROJECT}: ${OBJECTS} Makefile
	@echo "  HOSTLD  $@"
	${Q}${HOSTCC} ${OBJECTS} ${DOIMAGE_LD_FLAGS} -o $@
	@echo
	@echo "Built $@ successfully"
	@echo

%.o: %.c Makefile
	@echo "  HOSTCC  $<"
	${Q}${HOSTCC} -c ${HOSTCCFLAGS} ${INCLUDE_PATHS} $< -o $@

clean:
	${Q}${RM} ${PROJECT}
	${Q}${RM} ${OBJECTS}

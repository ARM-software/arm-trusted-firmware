#
# Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk

PROJECT := stm32image${BIN_EXT}
OBJECTS := stm32image.o
V := 0

override CPPFLAGS += -D_GNU_SOURCE
CFLAGS := -Wall -Werror -pedantic -std=c99
ifeq (${DEBUG},1)
  CFLAGS += -g -O0 -DDEBUG
else
  CFLAGS += -O2
endif

ifeq (${V},0)
  Q := @
else
  Q :=
endif

CC := gcc

.PHONY: all clean distclean

all: ${PROJECT}

${PROJECT}: ${OBJECTS} Makefile
	@echo "  LD      $@"
	${Q}${CC} ${OBJECTS} -o $@
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

%.o: %.c %.h Makefile
	@echo "  CC      $<"
	${Q}${CC} -c ${CFLAGS} $< -o $@

clean:
	$(call SHELL_DELETE_ALL, ${PROJECT} ${OBJECTS})

distclean: clean

#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

MAKE_HELPERS_DIRECTORY := ../../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk

PROJECT_1 := create_pbl${BIN_EXT}
OBJECTS_1 := create_pbl.o
PROJECT_2 := byte_swap${BIN_EXT}
OBJECTS_2 := byte_swap.o
V ?= 0

override CPPFLAGS += -D_GNU_SOURCE -D_XOPEN_SOURCE=700
CFLAGS := -Wall -Werror -pedantic -std=c99
ifeq (${DEBUG},1)
  CFLAGS += -g -O0 -DDEBUG
else
  CFLAGS += -O2
endif
LDLIBS :=

ifeq (${V},0)
  Q := @
else
  Q :=
endif

INCLUDE_PATHS :=

HOSTCC ?= gcc
CC = gcc

.PHONY: all clean distclean

all: create_pbl byte_swap

${PROJECT_1}: ${OBJECTS_1} Makefile
	@echo "  LD      $@"
	${Q}${HOSTCC} ${OBJECTS_1} -o $@ ${LDLIBS}
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

${PROJECT_2}: ${OBJECTS_2} Makefile
	@echo "  LD      $@"
	${Q}${HOSTCC} ${OBJECTS_2} -o $@ ${LDLIBS}
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

%.o: %.c %.h Makefile
	@echo "  CC      $<"
	${Q}${HOSTCC} -c ${CPPFLAGS} ${CFLAGS} ${INCLUDE_PATHS} $< -o $@

clean:
	$(call SHELL_DELETE_ALL, ${PROJECT_1} ${OBJECTS_1})
	$(call SHELL_DELETE_ALL, ${PROJECT_2} ${OBJECTS_2})

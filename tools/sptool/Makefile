#
# Copyright (c) 2018, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk

PROJECT := sptool${BIN_EXT}
OBJECTS := sptool.o
V ?= 0

override CPPFLAGS += -D_GNU_SOURCE -D_XOPEN_SOURCE=700
HOSTCCFLAGS := -Wall -Werror -pedantic -std=c99
ifeq (${DEBUG},1)
  HOSTCCFLAGS += -g -O0 -DDEBUG
else
  HOSTCCFLAGS += -O2
endif

ifeq (${V},0)
  Q := @
else
  Q :=
endif

INCLUDE_PATHS := -I../../include/tools_share

HOSTCC ?= gcc

.PHONY: all clean distclean

all: ${PROJECT}

${PROJECT}: ${OBJECTS} Makefile
	@echo "  HOSTLD  $@"
	${Q}${HOSTCC} ${OBJECTS} -o $@ ${LDLIBS}
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

%.o: %.c Makefile
	@echo "  HOSTCC  $<"
	${Q}${HOSTCC} -c ${CPPFLAGS} ${HOSTCCFLAGS} ${INCLUDE_PATHS} $< -o $@

clean:
	$(call SHELL_DELETE_ALL, ${PROJECT} ${OBJECTS})

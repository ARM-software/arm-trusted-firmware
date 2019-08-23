#
# Copyright (C) 2019 Remi Pommarel <repk@triplefau.lt>
#
# SPDX-License-Identifier:     BSD-3-Clause
# https://spdx.org/licenses
#
MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk

PROJECT := doimage${BIN_EXT}
OBJECTS := doimage.o
V := 0

HOSTCCFLAGS := -Wall -Werror -pedantic -std=c99 -D_GNU_SOURCE

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

HOSTCC := gcc

.PHONY: all clean distclean

all: ${PROJECT}

${PROJECT}: ${OBJECTS} Makefile
	@echo "  HOSTLD  $@"
	${Q}${HOSTCC} ${OBJECTS} -o $@
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

%.o: %.c Makefile
	@echo "  HOSTCC  $<"
	${Q}${HOSTCC} -c ${HOSTCCFLAGS} $< -o $@

clean:
	$(call SHELL_DELETE_ALL, ${PROJECT} ${OBJECTS})

distclean: clean

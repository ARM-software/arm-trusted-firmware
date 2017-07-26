#
# Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk

PROJECT := fiptool${BIN_EXT}
OBJECTS := fiptool.o tbbr_config.o
V ?= 0

override CPPFLAGS += -D_GNU_SOURCE -D_XOPEN_SOURCE=700
CFLAGS := -Wall -Werror -pedantic -std=c99
ifeq (${DEBUG},1)
  CFLAGS += -g -O0 -DDEBUG
else
  CFLAGS += -O2
endif
LDLIBS := -lcrypto

ifeq (${V},0)
  Q := @
else
  Q :=
endif

INCLUDE_PATHS := -I../../include/tools_share

HOSTCC ?= gcc

.PHONY: all clean distclean

all: ${PROJECT} fip_create

${PROJECT}: ${OBJECTS} Makefile
	@echo "  LD      $@"
	${Q}${HOSTCC} ${OBJECTS} -o $@ ${LDLIBS}
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

fip_create: fip_create.sh
	${Q}mkdir -p ../fip_create
	${Q}install -m 755 fip_create.sh ../fip_create/fip_create

%.o: %.c %.h Makefile
	@echo "  CC      $<"
	${Q}${HOSTCC} -c ${CPPFLAGS} ${CFLAGS} ${INCLUDE_PATHS} $< -o $@

clean:
	$(call SHELL_DELETE_ALL, ${PROJECT} ${OBJECTS} fip_create)

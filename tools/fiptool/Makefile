#
# Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk

PROJECT := fiptool${BIN_EXT}
OBJECTS := fiptool.o tbbr_config.o
V := 0
COPIED_H_FILES := uuid.h firmware_image_package.h

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

# Only include from local directory (see comment below).
INCLUDE_PATHS := -I.

CC := gcc

.PHONY: all clean distclean

all: ${PROJECT} fip_create

${PROJECT}: ${OBJECTS} Makefile
	@echo "  LD      $@"
	${Q}${CC} ${OBJECTS} -o $@ ${LDLIBS}
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

fip_create: fip_create.sh
	${Q}mkdir -p ../fip_create
	${Q}install -m 755 fip_create.sh ../fip_create/fip_create

%.o: %.c %.h ${COPIED_H_FILES} Makefile
	@echo "  CC      $<"
	${Q}${CC} -c ${CPPFLAGS} ${CFLAGS} ${INCLUDE_PATHS} $< -o $@

#
# Copy required library headers to a local directory so they can be included
# by this project without adding the library directories to the system include
# path. This avoids conflicts with definitions in the compiler standard
# include path.
#
uuid.h : ../../include/lib/stdlib/sys/uuid.h
	$(call SHELL_COPY,$<,$@)

firmware_image_package.h : ../../include/common/firmware_image_package.h
	$(call SHELL_COPY,$<,$@)

clean:
	$(call SHELL_DELETE_ALL, ${PROJECT} ${OBJECTS} fip_create)

distclean: clean
	$(call SHELL_DELETE_ALL, ${COPIED_H_FILES})

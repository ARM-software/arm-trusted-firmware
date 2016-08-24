#
# Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# Neither the name of ARM nor the names of its contributors may be used
# to endorse or promote products derived from this software without specific
# prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
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

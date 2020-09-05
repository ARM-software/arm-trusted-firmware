#
# Copyright (c) 2019-2020, Linaro Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

V		?= 0
BUILD_INFO	?= 1
DEBUG		:= 0
ENCTOOL		?= encrypt_fw${BIN_EXT}
BINARY		:= $(notdir ${ENCTOOL})
OPENSSL_DIR	:= /usr

OBJECTS := src/encrypt.o \
           src/cmd_opt.o \
           src/main.o

HOSTCCFLAGS := -Wall -std=c99

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk

ifeq (${DEBUG},1)
  HOSTCCFLAGS += -g -O0 -DDEBUG -DLOG_LEVEL=40
else
ifeq (${BUILD_INFO},1)
  HOSTCCFLAGS += -O2 -DLOG_LEVEL=20
else
  HOSTCCFLAGS += -O2 -DLOG_LEVEL=10
endif
endif
ifeq (${V},0)
  Q := @
else
  Q :=
endif

# Make soft links and include from local directory otherwise wrong headers
# could get pulled in from firmware tree.
INC_DIR := -I ./include -I ../../include/tools_share -I ${OPENSSL_DIR}/include
LIB_DIR := -L ${OPENSSL_DIR}/lib
LIB := -lssl -lcrypto

HOSTCC ?= gcc

.PHONY: all clean realclean

all: clean ${BINARY}

${BINARY}: ${OBJECTS} Makefile
	@echo "  HOSTLD  $@"
	@echo 'const char build_msg[] = "Built : "__TIME__", "__DATE__;' | \
                ${HOSTCC} -c ${HOSTCCFLAGS} -xc - -o src/build_msg.o
	${Q}${HOSTCC} src/build_msg.o ${OBJECTS} ${LIB_DIR} ${LIB} -o $@

%.o: %.c
	@echo "  HOSTCC  $<"
	${Q}${HOSTCC} -c ${HOSTCCFLAGS} ${INC_DIR} $< -o $@

clean:
	$(call SHELL_DELETE_ALL, src/build_msg.o ${OBJECTS})

realclean: clean
	$(call SHELL_DELETE,${BINARY})

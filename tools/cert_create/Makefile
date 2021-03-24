#
# Copyright (c) 2015-2021, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT		:= none
V		?= 0
DEBUG		:= 0
CRTTOOL		?= cert_create${BIN_EXT}
BINARY		:= $(notdir ${CRTTOOL})
OPENSSL_DIR	:= /usr
COT		:= tbbr

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk

ifneq (${PLAT},none)
TF_PLATFORM_ROOT	:=	../../plat/
include ${MAKE_HELPERS_DIRECTORY}plat_helpers.mk
PLAT_CERT_CREATE_HELPER_MK := ${PLAT_DIR}/cert_create_tbbr.mk
endif

# Common source files.
OBJECTS := src/cert.o \
           src/cmd_opt.o \
           src/ext.o \
           src/key.o \
           src/main.o \
           src/sha.o

# Chain of trust.
ifeq (${COT},tbbr)
  include src/tbbr/tbbr.mk
else ifeq (${COT},dualroot)
  include src/dualroot/cot.mk
else
  $(error Unknown chain of trust ${COT})
endif

ifneq (,$(wildcard ${PLAT_CERT_CREATE_HELPER_MK}))
include ${PLAT_CERT_CREATE_HELPER_MK}
endif

HOSTCCFLAGS := -Wall -std=c99

ifeq (${DEBUG},1)
  HOSTCCFLAGS += -g -O0 -DDEBUG -DLOG_LEVEL=40
else
  HOSTCCFLAGS += -O2 -DLOG_LEVEL=20
endif

ifeq (${V},0)
  Q := @
else
  Q :=
endif

HOSTCCFLAGS += ${DEFINES}

# Make soft links and include from local directory otherwise wrong headers
# could get pulled in from firmware tree.
INC_DIR += -I ./include -I ${PLAT_INCLUDE} -I ${OPENSSL_DIR}/include
LIB_DIR := -L ${OPENSSL_DIR}/lib
LIB := -lssl -lcrypto

HOSTCC ?= gcc

.PHONY: all clean realclean

all: ${BINARY}

${BINARY}: ${OBJECTS} Makefile
	@echo "  HOSTLD  $@"
	@echo 'const char build_msg[] = "Built : "__TIME__", "__DATE__; \
                const char platform_msg[] = "${PLAT_MSG}";' | \
                ${HOSTCC} -c ${HOSTCCFLAGS} -xc - -o src/build_msg.o
	${Q}${HOSTCC} src/build_msg.o ${OBJECTS} ${LIB_DIR} ${LIB} -o $@

%.o: %.c
	@echo "  HOSTCC  $<"
	${Q}${HOSTCC} -c ${HOSTCCFLAGS} ${INC_DIR} $< -o $@

clean:
	$(call SHELL_DELETE_ALL, src/build_msg.o ${OBJECTS})

realclean: clean
	$(call SHELL_DELETE,${BINARY})


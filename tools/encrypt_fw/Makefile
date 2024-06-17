#
# Copyright (c) 2019-2022, Linaro Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BUILD_INFO	?= 1
DEBUG		:= 0
ENCTOOL		?= encrypt_fw${BIN_EXT}
BINARY		:= $(notdir ${ENCTOOL})
OPENSSL_DIR	:= /usr

toolchains := host

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk
include ${MAKE_HELPERS_DIRECTORY}common.mk
include ${MAKE_HELPERS_DIRECTORY}defaults.mk
include ${MAKE_HELPERS_DIRECTORY}toolchain.mk

OBJECTS := src/encrypt.o \
           src/cmd_opt.o \
           src/main.o

HOSTCCFLAGS := -Wall -std=c99

# Select OpenSSL version flag according to the OpenSSL build selected
# from setting the OPENSSL_DIR path.
$(eval $(call SELECT_OPENSSL_API_VERSION))

ifeq (${DEBUG},1)
  HOSTCCFLAGS += -g -O0 -DDEBUG -DLOG_LEVEL=40
else
ifeq (${BUILD_INFO},1)
  HOSTCCFLAGS += -O2 -DLOG_LEVEL=20
else
  HOSTCCFLAGS += -O2 -DLOG_LEVEL=10
endif
endif

HOSTCCFLAGS += ${DEFINES}
# USING_OPENSSL3 flag will be added to the HOSTCCFLAGS variable with the proper
# computed value.
HOSTCCFLAGS += -DUSING_OPENSSL3=$(USING_OPENSSL3)


# Make soft links and include from local directory otherwise wrong headers
# could get pulled in from firmware tree.
INC_DIR := -I ./include -I ../../include/tools_share -I ${OPENSSL_DIR}/include

# Include library directories where OpenSSL library files are located.
# For a normal installation (i.e.: when ${OPENSSL_DIR} = /usr or
# /usr/local), binaries are located under the ${OPENSSL_DIR}/lib/
# directory. However, for a local build of OpenSSL, the built binaries are
# located under the main project directory (i.e.: ${OPENSSL_DIR}, not
# ${OPENSSL_DIR}/lib/).
LIB_DIR := -L ${OPENSSL_DIR}/lib -L ${OPENSSL_DIR}
LIB := -lssl -lcrypto

.PHONY: all clean realclean --openssl

all: --openssl ${BINARY}

${BINARY}: ${OBJECTS} Makefile
	$(s)echo "  HOSTLD  $@"
	$(q)echo 'const char build_msg[] = "Built : "__TIME__", "__DATE__;' | \
                $(host-cc) -c ${HOSTCCFLAGS} -xc - -o src/build_msg.o
	$(q)$(host-cc) src/build_msg.o ${OBJECTS} ${LIB_DIR} ${LIB} -o $@

%.o: %.c
	$(s)echo "  HOSTCC  $<"
	$(q)$(host-cc) -c ${HOSTCCFLAGS} ${INC_DIR} $< -o $@

--openssl:
ifeq ($(DEBUG),1)
	$(s)echo "Selected OpenSSL version: ${OPENSSL_CURRENT_VER}"
endif

clean:
	$(call SHELL_DELETE_ALL, src/build_msg.o ${OBJECTS})

realclean: clean
	$(call SHELL_DELETE,${BINARY})

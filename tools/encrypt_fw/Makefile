#
# Copyright (c) 2024-2025, Arm Limited. All rights reserved.
# Copyright (c) 2019-2022, Linaro Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BUILD_INFO	?= 1
DEBUG		:= 0
OPENSSL_DIR	:= /usr

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build-rules.mk
include ${MAKE_HELPERS_DIRECTORY}common.mk
include ${MAKE_HELPERS_DIRECTORY}defaults.mk
include ${MAKE_HELPERS_DIRECTORY}toolchain.mk

ENCTOOL_SOURCES :=	src/encrypt.c \
			src/cmd_opt.c \
			src/main.c

ENCTOOL_CFLAGS := -Wall -std=c99

# Select OpenSSL version flag according to the OpenSSL build selected
# from setting the OPENSSL_DIR path.
$(eval $(call SELECT_OPENSSL_API_VERSION))

ifeq (${DEBUG},1)
  ENCTOOL_CFLAGS += -g -O0
  ENCTOOL_DEFINES += DEBUG LOG_LEVEL=40
else
  ENCTOOL_CFLAGS += -O2
ifeq (${BUILD_INFO},1)
  ENCTOOL_DEFINES += LOG_LEVEL=20
else
  ENCTOOL_DEFINES += LOG_LEVEL=10
endif
endif

# USING_OPENSSL3 flag will be added to the proper computed value.
ENCTOOL_DEFINES += USING_OPENSSL3=$(USING_OPENSSL3)


# Make soft links and include from local directory otherwise wrong headers
# could get pulled in from firmware tree.
ENCTOOL_INCLUDE_DIRS := ./include ../../include/tools_share ${OPENSSL_DIR}/include

# Include library directories where OpenSSL library files are located.
# For a normal installation (i.e.: when ${OPENSSL_DIR} = /usr or
# /usr/local), binaries are located under the ${OPENSSL_DIR}/lib/
# directory. However, for a local build of OpenSSL, the built binaries are
# located under the main project directory (i.e.: ${OPENSSL_DIR}, not
# ${OPENSSL_DIR}/lib/).
ENCTOOL_LDFLAGS := -L${OPENSSL_DIR}/lib -L${OPENSSL_DIR}
ENCTOOL_LDFLAGS += -lssl -lcrypto

.PHONY: all clean realclean --openssl

all: --openssl

$(eval $(call MAKE_TOOL,$(BUILD_PLAT)/tools,encrypt_fw,ENCTOOL))

--openssl:
ifeq ($(DEBUG),1)
	$(s)echo "Selected OpenSSL version: ${OPENSSL_CURRENT_VER}"
endif

clean:
	$(q)rm -rf $(BUILD_PLAT)/tools/encrypt_fw

realclean: clean

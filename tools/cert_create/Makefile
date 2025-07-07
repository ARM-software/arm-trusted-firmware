#
# Copyright (c) 2015-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build-rules.mk
include ${MAKE_HELPERS_DIRECTORY}common.mk
include ${MAKE_HELPERS_DIRECTORY}defaults.mk
include ${MAKE_HELPERS_DIRECTORY}toolchain.mk
include ${MAKE_HELPERS_DIRECTORY}utilities.mk

ifneq (${PLAT},none)
TF_PLATFORM_ROOT	:=	../../plat/
include ${MAKE_HELPERS_DIRECTORY}plat_helpers.mk
PLAT_CERT_CREATE_HELPER_MK := ${PLAT_DIR}/cert_create_tbbr.mk
endif

# Common source files.
CRTTOOL_SOURCES	:= src/cert.c \
		src/cmd_opt.c \
		src/ext.c \
		src/key.c \
		src/main.c \
		src/sha.c

# Chain of trust.
ifeq (${COT},tbbr)
  include src/tbbr/tbbr.mk
else ifeq (${COT},dualroot)
  include src/dualroot/cot.mk
else ifeq (${COT},cca)
  include src/cca/cot.mk
else
  $(error Unknown chain of trust ${COT})
endif

ifneq (,$(wildcard ${PLAT_CERT_CREATE_HELPER_MK}))
include ${PLAT_CERT_CREATE_HELPER_MK}
endif

# Select OpenSSL version flag according to the OpenSSL build selected
# from setting the OPENSSL_DIR path.
$(eval $(call SELECT_OPENSSL_API_VERSION))

CRTTOOL_CFLAGS := -Wall -std=c99

ifeq (${DEBUG},1)
  CRTTOOL_DEFINES += DEBUG LOG_LEVEL=40
  CRTTOOL_CFLAGS+= -g -O0
else
  CRTTOOL_DEFINES += LOG_LEVEL=20
  CRTTOOL_CFLAGS += -O2
endif

CRTTOOL_DEFINES += PLAT_MSG=$(call escape-shell,"$(PLAT_MSG)")
# USING_OPENSSL3 flag will be added to the HOSTCCFLAGS variable with the proper
# computed value.
CRTTOOL_DEFINES += USING_OPENSSL3=$(USING_OPENSSL3)

# Make soft links and include from local directory otherwise wrong headers
# could get pulled in from firmware tree.
CRTTOOL_INCLUDE_DIRS += ./include ${PLAT_INCLUDE} ${OPENSSL_DIR}/include

# Include library directories where OpenSSL library files are located.
# For a normal installation (i.e.: when ${OPENSSL_DIR} = /usr or
# /usr/local), binaries are located under the ${OPENSSL_DIR}/lib/
# directory. However, for a local build of OpenSSL, the built binaries are
# located under the main project directory (i.e.: ${OPENSSL_DIR}, not
# ${OPENSSL_DIR}/lib/).
CRTTOOL_LDFLAGS += -L ${OPENSSL_DIR}/lib -L ${OPENSSL_DIR}
CRTTOOL_LDFLAGS += -lssl -lcrypto

.PHONY: all clean realclean --openssl

$(eval $(call MAKE_TOOL,$(BUILD_PLAT)/tools,cert_create,CRTTOOL))

all: --openssl

--openssl:
ifeq ($(DEBUG),1)
	$(s)echo "Selected OpenSSL version: ${OPENSSL_CURRENT_VER}"
endif

clean:
	$(q)rm -rf $(BUILD_PLAT)/tools/cert_create

realclean: clean

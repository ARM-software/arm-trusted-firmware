#
# Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build-rules.mk
include ${MAKE_HELPERS_DIRECTORY}common.mk
include ${MAKE_HELPERS_DIRECTORY}defaults.mk
include ${MAKE_HELPERS_DIRECTORY}toolchain.mk

FIPTOOL_SOURCES := fiptool.c tbbr_config.c
STATIC ?= 0

FIPTOOL_DEFINES += _GNU_SOURCE _XOPEN_SOURCE=700
FIPTOOL_CFLAGS := -Wall -Werror -pedantic -std=c99
ifeq (${DEBUG},1)
  FIPTOOL_CFLAGS += -g -O0 -DDEBUG
else
  FIPTOOL_CFLAGS += -O2
endif

FIPTOOL_INCLUDE_DIRS := ../../include/tools_share

FIPTOOL_DEFINES += STATIC=$(STATIC)

ifeq (${STATIC},1)
FIPTOOL_LDFLAGS := -static
else
OPENSSL_DIR := /usr

# Select OpenSSL version flag according to the OpenSSL build selected
# from setting the OPENSSL_DIR path.
$(eval $(call SELECT_OPENSSL_API_VERSION))

# USING_OPENSSL3 flag will be added to the HOSTCCFLAGS variable with the proper
# computed value.
FIPTOOL_DEFINES += USING_OPENSSL3=$(USING_OPENSSL3)

# Include library directories where OpenSSL library files are located.
# For a normal installation (i.e.: when ${OPENSSL_DIR} = /usr or
# /usr/local), binaries are located under the ${OPENSSL_DIR}/lib/
# directory. However, for a local build of OpenSSL, the built binaries are
# located under the main project directory (i.e.: ${OPENSSL_DIR}, not
# ${OPENSSL_DIR}/lib/).
FIPTOOL_LDFLAGS := -L${OPENSSL_DIR}/lib -L${OPENSSL_DIR} -lcrypto
FIPTOOL_INCLUDE_DIRS += ${OPENSSL_DIR}/include
endif # STATIC

ifneq (${PLAT},)
TF_PLATFORM_ROOT	:=	../../plat/
include ${MAKE_HELPERS_DIRECTORY}plat_helpers.mk
COMBINED_PATH_FRAG := plat_fiptool/
PLAT_FIPTOOL_HELPER_MK := $(foreach path_frag,$(subst /, ,$(patsubst ../../plat/%/,%,${PLAT_DIR})),\
			  $(eval COMBINED_PATH_FRAG := ${COMBINED_PATH_FRAG}/${path_frag})\
			  $(wildcard ${COMBINED_PATH_FRAG}/plat_fiptool.mk))
endif

ifneq (,$(wildcard $(lastword ${PLAT_FIPTOOL_HELPER_MK})))
include ${PLAT_FIPTOOL_HELPER_MK}
endif

$(eval $(call MAKE_TOOL,$(BUILD_PLAT)/tools,fiptool,FIPTOOL))

.PHONY: all clean distclean --openssl

all: --openssl

--openssl:
ifeq ($(STATIC),0)
ifeq ($(DEBUG),1)
	$(s)echo "Selected OpenSSL version: ${OPENSSL_CURRENT_VER}"
endif
endif # STATIC

clean:
	$(q)rm -rf $(BUILD_PLAT)/tools/fiptool

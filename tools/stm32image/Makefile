#
# Copyright (c) 2017-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build-rules.mk
include ${MAKE_HELPERS_DIRECTORY}common.mk
include ${MAKE_HELPERS_DIRECTORY}toolchain.mk

STM32IMAGE_SOURCES := stm32image.c

STM32IMAGE_CFLAGS := -Wall -Werror -pedantic -std=c99
STM32IMAGE_DEFINES := _GNU_SOURCE

ifeq (${DEBUG},1)
  STM32IMAGE_CFLAGS += -g -O0
  STM32IMAGE_DEFINES += DEBUG
else
  STM32IMAGE_CFLAGS += -O2
endif

.PHONY: all clean distclean

all:

$(eval $(call MAKE_TOOL,$(BUILD_PLAT)/tools,stm32image,STM32IMAGE))

clean:
	$(q)rm -rf $(BUILD_PLAT)/tools/stm32image

distclean: clean

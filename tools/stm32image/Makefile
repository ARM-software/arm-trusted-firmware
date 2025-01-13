#
# Copyright (c) 2017-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}common.mk
include ${MAKE_HELPERS_DIRECTORY}toolchain.mk

PROJECT := stm32image$(.exe)
OBJECTS := stm32image.o

HOSTCCFLAGS := -Wall -Werror -pedantic -std=c99 -D_GNU_SOURCE

ifeq (${DEBUG},1)
  HOSTCCFLAGS += -g -O0 -DDEBUG
else
  HOSTCCFLAGS += -O2
endif

.PHONY: all clean distclean

all: ${PROJECT}

${PROJECT}: ${OBJECTS} Makefile
	$(s)echo "  HOSTLD  $@"
	$(q)$(host-cc) ${OBJECTS} -o $@
	$(s)echo
	$(s)echo "Built $@ successfully"
	$(s)echo

%.o: %.c Makefile
	$(s)echo "  HOSTCC  $<"
	$(q)$(host-cc) -c ${HOSTCCFLAGS} $< -o $@

clean:
	$(q)rm -rf $(PROJECT) $(OBJECTS)

distclean: clean

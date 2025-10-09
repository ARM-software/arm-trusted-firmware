#
# Copyright (C) 2019 Remi Pommarel <repk@triplefau.lt>
# Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier:     BSD-3-Clause
# https://spdx.org/licenses
#

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}common.mk
include ${MAKE_HELPERS_DIRECTORY}toolchain.mk

PROJECT := doimage$(.exe)
OBJECTS := doimage.o

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

$(OBJECTS): %.o: %.c Makefile
	$(s)echo "  HOSTCC  $<"
	$(q)$(host-cc) -c ${HOSTCCFLAGS} $< -o $@

clean:
	$(q)rm -rf $(PROJECT) $(OBJECTS)

distclean: clean

#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

toolchains := host

MAKE_HELPERS_DIRECTORY := ../../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk
include ${MAKE_HELPERS_DIRECTORY}common.mk
include ${MAKE_HELPERS_DIRECTORY}toolchain.mk

PROJECT_1 := create_pbl${BIN_EXT}
OBJECTS_1 := create_pbl.o
PROJECT_2 := byte_swap${BIN_EXT}
OBJECTS_2 := byte_swap.o

override CPPFLAGS += -D_GNU_SOURCE -D_XOPEN_SOURCE=700
CFLAGS := -Wall -Werror -pedantic -std=c99
ifeq (${DEBUG},1)
  CFLAGS += -g -O0 -DDEBUG
else
  CFLAGS += -O2
endif
LDLIBS :=

INCLUDE_PATHS :=

.PHONY: all clean distclean

all: create_pbl byte_swap

${PROJECT_1}: ${OBJECTS_1} Makefile
	$(s)echo "  LD      $@"
	$(q)$(host-cc) ${OBJECTS_1} -o $@ ${LDLIBS}
	$(s)echo
	$(s)echo "Built $@ successfully"
	$(s)echo

${PROJECT_2}: ${OBJECTS_2} Makefile
	$(s)echo "  LD      $@"
	$(q)$(host-cc) ${OBJECTS_2} -o $@ ${LDLIBS}
	$(s)echo
	$(s)echo "Built $@ successfully"
	$(s)echo

%.o: %.c %.h Makefile
	$(s)echo "  CC      $<"
	$(q)$(host-cc) -c ${CPPFLAGS} ${CFLAGS} ${INCLUDE_PATHS} $< -o $@

clean:
	$(call SHELL_DELETE_ALL, ${PROJECT_1} ${OBJECTS_1})
	$(call SHELL_DELETE_ALL, ${PROJECT_2} ${OBJECTS_2})

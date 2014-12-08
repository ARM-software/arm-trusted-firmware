#
# Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# Neither the name of ARM nor the names of its contributors may be used
# to endorse or promote products derived from this software without specific
# prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

PROJECT = fip_create
OBJECTS = fip_create.o

CFLAGS = -Wall -Werror -pedantic -std=c99
ifeq (${DEBUG},1)
  CFLAGS += -g -O0 -DDEBUG
else
  CFLAGS += -O2
endif

# Make soft links and include from local directory otherwise wrong headers
# could get pulled in from firmware tree.
INCLUDE_PATHS = -I.

CC := gcc
RM := rm -rf

.PHONY: all clean

all: ${PROJECT}

${PROJECT}: ${OBJECTS} Makefile
	@echo "  LD      $@"
	${Q}${CC} ${OBJECTS} -o $@
	@echo
	@echo "Built $@ successfully"
	@echo

%.o: %.c %.h Makefile
	@echo "  CC      $<"
	${Q}${CC} -c ${CFLAGS} ${INCLUDE_PATHS} $< -o $@

clean:
	${Q}${RM} ${PROJECT}
	${Q}${RM} ${OBJECTS}

#
# Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

TSPD_DIR		:=	services/spd/tspd
SPD_INCLUDES		:=	-Iinclude/bl32/tsp

SPD_SOURCES		:=	services/spd/tspd/tspd_common.c		\
				services/spd/tspd/tspd_helpers.S	\
				services/spd/tspd/tspd_main.c		\
				services/spd/tspd/tspd_pm.c

# This dispatcher is paired with a Test Secure Payload source and we intend to
# build the Test Secure Payload along with this dispatcher.
#
# In cases where an associated Secure Payload lies outside this build
# system/source tree, the the dispatcher Makefile can either invoke an external
# build command or assume it pre-built

BL32_ROOT		:=	bl32/tsp

# Include SP's Makefile. The assumption is that the TSP's build system is
# compatible with that of Trusted Firmware, and it'll add and populate necessary
# build targets and variables
include ${BL32_ROOT}/tsp.mk

# Let the top-level Makefile know that we intend to build the SP from source
NEED_BL32		:=	yes

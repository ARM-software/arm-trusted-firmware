#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

ifneq (${ARCH}, aarch32)
	$(error SP_MIN is only supported on AArch32 platforms)
endif

include lib/psci/psci_lib.mk

INCLUDES		+=	-Iinclude/bl32/sp_min

BL32_SOURCES		+=	bl32/sp_min/sp_min_main.c		\
				bl32/sp_min/aarch32/entrypoint.S	\
				common/runtime_svc.c			\
				services/std_svc/std_svc_setup.c	\
				${PSCI_LIB_SOURCES}

ifeq (${ENABLE_PMF}, 1)
BL32_SOURCES		+=	lib/pmf/pmf_main.c
endif

BL32_LINKERFILE	:=	bl32/sp_min/sp_min.ld.S

# Include the platform-specific SP_MIN Makefile
# If no platform-specific SP_MIN Makefile exists, it means SP_MIN is not supported
# on this platform.
SP_MIN_PLAT_MAKEFILE := $(wildcard ${PLAT_DIR}/sp_min/sp_min-${PLAT}.mk)
ifeq (,${SP_MIN_PLAT_MAKEFILE})
  $(error SP_MIN is not supported on platform ${PLAT})
else
  include ${SP_MIN_PLAT_MAKEFILE}
endif

RESET_TO_SP_MIN	:= 0
$(eval $(call add_define,RESET_TO_SP_MIN))
$(eval $(call assert_boolean,RESET_TO_SP_MIN))

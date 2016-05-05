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

PSCI_LIB_SOURCES	:=	lib/el3_runtime/cpu_data_array.c	\
				lib/el3_runtime/${ARCH}/cpu_data.S	\
				lib/el3_runtime/${ARCH}/context_mgmt.c	\
				lib/cpus/${ARCH}/cpu_helpers.S		\
				lib/locks/exclusive/${ARCH}/spinlock.S	\
				lib/psci/psci_off.c			\
				lib/psci/psci_on.c			\
				lib/psci/psci_suspend.c			\
				lib/psci/psci_common.c			\
				lib/psci/psci_main.c			\
				lib/psci/psci_setup.c			\
				lib/psci/psci_system_off.c		\
				lib/psci/${ARCH}/psci_helpers.S

ifeq (${ARCH}, aarch64)
PSCI_LIB_SOURCES	+=	lib/el3_runtime/aarch64/context.S
endif

ifeq (${USE_COHERENT_MEM}, 1)
PSCI_LIB_SOURCES		+=	lib/locks/bakery/bakery_lock_coherent.c
else
PSCI_LIB_SOURCES		+=	lib/locks/bakery/bakery_lock_normal.c
endif

ifeq (${ENABLE_PSCI_STAT}, 1)
PSCI_LIB_SOURCES		+=	lib/psci/psci_stat.c
endif

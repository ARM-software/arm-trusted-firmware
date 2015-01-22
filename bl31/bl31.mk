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

BL31_SOURCES		+=	bl31/bl31_main.c				\
				bl31/context_mgmt.c				\
				bl31/cpu_data_array.c				\
				bl31/runtime_svc.c				\
				bl31/interrupt_mgmt.c				\
				bl31/aarch64/bl31_arch_setup.c			\
				bl31/aarch64/bl31_entrypoint.S			\
				bl31/aarch64/context.S				\
				bl31/aarch64/cpu_data.S				\
				bl31/aarch64/runtime_exceptions.S		\
				bl31/aarch64/crash_reporting.S			\
				lib/cpus/aarch64/cpu_helpers.S			\
				lib/locks/exclusive/spinlock.S			\
				services/std_svc/std_svc_setup.c		\
				services/std_svc/psci/psci_afflvl_off.c		\
				services/std_svc/psci/psci_afflvl_on.c		\
				services/std_svc/psci/psci_afflvl_suspend.c	\
				services/std_svc/psci/psci_common.c		\
				services/std_svc/psci/psci_entry.S		\
				services/std_svc/psci/psci_helpers.S		\
				services/std_svc/psci/psci_main.c		\
				services/std_svc/psci/psci_setup.c		\
				services/std_svc/psci/psci_system_off.c

ifeq (${USE_COHERENT_MEM}, 1)
BL31_SOURCES		+=	lib/locks/bakery/bakery_lock_coherent.c
else
BL31_SOURCES		+=	lib/locks/bakery/bakery_lock_normal.c
endif

BL31_LINKERFILE		:=	bl31/bl31.ld.S

# Flag used by the generic interrupt management framework to  determine if
# upon the assertion of an interrupt, it should pass the interrupt id or not
IMF_READ_INTERRUPT_ID	:=	0

$(eval $(call assert_boolean,IMF_READ_INTERRUPT_ID))
$(eval $(call add_define,IMF_READ_INTERRUPT_ID))

# Flag used to inidicate if Crash reporting via console should be included
# in BL3-1. This defaults to being present in DEBUG builds only
ifndef CRASH_REPORTING
CRASH_REPORTING		:=	$(DEBUG)
endif

$(eval $(call assert_boolean,CRASH_REPORTING))
$(eval $(call add_define,CRASH_REPORTING))

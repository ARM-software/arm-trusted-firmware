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

vpath			%.c	common					\
				lib					\
				arch/system/gic				\
				plat/${PLAT}				\
				arch/${ARCH}				\
				services/std_svc			\
				services/std_svc/psci			\
				lib/sync/locks/bakery			\
				plat/${PLAT}/${ARCH}			\
				${PLAT_BL31_C_VPATH}

vpath			%.S	lib/arch/${ARCH}			\
				services/std_svc			\
				services/std_svc/psci			\
				include					\
				plat/${PLAT}/${ARCH}			\
				lib/sync/locks/exclusive		\
				plat/common/${ARCH}			\
				arch/system/gic/${ARCH}			\
				common/${ARCH}				\
				${PLAT_BL31_S_VPATH}

BL31_SOURCES		+=	bl31_arch_setup.c			\
				bl31_entrypoint.S			\
				runtime_exceptions.S			\
				bl31_main.c				\
				std_svc_setup.c				\
				psci_entry.S				\
				psci_setup.c				\
				psci_common.c				\
				psci_afflvl_on.c			\
				psci_main.c				\
				psci_afflvl_off.c			\
				psci_afflvl_suspend.c			\
				spinlock.S				\
				gic_v3_sysregs.S			\
				bakery_lock.c				\
				runtime_svc.c				\
				early_exceptions.S			\
				context_mgmt.c				\
				context.S

BL31_LINKERFILE		:=	bl31.ld.S

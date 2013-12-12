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

vpath			%.c	drivers/arm/interconnect/cci-400/ common/ lib/			\
				drivers/arm/peripherals/pl011 plat/fvp common/psci		\
				lib/semihosting arch/aarch64/ lib/stdlib lib/sync/locks/bakery/	\
				drivers/power/ arch/system/gic/ plat/fvp/aarch64/

vpath			%.S	lib/arch/aarch64 common/psci					\
				lib/semihosting/aarch64 include/ plat/fvp/${ARCH}		\
				lib/sync/locks/exclusive plat/common/aarch64/			\
				arch/system/gic/${ARCH}

BL31_ASM_OBJS		:=	bl31_entrypoint.o runtime_exceptions.o psci_entry.o		\
				spinlock.o gic_v3_sysregs.o fvp_helpers.o
BL31_C_OBJS		:=	bl31_main.o bl31_plat_setup.o bl31_arch_setup.o		\
				exception_handlers.o bakery_lock.o cci400.o 		\
				fvp_common.o fvp_pm.o fvp_pwrc.o fvp_topology.o		\
				runtime_svc.o fvp_gic.o gic_v2.o gic_v3.o psci_setup.o	\
				psci_common.o psci_afflvl_on.o psci_main.o		\
				psci_afflvl_off.o psci_afflvl_suspend.o

BL31_ENTRY_POINT	:=	bl31_entrypoint
BL31_MAPFILE		:=	bl31.map
BL31_LINKERFILE		:=	bl31.ld

BL31_OBJS		:= 	$(BL31_C_OBJS) $(BL31_ASM_OBJS)

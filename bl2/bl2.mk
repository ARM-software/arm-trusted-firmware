#
# Copyright (c) 2013, ARM Limited and Contributors. All rights reserved.
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

vpath			%.c	common/ drivers/arm/interconnect/cci-400/		\
				drivers/arm/peripherals/pl011 common/ lib/ 		\
				plat/fvp plat/fvp/${ARCH} lib/semihosting arch/aarch64/	\
				lib/stdlib

vpath			%.S	lib/arch/aarch64					\
				lib/semihosting/aarch64					\
				include lib/sync/locks/exclusive

BL2_ASM_OBJS		:=	bl2_entrypoint.o spinlock.o
BL2_C_OBJS		:=	bl2_main.o bl2_plat_setup.o bl2_arch_setup.o fvp_common.o	\
				early_exceptions.o
BL2_ENTRY_POINT		:=	bl2_entrypoint
BL2_MAPFILE		:=	bl2.map
BL2_LINKERFILE		:=	bl2.ld

BL2_OBJS		:= 	$(BL2_C_OBJS) $(BL2_ASM_OBJS)
CFLAGS		        += 	$(DEFINES)

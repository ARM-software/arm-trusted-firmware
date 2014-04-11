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

#
# No additional platform system include directories required
#
# PLAT_INCLUDES		:=

PLAT_BL1_C_VPATH	:=	drivers/arm/cci400			\
				drivers/arm/pl011			\
				lib/${ARCH}				\
				lib/semihosting				\
				lib/stdlib				\
				drivers/io

PLAT_BL1_S_VPATH	:=	lib/semihosting/${ARCH}

PLAT_BL2_C_VPATH	:=	drivers/arm/cci400			\
				drivers/arm/pl011			\
				drivers/arm/tzc400			\
				lib/${ARCH}				\
				lib/stdlib				\
				lib/semihosting				\
				drivers/io

PLAT_BL2_S_VPATH	:=	lib/semihosting/${ARCH}

PLAT_BL31_C_VPATH	:=	drivers/arm/cci-400			\
				drivers/arm/pl011			\
				lib/${ARCH}				\
				lib/semihosting				\
				lib/stdlib				\
				drivers/io				\
				plat/fvp/drivers/pwrc

PLAT_BL31_S_VPATH	:=	lib/semihosting/${ARCH}

PLAT_BL_COMMON_SOURCES	:=	semihosting_call.S			\
				mmio.c					\
				pl011_console.c				\
				pl011.c					\
				semihosting.c				\
				sysreg_helpers.S			\
				plat_io_storage.c			\
				io_semihosting.c			\
				io_fip.c				\
				io_memmap.c				\
				xlat_tables.c

BL1_SOURCES		+=	bl1_plat_setup.c			\
				bl1_plat_helpers.S			\
				plat_helpers.S				\
				platform_up_stack.S			\
				plat_common.c				\
				cci400.c

BL2_SOURCES		+=	bl2_plat_setup.c			\
				platform_up_stack.S			\
				plat_common.c				\
				plat_security.c				\
				tzc400.c

BL31_SOURCES		+=	bl31_plat_setup.c			\
				plat_helpers.S				\
				platform_mp_stack.S			\
				plat_common.c				\
				plat_pm.c				\
				plat_topology.c				\
				plat_gic.c				\
				fvp_pwrc.c				\
				cci400.c				\
				gic_v2.c				\
				gic_v3.c

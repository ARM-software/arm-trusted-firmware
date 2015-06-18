#
# Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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


PLAT_INCLUDES		:=	-Iinclude/plat/arm/board/common			\
				-Iplat/arm/board/fvp/include


PLAT_BL_COMMON_SOURCES	:=	drivers/arm/pl011/pl011_console.S		\
				plat/arm/board/fvp/aarch64/fvp_common.c

BL1_SOURCES		+=	drivers/io/io_semihosting.c			\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a57.S			\
				lib/semihosting/semihosting.c			\
				lib/semihosting/aarch64/semihosting_call.S	\
				plat/arm/board/fvp/aarch64/fvp_helpers.S	\
				plat/arm/board/fvp/fvp_bl1_setup.c		\
				plat/arm/board/fvp/fvp_io_storage.c

BL2_SOURCES		+=	drivers/arm/sp804/sp804_delay_timer.c		\
				drivers/io/io_semihosting.c			\
				drivers/delay_timer/delay_timer.c		\
				lib/semihosting/semihosting.c			\
				lib/semihosting/aarch64/semihosting_call.S	\
				plat/arm/board/fvp/fvp_bl2_setup.c		\
				plat/arm/board/fvp/fvp_io_storage.c		\
				plat/arm/board/fvp/fvp_security.c

BL31_SOURCES		+=	lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a57.S			\
				plat/arm/board/fvp/fvp_bl31_setup.c		\
				plat/arm/board/fvp/fvp_pm.c			\
				plat/arm/board/fvp/fvp_security.c		\
				plat/arm/board/fvp/fvp_topology.c		\
				plat/arm/board/fvp/aarch64/fvp_helpers.S	\
				plat/arm/board/fvp/drivers/pwrc/fvp_pwrc.c

ifneq (${TRUSTED_BOARD_BOOT},0)
  BL1_SOURCES		+=	plat/arm/board/common/board_arm_trusted_boot.c
  BL2_SOURCES		+=	plat/arm/board/common/board_arm_trusted_boot.c
endif


include plat/arm/common/arm_common.mk

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

PLAT_INCLUDES		:=	-Iplat/juno/include/

PLAT_BL_COMMON_SOURCES	:=	drivers/arm/pl011/pl011_console.S	\
				drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				drivers/io/io_storage.c			\
				lib/aarch64/xlat_tables.c		\
				plat/common/aarch64/plat_common.c	\
				plat/juno/plat_io_storage.c

BL1_SOURCES		+=	drivers/arm/cci400/cci400.c		\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				plat/common/aarch64/platform_up_stack.S	\
				plat/juno/bl1_plat_setup.c		\
				plat/juno/aarch64/bl1_plat_helpers.S	\
				plat/juno/aarch64/plat_helpers.S	\
				plat/juno/aarch64/juno_common.c

BL2_SOURCES		+=	lib/locks/bakery/bakery_lock.c		\
				plat/common/aarch64/platform_up_stack.S	\
				plat/juno/bl2_plat_setup.c		\
				plat/juno/mhu.c				\
				plat/juno/aarch64/plat_helpers.S	\
				plat/juno/aarch64/juno_common.c		\
				plat/juno/scp_bootloader.c		\
				plat/juno/scpi.c

BL31_SOURCES		+=	drivers/arm/cci400/cci400.c		\
				drivers/arm/gic/gic_v2.c		\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				plat/common/aarch64/platform_mp_stack.S	\
				plat/juno/bl31_plat_setup.c		\
				plat/juno/mhu.c				\
				plat/juno/aarch64/plat_helpers.S	\
				plat/juno/aarch64/juno_common.c		\
				plat/juno/plat_pm.c			\
				plat/juno/plat_topology.c		\
				plat/juno/plat_gic.c			\
				plat/juno/scpi.c

ifneq (${RESET_TO_BL31},0)
  $(error "Using BL3-1 as the reset vector is not supported on Juno. \
  Please set RESET_TO_BL31 to 0.")
endif

NEED_BL30		:=	yes

# Enable workarounds for selected Cortex-A57 erratas.
ERRATA_A57_806969	:=	1
ERRATA_A57_813420	:=	1

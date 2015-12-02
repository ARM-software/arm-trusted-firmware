#
# Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

CRASH_REPORTING		:=	1
$(eval $(call add_define,CRASH_REPORTING))

ASM_ASSERTION		:=	1
$(eval $(call add_define,ASM_ASSERTION))

USE_COHERENT_MEM	:=	0

PLAT_INCLUDES		:=	-Iplat/nvidia/tegra/include/drivers \
				-Iplat/nvidia/tegra/include \
				-Iplat/nvidia/tegra/include/${TARGET_SOC}

PLAT_BL_COMMON_SOURCES	:=	lib/aarch64/xlat_tables.c			\
				plat/common/aarch64/plat_common.c

COMMON_DIR		:=	plat/nvidia/tegra/common

BL31_SOURCES		+=	drivers/arm/gic/gic_v2.c			\
				drivers/arm/gic/gic_v3.c			\
				drivers/console/console.S			\
				drivers/delay_timer/delay_timer.c		\
				drivers/ti/uart/16550_console.S			\
				plat/common/aarch64/platform_mp_stack.S		\
				plat/common/aarch64/plat_psci_common.c		\
				${COMMON_DIR}/aarch64/tegra_helpers.S		\
				${COMMON_DIR}/drivers/memctrl/memctrl.c		\
				${COMMON_DIR}/drivers/pmc/pmc.c			\
				${COMMON_DIR}/drivers/flowctrl/flowctrl.c	\
				${COMMON_DIR}/tegra_bl31_setup.c		\
				${COMMON_DIR}/tegra_delay_timer.c		\
				${COMMON_DIR}/tegra_gic.c			\
				${COMMON_DIR}/tegra_pm.c			\
				${COMMON_DIR}/tegra_sip_calls.c			\
				${COMMON_DIR}/tegra_topology.c

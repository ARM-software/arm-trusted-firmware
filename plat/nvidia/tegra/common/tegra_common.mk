#
# Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
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

ENABLE_ASSERTIONS	:=	1
$(eval $(call add_define,ENABLE_ASSERTIONS))

USE_COHERENT_MEM	:=	0

SEPARATE_CODE_AND_RODATA :=	1

PLAT_XLAT_TABLES_DYNAMIC :=	1
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))

PLAT_INCLUDES		:=	-Iplat/nvidia/tegra/include/drivers \
				-Iplat/nvidia/tegra/include \
				-Iplat/nvidia/tegra/include/${TARGET_SOC}

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

COMMON_DIR		:=	plat/nvidia/tegra/common

BL31_SOURCES		+=	drivers/arm/gic/gic_v2.c			\
				drivers/console/aarch64/console.S		\
				drivers/delay_timer/delay_timer.c		\
				drivers/ti/uart/aarch64/16550_console.S		\
				${COMMON_DIR}/aarch64/tegra_helpers.S		\
				${COMMON_DIR}/drivers/pmc/pmc.c			\
				${COMMON_DIR}/tegra_bl31_setup.c		\
				${COMMON_DIR}/tegra_delay_timer.c		\
				${COMMON_DIR}/tegra_fiq_glue.c			\
				${COMMON_DIR}/tegra_gic.c			\
				${COMMON_DIR}/tegra_platform.c			\
				${COMMON_DIR}/tegra_pm.c			\
				${COMMON_DIR}/tegra_sip_calls.c			\
				${COMMON_DIR}/tegra_topology.c

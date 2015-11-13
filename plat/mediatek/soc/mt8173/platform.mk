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

MTK_PLAT		:=	plat/mediatek
PLAT_SOC_DIR		:=	plat/mediatek/soc/${PLAT}

PLAT_INCLUDES		:=	-I${MTK_PLAT}/soc/common/			\
				-I${PLAT_SOC_DIR}/drivers/gpio/			\
				-I${PLAT_SOC_DIR}/drivers/i2c/			\
				-I${PLAT_SOC_DIR}/drivers/mtcmos/		\
				-I${PLAT_SOC_DIR}/drivers/pmic/			\
				-I${PLAT_SOC_DIR}/drivers/rtc/			\
				-I${PLAT_SOC_DIR}/drivers/spm/			\
				-I${PLAT_SOC_DIR}/drivers/timer/		\
				-I${PLAT_SOC_DIR}/drivers/uart/			\
				-I${PLAT_SOC_DIR}/include/

PLAT_BL_COMMON_SOURCES	:=	lib/aarch64/xlat_tables.c			\
				plat/common/aarch64/plat_common.c		\
				plat/common/plat_gic.c

BL31_SOURCES		+=	drivers/arm/cci/cci.c				\
				drivers/arm/gic/arm_gic.c			\
				drivers/arm/gic/gic_v2.c			\
				drivers/arm/gic/gic_v3.c			\
				drivers/console/console.S			\
				drivers/delay_timer/delay_timer.c		\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a57.S			\
				lib/cpus/aarch64/cortex_a72.S			\
				plat/common/aarch64/platform_mp_stack.S		\
				${MTK_PLAT}/soc/common/mtk_board_func.c		\
				${MTK_PLAT}/soc/common/mtk_sip_svc.c		\
				${PLAT_SOC_DIR}/aarch64/plat_helpers.S		\
				${PLAT_SOC_DIR}/aarch64/platform_common.c	\
				${PLAT_SOC_DIR}/bl31_plat_setup.c		\
				${PLAT_SOC_DIR}/drivers/gpio/gpio.c		\
				${PLAT_SOC_DIR}/drivers/i2c/i2c.c		\
				${PLAT_SOC_DIR}/drivers/mtcmos/mtcmos.c		\
				${PLAT_SOC_DIR}/drivers/pmic/pmic_wrap_init.c	\
				${PLAT_SOC_DIR}/drivers/rtc/rtc.c		\
				${PLAT_SOC_DIR}/drivers/spm/spm.c		\
				${PLAT_SOC_DIR}/drivers/spm/spm_hotplug.c	\
				${PLAT_SOC_DIR}/drivers/spm/spm_mcdi.c		\
				${PLAT_SOC_DIR}/drivers/spm/spm_suspend.c	\
				${PLAT_SOC_DIR}/drivers/timer/mt_cpuxgpt.c	\
				${PLAT_SOC_DIR}/drivers/uart/8250_console.S	\
				${PLAT_SOC_DIR}/plat_delay_timer.c		\
				${PLAT_SOC_DIR}/plat_mt_gic.c			\
				${PLAT_SOC_DIR}/plat_pm.c			\
				${PLAT_SOC_DIR}/plat_sip_calls.c		\
				${PLAT_SOC_DIR}/plat_topology.c			\
				${PLAT_SOC_DIR}/power_tracer.c			\
				${PLAT_SOC_DIR}/scu.c

# include common board
include plat/mediatek/board/common/board_common.mk

# Flag used by the MTK_platform port to determine the version of ARM GIC
# architecture to use for interrupt management in EL3.
ARM_GIC_ARCH		:=	2
$(eval $(call add_define,ARM_GIC_ARCH))

# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_826319	:=	1
ERRATA_A53_836870	:=	1

# indicate the reset vector address can be programmed
PROGRAMMABLE_RESET_ADDRESS	:=	1

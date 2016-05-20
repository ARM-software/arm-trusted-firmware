#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

RK_PLAT         :=      plat/rockchip
RK_PLAT_SOC     :=      ${RK_PLAT}/${PLAT}
RK_PLAT_COMMON  :=      ${RK_PLAT}/common

PLAT_INCLUDES           :=	-I${RK_PLAT_COMMON}/				\
                                -I${RK_PLAT_COMMON}/include/			\
                                -I${RK_PLAT_COMMON}/pmusram			\
                                -I${RK_PLAT_COMMON}/drivers/pmu/			\
				-I${RK_PLAT_SOC}/				\
                                -I${RK_PLAT_SOC}/drivers/pmu/                   \
                                -I${RK_PLAT_SOC}/drivers/soc/                   \
                                -I${RK_PLAT_SOC}/include/                       \

RK_GIC_SOURCES          :=      drivers/arm/gic/common/gic_common.c     \
                                drivers/arm/gic/v3/gicv3_main.c         \
                                drivers/arm/gic/v3/gicv3_helpers.c      \
                                plat/common/plat_gicv3.c                \
                                ${RK_PLAT}/common/rockchip_gicv3.c

PLAT_BL_COMMON_SOURCES  :=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
                                plat/common/aarch64/plat_common.c               \
				plat/common/aarch64/plat_psci_common.c

BL31_SOURCES            +=      ${RK_GIC_SOURCES}                               \
                                drivers/arm/cci/cci.c                           \
                                drivers/console/console.S                       \
                                drivers/ti/uart/16550_console.S                 \
                                drivers/delay_timer/delay_timer.c               \
                                drivers/delay_timer/generic_delay_timer.c	\
                                lib/cpus/aarch64/cortex_a53.S                   \
                                lib/cpus/aarch64/cortex_a72.S                   \
                                plat/common/aarch64/platform_mp_stack.S         \
                                ${RK_PLAT_COMMON}/aarch64/plat_helpers.S        \
                                ${RK_PLAT_COMMON}/bl31_plat_setup.c             \
                                ${RK_PLAT_COMMON}/pmusram/pmu_sram_cpus_on.S	\
				${RK_PLAT_COMMON}/pmusram/pmu_sram.c		\
                                ${RK_PLAT_COMMON}/plat_pm.c                     \
                                ${RK_PLAT_COMMON}/plat_topology.c               \
                                ${RK_PLAT_COMMON}/aarch64/platform_common.c        \
                                ${RK_PLAT_SOC}/drivers/pmu/pmu.c                \
                                ${RK_PLAT_SOC}/drivers/soc/soc.c

ENABLE_PLAT_COMPAT      :=      0

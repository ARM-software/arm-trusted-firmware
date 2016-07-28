#
# Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
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

JUNO_GIC_SOURCES	:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c		\
				plat/arm/common/arm_gicv2.c

JUNO_INTERCONNECT_SOURCES	:=	drivers/arm/cci/cci.c		\
					plat/arm/common/arm_cci.c

JUNO_SECURITY_SOURCES	:=	drivers/arm/tzc/tzc400.c		\
				plat/arm/board/juno/juno_security.c	\
				plat/arm/common/arm_tzc400.c


PLAT_INCLUDES		:=	-Iplat/arm/board/juno/include

PLAT_BL_COMMON_SOURCES	:=	plat/arm/board/juno/aarch64/juno_helpers.S

BL1_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				lib/cpus/aarch64/cortex_a72.S		\
				plat/arm/board/juno/juno_bl1_setup.c	\
				plat/arm/board/juno/juno_err.c		\
				${JUNO_INTERCONNECT_SOURCES}

BL2_SOURCES		+=	plat/arm/board/juno/juno_err.c		\
				${JUNO_SECURITY_SOURCES}

BL2U_SOURCES		+=	${JUNO_SECURITY_SOURCES}

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				lib/cpus/aarch64/cortex_a72.S		\
				plat/arm/board/juno/juno_pm.c		\
				plat/arm/board/juno/juno_topology.c	\
				${JUNO_GIC_SOURCES}			\
				${JUNO_INTERCONNECT_SOURCES}		\
				${JUNO_SECURITY_SOURCES}

# Enable workarounds for selected Cortex-A57 erratas.
ERRATA_A57_806969		:=	0
ERRATA_A57_813420		:=	1

# Enable option to skip L1 data cache flush during the Cortex-A57 cluster
# power down sequence
SKIP_A57_L1_FLUSH_PWR_DWN	:=	 1

# Disable the PSCI platform compatibility layer
ENABLE_PLAT_COMPAT		:= 	0

# Enable memory map related constants optimisation
ARM_BOARD_OPTIMISE_MEM		:=	1

include plat/arm/board/common/board_css.mk
include plat/arm/common/arm_common.mk
include plat/arm/soc/common/soc_css.mk
include plat/arm/css/common/css_common.mk

ifeq (${KEY_ALG},ecdsa)
    $(error "ECDSA key algorithm is not fully supported on Juno.")
endif

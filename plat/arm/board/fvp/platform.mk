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

# Use the Legacy GICv3 driver on the FVP by default to maintain compatibility.
FVP_USE_GIC_DRIVER	:= FVP_GICV3_LEGACY

# The FVP platform depends on this macro to build with correct GIC driver.
$(eval $(call add_define,FVP_USE_GIC_DRIVER))

# Choose the GIC sources depending upon the how the FVP will be invoked
ifeq (${FVP_USE_GIC_DRIVER}, FVP_GICV3)
FVP_GIC_SOURCES		:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v3/gicv3_main.c		\
				drivers/arm/gic/v3/gicv3_helpers.c	\
				plat/common/plat_gicv3.c		\
				plat/arm/common/arm_gicv3.c
else ifeq (${FVP_USE_GIC_DRIVER}, FVP_GICV2)
FVP_GIC_SOURCES		:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c		\
				plat/arm/common/arm_gicv2.c
else ifeq (${FVP_USE_GIC_DRIVER}, FVP_GICV3_LEGACY)
FVP_GIC_SOURCES		:=	drivers/arm/gic/arm_gic.c		\
				drivers/arm/gic/gic_v2.c		\
				drivers/arm/gic/gic_v3.c		\
				plat/common/plat_gic.c			\
				plat/arm/common/arm_gicv3_legacy.c
else
$(error "Incorrect GIC driver chosen on FVP port")
endif

PLAT_INCLUDES		:=	-Iplat/arm/board/fvp/include


PLAT_BL_COMMON_SOURCES	:=	plat/arm/board/fvp/aarch64/fvp_common.c

BL1_SOURCES		+=	drivers/io/io_semihosting.c			\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a57.S			\
				lib/semihosting/semihosting.c			\
				lib/semihosting/aarch64/semihosting_call.S	\
				plat/arm/board/fvp/aarch64/fvp_helpers.S	\
				plat/arm/board/fvp/fvp_bl1_setup.c		\
				plat/arm/board/fvp/fvp_err.c			\
				plat/arm/board/fvp/fvp_io_storage.c

BL2_SOURCES		+=	drivers/arm/sp804/sp804_delay_timer.c		\
				drivers/io/io_semihosting.c			\
				drivers/delay_timer/delay_timer.c		\
				lib/semihosting/semihosting.c			\
				lib/semihosting/aarch64/semihosting_call.S	\
				plat/arm/board/fvp/fvp_bl2_setup.c		\
				plat/arm/board/fvp/fvp_err.c			\
				plat/arm/board/fvp/fvp_io_storage.c		\
				plat/arm/board/fvp/fvp_security.c

BL2U_SOURCES		+=	plat/arm/board/fvp/fvp_bl2u_setup.c		\
				plat/arm/board/fvp/fvp_security.c

BL31_SOURCES		+=	lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a57.S			\
				plat/arm/board/fvp/fvp_bl31_setup.c		\
				plat/arm/board/fvp/fvp_pm.c			\
				plat/arm/board/fvp/fvp_security.c		\
				plat/arm/board/fvp/fvp_topology.c		\
				plat/arm/board/fvp/aarch64/fvp_helpers.S	\
				plat/arm/board/fvp/drivers/pwrc/fvp_pwrc.c	\
				${FVP_GIC_SOURCES}

# Disable the PSCI platform compatibility layer
ENABLE_PLAT_COMPAT	:= 	0

include plat/arm/board/common/board_common.mk
include plat/arm/common/arm_common.mk

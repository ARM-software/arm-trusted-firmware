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

# On ARM standard platorms, the TSP can execute from Trusted SRAM, Trusted
# DRAM (if available) or the TZC secured area of DRAM.
# Trusted SRAM is the default.

ARM_TSP_RAM_LOCATION	:=	tsram
ifeq (${ARM_TSP_RAM_LOCATION}, tsram)
  ARM_TSP_RAM_LOCATION_ID = ARM_TRUSTED_SRAM_ID
else ifeq (${ARM_TSP_RAM_LOCATION}, tdram)
  ARM_TSP_RAM_LOCATION_ID = ARM_TRUSTED_DRAM_ID
else ifeq (${ARM_TSP_RAM_LOCATION}, dram)
  ARM_TSP_RAM_LOCATION_ID = ARM_DRAM_ID
else
  $(error "Unsupported ARM_TSP_RAM_LOCATION value")
endif

# Process flags
$(eval $(call add_define,ARM_TSP_RAM_LOCATION_ID))

PLAT_INCLUDES		+=	-Iinclude/plat/arm/common			\
				-Iinclude/plat/arm/common/aarch64


PLAT_BL_COMMON_SOURCES	+=	lib/aarch64/xlat_tables.c			\
				plat/arm/common/aarch64/arm_common.c		\
				plat/arm/common/aarch64/arm_helpers.S		\
				plat/common/aarch64/plat_common.c

BL1_SOURCES		+=	drivers/arm/cci/cci.c				\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/arm/common/arm_bl1_setup.c			\
				plat/arm/common/arm_io_storage.c		\
				plat/common/aarch64/platform_up_stack.S

BL2_SOURCES		+=	drivers/arm/tzc400/tzc400.c			\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/arm/common/arm_bl2_setup.c			\
				plat/arm/common/arm_io_storage.c		\
				plat/arm/common/arm_security.c			\
				plat/common/aarch64/platform_up_stack.S

BL31_SOURCES		+=	drivers/arm/cci/cci.c				\
				drivers/arm/gic/arm_gic.c			\
				drivers/arm/gic/gic_v2.c			\
				drivers/arm/gic/gic_v3.c			\
				drivers/arm/tzc400/tzc400.c			\
				plat/arm/common/arm_bl31_setup.c		\
				plat/arm/common/arm_pm.c			\
				plat/arm/common/arm_security.c			\
				plat/arm/common/arm_topology.c			\
				plat/common/plat_gic.c				\
				plat/common/aarch64/platform_mp_stack.S

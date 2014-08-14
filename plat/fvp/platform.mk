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

# Shared memory may be allocated at the top of Trusted SRAM (tsram) or at the
# base of Trusted SRAM (tdram)
FVP_SHARED_DATA_LOCATION	:=	tsram
ifeq (${FVP_SHARED_DATA_LOCATION}, tsram)
  FVP_SHARED_DATA_LOCATION_ID := FVP_IN_TRUSTED_SRAM
else ifeq (${FVP_SHARED_DATA_LOCATION}, tdram)
  FVP_SHARED_DATA_LOCATION_ID := FVP_IN_TRUSTED_DRAM
else
  $(error "Unsupported FVP_SHARED_DATA_LOCATION value")
endif

# On FVP, the TSP can execute either from Trusted SRAM or Trusted DRAM.
# Trusted SRAM is the default.
FVP_TSP_RAM_LOCATION	:=	tsram
ifeq (${FVP_TSP_RAM_LOCATION}, tsram)
  FVP_TSP_RAM_LOCATION_ID := FVP_IN_TRUSTED_SRAM
else ifeq (${FVP_TSP_RAM_LOCATION}, tdram)
  FVP_TSP_RAM_LOCATION_ID := FVP_IN_TRUSTED_DRAM
else
  $(error "Unsupported FVP_TSP_RAM_LOCATION value")
endif

ifeq (${FVP_SHARED_DATA_LOCATION}, tsram)
  ifeq (${FVP_TSP_RAM_LOCATION}, tdram)
    $(error Shared data in Trusted SRAM and TSP in Trusted DRAM is not supported)
  endif
endif

# Process flags
$(eval $(call add_define,FVP_SHARED_DATA_LOCATION_ID))
$(eval $(call add_define,FVP_TSP_RAM_LOCATION_ID))

PLAT_INCLUDES		:=	-Iplat/fvp/include/

PLAT_BL_COMMON_SOURCES	:=	drivers/arm/pl011/pl011_console.S		\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_semihosting.c			\
				drivers/io/io_storage.c				\
				lib/aarch64/xlat_tables.c			\
				lib/semihosting/semihosting.c			\
				lib/semihosting/aarch64/semihosting_call.S	\
				plat/common/aarch64/plat_common.c		\
				plat/fvp/fvp_io_storage.c

BL1_SOURCES		+=	drivers/arm/cci400/cci400.c			\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a57.S			\
				plat/common/aarch64/platform_up_stack.S		\
				plat/fvp/bl1_fvp_setup.c			\
				plat/fvp/aarch64/fvp_common.c			\
				plat/fvp/aarch64/fvp_helpers.S

BL2_SOURCES		+=	drivers/arm/tzc400/tzc400.c			\
				plat/common/aarch64/platform_up_stack.S		\
				plat/fvp/bl2_fvp_setup.c			\
				plat/fvp/fvp_security.c				\
				plat/fvp/aarch64/fvp_common.c

BL31_SOURCES		+=	drivers/arm/cci400/cci400.c			\
				drivers/arm/gic/arm_gic.c			\
				drivers/arm/gic/gic_v2.c			\
				drivers/arm/gic/gic_v3.c			\
				drivers/arm/tzc400/tzc400.c			\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a57.S			\
				plat/common/plat_gic.c				\
				plat/common/aarch64/platform_mp_stack.S		\
				plat/fvp/bl31_fvp_setup.c			\
				plat/fvp/fvp_pm.c				\
				plat/fvp/fvp_security.c				\
				plat/fvp/fvp_topology.c				\
				plat/fvp/aarch64/fvp_helpers.S			\
				plat/fvp/aarch64/fvp_common.c			\
				plat/fvp/drivers/pwrc/fvp_pwrc.c

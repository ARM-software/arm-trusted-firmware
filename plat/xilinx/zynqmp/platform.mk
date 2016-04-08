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

ENABLE_PLAT_COMPAT := 0
PROGRAMMABLE_RESET_ADDRESS := 1
PSCI_EXTENDED_STATE_ID := 1
A53_DISABLE_NON_TEMPORAL_HINT := 0

ZYNQMP_ATF_LOCATION	?=	tsram
ifeq (${ZYNQMP_ATF_LOCATION}, tsram)
  ZYNQMP_ATF_LOCATION_ID := ZYNQMP_IN_TRUSTED_SRAM
else ifeq (${ZYNQMP_ATF_LOCATION}, tdram)
  ZYNQMP_ATF_LOCATION_ID := ZYNQMP_IN_TRUSTED_DRAM
else
  $(error "Unsupported ZYNQMP_ATF_LOCATION value")
endif

# On ZYNQMP, the TSP can execute either from Trusted SRAM or Trusted DRAM.
# Trusted SRAM is the default.
ZYNQMP_TSP_RAM_LOCATION	?=	tsram
ifeq (${ZYNQMP_TSP_RAM_LOCATION}, tsram)
  ZYNQMP_TSP_RAM_LOCATION_ID := ZYNQMP_IN_TRUSTED_SRAM
else ifeq (${ZYNQMP_TSP_RAM_LOCATION}, tdram)
  ZYNQMP_TSP_RAM_LOCATION_ID := ZYNQMP_IN_TRUSTED_DRAM
else
  $(error "Unsupported ZYNQMP_TSP_RAM_LOCATION value")
endif

# Process flags
$(eval $(call add_define,ZYNQMP_ATF_LOCATION_ID))
$(eval $(call add_define,ZYNQMP_TSP_RAM_LOCATION_ID))

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/			\
				-Iinclude/plat/arm/common/aarch64/		\
				-Iplat/xilinx/zynqmp/include/			\
				-Iplat/xilinx/zynqmp/pm_service/

PLAT_BL_COMMON_SOURCES	:=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v2/gicv2_main.c			\
				drivers/arm/gic/v2/gicv2_helpers.c		\
				drivers/cadence/uart/cdns_console.S		\
				drivers/console/console.S			\
				plat/arm/common/aarch64/arm_common.c		\
				plat/arm/common/aarch64/arm_helpers.S		\
				plat/arm/common/arm_cci.c			\
				plat/arm/common/arm_gicv2.c			\
				plat/common/plat_gicv2.c			\
				plat/common/aarch64/plat_common.c		\
				plat/xilinx/zynqmp/aarch64/zynqmp_helpers.S	\
				plat/xilinx/zynqmp/aarch64/zynqmp_common.c

BL31_SOURCES		+=	drivers/arm/cci/cci.c				\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				plat/common/aarch64/plat_psci_common.c		\
				plat/common/aarch64/platform_mp_stack.S		\
				plat/xilinx/zynqmp/bl31_zynqmp_setup.c		\
				plat/xilinx/zynqmp/plat_psci.c			\
				plat/xilinx/zynqmp/plat_zynqmp.c		\
				plat/xilinx/zynqmp/plat_topology.c		\
				plat/xilinx/zynqmp/sip_svc_setup.c		\
				plat/xilinx/zynqmp/pm_service/pm_svc_main.c	\
				plat/xilinx/zynqmp/pm_service/pm_api_sys.c	\
				plat/xilinx/zynqmp/pm_service/pm_ipi.c		\
				plat/xilinx/zynqmp/pm_service/pm_client.c

ifneq (${RESET_TO_BL31},1)
  $(error "Using BL31 as the reset vector is only one option supported on ZynqMP. Please set RESET_TO_BL31 to 1.")
endif

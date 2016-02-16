#
# Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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


# By default, SCP images are needed by CSS platforms.
CSS_LOAD_SCP_IMAGES	?=	1

PLAT_INCLUDES		+=	-Iinclude/plat/arm/css/common			\
				-Iinclude/plat/arm/css/common/aarch64


PLAT_BL_COMMON_SOURCES	+=	plat/arm/css/common/aarch64/css_helpers.S

BL1_SOURCES		+=	plat/arm/css/common/css_bl1_setup.c

BL2_SOURCES		+=	plat/arm/css/common/css_bl2_setup.c		\
				plat/arm/css/common/css_mhu.c			\
				plat/arm/css/common/css_scpi.c

BL2U_SOURCES		+=	plat/arm/css/common/css_bl2u_setup.c		\
				plat/arm/css/common/css_mhu.c			\
				plat/arm/css/common/css_scpi.c

BL31_SOURCES		+=	plat/arm/css/common/css_mhu.c			\
				plat/arm/css/common/css_pm.c			\
				plat/arm/css/common/css_scpi.c			\
				plat/arm/css/common/css_topology.c


ifneq (${RESET_TO_BL31},0)
  $(error "Using BL31 as the reset vector is not supported on CSS platforms. \
  Please set RESET_TO_BL31 to 0.")
endif

# Process CSS_LOAD_SCP_IMAGES flag
$(eval $(call assert_boolean,CSS_LOAD_SCP_IMAGES))
$(eval $(call add_define,CSS_LOAD_SCP_IMAGES))

ifeq (${CSS_LOAD_SCP_IMAGES},1)
  $(eval $(call FIP_ADD_IMG,SCP_BL2,--scp-fw))
  ifneq (${TRUSTED_BOARD_BOOT},0)
    $(eval $(call FWU_FIP_ADD_IMG,SCP_BL2U,--scp-fwu-cfg))
  endif

  BL2U_SOURCES		+=	plat/arm/css/common/css_scp_bootloader.c
  BL2_SOURCES		+=	plat/arm/css/common/css_scp_bootloader.c
endif

# Enable option to detect whether the SCP ROM firmware in use predates version
# 1.7.0 and therefore, is incompatible.
CSS_DETECT_PRE_1_7_0_SCP	:=	1

# Process CSS_DETECT_PRE_1_7_0_SCP flag
$(eval $(call assert_boolean,CSS_DETECT_PRE_1_7_0_SCP))
$(eval $(call add_define,CSS_DETECT_PRE_1_7_0_SCP))

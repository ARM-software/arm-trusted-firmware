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

PLAT_INCLUDES		+=	-Iinclude/plat/arm/css/common			\
				-Iinclude/plat/arm/css/common/aarch64


PLAT_BL_COMMON_SOURCES	+=	plat/arm/css/common/aarch64/css_helpers.S	\
				plat/arm/css/common/css_common.c

#BL1_SOURCES		+=

BL2_SOURCES		+=	plat/arm/css/common/css_bl2_setup.c		\
				plat/arm/css/common/css_mhu.c			\
				plat/arm/css/common/css_scp_bootloader.c	\
				plat/arm/css/common/css_scpi.c

BL31_SOURCES		+=	plat/arm/css/common/css_mhu.c			\
				plat/arm/css/common/css_pm.c			\
				plat/arm/css/common/css_scpi.c			\
				plat/arm/css/common/css_topology.c


ifneq (${RESET_TO_BL31},0)
  $(error "Using BL3-1 as the reset vector is not supported on CSS platforms. \
  Please set RESET_TO_BL31 to 0.")
endif

NEED_BL30		:=	yes

# Enable option to detect whether the SCP ROM firmware in use predates version
# 1.7.0 and therefore, is incompatible.
CSS_DETECT_PRE_1_7_0_SCP	:=	1

# Process CSS_DETECT_PRE_1_7_0_SCP flag
$(eval $(call assert_boolean,CSS_DETECT_PRE_1_7_0_SCP))
$(eval $(call add_define,CSS_DETECT_PRE_1_7_0_SCP))

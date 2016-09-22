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

PLAT_INCLUDES		+=	-Iinclude/plat/arm/board/common/			\
				-Iinclude/plat/arm/board/common/drivers

PLAT_BL_COMMON_SOURCES	+=	drivers/arm/pl011/${ARCH}/pl011_console.S		\
				plat/arm/board/common/${ARCH}/board_arm_helpers.S

BL1_SOURCES		+=	plat/arm/board/common/drivers/norflash/norflash.c

BL2_SOURCES		+=	plat/arm/board/common/drivers/norflash/norflash.c

#BL31_SOURCES		+=

ifneq (${TRUSTED_BOARD_BOOT},0)
    # ROTPK hash location
    ifeq (${ARM_ROTPK_LOCATION}, regs)
        ARM_ROTPK_LOCATION_ID = ARM_ROTPK_REGS_ID
    else ifeq (${ARM_ROTPK_LOCATION}, devel_rsa)
        ARM_ROTPK_LOCATION_ID = ARM_ROTPK_DEVEL_RSA_ID
    else
        $(error "Unsupported ARM_ROTPK_LOCATION value")
    endif
    $(eval $(call add_define,ARM_ROTPK_LOCATION_ID))

    # Certificate NV-Counters. Use values corresponding to tied off values in
    # ARM development platforms
    TFW_NVCTR_VAL	?=	31
    NTFW_NVCTR_VAL	?=	223

    BL1_SOURCES		+=	plat/arm/board/common/board_arm_trusted_boot.c
    BL2_SOURCES		+=	plat/arm/board/common/board_arm_trusted_boot.c
endif

# This flag controls whether memory usage needs to be optimised
ARM_BOARD_OPTIMISE_MEM	?=	0

# Process flags
$(eval $(call assert_boolean,ARM_BOARD_OPTIMISE_MEM))
$(eval $(call add_define,ARM_BOARD_OPTIMISE_MEM))

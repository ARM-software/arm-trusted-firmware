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

MTK_BOARD_COMMON	:=	plat/mediatek/board/common
MTK_BOARD_COMMON_DRIVER	:=	plat/mediatek/board/common/drivers

PLAT_INCLUDES		+=	-I${MTK_BOARD_COMMON}/drivers/include		\
				-I${MTK_BOARD_COMMON}/include

BL31_SOURCES		+=	${MTK_BOARD_COMMON}/board_common.c		\
				${MTK_BOARD_COMMON}/param_init.c		\
				${MTK_BOARD_COMMON_DRIVER}/ext_gpio/board_gpio.c

# Board drivers, e.g., external buck, external gpio, and system off handling
# code, are not built by default. You can specify the drivers platform code
# need by adding them to the ARM TF build command.
#
# MEM_CONFIG is another option that you can specify in the build command line.
# Different board/project may have differnt memory layout, we use MEM_CONFIG
# build option to support this flexibility. SRAM_192K is the default memory
# layout setting. If you don't specify MEM_CONFIG in build command line,
# SRAM_192K will be selected by default. The memory layout configs are kept in
# plat/mediatek/board/common/include/configs/.
#
# For example, to build bl31 binary for mt8173 with DA9212, MT6311, MT6391
# drivers, and memory layout is SRAM_192K, the build command arguments will be,
# PLAT=mt8173 MEM_CONFIG=SRAM_192K DA9212=1 MT6311=1 MT6391=1 DEBUG=1 bl31
#

# External buck driver
ifeq (${DA9212},1)
$(eval $(call add_define,DA9212))
BL31_SOURCES		+=	${MTK_BOARD_COMMON_DRIVER}/ext_buck/da9212.c
endif

ifeq (${MT6311},1)
$(eval $(call add_define,MT6311))
BL31_SOURCES		+=	${MTK_BOARD_COMMON_DRIVER}/ext_buck/mt6311.c
endif

# External gpio driver
ifeq (${MT6391},1)
$(eval $(call add_define,MT6391))
BL31_SOURCES		+=	${MTK_BOARD_COMMON_DRIVER}/ext_gpio/mt6391.c
endif

# Specify memory layout
ifneq (${MEM_CONFIG},)
$(eval $(call add_define,MEM_CONFIG))
endif

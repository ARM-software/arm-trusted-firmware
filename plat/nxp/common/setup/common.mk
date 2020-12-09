#
# Copyright 2018-2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

###############################################################################
# Flow begins in BL2 at EL3 mode
BL2_AT_EL3			:= 1

# Though one core is powered up by default, there are
# platform specific ways to release more than one core
COLD_BOOT_SINGLE_CPU		:= 0

PROGRAMMABLE_RESET_ADDRESS	:= 1

USE_COHERENT_MEM		:= 0

# Use generic OID definition (tbbr_oid.h)
USE_TBBR_DEFS			:= 1

PLAT_XLAT_TABLES_DYNAMIC	:= 0

ENABLE_SVE_FOR_NS		:= 0

ENABLE_STACK_PROTECTOR		:= 0

ERROR_DEPRECATED		:= 0

LS_DISABLE_TRUSTED_WDOG		:= 1

# On ARM platforms, separate the code and read-only data sections to allow
# mapping the former as executable and the latter as execute-never.
SEPARATE_CODE_AND_RODATA	:= 1

# Enable new version of image loading on ARM platforms
LOAD_IMAGE_V2			:= 1

RCW				:= ""

ifneq (${SPD},none)
$(eval $(call add_define, NXP_LOAD_BL32))
endif

###############################################################################

PLAT_TOOL_PATH		:=	tools/nxp
CREATE_PBL_TOOL_PATH	:=	${PLAT_TOOL_PATH}/create_pbl
PLAT_SETUP_PATH		:=	${PLAT_PATH}/common/setup

PLAT_INCLUDES		+=	-I${PLAT_SETUP_PATH}/include			\
				-Iinclude/plat/arm/common			\
				-Iinclude/drivers/arm   			\
				-Iinclude/lib					\
				-Iinclude/drivers/io			\
				-Ilib/psci

# Required without TBBR.
# To include the defines for DDR PHY Images.
PLAT_INCLUDES		+=	-Iinclude/common/tbbr

include ${PLAT_SETUP_PATH}/core.mk
PLAT_BL_COMMON_SOURCES	+= 	${CPU_LIBS} \
				plat/nxp/common/setup/ls_err.c		\
				plat/nxp/common/setup/ls_common.c

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES	+=	${PLAT_SETUP_PATH}/ls_stack_protector.c
endif

include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

BL2_SOURCES		+=	drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				drivers/io/io_storage.c			\
				common/desc_image_load.c 		\
				plat/nxp/common/setup/ls_image_load.c		\
				plat/nxp/common/setup/ls_io_storage.c		\
				plat/nxp/common/setup/ls_bl2_el3_setup.c	\
				plat/nxp/common/setup/${ARCH}/ls_bl2_mem_params_desc.c

BL31_SOURCES		+=	plat/nxp/common/setup/ls_bl31_setup.c	\

ifeq (${LS_EL3_INTERRUPT_HANDLER}, yes)
$(eval $(call add_define, LS_EL3_INTERRUPT_HANDLER))
BL31_SOURCES		+=	plat/nxp/common/setup/ls_interrupt_mgmt.c
endif

ifeq (${TEST_BL31}, 1)
BL31_SOURCES		+=	${TEST_SOURCES}
endif

# Verify build config
# -------------------

ifneq (${LOAD_IMAGE_V2}, 1)
  $(error Error: Layerscape needs LOAD_IMAGE_V2=1)
else
$(eval $(call add_define,LOAD_IMAGE_V2))
endif

include $(CREATE_PBL_TOOL_PATH)/create_pbl.mk

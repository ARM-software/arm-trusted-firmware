#
# Copyright 2018-2019 NXP
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

# do not enable SVE
ENABLE_SVE_FOR_NS		:= 0

MULTI_CONSOLE_API		:= 1

# On ARM platforms, separate the code and read-only data sections to allow
# mapping the former as executable and the latter as execute-never.
SEPARATE_CODE_AND_RODATA	:= 1

# do not use coherent memory
RCW				:= ""

ifneq (${SPD},none)
$(eval $(call add_define, LS_LOAD_BL32))
endif


###############################################################################

PLAT_DRIVERS_PATH	:=	drivers/nxp
PLAT_TOOL_PATH		:=	tools/nxp
PLAT_COMMON_PATH	:=	${PLAT_PATH}/common
PLAT_PSCI_PATH		:=	${PLAT_PATH}/psci
PLAT_SIPSVC_PATH	:=	${PLAT_PATH}/sip_svc

PLAT_INCLUDES		+=	-I${PLAT_COMMON_PATH}/include	\
				-I${PLAT_PSCI_PATH}/include	\
				-I${PLAT_SIPSVC_PATH}/include

include ${PLAT_COMMON_PATH}/core.mk
include ${PLAT_COMMON_PATH}/interconnect.mk
include ${PLAT_COMMON_PATH}/gic.mk
include ${PLAT_COMMON_PATH}/security.mk
include ${PLAT_COMMON_PATH}/console.mk
include ${PLAT_PSCI_PATH}/psci.mk
include ${PLAT_SIPSVC_PATH}/sipsvc.mk

ifeq (${TEST_BL31}, 1)
$(eval $(call add_define,TEST_BL31))
PLAT_TEST_PATH		:=	${PLAT_PATH}/test
include ${PLAT_TEST_PATH}/test.mk
endif

TIMER_SOURCES		:=	drivers/delay_timer/delay_timer.c	\
				plat/nxp/common/timer.c

PLAT_BL_COMMON_SOURCES	+=	${CONSOLE_SOURCES}

ifeq (${BOOT_MODE}, nor)
$(eval $(call add_define,NOR_BOOT))
BOOT_DEV_SOURCES		=	${PLAT_DRIVERS_PATH}/ifc/nor/ifc_nor.c
else ifeq (${BOOT_MODE}, nand)
$(eval $(call add_define,NAND_BOOT))
BOOT_DEV_SOURCES		=	${PLAT_DRIVERS_PATH}/ifc/nand/nand.c	\
					drivers/io/io_block.c
else ifeq (${BOOT_MODE}, qspi)
$(eval $(call add_define,QSPI_BOOT))
BOOT_DEV_SOURCES		=	${PLAT_DRIVERS_PATH}/qspi/qspi.c
else ifeq (${BOOT_MODE}, sd)
$(eval $(call add_define,SD_BOOT))
BOOT_DEV_SOURCES		=	${PLAT_DRIVERS_PATH}/sd/sd_mmc.c	\
					drivers/io/io_block.c
else ifeq (${BOOT_MODE}, emmc)
$(eval $(call add_define,SD_BOOT))
BOOT_DEV_SOURCES		=	${PLAT_DRIVERS_PATH}/sd/sd_mmc.c	\
					drivers/io/io_block.c
else ifeq (${BOOT_MODE}, flexspi_nor)
$(eval $(call add_define,FLEXSPI_NOR_BOOT))
BOOT_DEV_SOURCES		= ${PLAT_DRIVERS_PATH}/flexspi/nor/flexspi_nor.c
endif

ifeq ($(DDR_DRIVER),yes)
$(eval $(call add_define, DDR_INIT))
endif

ifeq ($(DDR_DRIVER),yes)
# define DDR_CNTRL_SOURCES
include $(PLAT_DRIVERS_PATH)/ddr/nxp-ddr/ddr.mk
PLAT_INCLUDES		+= -I$(PLAT_DRIVERS_PATH)/ddr/nxp-ddr
endif

ifeq ($(I2C_DRIVER),yes)
$(eval $(call add_define, I2C_INIT))
include $(PLAT_DRIVERS_PATH)/i2c/i2c.mk
endif

###############################################################################

PLAT_BL_COMMON_SOURCES	+= 	${CPU_LIBS}				\
				${INTERCONNECT_SOURCES}			\
				${TIMER_SOURCES}			\
				${SECURITY_SOURCES}

PLAT_BL_COMMON_SOURCES	+=	plat/nxp/common/${ARCH}/nxp_helpers.S	\
				plat/nxp/common/error_handler.c

include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

BL2_SOURCES		+=	drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				drivers/io/io_storage.c			\
				common/desc_image_load.c 		\
				${BOOT_DEV_SOURCES}			\
				${DDR_CNTLR_SOURCES}			\
				${PHY_SOURCES}				\
				plat/nxp/common/image_load.c		\
				plat/nxp/common/nxp_io_storage.c		\
				plat/nxp/common/bl2_el3_setup.c	\
				plat/nxp/common/${ARCH}/bl2_mem_params_desc.c

BL31_SOURCES	+=	plat/nxp/common/bl31_setup.c	\
				${GIC_SOURCES}	\
				${PSCI_SOURCES}	\
				${SIPSVC_SOURCES}


ifeq (${TEST_BL31}, 1)
BL31_SOURCES	+=	${TEST_SOURCES}
endif

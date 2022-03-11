#
# Copyright (c) 2015 - 2021, Broadcom
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_BL_COMMON_SOURCES	+=	plat/brcm/board/common/board_common.c

# If no board config makefile, do not include it
ifneq (${BOARD_CFG},)
BOARD_CFG_MAKE := $(shell find plat/brcm/board/${PLAT} -name '${BOARD_CFG}.mk')
$(eval $(call add_define,BOARD_CFG))
ifneq (${BOARD_CFG_MAKE},)
$(info Including ${BOARD_CFG_MAKE})
include ${BOARD_CFG_MAKE}
else
$(error Error: File ${BOARD_CFG}.mk not found in plat/brcm/board/${PLAT})
endif
endif

# To compile with highest log level (VERBOSE) set value to 50
LOG_LEVEL := 40

# Use custom generic timer clock
ifneq (${GENTIMER_ACTUAL_CLOCK},)
$(info Using GENTIMER_ACTUAL_CLOCK=$(GENTIMER_ACTUAL_CLOCK))
SYSCNT_FREQ := $(GENTIMER_ACTUAL_CLOCK)
$(eval $(call add_define,SYSCNT_FREQ))
endif

ifeq (${DRIVER_EMMC_ENABLE},)
DRIVER_EMMC_ENABLE :=1
endif

ifeq (${DRIVER_SPI_ENABLE},)
DRIVER_SPI_ENABLE := 0
endif

ifeq (${DRIVER_I2C_ENABLE},)
DRIVER_I2C_ENABLE := 0
endif

# By default, Trusted Watchdog is always enabled unless SPIN_ON_BL1_EXIT is set
ifeq (${BRCM_DISABLE_TRUSTED_WDOG},)
BRCM_DISABLE_TRUSTED_WDOG	:=	0
endif
ifeq (${SPIN_ON_BL1_EXIT}, 1)
BRCM_DISABLE_TRUSTED_WDOG	:=	1
endif

$(eval $(call assert_boolean,BRCM_DISABLE_TRUSTED_WDOG))
$(eval $(call add_define,BRCM_DISABLE_TRUSTED_WDOG))

# Process ARM_BL31_IN_DRAM flag
ifeq (${ARM_BL31_IN_DRAM},)
ARM_BL31_IN_DRAM		:=	0
endif
$(eval $(call assert_boolean,ARM_BL31_IN_DRAM))
$(eval $(call add_define,ARM_BL31_IN_DRAM))

ifeq (${STANDALONE_BL2},yes)
BL2_LOG_LEVEL := 40
$(eval $(call add_define,MMU_DISABLED))
endif

# BL2 XIP from QSPI
RUN_BL2_FROM_QSPI := 0
ifeq (${RUN_BL2_FROM_QSPI},1)
$(eval $(call add_define,RUN_BL2_FROM_QSPI))
endif

# BL2 XIP from NAND
RUN_BL2_FROM_NAND := 0
ifeq (${RUN_BL2_FROM_NAND},1)
$(eval $(call add_define,RUN_BL2_FROM_NAND))
endif

ifneq (${ELOG_AP_UART_LOG_BASE},)
$(eval $(call add_define,ELOG_AP_UART_LOG_BASE))
endif

ifeq (${ELOG_SUPPORT},1)
ifeq (${ELOG_STORE_MEDIA},DDR)
$(eval $(call add_define,ELOG_STORE_MEDIA_DDR))
ifneq (${ELOG_STORE_OFFSET},)
$(eval $(call add_define,ELOG_STORE_OFFSET))
endif
endif
endif

ifneq (${BL2_LOG_LEVEL},)
$(eval $(call add_define,BL2_LOG_LEVEL))
endif

ifneq (${BL31_LOG_LEVEL},)
$(eval $(call add_define,BL31_LOG_LEVEL))
endif

# Use CRMU SRAM from iHOST
ifneq (${USE_CRMU_SRAM},)
$(eval $(call add_define,USE_CRMU_SRAM))
endif

# Use PIO mode if DDR is not used
ifeq (${USE_DDR},yes)
EMMC_USE_DMA	:=	1
else
EMMC_USE_DMA	:=	0
endif
$(eval $(call add_define,EMMC_USE_DMA))

# On BRCM platforms, separate the code and read-only data sections to allow
# mapping the former as executable and the latter as execute-never.
SEPARATE_CODE_AND_RODATA	:=	1

# Use generic OID definition (tbbr_oid.h)
USE_TBBR_DEFS			:=	1

PLAT_INCLUDES		+=	-Iplat/brcm/board/common \
				-Iinclude/drivers/brcm \
				-Iinclude/drivers/brcm/emmc \
				-Iinclude/drivers/brcm/mdio

PLAT_BL_COMMON_SOURCES	+=	plat/brcm/common/brcm_common.c \
				plat/brcm/board/common/cmn_sec.c \
				plat/brcm/board/common/bcm_console.c \
				plat/brcm/board/common/brcm_mbedtls.c \
				plat/brcm/board/common/plat_setup.c \
				plat/brcm/board/common/platform_common.c \
				drivers/arm/sp804/sp804_delay_timer.c \
				drivers/brcm/sotp.c \
				drivers/delay_timer/delay_timer.c \
				drivers/io/io_fip.c \
				drivers/io/io_memmap.c \
				drivers/io/io_storage.c \
				plat/brcm/common/brcm_io_storage.c \
				plat/brcm/board/common/err.c \
				plat/brcm/board/common/sbl_util.c \
				drivers/arm/sp805/sp805.c

# Add RNG driver
DRIVER_RNG_ENABLE := 1
ifeq (${DRIVER_RNG_ENABLE},1)
PLAT_BL_COMMON_SOURCES	+=	drivers/brcm/rng.c
endif

# Add eMMC driver
ifeq (${DRIVER_EMMC_ENABLE},1)
$(eval $(call add_define,DRIVER_EMMC_ENABLE))

EMMC_SOURCES		+=	drivers/brcm/emmc/emmc_chal_sd.c \
				drivers/brcm/emmc/emmc_csl_sdcard.c \
				drivers/brcm/emmc/emmc_csl_sdcmd.c \
				drivers/brcm/emmc/emmc_pboot_hal_memory_drv.c

PLAT_BL_COMMON_SOURCES += ${EMMC_SOURCES}

ifeq (${DRIVER_EMMC_ENABLE_DATA_WIDTH_8BIT},)
$(eval $(call add_define,DRIVER_EMMC_ENABLE_DATA_WIDTH_8BIT))
endif
endif

BL2_SOURCES		+=	plat/brcm/common/brcm_bl2_mem_params_desc.c \
				plat/brcm/common/brcm_image_load.c \
				common/desc_image_load.c

BL2_SOURCES		+= 	plat/brcm/common/brcm_bl2_setup.c

BL31_SOURCES		+=	plat/brcm/common/brcm_bl31_setup.c

ifeq (${BCM_ELOG},yes)
ELOG_SOURCES		+= 	plat/brcm/board/common/bcm_elog.c
BL2_SOURCES		+= 	${ELOG_SOURCES}
BL31_SOURCES		+= 	${ELOG_SOURCES}
endif

# Add spi driver
ifeq (${DRIVER_SPI_ENABLE},1)
PLAT_BL_COMMON_SOURCES	+=	drivers/brcm/spi/iproc_spi.c \
				drivers/brcm/spi/iproc_qspi.c
endif

# Add spi nor/flash driver
ifeq (${DRIVER_SPI_NOR_ENABLE},1)
PLAT_BL_COMMON_SOURCES	+=	drivers/brcm/spi_sf.c \
				drivers/brcm/spi_flash.c
endif

ifeq (${DRIVER_I2C_ENABLE},1)
$(eval $(call add_define,DRIVER_I2C_ENABLE))
BL2_SOURCES		+= 	drivers/brcm/i2c/i2c.c
PLAT_INCLUDES		+=	-Iinclude/drivers/brcm/i2c
endif

ifeq (${DRIVER_OCOTP_ENABLE},1)
$(eval $(call add_define,DRIVER_OCOTP_ENABLE))
BL2_SOURCES		+= drivers/brcm/ocotp.c
endif

# Enable FRU table support
ifeq (${USE_FRU},yes)
$(eval $(call add_define,USE_FRU))
BL2_SOURCES		+= drivers/brcm/fru.c
endif

# Enable GPIO support
ifeq (${USE_GPIO},yes)
$(eval $(call add_define,USE_GPIO))
BL2_SOURCES             += drivers/gpio/gpio.c
BL2_SOURCES             += drivers/brcm/iproc_gpio.c
ifeq (${GPIO_SUPPORT_FLOAT_DETECTION},yes)
$(eval $(call add_define,GPIO_SUPPORT_FLOAT_DETECTION))
endif
endif

# Include mbedtls if it can be located
MBEDTLS_DIR ?= mbedtls
MBEDTLS_CHECK := $(shell find ${MBEDTLS_DIR}/include -name '$(notdir ${MBEDTLS_DIR})')

ifneq (${MBEDTLS_CHECK},)
$(info Found mbedTLS at ${MBEDTLS_DIR})
PLAT_INCLUDES += -I${MBEDTLS_DIR}/include/mbedtls

# By default, use RSA keys
KEY_ALG := rsa_1_5

# Include common TBB sources
AUTH_SOURCES	+= 	drivers/auth/auth_mod.c \
			drivers/auth/crypto_mod.c \
			drivers/auth/img_parser_mod.c \
			drivers/auth/tbbr/tbbr_cot_common.c \
			drivers/auth/tbbr/tbbr_cot_bl2.c

BL2_SOURCES	+=	${AUTH_SOURCES}

# Use ATF framework for MBEDTLS
TRUSTED_BOARD_BOOT := 1
CRYPTO_LIB_MK := drivers/auth/mbedtls/mbedtls_crypto.mk
IMG_PARSER_LIB_MK := drivers/auth/mbedtls/mbedtls_x509.mk
$(info Including ${CRYPTO_LIB_MK})
include ${CRYPTO_LIB_MK}
$(info Including ${IMG_PARSER_LIB_MK})
include ${IMG_PARSER_LIB_MK}

# Use ATF secure boot functions
# Use Hardcoded hash for devel

ARM_ROTPK_LOCATION=arm_rsa
ifeq (${ARM_ROTPK_LOCATION}, arm_rsa)
ARM_ROTPK_LOCATION_ID=ARM_ROTPK_DEVEL_RSA_ID
ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem
else ifeq (${ARM_ROTPK_LOCATION}, brcm_rsa)
ARM_ROTPK_LOCATION_ID=BRCM_ROTPK_SOTP_RSA_ID
ifeq (${ROT_KEY},)
ROT_KEY=plat/brcm/board/common/rotpk/rsa_dauth2048_key.pem
endif
KEY_FIND := $(shell m="${ROT_KEY}"; [ -f "$$m" ] && echo "$$m")
ifeq (${KEY_FIND},)
$(error Error: No ${ROT_KEY} located)
else
$(info Using ROT_KEY: ${ROT_KEY})
endif
else
$(error "Unsupported ARM_ROTPK_LOCATION value")
endif

$(eval $(call add_define,ARM_ROTPK_LOCATION_ID))
PLAT_BL_COMMON_SOURCES+=plat/brcm/board/common/board_arm_trusted_boot.c
endif

#M0 runtime firmware
ifdef SCP_BL2
$(eval $(call add_define,NEED_SCP_BL2))
SCP_CFG_DIR=$(dir ${SCP_BL2})
PLAT_INCLUDES += -I${SCP_CFG_DIR}
endif

ifneq (${NEED_BL33},yes)
# If there is no BL33, BL31 will jump to this address.
ifeq (${USE_DDR},yes)
PRELOADED_BL33_BASE := 0x80000000
else
PRELOADED_BL33_BASE := 0x74000000
endif
endif

# Use translation tables library v1 by default
ARM_XLAT_TABLES_LIB_V1		:=	1
ifeq (${ARM_XLAT_TABLES_LIB_V1}, 1)
$(eval $(call assert_boolean,ARM_XLAT_TABLES_LIB_V1))
$(eval $(call add_define,ARM_XLAT_TABLES_LIB_V1))
PLAT_BL_COMMON_SOURCES	+=	lib/xlat_tables/aarch64/xlat_tables.c \
				lib/xlat_tables/xlat_tables_common.c
endif

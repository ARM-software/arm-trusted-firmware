#
# Copyright (C) 2016 - 2020 Marvell International Ltd.
#
# SPDX-License-Identifier:     BSD-3-Clause
# https://spdx.org/licenses

PLAT_FAMILY		:= a8k
PLAT_INCLUDE_BASE	:= include/plat/marvell/armada/$(PLAT_FAMILY)
PLAT_COMMON_BASE	:= plat/marvell/armada/a8k/common
MARVELL_DRV_BASE	:= drivers/marvell
MARVELL_COMMON_BASE	:= plat/marvell/armada/common

MARVELL_SVC_TEST	:= 0
$(eval $(call add_define,MARVELL_SVC_TEST))

ERRATA_A72_859971	:= 1

# Enable MSS support for a8k family
MSS_SUPPORT		:= 1
$(eval $(call add_define,MSS_SUPPORT))

# Disable EL3 cache for power management
BL31_CACHE_DISABLE	:= 0
$(eval $(call add_define,BL31_CACHE_DISABLE))

$(eval $(call add_define,PCI_EP_SUPPORT))
$(eval $(call assert_boolean,PCI_EP_SUPPORT))

AP_NUM			:= 1
$(eval $(call add_define,AP_NUM))

DOIMAGEPATH		?=	tools/marvell/doimage
DOIMAGETOOL		?=	${DOIMAGEPATH}/doimage

include plat/marvell/marvell.mk
include tools/marvell/doimage/doimage.mk

ifeq (${MARVELL_SECURE_BOOT},1)
DOIMAGE_SEC_FLAGS := -c $(DOIMAGE_SEC)
DOIMAGE_LIBS_CHECK = \
        if ! [ -d "/usr/include/mbedtls" ]; then \
                        echo "****************************************" >&2; \
                        echo "Missing mbedTLS installation! " >&2; \
                        echo "Please download it from \"tls.mbed.org\"" >&2; \
			echo "Alternatively on Debian/Ubuntu system install" >&2; \
			echo "\"libmbedtls-dev\" package" >&2; \
                        echo "Make sure to use version 2.1.0 or later" >&2; \
                        echo "****************************************" >&2; \
                exit 1; \
        else if ! [ -f "/usr/include/libconfig.h" ]; then \
                        echo "********************************************************" >&2; \
                        echo "Missing Libconfig installation!" >&2; \
                        echo "Please download it from \"www.hyperrealm.com/libconfig/\"" >&2; \
                        echo "Alternatively on Debian/Ubuntu system install packages" >&2; \
                        echo "\"libconfig8\" and \"libconfig8-dev\"" >&2; \
                        echo "********************************************************" >&2; \
                exit 1; \
        fi \
        fi
else #MARVELL_SECURE_BOOT
DOIMAGE_LIBS_CHECK =
DOIMAGE_SEC_FLAGS =
endif #MARVELL_SECURE_BOOT

ROM_BIN_EXT ?= $(BUILD_PLAT)/ble.bin
DOIMAGE_FLAGS	+= -b $(ROM_BIN_EXT) $(NAND_DOIMAGE_FLAGS) $(DOIMAGE_SEC_FLAGS)

# Check whether to build system_power.c for the platform
ifneq ("$(wildcard $(BOARD_DIR)/board/system_power.c)","")
SYSTEM_POWER_SUPPORT = 1
else
SYSTEM_POWER_SUPPORT = 0
endif

# This define specifies DDR type for BLE
$(eval $(call add_define,CONFIG_DDR4))

# This define specifies DDR topology for BLE
DDR_TOPOLOGY	?= 0
$(eval $(call add_define,DDR_TOPOLOGY))

MARVELL_GIC_SOURCES	:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c

PLAT_INCLUDES		+=	-I$(BOARD_DIR)				\
				-I$(BOARD_DIR)/board			\
				-I$(CURDIR)/drivers/marvell		\
				-I$(PLAT_COMMON_BASE)/include		\
				-I$(PLAT_INCLUDE_BASE)/common

PLAT_BL_COMMON_SOURCES	:=	$(PLAT_COMMON_BASE)/aarch64/a8k_common.c \
				drivers/ti/uart/aarch64/16550_console.S

ifndef BLE_PORTING_SOURCES
BLE_PORTING_SOURCES	:=	$(BOARD_DIR)/board/dram_port.c \
				$(BOARD_DIR)/board/marvell_plat_config.c
endif

MARVELL_MOCHI_DRV	+=	$(MARVELL_DRV_BASE)/mochi/cp110_setup.c

BLE_SOURCES		:=	drivers/mentor/i2c/mi2cv.c		\
				$(PLAT_COMMON_BASE)/plat_ble_setup.c	\
				$(MARVELL_MOCHI_DRV)			\
				$(PLAT_COMMON_BASE)/plat_pm.c		\
				$(MARVELL_DRV_BASE)/ap807_clocks_init.c	\
				$(MARVELL_DRV_BASE)/thermal.c		\
				$(PLAT_COMMON_BASE)/plat_thermal.c	\
				$(BLE_PORTING_SOURCES)			\
				$(MARVELL_DRV_BASE)/ccu.c		\
				$(MARVELL_DRV_BASE)/io_win.c

BL1_SOURCES		+=	$(PLAT_COMMON_BASE)/aarch64/plat_helpers.S \
				lib/cpus/aarch64/cortex_a72.S

MARVELL_DRV		:= 	$(MARVELL_DRV_BASE)/io_win.c	\
				$(MARVELL_DRV_BASE)/iob.c	\
				$(MARVELL_DRV_BASE)/mci.c	\
				$(MARVELL_DRV_BASE)/amb_adec.c	\
				$(MARVELL_DRV_BASE)/ccu.c	\
				$(MARVELL_DRV_BASE)/cache_llc.c	\
				$(MARVELL_DRV_BASE)/comphy/phy-comphy-cp110.c	\
				$(MARVELL_DRV_BASE)/mc_trustzone/mc_trustzone.c	\
				$(MARVELL_DRV_BASE)/secure_dfx_access/armada_thermal.c	\
				$(MARVELL_DRV_BASE)/secure_dfx_access/misc_dfx.c	\
				$(MARVELL_DRV_BASE)/ddr_phy_access.c	\
				drivers/rambus/trng_ip_76.c

ifeq (${MSS_SUPPORT}, 1)
MARVELL_DRV		+=	$(MARVELL_DRV_BASE)/mg_conf_cm3/mg_conf_cm3.c
endif

ifndef BL31_PORTING_SOURCES
BL31_PORTING_SOURCES	:=	$(BOARD_DIR)/board/marvell_plat_config.c
endif

ifeq ($(SYSTEM_POWER_SUPPORT),1)
BL31_PORTING_SOURCES	+=	$(BOARD_DIR)/board/system_power.c
endif

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a72.S		       \
				$(PLAT_COMMON_BASE)/aarch64/plat_helpers.S     \
				$(PLAT_COMMON_BASE)/aarch64/plat_arch_config.c \
				$(PLAT_COMMON_BASE)/plat_pm.c		       \
				$(PLAT_COMMON_BASE)/plat_bl31_setup.c	       \
				$(MARVELL_COMMON_BASE)/marvell_gicv2.c	       \
				$(MARVELL_COMMON_BASE)/mrvl_sip_svc.c	       \
				$(MARVELL_COMMON_BASE)/marvell_ddr_info.c      \
				$(BL31_PORTING_SOURCES)			       \
				$(MARVELL_DRV)				       \
				$(MARVELL_MOCHI_DRV)			       \
				$(MARVELL_GIC_SOURCES)

# Add trace functionality for PM
BL31_SOURCES		+=	$(PLAT_COMMON_BASE)/plat_pm_trace.c


ifeq (${MSS_SUPPORT}, 1)
# Force builds with BL2 image on a80x0 platforms
ifndef SCP_BL2
 $(error "Error: SCP_BL2 image is mandatory for a8k family")
endif

# MSS (SCP) build
include $(PLAT_COMMON_BASE)/mss/mss_a8k.mk
endif

# BLE (ROM context execution code, AKA binary extension)
BLE_PATH	?=  $(PLAT_COMMON_BASE)/ble

include ${BLE_PATH}/ble.mk
$(eval $(call MAKE_BL,ble))

clean realclean distclean: mrvl_clean

.PHONY: mrvl_clean
mrvl_clean:
	$(s)echo "  Doimage CLEAN"
	$(q)${MAKE} PLAT=${PLAT} --no-print-directory -C ${DOIMAGEPATH} clean

${DOIMAGETOOL}: FORCE
	$(q)$(DOIMAGE_LIBS_CHECK)
	$(q)${MAKE} --no-print-directory -C ${DOIMAGEPATH}

${BUILD_PLAT}/${FLASH_IMAGE}: ${ROM_BIN_EXT} ${BUILD_PLAT}/${BOOT_IMAGE} ${DOIMAGETOOL}
	$(s)echo
	$(s)echo "Building flash image"
	$(q)${DOIMAGETOOL} ${DOIMAGE_FLAGS} ${BUILD_PLAT}/${BOOT_IMAGE} ${BUILD_PLAT}/${FLASH_IMAGE}
	$(s)echo
	$(s)echo "Built $@ successfully"
	$(s)echo

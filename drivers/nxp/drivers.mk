#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

###############################################################################


PLAT_DRIVERS_PATH		:=	drivers/nxp
PLAT_DRIVERS_INCLUDE_PATH	:=	include/drivers/nxp

ifeq (${SMMU_NEEDED},yes)
PLAT_INCLUDES	+= -Iinclude/drivers/nxp/smmu/
endif

ifeq (${DCFG_NEEDED},yes)
include $(PLAT_DRIVERS_PATH)/dcfg/dcfg.mk
endif

ifeq (${CSU_NEEDED},yes)
include $(PLAT_DRIVERS_PATH)/csu/csu.mk
endif

ifeq (${TIMER_NEEDED},yes)
include $(PLAT_DRIVERS_PATH)/timer/timer.mk
endif

ifeq (${INTERCONNECT_NEEDED},yes)
include ${PLAT_DRIVERS_PATH}/interconnect/interconnect.mk
endif

ifeq (${GIC_NEEDED},yes)
include ${PLAT_DRIVERS_PATH}/gic/gic.mk
endif

ifeq (${SD_MMC_NEEDED},yes)
include $(PLAT_DRIVERS_PATH)/sd/sd_mmc.mk
endif

ifeq (${CONSOLE_NEEDED},yes)
include $(PLAT_DRIVERS_PATH)/console/console.mk
endif

ifeq (${SFP_NEEDED},yes)
include $(PLAT_DRIVERS_PATH)/sfp/sfp.mk
endif

ifeq (${XSPI_NEEDED},yes)
include $(PLAT_DRIVERS_PATH)/flexspi/nor/flexspi_nor.mk
endif

ifeq (${QSPI_NEEDED},yes)
include $(PLAT_DRIVERS_PATH)/qspi/qspi.mk
endif

ifeq (${SNVS_NEEDED},yes)
include $(PLAT_DRIVERS_PATH)/sec_mon/sec_mon.mk
endif

ifeq ($(I2C_NEEDED),yes)
$(eval $(call add_define, I2C_INIT))
include $(PLAT_DRIVERS_PATH)/i2c/i2c.mk
endif

ifeq ($(DDR_DRIVER_NEEDED),yes)
$(eval $(call add_define, DDR_INIT))
# define DDR_CNTRL_SOURCES
ifeq ($(DDRCNTLR),MMDC)
include $(PLAT_DRIVERS_PATH)/ddr/fsl-mmdc/ddr.mk
else
include $(PLAT_DRIVERS_PATH)/ddr/nxp-ddr/ddr.mk
endif # DDR_CNTRL_SOURCES
endif

ifeq (${PMU_NEEDED},yes)
include $(PLAT_DRIVERS_PATH)/pmu/pmu.mk
endif

ifeq (${CRYPTO_NEEDED},yes)
include $(PLAT_DRIVERS_PATH)/crypto/caam/caam.mk
endif

ifeq (${TZASC_NEEDED},yes)
include $(PLAT_DRIVERS_PATH)/tzc/tzc.mk
endif

ifeq (${GPIO_NEEDED},yes)
include ${PLAT_DRIVERS_PATH}/gpio/gpio.mk
endif

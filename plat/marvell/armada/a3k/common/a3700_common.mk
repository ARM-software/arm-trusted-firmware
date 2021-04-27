#
# Copyright (C) 2018-2021 Marvell International Ltd.
#
# SPDX-License-Identifier:	BSD-3-Clause
# https://spdx.org/licenses
#

MARVELL_PLAT_BASE		:= plat/marvell/armada
MARVELL_PLAT_INCLUDE_BASE	:= include/plat/marvell/armada
PLAT_FAMILY			:= a3k
PLAT_FAMILY_BASE		:= $(MARVELL_PLAT_BASE)/$(PLAT_FAMILY)
PLAT_INCLUDE_BASE		:= $(MARVELL_PLAT_INCLUDE_BASE)/$(PLAT_FAMILY)
PLAT_COMMON_BASE		:= $(PLAT_FAMILY_BASE)/common
MARVELL_DRV_BASE		:= drivers/marvell
MARVELL_COMMON_BASE		:= $(MARVELL_PLAT_BASE)/common
HANDLE_EA_EL3_FIRST		:= 1

include plat/marvell/marvell.mk

#*********** A3700 *************

# GICV3
$(eval $(call add_define,CONFIG_GICV3))

# CCI-400
$(eval $(call add_define,USE_CCI))

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

MARVELL_GIC_SOURCES	:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c

PLAT_INCLUDES		:=	-I$(PLAT_FAMILY_BASE)/$(PLAT)		\
				-I$(PLAT_COMMON_BASE)/include		\
				-I$(PLAT_INCLUDE_BASE)/common		\
				-I$(MARVELL_DRV_BASE)			\
				-I$/drivers/arm/gic/common/

PLAT_BL_COMMON_SOURCES	:=	$(PLAT_COMMON_BASE)/aarch64/a3700_common.c \
				$(MARVELL_DRV_BASE)/uart/a3700_console.S

BL1_SOURCES		+=	$(PLAT_COMMON_BASE)/aarch64/plat_helpers.S \
				lib/cpus/aarch64/cortex_a53.S

MARVELL_DRV		:=	$(MARVELL_DRV_BASE)/comphy/phy-comphy-3700.c

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				$(PLAT_COMMON_BASE)/aarch64/plat_helpers.S \
				$(PLAT_COMMON_BASE)/plat_cci.c		\
				$(PLAT_COMMON_BASE)/plat_pm.c		\
				$(PLAT_COMMON_BASE)/dram_win.c		\
				$(PLAT_COMMON_BASE)/io_addr_dec.c	\
				$(PLAT_COMMON_BASE)/marvell_plat_config.c     \
				$(PLAT_COMMON_BASE)/a3700_ea.c		\
				$(PLAT_FAMILY_BASE)/$(PLAT)/plat_bl31_setup.c \
				$(MARVELL_COMMON_BASE)/marvell_cci.c	\
				$(MARVELL_COMMON_BASE)/marvell_ddr_info.c	\
				$(MARVELL_COMMON_BASE)/marvell_gicv3.c	\
				$(MARVELL_GIC_SOURCES)			\
				drivers/arm/cci/cci.c			\
				$(PLAT_COMMON_BASE)/a3700_sip_svc.c	\
				$(MARVELL_DRV)

ifeq ($(CM3_SYSTEM_RESET),1)
BL31_SOURCES		+=	$(PLAT_COMMON_BASE)/cm3_system_reset.c
endif

ifeq ($(A3720_DB_PM_WAKEUP_SRC),1)
BL31_SOURCES		+=	$(PLAT_FAMILY_BASE)/$(PLAT)/board/pm_src.c
endif

ifdef WTP

$(if $(wildcard $(value WTP)/*),,$(error "'WTP=$(value WTP)' was specified, but '$(value WTP)' directory does not exist"))
$(if $(shell test -s "$(value WTP)/branch.txt" || git -C $(value WTP) rev-parse --show-cdup 2>&1),$(error "'WTP=$(value WTP)' was specified, but '$(value WTP)' does not contain valid Marvell a3700_utils release tarball nor git repository"))

DOIMAGEPATH	:= $(WTP)
DOIMAGETOOL	:= $(DOIMAGEPATH)/wtptp/src/TBB_Linux/release/TBB_linux

BUILD_UART	:= uart-images
UART_IMAGE	:= $(BUILD_UART).tgz.bin

ifeq ($(MARVELL_SECURE_BOOT),1)
DOIMAGE_CFG	:= $(BUILD_PLAT)/atf-tim.txt
DOIMAGEUART_CFG	:= $(BUILD_PLAT)/$(BUILD_UART)/atf-tim.txt
IMAGESPATH	:= $(DOIMAGEPATH)/tim/trusted
TIMNCFG		:= $(BUILD_PLAT)/atf-timN.txt
TIMNUARTCFG	:= $(BUILD_PLAT)/$(BUILD_UART)/atf-timN.txt
TIMNSIG		:= $(IMAGESPATH)/timnsign.txt
TIM2IMGARGS	:= -i $(DOIMAGE_CFG) -n $(TIMNCFG)
TIMN_IMAGE	:= $$(grep "Image Filename:" -m 1 $(TIMNCFG) | cut -c 17-)
else #MARVELL_SECURE_BOOT
DOIMAGE_CFG	:= $(BUILD_PLAT)/atf-ntim.txt
DOIMAGEUART_CFG	:= $(BUILD_PLAT)/$(BUILD_UART)/atf-ntim.txt
IMAGESPATH	:= $(DOIMAGEPATH)/tim/untrusted
TIM2IMGARGS	:= -i $(DOIMAGE_CFG)
endif #MARVELL_SECURE_BOOT

TIMBUILD	:= $(DOIMAGEPATH)/script/buildtim.sh
TIM2IMG		:= $(DOIMAGEPATH)/script/tim2img.pl
TIMDDRTOOL	:= $(DOIMAGEPATH)/tim/ddr/ddr_tool

$(TIMBUILD): $(TIMDDRTOOL)

# WTMI_IMG is used to specify the customized RTOS image running over
# Service CPU (CM3 processor). By the default, it points to a
# baremetal binary of fuse programming in A3700_utils.
WTMI_IMG	:= $(DOIMAGEPATH)/wtmi/fuse/build/fuse.bin

# WTMI_MULTI_IMG is composed of CM3 RTOS image (WTMI_IMG)
# and sys-init image.
WTMI_MULTI_IMG		:= $(DOIMAGEPATH)/wtmi/build/wtmi.bin

WTMI_ENC_IMG		:= wtmi-enc.bin

SRCPATH			:= $(dir $(BL33))

CLOCKSPRESET		?= CPU_800_DDR_800

DDR_TOPOLOGY		?= 0

BOOTDEV			?= SPINOR
PARTNUM			?= 0

TIM_IMAGE		:= $$(grep "Image Filename:" -m 1 $(DOIMAGE_CFG) | cut -c 17-)
TIMBLDARGS		:= $(MARVELL_SECURE_BOOT) $(BOOTDEV) $(IMAGESPATH) $(DOIMAGEPATH) $(CLOCKSPRESET) \
				$(DDR_TOPOLOGY) $(PARTNUM) $(DEBUG) $(DOIMAGE_CFG) $(TIMNCFG) $(TIMNSIG) 1
TIMBLDUARTARGS		:= $(MARVELL_SECURE_BOOT) UART $(IMAGESPATH) $(DOIMAGEPATH) $(CLOCKSPRESET) \
				$(DDR_TOPOLOGY) 0 0 $(DOIMAGEUART_CFG) $(TIMNUARTCFG) $(TIMNSIG) 0

CRYPTOPP_LIBDIR		?= $(CRYPTOPP_PATH)
CRYPTOPP_INCDIR		?= $(CRYPTOPP_PATH)

$(DOIMAGETOOL): FORCE
	$(if $(CRYPTOPP_LIBDIR),,$(error "Platform '$(PLAT)' for WTP image tool requires CRYPTOPP_PATH or CRYPTOPP_LIBDIR. Please set CRYPTOPP_PATH or CRYPTOPP_LIBDIR to point to the right directory"))
	$(if $(CRYPTOPP_INCDIR),,$(error "Platform '$(PLAT)' for WTP image tool requires CRYPTOPP_PATH or CRYPTOPP_INCDIR. Please set CRYPTOPP_PATH or CRYPTOPP_INCDIR to point to the right directory"))
	$(if $(wildcard $(CRYPTOPP_LIBDIR)/*),,$(error "Either 'CRYPTOPP_PATH' or 'CRYPTOPP_LIB' was set to '$(CRYPTOPP_LIBDIR)', but '$(CRYPTOPP_LIBDIR)' does not exist"))
	$(if $(wildcard $(CRYPTOPP_INCDIR)/*),,$(error "Either 'CRYPTOPP_PATH' or 'CRYPTOPP_INCDIR' was set to '$(CRYPTOPP_INCDIR)', but '$(CRYPTOPP_INCDIR)' does not exist"))
ifdef CRYPTOPP_PATH
	$(Q)$(MAKE) --no-print-directory -C $(CRYPTOPP_PATH) -f GNUmakefile
endif
	$(Q)$(MAKE) --no-print-directory -C $(DOIMAGEPATH)/wtptp/src/TBB_Linux -f TBB_linux.mak LIBDIR=$(CRYPTOPP_LIBDIR) INCDIR=$(CRYPTOPP_INCDIR)

$(WTMI_MULTI_IMG): FORCE
	$(Q)$(MAKE) --no-print-directory -C $(DOIMAGEPATH) WTMI_IMG=$(WTMI_IMG) DDR_TOPOLOGY=$(DDR_TOPOLOGY) CLOCKSPRESET=$(CLOCKSPRESET) WTMI

$(BUILD_PLAT)/wtmi.bin: $(WTMI_MULTI_IMG)
	$(Q)cp -a $(WTMI_MULTI_IMG) $(BUILD_PLAT)/wtmi.bin

$(TIMDDRTOOL): FORCE
	$(if $(value MV_DDR_PATH),,$(error "Platform '${PLAT}' for ddr tool requires MV_DDR_PATH. Please set MV_DDR_PATH to point to the right directory"))
	$(if $(wildcard $(value MV_DDR_PATH)/*),,$(error "'MV_DDR_PATH=$(value MV_DDR_PATH)' was specified, but '$(value MV_DDR_PATH)' directory does not exist"))
	$(if $(shell test -s "$(value MV_DDR_PATH)/branch.txt" || git -C $(value MV_DDR_PATH) rev-parse --show-cdup 2>&1),$(error "'MV_DDR_PATH=$(value MV_DDR_PATH)' was specified, but '$(value MV_DDR_PATH)' does not contain valid Marvell mv_ddr release tarball nor git repository"))
	$(Q)$(MAKE) --no-print-directory -C $(DOIMAGEPATH) MV_DDR_PATH=$(MV_DDR_PATH) DDR_TOPOLOGY=$(DDR_TOPOLOGY) mv_ddr

$(BUILD_PLAT)/$(UART_IMAGE): $(BUILD_PLAT)/$(BOOT_IMAGE) $(BUILD_PLAT)/wtmi.bin $(DOIMAGETOOL) $(TIMBUILD) $(TIMDDRTOOL)
	@$(ECHO_BLANK_LINE)
	@echo "Building uart images"
	$(Q)mkdir -p $(BUILD_PLAT)/$(BUILD_UART)
	$(Q)cp -a $(BUILD_PLAT)/wtmi.bin $(BUILD_PLAT)/$(BUILD_UART)/wtmi.bin
	$(Q)cp -a $(BUILD_PLAT)/$(BOOT_IMAGE) $(BUILD_PLAT)/$(BUILD_UART)/$(BOOT_IMAGE)
	$(Q)cd $(BUILD_PLAT)/$(BUILD_UART) && $(TIMBUILD) $(TIMBLDUARTARGS)
	$(Q)sed -i 's|WTMI_IMG|wtmi.bin|1' $(DOIMAGEUART_CFG)
	$(Q)sed -i 's|BOOT_IMAGE|$(BOOT_IMAGE)|1' $(DOIMAGEUART_CFG)
ifeq ($(MARVELL_SECURE_BOOT),1)
	$(Q)sed -i 's|WTMI_IMG|wtmi.bin|1' $(TIMNUARTCFG)
	$(Q)sed -i 's|BOOT_IMAGE|$(BOOT_IMAGE)|1' $(TIMNUARTCFG)
endif
	$(Q)cd $(BUILD_PLAT)/$(BUILD_UART) && $(DOIMAGETOOL) -r $(DOIMAGEUART_CFG) -v -D
ifeq ($(MARVELL_SECURE_BOOT),1)
	$(Q)cd $(BUILD_PLAT)/$(BUILD_UART) && $(DOIMAGETOOL) -r $(TIMNUARTCFG)
endif
	$(Q)tar czf $(BUILD_PLAT)/$(UART_IMAGE) -C $(BUILD_PLAT) $(BUILD_UART)/$(TIM_IMAGE) $(BUILD_UART)/wtmi_h.bin $(BUILD_UART)/boot-image_h.bin
	@$(ECHO_BLANK_LINE)
	@echo "Built $@ successfully"
	@$(ECHO_BLANK_LINE)

$(BUILD_PLAT)/$(FLASH_IMAGE): $(BUILD_PLAT)/$(BOOT_IMAGE) $(BUILD_PLAT)/wtmi.bin $(DOIMAGETOOL) $(TIMBUILD) $(TIMDDRTOOL) $(TIM2IMG)
	@$(ECHO_BLANK_LINE)
	@echo "Building flash image"
	$(Q)cd $(BUILD_PLAT) && $(TIMBUILD) $(TIMBLDARGS)
	$(Q)sed -i 's|WTMI_IMG|wtmi.bin|1' $(DOIMAGE_CFG)
	$(Q)sed -i 's|BOOT_IMAGE|$(BOOT_IMAGE)|1' $(DOIMAGE_CFG)
ifeq ($(MARVELL_SECURE_BOOT),1)
	$(Q)sed -i 's|WTMI_IMG|wtmi.bin|1' $(TIMNCFG)
	$(Q)sed -i 's|BOOT_IMAGE|$(BOOT_IMAGE)|1' $(TIMNCFG)
	@$(ECHO_BLANK_LINE)
	@echo "=======================================================";
	@echo "  Secure boot. Encrypting wtmi and boot-image";
	@echo "=======================================================";
	@$(ECHO_BLANK_LINE)
	$(Q)cp $(BUILD_PLAT)/wtmi.bin $(BUILD_PLAT)/wtmi-align.bin
	$(Q)truncate -s %16 $(BUILD_PLAT)/wtmi-align.bin
	$(Q)openssl enc -aes-256-cbc -e -in $(BUILD_PLAT)/wtmi-align.bin \
	-out $(BUILD_PLAT)/$(WTMI_ENC_IMG) \
	-K `cat $(IMAGESPATH)/aes-256.txt` -nosalt \
	-iv `cat $(IMAGESPATH)/iv.txt` -p
	$(Q)truncate -s %16 $(BUILD_PLAT)/$(BOOT_IMAGE);
	$(Q)openssl enc -aes-256-cbc -e -in $(BUILD_PLAT)/$(BOOT_IMAGE) \
	-out $(BUILD_PLAT)/$(BOOT_ENC_IMAGE) \
	-K `cat $(IMAGESPATH)/aes-256.txt` -nosalt \
	-iv `cat $(IMAGESPATH)/iv.txt` -p
endif
	$(Q)cd $(BUILD_PLAT) && $(DOIMAGETOOL) -r $(DOIMAGE_CFG) -v -D
ifeq ($(MARVELL_SECURE_BOOT),1)
	$(Q)cd $(BUILD_PLAT) && $(DOIMAGETOOL) -r $(TIMNCFG)
	$(Q)sed -i 's|wtmi.bin|$(WTMI_ENC_IMG)|1' $(TIMNCFG)
	$(Q)sed -i 's|$(BOOT_IMAGE)|$(BOOT_ENC_IMAGE)|1' $(TIMNCFG)
endif
	$(Q)cd $(BUILD_PLAT) && $(TIM2IMG) $(TIM2IMGARGS) -o $(BUILD_PLAT)/$(FLASH_IMAGE)
	@$(ECHO_BLANK_LINE)
	@echo "Built $@ successfully"
	@$(ECHO_BLANK_LINE)

clean realclean distclean: mrvl_clean

.PHONY: mrvl_clean
mrvl_clean:
	-$(Q)$(MAKE) --no-print-directory -C $(DOIMAGEPATH) MV_DDR_PATH=$(MV_DDR_PATH) clean
	-$(Q)$(MAKE) --no-print-directory -C $(DOIMAGEPATH)/wtptp/src/TBB_Linux -f TBB_linux.mak clean
ifdef CRYPTOPP_PATH
	-$(Q)$(MAKE) --no-print-directory -C $(CRYPTOPP_PATH) -f GNUmakefile clean
endif

else # WTP

$(BUILD_PLAT)/$(UART_IMAGE) $(BUILD_PLAT)/$(FLASH_IMAGE):
	$(error "Platform '${PLAT}' for target '$@' requires WTP. Please set WTP to point to the right directory")

endif # WTP

.PHONY: mrvl_uart
mrvl_uart: $(BUILD_PLAT)/$(UART_IMAGE)

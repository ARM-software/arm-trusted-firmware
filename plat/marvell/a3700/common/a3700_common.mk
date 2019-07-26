#
# Copyright (C) 2018 Marvell International Ltd.
#
# SPDX-License-Identifier:	BSD-3-Clause
# https://spdx.org/licenses
#

MARVELL_PLAT_BASE		:= plat/marvell
MARVELL_PLAT_INCLUDE_BASE	:= include/plat/marvell
PLAT_FAMILY			:= a3700
PLAT_FAMILY_BASE		:= $(MARVELL_PLAT_BASE)/$(PLAT_FAMILY)
PLAT_INCLUDE_BASE		:= $(MARVELL_PLAT_INCLUDE_BASE)/$(PLAT_FAMILY)
PLAT_COMMON_BASE		:= $(PLAT_FAMILY_BASE)/common
MARVELL_DRV_BASE		:= drivers/marvell
MARVELL_COMMON_BASE		:= $(MARVELL_PLAT_BASE)/common
HANDLE_EA_EL3_FIRST		:= 1

include $(MARVELL_PLAT_BASE)/marvell.mk

#*********** A3700 *************
DOIMAGEPATH	:= $(WTP)
DOIMAGETOOL	:= $(DOIMAGEPATH)/wtptp/linux/tbb_linux

ifeq ($(MARVELL_SECURE_BOOT),1)
DOIMAGE_CFG	:= $(DOIMAGEPATH)/atf-tim.txt
IMAGESPATH	:= $(DOIMAGEPATH)/tim/trusted

TIMNCFG		:= $(DOIMAGEPATH)/atf-timN.txt
TIMNSIG		:= $(IMAGESPATH)/timnsign.txt
TIM2IMGARGS	:= -i $(DOIMAGE_CFG) -n $(TIMNCFG)
TIMN_IMAGE	:= $$(grep "Image Filename:" -m 1 $(TIMNCFG) | cut -c 17-)
else #MARVELL_SECURE_BOOT
DOIMAGE_CFG	:= $(DOIMAGEPATH)/atf-ntim.txt
IMAGESPATH	:= $(DOIMAGEPATH)/tim/untrusted
TIM2IMGARGS	:= -i $(DOIMAGE_CFG)
endif #MARVELL_SECURE_BOOT

TIMBUILD	:= $(DOIMAGEPATH)/script/buildtim.sh
TIM2IMG		:= $(DOIMAGEPATH)/script/tim2img.pl

# WTMI_IMG is used to specify the customized RTOS image running over
# Service CPU (CM3 processor). By the default, it points to a
# baremetal binary of fuse programming in A3700_utils.
WTMI_IMG	:= $(DOIMAGEPATH)/wtmi/fuse/build/fuse.bin

# WTMI_SYSINIT_IMG is used for the system early initialization,
# such as AVS settings, clock-tree setup and dynamic DDR PHY training.
# After the initialization is done, this image will be wiped out
# from the memory and CM3 will continue with RTOS image or other application.
WTMI_SYSINIT_IMG	:= $(DOIMAGEPATH)/wtmi/sys_init/build/sys_init.bin

# WTMI_MULTI_IMG is composed of CM3 RTOS image (WTMI_IMG)
# and sys-init image (WTMI_SYSINIT_IMG).
WTMI_MULTI_IMG		:= $(DOIMAGEPATH)/wtmi/build/wtmi.bin

WTMI_ENC_IMG		:= $(DOIMAGEPATH)/wtmi/build/wtmi-enc.bin
BUILD_UART		:= uart-images

SRCPATH			:= $(dir $(BL33))

CLOCKSPRESET		?= CPU_800_DDR_800

DDR_TOPOLOGY		?= 0

BOOTDEV			?= SPINOR
PARTNUM			?= 0

TIM_IMAGE		:= $$(grep "Image Filename:" -m 1 $(DOIMAGE_CFG) | cut -c 17-)
TIMBLDARGS		:= $(MARVELL_SECURE_BOOT) $(BOOTDEV) $(IMAGESPATH) $(DOIMAGEPATH) $(CLOCKSPRESET) \
				$(DDR_TOPOLOGY) $(PARTNUM) $(DEBUG) $(DOIMAGE_CFG) $(TIMNCFG) $(TIMNSIG) 1
TIMBLDUARTARGS		:= $(MARVELL_SECURE_BOOT) UART $(IMAGESPATH) $(DOIMAGEPATH) $(CLOCKSPRESET) \
				$(DDR_TOPOLOGY) 0 0 $(DOIMAGE_CFG) $(TIMNCFG) $(TIMNSIG) 0
DOIMAGE_FLAGS		:= -r $(DOIMAGE_CFG) -v -D

# GICV3
$(eval $(call add_define,CONFIG_GICV3))

# CCI-400
$(eval $(call add_define,USE_CCI))

MARVELL_GIC_SOURCES	:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v3/gicv3_main.c		\
				drivers/arm/gic/v3/gicv3_helpers.c	\
				drivers/arm/gic/v3/arm_gicv3_common.c	\
				plat/common/plat_gicv3.c		\
				drivers/arm/gic/v3/gic500.c

PLAT_INCLUDES		:=	-I$(PLAT_FAMILY_BASE)/$(PLAT)		\
				-I$(PLAT_COMMON_BASE)/include		\
				-I$(PLAT_INCLUDE_BASE)/common		\
				-I$(MARVELL_DRV_BASE)			\
				-I$/drivers/arm/gic/common/

PLAT_BL_COMMON_SOURCES	:=	$(PLAT_COMMON_BASE)/aarch64/a3700_common.c \
				$(MARVELL_COMMON_BASE)/marvell_cci.c	   \
				$(MARVELL_DRV_BASE)/uart/a3700_console.S

BL1_SOURCES		+=	$(PLAT_COMMON_BASE)/aarch64/plat_helpers.S \
				lib/cpus/aarch64/cortex_a53.S

BL31_PORTING_SOURCES	:=	$(PLAT_FAMILY_BASE)/$(PLAT)/board/pm_src.c

MARVELL_DRV		:=	$(MARVELL_DRV_BASE)/comphy/phy-comphy-3700.c

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				$(PLAT_COMMON_BASE)/aarch64/plat_helpers.S \
				$(PLAT_COMMON_BASE)/plat_pm.c		\
				$(PLAT_COMMON_BASE)/dram_win.c		\
				$(PLAT_COMMON_BASE)/io_addr_dec.c	\
				$(PLAT_COMMON_BASE)/marvell_plat_config.c     \
				$(PLAT_COMMON_BASE)/a3700_ea.c		\
				$(PLAT_FAMILY_BASE)/$(PLAT)/plat_bl31_setup.c \
				$(MARVELL_COMMON_BASE)/marvell_ddr_info.c	\
				$(MARVELL_COMMON_BASE)/marvell_gicv3.c	\
				$(MARVELL_GIC_SOURCES)			\
				drivers/arm/cci/cci.c			\
				$(BL31_PORTING_SOURCES)			\
				$(PLAT_COMMON_BASE)/a3700_sip_svc.c	\
				$(MARVELL_DRV)

mrvl_flash: ${BUILD_PLAT}/${FIP_NAME} ${DOIMAGETOOL}
	$(shell truncate -s %128K ${BUILD_PLAT}/bl1.bin)
	$(shell cat ${BUILD_PLAT}/bl1.bin ${BUILD_PLAT}/${FIP_NAME} > ${BUILD_PLAT}/${BOOT_IMAGE})
	$(shell truncate -s %4 ${BUILD_PLAT}/${BOOT_IMAGE})
	$(shell truncate -s %4 $(WTMI_IMG))
	@echo
	@echo "Building uart images"
	$(TIMBUILD) $(TIMBLDUARTARGS)
	@sed -i 's|WTMI_IMG|$(WTMI_MULTI_IMG)|1' $(DOIMAGE_CFG)
	@sed -i 's|BOOT_IMAGE|$(BUILD_PLAT)/$(BOOT_IMAGE)|1' $(DOIMAGE_CFG)
ifeq ($(MARVELL_SECURE_BOOT),1)
	@sed -i 's|WTMI_IMG|$(WTMI_MULTI_IMG)|1' $(TIMNCFG)
	@sed -i 's|BOOT_IMAGE|$(BUILD_PLAT)/$(BOOT_IMAGE)|1' $(TIMNCFG)
endif
	$(DOIMAGETOOL) $(DOIMAGE_FLAGS)
	@if [ -e "$(TIMNCFG)" ]; then $(DOIMAGETOOL) -r $(TIMNCFG); fi
	@rm -rf $(BUILD_PLAT)/$(BUILD_UART)*
	@mkdir $(BUILD_PLAT)/$(BUILD_UART)
	@mv -t $(BUILD_PLAT)/$(BUILD_UART) $(TIM_IMAGE) $(DOIMAGE_CFG) $(TIMN_IMAGE) $(TIMNCFG)
	@find . -name "*_h.*" |xargs cp -ut $(BUILD_PLAT)/$(BUILD_UART)
	@mv $(subst .bin,_h.bin,$(WTMI_MULTI_IMG)) $(BUILD_PLAT)/$(BUILD_UART)/wtmi_h.bin
	@tar czf $(BUILD_PLAT)/$(BUILD_UART).tgz -C $(BUILD_PLAT) ./$(BUILD_UART)
	@echo
	@echo "Building flash image"
	$(TIMBUILD) $(TIMBLDARGS)
	sed -i 's|WTMI_IMG|$(WTMI_MULTI_IMG)|1' $(DOIMAGE_CFG)
	sed -i 's|BOOT_IMAGE|$(BUILD_PLAT)/$(BOOT_IMAGE)|1' $(DOIMAGE_CFG)
ifeq ($(MARVELL_SECURE_BOOT),1)
	@sed -i 's|WTMI_IMG|$(WTMI_MULTI_IMG)|1' $(TIMNCFG)
	@sed -i 's|BOOT_IMAGE|$(BUILD_PLAT)/$(BOOT_IMAGE)|1' $(TIMNCFG)
	@echo -e "\n\t=======================================================\n";
	@echo -e "\t  Secure boot. Encrypting wtmi and boot-image \n";
	@echo -e "\t=======================================================\n";
	@truncate -s %16 $(WTMI_MULTI_IMG)
	@openssl enc -aes-256-cbc -e -in $(WTMI_MULTI_IMG) \
	-out $(WTMI_ENC_IMG) \
	-K `cat $(IMAGESPATH)/aes-256.txt` -nosalt \
	-iv `cat $(IMAGESPATH)/iv.txt` -p
	@truncate -s %16 $(BUILD_PLAT)/$(BOOT_IMAGE);
	@openssl enc -aes-256-cbc -e -in $(BUILD_PLAT)/$(BOOT_IMAGE) \
	-out $(BUILD_PLAT)/$(BOOT_ENC_IMAGE) \
	-K `cat $(IMAGESPATH)/aes-256.txt` -nosalt \
	-iv `cat $(IMAGESPATH)/iv.txt` -p
endif
	$(DOIMAGETOOL) $(DOIMAGE_FLAGS)
	@if [ -e "$(TIMNCFG)" ]; then $(DOIMAGETOOL) -r $(TIMNCFG); fi
	@if [ "$(MARVELL_SECURE_BOOT)" = "1" ]; then sed -i 's|$(WTMI_MULTI_IMG)|$(WTMI_ENC_IMG)|1;s|$(BOOT_IMAGE)|$(BOOT_ENC_IMAGE)|1;' $(TIMNCFG); fi
	$(TIM2IMG) $(TIM2IMGARGS) -o $(BUILD_PLAT)/$(FLASH_IMAGE)
	@mv -t $(BUILD_PLAT) $(TIM_IMAGE) $(DOIMAGE_CFG) $(TIMN_IMAGE) $(TIMNCFG) $(WTMI_IMG) $(WTMI_SYSINIT_IMG) $(WTMI_MULTI_IMG)
	@if [ "$(MARVELL_SECURE_BOOT)" = "1" ]; then mv -t $(BUILD_PLAT) $(WTMI_ENC_IMG) OtpHash.txt; fi
	@find . -name "*.txt" | grep -E "CSK[[:alnum:]]_KeyHash.txt|Tim_msg.txt|TIMHash.txt" | xargs rm -f

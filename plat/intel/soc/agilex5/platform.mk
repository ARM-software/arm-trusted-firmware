#
# Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
# Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
# Copyright (c) 2024, Altera Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
include lib/xlat_tables_v2/xlat_tables.mk
PLAT_INCLUDES		:=	\
			-Iplat/intel/soc/agilex5/include/		\
			-Iplat/intel/soc/common/drivers/		\
			-Iplat/intel/soc/common/lib/sha/		\
			-Iplat/intel/soc/common/include/

# GIC-600 configuration
GICV3_SUPPORT_GIC600	:=	1
# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk
AGX5_GICv3_SOURCES	:=	\
			${GICV3_SOURCES}				\
			plat/common/plat_gicv3.c

PLAT_BL_COMMON_SOURCES	:=	\
			${AGX5_GICv3_SOURCES}				\
			drivers/cadence/combo_phy/cdns_combo_phy.c	\
			drivers/cadence/emmc/cdns_sdmmc.c	\
			drivers/cadence/nand/cdns_nand.c	\
			drivers/delay_timer/delay_timer.c		\
			drivers/delay_timer/generic_delay_timer.c	\
			drivers/ti/uart/aarch64/16550_console.S		\
			plat/intel/soc/common/aarch64/platform_common.c	\
			plat/intel/soc/common/aarch64/plat_helpers.S	\
			plat/intel/soc/common/drivers/ccu/ncore_ccu.c	\
			plat/intel/soc/common/drivers/combophy/combophy.c			\
			plat/intel/soc/common/drivers/sdmmc/sdmmc.c			\
			plat/intel/soc/common/drivers/ddr/ddr.c			\
			plat/intel/soc/common/drivers/nand/nand.c			\
			plat/intel/soc/common/lib/sha/sha.c				\
			plat/intel/soc/common/socfpga_delay_timer.c

BL2_SOURCES		+=	\
		common/desc_image_load.c				\
		lib/xlat_tables_v2/aarch64/enable_mmu.S	\
		lib/xlat_tables_v2/xlat_tables_context.c \
		lib/xlat_tables_v2/xlat_tables_core.c \
		lib/xlat_tables_v2/aarch64/xlat_tables_arch.c \
		lib/xlat_tables_v2/xlat_tables_utils.c \
		drivers/mmc/mmc.c					\
		drivers/intel/soc/stratix10/io/s10_memmap_qspi.c	\
		drivers/io/io_storage.c					\
		drivers/io/io_block.c					\
		drivers/io/io_fip.c					\
		drivers/io/io_mtd.c					\
		drivers/partition/partition.c				\
		drivers/partition/gpt.c					\
		drivers/synopsys/emmc/dw_mmc.c				\
		lib/cpus/aarch64/cortex_a55.S				\
		lib/cpus/aarch64/cortex_a76.S				\
		plat/intel/soc/agilex5/soc/agilex5_clock_manager.c	\
		plat/intel/soc/agilex5/soc/agilex5_memory_controller.c	\
		plat/intel/soc/agilex5/soc/agilex5_mmc.c		\
		plat/intel/soc/agilex5/soc/agilex5_pinmux.c		\
		plat/intel/soc/agilex5/soc/agilex5_power_manager.c	\
		plat/intel/soc/agilex5/soc/agilex5_ddr.c		\
		plat/intel/soc/agilex5/soc/agilex5_iossm_mailbox.c	\
		plat/intel/soc/common/bl2_plat_mem_params_desc.c	\
		plat/intel/soc/common/socfpga_image_load.c		\
		plat/intel/soc/common/socfpga_ros.c			\
		plat/intel/soc/common/socfpga_storage.c			\
		plat/intel/soc/common/socfpga_vab.c			\
		plat/intel/soc/common/soc/socfpga_emac.c		\
		plat/intel/soc/common/soc/socfpga_firewall.c		\
		plat/intel/soc/common/soc/socfpga_handoff.c		\
		plat/intel/soc/common/soc/socfpga_mailbox.c		\
		plat/intel/soc/common/soc/socfpga_reset_manager.c	\
		plat/intel/soc/common/drivers/qspi/cadence_qspi.c	\
		plat/intel/soc/agilex5/bl2_plat_setup.c			\
		plat/intel/soc/common/drivers/wdt/watchdog.c

include lib/zlib/zlib.mk
PLAT_INCLUDES	+=	-Ilib/zlib
BL2_SOURCES	+=	$(ZLIB_SOURCES)

BL31_SOURCES	+=	\
		drivers/arm/cci/cci.c					\
		${XLAT_TABLES_LIB_SRCS}						\
		lib/cpus/aarch64/aem_generic.S				\
		lib/cpus/aarch64/cortex_a55.S				\
		lib/cpus/aarch64/cortex_a76.S				\
		plat/common/plat_psci_common.c				\
		plat/intel/soc/agilex5/bl31_plat_setup.c		\
		plat/intel/soc/agilex5/soc/agilex5_cache.S		\
		plat/intel/soc/agilex5/soc/agilex5_clock_manager.c	\
		plat/intel/soc/agilex5/soc/agilex5_power_manager.c	\
		plat/intel/soc/common/socfpga_psci.c			\
		plat/intel/soc/common/socfpga_sip_svc.c			\
		plat/intel/soc/common/socfpga_sip_svc_v2.c			\
		plat/intel/soc/common/socfpga_topology.c		\
		plat/intel/soc/common/sip/socfpga_sip_ecc.c		\
		plat/intel/soc/common/sip/socfpga_sip_fcs.c		\
		plat/intel/soc/common/soc/socfpga_mailbox.c		\
		plat/intel/soc/common/soc/socfpga_system_manager.c	\
		plat/intel/soc/common/soc/socfpga_reset_manager.c

# Configs for A76 and A55
HW_ASSISTED_COHERENCY := 1
USE_COHERENT_MEM := 0
CTX_INCLUDE_AARCH32_REGS := 0
ERRATA_A55_1530923 := 1

# Don't have the Linux kernel as a BL33 image by default
ARM_LINUX_KERNEL_AS_BL33	:=	0
$(eval $(call assert_boolean,ARM_LINUX_KERNEL_AS_BL33))
$(eval $(call add_define,ARM_LINUX_KERNEL_AS_BL33))
$(eval $(call add_define,ARM_PRELOADED_DTB_BASE))

# Configs for Boot Source
SOCFPGA_BOOT_SOURCE_SDMMC		?=	0
SOCFPGA_BOOT_SOURCE_QSPI		?=	0
SOCFPGA_BOOT_SOURCE_NAND		?=	0

$(eval $(call assert_booleans,\
	$(sort \
		SOCFPGA_BOOT_SOURCE_SDMMC \
		SOCFPGA_BOOT_SOURCE_QSPI \
		SOCFPGA_BOOT_SOURCE_NAND \
)))
$(eval $(call add_defines,\
	$(sort \
		SOCFPGA_BOOT_SOURCE_SDMMC \
		SOCFPGA_BOOT_SOURCE_QSPI \
		SOCFPGA_BOOT_SOURCE_NAND \
)))

# Configs for VAB Authentication
SOCFPGA_SECURE_VAB_AUTH  := 	0
$(eval $(call assert_boolean,SOCFPGA_SECURE_VAB_AUTH))
$(eval $(call add_define,SOCFPGA_SECURE_VAB_AUTH))

PROGRAMMABLE_RESET_ADDRESS	:= 0
RESET_TO_BL2			:= 1
BL2_INV_DCACHE			:= 0
#
# Copyright (c) 2018-2020, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PROGRAMMABLE_RESET_ADDRESS	:= 0
COLD_BOOT_SINGLE_CPU		:= 1
ARM_CCI_PRODUCT_ID		:= 500
TRUSTED_BOARD_BOOT		:= 1
RESET_TO_BL31			:= 1
GENERATE_COT			:= 1
BL2_AT_EL3			:= 1
ENABLE_SVE_FOR_NS		:= 0
MULTI_CONSOLE_API		:= 1

CRASH_REPORTING			:= 1
HANDLE_EA_EL3_FIRST		:= 1

$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))

ifeq (${SPD},none)
  SPD_NONE:=1
  $(eval $(call add_define,SPD_NONE))
endif

# LSI setting common define
RCAR_H3:=0
RCAR_M3:=1
RCAR_M3N:=2
RCAR_E3:=3
RCAR_H3N:=4
RCAR_D3:=5
RCAR_V3M:=6
RCAR_AUTO:=99
RZ_G2M:=100
RZ_G2H:=101
RZ_G2N:=102
RZ_G2E:=103
$(eval $(call add_define,RCAR_H3))
$(eval $(call add_define,RCAR_M3))
$(eval $(call add_define,RCAR_M3N))
$(eval $(call add_define,RCAR_E3))
$(eval $(call add_define,RCAR_H3N))
$(eval $(call add_define,RCAR_D3))
$(eval $(call add_define,RCAR_V3M))
$(eval $(call add_define,RCAR_AUTO))
$(eval $(call add_define,RZ_G2M))
$(eval $(call add_define,RZ_G2H))
$(eval $(call add_define,RZ_G2N))
$(eval $(call add_define,RZ_G2E))

RCAR_CUT_10:=0
RCAR_CUT_11:=1
RCAR_CUT_13:=3
RCAR_CUT_20:=10
RCAR_CUT_30:=20
$(eval $(call add_define,RCAR_CUT_10))
$(eval $(call add_define,RCAR_CUT_11))
$(eval $(call add_define,RCAR_CUT_13))
$(eval $(call add_define,RCAR_CUT_20))
$(eval $(call add_define,RCAR_CUT_30))

# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_835769  := 1
ERRATA_A53_843419  := 1
ERRATA_A53_855873  := 1

# Enable workarounds for selected Cortex-A57 erratas.
ERRATA_A57_859972  := 1
ERRATA_A57_813419  := 1

PLAT_INCLUDES	:=	-Iplat/renesas/common/include/registers	\
			-Iplat/renesas/common/include		\
			-Iplat/renesas/common

PLAT_BL_COMMON_SOURCES	:=	drivers/renesas/common/iic_dvfs/iic_dvfs.c \
				plat/renesas/common/rcar_common.c

RCAR_GIC_SOURCES	:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c

BL2_SOURCES	+=	${RCAR_GIC_SOURCES}				\
			lib/cpus/aarch64/cortex_a53.S			\
			lib/cpus/aarch64/cortex_a57.S			\
			${LIBFDT_SRCS}					\
			common/desc_image_load.c			\
			plat/renesas/common/aarch64/platform_common.c	\
			plat/renesas/common/aarch64/plat_helpers.S	\
			plat/renesas/common/bl2_interrupt_error.c	\
			plat/renesas/common/bl2_secure_setting.c	\
			plat/renesas/common/plat_storage.c		\
			plat/renesas/common/bl2_plat_mem_params_desc.c	\
			plat/renesas/common/plat_image_load.c		\
			plat/renesas/common/bl2_cpg_init.c		\
			drivers/renesas/common/console/rcar_printf.c	\
			drivers/renesas/common/scif/scif.S		\
			drivers/renesas/common/common.c			\
			drivers/renesas/common/io/io_emmcdrv.c		\
			drivers/renesas/common/io/io_memdrv.c		\
			drivers/renesas/common/io/io_rcar.c		\
			drivers/renesas/common/auth/auth_mod.c		\
			drivers/renesas/common/rpc/rpc_driver.c		\
			drivers/renesas/common/dma/dma_driver.c		\
			drivers/renesas/common/avs/avs_driver.c		\
			drivers/renesas/common/delay/micro_delay.c	\
			drivers/renesas/common/emmc/emmc_interrupt.c	\
			drivers/renesas/common/emmc/emmc_utility.c	\
			drivers/renesas/common/emmc/emmc_mount.c	\
			drivers/renesas/common/emmc/emmc_init.c		\
			drivers/renesas/common/emmc/emmc_read.c		\
			drivers/renesas/common/emmc/emmc_cmd.c		\
			drivers/renesas/common/watchdog/swdt.c		\
			drivers/renesas/common/rom/rom_api.c		\
			drivers/io/io_storage.c

BL31_SOURCES	+=	${RCAR_GIC_SOURCES}				\
			lib/cpus/aarch64/cortex_a53.S			\
			lib/cpus/aarch64/cortex_a57.S			\
			plat/common/plat_psci_common.c			\
			plat/renesas/common/plat_topology.c		\
			plat/renesas/common/aarch64/plat_helpers.S	\
			plat/renesas/common/aarch64/platform_common.c	\
			plat/renesas/common/bl31_plat_setup.c		\
			plat/renesas/common/plat_pm.c			\
			drivers/renesas/common/console/rcar_console.S	\
			drivers/renesas/common/console/rcar_printf.c	\
			drivers/renesas/common/delay/micro_delay.c	\
			drivers/renesas/common/pwrc/call_sram.S		\
			drivers/renesas/common/pwrc/pwrc.c		\
			drivers/renesas/common/common.c			\
			drivers/arm/cci/cci.c

include lib/xlat_tables_v2/xlat_tables.mk
include drivers/auth/mbedtls/mbedtls_crypto.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

#
# Copyright (c) 2019-2022, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#
# Translation tables library
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_INCLUDES		:=	-Iplat/imx/common/include		\
				-Iplat/imx/imx8m/include		\
				-Iplat/imx/imx8m/imx8mm/include		\
				-Idrivers/imx/usdhc			\
				-Iinclude/common/tbbr			\
				-Iinclude/lib/libfdt

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

include lib/libfdt/libfdt.mk

IMX_GIC_SOURCES		:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/common/plat_psci_common.c		\
				plat/imx/common/plat_imx8_gic.c

BL31_SOURCES		+=	plat/imx/common/imx8_helpers.S			\
				plat/imx/imx8m/gpc_common.c			\
				plat/imx/imx8m/imx_aipstz.c			\
				plat/imx/imx8m/imx_rdc.c			\
				plat/imx/imx8m/imx8m_csu.c			\
				plat/imx/imx8m/imx8m_caam.c			\
				plat/imx/imx8m/imx8m_psci_common.c		\
				plat/imx/imx8m/imx8mm/imx8mm_bl31_setup.c	\
				plat/imx/imx8m/imx8mm/imx8mm_psci.c		\
				plat/imx/imx8m/imx8mm/gpc.c			\
				plat/imx/common/imx8_topology.c			\
				plat/imx/common/imx_sip_handler.c		\
				plat/imx/common/imx_sip_svc.c			\
				plat/imx/common/imx_uart_console.S		\
				lib/cpus/aarch64/cortex_a53.S			\
				drivers/arm/tzc/tzc380.c			\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				${XLAT_TABLES_LIB_SRCS}				\
				${IMX_GIC_SOURCES}

ifeq (${NEED_BL2},yes)
BL2_SOURCES		+=	common/desc_image_load.c			\
				common/fdt_wrappers.c				\
				plat/imx/common/imx8_helpers.S			\
				plat/imx/common/imx_uart_console.S		\
				plat/imx/imx8m/imx8mm/imx8mm_bl2_el3_setup.c	\
				plat/imx/imx8m/imx8mm/gpc.c			\
				plat/imx/imx8m/imx_aipstz.c			\
				plat/common/plat_psci_common.c			\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				lib/xlat_tables/xlat_tables_common.c		\
				lib/cpus/aarch64/cortex_a53.S			\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				${PLAT_GIC_SOURCES}				\
				${PLAT_DRAM_SOURCES}				\
				drivers/mmc/mmc.c				\
				drivers/io/io_block.c				\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				drivers/imx/usdhc/imx_usdhc.c			\
				plat/imx/imx8m/imx8mm/imx8mm_bl2_mem_params_desc.c	\
				plat/imx/common/imx_io_storage.c		\
				plat/imx/imx8m/imx8m_image_load.c		\
				lib/optee/optee_utils.c
endif

# Add the build options to pack BLx images and kernel device tree
# in the FIP if the platform requires.
ifneq ($(BL2),)
RESET_TO_BL31		:=	0
$(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/tb_fw.crt,--tb-fw-cert))
endif
ifneq ($(BL32_EXTRA1),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA1,--tos-fw-extra1))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA2,--tos-fw-extra2))
endif
ifneq ($(HW_CONFIG),)
$(eval $(call TOOL_ADD_IMG,HW_CONFIG,--hw-config))
endif

ifeq (${NEED_BL2},yes)
$(eval $(call add_define,NEED_BL2))
LOAD_IMAGE_V2		:=	1
# Non-TF Boot ROM
BL2_AT_EL3		:=	1
endif

ifneq (${TRUSTED_BOARD_BOOT},0)

include drivers/auth/mbedtls/mbedtls_crypto.mk
include drivers/auth/mbedtls/mbedtls_x509.mk

AUTH_SOURCES	:=	drivers/auth/auth_mod.c			\
			drivers/auth/crypto_mod.c		\
			drivers/auth/img_parser_mod.c		\
			drivers/auth/tbbr/tbbr_cot_common.c     \
			drivers/auth/tbbr/tbbr_cot_bl2.c

BL2_SOURCES	+=	${AUTH_SOURCES}					\
			plat/common/tbbr/plat_tbbr.c			\
			plat/imx/imx8m/imx8mm/imx8mm_trusted_boot.c	\
			plat/imx/imx8m/imx8mm/imx8mm_rotpk.S

ROT_KEY             = $(BUILD_PLAT)/rot_key.pem
ROTPK_HASH          = $(BUILD_PLAT)/rotpk_sha256.bin

$(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))
$(eval $(call MAKE_LIB_DIRS))

$(BUILD_PLAT)/bl2/imx8mm_rotpk.o: $(ROTPK_HASH)

certificates: $(ROT_KEY)

$(ROT_KEY): | $(BUILD_PLAT)
	@echo "  OPENSSL $@"
	@if [ ! -f $(ROT_KEY) ]; then \
		openssl genrsa 2048 > $@ 2>/dev/null; \
	fi

$(ROTPK_HASH): $(ROT_KEY)
	@echo "  OPENSSL $@"
	$(Q)openssl rsa -in $< -pubout -outform DER 2>/dev/null |\
	openssl dgst -sha256 -binary > $@ 2>/dev/null
endif

USE_COHERENT_MEM	:=	1
RESET_TO_BL31		:=	1
A53_DISABLE_NON_TEMPORAL_HINT := 0

ERRATA_A53_835769	:=	1
ERRATA_A53_843419	:=	1
ERRATA_A53_855873	:=	1

BL32_BASE		?=	0xbe000000
$(eval $(call add_define,BL32_BASE))

BL32_SIZE		?=	0x2000000
$(eval $(call add_define,BL32_SIZE))

IMX_BOOT_UART_BASE	?=	0x30890000
$(eval $(call add_define,IMX_BOOT_UART_BASE))

EL3_EXCEPTION_HANDLING := $(SDEI_SUPPORT)
ifeq (${SDEI_SUPPORT}, 1)
BL31_SOURCES 		+= 	plat/imx/common/imx_ehf.c	\
				plat/imx/common/imx_sdei.c
endif

ifeq (${MEASURED_BOOT},1)
    MEASURED_BOOT_MK := drivers/measured_boot/event_log/event_log.mk
    $(info Including ${MEASURED_BOOT_MK})
    include ${MEASURED_BOOT_MK}

BL2_SOURCES		+=	plat/imx/imx8m/imx8m_measured_boot.c	\
				plat/imx/imx8m/imx8m_dyn_cfg_helpers.c	\
				${EVENT_LOG_SOURCES}
endif

ifeq (${SPD},trusty)
	BL31_CFLAGS    +=      -DPLAT_XLAT_TABLES_DYNAMIC=1
endif

#
# Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Architecture
$(eval $(call add_define,ARMV7_SUPPORTS_LARGE_PAGE_ADDRESSING))

TF_CFLAGS	+=	-mfpu=neon
ASFLAGS		+=	-mfpu=neon

# Platform
PLAT_INCLUDES		:=	-Idrivers/imx/uart			\
				-Iplat/imx/common/include		\
				-Iplat/imx/imx7/include			\
				-Idrivers/imx/timer			\
				-Idrivers/imx/usdhc			\

# Translation tables library
include lib/xlat_tables_v2/xlat_tables.mk

BL2_SOURCES		+=	common/desc_image_load.c			\
				drivers/delay_timer/delay_timer.c		\
				drivers/mmc/mmc.c				\
				drivers/io/io_block.c				\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				drivers/imx/timer/imx_gpt.c			\
				drivers/imx/uart/imx_uart.c			\
				drivers/imx/uart/imx_crash_uart.S		\
				lib/aarch32/arm32_aeabi_divmod.c		\
				lib/aarch32/arm32_aeabi_divmod_a32.S		\
				lib/cpus/aarch32/cortex_a7.S			\
				lib/optee/optee_utils.c				\
				plat/imx/common/imx_aips.c			\
				plat/imx/common/imx_caam.c			\
				plat/imx/common/imx_clock.c			\
				plat/imx/common/imx_csu.c			\
				plat/imx/common/imx_io_mux.c			\
				plat/imx/common/imx_snvs.c			\
				plat/imx/common/imx_wdog.c			\
				plat/imx/common/imx7_clock.c			\
				plat/imx/imx7/common/imx7_bl2_mem_params_desc.c	\
				plat/imx/imx7/common/imx7_bl2_el3_common.c	\
				plat/imx/imx7/common/imx7_helpers.S		\
				plat/imx/imx7/common/imx7_image_load.c		\
				plat/imx/imx7/common/imx7_io_storage.c		\
				plat/imx/common/aarch32/imx_uart_console.S	\
				${XLAT_TABLES_LIB_SRCS}

ifneq (${TRUSTED_BOARD_BOOT},0)

include drivers/auth/mbedtls/mbedtls_crypto.mk
include drivers/auth/mbedtls/mbedtls_x509.mk

AUTH_SOURCES	:=	drivers/auth/auth_mod.c			\
			drivers/auth/crypto_mod.c		\
			drivers/auth/img_parser_mod.c		\
			drivers/auth/tbbr/tbbr_cot.c

BL2_SOURCES		+=	${AUTH_SOURCES}					\
				plat/common/tbbr/plat_tbbr.c			\
				plat/imx/imx7/common/imx7_trusted_boot.c	\
				plat/imx/imx7/common/imx7_rotpk.S

ROT_KEY             = $(BUILD_PLAT)/rot_key.pem
ROTPK_HASH          = $(BUILD_PLAT)/rotpk_sha256.bin

$(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))
$(eval $(call MAKE_LIB_DIRS))

$(BUILD_PLAT)/bl2/imx7_rotpk.o: $(ROTPK_HASH)

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

# Add the build options to pack BLx images and kernel device tree
# in the FIP if the platform requires.
ifneq ($(BL2),)
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

# Verify build config
# -------------------

ifeq (${ARCH},aarch64)
  $(error Error: AArch64 not supported on i.mx7)
endif

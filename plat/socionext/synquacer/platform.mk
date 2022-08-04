#
# Copyright (c) 2018-2022, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

override PROGRAMMABLE_RESET_ADDRESS	:= 1
override USE_COHERENT_MEM		:= 1
override SEPARATE_CODE_AND_RODATA	:= 1
override ENABLE_SVE_FOR_NS		:= 0
# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_855873		:= 1

ifeq (${RESET_TO_BL31}, 1)
override RESET_TO_BL31          := 1
override TRUSTED_BOARD_BOOT     := 0
SQ_USE_SCMI_DRIVER              ?= 0
else
override RESET_TO_BL31          := 0
override BL2_AT_EL3             := 1
SQ_USE_SCMI_DRIVER              := 1
BL2_CPPFLAGS                    += -DPLAT_XLAT_TABLES_DYNAMIC
endif

# Libraries
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_PATH		:=	plat/socionext/synquacer
PLAT_INCLUDES		:=	-I$(PLAT_PATH)/include		\
				-I$(PLAT_PATH)/drivers/scpi	\
				-I$(PLAT_PATH)/drivers/mhu \
				-Idrivers/arm/css/scmi \
				-Idrivers/arm/css/scmi/vendor

PLAT_BL_COMMON_SOURCES	+=	$(PLAT_PATH)/sq_helpers.S		\
				drivers/arm/pl011/aarch64/pl011_console.S \
				drivers/delay_timer/delay_timer.c	\
				drivers/delay_timer/generic_delay_timer.c \
				lib/cpus/aarch64/cortex_a53.S		\
				$(PLAT_PATH)/sq_xlat_setup.c	\
				${XLAT_TABLES_LIB_SRCS}

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

ifneq (${RESET_TO_BL31}, 1)
BL2_SOURCES		+=	common/desc_image_load.c		\
				drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				drivers/io/io_storage.c			\
				$(PLAT_PATH)/sq_bl2_setup.c		\
				$(PLAT_PATH)/sq_image_desc.c	\
				$(PLAT_PATH)/sq_io_storage.c

ifeq (${TRUSTED_BOARD_BOOT},1)
include drivers/auth/mbedtls/mbedtls_crypto.mk
include drivers/auth/mbedtls/mbedtls_x509.mk
BL2_SOURCES		+=	drivers/auth/auth_mod.c			\
				drivers/auth/crypto_mod.c		\
				drivers/auth/img_parser_mod.c		\
				drivers/auth/tbbr/tbbr_cot_common.c	\
				drivers/auth/tbbr/tbbr_cot_bl2.c	\
				plat/common/tbbr/plat_tbbr.c		\
				$(PLAT_PATH)/sq_rotpk.S		\
				$(PLAT_PATH)/sq_tbbr.c

ROT_KEY			= $(BUILD_PLAT)/rot_key.pem
ROTPK_HASH		= $(BUILD_PLAT)/rotpk_sha256.bin

$(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))
$(BUILD_PLAT)/bl2/sq_rotpk.o: $(ROTPK_HASH)

certificates: $(ROT_KEY)
$(ROT_KEY): | $(BUILD_PLAT)
	@echo "  OPENSSL $@"
	$(Q)${OPENSSL_BIN_PATH}/openssl genrsa 2048 > $@ 2>/dev/null

$(ROTPK_HASH): $(ROT_KEY)
	@echo "  OPENSSL $@"
	$(Q)${OPENSSL_BIN_PATH}/openssl rsa -in $< -pubout -outform DER 2>/dev/null |\
	${OPENSSL_BIN_PATH}/openssl dgst -sha256 -binary > $@ 2>/dev/null

endif	# TRUSTED_BOARD_BOOT
endif

BL31_SOURCES		+=	drivers/arm/ccn/ccn.c			\
				${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/common/plat_psci_common.c		\
				$(PLAT_PATH)/sq_bl31_setup.c		\
				$(PLAT_PATH)/sq_ccn.c			\
				$(PLAT_PATH)/sq_topology.c		\
				$(PLAT_PATH)/sq_psci.c			\
				$(PLAT_PATH)/sq_gicv3.c			\
				$(PLAT_PATH)/drivers/scp/sq_scp.c

ifeq (${SQ_USE_SCMI_DRIVER},0)
BL31_SOURCES		+=	$(PLAT_PATH)/drivers/scpi/sq_scpi.c	\
				$(PLAT_PATH)/drivers/mhu/sq_mhu.c
else
BL31_SOURCES		+=	$(PLAT_PATH)/drivers/scp/sq_scmi.c		\
				drivers/arm/css/scmi/scmi_common.c		\
				drivers/arm/css/scmi/scmi_pwr_dmn_proto.c	\
				drivers/arm/css/scmi/scmi_sys_pwr_proto.c	\
				drivers/arm/css/scmi/vendor/scmi_sq.c	\
				drivers/arm/css/mhu/css_mhu_doorbell.c
endif

ifeq (${SPM_MM},1)
$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))

BL31_SOURCES		+=	$(PLAT_PATH)/sq_spm.c
endif

ifeq (${SQ_USE_SCMI_DRIVER},1)
$(eval $(call add_define,SQ_USE_SCMI_DRIVER))
endif

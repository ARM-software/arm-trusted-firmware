#
# Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_BL_COMMON_SOURCES	+=	drivers/arm/pl011/${ARCH}/pl011_console.S	\
				plat/arm/board/common/${ARCH}/board_arm_helpers.S

BL1_SOURCES		+=	drivers/cfi/v2m/v2m_flash.c

BL2_SOURCES		+=	drivers/cfi/v2m/v2m_flash.c

ifneq (${TRUSTED_BOARD_BOOT},0)
ARM_ROTPK_S = plat/arm/board/common/rotpk/arm_dev_rotpk.S
ARM_ROTPK = $(BUILD_PLAT)/arm_rotpk.bin
ARM_ROTPK_IS_HASH := 1
$(eval $(call add_define_val,ARM_ROTPK,'"$(ARM_ROTPK)"'))

# ROTPK hash location
ifeq (${ARM_ROTPK_LOCATION}, regs)
	ARM_ROTPK_LOCATION_ID = ARM_ROTPK_REGS_ID
else
# The ROTPK is a development key
ifeq (${ARM_ROTPK_LOCATION}, devel_rsa)
	CRYPTO_ALG=rsa
	ARM_ROTPK_LOCATION_ID = ARM_ROTPK_DEVEL_RSA_ID
	ROT_KEY ?= plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem
$(warning Development keys support for FVP is deprecated. Use `regs` \
option instead)
else ifeq (${ARM_ROTPK_LOCATION}, devel_ecdsa)
	CRYPTO_ALG=ec
	ARM_ROTPK_LOCATION_ID = ARM_ROTPK_DEVEL_ECDSA_ID
	ROT_KEY ?= plat/arm/board/common/rotpk/arm_rotprivk_ecdsa.pem
$(warning Development keys support for FVP is deprecated. Use `regs` \
option instead)
else ifeq (${ARM_ROTPK_LOCATION}, devel_full_dev_rsa_key)
	CRYPTO_ALG=rsa
	ARM_ROTPK_LOCATION_ID = ARM_ROTPK_DEVEL_FULL_DEV_RSA_KEY_ID
	ROT_KEY ?= plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem
	ARM_ROTPK_IS_HASH = 0
$(warning Development keys support for FVP is deprecated. Use `regs` \
option instead)
else ifeq (${ARM_ROTPK_LOCATION}, devel_full_dev_ecdsa_key)
	CRYPTO_ALG=ec
	ARM_ROTPK_LOCATION_ID = ARM_ROTPK_DEVEL_FULL_DEV_ECDSA_KEY_ID
	ROT_KEY ?= plat/arm/board/common/rotpk/arm_rotprivk_ecdsa.pem
	ARM_ROTPK_IS_HASH = 0
$(warning Development keys support for FVP is deprecated. Use `regs` \
option instead)
else
$(error "Unsupported ARM_ROTPK_LOCATION value")
endif
$(BUILD_PLAT)/bl1/arm_dev_rotpk.o : $(ARM_ROTPK)
$(BUILD_PLAT)/bl2/arm_dev_rotpk.o : $(ARM_ROTPK)
endif

$(eval $(call add_define,ARM_ROTPK_LOCATION_ID))
$(eval $(call add_define,ARM_ROTPK_IS_HASH))

ifeq (${ENABLE_RME}, 1)
COT	:=	cca
endif

# Force generation of the ROT public key if ROT_KEY is specified
ifdef ROT_KEY
	PK_PREREQUISITES = $(ROT_KEY) FORCE
endif

$(ARM_ROTPK) : $(PK_PREREQUISITES)
ifndef ROT_KEY
	$(error Cannot generate public key: no ROT_KEY defined)
endif
ifeq ($(ARM_ROTPK_IS_HASH), 1)
	${OPENSSL_BIN_PATH}/openssl ${CRYPTO_ALG} -in $< -pubout -outform DER | \
	${OPENSSL_BIN_PATH}/openssl dgst -${HASH_ALG} -binary -out $@
else
	${OPENSSL_BIN_PATH}/openssl ${CRYPTO_ALG} -in $< -pubout -outform DER -out $@
endif

# Certificate NV-Counters. Use values corresponding to tied off values in
# ARM development platforms
TFW_NVCTR_VAL	?=	31
NTFW_NVCTR_VAL	?=	223
# The CCA Non-Volatile Counter only exists on some Arm development platforms.
# On others, we mock it by aliasing it to the Trusted Firmware Non-Volatile counter,
# hence we set both counters to the same default value.
CCAFW_NVCTR_VAL	?=	31

BL1_SOURCES		+=	plat/arm/board/common/board_arm_trusted_boot.c \
				${ARM_ROTPK_S}
BL2_SOURCES		+=	plat/arm/board/common/board_arm_trusted_boot.c \
				${ARM_ROTPK_S}

# Allows platform code to provide implementation variants depending on the
# selected chain of trust.
$(eval $(call add_define,ARM_COT_${COT}))

ifeq (${COT},dualroot)
# Platform Root of Trust key files.
ARM_PROT_KEY		:=	plat/arm/board/common/protpk/arm_protprivk_rsa.pem
ARM_PROTPK_HASH		:=	plat/arm/board/common/protpk/arm_protpk_rsa_sha256.bin

# Provide the private key to cert_create tool. It needs it to sign the images.
PROT_KEY		:=	${ARM_PROT_KEY}

$(eval $(call add_define_val,ARM_PROTPK_HASH,'"$(ARM_PROTPK_HASH)"'))

BL1_SOURCES		+=	plat/arm/board/common/protpk/arm_dev_protpk.S
BL2_SOURCES		+=	plat/arm/board/common/protpk/arm_dev_protpk.S

$(BUILD_PLAT)/bl1/arm_dev_protpk.o: $(ARM_PROTPK_HASH)
$(BUILD_PLAT)/bl2/arm_dev_protpk.o: $(ARM_PROTPK_HASH)
endif

ifeq (${COT},cca)
# Platform and Secure World Root of Trust key files.
ARM_PROT_KEY		:=	plat/arm/board/common/protpk/arm_protprivk_rsa.pem
ARM_PROTPK_HASH		:=	plat/arm/board/common/protpk/arm_protpk_rsa_sha256.bin
ARM_SWD_ROT_KEY		:=	plat/arm/board/common/swd_rotpk/arm_swd_rotprivk_rsa.pem
ARM_SWD_ROTPK_HASH	:=	plat/arm/board/common/swd_rotpk/arm_swd_rotpk_rsa_sha256.bin

# Provide the private keys to cert_create tool. It needs them to sign the images.
PROT_KEY		:=	${ARM_PROT_KEY}
SWD_ROT_KEY		:=	${ARM_SWD_ROT_KEY}

$(eval $(call add_define_val,ARM_PROTPK_HASH,'"$(ARM_PROTPK_HASH)"'))
$(eval $(call add_define_val,ARM_SWD_ROTPK_HASH,'"$(ARM_SWD_ROTPK_HASH)"'))

BL1_SOURCES		+=	plat/arm/board/common/protpk/arm_dev_protpk.S \
				plat/arm/board/common/swd_rotpk/arm_dev_swd_rotpk.S
BL2_SOURCES		+=	plat/arm/board/common/protpk/arm_dev_protpk.S \
				plat/arm/board/common/swd_rotpk/arm_dev_swd_rotpk.S

$(BUILD_PLAT)/bl1/arm_dev_protpk.o: $(ARM_PROTPK_HASH)
$(BUILD_PLAT)/bl1/arm_dev_swd_rotpk.o: $(ARM_SWD_ROTPK_HASH)
$(BUILD_PLAT)/bl2/arm_dev_protpk.o: $(ARM_PROTPK_HASH)
$(BUILD_PLAT)/bl2/arm_dev_swd_rotpk.o: $(ARM_SWD_ROTPK_HASH)
endif

endif

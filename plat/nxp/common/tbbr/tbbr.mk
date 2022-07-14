#
# Copyright 2020-2022 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# For TRUSTED_BOARD_BOOT platforms need to include this makefile
# Following definations are to be provided by platform.mk file or
# by user - BL33_INPUT_FILE, BL32_INPUT_FILE, BL31_INPUT_FILE

ifeq ($(CHASSIS), 2)
include $(PLAT_DRIVERS_PATH)/csu/csu.mk
CSF_FILE		:=	input_blx_ch${CHASSIS}
BL2_CSF_FILE		:=	input_bl2_ch${CHASSIS}
else
ifeq ($(CHASSIS), 3)
CSF_FILE		:=	input_blx_ch${CHASSIS}
BL2_CSF_FILE		:=	input_bl2_ch${CHASSIS}
PBI_CSF_FILE		:=	input_pbi_ch${CHASSIS}
$(eval $(call add_define, CSF_HDR_CH3))
else
ifeq ($(CHASSIS), 3_2)
CSF_FILE		:=	input_blx_ch3
BL2_CSF_FILE		:=	input_bl2_ch${CHASSIS}
PBI_CSF_FILE		:=	input_pbi_ch${CHASSIS}
$(eval $(call add_define, CSF_HDR_CH3))
else
    $(error -> CHASSIS not set!)
endif
endif
endif

PLAT_AUTH_PATH		:=  $(PLAT_DRIVERS_PATH)/auth


ifeq (${BL2_INPUT_FILE},)
    BL2_INPUT_FILE	:= $(PLAT_AUTH_PATH)/csf_hdr_parser/${BL2_CSF_FILE}
endif

ifeq (${PBI_INPUT_FILE},)
    PBI_INPUT_FILE	:= $(PLAT_AUTH_PATH)/csf_hdr_parser/${PBI_CSF_FILE}
endif

# If MBEDTLS_DIR is not specified, use CSF Header option
ifeq (${MBEDTLS_DIR},)
    # Generic image processing filters to prepend CSF header
    ifeq (${BL33_INPUT_FILE},)
    BL33_INPUT_FILE	:= $(PLAT_AUTH_PATH)/csf_hdr_parser/${CSF_FILE}
    endif

    ifeq (${BL31_INPUT_FILE},)
    BL31_INPUT_FILE	:= $(PLAT_AUTH_PATH)/csf_hdr_parser/${CSF_FILE}
    endif

    ifeq (${BL32_INPUT_FILE},)
    BL32_INPUT_FILE	:= $(PLAT_AUTH_PATH)/csf_hdr_parser/${CSF_FILE}
    endif

    ifeq (${FUSE_INPUT_FILE},)
    FUSE_INPUT_FILE	:= $(PLAT_AUTH_PATH)/csf_hdr_parser/${CSF_FILE}
    endif

    PLAT_INCLUDES	+= -I$(PLAT_DRIVERS_PATH)/sfp
    PLAT_TBBR_SOURCES	+= $(PLAT_AUTH_PATH)/csf_hdr_parser/cot.c	\
			   $(PLAT_COMMON_PATH)/tbbr/csf_tbbr.c
    # IMG PARSER here is CSF header parser
    include $(PLAT_DRIVERS_PATH)/auth/csf_hdr_parser/csf_hdr.mk
    PLAT_TBBR_SOURCES 	+=	$(CSF_HDR_SOURCES)

    SCP_BL2_PRE_TOOL_FILTER	:= CST_SCP_BL2
    BL31_PRE_TOOL_FILTER	:= CST_BL31
    BL32_PRE_TOOL_FILTER	:= CST_BL32
    BL33_PRE_TOOL_FILTER	:= CST_BL33
else

    ifeq (${DISABLE_FUSE_WRITE}, 1)
        $(eval $(call add_define,DISABLE_FUSE_WRITE))
    endif

    # For Mbedtls currently crypto is not supported via CAAM
    # enable it when that support is there
    CAAM_INTEG		:= 0
    KEY_ALG		:= rsa
    KEY_SIZE		:= 2048

    $(eval $(call add_define,MBEDTLS_X509))
    ifeq (${PLAT_DDR_PHY},PHY_GEN2)
        $(eval $(call add_define,PLAT_DEF_OID))
    endif
    include drivers/auth/mbedtls/mbedtls_x509.mk


    PLAT_TBBR_SOURCES	+= $(PLAT_AUTH_PATH)/tbbr/tbbr_cot.c \
			   $(PLAT_COMMON_PATH)/tbbr/nxp_rotpk.S \
			   $(PLAT_COMMON_PATH)/tbbr/x509_tbbr.c

    #ROTPK key is embedded in BL2 image
    ifeq (${ROT_KEY},)
	ROT_KEY		= $(BUILD_PLAT)/rot_key.pem
    endif

    ifeq (${SAVE_KEYS},1)

        ifeq (${TRUSTED_WORLD_KEY},)
            TRUSTED_WORLD_KEY = ${BUILD_PLAT}/trusted.pem
        endif

        ifeq (${NON_TRUSTED_WORLD_KEY},)
            NON_TRUSTED_WORLD_KEY = ${BUILD_PLAT}/non-trusted.pem
        endif

        ifeq (${BL31_KEY},)
            BL31_KEY = ${BUILD_PLAT}/soc.pem
        endif

        ifeq (${BL32_KEY},)
            BL32_KEY = ${BUILD_PLAT}/trusted_os.pem
        endif

        ifeq (${BL33_KEY},)
            BL33_KEY = ${BUILD_PLAT}/non-trusted_os.pem
        endif

    endif

    ROTPK_HASH		= $(BUILD_PLAT)/rotpk_sha256.bin

    $(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))

    $(BUILD_PLAT)/bl2/nxp_rotpk.o: $(ROTPK_HASH)

    certificates: $(ROT_KEY)
    $(ROT_KEY): | $(BUILD_PLAT)
	@echo "  OPENSSL $@"
	@if [ ! -f $(ROT_KEY) ]; then \
		${OPENSSL_BIN_PATH}/openssl genrsa 2048 > $@ 2>/dev/null; \
	fi

    $(ROTPK_HASH): $(ROT_KEY)
	@echo "  OPENSSL $@"
	$(Q)${OPENSSL_BIN_PATH}/openssl rsa -in $< -pubout -outform DER 2>/dev/null |\
	${OPENSSL_BIN_PATH}/openssl dgst -sha256 -binary > $@ 2>/dev/null

endif #MBEDTLS_DIR

PLAT_INCLUDES		+=	-Iinclude/common/tbbr

# Generic files for authentication framework
TBBR_SOURCES		+=	drivers/auth/auth_mod.c		\
				drivers/auth/crypto_mod.c	\
				drivers/auth/img_parser_mod.c	\
				plat/common/tbbr/plat_tbbr.c	\
				${PLAT_TBBR_SOURCES}

# If CAAM_INTEG is not defined (would be scenario with MBED TLS)
# include mbedtls_crypto
ifeq (${CAAM_INTEG},0)
    include drivers/auth/mbedtls/mbedtls_crypto.mk
else
    include $(PLAT_DRIVERS_PATH)/crypto/caam/src/auth/auth.mk
    TBBR_SOURCES	+= ${AUTH_SOURCES}
endif

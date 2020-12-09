#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

NEED_FUSE	:= yes

$(eval $(call add_define, PLAT_DEF_FIP_UUID))
$(eval $(call add_define, POLICY_FUSE_PROVISION))
$(eval $(call add_define, PLAT_TBBR_IMG_DEF))

$(eval $(call SET_NXP_MAKE_FLAG,IMG_LOADR_NEEDED,BL2))
$(eval $(call SET_NXP_MAKE_FLAG,SFP_NEEDED,BL2))
$(eval $(call SET_NXP_MAKE_FLAG,GPIO_NEEDED,BL2))

FIP_HANDLER_PATH	:=  ${PLAT_COMMON_PATH}/fip_handler
FIP_HANDLER_COMMON_PATH	:=  ${FIP_HANDLER_PATH}/common

FUSE_SOURCES		:=  ${FIP_HANDLER_PATH}/fuse_fip/fuse_io_storage.c

PLAT_INCLUDES		+=  -I${FIP_HANDLER_COMMON_PATH}\
			    -I${FIP_HANDLER_PATH}/fuse_fip

FUSE_FIP_NAME		:=	fuse_fip.bin

fip_fuse: ${BUILD_PLAT}/${FUSE_FIP_NAME}

ifeq (${FUSE_PROV_FILE},)

$(shell cp tools/nxp/plat_fiptool/plat_fiptool.mk ${PLAT_DIR})

else
ifeq (${TRUSTED_BOARD_BOOT},1)
FUSE_PROV_FILE_SB = $(notdir ${FUSE_PROV_FILE})_prov.sb
FUSE_FIP_ARGS += --fuse-prov ${BUILD_PLAT}/${FUSE_PROV_FILE_SB}
FUSE_FIP_DEPS += ${BUILD_PLAT}/${FUSE_PROV_FILE_SB}
else
FUSE_FIP_ARGS += --fuse-prov ${FUSE_PROV_FILE}
FUSE_FIP_DEPS += ${FUSE_PROV_FILE}
endif
endif

ifeq (${FUSE_UP_FILE},)
else
ifeq (${TRUSTED_BOARD_BOOT},1)
FUSE_UP_FILE_SB = $(notdir ${FUSE_UP_FILE})_up.sb
FUSE_FIP_ARGS += --fuse-up ${BUILD_PLAT}/${FUSE_UP_FILE_SB}
FUSE_FIP_DEPS += ${BUILD_PLAT}/${FUSE_UP_FILE_SB}
else
FUSE_FIP_ARGS += --fuse-up ${FUSE_UP_FILE}
FUSE_FIP_DEPS += ${FUSE_UP_FILE}
endif
endif

ifeq (${TRUSTED_BOARD_BOOT},1)

ifeq (${MBEDTLS_DIR},)
else
  $(error Error: Trusted Board Boot with X509 certificates not supported with FUSE_PROG build option)
endif

# Path to CST directory is required to generate the CSF header
# and prepend it to image before fip image gets generated
ifeq (${CST_DIR},)
  $(error Error: CST_DIR not set)
endif

ifeq (${FUSE_INPUT_FILE},)
FUSE_INPUT_FILE := $(PLAT_DRIVERS_PATH)/auth/csf_hdr_parser/${CSF_FILE}
endif

ifeq (${FUSE_PROV_FILE},)
else
${BUILD_PLAT}/${FUSE_PROV_FILE_SB}: ${FUSE_PROV_FILE}
	@echo " Generating CSF Header for $@ $<"
	$(CST_DIR)/create_hdr_esbc --in $< --out $@ --app_off ${CSF_HDR_SZ} \
					--app $< ${FUSE_INPUT_FILE}
endif

ifeq (${FUSE_UP_FILE},)
else
${BUILD_PLAT}/${FUSE_UP_FILE_SB}: ${FUSE_UP_FILE}
	@echo " Generating CSF Header for $@ $<"
	$(CST_DIR)/create_hdr_esbc --in $< --out $@ --app_off ${CSF_HDR_SZ} \
					--app $< ${FUSE_INPUT_FILE}
endif

endif

${BUILD_PLAT}/${FUSE_FIP_NAME}: fiptool ${FUSE_FIP_DEPS}
ifeq (${FUSE_FIP_DEPS},)
	$(error "Error: FUSE_PROV_FILE or/and FUSE_UP_FILE needs to point to the right file")
endif
	${FIPTOOL} create ${FUSE_FIP_ARGS} $@
	${FIPTOOL} info $@
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

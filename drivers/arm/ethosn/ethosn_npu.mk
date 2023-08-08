#
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Arm(R) Ethos(TM)-N NPU SiP service
ETHOSN_NPU_DRIVER			:=	0

$(eval $(call assert_boolean,ETHOSN_NPU_DRIVER))
$(eval $(call add_define,ETHOSN_NPU_DRIVER))

#Ethos-N NPU TZMP1
ETHOSN_NPU_TZMP1			:=	0
$(eval $(call assert_boolean,ETHOSN_NPU_TZMP1))
$(eval $(call add_define,ETHOSN_NPU_TZMP1))
ifeq (${ETHOSN_NPU_TZMP1},1)
  ifeq (${ETHOSN_NPU_DRIVER},0)
    $(error "ETHOSN_NPU_TZMP1 is only available if ETHOSN_NPU_DRIVER=1)
  endif
  ifeq (${PLAT},juno)
    $(eval $(call add_define,JUNO_ETHOSN_TZMP1))
  else
    $(error "ETHOSN_NPU_TZMP1 only supported on Juno platform, not ", ${PLAT})
  endif

  ifeq (${TRUSTED_BOARD_BOOT},0)
    # We rely on TRUSTED_BOARD_BOOT to prevent the firmware code from being
    # tampered with, which is required to protect the confidentiality of protected
    # inference data.
    $(error "ETHOSN_NPU_TZMP1 is only available if TRUSTED_BOARD_BOOT is enabled)
  endif

  # We need the FW certificate and key certificate
  $(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/npu_fw_key.crt,--npu-fw-key-cert))
  $(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/npu_fw_content.crt,--npu-fw-cert))
  # We need the firmware to be built into the FIP
  $(eval $(call TOOL_ADD_IMG,ETHOSN_NPU_FW,--npu-fw))

  # Needed for our OIDs to be available in tbbr_cot_bl2.c
  $(eval $(call add_define, PLAT_DEF_OID))

  # Needed so that UUIDs from the FIP are available in BL2
  $(eval $(call add_define,PLAT_DEF_FIP_UUID))

  PLAT_INCLUDES	+=	-I${PLAT_DIR}certificate/include
  PLAT_INCLUDES	+=	-Iinclude/drivers/arm/
  PLAT_INCLUDES	+=	-I${PLAT_DIR}fip
endif

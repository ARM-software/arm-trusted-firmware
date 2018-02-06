#
# Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# This file defines the keys and certificates that must be created to establish
# a Chain of Trust following the TBBR document. These definitions include the
# command line options passed to the cert_create and fiptool commands.
#
# Expected environment:
#
#   BUILD_PLAT: output directory
#   NEED_BL32: indicates whether BL32 is needed by the platform
#   BL2: image filename (optional). Default is IMG_BIN(2) (see macro IMG_BIN)
#   SCP_BL2: image filename (optional). Default is IMG_BIN(30)
#   BL31: image filename (optional). Default is IMG_BIN(31)
#   BL32: image filename (optional). Default is IMG_BIN(32)
#   BL33: image filename (optional). Default is IMG_BIN(33)
#
# Build options added by this file:
#
#   KEY_ALG
#   ROT_KEY
#   TRUSTED_WORLD_KEY
#   NON_TRUSTED_WORLD_KEY
#   SCP_BL2_KEY
#   BL31_KEY
#   BL32_KEY
#   BL33_KEY
#

# Certificate generation tool default parameters
TRUSTED_KEY_CERT	:=	${BUILD_PLAT}/trusted_key.crt
FWU_CERT		:=	${BUILD_PLAT}/fwu_cert.crt

# Default non-volatile counter values (overridable by the platform)
TFW_NVCTR_VAL		?=	0
NTFW_NVCTR_VAL		?=	0

# Pass the non-volatile counters to the cert_create tool
$(eval $(call CERT_ADD_CMD_OPT,${TFW_NVCTR_VAL},--tfw-nvctr))
$(eval $(call CERT_ADD_CMD_OPT,${NTFW_NVCTR_VAL},--ntfw-nvctr))

# Add Trusted Key certificate to the fiptool and cert_create command line options
$(eval $(call TOOL_ADD_PAYLOAD,${TRUSTED_KEY_CERT},--trusted-key-cert))

# Add fwu certificate to the fiptool and cert_create command line options
$(eval $(call TOOL_ADD_PAYLOAD,${FWU_CERT},--fwu-cert,,FWU_))

# Add the keys to the cert_create command line options (private keys are NOT
# packed in the FIP). Developers can use their own keys by specifying the proper
# build option in the command line when building the Trusted Firmware
$(if ${KEY_ALG},$(eval $(call CERT_ADD_CMD_OPT,${KEY_ALG},--key-alg)))
$(if ${HASH_ALG},$(eval $(call CERT_ADD_CMD_OPT,${HASH_ALG},--hash-alg)))
$(if ${ROT_KEY},$(eval $(call CERT_ADD_CMD_OPT,${ROT_KEY},--rot-key)))
$(if ${ROT_KEY},$(eval $(call CERT_ADD_CMD_OPT,${ROT_KEY},--rot-key,FWU_)))
$(if ${TRUSTED_WORLD_KEY},$(eval $(call CERT_ADD_CMD_OPT,${TRUSTED_WORLD_KEY},--trusted-world-key)))
$(if ${NON_TRUSTED_WORLD_KEY},$(eval $(call CERT_ADD_CMD_OPT,${NON_TRUSTED_WORLD_KEY},--non-trusted-world-key)))


# Add the BL2 CoT (image cert)
ifeq (${BL2_AT_EL3}, 0)
$(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/tb_fw.crt,--tb-fw-cert))
endif

# Add the SCP_BL2 CoT (key cert + img cert)
ifneq (${SCP_BL2},)
    $(if ${SCP_BL2_KEY},$(eval $(call CERT_ADD_CMD_OPT,${SCP_BL2_KEY},--scp-fw-key)))
    $(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/scp_fw_content.crt,--scp-fw-cert))
    $(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/scp_fw_key.crt,--scp-fw-key-cert))
endif

ifeq (${ARCH},aarch64)
ifeq (${NEED_BL31},yes)
# Add the BL31 CoT (key cert + img cert)
$(if ${BL31_KEY},$(eval $(call CERT_ADD_CMD_OPT,${BL31_KEY},--soc-fw-key)))
$(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/soc_fw_content.crt,--soc-fw-cert))
$(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/soc_fw_key.crt,--soc-fw-key-cert))
endif
endif

# Add the BL32 CoT (key cert + img cert)
ifeq (${NEED_BL32},yes)
    $(if ${BL32_KEY},$(eval $(call CERT_ADD_CMD_OPT,${BL32_KEY},--tos-fw-key)))
    $(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/tos_fw_content.crt,--tos-fw-cert))
    $(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/tos_fw_key.crt,--tos-fw-key-cert))
endif

# Add the BL33 CoT (key cert + img cert)
ifneq (${BL33},)
    $(if ${BL33_KEY},$(eval $(call CERT_ADD_CMD_OPT,${BL33_KEY},--nt-fw-key)))
    $(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/nt_fw_content.crt,--nt-fw-cert))
    $(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/nt_fw_key.crt,--nt-fw-key-cert))
endif

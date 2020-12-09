#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# This file defines the keys and certificates that must be created to establish
# a Chain of Trust for the DDR FW. These definitions include the
# command line options passed to the cert_create and fiptool commands for DDR FW.
# A DDR FW key is used for signing the DDR Firmware. The DDR key is authenticated
# by the Trusted World Key. Two content certificates are created:
# For DDR RDIMM Images [ signed by DDR FW Key]
# For DDR UDIMM Images [ signed by DDR FW Key]
#
# Expected environment:
#
#   BUILD_PLAT: output directory
#
# Build options added by this file:
#
#   KEY_ALG
#   KEY_SIZE
#   TRUSTED_WORLD_KEY
#   NON_TRUSTED_WORLD_KEY
#

# Copy the tbbr.mk from PLAT_TOOL_PATH/cert_create_helper
# to the ${PLAT_DIR}. So that cert_create is enabled
# to create certificates for DDR
$(shell cp ${PLAT_TOOL_PATH}/cert_create_helper/cert_create_tbbr.mk ${PLAT_DIR})

# Certificate generation tool default parameters
DDR_FW_CERT		:=	${BUILD_PLAT}/ddr_fw_key_cert.crt

# Default non-volatile counter values (overridable by the platform)
TFW_NVCTR_VAL		?=	0
NTFW_NVCTR_VAL		?=	0

# Pass the non-volatile counters to the cert_create tool
$(eval $(call CERT_ADD_CMD_OPT,${TFW_NVCTR_VAL},--tfw-nvctr,DDR_))

$(shell mkdir -p '${BUILD_PLAT}')

ifeq (${DDR_KEY},)
DDR_KEY=${BUILD_PLAT}/ddr.pem
endif

ifeq (${TRUSTED_KEY_CERT},)
$(info Generating: Trusted key certificate as part of DDR cert creation)
TRUSTED_KEY_CERT	:=	${BUILD_PLAT}/trusted_key.crt
$(eval $(call TOOL_ADD_PAYLOAD,${TRUSTED_KEY_CERT},--trusted-key-cert,))
$(eval $(call TOOL_ADD_PAYLOAD,${TRUSTED_KEY_CERT},--trusted-key-cert,,DDR_))
else
$(info Using: Trusted key certificate as part of DDR cert creation)
DDR_FIP_ARGS += --trusted-key-cert ${TRUSTED_KEY_CERT}
endif

# Add the keys to the cert_create command line options (private keys are NOT
# packed in the FIP). Developers can use their own keys by specifying the proper
# build option in the command line when building the Trusted Firmware
$(if ${KEY_ALG},$(eval $(call CERT_ADD_CMD_OPT,${KEY_ALG},--key-alg,DDR_)))
$(if ${KEY_SIZE},$(eval $(call CERT_ADD_CMD_OPT,${KEY_SIZE},--key-size,DDR_)))
$(if ${HASH_ALG},$(eval $(call CERT_ADD_CMD_OPT,${HASH_ALG},--hash-alg,DDR_)))
$(if ${ROT_KEY},$(eval $(call CERT_ADD_CMD_OPT,${ROT_KEY},--rot-key,DDR_)))
$(if ${TRUSTED_WORLD_KEY},$(eval $(call CERT_ADD_CMD_OPT,${TRUSTED_WORLD_KEY},--trusted-world-key,DDR_)))
$(if ${NON_TRUSTED_WORLD_KEY},$(eval $(call CERT_ADD_CMD_OPT,${NON_TRUSTED_WORLD_KEY},--non-trusted-world-key, DDR_)))

# Add the DDR CoT (key cert + img cert)
$(if ${DDR_KEY},$(eval $(call CERT_ADD_CMD_OPT,${DDR_KEY},--ddr-fw-key,DDR_)))
$(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/ddr_fw_key.crt,--ddr-fw-key-cert,,DDR_))
$(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/ddr_udimm_fw_content.crt,--ddr-udimm-fw-cert,,DDR_))
$(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/ddr_rdimm_fw_content.crt,--ddr-rdimm-fw-cert,,DDR_))

$(eval $(call TOOL_ADD_IMG,DDR_IMEM_UDIMM_1D,--ddr-immem-udimm-1d,DDR_))
$(eval $(call TOOL_ADD_IMG,DDR_IMEM_UDIMM_2D,--ddr-immem-udimm-2d,DDR_))
$(eval $(call TOOL_ADD_IMG,DDR_DMEM_UDIMM_1D,--ddr-dmmem-udimm-1d,DDR_))
$(eval $(call TOOL_ADD_IMG,DDR_DMEM_UDIMM_2D,--ddr-dmmem-udimm-2d,DDR_))

$(eval $(call TOOL_ADD_IMG,DDR_IMEM_RDIMM_1D,--ddr-immem-rdimm-1d,DDR_))
$(eval $(call TOOL_ADD_IMG,DDR_IMEM_RDIMM_2D,--ddr-immem-rdimm-2d,DDR_))
$(eval $(call TOOL_ADD_IMG,DDR_DMEM_RDIMM_1D,--ddr-dmmem-rdimm-1d,DDR_))
$(eval $(call TOOL_ADD_IMG,DDR_DMEM_RDIMM_2D,--ddr-dmmem-rdimm-2d,DDR_))

DDR_FIP_DEPS += ddr_certificates

# Process TBB related flags
ifneq (${GENERATE_COT},0)
        # Common cert_create options
        ifneq (${CREATE_KEYS},0)
                $(eval DDR_CRT_ARGS += -n)
                ifneq (${SAVE_KEYS},0)
                       $(eval DDR_CRT_ARGS += -k)
                endif
        endif
endif

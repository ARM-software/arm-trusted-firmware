#
# Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# Neither the name of ARM nor the names of its contributors may be used
# to endorse or promote products derived from this software without specific
# prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

# This file defines the keys and certificates that must be created to establish
# a Chain of Trust following the TBBR document. These definitions include the
# command line options passed to the cert_create and fip_create tools.
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

# Add Trusted Key certificate to the fip_create and cert_create command line options
$(eval $(call FIP_ADD_PAYLOAD,${TRUSTED_KEY_CERT},--trusted-key-cert))
$(eval $(call CERT_ADD_CMD_OPT,${TRUSTED_KEY_CERT},--trusted-key-cert))

# Add fwu certificate to the fip_create and cert_create command line options
$(eval $(call FWU_FIP_ADD_PAYLOAD,${FWU_CERT},--fwu-cert))
$(eval $(call FWU_CERT_ADD_CMD_OPT,${FWU_CERT},--fwu-cert))

# Add the keys to the cert_create command line options (private keys are NOT
# packed in the FIP). Developers can use their own keys by specifying the proper
# build option in the command line when building the Trusted Firmware
$(if ${KEY_ALG},$(eval $(call CERT_ADD_CMD_OPT,${KEY_ALG},--key-alg)))
$(if ${ROT_KEY},$(eval $(call CERT_ADD_CMD_OPT,${ROT_KEY},--rot-key)))
$(if ${ROT_KEY},$(eval $(call FWU_CERT_ADD_CMD_OPT,${ROT_KEY},--rot-key)))
$(if ${TRUSTED_WORLD_KEY},$(eval $(call CERT_ADD_CMD_OPT,${TRUSTED_WORLD_KEY},--trusted-world-key)))
$(if ${NON_TRUSTED_WORLD_KEY},$(eval $(call CERT_ADD_CMD_OPT,${NON_TRUSTED_WORLD_KEY},--non-trusted-world-key)))

# Add the BL2 CoT (image cert + image)
$(if ${BL2},$(eval $(call CERT_ADD_CMD_OPT,${BL2},--tb-fw,true)),\
            $(eval $(call CERT_ADD_CMD_OPT,$(call IMG_BIN,2),--tb-fw,true)))
$(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/tb_fw.crt,--tb-fw-cert))
$(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/tb_fw.crt,--tb-fw-cert))

# Add the SCP_BL2 CoT (key cert + img cert + image)
ifneq (${SCP_BL2},)
    $(eval $(call CERT_ADD_CMD_OPT,${SCP_BL2},--scp-fw,true))
    $(if ${SCP_BL2_KEY},$(eval $(call CERT_ADD_CMD_OPT,${SCP_BL2_KEY},--scp-fw-key)))
    $(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/scp_fw_content.crt,--scp-fw-cert))
    $(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/scp_fw_key.crt,--scp-fw-key-cert))
    $(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/scp_fw_content.crt,--scp-fw-cert))
    $(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/scp_fw_key.crt,--scp-fw-key-cert))
endif

# Add the BL31 CoT (key cert + img cert + image)
$(if ${BL31},$(eval $(call CERT_ADD_CMD_OPT,${BL31},--soc-fw,true)),\
             $(eval $(call CERT_ADD_CMD_OPT,$(call IMG_BIN,31),--soc-fw,true)))
$(if ${BL31_KEY},$(eval $(call CERT_ADD_CMD_OPT,${BL31_KEY},--soc-fw-key)))
$(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/soc_fw_content.crt,--soc-fw-cert))
$(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/soc_fw_key.crt,--soc-fw-key-cert))
$(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/soc_fw_content.crt,--soc-fw-cert))
$(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/soc_fw_key.crt,--soc-fw-key-cert))

# Add the BL32 CoT (key cert + img cert + image)
ifeq (${NEED_BL32},yes)
    $(if ${BL32},$(eval $(call CERT_ADD_CMD_OPT,${BL32},--tos-fw,true)),\
                 $(if ${BL32_SOURCES},$(eval $(call CERT_ADD_CMD_OPT,$(call IMG_BIN,32),--tos-fw,true))))
    $(if ${BL32_KEY},$(eval $(call CERT_ADD_CMD_OPT,${BL32_KEY},--tos-fw-key)))
    $(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/tos_fw_content.crt,--tos-fw-cert))
    $(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/tos_fw_key.crt,--tos-fw-key-cert))
    $(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/tos_fw_content.crt,--tos-fw-cert))
    $(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/tos_fw_key.crt,--tos-fw-key-cert))
endif

# Add the BL33 CoT (key cert + img cert + image)
ifneq (${BL33},)
    $(eval $(call CERT_ADD_CMD_OPT,${BL33},--nt-fw,true))
    $(if ${BL33_KEY},$(eval $(call CERT_ADD_CMD_OPT,${BL33_KEY},--nt-fw-key)))
    $(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/nt_fw_content.crt,--nt-fw-cert))
    $(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/nt_fw_key.crt,--nt-fw-key-cert))
    $(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/nt_fw_content.crt,--nt-fw-cert))
    $(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/nt_fw_key.crt,--nt-fw-key-cert))
endif

# Add the BL2U image
$(if ${BL2U},$(eval $(call FWU_CERT_ADD_CMD_OPT,${BL2U},--ap-fwu-cfg,true)),\
     $(eval $(call FWU_CERT_ADD_CMD_OPT,$(call IMG_BIN,2u),--ap-fwu-cfg,true)))

# Add the SCP_BL2U image
ifneq (${SCP_BL2U},)
    $(eval $(call FWU_CERT_ADD_CMD_OPT,${SCP_BL2U},--scp-fwu-cfg,true))
endif

# Add the NS_BL2U image
ifneq (${NS_BL2U},)
    $(eval $(call FWU_CERT_ADD_CMD_OPT,${NS_BL2U},--fwu,true))
endif

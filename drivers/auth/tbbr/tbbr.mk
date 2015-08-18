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
#   NEED_BL32: indicates whether BL3-2 is needed by the platform
#   BL2: image filename (optional). Default is IMG_BIN(2) (see macro IMG_BIN)
#   BL30: image filename (optional). Default is IMG_BIN(30)
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
#   BL30_KEY
#   BL31_KEY
#   BL32_KEY
#   BL33_KEY
#

# Certificate generation tool default parameters
TRUSTED_KEY_CERT	:=	${BUILD_PLAT}/trusted_key.crt

# Add Trusted Key certificate to the fip_create and cert_create command line options
$(eval $(call FIP_ADD_PAYLOAD,${TRUSTED_KEY_CERT},--trusted-key-cert))
$(eval $(call CERT_ADD_CMD_OPT,${TRUSTED_KEY_CERT},--trusted-key-cert))

# Add the keys to the cert_create command line options (private keys are NOT
# packed in the FIP). Developers can use their own keys by specifying the proper
# build option in the command line when building the Trusted Firmware
$(if ${KEY_ALG},$(eval $(call CERT_ADD_CMD_OPT,${KEY_ALG},--key-alg)))
$(if ${ROT_KEY},$(eval $(call CERT_ADD_CMD_OPT,${ROT_KEY},--rot-key)))
$(if ${TRUSTED_WORLD_KEY},$(eval $(call CERT_ADD_CMD_OPT,${TRUSTED_WORLD_KEY},--trusted-world-key)))
$(if ${NON_TRUSTED_WORLD_KEY},$(eval $(call CERT_ADD_CMD_OPT,${NON_TRUSTED_WORLD_KEY},--non-trusted-world-key)))

# Add the BL2 CoT (image cert + image)
$(if ${BL2},$(eval $(call CERT_ADD_CMD_OPT,${BL2},--bl2,true)),\
            $(eval $(call CERT_ADD_CMD_OPT,$(call IMG_BIN,2),--bl2,true)))
$(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/bl2.crt,--bl2-cert))
$(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/bl2.crt,--bl2-cert))

# Add the BL30 CoT (key cert + img cert + image)
ifneq (${BL30},)
    $(eval $(call CERT_ADD_CMD_OPT,${BL30},--bl30,true))
    $(if ${BL30_KEY},$(eval $(call CERT_ADD_CMD_OPT,${BL30_KEY},--bl30-key)))
    $(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/bl30.crt,--bl30-cert))
    $(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/bl30_key.crt,--bl30-key-cert))
    $(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/bl30.crt,--bl30-cert))
    $(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/bl30_key.crt,--bl30-key-cert))
endif

# Add the BL31 CoT (key cert + img cert + image)
$(if ${BL31},$(eval $(call CERT_ADD_CMD_OPT,${BL31},--bl31,true)),\
             $(eval $(call CERT_ADD_CMD_OPT,$(call IMG_BIN,31),--bl31,true)))
$(if ${BL31_KEY},$(eval $(call CERT_ADD_CMD_OPT,${BL31_KEY},--bl31-key)))
$(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/bl31.crt,--bl31-cert))
$(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/bl31_key.crt,--bl31-key-cert))
$(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/bl31.crt,--bl31-cert))
$(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/bl31_key.crt,--bl31-key-cert))

# Add the BL32 CoT (key cert + img cert + image)
ifeq (${NEED_BL32},yes)
    $(if ${BL32},$(eval $(call CERT_ADD_CMD_OPT,${BL32},--bl32,true)),\
                 $(eval $(call CERT_ADD_CMD_OPT,$(call IMG_BIN,32),--bl32,true)))
    $(if ${BL32_KEY},$(eval $(call CERT_ADD_CMD_OPT,${BL32_KEY},--bl32-key)))
    $(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/bl32.crt,--bl32-cert))
    $(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/bl32_key.crt,--bl32-key-cert))
    $(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/bl32.crt,--bl32-cert))
    $(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/bl32_key.crt,--bl32-key-cert))
endif

# Add the BL33 CoT (key cert + img cert + image)
ifneq (${BL33},)
    $(eval $(call CERT_ADD_CMD_OPT,${BL33},--bl33,true))
    $(if ${BL33_KEY},$(eval $(call CERT_ADD_CMD_OPT,${BL33_KEY},--bl33-key)))
    $(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/bl33.crt,--bl33-cert))
    $(eval $(call CERT_ADD_CMD_OPT,${BUILD_PLAT}/bl33_key.crt,--bl33-key-cert))
    $(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/bl33.crt,--bl33-cert))
    $(eval $(call FIP_ADD_PAYLOAD,${BUILD_PLAT}/bl33_key.crt,--bl33-key-cert))
endif

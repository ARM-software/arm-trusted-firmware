#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Compile time defines used by NXP platforms

PLAT_DEF_OID := yes

ifeq (${PLAT_DEF_OID},yes)

$(eval $(call add_define, PLAT_DEF_OID))
$(eval $(call add_define, PDEF_KEYS))
$(eval $(call add_define, PDEF_CERTS))
$(eval $(call add_define, PDEF_EXTS))


INC_DIR += -I../../plat/nxp/common/fip_handler/common/

PDEF_CERT_TOOL_PATH		:=	../nxp/cert_create_helper
PLAT_INCLUDE			+=	-I${PDEF_CERT_TOOL_PATH}/include

PLAT_OBJECTS			+=	${PDEF_CERT_TOOL_PATH}/src/pdef_tbb_cert.o \
					${PDEF_CERT_TOOL_PATH}/src/pdef_tbb_ext.o \
					${PDEF_CERT_TOOL_PATH}/src/pdef_tbb_key.o

$(shell rm ${PLAT_OBJECTS})

OBJECTS				+= ${PLAT_OBJECTS}
endif

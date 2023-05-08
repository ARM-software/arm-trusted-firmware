#
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_DEF_OID := 1

ifeq (${PLAT_DEF_OID},1)
  ifeq (${ETHOSN_NPU_DRIVER},1)
    $(eval $(call add_define, PLAT_DEF_OID))
    $(eval $(call add_define, PDEF_CERTS))
    $(eval $(call add_define, PDEF_EXTS))
    $(eval $(call add_define, PDEF_KEYS))

    PLAT_INCLUDE			+=	-I ${PLAT_DIR}/certificate/include \
						-I ../../include/drivers/arm

    PLAT_OBJECTS			+=	${PLAT_DIR}certificate/src/juno_tbb_cert.o \
						${PLAT_DIR}certificate/src/juno_tbb_ext.o \
						${PLAT_DIR}certificate/src/juno_tbb_key.o

    OBJECTS				+=	${PLAT_OBJECTS}
  endif
endif

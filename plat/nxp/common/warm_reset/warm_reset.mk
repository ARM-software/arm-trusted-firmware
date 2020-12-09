#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------
ifeq (${WARM_RST_ADDED},)

WARM_RST_ADDED	:=	1
NXP_NV_SW_MAINT_LAST_EXEC_DATA := yes

$(eval $(call add_define,NXP_WARM_BOOT))


WARM_RST_INCLUDES	+=	-I${PLAT_COMMON_PATH}/warm_reset
WARM_RST_BL31_SOURCES	+=	${PLAT_SOC_PATH}/$(ARCH)/${SOC}_warm_rst.S

WARM_RST_BL_COMM_SOURCES	+=	${PLAT_COMMON_PATH}/warm_reset/plat_warm_reset.c

endif

#
# Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#


# By default, SCP images are needed by CSS platforms.
CSS_LOAD_SCP_IMAGES	?=	1

# By default, SCMI driver is disabled for CSS platforms
CSS_USE_SCMI_SDS_DRIVER	?=	0

PLAT_INCLUDES		+=	-Iinclude/plat/arm/css/common/aarch64


PLAT_BL_COMMON_SOURCES	+=	plat/arm/css/common/${ARCH}/css_helpers.S

BL1_SOURCES		+=	plat/arm/css/common/css_bl1_setup.c

BL2_SOURCES		+=	plat/arm/css/common/css_bl2_setup.c

BL2U_SOURCES		+=	plat/arm/css/common/css_bl2u_setup.c

BL31_SOURCES		+=	plat/arm/css/common/css_pm.c			\
				plat/arm/css/common/css_topology.c

ifeq (${CSS_USE_SCMI_SDS_DRIVER},0)
BL31_SOURCES		+=	drivers/arm/css/mhu/css_mhu.c			\
				drivers/arm/css/scp/css_pm_scpi.c		\
				drivers/arm/css/scpi/css_scpi.c
else
BL31_SOURCES		+=	drivers/arm/css/mhu/css_mhu_doorbell.c		\
				drivers/arm/css/scmi/scmi_ap_core_proto.c	\
				drivers/arm/css/scmi/scmi_common.c		\
				drivers/arm/css/scmi/scmi_pwr_dmn_proto.c	\
				drivers/arm/css/scmi/scmi_sys_pwr_proto.c	\
				drivers/arm/css/scp/css_pm_scmi.c
endif

# Process CSS_LOAD_SCP_IMAGES flag
$(eval $(call assert_boolean,CSS_LOAD_SCP_IMAGES))
$(eval $(call add_define,CSS_LOAD_SCP_IMAGES))

ifeq (${CSS_LOAD_SCP_IMAGES},1)
  NEED_SCP_BL2 := yes
  ifneq (${TRUSTED_BOARD_BOOT},0)
    $(eval $(call TOOL_ADD_IMG,scp_bl2u,--scp-fwu-cfg,FWU_))
  endif

  ifeq (${CSS_USE_SCMI_SDS_DRIVER},1)
    BL2U_SOURCES	+=	drivers/arm/css/scp/css_sds.c			\
				drivers/arm/css/sds/sds.c

    BL2_SOURCES		+=	drivers/arm/css/scp/css_sds.c			\
				drivers/arm/css/sds/sds.c
  else
    BL2U_SOURCES	+=	drivers/arm/css/mhu/css_mhu.c			\
				drivers/arm/css/scp/css_bom_bootloader.c	\
				drivers/arm/css/scpi/css_scpi.c

    BL2_SOURCES		+=	drivers/arm/css/mhu/css_mhu.c			\
				drivers/arm/css/scp/css_bom_bootloader.c	\
				drivers/arm/css/scpi/css_scpi.c
    # Enable option to detect whether the SCP ROM firmware in use predates version
    # 1.7.0 and therefore, is incompatible.
    CSS_DETECT_PRE_1_7_0_SCP	:=	1

    # Process CSS_DETECT_PRE_1_7_0_SCP flag
    $(eval $(call assert_boolean,CSS_DETECT_PRE_1_7_0_SCP))
    $(eval $(call add_define,CSS_DETECT_PRE_1_7_0_SCP))
  endif
endif

ifeq (${CSS_USE_SCMI_SDS_DRIVER},1)
  PLAT_BL_COMMON_SOURCES	+=	drivers/arm/css/sds/${ARCH}/sds_helpers.S
endif

# Process CSS_USE_SCMI_SDS_DRIVER flag
$(eval $(call assert_boolean,CSS_USE_SCMI_SDS_DRIVER))
$(eval $(call add_define,CSS_USE_SCMI_SDS_DRIVER))

# Process CSS_NON_SECURE_UART flag
# This undocumented build option is only to enable debug access to the UART
# from non secure code, which is useful on some platforms.
# Default (obviously) is off.
CSS_NON_SECURE_UART		:= 0
$(eval $(call assert_boolean,CSS_NON_SECURE_UART))
$(eval $(call add_define,CSS_NON_SECURE_UART))


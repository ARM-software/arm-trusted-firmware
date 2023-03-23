#
# Copyright (c) 2023, Advanced Micro Devices, Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LIBPM_SRCS	:=	$(addprefix plat/xilinx/common/pm_service/,	\
			pm_ipi.c)

LIBPM_SRCS      +=      $(addprefix plat/xilinx/zynqmp/pm_service/,  \
                        pm_svc_main.c 					\
			pm_api_sys.c					\
			pm_api_pinctrl.c				\
			pm_api_ioctl.c					\
			pm_api_clock.c					\
			pm_client.c)

$(eval $(call MAKE_LIB,pm))

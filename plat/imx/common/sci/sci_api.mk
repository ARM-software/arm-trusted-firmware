#
# Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL31_SOURCES	+=	plat/imx/common/sci/ipc.c			\
			plat/imx/common/sci/imx8_mu.c			\
			plat/imx/common/sci/svc/pad/pad_rpc_clnt.c	\
			plat/imx/common/sci/svc/pm/pm_rpc_clnt.c	\
			plat/imx/common/sci/svc/rm/rm_rpc_clnt.c	\
			plat/imx/common/sci/svc/timer/timer_rpc_clnt.c	\
			plat/imx/common/sci/svc/misc/misc_rpc_clnt.c

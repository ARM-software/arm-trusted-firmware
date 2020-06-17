#
# Copyright (C) 2018 Marvell International Ltd.
#
# SPDX-License-Identifier:     BSD-3-Clause
# https://spdx.org/licenses
#


PLAT_MARVELL		:= 	plat/marvell/armada
MSS_SOURCE		:= 	$(PLAT_MARVELL)/common/mss

BL2_SOURCES		+=	$(MSS_SOURCE)/mss_scp_bootloader.c		\
				$(PLAT_MARVELL)/common/plat_delay_timer.c	\
				drivers/delay_timer/delay_timer.c		\
				$(MARVELL_DRV)					\
				$(BOARD_DIR)/board/marvell_plat_config.c

BL31_SOURCES		+=	$(MSS_SOURCE)/mss_ipc_drv.c

PLAT_INCLUDES           +=      -I$(MSS_SOURCE)

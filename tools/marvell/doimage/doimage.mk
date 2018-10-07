#
# Copyright (C) 2018 Marvell International Ltd.
#
# SPDX-License-Identifier:     BSD-3-Clause
# https://spdx.org/licenses

DOIMAGE_FLAGS		?= 	-l 0x4100000 -e 0x4100000


#NAND params
#Open and update the below when using NAND as a boot device.

CONFIG_MVEBU_NAND_BLOCK_SIZE	:= 256
CONFIG_MVEBU_NAND_CELL_TYPE	:= SLC
NAND_DOIMAGE_FLAGS := -t $(CONFIG_MVEBU_NAND_CELL_TYPE) -n $(CONFIG_MVEBU_NAND_BLOCK_SIZE)

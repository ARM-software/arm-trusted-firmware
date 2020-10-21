# Copyright (C) 2018 Marvell International Ltd.
#
# SPDX-License-Identifier:     BSD-3-Clause
# https://spdx.org/licenses

# Marvell images
BOOT_IMAGE			:= boot-image.bin
BOOT_ENC_IMAGE			:= boot-image-enc.bin
FLASH_IMAGE			:= flash-image.bin

# Make non-trusted image by default
MARVELL_SECURE_BOOT		:= 0
$(eval $(call add_define,MARVELL_SECURE_BOOT))

# Enable compilation for Palladium emulation platform
PALLADIUM			:= 0
$(eval $(call add_define,PALLADIUM))

# Set board to work with DDR 32bit
DDR32				:= 0
$(eval $(call add_define,DDR32))

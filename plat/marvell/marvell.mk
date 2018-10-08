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

ifeq (${MARVELL_SECURE_BOOT},1)
DOIMAGE_SEC_FLAGS := -c $(DOIMAGE_SEC)
DOIMAGE_LIBS_CHECK = \
        if ! [ -d "/usr/include/mbedtls" ]; then \
                        echo "****************************************" >&2; \
                        echo "Missing mbedTLS installation! " >&2; \
                        echo "Please download it from \"tls.mbed.org\"" >&2; \
			echo "Alternatively on Debian/Ubuntu system install" >&2; \
			echo "\"libmbedtls-dev\" package" >&2; \
                        echo "Make sure to use version 2.1.0 or later" >&2; \
                        echo "****************************************" >&2; \
                exit 1; \
        else if ! [ -f "/usr/include/libconfig.h" ]; then \
                        echo "********************************************************" >&2; \
                        echo "Missing Libconfig installation!" >&2; \
                        echo "Please download it from \"www.hyperrealm.com/libconfig/\"" >&2; \
                        echo "Alternatively on Debian/Ubuntu system install packages" >&2; \
                        echo "\"libconfig8\" and \"libconfig8-dev\"" >&2; \
                        echo "********************************************************" >&2; \
                exit 1; \
        fi \
        fi
else #MARVELL_SECURE_BOOT
DOIMAGE_LIBS_CHECK =
DOIMAGE_SEC_FLAGS =
endif #MARVELL_SECURE_BOOT

mrvl_clean:
	@echo "  Doimage CLEAN"
	${Q}${MAKE} PLAT=${PLAT} --no-print-directory -C ${DOIMAGEPATH} clean

${DOIMAGETOOL}: mrvl_clean
	@$(DOIMAGE_LIBS_CHECK)
	${Q}${MAKE} --no-print-directory -C ${DOIMAGEPATH} VERSION=$(SUBVERSION) WTMI_IMG=$(WTMI_IMG)



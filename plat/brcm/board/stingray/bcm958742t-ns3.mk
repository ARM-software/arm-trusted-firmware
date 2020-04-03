#
# Copyright (c) 2015 - 2020, Broadcom
#
# SPDX-License-Identifier: BSD-3-Clause
#

#######################################################
# Board config file for bcm958742t-ns3 Stingray SST100-NS3
#######################################################

include plat/brcm/board/stingray/bcm958742t.mk

# Load BL33 at 0xFF00_0000 address
ifneq (${BL33_OVERRIDE_LOAD_ADDR},)
$(eval $(call add_define_val,BL33_OVERRIDE_LOAD_ADDR,0xFF000000))
endif

# Nitro DDR secure memory
# Nitro FW and config 0x8AE00000 - 0x8B000000
# Nitro Crash dump 0x8B000000 - 0x8D000000
DDR_NITRO_SECURE_REGION_START := 0x8AE00000
DDR_NITRO_SECURE_REGION_END := 0x8D000000

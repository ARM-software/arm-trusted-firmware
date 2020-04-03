#
# Copyright (c) 2015 - 2020, Broadcom
#
# SPDX-License-Identifier: BSD-3-Clause
#

#######################################################
# Board config file for bcm958742t Stingray SST100
#######################################################
BOARD_FAMILY := "<bcm958742t.h>"
$(eval $(call add_define,BOARD_FAMILY))

# Board has internal programmable regulator
IHOST_REG_TYPE := IHOST_REG_INTEGRATED
$(eval $(call add_define,IHOST_REG_TYPE))

# Board has internal programmable regulator
VDDC_REG_TYPE := VDDC_REG_INTEGRATED
$(eval $(call add_define,VDDC_REG_TYPE))

#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#
#-----------------------------------------------------------------------------

# SNPS ddr phy driver files

DDR_PHY_C  =
DDR_PHY_H  =

$(DDR_PHY_C): $(DDR_PHY_H) $(COMMON_HDRS) src
	@cp -r "$(DDR_PHY_PATH)/$@" "$(SRC_DIR)/$@"

$(DDR_PHY_H): src
	@cp -r "$(DDR_PHY_PATH)/$@" "$(SRC_DIR)/$@"

#------------------------------------------------

#
# Copyright 2018-2019 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------

  # SFP driver files
ifeq ($(fuse), on)
  PLATFLAGS +=-DCNFG_FUSE=1
  SFP_H =  $(wildcard  $(DRVR_SRC)/sfp/include/*.h)
  SFP_H := $(notdir $(SFP_H))
  SFP_C =  $(wildcard  $(DRVR_SRC)/sfp/src/*.c)
  SFP_C := $(notdir $(SFP_C))
else
  SFP_H =
  SFP_C =
endif

$(SFP_C): $(SFP_H) $(COMMON_HDRS) src
	@cp -r "$(DRVR_SRC)/sfp/src/$@" "$(SRC_DIR)/$@"

$(SFP_H): src
	@cp -r "$(DRVR_SRC)/sfp/include/$@" "$(SRC_DIR)/$@"

BUILD_C   += $(SFP_C)
HDRS_ASM  += $(SFP_H)

#------------------------------------------------

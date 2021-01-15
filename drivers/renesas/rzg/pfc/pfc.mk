#
# Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${RCAR_LSI},${RCAR_AUTO})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2M/pfc_init_g2m.c

else ifdef RCAR_LSI_CUT_COMPAT
  ifeq (${RCAR_LSI},${RZ_G2M})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2M/pfc_init_g2m.c
  endif
else
  ifeq (${RCAR_LSI},${RZ_G2M})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2M/pfc_init_g2m.c
  endif
endif

BL2_SOURCES += drivers/renesas/rzg/pfc/pfc_init.c

#
# Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${RCAR_LSI},${RCAR_AUTO})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2M/pfc_init_g2m.c
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2H/pfc_init_g2h.c
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2N/pfc_init_g2n.c
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2E/pfc_init_g2e.c

else ifdef RCAR_LSI_CUT_COMPAT
  ifeq (${RCAR_LSI},${RZ_G2M})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2M/pfc_init_g2m.c
  endif
  ifeq (${RCAR_LSI},${RZ_G2H})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2H/pfc_init_g2h.c
  endif
  ifeq (${RCAR_LSI},${RZ_G2N})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2N/pfc_init_g2n.c
  endif
  ifeq (${RCAR_LSI},${RZ_G2E})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2E/pfc_init_g2e.c
  endif
else
  ifeq (${RCAR_LSI},${RZ_G2M})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2M/pfc_init_g2m.c
  endif
  ifeq (${RCAR_LSI},${RZ_G2H})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2H/pfc_init_g2h.c
  endif
  ifeq (${RCAR_LSI},${RZ_G2N})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2N/pfc_init_g2n.c
  endif
  ifeq (${RCAR_LSI},${RZ_G2E})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2E/pfc_init_g2e.c
  endif
endif

BL2_SOURCES += drivers/renesas/rzg/pfc/pfc_init.c

#
# Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${RCAR_LSI},${RCAR_AUTO})
    BL2_SOURCES += drivers/renesas/rzg/qos/G2M/qos_init_g2m_v10.c
    BL2_SOURCES += drivers/renesas/rzg/qos/G2M/qos_init_g2m_v11.c
    BL2_SOURCES += drivers/renesas/rzg/qos/G2M/qos_init_g2m_v30.c
else ifeq (${RCAR_LSI_CUT_COMPAT},1)
  ifeq (${RCAR_LSI},${RZ_G2M})
    BL2_SOURCES += drivers/renesas/rzg/qos/G2M/qos_init_g2m_v10.c
    BL2_SOURCES += drivers/renesas/rzg/qos/G2M/qos_init_g2m_v11.c
    BL2_SOURCES += drivers/renesas/rzg/qos/G2M/qos_init_g2m_v30.c
  endif
else
  ifeq (${RCAR_LSI},${RZ_G2M})
    ifeq (${LSI_CUT},10)
     BL2_SOURCES += drivers/renesas/rzg/qos/G2M/qos_init_g2m_v10.c
    else ifeq (${LSI_CUT},11)
     BL2_SOURCES += drivers/renesas/rzg/qos/G2M/qos_init_g2m_v11.c
    else ifeq (${LSI_CUT},13)
     BL2_SOURCES += drivers/renesas/rzg/qos/G2M/qos_init_g2m_v11.c
    else ifeq (${LSI_CUT},30)
     BL2_SOURCES += drivers/renesas/rzg/qos/G2M/qos_init_g2m_v30.c
    else
#    LSI_CUT 30 or later
     BL2_SOURCES += drivers/renesas/rzg/qos/G2M/qos_init_g2m_v30.c
    endif
  endif
endif

BL2_SOURCES += drivers/renesas/rzg/qos/qos_init.c

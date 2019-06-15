#
# Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${RCAR_LSI},${RCAR_AUTO})
#   E3, H3N not available for LSI_AUTO
    BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3_v10.c
    BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3_v11.c
    BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3_v20.c
    BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3_v30.c
    BL2_SOURCES += drivers/renesas/rcar/qos/M3/qos_init_m3_v10.c
    BL2_SOURCES += drivers/renesas/rcar/qos/M3/qos_init_m3_v11.c
    BL2_SOURCES += drivers/renesas/rcar/qos/M3/qos_init_m3_v30.c
    BL2_SOURCES += drivers/renesas/rcar/qos/M3N/qos_init_m3n_v10.c
    BL2_SOURCES += drivers/renesas/rcar/qos/V3M/qos_init_v3m.c
else ifdef RCAR_LSI_CUT_COMPAT
  ifeq (${RCAR_LSI},${RCAR_H3})
    BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3_v10.c
    BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3_v11.c
    BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3_v20.c
    BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3_v30.c
  endif
  ifeq (${RCAR_LSI},${RCAR_H3N})
    BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3n_v30.c
  endif
  ifeq (${RCAR_LSI},${RCAR_M3})
    BL2_SOURCES += drivers/renesas/rcar/qos/M3/qos_init_m3_v10.c
    BL2_SOURCES += drivers/renesas/rcar/qos/M3/qos_init_m3_v11.c
    BL2_SOURCES += drivers/renesas/rcar/qos/M3/qos_init_m3_v30.c
  endif
  ifeq (${RCAR_LSI},${RCAR_M3N})
    BL2_SOURCES += drivers/renesas/rcar/qos/M3N/qos_init_m3n_v10.c
  endif
  ifeq (${RCAR_LSI},${RCAR_V3M})
    BL2_SOURCES += drivers/renesas/rcar/qos/V3M/qos_init_v3m.c
  endif
  ifeq (${RCAR_LSI},${RCAR_E3})
    BL2_SOURCES += drivers/renesas/rcar/qos/E3/qos_init_e3_v10.c
  endif
  ifeq (${RCAR_LSI},${RCAR_D3})
    BL2_SOURCES += drivers/renesas/rcar/qos/D3/qos_init_d3.c
  endif
else
  ifeq (${RCAR_LSI},${RCAR_H3})
    ifeq (${LSI_CUT},10)
      BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3_v10.c
    else ifeq (${LSI_CUT},11)
      BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3_v11.c
    else ifeq (${LSI_CUT},20)
      BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3_v20.c
    else ifeq (${LSI_CUT},30)
      BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3_v30.c
    else
#     LSI_CUT 30 or later
      BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3_v30.c
    endif
  endif
  ifeq (${RCAR_LSI},${RCAR_H3N})
    ifeq (${LSI_CUT},30)
      BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3n_v30.c
    else
#     LSI_CUT 30 or later
      BL2_SOURCES += drivers/renesas/rcar/qos/H3/qos_init_h3n_v30.c
    endif
  endif
  ifeq (${RCAR_LSI},${RCAR_M3})
    ifeq (${LSI_CUT},10)
     BL2_SOURCES += drivers/renesas/rcar/qos/M3/qos_init_m3_v10.c
    else ifeq (${LSI_CUT},11)
     BL2_SOURCES += drivers/renesas/rcar/qos/M3/qos_init_m3_v11.c
    else ifeq (${LSI_CUT},13)
     BL2_SOURCES += drivers/renesas/rcar/qos/M3/qos_init_m3_v11.c
    else ifeq (${LSI_CUT},30)
     BL2_SOURCES += drivers/renesas/rcar/qos/M3/qos_init_m3_v30.c
    else
#    LSI_CUT 30 or later
     BL2_SOURCES += drivers/renesas/rcar/qos/M3/qos_init_m3_v30.c
    endif
  endif
  ifeq (${RCAR_LSI},${RCAR_M3N})
    ifeq (${LSI_CUT},10)
     BL2_SOURCES += drivers/renesas/rcar/qos/M3N/qos_init_m3n_v10.c
    else
#    LSI_CUT 10 or later
     BL2_SOURCES += drivers/renesas/rcar/qos/M3N/qos_init_m3n_v10.c
    endif
  endif
  ifeq (${RCAR_LSI},${RCAR_V3M})
    BL2_SOURCES += drivers/renesas/rcar/qos/V3M/qos_init_v3m.c
  endif
  ifeq (${RCAR_LSI},${RCAR_E3})
    ifeq (${LSI_CUT},10)
     BL2_SOURCES += drivers/renesas/rcar/qos/E3/qos_init_e3_v10.c
    else
#    LSI_CUT 10 or later
     BL2_SOURCES += drivers/renesas/rcar/qos/E3/qos_init_e3_v10.c
    endif
  endif
  ifeq (${RCAR_LSI},${RCAR_D3})
    BL2_SOURCES += drivers/renesas/rcar/qos/E3/qos_init_d3.c
  endif
endif

BL2_SOURCES += drivers/renesas/rcar/qos/qos_init.c

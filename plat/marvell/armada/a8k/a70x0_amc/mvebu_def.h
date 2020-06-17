/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef MVEBU_DEF_H
#define MVEBU_DEF_H

#include <a8k_plat_def.h>

#define CP_COUNT		1	/* A70x0 has single CP0 */

/***********************************************************************
 * Required platform porting definitions common to all
 * Management Compute SubSystems (MSS)
 ***********************************************************************
 */
/*
 * Load address of SCP_BL2
 * SCP_BL2 is loaded to the same place as BL31.
 * Once SCP_BL2 is transferred to the SCP,
 * it is discarded and BL31 is loaded over the top.
 */
#ifdef SCP_IMAGE
#define SCP_BL2_BASE		BL31_BASE
#endif


#endif /* MVEBU_DEF_H */

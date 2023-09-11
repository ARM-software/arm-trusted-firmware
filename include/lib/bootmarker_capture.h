/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOOTMARKER_CAPTURE_H
#define BOOTMARKER_CAPTURE_H

#define BL1_ENTRY	U(0)
#define BL1_EXIT	U(1)
#define BL2_ENTRY	U(2)
#define BL2_EXIT	U(3)
#define BL31_ENTRY	U(4)
#define BL31_EXIT	U(5)
#define BL_TOTAL_IDS	U(6)

#ifdef __ASSEMBLER__
PMF_DECLARE_CAPTURE_TIMESTAMP(bl_svc)
#endif  /*__ASSEMBLER__*/

#endif  /*BOOTMARKER_CAPTURE_H*/

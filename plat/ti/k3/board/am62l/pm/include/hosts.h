/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef AM62X_HOSTS_H
#define AM62X_HOSTS_H

/** TIFS(Secure): Device Management and Security Control */
#define HOST_ID_TIFS (0U)
/** DM(Non Secure): Device Management */
#define HOST_ID_DM (254U)
/** MAIN_0_R5_0(Secure): Cortex R5_0 context 0 on Main island(BOOT) */
#define HOST_ID_MAIN_0_R5_0 (35U)
/** MAIN_0_R5_1(Non Secure): Cortex R5_0 context 1 on Main island */
#define HOST_ID_MAIN_0_R5_1 (36U)
/** MAIN_0_R5_2(Secure): Cortex R5_0 context 2 on Main island */
#define HOST_ID_MAIN_0_R5_2 (37U)
/** MAIN_0_R5_3(Non Secure): Cortex R5_0 context 3 on Main island */
#define HOST_ID_MAIN_0_R5_3 (38U)
/** A53_0(Secure): Cortex a53 context 0 on Main island */
#define HOST_ID_A53_0 (10U)
/** A53_1(Secure): Cortex A53 context 1 on Main island */
#define HOST_ID_A53_1 (11U)
/** A53_2(Non Secure): Cortex A53 context 2 on Main island */
#define HOST_ID_A53_2 (12U)
/** A53_3(Non Secure): Cortex A53 context 3 on Main island */
#define HOST_ID_A53_3 (13U)
/** M4_0(Non Secure): M4 */
#define HOST_ID_M4_0 (30U)
/** GPU(Non Secure): GPU context 0 on Main island */
#define HOST_ID_GPU (31U)
/** A53_4(Non Secure): Cortex A53 context 1 on Main island */
#define HOST_ID_A53_4 (14U)
/** DM2TIFS(Secure): DM to TIFS communication */
#define HOST_ID_DM2TIFS (250U)
/** TIFS2DM(Non Secure): TIFS to DM communication */
#define HOST_ID_TIFS2DM (251U)
/** HSM(Secure): HSM Controller */
#define HOST_ID_HSM (253U)

/**
 * Host catch all. Used in board configuration resource assignments to define
 * resource ranges usable by all hosts. Cannot be used
 */
#define HOST_ID_ALL (128U)

/** Number of unique hosts on the SoC */
#define HOST_ID_CNT (16U)

#endif /* AM62X_HOSTS_H */

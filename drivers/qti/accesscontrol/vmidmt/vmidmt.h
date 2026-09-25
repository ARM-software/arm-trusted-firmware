/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VMIDMT_H
#define VMIDMT_H

#include <stdbool.h>

#define ACC_ERR_VMIDMT_CFG_FAIL 1

#define ACC_VMID_NOACCESS 0
#define ACC_VMID_VMID_0 0
#define ACC_VMID_RPM 1
#define ACC_VMID_TZ 2
#define ACC_VMID_AP 3
#define ACC_VMID_MSS 4
#define ACC_VMID_LPASS 5
#define ACC_VMID_CP 6
#define ACC_VMID_VIDEO 7
#define ACC_VMID_ZAP_SHADER 8
#define ACC_VMID_MDSS 9
/* Space for VMID 10 */
#define ACC_VMID_SSC 11

/* IPA VMIDs (used to generate SIDs) */
#define ACC_VMID_IPA_AP (0b1100) /* 12 */
#define ACC_VMID_IPA_UC_PIPE (0b1101) /* 13 */
#define ACC_VMID_IPA_WIFI ACC_VMID_IPA_UC_PIPE
#define ACC_VMID_IPA_UC (0b1110) /* 14 */
#define ACC_VMID_IPA_GP (0b0000) /* 0  */
#define ACC_VMID_IPA_FW (0b0010) /* 2  */
#define ACC_VMID_IPA_PERIPH_1 (0b0001) /* 1  */
#define ACC_VMID_IPA_PERIPH_2 (0b0011) /* 3  */
#define ACC_VMID_IPA_PERIPH_3 (0b0100) /* 4  */
#define ACC_VMID_IPA_PERIPH_4 (0b0101) /* 5  */

/*
 * 9x35/9x45 IPA VMID behavior:
 * IPA u_c handles power collapse save/restore. Without XPU/VMIDMT retention,
 * u_c firmware may reprogram VMIDMT. Security requires IPA HW to force VMID
 * bit 4 high so it cannot use regular system VMIDs.
 */
#define ACC_VMID_IPA_AP_MDM 0x13 /* BIT 4 SET | ACC_VMID_AP  */
#define ACC_VMID_IPA_UC_PIPE_MDM 0x14 /* BIT 4 SET | ACC_VMID_MSS */
#define ACC_VMID_IPA_UC_MDM 0x15 /* BIT 4 SET & new VMID     */

#define ACC_VMID_WLAN 15

/*
 */
#define ACC_VMID_AP_GSI 0x16
#define ACC_VMID_QUP_TZ 0x0

#define ACC_VMID_NOACCESS_BIT (1U << ACC_VMID_NOACCESS)
#define ACC_VMID_VMID_0_BIT (1U << ACC_VMID_VMID_0)
#define ACC_VMID_TZ_BIT (1U << ACC_VMID_TZ)
#define ACC_VMID_RPM_BIT (1U << ACC_VMID_RPM)
#define ACC_VMID_LPASS_BIT (1U << ACC_VMID_LPASS)
#define ACC_VMID_MSS_BIT (1U << ACC_VMID_MSS)
#define ACC_VMID_AP_BIT (1U << ACC_VMID_AP)
#define ACC_VMID_CP_BIT (1U << ACC_VMID_CP)
#define ACC_VMID_VIDEO_BIT (1U << ACC_VMID_VIDEO)
#define ACC_VMID_MDSS_BIT (1U << ACC_VMID_MDSS)
#define ACC_VMID_SSC_BIT (1U << ACC_VMID_SSC)

#define ACC_VMID_IPA_AP_BIT (1U << ACC_VMID_IPA_AP)
#define ACC_VMID_IPA_UC_PIPE_BIT (1U << ACC_VMID_IPA_UC_PIPE)
#define ACC_VMID_IPA_UC_BIT (1U << ACC_VMID_IPA_UC)

#define ACC_VMID_IPA_AP_MDM_BIT (1U << ACC_VMID_IPA_AP_MDM)
#define ACC_VMID_IPA_UC_PIPE_MDM_BIT (1U << ACC_VMID_IPA_UC_PIPE_MDM)
#define ACC_VMID_IPA_UC_MDM_BIT (1U << ACC_VMID_IPA_UC_MDM)

#define ACC_VMID_WLAN_BIT (1U << ACC_VMID_WLAN)
#define ACC_VMID_ZAP_SHADER_BIT (1U << ACC_VMID_ZAP_SHADER)
#define ACC_VMID_IPA_WIFI_BIT (1U << ACC_VMID_IPA_WIFI)

/* All VMIDs except NOACCESS, with domain bits masked out */
#define ACC_ALL_VMID ((~MSM_VMID_NOACCESS) & (~MSM_DOMAIN_MASK))

/* Domain definitions. Start from 31 downward to avoid VMID conflicts. */
#define ACC_DOMAIN_MASK 0xFF000000 /* Reserve 8 bits for domains */
#define ACC_DOMAIN_MSA 31
#define ACC_DOMAIN_HYP 30
#define ACC_DOMAIN_SP 29
#define ACC_DOMAIN_TZ 28

/* Domain bit masks */
#define ACC_DOMAIN_MSA_BIT (1U << ACC_DOMAIN_MSA)
#define ACC_DOMAIN_HYP_BIT (1U << ACC_DOMAIN_HYP)
#define ACC_DOMAIN_SP_BIT (1U << ACC_DOMAIN_SP)
#define ACC_DOMAIN_TZ_BIT (1U << ACC_DOMAIN_TZ)

int vmidmt_configure(void);

#endif /* VMIDMT_H */

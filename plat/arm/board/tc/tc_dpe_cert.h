/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TC_DPE_CERT_H
#define TC_DPE_CERT_H

/*
 * The certificate structure on the TC platform:
 *   - The arrows indicate the parent/child relationships (who loads who).
 *   - The boxes indicate the certificates.
 *
 *                                                                  AP FW Cert.
 *                                                      +--------------------------------+
 *                                                      |                                |
 *                             Plat Cert.               |                +->SPx          |           Hyper Cert.
 *                     +--------------------------+     |                +->SP1          |     +--------------------+
 *     RoT Cert.       |                          |     |                +->TOS_FW_CONF  |     |                    |
 *  +------------+     |      +->SCP_BL1    +-----+-----+-->FW_CONF      +->AP_BL32      |     |     +->PVMFW       |
 *  |            |     |      |             |     |     |                |               |     |     |              |
 *  | RSE_BL1_2--+-----+-->RSE_BL2------->AP_BL1--+-----+------------->AP_BL2------------+-----+-->AP_BL33          |
 *  |            |     |      |             |     |     |                |               |     |     |              |
 *  +------------+     |      +->RSE_S      +-----+-----+-->TB_FW_CONF   +->AP_BL31      |     |     +->HYPERVISOR  |
 *                     |      +->RSE_NS           |     |                +->SCP_BL2      |     |                    |
 *                     |                          |     |                +->HW_CONF      |     |                    |
 *                     +--------------------------+     |                +---------------+-----+-->NT_FW_CONF       |
 *                                                      |                                |     |                    |
 *                                                      +--------------------------------+     +--------------------+
 */

#define DPE_AP_FW_CERT_ID		0x300 /* Includes: FW_CONF - SP1 */
#define DPE_HYPERVISOR_CERT_ID		0x400 /* Includes: AP_BL33 - PVMFW */

/* Common definition */
#define DPE_CERT_ID_SAME_AS_PARENT	0xFFFFFFFF

#endif /* TC_DPE_CERT_H */

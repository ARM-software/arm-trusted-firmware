/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TC_DPE_H
#define TC_DPE_H

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

/*
 * Target Locality:
 *    The goal is to specify that a certain component is expected to run and
 *    thereby send DPE commands from a given security domain. RSE is capable of
 *    of distinguishing the client's locality based on the MHU channel used for
 *    communication.
 *    Defines here must match with RSE side:
 */
#define LOCALITY_NONE		-1
/* #define LOCALITY_RSE_S	0 */  /* Not applicable on AP side */
/* #define LOCALITY_RSE_NS	1 */  /* Not applicable on AP side */
#define LOCALITY_AP_S		 2
#define LOCALITY_AP_NS		 3

#endif /* TC_DPE_H */

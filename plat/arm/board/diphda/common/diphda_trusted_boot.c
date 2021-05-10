/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

/*
 * Return the ROTPK hash in the following ASN.1 structure in DER format:
 *
 * AlgorithmIdentifier  ::=  SEQUENCE  {
 *     algorithm         OBJECT IDENTIFIER,
 *     parameters        ANY DEFINED BY algorithm OPTIONAL
 * }
 *
 * DigestInfo ::= SEQUENCE {
 *     digestAlgorithm   AlgorithmIdentifier,
 *     digest            OCTET STRING
 * }
 *
 * The function returns 0 on success. Any other value is treated as error by the
 * Trusted Board Boot. The function also reports extra information related
 * to the ROTPK in the flags parameter: ROTPK_IS_HASH, ROTPK_NOT_DEPLOYED.
 *
 * Refer to the TF-A porting-guide document for more details.
 */
int plat_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
	return arm_get_rotpk_info(cookie, key_ptr, key_len, flags);
}

/*
 * STUB overriding the non-volatile counter reading.
 * NV counters are not implemented at this stage of development.
 * Return: 0 = success
 */
int plat_get_nv_ctr(void *cookie, unsigned int *nv_ctr)
{
    *nv_ctr = DIPHDA_FW_NVCTR_VAL;
    return 0;
}

/*
 * STUB overriding the non-volatile counter updating.
 * NV counters are not implemented at this stage of development.
 * Return: 0 = success
 */
int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr)
{
    return 0;
}

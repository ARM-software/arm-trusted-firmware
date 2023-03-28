/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CCA_ATTESTATION_H
#define CCA_ATTESTATION_H

#include <stdint.h>
#include <psa/crypto_types.h>

psa_status_t
cca_attestation_get_realm_key(uintptr_t buf, size_t *len, unsigned int type);

psa_status_t
cca_attestation_get_plat_token(uintptr_t buf, size_t *len,
			       uintptr_t hash, size_t hash_size);

#endif /* CCA_ATTESTATION_H */

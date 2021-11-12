/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SME_H
#define SME_H

#include <stdbool.h>

#include <context.h>

/*
 * Maximum value of LEN field in SMCR_ELx. This is different than the maximum
 * supported value which is platform dependent. In the first version of SME the
 * LEN field is limited to 4 bits but will be expanded in future iterations.
 * To support different versions, the code that discovers the supported vector
 * lengths will write the max value into SMCR_ELx then read it back to see how
 * many bits are implemented.
 */
#define SME_SMCR_LEN_MAX	U(0x1FF)

void sme_enable(cpu_context_t *context);
void sme_disable(cpu_context_t *context);

#endif /* SME_H */

/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sq_common.h>
#include "sq_scpi.h"

/*
 * Helper function to get dram information from SCP.
 */
uint32_t sq_scp_get_draminfo(struct draminfo *info)
{
#if SQ_USE_SCMI_DRIVER
	sq_scmi_get_draminfo(info);
#else
	scpi_get_draminfo(info);
#endif
	return 0;
}

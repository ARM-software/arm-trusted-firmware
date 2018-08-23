/********************************************************************
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/

#include <platform.h>
#include <platform_def.h>

u_register_t plat_get_stack_protector_canary(void)
{
	/* Read a random data populated by previous boot loader image. */
	return *(volatile u_register_t *) (QTI_SHARED_IMEM_TF_STACK_CANARY_ADDR);
}

/*
 * Copyright (c) 2014-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <platform_def.h>
#include <common/debug.h>
#include <drivers/arm/tzc400.h>
#include <plat/arm/common/plat_arm.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/interrupt_mgmt/interrupt_mgmt.h>

#pragma weak plat_arm_security_setup
#pragma weak tzc400_interrupt_handler

void tzc400_interrupt_handler(uint32_t id, uint32_t flags, void *handle, void *cookie)
{
    uint32_t status;
    status = tzc400_get_int_status(PLAT_ARM_TZC_BASE);
    tzc400_clear_int_status(PLAT_ARM_TZC_BASE, status);
    ERROR("TZC400: Security violation detected!\n");
    ERROR("Status: 0x%x\n", status);
    uint32_t fail_addr = tzc400_get_fail_address(PLAT_ARM_TZC_BASE);
    uint32_t fail_id = tzc400_get_fail_id(PLAT_ARM_TZC_BASE);
    ERROR("Failure address: 0x%x, ID: 0x%x\n", fail_addr, fail_id);
}

/*******************************************************************************
 * Initialize the TrustZone Controller for ARM standard platforms.
 * When booting an EL3 payload, this is simplified: we configure region 0 with
 * secure access only and do not enable any other region.
 ******************************************************************************/

void arm_tzc400_setup(uintptr_t tzc_base,
            const arm_tzc_regions_info_t *tzc_regions)
{
#ifndef EL3_PAYLOAD_BASE
    unsigned int region_index = 1U;
    const arm_tzc_regions_info_t *p;
    const arm_tzc_regions_info_t init_tzc_regions[] = {
        ARM_TZC_REGIONS_DEF,
        {0}
    };
#endif
    INFO("Configuring TrustZone Controller\n");
    tzc400_init(tzc_base);
    tzc400_disable_filters();
#ifndef EL3_PAYLOAD_BASE
    if (tzc_regions == NULL)
        p = init_tzc_regions;
    else
        p = tzc_regions;
    tzc400_configure_region0(TZC_REGION_S_NONE, 0);
    for (; p->base != 0ULL; p++) {
        tzc400_configure_region(PLAT_ARM_TZC_FILTERS, region_index,
            p->base, p->end, p->sec_attr, p->nsaid_permissions);
        region_index++;
    }
    INFO("Total %u regions set.\n", region_index);
#else
    tzc400_configure_region0(TZC_REGION_S_RDWR, PLAT_ARM_TZC_NS_DEV_ACCESS);
#endif

    uint32_t flags = 0;
    SET_SECURITY_STATE(flags, SECURE);
    interrupt_handler_register(PLAT_TZC_INTERRUPT, tzc400_interrupt_handler, flags);
    tzc400_enable_interrupts();
    tzc400_set_action(TZC_ACTION_INT);
    tzc400_enable_filters();
}

void plat_arm_security_setup(void)
{
    arm_tzc400_setup(PLAT_ARM_TZC_BASE, NULL);
}

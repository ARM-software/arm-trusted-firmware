/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <drivers/arm/gicv3.h>

#include <platform.h>
#include <platform_def.h>
#include <qti_plat.h>
#include <qtiseclib_defs.h>
#include <qtiseclib_defs_plat.h>

/* The GICv3 driver only needs to be initialized in EL3 */
static uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

/* Array of interrupts to be configured by the gic driver */
static const interrupt_prop_t qti_interrupt_props[] = {
	INTR_PROP_DESC(QTISECLIB_INT_ID_CPU_WAKEUP_SGI,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(QTISECLIB_INT_ID_RESET_SGI, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(QTISECLIB_INT_ID_SEC_WDOG_BARK, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(QTISECLIB_INT_ID_NON_SEC_WDOG_BITE,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(QTISECLIB_INT_ID_VMIDMT_ERR_CLT_SEC,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(QTISECLIB_INT_ID_VMIDMT_ERR_CLT_NONSEC,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(QTISECLIB_INT_ID_VMIDMT_ERR_CFG_SEC,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(QTISECLIB_INT_ID_VMIDMT_ERR_CFG_NONSEC,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(QTISECLIB_INT_ID_XPU_SEC, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(QTISECLIB_INT_ID_XPU_NON_SEC, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
#ifdef QTISECLIB_INT_ID_A1_NOC_ERROR
	INTR_PROP_DESC(QTISECLIB_INT_ID_A1_NOC_ERROR, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
#endif
	INTR_PROP_DESC(QTISECLIB_INT_ID_A2_NOC_ERROR, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(QTISECLIB_INT_ID_CONFIG_NOC_ERROR,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(QTISECLIB_INT_ID_DC_NOC_ERROR, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(QTISECLIB_INT_ID_MEM_NOC_ERROR, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(QTISECLIB_INT_ID_SYSTEM_NOC_ERROR,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(QTISECLIB_INT_ID_MMSS_NOC_ERROR,
		       GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
#ifdef QTISECLIB_INT_ID_LPASS_AGNOC_ERROR
	INTR_PROP_DESC(QTISECLIB_INT_ID_LPASS_AGNOC_ERROR, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
#endif
#ifdef QTISECLIB_INT_ID_NSP_NOC_ERROR
	INTR_PROP_DESC(QTISECLIB_INT_ID_NSP_NOC_ERROR, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0,
		       GIC_INTR_CFG_EDGE),
#endif
};

const gicv3_driver_data_t qti_gic_data = {
	.gicd_base = QTI_GICD_BASE,
	.gicr_base = QTI_GICR_BASE,
	.interrupt_props = qti_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(qti_interrupt_props),
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = plat_qti_core_pos_by_mpidr
};

void plat_qti_gic_driver_init(void)
{
	/*
	 * The GICv3 driver is initialized in EL3 and does not need
	 * to be initialized again in SEL1. This is because the S-EL1
	 * can use GIC system registers to manage interrupts and does
	 * not need GIC interface base addresses to be configured.
	 */
	gicv3_driver_init(&qti_gic_data);
}

/******************************************************************************
 * ARM common helper to initialize the GIC. Only invoked by BL31
 *****************************************************************************/
void plat_qti_gic_init(void)
{
	unsigned int i;

	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());

	/* Route secure spi interrupt to ANY. */
	for (i = 0; i < ARRAY_SIZE(qti_interrupt_props); i++) {
		unsigned int int_id = qti_interrupt_props[i].intr_num;

		if (plat_ic_is_spi(int_id)) {
			gicv3_set_spi_routing(int_id, GICV3_IRM_ANY, 0x0);
		}
	}
}

void gic_set_spi_routing(unsigned int id, unsigned int irm, u_register_t target)
{
	gicv3_set_spi_routing(id, irm, target);
}

/******************************************************************************
 * ARM common helper to enable the GIC CPU interface
 *****************************************************************************/
void plat_qti_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helper to disable the GIC CPU interface
 *****************************************************************************/
void plat_qti_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helper to initialize the per-CPU redistributor interface in GICv3
 *****************************************************************************/
void plat_qti_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helpers to power GIC redistributor interface
 *****************************************************************************/
void plat_qti_gic_redistif_on(void)
{
	gicv3_rdistif_on(plat_my_core_pos());
}

void plat_qti_gic_redistif_off(void)
{
	gicv3_rdistif_off(plat_my_core_pos());
}

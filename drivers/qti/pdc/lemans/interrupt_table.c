/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/qti/pdc/pdc_internal.h>
#include <lib/utils_def.h>

/* PDC interrupt mapping for lemans (qcs9075) APSS */
struct pdc_interrupt_mapping g_pdc_interrupt_mapping[] = {
	/* Bit 0 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 512 }, /* rpmh_wake */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 513 }, /* ee0_apps_hlos_spmi_periph_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 514 }, /* ee1_apps_trustzone_spmi_periph_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV0 }, 515 }, /* secure_wdog_expired */

	/* Bit 4 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV0 }, 516 }, /* secure_wdog_bark_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV0 }, 517 }, /* aop_wdog_expired_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 518 }, /* wdog_exp_irq_mx */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 519 }, /* not-connected */

	/* Bit 8 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 520 }, /* not-connected */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 521 }, /* not-connected */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 522 }, /* not-connected */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 523 }, /* eud_int_mx[1] */

	/* Bit 12 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 524 }, /* eud_p1_dpse_int_mx */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 525 }, /* eud_p1_dmse_int_mx */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 526 }, /* eud_p0_dpse_int_mx */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 527 }, /* eud_p0_dmse_int_mx */

	/* Bit 16 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 528 }, /* codec_tx_swr_irq[1] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 529 }, /* qmp_usb3_lfps_rxterm_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 530 }, /* aoss_pmic_arb_mpu_xpu_summary_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 531 }, /* rpmh_wake_2 */

	/* Bit 20 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 532 }, /* tsens0_tsens_0C_int */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 533 }, /* tsens1_tsens_0C_int */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 534 }, /* pdc_apps_epcb_timeout_summary_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 535 }, /* spmi_protocol_irq */

	/* Bit 24 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 536 }, /* tsense0_tsense_max_min_int */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 537 }, /* tsense1_tsense_max_min_int */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 538 }, /* tsense0_upper_lower_intr */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 539 }, /* tsense1_upper_lower_intr */

	/* Bit 28 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 540 }, /* tsense0_critical_intr */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 541 }, /* tsense1_critical_intr */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 542 }, /* i3c_ibi_irq_s0[0] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 543 }, /* i3c_ibi_irq_s0[1] */

	/* Bit 32 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 544 }, /* i3c_ibi_irq_s1[0] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 545 }, /* i3c_ibi_irq_s1[1] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 546 }, /* i3c_ibi_irq_s2[0] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 547 }, /* i3c_ibi_irq_s2[1] */

	/* Bit 36 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 548 }, /* i3c_ibi_irq_s3[0] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 549 }, /* i3c_ibi_irq_s3[1] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 550 }, /* not-connected */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 551 }, /* not-connected */

	/* Bit 40 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 172 }, /* pcie_global_int */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 173 }, /* pcie_int_msi_dev0 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 174 }, /* pcie_int_msi_dev1 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 175 }, /* pcie_int_msi_dev2 */

	/* Bit 44 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 176 }, /* pcie_int_msi_dev3 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 177 }, /* pcie_int_msi_dev4 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 178 }, /* pcie_int_msi_dev5 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 179 }, /* pcie_int_msi_dev6 */

	/* Bit 48 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 180 }, /* pcie_int_msi_dev7 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 181 }, /* pcie_inta */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 182 }, /* pcie_intb */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 183 }, /* pcie_intc */

	/* Bit 52 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 184 }, /* pcie_intd */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 185 }, /* pcie_int_mhi_a7 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 295 }, /* pcie_int_edma_int */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 338 }, /* pcie_global_int */

	/* Bit 56 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 339 }, /* pcie_int_msi_dev0 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 340 }, /* pcie_int_msi_dev1 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 341 }, /* pcie_int_msi_dev2 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 344 }, /* pcie_int_msi_dev3 */

	/* Bit 60 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 345 }, /* pcie_int_msi_dev4 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 346 }, /* pcie_int_msi_dev5 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 406 }, /* pcie_int_msi_dev6 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 407 }, /* pcie_int_msi_dev7 */

	/* Bit 64 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 466 }, /* pcie_inta */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 467 }, /* pcie_intb */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 470 }, /* pcie_intc */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 471 }, /* pcie_intd */

	/* Bit 68 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 472 }, /* pcie_int_mhi_a7 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 118 }, /* pcie_int_edma_int */
};

const uint32_t g_pdc_interrupt_table_size = ARRAY_SIZE(g_pdc_interrupt_mapping);

/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/qti/pdc/pdc_seq.h>

/* CX retention, no AOSS sleep */
static uint8_t apps_cx_ret[] = {
	0xe1,	/* WAIT1  rx_fifo_empty        */
	0xa1,	/* SET1   pdc_mode_control      */
	0xa0,	/* SET1   timer_match_valid     */
	0xbf,	/* SET1   seq_tcs_0_start       */
	0xff,	/* WAIT1  tcs_0_seq_cmp         */
	0xa4,	/* SET1   dbg_resource_rel_req  */
	0xa5,	/* SET1   wfi_profiling_unit_3  */
	0xe0,	/* WAIT1  sequence_resume       */
	0x85,	/* SET0   wfi_profiling_unit_3  */
	0x84,	/* SET0   dbg_resource_rel_req  */
	0xbc,	/* SET1   seq_tcs_3_start       */
	0xfc,	/* WAIT1  tcs_3_seq_cmp         */
	0x80,	/* SET0   timer_match_valid     */
	0x81,	/* SET0   pdc_mode_control      */
	0xa7,	/* SET1   wakeup_req            */
	0x87,	/* SET0   wakeup_req            */
	0xe7,	/* WAIT1  wakeup_ack            */
	0x20,	/* RETURN                       */
	0x20,	/* RETURN                       */
};

/* CX retention with AOSS sleep */
static uint8_t apps_cx_ret_aoss[] = {
	0xe1,	/* WAIT1  rx_fifo_empty        */
	0xa1,	/* SET1   pdc_mode_control      */
	0xa0,	/* SET1   timer_match_valid     */
	0xbf,	/* SET1   seq_tcs_0_start       */
	0xff,	/* WAIT1  tcs_0_seq_cmp         */
	0xa2,	/* SET1   soc_sleep_req         */
	0xa4,	/* SET1   dbg_resource_rel_req  */
	0xa5,	/* SET1   wfi_profiling_unit_3  */
	0xe0,	/* WAIT1  sequence_resume       */
	0x85,	/* SET0   wfi_profiling_unit_3  */
	0x82,	/* SET0   soc_sleep_req         */
	0xe2,	/* WAIT1  soc_sleep_unlock      */
	0x84,	/* SET0   dbg_resource_rel_req  */
	0xbc,	/* SET1   seq_tcs_3_start       */
	0xfc,	/* WAIT1  tcs_3_seq_cmp         */
	0x80,	/* SET0   timer_match_valid     */
	0x81,	/* SET0   pdc_mode_control      */
	0xa7,	/* SET1   wakeup_req            */
	0x87,	/* SET0   wakeup_req            */
	0xe7,	/* WAIT1  wakeup_ack            */
	0x20,	/* RETURN                       */
	0x20,	/* RETURN                       */
	0x20,	/* RETURN                       */
};

static uint8_t apps_branches[PDC_SEQ_BR_ADDR_REG_COUNT] = { 0 };
static uint32_t apps_delays[PDC_SEQ_DELAY_REG_COUNT] = { 0 };

static struct pdc_seq_cfg apps_pdc_cfg = {
	PDC_SEQ_ARRAY(apps_branches),
	PDC_SEQ_ARRAY(apps_delays),
};

static struct pdc_seq_mode apps_pdc_modes[] = {
	{ PDC_SEQ_ARRAY(apps_cx_ret),      1, 0 }, /* mode_id 1: CXret */
	{ PDC_SEQ_ARRAY(apps_cx_ret_aoss), 3, 0 }, /* mode_id 3: CXret + AOSS */
};

static struct pdc_seq pdc_seq_instance = {
	PDC_SEQ_APPS,
	&apps_pdc_cfg,
	PDC_SEQ_ARRAY(apps_pdc_modes),
	0x200000U,	/* PDC offset from AOSS base */
	0xA00000U,	/* RSC offset from SS base   */
};

struct pdc_seq *g_pdc_seqs = &pdc_seq_instance;
uint32_t g_pdc_seq_count = 1U;

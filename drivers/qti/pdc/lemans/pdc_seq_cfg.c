/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/qti/pdc/pdc_seq.h>

/* CX collapse, no AOSS sleep */
static uint8_t apps_cx_col[] = {
	0xe1,	/* WAIT1     rx_fifo_empty           */
	0xa1,	/* SET1      pdc_mode_control        */
	0xa0,	/* SET1      timer_match_valid       */
	0xaa,	/* SET1      md_from_sail_iso_en     */
	0xab,	/* SET1      md_2_sail_sleep_en_req  */
	0xee,	/* WAIT1     md_2_sail_sleep_en_ack  */
	0x8b,	/* SET0      md_2_sail_sleep_en_req  */
	0xbe,	/* SET1      seq_tcs_1_start         */
	0xfe,	/* WAIT1     tcs_1_seq_cmp           */
	0xa3,	/* SET1      cx_collapse_req         */
	0xa4,	/* SET1      dbg_resource_rel_req    */
	0xa5,	/* SET1      wfi_profiling_unit_3    */
	0xe0,	/* WAIT1     sequence_resume         */
	0x85,	/* SET0      wfi_profiling_unit_3    */
	0xad,	/* SET1      md_2_sail_wakeup_int    */
	0x83,	/* SET0      cx_collapse_req         */
	0x84,	/* SET0      dbg_resource_rel_req    */
	0xe3,	/* WAIT1     cx_collapse_unlock      */
	0xbc,	/* SET1      seq_tcs_3_start         */
	0xfc,	/* WAIT1     tcs_3_seq_cmp           */
	0xac,	/* SET1      md_2_sail_pwr_rdy_req   */
	0xef,	/* WAIT1     md_2_sail_pwr_ready_ack */
	0x8c,	/* SET0      md_2_sail_pwr_rdy_req   */
	0xcf,	/* WAIT0     md_2_sail_pwr_ready_ack */
	0x8a,	/* SET0      md_from_sail_iso_en     */
	0x80,	/* SET0      timer_match_valid       */
	0x81,	/* SET0      pdc_mode_control        */
	0xa7,	/* SET1      wakeup_req              */
	0x87,	/* SET0      wakeup_req              */
	0xe7,	/* WAIT1     wakeup_ack              */
	0x20,	/* RETURN                            */
};

/* CX collapse with AOSS sleep */
static uint8_t apps_cx_col_aoss[] = {
	0xe1,	/* WAIT1     rx_fifo_empty           */
	0xa1,	/* SET1      pdc_mode_control        */
	0xa0,	/* SET1      timer_match_valid       */
	0xaa,	/* SET1      md_from_sail_iso_en     */
	0xab,	/* SET1      md_2_sail_sleep_en_req  */
	0xee,	/* WAIT1     md_2_sail_sleep_en_ack  */
	0x8b,	/* SET0      md_2_sail_sleep_en_req  */
	0xbe,	/* SET1      seq_tcs_1_start         */
	0xfe,	/* WAIT1     tcs_1_seq_cmp           */
	0xa3,	/* SET1      cx_collapse_req         */
	0xa2,	/* SET1      soc_sleep_req           */
	0xa4,	/* SET1      dbg_resource_rel_req    */
	0xa5,	/* SET1      wfi_profiling_unit_3    */
	0xe0,	/* WAIT1     sequence_resume         */
	0x85,	/* SET0      wfi_profiling_unit_3    */
	0xad,	/* SET1      md_2_sail_wakeup_int    */
	0x82,	/* SET0      soc_sleep_req           */
	0x83,	/* SET0      cx_collapse_req         */
	0xe2,	/* WAIT1     soc_sleep_unlock        */
	0x84,	/* SET0      dbg_resource_rel_req    */
	0xe3,	/* WAIT1     cx_collapse_unlock      */
	0xbc,	/* SET1      seq_tcs_3_start         */
	0xfc,	/* WAIT1     tcs_3_seq_cmp           */
	0xac,	/* SET1      md_2_sail_pwr_rdy_req   */
	0xef,	/* WAIT1     md_2_sail_pwr_ready_ack */
	0x8c,	/* SET0      md_2_sail_pwr_rdy_req   */
	0xcf,	/* WAIT0     md_2_sail_pwr_ready_ack */
	0x8a,	/* SET0      md_from_sail_iso_en     */
	0x80,	/* SET0      timer_match_valid       */
	0x81,	/* SET0      pdc_mode_control        */
	0xa7,	/* SET1      wakeup_req              */
	0x87,	/* SET0      wakeup_req              */
	0xe7,	/* WAIT1     wakeup_ack              */
	0x20,	/* RETURN                            */
};

static uint8_t apps_branches[PDC_SEQ_BR_ADDR_REG_COUNT] = { 0 };
static uint32_t apps_delays[PDC_SEQ_DELAY_REG_COUNT] = { 0 };

static struct pdc_seq_cfg apps_pdc_cfg = {
	PDC_SEQ_ARRAY(apps_branches),
	PDC_SEQ_ARRAY(apps_delays),
};

static struct pdc_seq_mode apps_pdc_modes[] = {
	{ PDC_SEQ_ARRAY(apps_cx_col),      1, 0 }, /* mode_id 1: CXcol */
	{ PDC_SEQ_ARRAY(apps_cx_col_aoss), 3, 0 }, /* mode_id 3: CXcol + AOSS */
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

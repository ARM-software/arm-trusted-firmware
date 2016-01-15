/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __PMIC_WRAP_INIT_H__
#define __PMIC_WRAP_INIT_H__

#include <mt8173_def.h>
#include <stdint.h>

/* external API */
int32_t pwrap_read(uint32_t adr, uint16_t *rdata);
int32_t pwrap_write(uint32_t adr, uint16_t wdata);

static struct mt8173_pmic_wrap_regs *const mt8173_pwrap =
	(void *)PMIC_WRAP_BASE;

/* timeout setting */
enum {
	TIMEOUT_RESET       = 50,	/* us */
	TIMEOUT_READ        = 50,	/* us */
	TIMEOUT_WAIT_IDLE   = 50	/* us */
};

/* PMIC_WRAP registers */
struct mt8173_pmic_wrap_regs {
	uint32_t mux_sel;
	uint32_t wrap_en;
	uint32_t dio_en;
	uint32_t sidly;
	uint32_t rddmy;
	uint32_t si_ck_con;
	uint32_t cshext_write;
	uint32_t cshext_read;
	uint32_t cslext_start;
	uint32_t cslext_end;
	uint32_t staupd_prd;
	uint32_t staupd_grpen;
	uint32_t reserved[4];
	uint32_t staupd_man_trig;
	uint32_t staupd_sta;
	uint32_t wrap_sta;
	uint32_t harb_init;
	uint32_t harb_hprio;
	uint32_t hiprio_arb_en;
	uint32_t harb_sta0;
	uint32_t harb_sta1;
	uint32_t man_en;
	uint32_t man_cmd;
	uint32_t man_rdata;
	uint32_t man_vldclr;
	uint32_t wacs0_en;
	uint32_t init_done0;
	uint32_t wacs0_cmd;
	uint32_t wacs0_rdata;
	uint32_t wacs0_vldclr;
	uint32_t wacs1_en;
	uint32_t init_done1;
	uint32_t wacs1_cmd;
	uint32_t wacs1_rdata;
	uint32_t wacs1_vldclr;
	uint32_t wacs2_en;
	uint32_t init_done2;
	uint32_t wacs2_cmd;
	uint32_t wacs2_rdata;
	uint32_t wacs2_vldclr;
	uint32_t int_en;
	uint32_t int_flg_raw;
	uint32_t int_flg;
	uint32_t int_clr;
	uint32_t sig_adr;
	uint32_t sig_mode;
	uint32_t sig_value;
	uint32_t sig_errval;
	uint32_t crc_en;
	uint32_t timer_en;
	uint32_t timer_sta;
	uint32_t wdt_unit;
	uint32_t wdt_src_en;
	uint32_t wdt_flg;
	uint32_t debug_int_sel;
	uint32_t dvfs_adr0;
	uint32_t dvfs_wdata0;
	uint32_t dvfs_adr1;
	uint32_t dvfs_wdata1;
	uint32_t dvfs_adr2;
	uint32_t dvfs_wdata2;
	uint32_t dvfs_adr3;
	uint32_t dvfs_wdata3;
	uint32_t dvfs_adr4;
	uint32_t dvfs_wdata4;
	uint32_t dvfs_adr5;
	uint32_t dvfs_wdata5;
	uint32_t dvfs_adr6;
	uint32_t dvfs_wdata6;
	uint32_t dvfs_adr7;
	uint32_t dvfs_wdata7;
	uint32_t spminf_sta;
	uint32_t cipher_key_sel;
	uint32_t cipher_iv_sel;
	uint32_t cipher_en;
	uint32_t cipher_rdy;
	uint32_t cipher_mode;
	uint32_t cipher_swrst;
	uint32_t dcm_en;
	uint32_t dcm_dbc_prd;
};

enum {
	RDATA_WACS_RDATA_SHIFT = 0,
	RDATA_WACS_FSM_SHIFT = 16,
	RDATA_WACS_REQ_SHIFT = 19,
	RDATA_SYNC_IDLE_SHIFT,
	RDATA_INIT_DONE_SHIFT,
	RDATA_SYS_IDLE_SHIFT,
};

enum {
	RDATA_WACS_RDATA_MASK = 0xffff,
	RDATA_WACS_FSM_MASK = 0x7,
	RDATA_WACS_REQ_MASK = 0x1,
	RDATA_SYNC_IDLE_MASK = 0x1,
	RDATA_INIT_DONE_MASK = 0x1,
	RDATA_SYS_IDLE_MASK = 0x1,
};

/* WACS_FSM */
enum {
	WACS_FSM_IDLE            = 0x00,
	WACS_FSM_REQ             = 0x02,
	WACS_FSM_WFDLE           = 0x04,
	WACS_FSM_WFVLDCLR        = 0x06,
	WACS_INIT_DONE           = 0x01,
	WACS_SYNC_IDLE           = 0x01,
	WACS_SYNC_BUSY           = 0x00
};

/* error information flag */
enum {
	E_PWR_INVALID_ARG             = 1,
	E_PWR_INVALID_RW              = 2,
	E_PWR_INVALID_ADDR            = 3,
	E_PWR_INVALID_WDAT            = 4,
	E_PWR_INVALID_OP_MANUAL       = 5,
	E_PWR_NOT_IDLE_STATE          = 6,
	E_PWR_NOT_INIT_DONE           = 7,
	E_PWR_NOT_INIT_DONE_READ      = 8,
	E_PWR_WAIT_IDLE_TIMEOUT       = 9,
	E_PWR_WAIT_IDLE_TIMEOUT_READ  = 10,
	E_PWR_INIT_SIDLY_FAIL         = 11,
	E_PWR_RESET_TIMEOUT           = 12,
	E_PWR_TIMEOUT                 = 13,
	E_PWR_INIT_RESET_SPI          = 20,
	E_PWR_INIT_SIDLY              = 21,
	E_PWR_INIT_REG_CLOCK          = 22,
	E_PWR_INIT_ENABLE_PMIC        = 23,
	E_PWR_INIT_DIO                = 24,
	E_PWR_INIT_CIPHER             = 25,
	E_PWR_INIT_WRITE_TEST         = 26,
	E_PWR_INIT_ENABLE_CRC         = 27,
	E_PWR_INIT_ENABLE_DEWRAP      = 28,
	E_PWR_INIT_ENABLE_EVENT       = 29,
	E_PWR_READ_TEST_FAIL          = 30,
	E_PWR_WRITE_TEST_FAIL         = 31,
	E_PWR_SWITCH_DIO              = 32
};

#endif /* __PMIC_WRAP_INIT_H__ */

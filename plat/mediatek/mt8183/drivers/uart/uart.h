/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __UART_H__
#define __UART_H__

#include <platform_def.h>

/* UART HW information */
#define HW_SUPPORT_UART_PORTS	2
#define DRV_SUPPORT_UART_PORTS	2

/* console UART clock cg */
#define UART_CLOCK_GATE_SET		(INFRACFG_AO_BASE + 0x80)
#define UART_CLOCK_GATE_CLR		(INFRACFG_AO_BASE + 0x84)
#define UART_CLOCK_GATE_STA		(INFRACFG_AO_BASE + 0x90)
#define UART0_CLOCK_GATE_BIT		(1U<<22)
#define UART1_CLOCK_GATE_BIT		(1U<<23)

/* UART registers */
#define UART_RBR(_baseaddr)			(_baseaddr + 0x0)
#define UART_THR(_baseaddr)			(_baseaddr + 0x0)
#define UART_IER(_baseaddr)			(_baseaddr + 0x4)
#define UART_IIR(_baseaddr)			(_baseaddr + 0x8)
#define UART_FCR(_baseaddr)			(_baseaddr + 0x8)
#define UART_LCR(_baseaddr)			(_baseaddr + 0xc)
#define UART_MCR(_baseaddr)			(_baseaddr + 0x10)
#define UART_LSR(_baseaddr)			(_baseaddr + 0x14)
#define UART_MSR(_baseaddr)			(_baseaddr + 0x18)
#define UART_SCR(_baseaddr)			(_baseaddr + 0x1c)
#define UART_DLL(_baseaddr)			(_baseaddr + 0x0)
#define UART_DLH(_baseaddr)			(_baseaddr + 0x4)
#define UART_EFR(_baseaddr)			(_baseaddr + 0x8)
#define UART_XON1(_baseaddr)			(_baseaddr + 0x10)
#define UART_XON2(_baseaddr)			(_baseaddr + 0x14)
#define UART_XOFF1(_baseaddr)			(_baseaddr + 0x18)
#define UART_XOFF2(_baseaddr)			(_baseaddr + 0x1c)
#define UART_AUTOBAUD(_baseaddr)		(_baseaddr + 0x20)
#define UART_HIGHSPEED(_baseaddr)		(_baseaddr + 0x24)
#define UART_SAMPLE_COUNT(_baseaddr)		(_baseaddr + 0x28)
#define UART_SAMPLE_POINT(_baseaddr)		(_baseaddr + 0x2c)
#define UART_AUTOBAUD_REG(_baseaddr)		(_baseaddr + 0x30)
#define UART_RATE_FIX_REG(_baseaddr)		(_baseaddr + 0x34)
#define UART_AUTO_BAUDSAMPLE(_baseaddr)		(_baseaddr + 0x38)
#define UART_GUARD(_baseaddr)			(_baseaddr + 0x3c)
#define UART_ESCAPE_DAT(_baseaddr)		(_baseaddr + 0x40)
#define UART_ESCAPE_EN(_baseaddr)		(_baseaddr + 0x44)
#define UART_SLEEP_EN(_baseaddr)		(_baseaddr + 0x48)
#define UART_DMA_EN(_baseaddr)			(_baseaddr + 0x4c)
#define UART_RXTRI_AD(_baseaddr)		(_baseaddr + 0x50)
#define UART_FRACDIV_L(_baseaddr)		(_baseaddr + 0x54)
#define UART_FRACDIV_M(_baseaddr)		(_baseaddr + 0x58)
#define UART_FCR_RD(_baseaddr)			(_baseaddr + 0x5C)
#define UART_USB_RX_SEL(_baseaddr)		(_baseaddr + 0xB0)
#define UART_SLEEP_REQ(_baseaddr)		(_baseaddr + 0xB4)
#define UART_SLEEP_ACK(_baseaddr)		(_baseaddr + 0xB8)
#define UART_SPM_SEL(_baseaddr)			(_baseaddr + 0xBC)
#define UART_LCR_DLAB				0x0080
#define UART_LCR_MODE_B				0x00bf

enum uart_port_ID {
	UART_PORT0 = 0,
	UART_PORT1
};

struct mt_uart_register {
	unsigned int dll;
	unsigned int dlh;
	unsigned int ier;
	unsigned int lcr;
	unsigned int mcr;
	unsigned int fcr;
	unsigned int lsr;
	unsigned int scr;
	unsigned int efr;
	unsigned int highspeed;
	unsigned int sample_count;
	unsigned int sample_point;
	unsigned int fracdiv_l;
	unsigned int fracdiv_m;
	unsigned int escape_en;
	unsigned int guard;
	unsigned int rx_sel;
};

struct mt_uart {
	unsigned long base;
	struct mt_uart_register registers;
};

/* external API */
void mt_uart_save(void);
void mt_uart_restore(void);
void mt_console_uart_cg(int on);
int mt_console_uart_cg_status(void);

#endif /* __UART_H__ */

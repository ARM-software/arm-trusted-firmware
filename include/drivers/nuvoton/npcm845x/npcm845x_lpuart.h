/*
 * Copyright (c) 2015-2023, ARM Limited and Contributors. All rights reserved.
 *
 * Copyright (C) 2022-2023 Nuvoton Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ASM_ARCH_UART_H_
#define __ASM_ARCH_UART_H_

#ifndef __ASSEMBLY__

struct npcmX50_uart {
	union {
		unsigned int rbr;
		unsigned int thr;
		unsigned int dll;
	};
	union {
		unsigned int ier;
		unsigned int dlm;
	};
	union {
		unsigned int iir;
		unsigned int fcr;
	};
	unsigned int lcr;
	unsigned int mcr;
	unsigned int lsr;
	unsigned int msr;
	unsigned int tor;
};

typedef enum {
	/*
	 * UART0 is a general UART block without modem-I/O-control
	 * connection to external signals.
	 */
	UART0_DEV = 0,
	/*
	 * UART1-3 are each a general UART with modem-I/O-control
	 * connection to external signals.
	 */
	UART1_DEV,
	UART2_DEV,
	UART3_DEV,
} UART_DEV_T;

typedef enum {
	/*
	 * 0 0 0: Mode 1:
	 * HSP1 connected to SI2,
	 * HSP2 connected to UART2,
	 * UART1 snoops HSP1,
	 * UART3 snoops SI2
	 */
	UART_MUX_MODE1 = 0,
	/*
	 * 0 0 1: Mode 2:
	 * HSP1 connected to UART1,
	 * HSP2 connected to SI2,
	 * UART2 snoops HSP2,
	 * UART3 snoops SI2
	 */
	UART_MUX_MODE2,
	/*
	 * 0 1 0: Mode 3:
	 * HSP1 connected to UART1,
	 * HSP2 connected to UART2,
	 * UART3 connected to SI2
	 */
	UART_MUX_MODE3,
	/*
	 * 0 1 1: Mode 4:
	 * HSP1 connected to SI1,
	 * HSP2 connected to SI2,
	 * UART1 snoops SI1,
	 * UART3 snoops SI2,
	 * UART2 snoops HSP1 (default)
	 */
	UART_MUX_MODE4,
	/*
	 * 1 0 0: Mode 5:
	 * HSP1 connected to SI1,
	 * HSP2 connected to UART2,
	 * UART1 snoops HSP1,
	 * UART3 snoops SI1
	 */
	UART_MUX_MODE5,
	/*
	 * 1 0 1: Mode 6:
	 * HSP1 connected to SI1,
	 * HSP2 connected to SI2,
	 * UART1 snoops SI1,
	 * UART3 snoops SI2,
	 * UART2 snoops HSP2
	 */
	UART_MUX_MODE6,
	/*
	 * 1 1 0: Mode 7:
	 * HSP1 connected to SI1,
	 * HSP2 connected to UART2,
	 * UART1 snoops HSP1,
	 * UART3 connected to SI2
	 */
	UART_MUX_MODE7,
	/* Skip UART mode configuration. */
	UART_MUX_RESERVED,
	/*
	 * A SW option to allow config of UART
	 * without touching the UART mux.
	 */
	UART_MUX_SKIP_CONFIG
} UART_MUX_T;

/*---------------------------------------------------------------------------*/
/* Common baudrate definitions                                               */
/*---------------------------------------------------------------------------*/
typedef enum {
	UART_BAUDRATE_110 = 110,
	UART_BAUDRATE_300 = 300,
	UART_BAUDRATE_600 = 600,
	UART_BAUDRATE_1200 = 1200,
	UART_BAUDRATE_2400 = 2400,
	UART_BAUDRATE_4800 = 4800,
	UART_BAUDRATE_9600 = 9600,
	UART_BAUDRATE_14400 = 14400,
	UART_BAUDRATE_19200 = 19200,
	UART_BAUDRATE_38400 = 38400,
	UART_BAUDRATE_57600 = 57600,
	UART_BAUDRATE_115200 = 115200,
	UART_BAUDRATE_230400 = 230400,
	UART_BAUDRATE_380400 = 380400,
	UART_BAUDRATE_460800 = 460800,
} UART_BAUDRATE_T;

/*---------------------------------------------------------------------------*/
/* UART parity types                                                         */
/*---------------------------------------------------------------------------*/
typedef enum {
	UART_PARITY_NONE = 0,
	UART_PARITY_EVEN,
	UART_PARITY_ODD,
} UART_PARITY_T;

/*---------------------------------------------------------------------------*/
/* Uart stop bits                                                            */
/*---------------------------------------------------------------------------*/
typedef enum {
	UART_STOPBIT_1 = 0x00,
	UART_STOPBIT_DYNAMIC,
} UART_STOPBIT_T;

enum FCR_RFITL_TYPE {
	FCR_RFITL_1B = 0x0,
	FCR_RFITL_4B = 0x4,
	FCR_RFITL_8B = 0x8,
	FCR_RFITL_14B = 0xC,
};

enum LCR_WLS_TYPE {
	LCR_WLS_5bit = 0x0,
	LCR_WLS_6bit = 0x1,
	LCR_WLS_7bit = 0x2,
	LCR_WLS_8bit = 0x3,
};

#define IER_DBGACK (1 << 4)
#define IER_MSIE (1 << 3)
#define IER_RLSE (1 << 2)
#define IER_THREIE (1 << 1)
#define IER_RDAIE (1 << 0)

#define IIR_FMES (1 << 7)
#define IIR_RFTLS (1 << 5)
#define IIR_DMS (1 << 4)
#define IIR_IID (1 << 1)
#define IIR_NIP (1 << 0)

#define FCR_RFITL_1B (0 << 4)
#define FCR_RFITL_4B (4 << 4)
#define FCR_RFITL_8B (8 << 4)
#define FCR_RFITL_14B (12 << 4)
#define FCR_DMS (1 << 3)
#define FCR_TFR (1 << 2)
#define FCR_RFR (1 << 1)
#define FCR_FME (1 << 0)

#define LCR_DLAB (1 << 7)
#define LCR_BCB (1 << 6)
#define LCR_SPE (1 << 5)
#define LCR_EPS (1 << 4)
#define LCR_PBE (1 << 3)
#define LCR_NSB (1 << 2)
#define LCR_WLS_8b (3 << 0)
#define LCR_WLS_7b (2 << 0)
#define LCR_WLS_6b (1 << 0)
#define LCR_WLS_5b (0 << 0)

#define MCR_LBME (1 << 4)
#define MCR_OUT2 (1 << 3)
#define MCR_RTS (1 << 1)
#define MCR_DTR (1 << 0)

#define LSR_ERR_RX (1 << 7)
#define LSR_TE (1 << 6)
#define LSR_THRE (1 << 5)
#define LSR_BII (1 << 4)
#define LSR_FEI (1 << 3)
#define LSR_PEI (1 << 2)
#define LSR_OEI (1 << 1)
#define LSR_RFDR (1 << 0)

#define MSR_DCD (1 << 7)
#define MSR_RI (1 << 6)
#define MSR_DSR (1 << 5)
#define MSR_CTS (1 << 4)
#define MSR_DDCD (1 << 3)
#define MSR_DRI (1 << 2)
#define MSR_DDSR (1 << 1)
#define MSR_DCTS (1 << 0)

#endif /* __ASSEMBLY__ */

uintptr_t npcm845x_get_base_uart(UART_DEV_T dev);
void CLK_ResetUART(void);
int UART_Init(UART_DEV_T devNum, UART_BAUDRATE_T baudRate);

#endif /* __ASM_ARCH_UART_H_ */

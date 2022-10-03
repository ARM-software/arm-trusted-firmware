/*
 * Copyright (c) 2021-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32_UART_H
#define STM32_UART_H

/* UART word length */
#define STM32_UART_WORDLENGTH_7B		USART_CR1_M1
#define STM32_UART_WORDLENGTH_8B		0x00000000U
#define STM32_UART_WORDLENGTH_9B		USART_CR1_M0

/* UART number of stop bits */
#define STM32_UART_STOPBITS_0_5			USART_CR2_STOP_0
#define STM32_UART_STOPBITS_1			0x00000000U
#define STM32_UART_STOPBITS_1_5			(USART_CR2_STOP_0 | USART_CR2_STOP_1)
#define STM32_UART_STOPBITS_2			USART_CR2_STOP_1

/* UART parity */
#define STM32_UART_PARITY_NONE			0x00000000U
#define STM32_UART_PARITY_EVEN			USART_CR1_PCE
#define STM32_UART_PARITY_ODD			(USART_CR1_PCE | USART_CR1_PS)

/* UART transfer mode */
#define STM32_UART_MODE_RX			USART_CR1_RE
#define STM32_UART_MODE_TX			USART_CR1_TE
#define STM32_UART_MODE_TX_RX			(USART_CR1_TE | USART_CR1_RE)

/* UART hardware flow control */
#define STM32_UART_HWCONTROL_NONE		0x00000000U
#define STM32_UART_HWCONTROL_RTS		USART_CR3_RTSE
#define STM32_UART_HWCONTROL_CTS		USART_CR3_CTSE
#define STM32_UART_HWCONTROL_RTS_CTS		(USART_CR3_RTSE | USART_CR3_CTSE)

/* UART prescaler */
#define STM32_UART_PRESCALER_DIV1		0x00000000U
#define STM32_UART_PRESCALER_DIV2		0x00000001U
#define STM32_UART_PRESCALER_DIV4		0x00000002U
#define STM32_UART_PRESCALER_DIV6		0x00000003U
#define STM32_UART_PRESCALER_DIV8		0x00000004U
#define STM32_UART_PRESCALER_DIV10		0x00000005U
#define STM32_UART_PRESCALER_DIV12		0x00000006U
#define STM32_UART_PRESCALER_DIV16		0x00000007U
#define STM32_UART_PRESCALER_DIV32		0x00000008U
#define STM32_UART_PRESCALER_DIV64		0x00000009U
#define STM32_UART_PRESCALER_DIV128		0x0000000AU
#define STM32_UART_PRESCALER_DIV256		0x0000000BU
#define STM32_UART_PRESCALER_NB			0x0000000CU

/* UART fifo mode */
#define STM32_UART_FIFOMODE_EN			USART_CR1_FIFOEN
#define STM32_UART_FIFOMODE_DIS			0x00000000U

/* UART TXFIFO threshold level */
#define STM32_UART_TXFIFO_THRESHOLD_1EIGHTHFULL		0x00000000U
#define STM32_UART_TXFIFO_THRESHOLD_1QUARTERFUL		USART_CR3_TXFTCFG_0
#define STM32_UART_TXFIFO_THRESHOLD_HALFFULL		USART_CR3_TXFTCFG_1
#define STM32_UART_TXFIFO_THRESHOLD_3QUARTERSFULL	(USART_CR3_TXFTCFG_0 | USART_CR3_TXFTCFG_1)
#define STM32_UART_TXFIFO_THRESHOLD_7EIGHTHFULL		USART_CR3_TXFTCFG_2
#define STM32_UART_TXFIFO_THRESHOLD_EMPTY		(USART_CR3_TXFTCFG_2 | USART_CR3_TXFTCFG_0)

/* UART RXFIFO threshold level */
#define STM32_UART_RXFIFO_THRESHOLD_1EIGHTHFULL		0x00000000U
#define STM32_UART_RXFIFO_THRESHOLD_1QUARTERFULL	USART_CR3_RXFTCFG_0
#define STM32_UART_RXFIFO_THRESHOLD_HALFFULL		USART_CR3_RXFTCFG_1
#define STM32_UART_RXFIFO_THRESHOLD_3QUARTERSFULL	(USART_CR3_RXFTCFG_0 | USART_CR3_RXFTCFG_1)
#define STM32_UART_RXFIFO_THRESHOLD_7EIGHTHFULL		USART_CR3_RXFTCFG_2
#define STM32_UART_RXFIFO_THRESHOLD_FULL		(USART_CR3_RXFTCFG_2 | USART_CR3_RXFTCFG_0)

struct stm32_uart_init_s {
	uint32_t baud_rate;		/*
					 * Configures the UART communication
					 * baud rate.
					 */

	uint32_t word_length;		/*
					 * Specifies the number of data bits
					 * transmitted or received in a frame.
					 * This parameter can be a value of
					 * @ref STM32_UART_WORDLENGTH_*.
					 */

	uint32_t stop_bits;		/*
					 * Specifies the number of stop bits
					 * transmitted. This parameter can be
					 * a value of @ref STM32_UART_STOPBITS_*.
					 */

	uint32_t parity;		/*
					 * Specifies the parity mode.
					 * This parameter can be a value of
					 * @ref STM32_UART_PARITY_*.
					 */

	uint32_t mode;			/*
					 * Specifies whether the receive or
					 * transmit mode is enabled or
					 * disabled. This parameter can be a
					 * value of @ref @ref STM32_UART_MODE_*.
					 */

	uint32_t hw_flow_control;	/*
					 * Specifies whether the hardware flow
					 * control mode is enabled or
					 * disabled. This parameter can be a
					 * value of @ref STM32_UARTHWCONTROL_*.
					 */

	uint32_t one_bit_sampling;	/*
					 * Specifies whether a single sample
					 * or three samples' majority vote is
					 * selected. This parameter can be 0
					 * or USART_CR3_ONEBIT.
					 */

	uint32_t prescaler;		/*
					 * Specifies the prescaler value used
					 * to divide the UART clock source.
					 * This parameter can be a value of
					 * @ref STM32_UART_PRESCALER_*.
					 */

	uint32_t fifo_mode;		/*
					 * Specifies if the FIFO mode will be
					 * used. This parameter can be a value
					 * of @ref STM32_UART_FIFOMODE_*.
					 */

	uint32_t tx_fifo_threshold;	/*
					 * Specifies the TXFIFO threshold
					 * level. This parameter can be a
					 * value of @ref
					 * STM32_UART_TXFIFO_THRESHOLD_*.
					 */

	uint32_t rx_fifo_threshold;	/*
					 * Specifies the RXFIFO threshold
					 * level. This parameter can be a
					 * value of @ref
					 * STM32_UART_RXFIFO_THRESHOLD_*.
					 */
};

struct stm32_uart_handle_s {
	uint32_t base;
	uint32_t rdr_mask;
};

int stm32_uart_init(struct stm32_uart_handle_s *huart,
		    uintptr_t base_addr,
		    const struct stm32_uart_init_s *init);
void stm32_uart_stop(uintptr_t base_addr);
int stm32_uart_putc(struct stm32_uart_handle_s *huart, int c);
int stm32_uart_flush(struct stm32_uart_handle_s *huart);
int stm32_uart_getc(struct stm32_uart_handle_s *huart);

#endif /* STM32_UART_H */

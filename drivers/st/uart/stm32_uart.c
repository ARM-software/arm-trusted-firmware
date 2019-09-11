/*
 * Copyright (c) 2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <common/bl_common.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32_uart.h>
#include <drivers/st/stm32_uart_regs.h>
#include <drivers/st/stm32mp_clkfunc.h>
#include <lib/mmio.h>

#include <platform_def.h>

/* UART time-out value */
#define STM32_UART_TIMEOUT_US	20000U

/* Mask to clear ALL the configuration registers */

#define STM32_UART_CR1_FIELDS \
		(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | \
		 USART_CR1_RE | USART_CR1_OVER8 | USART_CR1_FIFOEN)

#define STM32_UART_CR2_FIELDS \
		(USART_CR2_SLVEN | USART_CR2_DIS_NSS | USART_CR2_ADDM7 | \
		 USART_CR2_LBDL | USART_CR2_LBDIE | USART_CR2_LBCL | \
		 USART_CR2_CPHA | USART_CR2_CPOL | USART_CR2_CLKEN | \
		 USART_CR2_STOP | USART_CR2_LINEN | USART_CR2_SWAP | \
		 USART_CR2_RXINV | USART_CR2_TXINV | USART_CR2_DATAINV | \
		 USART_CR2_MSBFIRST | USART_CR2_ABREN | USART_CR2_ABRMODE | \
		 USART_CR2_RTOEN | USART_CR2_ADD)

#define STM32_UART_CR3_FIELDS \
		(USART_CR3_EIE | USART_CR3_IREN | USART_CR3_IRLP | \
		 USART_CR3_HDSEL | USART_CR3_NACK | USART_CR3_SCEN | \
		 USART_CR3_DMAR | USART_CR3_DMAT | USART_CR3_RTSE | \
		 USART_CR3_CTSE | USART_CR3_CTSIE | USART_CR3_ONEBIT | \
		 USART_CR3_OVRDIS | USART_CR3_DDRE | USART_CR3_DEM | \
		 USART_CR3_DEP | USART_CR3_SCARCNT | USART_CR3_WUS | \
		 USART_CR3_WUFIE | USART_CR3_TXFTIE | USART_CR3_TCBGTIE | \
		 USART_CR3_RXFTCFG | USART_CR3_RXFTIE | USART_CR3_TXFTCFG)

#define STM32_UART_ISR_ERRORS	 \
		(USART_ISR_ORE | USART_ISR_NE |  USART_ISR_FE | USART_ISR_PE)

static const uint16_t presc_table[STM32_UART_PRESCALER_NB] = {
	1U, 2U, 4U, 6U, 8U, 10U, 12U, 16U, 32U, 64U, 128U, 256U
};

/* @brief  BRR division operation to set BRR register in 8-bit oversampling
 * mode.
 * @param  clockfreq: UART clock.
 * @param  baud_rate: Baud rate set by the user.
 * @param  prescaler: UART prescaler value.
 * @retval Division result.
 */
static uint32_t uart_div_sampling8(unsigned long clockfreq,
				   uint32_t baud_rate,
				   uint32_t prescaler)
{
	uint32_t scaled_freq = clockfreq / presc_table[prescaler];

	return ((scaled_freq * 2) + (baud_rate / 2)) / baud_rate;

}

/* @brief  BRR division operation to set BRR register in 16-bit oversampling
 * mode.
 * @param  clockfreq: UART clock.
 * @param  baud_rate: Baud rate set by the user.
 * @param  prescaler: UART prescaler value.
 * @retval Division result.
 */
static uint32_t uart_div_sampling16(unsigned long clockfreq,
				    uint32_t baud_rate,
				    uint32_t prescaler)
{
	uint32_t scaled_freq = clockfreq / presc_table[prescaler];

	return (scaled_freq + (baud_rate / 2)) / baud_rate;

}

/*
 * @brief  Return the UART clock frequency.
 * @param  huart: UART handle.
 * @retval Frequency value in Hz.
 */
static unsigned long uart_get_clock_freq(struct stm32_uart_handle_s *huart)
{
	return fdt_get_uart_clock_freq((uintptr_t)huart->base);
}

/*
 * @brief  Configure the UART peripheral.
 * @param  huart: UART handle.
 * @retval UART status.
 */
static int uart_set_config(struct stm32_uart_handle_s *huart,
			   const struct stm32_uart_init_s *init)
{
	uint32_t tmpreg;
	unsigned long clockfreq;
	uint32_t brrtemp;

	/*
	 * ---------------------- USART CR1 Configuration --------------------
	 * Clear M, PCE, PS, TE, RE and OVER8 bits and configure
	 * the UART word length, parity, mode and oversampling:
	 * - set the M bits according to init->word_length value,
	 * - set PCE and PS bits according to init->parity value,
	 * - set TE and RE bits according to init->mode value,
	 * - set OVER8 bit according to init->over_sampling value.
	 */
	tmpreg = init->word_length |
		 init->parity |
		 init->mode |
		 init->over_sampling |
		 init->fifo_mode;
	mmio_clrsetbits_32(huart->base + USART_CR1, STM32_UART_CR1_FIELDS, tmpreg);

	/*
	 * --------------------- USART CR2 Configuration ---------------------
	 * Configure the UART Stop Bits: Set STOP[13:12] bits according
	 * to init->stop_bits value.
	 */
	mmio_clrsetbits_32(huart->base + USART_CR2, STM32_UART_CR2_FIELDS,
			   init->stop_bits);

	/*
	 * --------------------- USART CR3 Configuration ---------------------
	 * Configure:
	 * - UART HardWare Flow Control: set CTSE and RTSE bits according
	 *   to init->hw_flow_control value,
	 * - one-bit sampling method versus three samples' majority rule
	 *   according to init->one_bit_sampling (not applicable to
	 *   LPUART),
	 * - set TXFTCFG bit according to init->tx_fifo_threshold value,
	 * - set RXFTCFG bit according to init->rx_fifo_threshold value.
	 */
	tmpreg = init->hw_flow_control | init->one_bit_sampling;

	if (init->fifo_mode == USART_CR1_FIFOEN) {
		tmpreg |= init->tx_fifo_threshold |
			  init->rx_fifo_threshold;
	}

	mmio_clrsetbits_32(huart->base + USART_CR3, STM32_UART_CR3_FIELDS, tmpreg);

	/*
	 * --------------------- USART PRESC Configuration -------------------
	 * Configure UART Clock Prescaler : set PRESCALER according to
	 * init->prescaler value.
	 */
	assert(init->prescaler < STM32_UART_PRESCALER_NB);
	mmio_clrsetbits_32(huart->base + USART_PRESC, USART_PRESC_PRESCALER,
			   init->prescaler);

	/*---------------------- USART BRR configuration --------------------*/
	clockfreq = uart_get_clock_freq(huart);
	if (clockfreq == 0UL) {
		return -ENODEV;
	}

	if (init->over_sampling == STM32_UART_OVERSAMPLING_8) {
		uint32_t usartdiv = uart_div_sampling8(clockfreq,
						       init->baud_rate,
						       init->prescaler);

		brrtemp = (usartdiv & USART_BRR_DIV_MANTISSA) |
			  ((usartdiv & USART_BRR_DIV_FRACTION) >> 1);
	} else {
		brrtemp = uart_div_sampling16(clockfreq,
					      init->baud_rate,
					      init->prescaler) &
			  (USART_BRR_DIV_FRACTION | USART_BRR_DIV_MANTISSA);
	}
	mmio_write_32(huart->base + USART_BRR, brrtemp);

	return 0;
}

/*
 * @brief  Handle UART communication timeout.
 * @param  huart: UART handle.
 * @param  flag: Specifies the UART flag to check.
 * @retval UART status.
 */
static int stm32_uart_wait_flag(struct stm32_uart_handle_s *huart, uint32_t flag)
{
	uint64_t timeout_ref = timeout_init_us(STM32_UART_TIMEOUT_US);

	while ((mmio_read_32(huart->base + USART_ISR) & flag) == 0U) {
		if (timeout_elapsed(timeout_ref)) {
			return -ETIMEDOUT;
		}
	}

	return 0;
}

/*
 * @brief  Check the UART idle State.
 * @param  huart: UART handle.
 * @retval UART status.
 */
static int stm32_uart_check_idle(struct stm32_uart_handle_s *huart)
{
	int ret;

	/* Check if the transmitter is enabled */
	if ((mmio_read_32(huart->base + USART_CR1) & USART_CR1_TE) == USART_CR1_TE) {
		ret = stm32_uart_wait_flag(huart, USART_ISR_TEACK);
		if (ret != 0) {
			return ret;
		}
	}

	/* Check if the receiver is enabled */
	if ((mmio_read_32(huart->base + USART_CR1) & USART_CR1_RE) == USART_CR1_RE) {
		ret = stm32_uart_wait_flag(huart, USART_ISR_REACK);
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

/*
 * @brief  Compute RDR register mask depending on word length.
 * @param  huart: UART handle.
 * @retval Mask value.
 */
static unsigned int stm32_uart_rdr_mask(const struct stm32_uart_init_s *init)
{
	unsigned int mask = 0U;

	switch (init->word_length) {
	case STM32_UART_WORDLENGTH_9B:
		mask = GENMASK(8, 0);
		break;
	case STM32_UART_WORDLENGTH_8B:
		mask = GENMASK(7, 0);
		break;
	case STM32_UART_WORDLENGTH_7B:
		mask = GENMASK(6, 0);
		break;
	default:
		break; /* not reached */
	}

	if (init->parity != STM32_UART_PARITY_NONE) {
		mask >>= 1;
	}

	return mask;
}

/*
 * @brief  Check interrupt and status errors.
 * @retval True if error detected, false otherwise.
 */
static bool stm32_uart_error_detected(struct stm32_uart_handle_s *huart)
{
	return (mmio_read_32(huart->base + USART_ISR) & STM32_UART_ISR_ERRORS) != 0U;
}

/*
 * @brief  Clear status errors.
 */
static void stm32_uart_error_clear(struct stm32_uart_handle_s *huart)
{
	mmio_write_32(huart->base + USART_ICR, STM32_UART_ISR_ERRORS);
}

/*
 * @brief  Stop the UART.
 * @param  base: UART base address.
 */
void stm32_uart_stop(uintptr_t base)
{
	mmio_clrbits_32(base + USART_CR1, USART_CR1_UE);
}

/*
 * @brief  Initialize UART.
 * @param  huart: UART handle.
 * @param  base_addr: base address of UART.
 * @param  init: UART initialization parameter.
 * @retval UART status.
 */

int stm32_uart_init(struct stm32_uart_handle_s *huart,
		    uintptr_t base_addr,
		    const struct stm32_uart_init_s *init)
{
	int ret;

	if (huart == NULL || init == NULL || base_addr == 0U) {
		return -EINVAL;
	}

	huart->base = base_addr;

	/* Disable the peripheral */
	stm32_uart_stop(huart->base);

	/* Computation of UART mask to apply to RDR register */
	huart->rdr_mask = stm32_uart_rdr_mask(init);

	/* Init the peripheral */
	ret = uart_set_config(huart, init);
	if (ret != 0) {
		return ret;
	}

	/* Enable the peripheral */
	mmio_setbits_32(huart->base + USART_CR1, USART_CR1_UE);

	/* TEACK and/or REACK to check */
	return stm32_uart_check_idle(huart);
}

/*
 * @brief  Transmit one data in no blocking mode.
 * @param  huart: UART handle.
 * @param  c: data to sent.
 * @retval UART status.
 */
int stm32_uart_putc(struct stm32_uart_handle_s *huart, int c)
{
	int ret;

	if (huart == NULL) {
		return -EINVAL;
	}

	ret = stm32_uart_wait_flag(huart, USART_ISR_TXE);
	if (ret != 0) {
		return ret;
	}

	mmio_write_32(huart->base + USART_TDR, c);
	if (stm32_uart_error_detected(huart)) {
		stm32_uart_error_clear(huart);
		return -EFAULT;
	}

	return 0;
}

/*
 * @brief  Flush TX Transmit fifo
 * @param  huart: UART handle.
 * @retval UART status.
 */
int stm32_uart_flush(struct stm32_uart_handle_s *huart)
{
	int ret;

	if (huart == NULL) {
		return -EINVAL;
	}

	ret = stm32_uart_wait_flag(huart, USART_ISR_TXE);
	if (ret != 0) {
		return ret;
	}

	return stm32_uart_wait_flag(huart, USART_ISR_TC);
}

/*
 * @brief  Receive a data in no blocking mode.
 * @retval value if >0 or UART status.
 */
int stm32_uart_getc(struct stm32_uart_handle_s *huart)
{
	uint32_t data;

	if (huart == NULL) {
		return -EINVAL;
	}

	/* Check if data is available */
	if ((mmio_read_32(huart->base + USART_ISR) & USART_ISR_RXNE) == 0U) {
		return -EAGAIN;
	}

	data = mmio_read_32(huart->base + USART_RDR) & huart->rdr_mask;

	if (stm32_uart_error_detected(huart)) {
		stm32_uart_error_clear(huart);
		return -EFAULT;
	}

	return (int)data;
}

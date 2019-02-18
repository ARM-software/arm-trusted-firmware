/*
 * Copyright (c) 2016-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32_I2C_H
#define STM32_I2C_H

#include <stdint.h>

#include <lib/utils_def.h>

/* Bit definition for I2C_CR1 register */
#define I2C_CR1_PE			BIT(0)
#define I2C_CR1_TXIE			BIT(1)
#define I2C_CR1_RXIE			BIT(2)
#define I2C_CR1_ADDRIE			BIT(3)
#define I2C_CR1_NACKIE			BIT(4)
#define I2C_CR1_STOPIE			BIT(5)
#define I2C_CR1_TCIE			BIT(6)
#define I2C_CR1_ERRIE			BIT(7)
#define I2C_CR1_DNF			GENMASK(11, 8)
#define I2C_CR1_ANFOFF			BIT(12)
#define I2C_CR1_SWRST			BIT(13)
#define I2C_CR1_TXDMAEN			BIT(14)
#define I2C_CR1_RXDMAEN			BIT(15)
#define I2C_CR1_SBC			BIT(16)
#define I2C_CR1_NOSTRETCH		BIT(17)
#define I2C_CR1_WUPEN			BIT(18)
#define I2C_CR1_GCEN			BIT(19)
#define I2C_CR1_SMBHEN			BIT(22)
#define I2C_CR1_SMBDEN			BIT(21)
#define I2C_CR1_ALERTEN			BIT(22)
#define I2C_CR1_PECEN			BIT(23)

/* Bit definition for I2C_CR2 register */
#define I2C_CR2_SADD			GENMASK(9, 0)
#define I2C_CR2_RD_WRN			BIT(10)
#define I2C_CR2_RD_WRN_OFFSET		10U
#define I2C_CR2_ADD10			BIT(11)
#define I2C_CR2_HEAD10R			BIT(12)
#define I2C_CR2_START			BIT(13)
#define I2C_CR2_STOP			BIT(14)
#define I2C_CR2_NACK			BIT(15)
#define I2C_CR2_NBYTES			GENMASK(23, 16)
#define I2C_CR2_NBYTES_OFFSET		16U
#define I2C_CR2_RELOAD			BIT(24)
#define I2C_CR2_AUTOEND			BIT(25)
#define I2C_CR2_PECBYTE			BIT(26)

/* Bit definition for I2C_OAR1 register */
#define I2C_OAR1_OA1			GENMASK(9, 0)
#define I2C_OAR1_OA1MODE		BIT(10)
#define I2C_OAR1_OA1EN			BIT(15)

/* Bit definition for I2C_OAR2 register */
#define I2C_OAR2_OA2			GENMASK(7, 1)
#define I2C_OAR2_OA2MSK			GENMASK(10, 8)
#define I2C_OAR2_OA2NOMASK		0
#define I2C_OAR2_OA2MASK01		BIT(8)
#define I2C_OAR2_OA2MASK02		BIT(9)
#define I2C_OAR2_OA2MASK03		GENMASK(9, 8)
#define I2C_OAR2_OA2MASK04		BIT(10)
#define I2C_OAR2_OA2MASK05		(BIT(8) | BIT(10))
#define I2C_OAR2_OA2MASK06		(BIT(9) | BIT(10))
#define I2C_OAR2_OA2MASK07		GENMASK(10, 8)
#define I2C_OAR2_OA2EN			BIT(15)

/* Bit definition for I2C_TIMINGR register */
#define I2C_TIMINGR_SCLL		GENMASK(7, 0)
#define I2C_TIMINGR_SCLH		GENMASK(15, 8)
#define I2C_TIMINGR_SDADEL		GENMASK(19, 16)
#define I2C_TIMINGR_SCLDEL		GENMASK(23, 20)
#define I2C_TIMINGR_PRESC		GENMASK(31, 28)

/* Bit definition for I2C_TIMEOUTR register */
#define I2C_TIMEOUTR_TIMEOUTA		GENMASK(11, 0)
#define I2C_TIMEOUTR_TIDLE		BIT(12)
#define I2C_TIMEOUTR_TIMOUTEN		BIT(15)
#define I2C_TIMEOUTR_TIMEOUTB		GENMASK(27, 16)
#define I2C_TIMEOUTR_TEXTEN		BIT(31)

/* Bit definition for I2C_ISR register */
#define I2C_ISR_TXE			BIT(0)
#define I2C_ISR_TXIS			BIT(1)
#define I2C_ISR_RXNE			BIT(2)
#define I2C_ISR_ADDR			BIT(3)
#define I2C_ISR_NACKF			BIT(4)
#define I2C_ISR_STOPF			BIT(5)
#define I2C_ISR_TC			BIT(6)
#define I2C_ISR_TCR			BIT(7)
#define I2C_ISR_BERR			BIT(8)
#define I2C_ISR_ARLO			BIT(9)
#define I2C_ISR_OVR			BIT(10)
#define I2C_ISR_PECERR			BIT(11)
#define I2C_ISR_TIMEOUT			BIT(12)
#define I2C_ISR_ALERT			BIT(13)
#define I2C_ISR_BUSY			BIT(15)
#define I2C_ISR_DIR			BIT(16)
#define I2C_ISR_ADDCODE			GENMASK(23, 17)

/* Bit definition for I2C_ICR register */
#define I2C_ICR_ADDRCF			BIT(3)
#define I2C_ICR_NACKCF			BIT(4)
#define I2C_ICR_STOPCF			BIT(5)
#define I2C_ICR_BERRCF			BIT(8)
#define I2C_ICR_ARLOCF			BIT(9)
#define I2C_ICR_OVRCF			BIT(10)
#define I2C_ICR_PECCF			BIT(11)
#define I2C_ICR_TIMOUTCF		BIT(12)
#define I2C_ICR_ALERTCF			BIT(13)

enum i2c_speed_e {
	I2C_SPEED_STANDARD,	/* 100 kHz */
	I2C_SPEED_FAST,		/* 400 kHz */
	I2C_SPEED_FAST_PLUS,	/* 1 MHz   */
};

#define STANDARD_RATE				100000
#define FAST_RATE				400000
#define FAST_PLUS_RATE				1000000

struct stm32_i2c_init_s {
	uint32_t own_address1;		/*
					 * Specifies the first device own
					 * address. This parameter can be a
					 * 7-bit or 10-bit address.
					 */

	uint32_t addressing_mode;	/*
					 * Specifies if 7-bit or 10-bit
					 * addressing mode is selected.
					 * This parameter can be a value of
					 * @ref I2C_ADDRESSING_MODE.
					 */

	uint32_t dual_address_mode;	/*
					 * Specifies if dual addressing mode is
					 * selected.
					 * This parameter can be a value of @ref
					 * I2C_DUAL_ADDRESSING_MODE.
					 */

	uint32_t own_address2;		/*
					 * Specifies the second device own
					 * address if dual addressing mode is
					 * selected. This parameter can be a
					 * 7-bit address.
					 */

	uint32_t own_address2_masks;	/*
					 * Specifies the acknowledge mask
					 * address second device own address
					 * if dual addressing mode is selected
					 * This parameter can be a value of @ref
					 * I2C_OWN_ADDRESS2_MASKS.
					 */

	uint32_t general_call_mode;	/*
					 * Specifies if general call mode is
					 * selected.
					 * This parameter can be a value of @ref
					 * I2C_GENERAL_CALL_ADDRESSING_MODE.
					 */

	uint32_t no_stretch_mode;	/*
					 * Specifies if nostretch mode is
					 * selected.
					 * This parameter can be a value of @ref
					 * I2C_NOSTRETCH_MODE.
					 */

	uint32_t rise_time;		/*
					 * Specifies the SCL clock pin rising
					 * time in nanoseconds.
					 */

	uint32_t fall_time;		/*
					 * Specifies the SCL clock pin falling
					 * time in nanoseconds.
					 */

	enum i2c_speed_e speed_mode;	/*
					 * Specifies the I2C clock source
					 * frequency mode.
					 * This parameter can be a value of @ref
					 * i2c_speed_mode_e.
					 */

	int analog_filter;		/*
					 * Specifies if the I2C analog noise
					 * filter is selected.
					 * This parameter can be 0 (filter
					 * off), all other values mean filter
					 * on.
					 */

	uint8_t digital_filter_coef;	/*
					 * Specifies the I2C digital noise
					 * filter coefficient.
					 * This parameter can be a value
					 * between 0 and
					 * STM32_I2C_DIGITAL_FILTER_MAX.
					 */
};

enum i2c_state_e {
	I2C_STATE_RESET          = 0x00U,	/* Not yet initialized       */
	I2C_STATE_READY          = 0x20U,	/* Ready for use             */
	I2C_STATE_BUSY           = 0x24U,	/* Internal process ongoing  */
	I2C_STATE_BUSY_TX        = 0x21U,	/* Data Transmission ongoing */
	I2C_STATE_BUSY_RX        = 0x22U,	/* Data Reception ongoing    */
};

enum i2c_mode_e {
	I2C_MODE_NONE   = 0x00U,	/* No active communication      */
	I2C_MODE_MASTER = 0x10U,	/* Communication in Master Mode */
	I2C_MODE_SLAVE  = 0x20U,	/* Communication in Slave Mode  */
	I2C_MODE_MEM    = 0x40U		/* Communication in Memory Mode */

};

#define I2C_ERROR_NONE		0x00000000U	/* No error              */
#define I2C_ERROR_BERR		0x00000001U	/* BERR error            */
#define I2C_ERROR_ARLO		0x00000002U	/* ARLO error            */
#define I2C_ERROR_AF		0x00000004U	/* ACKF error            */
#define I2C_ERROR_OVR		0x00000008U	/* OVR error             */
#define I2C_ERROR_DMA		0x00000010U	/* DMA transfer error    */
#define I2C_ERROR_TIMEOUT	0x00000020U	/* Timeout error         */
#define I2C_ERROR_SIZE		0x00000040U	/* Size Management error */

struct i2c_handle_s {
	uint32_t i2c_base_addr;			/* Registers base address */
	unsigned int dt_status;			/* DT nsec/sec status     */
	unsigned int clock;			/* Clock reference        */
	uint8_t lock;				/* Locking object         */
	enum i2c_state_e i2c_state;		/* Communication state    */
	enum i2c_mode_e i2c_mode;		/* Communication mode     */
	uint32_t i2c_err;			/* Error code             */
};

#define I2C_ADDRESSINGMODE_7BIT		0x00000001U
#define I2C_ADDRESSINGMODE_10BIT	0x00000002U

#define I2C_DUALADDRESS_DISABLE		0x00000000U
#define I2C_DUALADDRESS_ENABLE		I2C_OAR2_OA2EN

#define I2C_GENERALCALL_DISABLE		0x00000000U
#define I2C_GENERALCALL_ENABLE		I2C_CR1_GCEN

#define I2C_NOSTRETCH_DISABLE		0x00000000U
#define I2C_NOSTRETCH_ENABLE		I2C_CR1_NOSTRETCH

#define I2C_MEMADD_SIZE_8BIT		0x00000001U
#define I2C_MEMADD_SIZE_16BIT		0x00000002U

#define I2C_RELOAD_MODE			I2C_CR2_RELOAD
#define I2C_AUTOEND_MODE		I2C_CR2_AUTOEND
#define I2C_SOFTEND_MODE		0x00000000U

#define I2C_NO_STARTSTOP		0x00000000U
#define I2C_GENERATE_STOP		(BIT(31) | I2C_CR2_STOP)
#define I2C_GENERATE_START_READ		(BIT(31) | I2C_CR2_START | \
					 I2C_CR2_RD_WRN)
#define I2C_GENERATE_START_WRITE	(BIT(31) | I2C_CR2_START)

#define I2C_FLAG_TXE			I2C_ISR_TXE
#define I2C_FLAG_TXIS			I2C_ISR_TXIS
#define I2C_FLAG_RXNE			I2C_ISR_RXNE
#define I2C_FLAG_ADDR			I2C_ISR_ADDR
#define I2C_FLAG_AF			I2C_ISR_NACKF
#define I2C_FLAG_STOPF			I2C_ISR_STOPF
#define I2C_FLAG_TC			I2C_ISR_TC
#define I2C_FLAG_TCR			I2C_ISR_TCR
#define I2C_FLAG_BERR			I2C_ISR_BERR
#define I2C_FLAG_ARLO			I2C_ISR_ARLO
#define I2C_FLAG_OVR			I2C_ISR_OVR
#define I2C_FLAG_PECERR			I2C_ISR_PECERR
#define I2C_FLAG_TIMEOUT		I2C_ISR_TIMEOUT
#define I2C_FLAG_ALERT			I2C_ISR_ALERT
#define I2C_FLAG_BUSY			I2C_ISR_BUSY
#define I2C_FLAG_DIR			I2C_ISR_DIR

#define I2C_RESET_CR2			(I2C_CR2_SADD | I2C_CR2_HEAD10R | \
					 I2C_CR2_NBYTES | I2C_CR2_RELOAD  | \
					 I2C_CR2_RD_WRN)

#define I2C_TIMEOUT_BUSY_MS		25U

#define I2C_ANALOGFILTER_ENABLE		0x00000000U
#define I2C_ANALOGFILTER_DISABLE	I2C_CR1_ANFOFF

/* STM32 specific defines */
#define STM32_I2C_RISE_TIME_DEFAULT		25	/* ns */
#define STM32_I2C_FALL_TIME_DEFAULT		10	/* ns */
#define STM32_I2C_SPEED_DEFAULT			I2C_SPEED_STANDARD
#define STM32_I2C_ANALOG_FILTER_DELAY_MIN	50	/* ns */
#define STM32_I2C_ANALOG_FILTER_DELAY_MAX	260	/* ns */
#define STM32_I2C_DIGITAL_FILTER_MAX		16

int stm32_i2c_get_setup_from_fdt(void *fdt, int node,
				 struct stm32_i2c_init_s *init);
int stm32_i2c_init(struct i2c_handle_s *hi2c,
		   struct stm32_i2c_init_s *init_data);
int stm32_i2c_mem_write(struct i2c_handle_s *hi2c, uint16_t dev_addr,
			uint16_t mem_addr, uint16_t mem_add_size,
			uint8_t *p_data, uint16_t size, uint32_t timeout_ms);
int stm32_i2c_mem_read(struct i2c_handle_s *hi2c, uint16_t dev_addr,
		       uint16_t mem_addr, uint16_t mem_add_size,
		       uint8_t *p_data, uint16_t size, uint32_t timeout_ms);
int stm32_i2c_master_transmit(struct i2c_handle_s *hi2c, uint16_t dev_addr,
			      uint8_t *p_data, uint16_t size,
			      uint32_t timeout_ms);
int stm32_i2c_master_receive(struct i2c_handle_s *hi2c, uint16_t dev_addr,
			     uint8_t *p_data, uint16_t size,
			     uint32_t timeout_ms);
bool stm32_i2c_is_device_ready(struct i2c_handle_s *hi2c, uint16_t dev_addr,
			       uint32_t trials, uint32_t timeout_ms);

#endif /* STM32_I2C_H */

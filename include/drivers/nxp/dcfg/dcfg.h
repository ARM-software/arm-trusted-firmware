/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef DCFG_H
#define DCFG_H

#include <endian.h>

#if defined(CONFIG_CHASSIS_2)
#include <dcfg_lsch2.h>
#elif defined(CONFIG_CHASSIS_3_2)
#include <dcfg_lsch3.h>
#endif

#ifdef NXP_GUR_BE
#define gur_in32(a)		bswap32(mmio_read_32((uintptr_t)(a)))
#define gur_out32(a, v)		mmio_write_32((uintptr_t)(a), bswap32(v))
#elif defined(NXP_GUR_LE)
#define gur_in32(a)		mmio_read_32((uintptr_t)(a))
#define gur_out32(a, v)		mmio_write_32((uintptr_t)(a), v)
#else
#error Please define CCSR GUR register endianness
#endif

typedef struct {
	union {
		uint32_t val;
		struct {
			uint32_t min_ver:4;
			uint32_t maj_ver:4;
#if defined(CONFIG_CHASSIS_3) || defined(CONFIG_CHASSIS_3_2)
			uint32_t personality:6;
			uint32_t rsv1:2;
#elif defined(CONFIG_CHASSIS_2)
			uint32_t personality:8;

#endif
#if defined(CONFIG_CHASSIS_3) || defined(CONFIG_CHASSIS_3_2)
			uint32_t dev_id:6;
			uint32_t rsv2:2;
			uint32_t family:4;
#elif defined(CONFIG_CHASSIS_2)
			uint32_t dev_id:12;
#endif
			uint32_t mfr_id;
		} __packed bf;
		struct {
			uint32_t maj_min:8;
			uint32_t version; /* SoC version without major and minor info */
		} __packed bf_ver;
	} __packed svr_reg;
	bool sec_enabled;
	bool is_populated;
} soc_info_t;

typedef struct {
	bool is_populated;
	uint8_t ocram_present;
	uint8_t ddrc1_present;
#if defined(CONFIG_CHASSIS_3) || defined(CONFIG_CHASSIS_3_2)
	uint8_t ddrc2_present;
#endif
} devdisr5_info_t;

typedef struct {
	uint32_t porsr1;
	uintptr_t g_nxp_dcfg_addr;
	unsigned long nxp_sysclk_freq;
	unsigned long nxp_ddrclk_freq;
	unsigned int nxp_plat_clk_divider;
} dcfg_init_info_t;


struct sysinfo {
	unsigned long freq_platform;
	unsigned long freq_ddr_pll0;
	unsigned long freq_ddr_pll1;
};

int get_clocks(struct sysinfo *sys);

/* Read the PORSR1 register */
uint32_t read_reg_porsr1(void);

/*******************************************************************************
 * Returns true if secur eboot is enabled on board
 * mode = 0  (development mode - sb_en = 1)
 * mode = 1 (production mode - ITS = 1)
 ******************************************************************************/
bool check_boot_mode_secure(uint32_t *mode);

const soc_info_t *get_soc_info();
const devdisr5_info_t *get_devdisr5_info();

void dcfg_init(dcfg_init_info_t *dcfg_init_data);
bool is_sec_enabled(void);

void error_handler(int error_code);
#endif /*	DCFG_H	*/

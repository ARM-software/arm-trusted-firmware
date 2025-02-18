/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LPM_STB_H
#define LPM_STB_H

#include <plat/common/platform.h>

/**
 * @brief function to copy A53 stub from DDR to WKUP SRAM
 *
 * Return: 0 if all goes well, else failure
 */
int32_t k3_lpm_stub_copy_to_sram(void);

/**
 * @brief low power mode resume c function
 *
 */
void k3_lpm_resume_c(void);

/**
 * @brief suspending the device into S2R state
 *
 */
void k3_suspend_to_ram(uint32_t mode);

/**
 * @brief Enable or disable low power mode wake up sources
 *
 * @param enable bool to chose between enable and disable
 */
void k3_config_wake_sources(bool enable);

/**
 * @brief function to jump to A53 stub in WKUP SRAM during low power mode
 *
 */
void k3_lpm_stub_entry(uint32_t mode);

/**
 * @brief function to program the magic words in WKUP CTRL MMR
 *
 */
void k3_lpm_config_magic_words(uint32_t mode);

/**
 * @brief function to check whether CAN IO magic words are latched
 *
 */
bool k3_lpm_check_can_io_latch(void);

/**
 * @brief function to set/remove the IO isolation
 *
 */
int32_t k3_lpm_set_io_isolation(bool enable);

/**
 * @brief abort function during lpm sequence
 *
 */
__wkupsramfunc void lpm_abort(void);

#endif /* LPM_STB_H */

/*
 * Copyright (c) 2024-2026, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LPM_STB_H
#define LPM_STB_H

#include <plat/common/platform.h>

/**
 * @brief Function to copy A53 stub from DDR to WKUP SRAM
 *
 * @return 0 on success, error code otherwise
 */
int32_t k3low_lpm_stub_copy_to_sram(void);

/**
 * @brief Low power mode resume c function
 */
void k3low_lpm_resume_c(void);

/**
 * @brief Suspending the device into S2R state
 *
 * @param mode LPM mode to enter
 */
void k3low_suspend_to_ram(uint32_t mode);

/**
 * @brief Enable or disable low power mode wake up sources
 *
 * @param enable Boolean to choose between enable and disable
 */
void k3low_config_wake_sources(bool enable);

/**
 * @brief Function to jump to A53 stub in WKUP SRAM during low power mode
 *
 * @param mode LPM mode to enter
 */
void k3low_lpm_stub_entry(uint32_t mode);

/**
 * @brief Function to program the magic words in WKUP CTRL MMR
 *
 * @param mode LPM mode to enter
 */
void k3low_lpm_config_magic_words(uint32_t mode);

/**
 * @brief Function to check whether CAN IO magic words are latched
 *
 * @return true if latched, false otherwise
 */
bool k3low_lpm_check_can_io_latch(void);

/**
 * @brief Function to set/remove the IO isolation
 *
 * @param enable Boolean to set or remove isolation
 * @return 0 on success, error code otherwise
 */
int32_t k3low_lpm_set_io_isolation(bool enable);

/**
 * @brief Abort function during lpm sequence
 */
__wkupsramfunc void k3low_lpm_abort(void);

#endif /* LPM_STB_H */

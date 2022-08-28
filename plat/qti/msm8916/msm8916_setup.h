/*
 * Copyright (c) 2022-2023, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MSM8916_SETUP_H
#define MSM8916_SETUP_H

void msm8916_early_platform_setup(void);
void msm8916_plat_arch_setup(uintptr_t base, size_t size);
void msm8916_platform_setup(void);

#endif /* MSM8916_SETUP_H */

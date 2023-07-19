/*
 * Copyright (c) 2021-2022, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MSM8916_PM_H
#define MSM8916_PM_H

void msm8916_cpu_boot(uintptr_t acs);
void msm8916_l2_boot(uintptr_t base);

#endif /* MSM8916_PM_H */

/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/qti/pdc/pdc_internal.h>
#include <lib/utils_def.h>

/* Valid GPIO mux input numbers for lemans (qcs9075) APSS */
struct pdc_gpio_inputs g_pdc_gpio_inputs[] = {
	/* Input 0 */
	{ 145, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_145_mx */
	{   0, PDC_GPIO_INVALID }, /* open */
	{ 112, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_112_mx */
	{  39, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_39_mx */

	/* Input 4 */
	{  86, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_86_mx */
	{  48, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_48_mx */
	{  91, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_91_mx */
	{  34, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_34_mx */

	/* Input 8 */
	{  12, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_12_mx */
	{  15, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_15_mx */
	{  16, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_16_mx */
	{  18, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_18_mx */

	/* Input 12 */
	{  20, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_20_mx */
	{  21, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_21_mx */
	{  55, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_55_mx */
	{  43, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_43_mx */

	/* Input 16 */
	{  24, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_24_mx */
	{ 150, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_150_mx */
	{ 151, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_151_mx */
	{ 153, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_153_mx */

	/* Input 20 */
	{  31, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_31_mx */
	{   0, PDC_GPIO_INVALID }, /* wlan2host_sol_aoss_wkp */
	{  35, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_35_mx */
	{  11, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_11_mx */

	/* Input 24 */
	{ 130, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_130_mx */
	{  25, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_25_mx */
	{ 102, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_102_mx */
	{  40, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_40_mx */

	/* Input 28 */
	{  41, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_41_mx */
	{  23, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_23_mx */
	{  44, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_44_mx */
	{  45, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_45_mx */

	/* Input 32 */
	{  47, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_47_mx */
	{   7, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_7_mx */
	{  63, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_63_mx */
	{  78, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_78_mx */

	/* Input 36 */
	{  79, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_79_mx */
	{  22, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_22_mx */
	{  56, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_56_mx */
	{   0, PDC_GPIO_INVALID }, /* core_bi_px_core_in_mx_sdc1_data_1 */

	/* Input 40 */
	{  59, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_59_mx */
	{  60, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_60_mx */
	{  51, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_51_mx */
	{ 142, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_142_mx */

	/* Input 44 */
	{ 104, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_104_mx */
	{ 141, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_141_mx */
	{ 103, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_103_mx */
	{  54, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_54_mx */

	/* Input 48 */
	{  80, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_80_mx */
	{  81, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_81_mx */
	{ 158, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_158_mx */
	{   4, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_4_mx */

	/* Input 52 */
	{  83, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_83_mx */
	{ 143, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_143_mx */
	{  89, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_89_mx */
	{  77, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_77_mx */

	/* Input 56 */
	{ 101, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_101_mx */
	{  92, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_92_mx */
	{  93, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_93_mx */
	{ 156, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_156_mx */

	/* Input 60 */
	{ 145, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_145_mx */
	{   3, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_3_mx */
	{ 148, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_148_mx */
	{  75, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_75_mx */

	/* Input 64 */
	{   0, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_0_mx */
	{ 116, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_116_mx */
	{ 161, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_161_mx */
	{ 119, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_119_mx */

	/* Input 68 */
	{ 121, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_121_mx */
	{ 123, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_123_mx */
	{ 125, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_125_mx */
	{ 127, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_127_mx */

	/* Input 72 */
	{ 163, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_163_mx */
	{ 129, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_129_mx */
	{  32, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_32_mx */
	{ 131, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_131_mx */

	/* Input 76 */
	{ 133, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_133_mx */
	{ 136, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_136_mx */
	{ 140, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_140_mx */
	{  90, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_90_mx */

	/* Input 80 */
	{  72, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_72_mx */
	{   0, PDC_GPIO_INVALID }, /* core_bi_px_core_in_mx_sdc2_cmd */
	{   0, PDC_GPIO_INVALID }, /* core_bi_px_core_in_mx_sdc2_data_1 */
	{   0, PDC_GPIO_INVALID }, /* core_bi_px_core_in_mx_sdc2_data_3 */

	/* Input 84 */
	{  88, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_88_mx */
	{   8, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_8_mx */
	{  52, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_52_mx */
	{  36, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_36_mx */

	/* Input 88 */
	{  27, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_27_mx */
	{  28, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_28_mx */
	{  95, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_95_mx */
	{ 157, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_157_mx */

	/* Input 92 */
	{  82, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_82_mx */
	{ 117, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_117_mx */
	{ 167, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_167_mx */
	{ 128, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_128_mx */

	/* Input 96 */
	{ 172, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_172_mx */
	{ 174, PDC_GPIO_INVALID }, /* to_aoss_core_in_mx_gpio_174_mx */
};

const uint32_t g_pdc_gpio_input_size = ARRAY_SIZE(g_pdc_gpio_inputs);

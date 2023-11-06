/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include "cpu_errata_info.h"
#include <lib/smccc.h>
#include <lib/utils_def.h>
#include <services/errata_abi_svc.h>
#include <smccc_helpers.h>

/*
 * Global pointer that points to the specific
 * structure based on the MIDR part number
 */
struct em_cpu_list *cpu_ptr;

extern uint8_t cpu_get_rev_var(void);

/* Structure array that holds CPU specific errata information */
struct em_cpu_list cpu_list[] = {
#if CORTEX_A9_H_INC
{
	.cpu_partnumber = CORTEX_A9_MIDR,
	.cpu_errata_list = {
		[0] = {794073, 0x00, 0xFF, ERRATA_A9_794073},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A9_H_INC */

#if CORTEX_A15_H_INC
{
	.cpu_partnumber = CORTEX_A15_MIDR,
	.cpu_errata_list = {
		[0] = {816470, 0x30, 0xFF, ERRATA_A15_816470},
		[1] = {827671, 0x30, 0xFF, ERRATA_A15_827671},
		[2 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A15_H_INC */

#if CORTEX_A17_H_INC
{
	.cpu_partnumber = CORTEX_A17_MIDR,
	.cpu_errata_list = {
		[0] = {852421, 0x00, 0x12, ERRATA_A17_852421},
		[1] = {852423, 0x00, 0x12, ERRATA_A17_852423},
		[2 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A17_H_INC */

#if CORTEX_A35_H_INC
{
	.cpu_partnumber = CORTEX_A35_MIDR,
	.cpu_errata_list = {
		[0] = {855472, 0x00, 0x00, ERRATA_A35_855472},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A35_H_INC */

#if CORTEX_A53_H_INC
{
	.cpu_partnumber = CORTEX_A53_MIDR,
	.cpu_errata_list = {
		[0] = {819472, 0x00, 0x01, ERRATA_A53_819472},
		[1] = {824069, 0x00, 0x02, ERRATA_A53_824069},
		[2] = {826319, 0x00, 0x02, ERRATA_A53_826319},
		[3] = {827319, 0x00, 0x02, ERRATA_A53_827319},
		[4] = {835769, 0x00, 0x04, ERRATA_A53_835769},
		[5] = {836870, 0x00, 0x03, ERRATA_A53_836870},
		[6] = {843419, 0x00, 0x04, ERRATA_A53_843419},
		[7] = {855873, 0x03, 0xFF, ERRATA_A53_855873},
		[8] = {1530924, 0x00, 0xFF, ERRATA_A53_1530924},
		[9 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A53_H_INC */

#if CORTEX_A55_H_INC
{
	.cpu_partnumber = CORTEX_A55_MIDR,
	.cpu_errata_list = {
		[0] = {768277, 0x00, 0x00, ERRATA_A55_768277},
		[1] = {778703, 0x00, 0x00, ERRATA_A55_778703},
		[2] = {798797, 0x00, 0x00, ERRATA_A55_798797},
		[3] = {846532, 0x00, 0x01, ERRATA_A55_846532},
		[4] = {903758, 0x00, 0x01, ERRATA_A55_903758},
		[5] = {1221012, 0x00, 0x10, ERRATA_A55_1221012},
		[6] = {1530923, 0x00, 0xFF, ERRATA_A55_1530923},
		[7 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A55_H_INC */

#if CORTEX_A57_H_INC
{
	.cpu_partnumber = CORTEX_A57_MIDR,
	.cpu_errata_list = {
		[0] = {806969, 0x00, 0x00, ERRATA_A57_806969},
		[1] = {813419, 0x00, 0x00, ERRATA_A57_813419},
		[2] = {813420, 0x00, 0x00, ERRATA_A57_813420},
		[3] = {814670, 0x00, 0x00, ERRATA_A57_814670},
		[4] = {817169, 0x00, 0x01, ERRATA_A57_817169},
		[5] = {826974, 0x00, 0x11, ERRATA_A57_826974},
		[6] = {826977, 0x00, 0x11, ERRATA_A57_826977},
		[7] = {828024, 0x00, 0x11, ERRATA_A57_828024},
		[8] = {829520, 0x00, 0x12, ERRATA_A57_829520},
		[9] = {833471, 0x00, 0x12, ERRATA_A57_833471},
		[10] = {859972, 0x00, 0x13, ERRATA_A57_859972},
		[11] = {1319537, 0x00, 0xFF, ERRATA_A57_1319537},
		[12 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A57_H_INC */

#if CORTEX_A72_H_INC
{
	.cpu_partnumber = CORTEX_A72_MIDR,
	.cpu_errata_list = {
		[0] = {859971, 0x00, 0x03, ERRATA_A72_859971},
		[1] = {1319367, 0x00, 0xFF, ERRATA_A72_1319367},
		[2 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A72_H_INC */

#if CORTEX_A73_H_INC
{
	.cpu_partnumber = CORTEX_A73_MIDR,
	.cpu_errata_list = {
		[0] = {852427, 0x00, 0x00, ERRATA_A73_852427},
		[1] = {855423, 0x00, 0x01, ERRATA_A73_855423},
		[2 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A73_H_INC */

#if CORTEX_A75_H_INC
{
	.cpu_partnumber = CORTEX_A75_MIDR,
	.cpu_errata_list = {
		[0] = {764081, 0x00, 0x00, ERRATA_A75_764081},
		[1] = {790748, 0x00, 0x00, ERRATA_A75_790748},
		[2 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A75_H_INC */

#if CORTEX_A76_H_INC
{
	.cpu_partnumber = CORTEX_A76_MIDR,
	.cpu_errata_list = {
		[0] = {1073348, 0x00, 0x10, ERRATA_A76_1073348},
		[1] = {1130799, 0x00, 0x20, ERRATA_A76_1130799},
		[2] = {1165522, 0x00, 0xFF, ERRATA_A76_1165522},
		[3] = {1220197, 0x00, 0x20, ERRATA_A76_1220197},
		[4] = {1257314, 0x00, 0x30, ERRATA_A76_1257314},
		[5] = {1262606, 0x00, 0x30, ERRATA_A76_1262606},
		[6] = {1262888, 0x00, 0x30, ERRATA_A76_1262888},
		[7] = {1275112, 0x00, 0x30, ERRATA_A76_1275112},
		[8] = {1286807, 0x00, 0x30, ERRATA_A76_1286807},
		[9] = {1791580, 0x00, 0x40, ERRATA_A76_1791580},
		[10] = {1868343, 0x00, 0x40, ERRATA_A76_1868343},
		[11] = {1946160, 0x30, 0x41, ERRATA_A76_1946160},
		[12] = {2743102, 0x00, 0x41, ERRATA_A76_2743102},
		[13 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A76_H_INC */

#if CORTEX_A77_H_INC
{
	.cpu_partnumber = CORTEX_A77_MIDR,
	.cpu_errata_list = {
		[0] = {1508412, 0x00, 0x10, ERRATA_A77_1508412},
		[1] = {1791578, 0x00, 0x11, ERRATA_A77_1791578},
		[2] = {1800714, 0x00, 0x11, ERRATA_A77_1800714},
		[3] = {1925769, 0x00, 0x11, ERRATA_A77_1925769},
		[4] = {1946167, 0x00, 0x11, ERRATA_A77_1946167},
		[5] = {2356587, 0x00, 0x11, ERRATA_A77_2356587},
		[6] = {2743100, 0x00, 0x11, ERRATA_A77_2743100},
		[7 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A77_H_INC */

#if CORTEX_A78_H_INC
{
	.cpu_partnumber = CORTEX_A78_MIDR,
	.cpu_errata_list = {
		[0] = {1688305, 0x00, 0x10, ERRATA_A78_1688305},
		[1] = {1821534, 0x00, 0x10, ERRATA_A78_1821534},
		[2] = {1941498, 0x00, 0x11, ERRATA_A78_1941498},
		[3] = {1951500, 0x10, 0x11, ERRATA_A78_1951500},
		[4] = {1952683, 0x00, 0x00, ERRATA_A78_1952683},
		[5] = {2132060, 0x00, 0x12, ERRATA_A78_2132060},
		[6] = {2242635, 0x10, 0x12, ERRATA_A78_2242635},
		[7] = {2376745, 0x00, 0x12, ERRATA_A78_2376745},
		[8] = {2395406, 0x00, 0x12, ERRATA_A78_2395406},
		[9] = {2712571, 0x00, 0x12, ERRATA_A78_2712571, \
			ERRATA_NON_ARM_INTERCONNECT},
		[10] = {2742426, 0x00, 0x12, ERRATA_A78_2742426},
		[11] = {2772019, 0x00, 0x12, ERRATA_A78_2772019},
		[12] = {2779479, 0x00, 0x12, ERRATA_A78_2779479},
		[13 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A78_H_INC */

#if CORTEX_A78_AE_H_INC
{
	.cpu_partnumber = CORTEX_A78_AE_MIDR,
	.cpu_errata_list = {
		[0] = {1941500, 0x00, 0x01, ERRATA_A78_AE_1941500},
		[1] = {1951502, 0x00, 0x01, ERRATA_A78_AE_1951502},
		[2] = {2376748, 0x00, 0x02, ERRATA_A78_AE_2376748},
		[3] = {2395408, 0x00, 0x01, ERRATA_A78_AE_2395408},
		[4] = {2712574, 0x00, 0x02, ERRATA_A78_AE_2712574, \
			ERRATA_NON_ARM_INTERCONNECT},
		[5 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A78_AE_H_INC */

#if CORTEX_A78C_H_INC
{
	.cpu_partnumber = CORTEX_A78C_MIDR,
	.cpu_errata_list = {
		[0] = {1827430, 0x00, 0x00, ERRATA_A78C_1827430},
		[1] = {1827440, 0x00, 0x00, ERRATA_A78C_1827440},
		[2] = {2132064, 0x01, 0x02, ERRATA_A78C_2132064},
		[3] = {2242638, 0x01, 0x02, ERRATA_A78C_2242638},
		[4] = {2376749, 0x01, 0x02, ERRATA_A78C_2376749},
		[5] = {2395411, 0x01, 0x02, ERRATA_A78C_2395411},
		[6] = {2712575, 0x01, 0x02, ERRATA_A78C_2712575, \
			ERRATA_NON_ARM_INTERCONNECT},
		[7] = {2772121, 0x00, 0x02, ERRATA_A78C_2772121},
		[8] = {2779484, 0x01, 0x02, ERRATA_A78C_2779484},
		[9 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A78C_H_INC */

#if CORTEX_X1_H_INC
{
	.cpu_partnumber = CORTEX_X1_MIDR,
	.cpu_errata_list = {
		[0] = {1688305, 0x00, 0x10, ERRATA_X1_1688305},
		[1] = {1821534, 0x00, 0x10, ERRATA_X1_1821534},
		[2] = {1827429, 0x00, 0x10, ERRATA_X1_1827429},
		[3 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_X1_H_INC */

#if NEOVERSE_N1_H_INC
{
	.cpu_partnumber = NEOVERSE_N1_MIDR,
	.cpu_errata_list = {
		[0] = {1043202, 0x00, 0x10, ERRATA_N1_1043202},
		[1] = {1073348, 0x00, 0x10, ERRATA_N1_1073348},
		[2] = {1130799, 0x00, 0x20, ERRATA_N1_1130799},
		[3] = {1165347, 0x00, 0x20, ERRATA_N1_1165347},
		[4] = {1207823, 0x00, 0x20, ERRATA_N1_1207823},
		[5] = {1220197, 0x00, 0x20, ERRATA_N1_1220197},
		[6] = {1257314, 0x00, 0x30, ERRATA_N1_1257314},
		[7] = {1262606, 0x00, 0x30, ERRATA_N1_1262606},
		[8] = {1262888, 0x00, 0x30, ERRATA_N1_1262888},
		[9] = {1275112, 0x00, 0x30, ERRATA_N1_1275112},
		[10] = {1315703, 0x00, 0x30, ERRATA_N1_1315703},
		[11] = {1542419, 0x30, 0x40, ERRATA_N1_1542419},
		[12] = {1868343, 0x00, 0x40, ERRATA_N1_1868343},
		[13] = {1946160, 0x30, 0x41, ERRATA_N1_1946160},
		[14] = {2743102, 0x00, 0x41, ERRATA_N1_2743102},
		[15 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* NEOVERSE_N1_H_INC */

#if NEOVERSE_V1_H_INC
{
	.cpu_partnumber = NEOVERSE_V1_MIDR,
	.cpu_errata_list = {
		[0] = {1618635, 0x00, 0x00, ERRATA_V1_1618635},
		[1] = {1774420, 0x00, 0x10, ERRATA_V1_1774420},
		[2] = {1791573, 0x00, 0x10, ERRATA_V1_1791573},
		[3] = {1852267, 0x00, 0x10, ERRATA_V1_1852267},
		[4] = {1925756, 0x00, 0x11, ERRATA_V1_1925756},
		[5] = {1940577, 0x10, 0x11, ERRATA_V1_1940577},
		[6] = {1966096, 0x10, 0x11, ERRATA_V1_1966096},
		[7] = {2108267, 0x00, 0x12, ERRATA_V1_2108267},
		[8] = {2139242, 0x00, 0x11, ERRATA_V1_2139242},
		[9] = {2216392, 0x10, 0x11, ERRATA_V1_2216392},
		[10] = {2294912, 0x00, 0x12, ERRATA_V1_2294912},
		[11] = {2372203, 0x00, 0x11, ERRATA_V1_2372203},
		[12] = {2701953, 0x00, 0x11, ERRATA_V1_2701953, \
			ERRATA_NON_ARM_INTERCONNECT},
		[13] = {2743093, 0x00, 0x12, ERRATA_V1_2743093},
		[14] = {2743233, 0x00, 0x12, ERRATA_V1_2743233},
		[15] = {2779461, 0x00, 0x12, ERRATA_V1_2779461},
		[16 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* NEOVERSE_V1_H_INC */

#if CORTEX_A710_H_INC
{
	.cpu_partnumber = CORTEX_A710_MIDR,
	.cpu_errata_list = {
		[0] = {1987031, 0x00, 0x20, ERRATA_A710_1987031},
		[1] = {2008768, 0x00, 0x20, ERRATA_A710_2008768},
		[2] = {2017096, 0x00, 0x20, ERRATA_A710_2017096},
		[3] = {2055002, 0x10, 0x20, ERRATA_A710_2055002},
		[4] = {2058056, 0x00, 0x21, ERRATA_A710_2058056},
		[5] = {2081180, 0x00, 0x20, ERRATA_A710_2081180},
		[6] = {2083908, 0x20, 0x20, ERRATA_A710_2083908},
		[7] = {2136059, 0x00, 0x20, ERRATA_A710_2136059},
		[8] = {2147715, 0x20, 0x20, ERRATA_A710_2147715},
		[9] = {2216384, 0x00, 0x20, ERRATA_A710_2216384},
		[10] = {2267065, 0x00, 0x20, ERRATA_A710_2267065},
		[11] = {2282622, 0x00, 0x21, ERRATA_A710_2282622},
		[12] = {2291219, 0x00, 0x20, ERRATA_A710_2291219},
		[13] = {2371105, 0x00, 0x20, ERRATA_A710_2371105},
		[14] = {2701952, 0x00, 0x21, ERRATA_A710_2701952, \
			ERRATA_NON_ARM_INTERCONNECT},
		[15] = {2742423, 0x00, 0x21, ERRATA_A710_2742423},
		[16] = {2768515, 0x00, 0x21, ERRATA_A710_2768515},
		[17 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A710_H_INC */

#if NEOVERSE_N2_H_INC
{
	.cpu_partnumber = NEOVERSE_N2_MIDR,
	.cpu_errata_list = {
		[0] = {2002655, 0x00, 0x00, ERRATA_N2_2002655},
		[1] = {2009478, 0x00, 0x00, ERRATA_N2_2009478},
		[2] = {2025414, 0x00, 0x00, ERRATA_N2_2025414},
		[3] = {2067956, 0x00, 0x00, ERRATA_N2_2067956},
		[4] = {2138953, 0x00, 0x03, ERRATA_N2_2138953},
		[5] = {2138956, 0x00, 0x00, ERRATA_N2_2138956},
		[6] = {2138958, 0x00, 0x00, ERRATA_N2_2138958},
		[7] = {2189731, 0x00, 0x00, ERRATA_N2_2189731},
		[8] = {2242400, 0x00, 0x00, ERRATA_N2_2242400},
		[9] = {2242415, 0x00, 0x00, ERRATA_N2_2242415},
		[10] = {2280757, 0x00, 0x00, ERRATA_N2_2280757},
		[11] = {2326639, 0x00, 0x00, ERRATA_N2_2326639},
		[12] = {2340933, 0x00, 0x00, ERRATA_N2_2340933},
		[13] = {2346952, 0x00, 0x02, ERRATA_N2_2346952},
		[14] = {2376738, 0x00, 0x00, ERRATA_N2_2376738},
		[15] = {2388450, 0x00, 0x00, ERRATA_N2_2388450},
		[16] = {2728475, 0x00, 0x02, ERRATA_N2_2728475, \
			ERRATA_NON_ARM_INTERCONNECT},
		[17] = {2743014, 0x00, 0x02, ERRATA_N2_2743014},
		[18] = {2743089, 0x00, 0x02, ERRATA_N2_2743089},
		[19] = {2779511, 0x00, 0x02, ERRATA_N2_2779511},
		[20 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* NEOVERSE_N2_H_INC */

#if CORTEX_X2_H_INC
{
	.cpu_partnumber = CORTEX_X2_MIDR,
	.cpu_errata_list = {
		[0] = {2002765, 0x00, 0x20, ERRATA_X2_2002765},
		[1] = {2017096, 0x00, 0x20, ERRATA_X2_2017096},
		[2] = {2058056, 0x00, 0x21, ERRATA_X2_2058056},
		[3] = {2081180, 0x00, 0x20, ERRATA_X2_2081180},
		[4] = {2083908, 0x20, 0x20, ERRATA_X2_2083908},
		[5] = {2147715, 0x20, 0x20, ERRATA_X2_2147715},
		[6] = {2216384, 0x00, 0x20, ERRATA_X2_2216384},
		[7] = {2282622, 0x00, 0x21, ERRATA_X2_2282622},
		[8] = {2371105, 0x00, 0x20, ERRATA_X2_2371105},
		[9] = {2701952, 0x00, 0x21, ERRATA_X2_2701952, \
			ERRATA_NON_ARM_INTERCONNECT},
		[10] = {2742423, 0x00, 0x21, ERRATA_X2_2742423},
		[11] = {2768515, 0x00, 0x21, ERRATA_X2_2768515},
		[12 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_X2_H_INC */

#if CORTEX_A510_H_INC
{
	.cpu_partnumber = CORTEX_A510_MIDR,
	.cpu_errata_list = {
		[0] = {1922240, 0x00, 0x00, ERRATA_A510_1922240},
		[1] = {2041909, 0x02, 0x02, ERRATA_A510_2041909},
		[2] = {2042739, 0x00, 0x02, ERRATA_A510_2042739},
		[3] = {2080326, 0x02, 0x02, ERRATA_A510_2080326},
		[4] = {2172148, 0x00, 0x10, ERRATA_A510_2172148},
		[5] = {2218950, 0x00, 0x10, ERRATA_A510_2218950},
		[6] = {2250311, 0x00, 0x10, ERRATA_A510_2250311},
		[7] = {2288014, 0x00, 0x10, ERRATA_A510_2288014},
		[8] = {2347730, 0x00, 0x11, ERRATA_A510_2347730},
		[9] = {2371937, 0x00, 0x11, ERRATA_A510_2371937},
		[10] = {2666669, 0x00, 0x11, ERRATA_A510_2666669},
		[11] = {2684597, 0x00, 0x12, ERRATA_A510_2684597},
		[12 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A510_H_INC */

#if NEOVERSE_V2_H_INC
{
	.cpu_partnumber = NEOVERSE_V2_MIDR,
	.cpu_errata_list = {
		[0] = {2331132, 0x00, 0x02, ERRATA_V2_2331132},
		[1] = {2719103, 0x00, 0x01, ERRATA_V2_2719103, \
			ERRATA_NON_ARM_INTERCONNECT},
		[2] = {2719105, 0x00, 0x01, ERRATA_V2_2719105},
		[3] = {2743011, 0x00, 0x01, ERRATA_V2_2743011},
		[4] = {2779510, 0x00, 0x01, ERRATA_V2_2779510},
		[5] = {2801372, 0x00, 0x01, ERRATA_V2_2801372},
		[6 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* NEOVERSE_V2_H_INC */

#if CORTEX_A715_H_INC
{
	.cpu_partnumber = CORTEX_A715_MIDR,
	.cpu_errata_list = {
		[0] = {2701951, 0x00, 0x11, ERRATA_A715_2701951, \
			ERRATA_NON_ARM_INTERCONNECT},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A715_H_INC */

#if CORTEX_X3_H_INC
{
	.cpu_partnumber = CORTEX_X3_MIDR,
	.cpu_errata_list = {
		[0] = {2070301, 0x00, 0x12, ERRATA_X3_2070301},
		[1] = {2313909, 0x00, 0x10, ERRATA_X3_2313909},
		[2] = {2615812, 0x00, 0x11, ERRATA_X3_2615812},
		[3] = {2742421, 0x00, 0x11, ERRATA_X3_2742421},
		[4 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_X3_H_INC */
};

/*
 * Function to do binary search and check for the specific errata ID
 * in the array of structures specific to the cpu identified.
 */
int32_t binary_search(struct em_cpu_list *ptr, uint32_t erratum_id, uint8_t rxpx_val)
{
	int low_index = 0U, mid_index = 0U;

	int high_index = MAX_ERRATA_ENTRIES - 1;

	assert(ptr != NULL);

	/*
	 * Pointer to the errata list of the cpu that matches
	 * extracted partnumber in the cpu list
	 */
	struct em_cpu *erratum_ptr = NULL;

	while (low_index <= high_index) {
		mid_index = (low_index + high_index) / 2;

		erratum_ptr = &ptr->cpu_errata_list[mid_index];
		assert(erratum_ptr != NULL);

		if (erratum_id < erratum_ptr->em_errata_id) {
			high_index = mid_index - 1;
		} else if (erratum_id > erratum_ptr->em_errata_id) {
			low_index = mid_index + 1;
		} else if (erratum_id == erratum_ptr->em_errata_id) {
			if (RXPX_RANGE(rxpx_val, erratum_ptr->em_rxpx_lo, \
				erratum_ptr->em_rxpx_hi)) {
				if ((erratum_ptr->errata_enabled) && \
				(!(erratum_ptr->non_arm_interconnect))) {
					return EM_HIGHER_EL_MITIGATION;
				}
				return EM_AFFECTED;
			}
			return EM_NOT_AFFECTED;
		}
	}
	/* no matching errata ID */
	return EM_UNKNOWN_ERRATUM;
}

/* Function to check if the errata exists for the specific CPU and rxpx */
int32_t verify_errata_implemented(uint32_t errata_id, uint32_t forward_flag)
{
	/*
	 * Read MIDR value and extract the revision, variant and partnumber
	 */
	static uint32_t midr_val, cpu_partnum;
	static uint8_t  cpu_rxpx_val;
	int32_t ret_val = EM_UNKNOWN_ERRATUM;

	/* Determine the number of cpu listed in the cpu list */
	uint8_t size_cpulist = ARRAY_SIZE(cpu_list);

	/* Read the midr reg to extract cpu, revision and variant info */
	midr_val = read_midr();

	/* Extract revision and variant from the MIDR register */
	cpu_rxpx_val = cpu_get_rev_var();

	/* Extract the cpu partnumber and check if the cpu is in the cpu list */
	cpu_partnum = EXTRACT_PARTNUM(midr_val);

	for (uint8_t i = 0; i < size_cpulist; i++) {
		cpu_ptr = &cpu_list[i];
		uint16_t partnum_extracted = EXTRACT_PARTNUM(cpu_ptr->cpu_partnumber);

		if (partnum_extracted == cpu_partnum) {
			/*
			 * If the midr value is in the cpu list, binary search
			 * for the errata ID and specific revision in the list.
			 */
			ret_val = binary_search(cpu_ptr, errata_id, cpu_rxpx_val);
			break;
		}
	}
	return ret_val;
}

/* Predicate indicating that a function id is part of EM_ABI */
bool is_errata_fid(uint32_t smc_fid)
{
	return ((smc_fid == ARM_EM_VERSION) ||
		(smc_fid == ARM_EM_FEATURES) ||
		(smc_fid == ARM_EM_CPU_ERRATUM_FEATURES));

}

bool validate_spsr_mode(void)
{
	/* In AArch64, if the caller is EL1, return true */

	#if __aarch64__
		if (GET_EL(read_spsr_el3()) == MODE_EL1) {
			return true;
		}
		return false;
	#else

	/* In AArch32, if in system/svc mode, return true */
		uint8_t read_el_state = GET_M32(read_spsr());

		if ((read_el_state == (MODE32_svc)) || (read_el_state == MODE32_sys)) {
			return true;
		}
		return false;
	#endif /* __aarch64__ */
}

uintptr_t errata_abi_smc_handler(uint32_t smc_fid, u_register_t x1,
				u_register_t x2, u_register_t x3, u_register_t x4,
				void *cookie, void *handle, u_register_t flags)
{
	int32_t ret_id = EM_UNKNOWN_ERRATUM;

	switch (smc_fid) {
	case ARM_EM_VERSION:
		SMC_RET1(handle, MAKE_SMCCC_VERSION(
			EM_VERSION_MAJOR, EM_VERSION_MINOR
		));
		break; /* unreachable */
	case ARM_EM_FEATURES:
		if (is_errata_fid((uint32_t)x1)) {
			SMC_RET1(handle, EM_SUCCESS);
		}

		SMC_RET1(handle, EM_NOT_SUPPORTED);
		break; /* unreachable */
	case ARM_EM_CPU_ERRATUM_FEATURES:

		/*
		 * If the forward flag is greater than zero and the calling EL
		 * is EL1 in AArch64 or in system mode or svc mode in case of AArch32,
		 * return Invalid Parameters.
		 */
		if (((uint32_t)x2 != 0) && (validate_spsr_mode())) {
			SMC_RET1(handle, EM_INVALID_PARAMETERS);
		}
		ret_id = verify_errata_implemented((uint32_t)x1, (uint32_t)x2);
		SMC_RET1(handle, ret_id);
		break; /* unreachable */
	default:
		{
		   WARN("Unimplemented Errata ABI Service Call: 0x%x\n", smc_fid);
		   SMC_RET1(handle, EM_UNKNOWN_ERRATUM);
		   break; /* unreachable */
		}
	}
}

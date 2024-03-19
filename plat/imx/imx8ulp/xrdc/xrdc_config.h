/*
 * Copyright 2020-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <xrdc.h>

#define SP(X)		((X) << 9)
#define SU(X)		((X) << 6)
#define NP(X)		((X) << 3)
#define NU(X)		((X) << 0)

#define RWX		7
#define RW		6
#define R		4
#define X		1

struct xrdc_mda_config imx8ulp_mda[] = {
	{ 0, 7, MDA_SA_PT }, /* A core */
	{ 1, 1, MDA_SA_NS }, /* DMA1 */
	{ 2, 1, MDA_SA_NS }, /* USB */
	{ 3, 1, MDA_SA_NS }, /* PXP-> .M10 */
	{ 4, 1, MDA_SA_NS }, /* ENET */
	{ 5, 1, MDA_SA_PT }, /* CAAM */
	{ 6, 1, MDA_SA_NS }, /* USDHC0 */
	{ 7, 1, MDA_SA_NS }, /* USDHC1 */
	{ 8, 1, MDA_SA_NS }, /* USDHC2 */
	{ 9, 2, MDA_SA_NS }, /* HIFI4 */
	{ 10, 3, MDA_SA_NS }, /* GPU3D */
	{ 11, 3, MDA_SA_NS }, /* GPU2D */
	{ 12, 3, MDA_SA_NS }, /* EPDC */
	{ 13, 3, MDA_SA_NS }, /* DCNano */
	{ 14, 3, MDA_SA_NS }, /* ISI */
	{ 15, 3, MDA_SA_NS }, /* PXP->NIC_LPAV.M0 */
	{ 16, 3, MDA_SA_NS }, /* DMA2 */
};

#ifdef SPD_opteed
#define TEE_SHM_SIZE 0x400000
#else
#define TEE_SHM_SIZE 0x0
#endif

#if defined(SPD_opteed) || defined(SPD_trusty)
#define DRAM_MEM_0_START (0x80000000)
#define DRAM_MEM_0_SIZE (BL32_BASE - 0x80000000)

#define DRAM_MEM_1_START (BL32_BASE)
#define DRAM_MEM_1_SIZE (BL32_SIZE - TEE_SHM_SIZE)

#ifndef SPD_trusty
#define DRAM_MEM_2_START (DRAM_MEM_1_START + DRAM_MEM_1_SIZE)
#define DRAM_MEM_2_SIZE (0x80000000 - DRAM_MEM_1_SIZE - DRAM_MEM_0_SIZE)
#else
#define SECURE_HEAP_START   (0xA9600000)
#define SECURE_HEAP_SIZE    (0x6000000)
#define DRAM_MEM_END        (0x100000000)

#define DRAM_MEM_2_START (DRAM_MEM_1_START + DRAM_MEM_1_SIZE)
#define DRAM_MEM_2_SIZE  (SECURE_HEAP_START - DRAM_MEM_2_START)
#define DRAM_MEM_3_START (DRAM_MEM_2_START + DRAM_MEM_2_SIZE)
#define DRAM_MEM_3_SIZE  (SECURE_HEAP_SIZE)
#define DRAM_MEM_4_START (DRAM_MEM_3_START + DRAM_MEM_3_SIZE)
#define DRAM_MEM_4_SIZE  (DRAM_MEM_END - DRAM_MEM_4_START)
#endif
#endif

struct xrdc_mrc_config imx8ulp_mrc[] = {
	{ 0, 0, 0x0,        0x30000,    {0, 0, 0, 0, 0, 0, 0, 1}, {0xfff, 0} }, /* ROM1 */
	{ 1, 0, 0x60000000, 0x10000000, {1, 1, 0, 0, 1, 0, 1, 1}, {0xfff, 0} }, /* Flexspi2 */
	{ 2, 0, 0x22020000, 0x40000,    {1, 1, 0, 0, 1, 0, 1, 1}, {0xfff, 0} }, /* SRAM2 */
	{ 3, 0, 0x22010000, 0x10000,    {1, 1, 0, 0, 1, 0, 1, 1}, {0xfff, 0} }, /* SRAM0 */
#if defined(SPD_opteed) || defined(SPD_trusty)
	{ 4, 0, DRAM_MEM_0_START, DRAM_MEM_0_SIZE, {0, 1, 0, 0, 0, 0, 0, 1}, {0xfff, 0} }, /* DRAM for A35, DMA1, USDHC0*/
	{ 4, 1, DRAM_MEM_1_START, DRAM_MEM_1_SIZE, {0, 1, 0, 0, 0, 0, 0, 1}, {0xfc0, 0} }, /* TEE DRAM for A35, DMA1, USDHC0*/
	{ 4, 2, DRAM_MEM_2_START, DRAM_MEM_2_SIZE, {0, 1, 0, 0, 0, 0, 0, 1}, {0xfff, 0} }, /* DRAM for A35, DMA1, USDHC0*/
#ifdef SPD_trusty
	{ 4, 3, DRAM_MEM_3_START, DRAM_MEM_3_SIZE, {0, 1, 0, 0, 0, 0, 0, 1}, {0xfc0, 0} }, /* DRAM for A35, DMA1, USDHC0*/
	{ 4, 4, DRAM_MEM_4_START, DRAM_MEM_4_SIZE, {0, 1, 0, 0, 0, 0, 0, 1}, {0xfff, 0} }, /* DRAM for A35, DMA1, USDHC0*/
#endif

	{ 5, 0, DRAM_MEM_0_START, DRAM_MEM_0_SIZE, {0, 1, 0, 0, 0, 0, 0, 0}, {0xfff, 0} }, /* DRAM for NIC_PER */
	{ 5, 1, DRAM_MEM_1_START, DRAM_MEM_1_SIZE, {0, 1, 0, 0, 0, 0, 0, 0}, {0xfc0, 0} }, /* TEE DRAM for NIC_PER */
	{ 5, 2, DRAM_MEM_2_START, DRAM_MEM_2_SIZE, {0, 1, 0, 0, 0, 0, 0, 0}, {0xfff, 0} }, /* DRAM for NIC_PER */
#ifdef SPD_trusty
	{ 5, 3, DRAM_MEM_3_START, DRAM_MEM_3_SIZE, {0, 1, 0, 0, 0, 0, 0, 0}, {0xfc0, 0} }, /* DRAM for NIC_PER */
	{ 5, 4, DRAM_MEM_4_START, DRAM_MEM_4_SIZE, {0, 1, 0, 0, 0, 0, 0, 0}, {0xfff, 0} }, /* DRAM for NIC_PER */
#endif

#ifdef SPD_trusty
	{ 6, 0, DRAM_MEM_0_START, DRAM_MEM_0_SIZE, {1, 1, 0, 2, 1, 0, 1, 1}, {0xfff, 0x93f} }, /* DRAM for LPAV and RTD*/
	{ 6, 1, DRAM_MEM_1_START, DRAM_MEM_1_SIZE, {1, 1, 0, 1, 1, 0, 1, 1}, {0xfc0, 0} }, /* TEE DRAM for LPAV and RTD*/
	{ 6, 2, DRAM_MEM_2_START, DRAM_MEM_2_SIZE, {1, 1, 0, 2, 1, 0, 1, 1}, {0xfff, 0x93f} }, /* DRAM for LPAV and RTD*/
	{ 6, 3, DRAM_MEM_3_START, DRAM_MEM_3_SIZE, {1, 1, 0, 1, 1, 0, 1, 1}, {0xfc0, 0} }, /* DRAM for LPAV and RTD*/
	{ 6, 4, DRAM_MEM_4_START, DRAM_MEM_4_SIZE, {1, 1, 0, 2, 1, 0, 1, 1}, {0xfff, 0x93f} }, /* DRAM for LPAV and RTD*/
#else
	{ 6, 0, DRAM_MEM_0_START, DRAM_MEM_0_SIZE, {1, 1, 0, 1, 1, 0, 1, 1}, {0xfff, 0} }, /* DRAM for LPAV and RTD*/
	{ 6, 1, DRAM_MEM_1_START, DRAM_MEM_1_SIZE, {1, 1, 0, 1, 1, 0, 1, 1}, {0xfc0, 0} }, /* TEE DRAM for LPAV and RTD*/
	{ 6, 2, DRAM_MEM_2_START, DRAM_MEM_2_SIZE, {1, 1, 0, 1, 1, 0, 1, 1}, {0xfff, 0} }, /* DRAM for LPAV and RTD*/
#endif
#else
	{ 4, 0, 0x80000000, 0x80000000, {0, 1, 0, 0, 0, 0, 0, 1}, {0xfff, 0} }, /* DRAM for A35, DMA1, USDHC0*/
	{ 5, 0, 0x80000000, 0x80000000, {0, 1, 0, 0, 0, 0, 0, 0}, {0xfff, 0} }, /* DRAM for NIC_PER */
	{ 6, 0, 0x80000000, 0x80000000, {1, 1, 0, 1, 1, 0, 1, 1}, {0xfff, 0} }, /* DRAM for LPAV and RTD*/
#endif
	{ 7, 0, 0x80000000, 0x10000000, {0, 0, 1, 0, 0, 0, 0, 0}, {0xfff, 0} }, /* DRAM for HIFI4 */
	{ 7, 1, 0x90000000, 0x10000000, {0, 0, 1, 0, 0, 0, 0, 0}, {0xfff, 0} }, /* DRAM for HIFI4 */
	{ 8, 0, 0x21000000, 0x10000,    {1, 1, 1, 1, 1, 0, 1, 1}, {0xfff, 0} }, /* SRAM1 */
	{ 9, 0, 0x1ffc0000, 0xc0000,    {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0} }, /* SSRAM for HIFI4 */
	{ 10, 0, 0x1ffc0000, 0xc0000,   {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0} }, /* SSRAM for LPAV */
	{ 11, 0, 0x21170000, 0x10000,   {0, 0, 1, 0, 0, 0, 0, 2}, {0xfff, SP(RW) | SU(RW) | NP(RW)} }, /* HIFI4 TCM */
	{ 11, 1, 0x21180000, 0x10000,   {0, 0, 1, 0, 0, 0, 0, 2}, {SP(RW) | SU(RW) | NP(RW) | NU(RW), SP(RW) | SU(RW) | NP(RW)} }, /* HIFI4 TCM */
	{ 12, 0, 0x2d400000, 0x100000,  {0, 0, 0, 0, 0, 0, 0, 1}, {SP(RW) | SU(RW) | NP(RW) | NU(RW), 0} }, /* GIC500 */
};

struct xrdc_pac_msc_config imx8ulp_pdac[] = {
	{ 0, PAC_SLOT_ALL, {0, 7, 0, 0, 0, 0, 0, 7} }, /* PAC0 */
	{ 0, 44, {0, 7, 7, 0, 0, 0, 0, 7} }, /* PAC0 slot 44 for CGC1 */
	{ 0, 36, {0, 0, 0, 0, 0, 0, 7, 7} }, /* PAC0 slot 36 for CMC1 */
	{ 0, 41, {0, 0, 0, 0, 0, 0, 7, 7} }, /* PAC0 slot 41 for SIM_AD */
	{ 1, PAC_SLOT_ALL, {0, 7, 0, 0, 0, 0, 0, 7} }, /* PAC1 */
	{ 1, 0, {0, 7, 7, 0, 0, 0, 7, 7} }, /* PAC1 slot 0 for PCC4 */
	{ 1, 6, {0, 7, 7, 0, 0, 0, 0, 7} }, /* PAC1 slot 6 for LPUART6 */
	{ 1, 7, {0, 7, 7, 0, 0, 0, 0, 7} }, /* PAC1 slot 7 for LPUART7 */
	{ 1, 9,  {0, 7, 7, 7, 0, 0, 0, 7} }, /* SAI5 for HIFI4 and eDMA2 */
	{ 1, 12, {0, 7, 7, 0, 0, 0, 7, 7} }, /* PAC1 slot 12 for IOMUXC1 */
	{ 2, PAC_SLOT_ALL, {7, 7, 7, 7, 0, 0, 7, 7} }, /* PAC2 */
};

struct xrdc_pac_msc_config imx8ulp_msc[] = {
	{ 0, 0, {0, 0, 0, 0, 0, 0, 7, 7} }, /* MSC0 GPIOE */
	{ 0, 1, {0, 0, 0, 0, 0, 0, 7, 7} }, /* MSC0 GPIOF */
	{ 1, MSC_SLOT_ALL, {0, 0, 0, 0, 0, 0, 7, 7} }, /* MSC1 GPIOD */
	{ 2, MSC_SLOT_ALL, {0, 0, 0, 0, 0, 0, 7, 7} }, /* MSC2 GPU3D/2D/DCNANO/DDR registers */
};

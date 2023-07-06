/*
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef NCORE_CCU_H
#define NCORE_CCU_H

#include <stdbool.h>
#include <stdint.h>

#ifndef CCU_ACTIVATE_COH_FPGA
#define CCU_ACTIVATE_COH_FPGA 0
#endif
// Address map for ccu init
#define addr_CAIUIDR1				(0x1C000000)
#define addr_GRBUNRRUCR				(0x1c0ffff8)
#define base_addr_NRS_CAIU0			(0x1c000000)
#define base_addr_NRS_NCAIU0			(0x1c001000)
#define base_addr_NRS_NCAIU1			(0x1c002000)
#define base_addr_NRS_NCAIU2			(0x1c003000)
#define base_addr_NRS_NCAIU3			(0x1c004000)
#define base_addr_NRS_DCE0			(0x1c005000)
#define base_addr_NRS_DCE1			(0x1c006000)
//#define base_addr_NRS_DMI0			(0x1c007000)
//#define base_addr_NRS_DMI1			(0x1c008000)
//DMI
#define ALT_CCU_CCU_DMI0_DMIUSMCTCR_ADDR	0x1C007300
#define ALT_CCU_CCU_DMI1_DMIUSMCTCR_ADDR	0x1C008300
//DSU
#define ALT_CCU_DSU_CAIUAMIGR_ADDR		0x1C0003C0
#define ALT_CCU_DSU_CAIUMIFSR_ADDR		0x1C0003C4
#define ALT_CCU_DSU_CAIUGPRBLR1_ADDR		0x1C000414
#define ALT_CCU_DSU_CAIUGPRBHR1_ADDR		0x1C000418
#define ALT_CCU_DSU_CAIUGPRAR1_ADDR		0x1C000410
#define ALT_CCU_DSU_CAIUGPRBLR2_ADDR		0x1C000424
#define ALT_CCU_DSU_CAIUGPRBHR2_ADDR		0x1C000428
#define ALT_CCU_DSU_CAIUGPRAR2_ADDR		0x1C000420
#define ALT_CCU_DSU_CAIUGPRBLR4_ADDR		0x1C000444
#define ALT_CCU_DSU_CAIUGPRBHR4_ADDR		0x1C000448
#define ALT_CCU_DSU_CAIUGPRAR4_ADDR		0x1C000440
#define ALT_CCU_DSU_CAIUGPRBLR5_ADDR		0x1C000454
#define ALT_CCU_DSU_CAIUGPRBHR5_ADDR		0x1C000458
#define ALT_CCU_DSU_CAIUGPRAR5_ADDR		0x1C000450
#define ALT_CCU_DSU_CAIUGPRBLR6_ADDR		0x1C000464
#define ALT_CCU_DSU_CAIUGPRBHR6_ADDR		0x1C000468
#define ALT_CCU_DSU_CAIUGPRAR6_ADDR		0x1C000460
#define ALT_CCU_DSU_CAIUGPRBLR7_ADDR		0x1C000474
#define ALT_CCU_DSU_CAIUGPRBHR7_ADDR		0x1C000478
#define ALT_CCU_DSU_CAIUGPRAR7_ADDR		0x1C000470
#define ALT_CCU_DSU_CAIUGPRBLR8_ADDR		0x1C000484
#define ALT_CCU_DSU_CAIUGPRBHR8_ADDR		0x1C000488
#define ALT_CCU_DSU_CAIUGPRAR8_ADDR		0x1C000480
#define ALT_CCU_DSU_CAIUGPRBLR9_ADDR		0x1C000494
#define ALT_CCU_DSU_CAIUGPRBHR9_ADDR		0x1C000498
#define ALT_CCU_DSU_CAIUGPRAR9_ADDR		0x1C000490
#define ALT_CCU_DSU_CAIUGPRBLR10_ADDR		0x1C0004A4
#define ALT_CCU_DSU_CAIUGPRBHR10_ADDR		0x1C0004A8
#define ALT_CCU_DSU_CAIUGPRAR10_ADDR		0x1C0004A0
//GIC
#define ALT_CCU_GIC_M_XAIUAMIGR_ADDR		0x1C0023C0
#define ALT_CCU_GIC_M_XAIUMIFSR_ADDR		0x1C0023C4
#define ALT_CCU_GIC_M_XAIUGPRBLR1_ADDR		0x1C002414
#define ALT_CCU_GIC_M_XAIUGPRBHR1_ADDR		0x1C002418
#define ALT_CCU_GIC_M_XAIUGPRAR1_ADDR		0x1C002410
#define ALT_CCU_GIC_M_XAIUGPRBLR6_ADDR		0x1C002464
#define ALT_CCU_GIC_M_XAIUGPRBHR6_ADDR		0x1C002468
#define ALT_CCU_GIC_M_XAIUGPRAR6_ADDR		0x1C002460
#define ALT_CCU_GIC_M_XAIUGPRBLR8_ADDR		0x1C002484
#define ALT_CCU_GIC_M_XAIUGPRBHR8_ADDR		0x1C002488
#define ALT_CCU_GIC_M_XAIUGPRAR8_ADDR		0x1C002480
#define ALT_CCU_GIC_M_XAIUGPRBLR10_ADDR		0x1C0024A4
#define ALT_CCU_GIC_M_XAIUGPRBHR10_ADDR		0x1C0024A8
#define ALT_CCU_GIC_M_XAIUGPRAR10_ADDR		0x1C0024A0
//FPGA2SOC
#define ALT_CCU_FPGA2SOC_XAIUAMIGR_ADDR		0x1C0013C0
#define ALT_CCU_FPGA2SOC_XAIUMIFSR_ADDR		0x1C0013C4
#define ALT_CCU_FPGA2SOC_XAIUGPRBLR1_ADDR	0x1C001414
#define ALT_CCU_FPGA2SOC_XAIUGPRBHR1_ADDR	0x1C001418
#define ALT_CCU_FPGA2SOC_XAIUGPRAR1_ADDR	0x1C001410
#define ALT_CCU_FPGA2SOC_XAIUGPRBLR6_ADDR	0x1C001464
#define ALT_CCU_FPGA2SOC_XAIUGPRBHR6_ADDR	0x1C001468
#define ALT_CCU_FPGA2SOC_XAIUGPRAR6_ADDR	0x1C001460
#define ALT_CCU_FPGA2SOC_XAIUGPRBLR8_ADDR	0x1C001484
#define ALT_CCU_FPGA2SOC_XAIUGPRBHR8_ADDR	0x1C001488
#define ALT_CCU_FPGA2SOC_XAIUGPRAR8_ADDR	0x1C001480
#define ALT_CCU_FPGA2SOC_XAIUGPRBLR10_ADDR	0x1C0014A4
#define ALT_CCU_FPGA2SOC_XAIUGPRBHR10_ADDR	0x1C0014A8
#define ALT_CCU_FPGA2SOC_XAIUGPRAR10_ADDR	0x1C0014A0
//TCU
#define ALT_CCU_TCU_BASE 0x1C003000
#define ALT_CCU_TCU_XAIUAMIGR_ADDR		ALT_CCU_TCU_BASE + 0x03C0
#define ALT_CCU_TCU_XAIUMIFSR_ADDR		ALT_CCU_TCU_BASE + 0x03C4
#define ALT_CCU_TCU_XAIUGPRBLR0_ADDR		ALT_CCU_TCU_BASE + 0x0404
#define ALT_CCU_TCU_XAIUGPRBHR0_ADDR		ALT_CCU_TCU_BASE + 0x0408
#define ALT_CCU_TCU_XAIUGPRAR0_ADDR		ALT_CCU_TCU_BASE + 0x0400
#define ALT_CCU_TCU_XAIUGPRBLR1_ADDR		ALT_CCU_TCU_BASE + 0x0414
#define ALT_CCU_TCU_XAIUGPRBHR1_ADDR		ALT_CCU_TCU_BASE + 0x0418
#define ALT_CCU_TCU_XAIUGPRAR1_ADDR		ALT_CCU_TCU_BASE + 0x0410
#define ALT_CCU_TCU_XAIUGPRBLR2_ADDR		ALT_CCU_TCU_BASE + 0x0424
#define ALT_CCU_TCU_XAIUGPRBHR2_ADDR		ALT_CCU_TCU_BASE + 0x0428
#define ALT_CCU_TCU_XAIUGPRAR2_ADDR		ALT_CCU_TCU_BASE + 0x0420
#define ALT_CCU_TCU_XAIUGPRBLR6_ADDR		0x1C003464
#define ALT_CCU_TCU_XAIUGPRBHR6_ADDR		0x1C003468
#define ALT_CCU_TCU_XAIUGPRAR6_ADDR		0x1C003460
#define ALT_CCU_TCU_XAIUGPRBLR8_ADDR		0x1C003484
#define ALT_CCU_TCU_XAIUGPRBHR8_ADDR		0x1C003488
#define ALT_CCU_TCU_XAIUGPRAR8_ADDR		0x1C003480
#define ALT_CCU_TCU_XAIUGPRBLR10_ADDR		0x1C0034A4
#define ALT_CCU_TCU_XAIUGPRBHR10_ADDR		0x1C0034A8
#define ALT_CCU_TCU_XAIUGPRAR10_ADDR		0x1C0034A0
//IOM
#define ALT_CCU_CCU_IOM_XAIUAMIGR_ADDR		0x1C0043C0
#define ALT_CCU_CCU_IOM_XAIUMIFSR_ADDR		0x1C0013C4
#define ALT_CCU_IOM_XAIUGPRBLR1_ADDR		0x1C001414
#define ALT_CCU_IOM_XAIUGPRBHR1_ADDR		0x1C001418
#define ALT_CCU_IOM_XAIUGPRAR1_ADDR		0x1C001410
#define ALT_CCU_CCU_IOM_XAIUGPRBLR6_ADDR	0x1C001464
#define ALT_CCU_CCU_IOM_XAIUGPRBHR6_ADDR	0x1C001468
#define ALT_CCU_CCU_IOM_XAIUGPRAR6_ADDR		0x1C001460
#define ALT_CCU_CCU_IOM_XAIUGPRBLR8_ADDR	0x1C001484
#define ALT_CCU_CCU_IOM_XAIUGPRBHR8_ADDR	0x1C001488
#define ALT_CCU_CCU_IOM_XAIUGPRAR8_ADDR		0x1C001480
#define ALT_CCU_CCU_IOM_XAIUGPRBLR10_ADDR	0x1C0014A4
#define ALT_CCU_CCU_IOM_XAIUGPRBHR10_ADDR	0x1C0014A8
#define ALT_CCU_CCU_IOM_XAIUGPRAR10_ADDR	0x1C0014A0
//DCE
#define ALT_CCU_DCE0_DCEUAMIGR_ADDR		0x1C0053C0
#define ALT_CCU_DCE0_DCEUMIFSR_ADDR		0x1C0053C4
#define ALT_CCU_DCE0_DCEUGPRBLR6_ADDR		0x1C005464
#define ALT_CCU_DCE0_DCEUGPRBHR6_ADDR		0x1C005468
#define ALT_CCU_DCE0_DCEUGPRAR6_ADDR		0x1C005460
#define ALT_CCU_DCE0_DCEUGPRBLR8_ADDR		0x1C005484
#define ALT_CCU_DCE0_DCEUGPRBHR8_ADDR		0x1C005488
#define ALT_CCU_DCE0_DCEUGPRAR8_ADDR		0x1C005480
#define ALT_CCU_DCE0_DCEUGPRBLR10_ADDR		0x1C0054A4
#define ALT_CCU_DCE0_DCEUGPRBHR10_ADDR		0x1C0054A8
#define ALT_CCU_DCE0_DCEUGPRAR10_ADDR		0x1C0054A0
#define ALT_CCU_DCE1_DCEUAMIGR_ADDR		0x1C0063C0
#define ALT_CCU_DCE1_DCEUMIFSR_ADDR		0x1C0063C4
#define ALT_CCU_DCE1_DCEUGPRBLR6_ADDR		0x1C006464
#define ALT_CCU_DCE1_DCEUGPRBHR6_ADDR		0x1C006468
#define ALT_CCU_DCE1_DCEUGPRAR6_ADDR		0x1C006460
#define ALT_CCU_DCE1_DCEUGPRBLR8_ADDR		0x1C006484
#define ALT_CCU_DCE1_DCEUGPRBHR8_ADDR		0x1C006488
#define ALT_CCU_DCE1_DCEUGPRAR8_ADDR		0x1C006480
#define ALT_CCU_DCE1_DCEUGPRBLR10_ADDR		0x1C0064A4
#define ALT_CCU_DCE1_DCEUGPRBHR10_ADDR		0x1C0064A8
#define ALT_CCU_DCE1_DCEUGPRAR10_ADDR		0x1C0064A0
#define offset_NRS_GPRAR0			(0x400)
#define offset_NRS_GPRBLR0			(0x404)
#define offset_NRS_GPRBHR0			(0x408)
#define offset_NRS_GPRAR1			(0x410)
#define offset_NRS_GPRBLR1			(0x414)
#define offset_NRS_GPRBHR1			(0x418)
#define offset_NRS_GPRAR2			(0x420)
#define offset_NRS_GPRBLR2			(0x424)
#define offset_NRS_GPRBHR2			(0x428)
#define offset_NRS_GPRAR3			(0x430)
#define offset_NRS_GPRBLR3			(0x434)
#define offset_NRS_GPRBHR3			(0x438)
#define offset_NRS_GPRAR4			(0x440)
#define offset_NRS_GPRBLR4			(0x444)
#define offset_NRS_GPRBHR4			(0x448)
#define offset_NRS_GPRAR5			(0x450)
#define offset_NRS_GPRBLR5			(0x454)
#define offset_NRS_GPRBHR5			(0x458)
#define offset_NRS_GPRAR6			(0x460)
#define offset_NRS_GPRBLR6			(0x464)
#define offset_NRS_GPRBHR6			(0x468)
#define offset_NRS_GPRAR7			(0x470)
#define offset_NRS_GPRBLR7			(0x474)
#define offset_NRS_GPRBHR7			(0x478)
#define offset_NRS_GPRAR8			(0x480)
#define offset_NRS_GPRBLR8			(0x484)
#define offset_NRS_GPRBHR8			(0x488)
#define offset_NRS_GPRAR9			(0x490)
#define offset_NRS_GPRBLR9			(0x494)
#define offset_NRS_GPRBHR9			(0x498)
#define offset_NRS_GPRAR10			(0x4a0)
#define offset_NRS_GPRBLR10			(0x4a4)
#define offset_NRS_GPRBHR10			(0x4a8)
#define offset_NRS_AMIGR			(0x3c0)
#define offset_NRS_MIFSR			(0x3c4)
#define offset_NRS_DMIUSMCTCR			(0x300)
#define base_addr_DII0_PSSPERIPHS		(0x10000)
#define base_addr_DII0_LWHPS2FPGA		(0x20000)
#define base_addr_DII0_HPS2FPGA_1G		(0x40000)
#define base_addr_DII0_HPS2FPGA_15G		(0x400000)
#define base_addr_DII0_HPS2FPGA_240G		(0x4000000)
#define base_addr_DII1_MPFEREGS			(0x18000)
#define base_addr_DII2_GICREGS			(0x1D000)
#define base_addr_DII3_OCRAM			(0x0)
#define base_addr_BHR				(0x0)
#define base_addr_DMI_SDRAM_2G			(0x80000)
#define base_addr_DMI_SDRAM_30G			(0x800000)
#define base_addr_DMI_SDRAM_480G		(0x8000000)
// ((0x0<<9) | (0xf<<20) | (0x1<<30) | (0x1<<31))
#define wr_DII0_PSSPERIPHS			0xC0F00000
// ((0x0<<9) | (0x11<<20) | (0x1<<30) | (0x1<<31))
#define wr_DII0_LWHPS2FPGA			0xC1100000
// ((0x0<<9) | (0x12<<20) | (0x1<<30) | (0x1<<31))
#define wr_DII0_HPS2FPGA_1G			0xC1200000
// ((0x0<<9) | (0x16<<20) | (0x1<<30) | (0x1<<31))
#define wr_DII0_HPS2FPGA_15G			0xC1600000
// ((0x0<<9) | (0x1a<<20) | (0x1<<30) | (0x1<<31))
#define wr_DII0_HPS2FPGA_240G			0xC1A00000
// ((0x1<<9) | (0xe<<20) | (0x1<<30) | (0x1<<31))
#define wr_DII1_MPFEREGS			0xC0E00200
// ((0x2<<9) | (0x8<<20) | (0x1<<30) | (0x1<<31))
#define wr_DII2_GICREGS				0xC0800400
// ((0x3<<9) | (0x9<<20) | (0x1<<30) | (0x1<<31))
#define wr_DII3_OCRAM				0xC0900600
// ((0x0<<9) | (0x12<<20) | (0x0<<30) | (0x1<<31))
#define wr_DMI_SDRAM_1G_ORDERED			0x81200000
// ((0x1<<1) | (0x1<<2) | (0x0<<9) | (0x12<<20) | (0x0<<30) | (0x1<<31))
#define wr_DMI_SDRAM_1G				0x81200006
// ((0x0<<9) | (0x13<<20) | (0x0<<30) | (0x1<<31))
#define wr_DMI_SDRAM_2G_ORDERED			0x81300000
// ((0x1<<1) | (0x1<<2) | (0x0<<9) | (0x13<<20) | (0x0<<30) | (0x1<<31))
#define wr_DMI_SDRAM_2G				0x81300006
// ((0x0<<9) | (0x16<<20) | (0x0<<30) | (0x1<<31))
#define wr_DMI_SDRAM_15G_ORDERED		0x81600000
// ((0x1<<1) | (0x1<<2) | (0x0<<9) | (0x16<<20) | (0x0<<30) | (0x1<<31))
#define wr_DMI_SDRAM_15G			0x81600006
// ((0x0<<9) | (0x17<<20) | (0x0<<30) | (0x1<<31))
#define wr_DMI_SDRAM_30G_ORDERED		0x81700000
// ((0x1<<1) | (0x1<<2) | (0x0<<9) | (0x17<<20) | (0x0<<30) | (0x1<<31))
#define wr_DMI_SDRAM_30G			0x81700006
// ((0x0<<9) | (0x1a<<20) | (0x0<<30) | (0x1<<31))
#define wr_DMI_SDRAM_240G_ORDERED		0x81a00000
// ((0x1<<1) | (0x1<<2) | (0x0<<9) | (0x1a<<20) | (0x0<<30) | (0x1<<31))
#define wr_DMI_SDRAM_240G			0x81a00006
// ((0x0<<9) | (0x1b<<20) | (0x0<<30) | (0x1<<31))
#define wr_DMI_SDRAM_480G_ORDERED		0x81b00000
// ((0x1<<1) | (0x1<<2) | (0x0<<9) | (0x1b<<20) | (0x0<<30) | (0x1<<31))
#define wr_DMI_SDRAM_480G			0x81b00006

typedef enum CCU_REGION_SECURITY_e {
    //
    // Allow secure accesses only.
    //
	CCU_REGION_SECURITY_SECURE_ONLY,
    //
    // Allow non-secure accesses only.
    //
	CCU_REGION_SECURITY_NON_SECURE_ONLY,
    //
    // Allow accesses of any security state.
    //
	CCU_REGION_SECURITY_DONT_CARE
} CCU_REGION_SECURITY_t;
typedef enum CCU_REGION_PRIVILEGE_e {
    //
    // Allow privileged accesses only.
    //
	CCU_REGION_PRIVILEGE_PRIVILEGED_ONLY,
    //
    // Allow unprivileged accesses only.
    //
	CCU_REGION_PRIVILEGE_NON_PRIVILEGED_ONLY,
    //
    // Allow accesses of any privilege.
    //
	CCU_REGION_PRIVILEGE_DONT_CARE
} CCU_REGION_PRIVILEGE_t;
//
// Initializes the CCU by enabling all regions except RAM 1 - 5.
// This is needed because of an RTL change around 2016.02.24.
//
// Runtime measurement:
//  - arm     : 14,830,000 ps (2016.05.31; sanity/printf_aarch32)
//  - aarch64 : 14,837,500 ps (2016.05.31; sanity/printf)
//
// Runtime history:
//  - arm     : 20,916,668 ps (2016.05.30; sanity/printf_aarch32)
//  - aarch64 : 20,924,168 ps (2016.05.30; sanity/printf)
//
int ccu_hps_init(void);

typedef enum ccu_hps_ram_region_e {
	ccu_hps_ram_region_ramspace0 = 0,
	ccu_hps_ram_region_ramspace1 = 1,
	ccu_hps_ram_region_ramspace2 = 2,
	ccu_hps_ram_region_ramspace3 = 3,
	ccu_hps_ram_region_ramspace4 = 4,
	ccu_hps_ram_region_ramspace5 = 5,
} ccu_hps_ram_region_t;

// Disables a RAM (OCRAM) region with the given ID.
int ccu_hps_ram_region_disable(int id);

// Enables a RAM (OCRAM) region with the given ID.
int ccu_hps_ram_region_enable(int id);

// Attempts to remap a RAM (OCRAM) region with the given ID to span the given
// start and end address. It also assigns the security and privilege policy.
// Regions must be a power-of-two size with a minimum size of 64B.
int ccu_hps_ram_region_remap(int id, uintptr_t start, uintptr_t end,
CCU_REGION_SECURITY_t security, CCU_REGION_PRIVILEGE_t privilege);

// Verifies that all enabled RAM (OCRAM) regions does not overlap.
int ccu_hps_ram_validate(void);

typedef enum ccu_hps_mem_region_e {
	ccu_hps_mem_region_ddrspace0  = 0,
	ccu_hps_mem_region_memspace0  = 1,
	ccu_hps_mem_region_memspace1a = 2,
	ccu_hps_mem_region_memspace1b = 3,
	ccu_hps_mem_region_memspace1c = 4,
	ccu_hps_mem_region_memspace1d = 5,
	ccu_hps_mem_region_memspace1e = 6,
} ccu_hps_mem_region_t;

// Disables mem0 (DDR) region with the given ID.
int ccu_hps_mem0_region_disable(int id);

// Enables mem0 (DDR) region with the given ID.
int ccu_hps_mem0_region_enable(int id);

// Attempts to remap mem0 (DDR) region with the given ID to span the given
// start and end address. It also assigns the security nad privlege policy.
// Regions must be a power-of-two in size with a minimum size of 64B.
int ccu_hps_mem0_region_remap(int id, uintptr_t start, uintptr_t end,
CCU_REGION_SECURITY_t security, CCU_REGION_PRIVILEGE_t privilege);

// Verifies that all enabled mem0 (DDR) regions does not overlap.
int ccu_hps_mem0_validate(void);

typedef enum ccu_hps_ios_region_e {
	ccu_hps_ios_region_iospace0a = 0,
	ccu_hps_ios_region_iospace0b = 1,
	ccu_hps_ios_region_iospace1a = 2,
	ccu_hps_ios_region_iospace1b = 3,
	ccu_hps_ios_region_iospace1c = 4,
	ccu_hps_ios_region_iospace1d = 5,
	ccu_hps_ios_region_iospace1e = 6,
	ccu_hps_ios_region_iospace1f = 7,
	ccu_hps_ios_region_iospace1g = 8,
	ccu_hps_ios_region_iospace2a = 9,
	ccu_hps_ios_region_iospace2b = 10,
	ccu_hps_ios_region_iospace2c = 11,
} ccu_hps_ios_region_t;

// Disables the IOS (IO Slave) region with the given ID.
int ccu_hps_ios_region_disable(int id);

// Enables the IOS (IO Slave) region with the given ID.
int ccu_hps_ios_region_enable(int id);


#define NCORE_CCU_OFFSET			0xf7000000

/* Coherent Sub-System Address Map */
#define NCORE_CAIU_OFFSET			0x00000
#define NCORE_CAIU_SIZE				0x01000
#define NCORE_NCBU_OFFSET			0x60000
#define NCORE_NCBU_SIZE				0x01000
#define NCORE_DIRU_OFFSET			0x80000
#define NCORE_DIRU_SIZE				0x01000
#define NCORE_CMIU_OFFSET			0xc0000
#define NCORE_CMIU_SIZE				0x01000
#define NCORE_CSR_OFFSET			0xff000
#define NCORE_CSADSERO				0x00040
#define NCORE_CSUIDR				0x00ff8
#define NCORE_CSIDR				0x00ffc
/* Directory Unit Register Map */
#define NCORE_DIRUSFER				0x00010
#define NCORE_DIRUMRHER				0x00070
#define NCORE_DIRUSFMCR				0x00080
#define NCORE_DIRUSFMAR				0x00084
/* Coherent Agent Interface Unit Register Map */
#define NCORE_CAIUIDR				0x00ffc
/* Snoop Enable Register */
#define NCORE_DIRUCASER0			0x00040
#define NCORE_DIRUCASER1			0x00044
#define NCORE_DIRUCASER2			0x00048
#define NCORE_DIRUCASER3			0x0004c
#define NCORE_CSADSER0				0x00040
#define NCORE_CSADSER1				0x00044
#define NCORE_CSADSER2				0x00048
#define NCORE_CSADSER3				0x0004c
/* Protocols Definition */
#define ACE_W_DVM				0
#define ACE_L_W_DVM				1
#define ACE_WO_DVM				2
#define ACE_L_WO_DVM				3
/* Bypass OC Ram Firewall */
#define NCORE_FW_OCRAM_BLK_BASE			0x100200
#define NCORE_FW_OCRAM_BLK_CGF1			0x04
#define NCORE_FW_OCRAM_BLK_CGF2			0x08
#define NCORE_FW_OCRAM_BLK_CGF3			0x0c
#define NCORE_FW_OCRAM_BLK_CGF4			0x10
#define OCRAM_PRIVILEGED_MASK			BIT(29)
#define OCRAM_SECURE_MASK			BIT(30)
/* Macros */
#define NCORE_CCU_REG(base)			(NCORE_CCU_OFFSET + (base))
#define NCORE_CCU_CSR(reg)			(NCORE_CCU_REG(NCORE_CSR_OFFSET)\
						+ (reg))
#define NCORE_CCU_DIR(reg)			(NCORE_CCU_REG(NCORE_DIRU_OFFSET)\
						+ (reg))
#define NCORE_CCU_CAI(reg)			(NCORE_CCU_REG(NCORE_CAIU_OFFSET)\
						+ (reg))
#define DIRECTORY_UNIT(x, reg)			(NCORE_CCU_DIR(reg)\
						+ NCORE_DIRU_SIZE * (x))
#define COH_AGENT_UNIT(x, reg)			(NCORE_CCU_CAI(reg)\
						+ NCORE_CAIU_SIZE * (x))
#define COH_CPU0_BYPASS_REG(reg)		(NCORE_CCU_REG(NCORE_FW_OCRAM_BLK_BASE)\
						+ (reg))
#define CSUIDR_NUM_CMI(x)			(((x) & 0x3f000000) >> 24)
#define CSUIDR_NUM_DIR(x)			(((x) & 0x003f0000) >> 16)
#define CSUIDR_NUM_NCB(x)			(((x) & 0x00003f00) >> 8)
#define CSUIDR_NUM_CAI(x)			(((x) & 0x0000007f) >> 0)
#define CSIDR_NUM_SF(x)				(((x) & 0x007c0000) >> 18)
#define SNOOP_FILTER_ID(x)			(((x) << 16))
#define CACHING_AGENT_BIT(x)			(((x) & 0x08000) >> 15)
#define CACHING_AGENT_TYPE(x)			(((x) & 0xf0000) >> 16)

typedef struct coh_ss_id {
	uint8_t num_coh_mem;
	uint8_t num_directory;
	uint8_t num_non_coh_bridge;
	uint8_t num_coh_agent;
	uint8_t num_snoop_filter;
} coh_ss_id_t;

uint32_t init_ncore_ccu(void);
void ncore_enable_ocram_firewall(void);
void setup_smmu_stream_id(void);

#endif

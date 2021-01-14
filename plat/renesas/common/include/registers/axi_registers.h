/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AXI_REGISTERS_H
#define AXI_REGISTERS_H

/* AXI registers */

/* AXI base address */
#define	AXI_BASE	(0xE6780000U)

/* address split */

/* AXI address split control 0 */
#define	AXI_ADSPLCR0	(AXI_BASE + 0x4008U)
/* AXI address split control 1 */
#define	AXI_ADSPLCR1	(AXI_BASE + 0x400CU)
/* AXI address split control 2 */
#define	AXI_ADSPLCR2	(AXI_BASE + 0x4010U)
/* AXI address split control 3 */
#define	AXI_ADSPLCR3	(AXI_BASE + 0x4014U)

/* functional safety */

/* AXI functional safety control */
#define	AXI_FUSACR	(AXI_BASE + 0x4020U)

/* decompression */

/* AXI decompression area configuration A0 */
#define	AXI_DCMPAREACRA0	(AXI_BASE + 0x4100U)
/* AXI decompression area configuration B0 */
#define	AXI_DCMPAREACRB0	(AXI_BASE + 0x4104U)
/* AXI decompression area configuration A1 */
#define	AXI_DCMPAREACRA1	(AXI_BASE + 0x4108U)
/* AXI decompression area configuration B1 */
#define	AXI_DCMPAREACRB1	(AXI_BASE + 0x410CU)
/* AXI decompression area configuration A2 */
#define	AXI_DCMPAREACRA2	(AXI_BASE + 0x4110U)
/* AXI decompression area configuration B2 */
#define	AXI_DCMPAREACRB2	(AXI_BASE + 0x4114U)
/* AXI decompression area configuration A3 */
#define	AXI_DCMPAREACRA3	(AXI_BASE + 0x4118U)
/* AXI decompression area configuration B3 */
#define	AXI_DCMPAREACRB3	(AXI_BASE + 0x411CU)
/* AXI decompression area configuration A4 */
#define	AXI_DCMPAREACRA4	(AXI_BASE + 0x4120U)
/* AXI decompression area configuration B4 */
#define	AXI_DCMPAREACRB4	(AXI_BASE + 0x4124U)
/* AXI decompression area configuration A5 */
#define	AXI_DCMPAREACRA5	(AXI_BASE + 0x4128U)
/* AXI decompression area configuration B5 */
#define	AXI_DCMPAREACRB5	(AXI_BASE + 0x412CU)
/* AXI decompression area configuration A6 */
#define	AXI_DCMPAREACRA6	(AXI_BASE + 0x4130U)
/* AXI decompression area configuration B6 */
#define	AXI_DCMPAREACRB6	(AXI_BASE + 0x4134U)
/* AXI decompression area configuration A7 */
#define	AXI_DCMPAREACRA7	(AXI_BASE + 0x4138U)
/* AXI decompression area configuration B7 */
#define	AXI_DCMPAREACRB7	(AXI_BASE + 0x413CU)
/* AXI decompression area configuration A8 */
#define	AXI_DCMPAREACRA8	(AXI_BASE + 0x4140U)
/* AXI decompression area configuration B8 */
#define	AXI_DCMPAREACRB8	(AXI_BASE + 0x4144U)
/* AXI decompression area configuration A9 */
#define	AXI_DCMPAREACRA9	(AXI_BASE + 0x4148U)
/* AXI decompression area configuration B9 */
#define	AXI_DCMPAREACRB9	(AXI_BASE + 0x414CU)
/* AXI decompression area configuration A10 */
#define	AXI_DCMPAREACRA10	(AXI_BASE + 0x4150U)
/* AXI decompression area configuration B10 */
#define	AXI_DCMPAREACRB10	(AXI_BASE + 0x4154U)
/* AXI decompression area configuration A11 */
#define	AXI_DCMPAREACRA11	(AXI_BASE + 0x4158U)
/* AXI decompression area configuration B11 */
#define	AXI_DCMPAREACRB11	(AXI_BASE + 0x415CU)
/* AXI decompression area configuration A12 */
#define	AXI_DCMPAREACRA12	(AXI_BASE + 0x4160U)
/* AXI decompression area configuration B12 */
#define	AXI_DCMPAREACRB12	(AXI_BASE + 0x4164U)
/* AXI decompression area configuration A13 */
#define	AXI_DCMPAREACRA13	(AXI_BASE + 0x4168U)
/* AXI decompression area configuration B13 */
#define	AXI_DCMPAREACRB13	(AXI_BASE + 0x416CU)
/* AXI decompression area configuration A14 */
#define	AXI_DCMPAREACRA14	(AXI_BASE + 0x4170U)
/* AXI decompression area configuration B14 */
#define	AXI_DCMPAREACRB14	(AXI_BASE + 0x4174U)
/* AXI decompression area configuration A15 */
#define	AXI_DCMPAREACRA15	(AXI_BASE + 0x4178U)
/* AXI decompression area configuration B15 */
#define	AXI_DCMPAREACRB15	(AXI_BASE + 0x417CU)
/* AXI decompression shadow area configuration */
#define	AXI_DCMPSHDWCR	(AXI_BASE + 0x4280U)

/* SDRAM protection */

/* AXI dram protected area division 0 */
#define	AXI_DPTDIVCR0	(AXI_BASE + 0x4400U)
/* AXI dram protected area division 1 */
#define	AXI_DPTDIVCR1	(AXI_BASE + 0x4404U)
/* AXI dram protected area division 2 */
#define	AXI_DPTDIVCR2	(AXI_BASE + 0x4408U)
/* AXI dram protected area division 3 */
#define	AXI_DPTDIVCR3	(AXI_BASE + 0x440CU)
/* AXI dram protected area division 4 */
#define	AXI_DPTDIVCR4	(AXI_BASE + 0x4410U)
/* AXI dram protected area division 5 */
#define	AXI_DPTDIVCR5	(AXI_BASE + 0x4414U)
/* AXI dram protected area division 6 */
#define	AXI_DPTDIVCR6	(AXI_BASE + 0x4418U)
/* AXI dram protected area division 7 */
#define	AXI_DPTDIVCR7	(AXI_BASE + 0x441CU)
/* AXI dram protected area division 8 */
#define	AXI_DPTDIVCR8	(AXI_BASE + 0x4420U)
/* AXI dram protected area division 9 */
#define	AXI_DPTDIVCR9	(AXI_BASE + 0x4424U)
/* AXI dram protected area division 10 */
#define	AXI_DPTDIVCR10	(AXI_BASE + 0x4428U)
/* AXI dram protected area division 11 */
#define	AXI_DPTDIVCR11	(AXI_BASE + 0x442CU)
/* AXI dram protected area division 12 */
#define	AXI_DPTDIVCR12	(AXI_BASE + 0x4430U)
/* AXI dram protected area division 13 */
#define	AXI_DPTDIVCR13	(AXI_BASE + 0x4434U)
/* AXI dram protected area division 14 */
#define	AXI_DPTDIVCR14	(AXI_BASE + 0x4438U)

/* AXI dram protected area setting 0 */
#define	AXI_DPTCR0	(AXI_BASE + 0x4440U)
/* AXI dram protected area setting 1 */
#define	AXI_DPTCR1	(AXI_BASE + 0x4444U)
/* AXI dram protected area setting 2 */
#define	AXI_DPTCR2	(AXI_BASE + 0x4448U)
/* AXI dram protected area setting 3 */
#define	AXI_DPTCR3	(AXI_BASE + 0x444CU)
/* AXI dram protected area setting 4 */
#define	AXI_DPTCR4	(AXI_BASE + 0x4450U)
/* AXI dram protected area setting 5 */
#define	AXI_DPTCR5	(AXI_BASE + 0x4454U)
/* AXI dram protected area setting 6 */
#define	AXI_DPTCR6	(AXI_BASE + 0x4458U)
/* AXI dram protected area setting 7 */
#define	AXI_DPTCR7	(AXI_BASE + 0x445CU)
/* AXI dram protected area setting 8 */
#define	AXI_DPTCR8	(AXI_BASE + 0x4460U)
/* AXI dram protected area setting 9 */
#define	AXI_DPTCR9	(AXI_BASE + 0x4464U)
/* AXI dram protected area setting 10 */
#define	AXI_DPTCR10	(AXI_BASE + 0x4468U)
/* AXI dram protected area setting 11 */
#define	AXI_DPTCR11	(AXI_BASE + 0x446CU)
/* AXI dram protected area setting 12 */
#define	AXI_DPTCR12	(AXI_BASE + 0x4470U)
/* AXI dram protected area setting 13 */
#define	AXI_DPTCR13	(AXI_BASE + 0x4474U)
/* AXI dram protected area setting 14 */
#define	AXI_DPTCR14	(AXI_BASE + 0x4478U)
/* AXI dram protected area setting 15 */
#define	AXI_DPTCR15	(AXI_BASE + 0x447CU)

/* SRAM protection */

/* AXI sram protected area division 0 */
#define	AXI_SPTDIVCR0	(AXI_BASE + 0x4500U)
/* AXI sram protected area division 1 */
#define	AXI_SPTDIVCR1	(AXI_BASE + 0x4504U)
/* AXI sram protected area division 2 */
#define	AXI_SPTDIVCR2	(AXI_BASE + 0x4508U)
/* AXI sram protected area division 3 */
#define	AXI_SPTDIVCR3	(AXI_BASE + 0x450CU)
/* AXI sram protected area division 4 */
#define	AXI_SPTDIVCR4	(AXI_BASE + 0x4510U)
/* AXI sram protected area division 5 */
#define	AXI_SPTDIVCR5	(AXI_BASE + 0x4514U)
/* AXI sram protected area division 6 */
#define	AXI_SPTDIVCR6	(AXI_BASE + 0x4518U)
/* AXI sram protected area division 7 */
#define	AXI_SPTDIVCR7	(AXI_BASE + 0x451CU)
/* AXI sram protected area division 8 */
#define	AXI_SPTDIVCR8	(AXI_BASE + 0x4520U)
/* AXI sram protected area division 9 */
#define	AXI_SPTDIVCR9	(AXI_BASE + 0x4524U)
/* AXI sram protected area division 10 */
#define	AXI_SPTDIVCR10	(AXI_BASE + 0x4528U)
/* AXI sram protected area division 11 */
#define	AXI_SPTDIVCR11	(AXI_BASE + 0x452CU)
/* AXI sram protected area division 12 */
#define	AXI_SPTDIVCR12	(AXI_BASE + 0x4530U)
/* AXI sram protected area division 13 */
#define	AXI_SPTDIVCR13	(AXI_BASE + 0x4534U)
/* AXI sram protected area division 14 */
#define	AXI_SPTDIVCR14	(AXI_BASE + 0x4538U)

/* AXI sram protected area setting 0 */
#define	AXI_SPTCR0	(AXI_BASE + 0x4540U)
/* AXI sram protected area setting 1 */
#define	AXI_SPTCR1	(AXI_BASE + 0x4544U)
/* AXI sram protected area setting 2 */
#define	AXI_SPTCR2	(AXI_BASE + 0x4548U)
/* AXI sram protected area setting 3 */
#define	AXI_SPTCR3	(AXI_BASE + 0x454CU)
/* AXI sram protected area setting 4 */
#define	AXI_SPTCR4	(AXI_BASE + 0x4550U)
/* AXI sram protected area setting 5 */
#define	AXI_SPTCR5	(AXI_BASE + 0x4554U)
/* AXI sram protected area setting 6 */
#define	AXI_SPTCR6	(AXI_BASE + 0x4558U)
/* AXI sram protected area setting 7 */
#define	AXI_SPTCR7	(AXI_BASE + 0x455CU)
/* AXI sram protected area setting 8 */
#define	AXI_SPTCR8	(AXI_BASE + 0x4560U)
/* AXI sram protected area setting 9 */
#define	AXI_SPTCR9	(AXI_BASE + 0x4564U)
/* AXI sram protected area setting 10 */
#define	AXI_SPTCR10	(AXI_BASE + 0x4568U)
/* AXI sram protected area setting 11 */
#define	AXI_SPTCR11	(AXI_BASE + 0x456CU)
/* AXI sram protected area setting 12 */
#define	AXI_SPTCR12	(AXI_BASE + 0x4570U)
/* AXI sram protected area setting 13 */
#define	AXI_SPTCR13	(AXI_BASE + 0x4574U)
/* AXI sram protected area setting 14 */
#define	AXI_SPTCR14	(AXI_BASE + 0x4578U)
/* AXI sram protected area setting 15 */
#define	AXI_SPTCR15	(AXI_BASE + 0x457CU)

/* EDC base address */
#define	EDC_BASE	(0xFF840000U)

/* EDC edc enable */
#define	EDC_EDCEN	(EDC_BASE + 0x0010U)
/* EDC edc status 0 */
#define	EDC_EDCST0	(EDC_BASE + 0x0020U)
/* EDC edc status 1 */
#define	EDC_EDCST1	(EDC_BASE + 0x0024U)
/* EDC edc interrupt enable 0 */
#define	EDC_EDCINTEN0	(EDC_BASE + 0x0040U)
/* EDC edc interrupt enable 1 */
#define	EDC_EDCINTEN1	(EDC_BASE + 0x0044U)

#endif /* AXI_REGISTERS_H */

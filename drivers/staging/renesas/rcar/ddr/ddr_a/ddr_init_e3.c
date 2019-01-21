/*
 * Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>

#include "boot_init_dram_regdef_e3.h"
#include "ddr_init_e3.h"

#include "../dram_sub_func.h"

/*  rev.0.04 add variables */
/*******************************************************************************
 *  variables
 ******************************************************************************/
uint32_t ddrBackup;

/*  rev.0.03 add Prototypes */
/*******************************************************************************
 *  Prototypes
 ******************************************************************************/
/* static uint32_t init_ddr(void); rev.0.04 */
/* static uint32_t recovery_from_backup_mode(void);  rev.0.04 */
/* int32_t dram_update_boot_status(uint32_t status); rev.0.04 */

/*  rev.0.03 add Comment */
/*******************************************************************************
 *  register write/read function
 ******************************************************************************/
static void    WriteReg_32(uint32_t a, uint32_t v)
{
    (*(volatile uint32_t*)(uintptr_t)a) = v;
} /*  WriteReg_32 */

static uint32_t ReadReg_32(uint32_t a)
{
    uint32_t w = (*(volatile uint32_t*)(uintptr_t)a);
    return w;
} /*  ReadReg_32 */

/*  rev.0.04 add Comment */
/*******************************************************************************
 *  Initialize ddr
 ******************************************************************************/
uint32_t init_ddr(void)
{
   uint32_t RegVal_R2, RegVal_R5, RegVal_R6, RegVal_R7, RegVal_R12, i;
   uint32_t ddr_md;

/* rev.0.08 */
   uint32_t RegVal, j;
   uint32_t dqsgd_0c, bdlcount_0c, bdlcount_0c_div2, bdlcount_0c_div4, bdlcount_0c_div8, bdlcount_0c_div16;
   uint32_t gatesl_0c, rdqsd_0c, rdqsnd_0c, rbd_0c[4];
   uint32_t pdqsr_ctl, lcdl_ctl, lcdl_judge1, lcdl_judge2;
/* rev.0.10 */
   uint32_t pdr_ctl;
/* rev.0.11 */
   uint32_t byp_ctl;

/* rev.0.08 */
   if ((ReadReg_32(0xFFF00044) & 0x000000FF) == 0x00000000) {
     pdqsr_ctl  = 1;
     lcdl_ctl   = 1;
     pdr_ctl    = 1;  /* rev.0.10 */
     byp_ctl    = 1;  /* rev.0.11 */
    } else {
     pdqsr_ctl  = 0;
     lcdl_ctl   = 0;
     pdr_ctl    = 0;  /* rev.0.10 */
     byp_ctl    = 0;  /* rev.0.11 */
   }

   /*  Judge the DDR bit rate (ddr_md : 0 = 1584Mbps, 1 = 1856Mbps) */
   ddr_md = (ReadReg_32(RST_MODEMR) >> 19) & BIT0;

   /*  1584Mbps setting */
   if (ddr_md == 0) {
      /* CPG setting ===============================================*/
      WriteReg_32(CPG_CPGWPR, 0x5A5AFFFF);
      WriteReg_32(CPG_CPGWPCR, 0xA5A50000);

      WriteReg_32(CPG_SRCR4, 0x20000000);

      WriteReg_32(0xE61500DC, 0xe2200000);  /*  Change to 1584Mbps */
      while ((BIT11 & ReadReg_32(CPG_PLLECR)) == 0);

      WriteReg_32(CPG_SRSTCLR4, 0x20000000);

      WriteReg_32(CPG_CPGWPCR, 0xA5A50001);

      /* CPG setting ===============================================*/
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBSYSCNT0, 0x00001234);
   WriteReg_32(DBSC_E3_DBKIND, 0x00000007);

#if RCAR_DRAM_DDR3L_MEMCONF == 0
   WriteReg_32(DBSC_E3_DBMEMCONF00, 0x0f030a02); /*  1GB */
#elif RCAR_DRAM_DDR3L_MEMCONF == 1
   WriteReg_32(DBSC_E3_DBMEMCONF00, 0x10030a02); /*  2GB(default) */
#elif RCAR_DRAM_DDR3L_MEMCONF == 2
   WriteReg_32(DBSC_E3_DBMEMCONF00, 0x10030b02); /*  4GB */
#else
   WriteReg_32(DBSC_E3_DBMEMCONF00, 0x10030a02); /*  2GB */
#endif

#if RCAR_DRAM_DDR3L_MEMDUAL == 1
	 RegVal_R2 = (ReadReg_32(0xE6790614));
         WriteReg_32(0xE6790614, RegVal_R2 | 0x00000003); /*  MCS1_N/MODT1 are activated. */
#endif


   WriteReg_32(DBSC_E3_DBPHYCONF0, 0x00000001);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBTR0, 0x0000000B);
      WriteReg_32(DBSC_E3_DBTR1, 0x00000008);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBTR0, 0x0000000D);
      WriteReg_32(DBSC_E3_DBTR1, 0x00000009);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBTR2, 0x00000000);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBTR3, 0x0000000B);
      WriteReg_32(DBSC_E3_DBTR4, 0x000B000B);
      WriteReg_32(DBSC_E3_DBTR5, 0x00000027);
      WriteReg_32(DBSC_E3_DBTR6, 0x0000001C);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBTR3, 0x0000000D);
      WriteReg_32(DBSC_E3_DBTR4, 0x000D000D);
      WriteReg_32(DBSC_E3_DBTR5, 0x0000002D);
      WriteReg_32(DBSC_E3_DBTR6, 0x00000020);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBTR7, 0x00060006);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBTR8, 0x00000020);
      WriteReg_32(DBSC_E3_DBTR9, 0x00000006);
      WriteReg_32(DBSC_E3_DBTR10, 0x0000000C);
      WriteReg_32(DBSC_E3_DBTR11, 0x0000000A);
      WriteReg_32(DBSC_E3_DBTR12, 0x00120012);
      WriteReg_32(DBSC_E3_DBTR13, 0x000000CE);
      WriteReg_32(DBSC_E3_DBTR14, 0x00140005);
      WriteReg_32(DBSC_E3_DBTR15, 0x00050004);
      WriteReg_32(DBSC_E3_DBTR16, 0x071F0305);
      WriteReg_32(DBSC_E3_DBTR17, 0x040C0000);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBTR8, 0x00000021);
      WriteReg_32(DBSC_E3_DBTR9, 0x00000007);
      WriteReg_32(DBSC_E3_DBTR10, 0x0000000E);
      WriteReg_32(DBSC_E3_DBTR11, 0x0000000C);
      WriteReg_32(DBSC_E3_DBTR12, 0x00140014);
      WriteReg_32(DBSC_E3_DBTR13, 0x000000F2);
      WriteReg_32(DBSC_E3_DBTR14, 0x00170006);
      WriteReg_32(DBSC_E3_DBTR15, 0x00060005);
      WriteReg_32(DBSC_E3_DBTR16, 0x09210507);
      WriteReg_32(DBSC_E3_DBTR17, 0x040E0000);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBTR18, 0x00000200);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBTR19, 0x01000040);
      WriteReg_32(DBSC_E3_DBTR20, 0x020000D6);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBTR19, 0x0129004B);
      WriteReg_32(DBSC_E3_DBTR20, 0x020000FB);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBTR21, 0x00040004);
   WriteReg_32(DBSC_E3_DBBL, 0x00000000);
   WriteReg_32(DBSC_E3_DBODT0, 0x00000001);
   WriteReg_32(DBSC_E3_DBADJ0, 0x00000001);
   WriteReg_32(DBSC_E3_DBSYSCONF1, 0x00000002);
   WriteReg_32(DBSC_E3_DBDFICNT0, 0x00000010);
   WriteReg_32(DBSC_E3_DBBCAMDIS, 0x00000001);
   WriteReg_32(DBSC_E3_DBSCHRW1, 0x00000046);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_SCFCTST0, 0x0D050B03);
      WriteReg_32(DBSC_E3_SCFCTST1, 0x0306030C);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_SCFCTST0, 0x0C050B03);
      WriteReg_32(DBSC_E3_SCFCTST1, 0x0305030C);
   } /*  ddr_md */

   /*  rev.0.03 add Comment */
   /****************************************************************************
    *  Initial_Step0( INITBYP )
    ***************************************************************************/
   WriteReg_32(DBSC_E3_DBPDLK0, 0x0000A55A);
   WriteReg_32(DBSC_E3_DBCMD, 0x01840001);
   WriteReg_32(DBSC_E3_DBCMD, 0x08840000);
   NOTICE("BL2: [COLD_BOOT]\n");	/* rev.0.11 */
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x80010000);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   /*  rev.0.03 add Comment */
   /****************************************************************************
    *  Initial_Step1( ZCAL,PLLINIT,DCAL,PHYRST training )
    ***************************************************************************/
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000008);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x000B8000);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000090);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x04058904);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x04058A04);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000091);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0007BB6B);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000095);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0007BBAD);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000099);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0007BB6B);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000090);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x04058900);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x04058A00);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000021);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0024641E);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00010073);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   /*  rev.0.03 add Comment */
   /****************************************************************************
    *  Initial_Step2( DRAMRST/DRAMINT training )
    ***************************************************************************/
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000090);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0C058900);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0C058A00);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000090);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x04058900);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x04058A00);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000003);
   if (byp_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0780C720);
   } else {
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0780C700);
   }
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000007);
   while ((BIT30 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000004);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, (uint32_t)(REFRESH_RATE * 792 / 125) - 400 + 0x08B00000);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, (uint32_t)(REFRESH_RATE * 928 / 125) - 400 + 0x0A300000);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000022);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x1000040B);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000023);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x2D9C0B66);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x35A00D77);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000024);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x2A88B400);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x2A8A2C28);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000025);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x30005200);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x30005E00);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000026);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0014A9C9);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0014CB49);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000027);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000D70);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000F14);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000028);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000046);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000029);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      if (REFRESH_RATE > 3900) {
          WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000018);  /*            [7]SRT=0 */
      } else {
          WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000098);  /*            [7]SRT=1 */
      }
   } else {                                        /*  1856Mbps */
      if (REFRESH_RATE > 3900) {
          WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000020);  /*            [7]SRT=0 */
      } else {
          WriteReg_32(DBSC_E3_DBPDRGD0, 0x000000A0);  /*            [7]SRT=1 */
      } /*  REFRESH_RATE */
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x0000002C);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x81003047);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000020);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00181884);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x0000001A);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x33C03C10);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A7);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A8);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A9);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x000D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C7);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C8);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C9);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x000D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E7);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E8);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E9);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x000D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000107);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000108);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000109);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x000D0D0D);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00010181);
   WriteReg_32(DBSC_E3_DBCMD, 0x08840001);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   /*  rev.0.03 add Comment */
   /****************************************************************************
    *  Initial_Step3( WL/QSG training )
    ***************************************************************************/
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00010601);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   for (i = 0; i < 4; i++) {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B1 + i * 0x20);
      RegVal_R5 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0x0000FF00) >> 0x8;
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B4 + i * 0x20);
      RegVal_R6 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0x000000FF);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B3 + i * 0x20);
      RegVal_R7 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0x00000007);
      if (RegVal_R6 > 0) {
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFFF8);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | ((RegVal_R7 + 0x1) & 0x00000007));
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFF00);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | RegVal_R6);
      } else {
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFFF8);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | RegVal_R7);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFF00);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | ((RegVal_R6 + ((RegVal_R5) << 1)) & 0x000000FF));
      } /*  RegVal_R6 */
   } /*  for i */

   /*  rev.0.10 move Comment */
   /****************************************************************************
    *  Initial_Step4( WLADJ training )
    ***************************************************************************/
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000005);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0xC1AA00C0);

   /* rev.0.08 */
   if (pdqsr_ctl == 1){} else {
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000100);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   }

   /* PDR always off */	/* rev.0.10 */
   if (pdr_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000103);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
   }

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00010801);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   /****************************************************************************
    *  Initial_Step5(Read Data Bit Deskew)
    ***************************************************************************/
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000005);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0xC1AA00D8);

   /* rev.0.08 */
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00011001);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

if (pdqsr_ctl == 1) {
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000100);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
}

   /* PDR dynamic */	/* rev.0.10 */
   if (pdr_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000103);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
   }

   /****************************************************************************
    *  Initial_Step6(Write Data Bit Deskew)
    ***************************************************************************/
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00012001);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   /****************************************************************************
    *  Initial_Step7(Read Data Eye Training)
    ***************************************************************************/
if (pdqsr_ctl == 1) {
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000100);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
}

   /* PDR always off */	/* rev.0.10 */
   if (pdr_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000103);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
   }

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00014001);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

if (pdqsr_ctl == 1) {
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000100);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
}

   /* PDR dynamic */	/* rev.0.10 */
   if (pdr_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000103);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
   }

   /****************************************************************************
    *  Initial_Step8(Write Data Eye Training)
    ***************************************************************************/
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00018001);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   /*  rev.0.03 add Comment */
   /****************************************************************************
    *  Initial_Step3_2( DQS Gate Training )
    ***************************************************************************/
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000100);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x0000002C);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x81003087);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00010401);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   for (i = 0; i < 4; i++) {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B1 + i * 0x20);
      RegVal_R5 = ((ReadReg_32(DBSC_E3_DBPDRGD0) & 0x0000FF00) >> 0x8);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B4 + i * 0x20);
      RegVal_R6 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0x000000FF);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B3 + i * 0x20);
      RegVal_R7 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0x00000007);
      RegVal_R12 = (RegVal_R5 >> 0x2);
      if (RegVal_R12 < RegVal_R6) {
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFFF8);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | ((RegVal_R7 + 0x1) & 0x00000007));
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFF00);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | ((RegVal_R6 - (RegVal_R12)) & 0x000000FF));
      } else {
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFFF8);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | (RegVal_R7 & 0x00000007));
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFF00);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | ((RegVal_R6 + (RegVal_R5) + ((RegVal_R5) >> 1) + (RegVal_R12)) & 0x000000FF));
      } /*  RegVal_R12 < RegVal_R6 */
   } /*  for i */

   /*  rev.0.10 move Comment */
   /****************************************************************************
    *  Initial_Step5-2_7-2( Rd bit Rd eye )
    ***************************************************************************/
/* rev.0.08 */
   if (pdqsr_ctl == 1){} else {
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000100);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   }

   /* PDR always off */	/* rev.0.10 */
   if (pdr_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000103);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
   }

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00015001);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

/* rev.0.08 */
   if (lcdl_ctl == 1) {
       for (i = 0; i < 4; i++) {
          WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
	  dqsgd_0c = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0x000000FF);
          WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B1 + i * 0x20);
	  bdlcount_0c = ((ReadReg_32(DBSC_E3_DBPDRGD0) & 0x0000FF00) >> 8);
	  bdlcount_0c_div2  = (bdlcount_0c >> 1);
	  bdlcount_0c_div4  = (bdlcount_0c >> 2);
	  bdlcount_0c_div8  = (bdlcount_0c >> 3);
	  bdlcount_0c_div16 = (bdlcount_0c >> 4);

          if (ddr_md == 0) {                                 /*  1584Mbps */
	     lcdl_judge1 = bdlcount_0c_div2 + bdlcount_0c_div4 + bdlcount_0c_div8;
	     lcdl_judge2 = bdlcount_0c + bdlcount_0c_div4 + bdlcount_0c_div16;
	  } else {                                        /*  1856Mbps */
	     lcdl_judge1 = bdlcount_0c_div2 + bdlcount_0c_div4;
	     lcdl_judge2 = bdlcount_0c + bdlcount_0c_div4;
	  } /*  ddr_md */

	  if (dqsgd_0c > lcdl_judge1) {
	     if (dqsgd_0c <= lcdl_judge2) {
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFF00);
                WriteReg_32(DBSC_E3_DBPDRGD0, ((dqsgd_0c - bdlcount_0c_div8) | RegVal));
	      } else {
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFF00);
		WriteReg_32(DBSC_E3_DBPDRGD0, RegVal);
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
		gatesl_0c = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0x00000007);
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFFF8);
                WriteReg_32(DBSC_E3_DBPDRGD0, (RegVal | (gatesl_0c + 1)));
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000AF + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0));
		rdqsd_0c = (RegVal & 0x0000FF00) >> 8;
		rdqsnd_0c = (RegVal & 0x00FF0000) >> 16;
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000AF + i * 0x20);
                WriteReg_32(DBSC_E3_DBPDRGD0, ((RegVal & 0xFF0000FF) | ((rdqsd_0c + bdlcount_0c_div4) << 8) | ((rdqsnd_0c + bdlcount_0c_div4) << 16)));
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000AA + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0));
                rbd_0c[0] = (RegVal) &0x0000001f;
		rbd_0c[1] = (RegVal >>  8) & 0x0000001f;
		rbd_0c[2] = (RegVal >> 16) & 0x0000001f;
		rbd_0c[3] = (RegVal >> 24) & 0x0000001f;
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000AA + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xE0E0E0E0);
                for (j = 0; j < 4; j++) {
		    rbd_0c[j] = (rbd_0c[j] + bdlcount_0c_div4);
		    if (rbd_0c[j] > 0x1F) rbd_0c[j] = 0x1F;
                    RegVal = RegVal | (rbd_0c[j] << 8 * j);
		}
		WriteReg_32(DBSC_E3_DBPDRGD0, RegVal);
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000AB + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0));
                rbd_0c[0] = (RegVal) &0x0000001f;
		rbd_0c[1] = (RegVal >>  8) & 0x0000001f;
		rbd_0c[2] = (RegVal >> 16) & 0x0000001f;
		rbd_0c[3] = (RegVal >> 24) & 0x0000001f;
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000AB + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xE0E0E0E0);
                for (j = 0; j < 4; j++) {
		    rbd_0c[j] = (rbd_0c[j] + bdlcount_0c_div4);
		    if (rbd_0c[j] > 0x1F) rbd_0c[j] = 0x1F;
                    RegVal = RegVal | (rbd_0c[j] << 8 * j);
		}
		WriteReg_32(DBSC_E3_DBPDRGD0, RegVal);
	     }
	  }
       }
       WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000002);
       WriteReg_32(DBSC_E3_DBPDRGD0, 0x07D81E37);
   }

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000003);
   if (byp_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0380C720);
   } else {
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0380C700);
   }
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000007);
   while ((BIT30 & ReadReg_32(DBSC_E3_DBPDRGD0)) != 0);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000021);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0024643E);

   WriteReg_32(DBSC_E3_DBBUS0CNF1, 0x00000010);
   WriteReg_32(DBSC_E3_DBCALCNF, (uint32_t)(64000000 / REFRESH_RATE) + 0x01000000);
   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBRFCNF1, (uint32_t)(REFRESH_RATE * 99 / 125) + 0x00080000);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBRFCNF1, (uint32_t)(REFRESH_RATE * 116 / 125) + 0x00080000);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBRFCNF2, 0x00010000);
   WriteReg_32(DBSC_E3_DBDFICUPDCNF, 0x40100001);
   WriteReg_32(DBSC_E3_DBRFEN, 0x00000001);
   WriteReg_32(DBSC_E3_DBACEN, 0x00000001);

/* rev.0.08 */
   if (pdqsr_ctl == 1) {
   WriteReg_32(0xE67F0018, 0x00000001);
   RegVal = ReadReg_32(0x40000000);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000000);
   WriteReg_32(DBSC_E3_DBPDRGD0, RegVal);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000100);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   }

   /* PDR dynamic */	/* rev.0.10 */
   if (pdr_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000103);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
   }

   /*  rev.0.03 add Comment */
   /****************************************************************************
    *  Initial_Step9( Initial End )
    ***************************************************************************/
   WriteReg_32(DBSC_E3_DBPDLK0, 0x00000000);
   WriteReg_32(DBSC_E3_DBSYSCNT0, 0x00000000);

#ifdef ddr_qos_init_setting /*  only for non qos_init */
   WriteReg_32(DBSC_E3_DBSYSCNT0, 0x00001234);
   WriteReg_32(DBSC_E3_DBCAM0CNF1, 0x00043218);
   WriteReg_32(DBSC_E3_DBCAM0CNF2, 0x000000F4);
   WriteReg_32(DBSC_E3_DBSCHCNT0, 0x000f0037);
   WriteReg_32(DBSC_E3_DBSCHSZ0, 0x00000001);
   WriteReg_32(DBSC_E3_DBSCHRW0, 0x22421111);
   WriteReg_32(DBSC_E3_SCFCTST2, 0x012F1123);
   WriteReg_32(DBSC_E3_DBSCHQOS00, 0x00000F00);
   WriteReg_32(DBSC_E3_DBSCHQOS01, 0x00000B00);
   WriteReg_32(DBSC_E3_DBSCHQOS02, 0x00000000);
   WriteReg_32(DBSC_E3_DBSCHQOS03, 0x00000000);
   WriteReg_32(DBSC_E3_DBSCHQOS40, 0x00000300);
   WriteReg_32(DBSC_E3_DBSCHQOS41, 0x000002F0);
   WriteReg_32(DBSC_E3_DBSCHQOS42, 0x00000200);
   WriteReg_32(DBSC_E3_DBSCHQOS43, 0x00000100);
   WriteReg_32(DBSC_E3_DBSCHQOS90, 0x00000100);
   WriteReg_32(DBSC_E3_DBSCHQOS91, 0x000000F0);
   WriteReg_32(DBSC_E3_DBSCHQOS92, 0x000000A0);
   WriteReg_32(DBSC_E3_DBSCHQOS93, 0x00000040);
   WriteReg_32(DBSC_E3_DBSCHQOS130, 0x00000100);
   WriteReg_32(DBSC_E3_DBSCHQOS131, 0x000000F0);
   WriteReg_32(DBSC_E3_DBSCHQOS132, 0x000000A0);
   WriteReg_32(DBSC_E3_DBSCHQOS133, 0x00000040);
   WriteReg_32(DBSC_E3_DBSCHQOS140, 0x000000C0);
   WriteReg_32(DBSC_E3_DBSCHQOS141, 0x000000B0);
   WriteReg_32(DBSC_E3_DBSCHQOS142, 0x00000080);
   WriteReg_32(DBSC_E3_DBSCHQOS143, 0x00000040);
   WriteReg_32(DBSC_E3_DBSCHQOS150, 0x00000040);
   WriteReg_32(DBSC_E3_DBSCHQOS151, 0x00000030);
   WriteReg_32(DBSC_E3_DBSCHQOS152, 0x00000020);
   WriteReg_32(DBSC_E3_DBSCHQOS153, 0x00000010);

/* rev.0.08 */
   if (pdqsr_ctl == 1){} else {
   WriteReg_32(0xE67F0018, 0x00000001);
   }

   WriteReg_32(DBSC_E3_DBSYSCNT0, 0x00000000);
#endif

   return 1;   /*  rev.0.04 Restore the return code */

} /*  init_ddr */

/*  rev.0.04 add function */
uint32_t recovery_from_backup_mode(void)
{
   /****************************************************************************
    *  recovery_Step0(DBSC Setting 1) / same "init_ddr"
    ***************************************************************************/
   uint32_t RegVal_R2, RegVal_R5, RegVal_R6, RegVal_R7, RegVal_R12, i;
   uint32_t ddr_md;
   uint32_t err;

/* rev.0.08 */
   uint32_t RegVal, j;
   uint32_t dqsgd_0c, bdlcount_0c, bdlcount_0c_div2, bdlcount_0c_div4, bdlcount_0c_div8, bdlcount_0c_div16;
   uint32_t gatesl_0c, rdqsd_0c, rdqsnd_0c, rbd_0c[4];
   uint32_t pdqsr_ctl, lcdl_ctl, lcdl_judge1, lcdl_judge2;
   /* rev.0.10 */
   uint32_t pdr_ctl;
   /* rev.0.11 */
   uint32_t byp_ctl;

/* rev.0.08 */
   if ((ReadReg_32(0xFFF00044) & 0x000000FF) == 0x00000000) {
     pdqsr_ctl  = 1;
     lcdl_ctl   = 1;
     pdr_ctl    = 1;  /* rev.0.10 */
     byp_ctl    = 1;  /* rev.0.11 */
    } else {
     pdqsr_ctl  = 0;
     lcdl_ctl   = 0;
     pdr_ctl    = 0;  /* rev.0.10 */
     byp_ctl    = 0;  /* rev.0.11 */
   }

   /*  Judge the DDR bit rate (ddr_md : 0 = 1584Mbps, 1 = 1856Mbps) */
   ddr_md = (ReadReg_32(RST_MODEMR) >> 19) & BIT0;

   /*  1584Mbps setting */
   if (ddr_md == 0) {
   /* CPG setting ===============================================*/
   WriteReg_32(CPG_CPGWPR, 0x5A5AFFFF);
   WriteReg_32(CPG_CPGWPCR, 0xA5A50000);

   WriteReg_32(CPG_SRCR4, 0x20000000);

   WriteReg_32(0xE61500DC, 0xe2200000);  /*  Change to 1584Mbps */
   while ((BIT11 & ReadReg_32(CPG_PLLECR)) == 0);

   WriteReg_32(CPG_SRSTCLR4, 0x20000000);

   WriteReg_32(CPG_CPGWPCR, 0xA5A50001);

   /* CPG setting ===============================================*/
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBSYSCNT0, 0x00001234);
   WriteReg_32(DBSC_E3_DBKIND, 0x00000007);

#if RCAR_DRAM_DDR3L_MEMCONF == 0
   WriteReg_32(DBSC_E3_DBMEMCONF00, 0x0f030a02);
#elif RCAR_DRAM_DDR3L_MEMCONF == 1
   WriteReg_32(DBSC_E3_DBMEMCONF00, 0x10030a02);
#elif RCAR_DRAM_DDR3L_MEMCONF == 2
   WriteReg_32(DBSC_E3_DBMEMCONF00, 0x10030b02);
#else
   WriteReg_32(DBSC_E3_DBMEMCONF00, 0x10030a02);
#endif

/* rev.0.08 */
#if RCAR_DRAM_DDR3L_MEMDUAL == 1
	 RegVal_R2 = (ReadReg_32(0xE6790614));
         WriteReg_32(0xE6790614, RegVal_R2 | 0x00000003); /*  MCS1_N/MODT1 are activated. */
#endif

   WriteReg_32(DBSC_E3_DBPHYCONF0, 0x00000001);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBTR0, 0x0000000B);
      WriteReg_32(DBSC_E3_DBTR1, 0x00000008);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBTR0, 0x0000000D);
      WriteReg_32(DBSC_E3_DBTR1, 0x00000009);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBTR2, 0x00000000);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBTR3, 0x0000000B);
      WriteReg_32(DBSC_E3_DBTR4, 0x000B000B);
      WriteReg_32(DBSC_E3_DBTR5, 0x00000027);
      WriteReg_32(DBSC_E3_DBTR6, 0x0000001C);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBTR3, 0x0000000D);
      WriteReg_32(DBSC_E3_DBTR4, 0x000D000D);
      WriteReg_32(DBSC_E3_DBTR5, 0x0000002D);
      WriteReg_32(DBSC_E3_DBTR6, 0x00000020);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBTR7, 0x00060006);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBTR8, 0x00000020);
      WriteReg_32(DBSC_E3_DBTR9, 0x00000006);
      WriteReg_32(DBSC_E3_DBTR10, 0x0000000C);
      WriteReg_32(DBSC_E3_DBTR11, 0x0000000A);
      WriteReg_32(DBSC_E3_DBTR12, 0x00120012);
      WriteReg_32(DBSC_E3_DBTR13, 0x000000CE);
      WriteReg_32(DBSC_E3_DBTR14, 0x00140005);
      WriteReg_32(DBSC_E3_DBTR15, 0x00050004);
      WriteReg_32(DBSC_E3_DBTR16, 0x071F0305);
      WriteReg_32(DBSC_E3_DBTR17, 0x040C0000);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBTR8, 0x00000021);
      WriteReg_32(DBSC_E3_DBTR9, 0x00000007);
      WriteReg_32(DBSC_E3_DBTR10, 0x0000000E);
      WriteReg_32(DBSC_E3_DBTR11, 0x0000000C);
      WriteReg_32(DBSC_E3_DBTR12, 0x00140014);
      WriteReg_32(DBSC_E3_DBTR13, 0x000000F2);
      WriteReg_32(DBSC_E3_DBTR14, 0x00170006);
      WriteReg_32(DBSC_E3_DBTR15, 0x00060005);
      WriteReg_32(DBSC_E3_DBTR16, 0x09210507);
      WriteReg_32(DBSC_E3_DBTR17, 0x040E0000);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBTR18, 0x00000200);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBTR19, 0x01000040);
      WriteReg_32(DBSC_E3_DBTR20, 0x020000D6);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBTR19, 0x0129004B);
      WriteReg_32(DBSC_E3_DBTR20, 0x020000FB);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBTR21, 0x00040004);
   WriteReg_32(DBSC_E3_DBBL, 0x00000000);
   WriteReg_32(DBSC_E3_DBODT0, 0x00000001);
   WriteReg_32(DBSC_E3_DBADJ0, 0x00000001);
   WriteReg_32(DBSC_E3_DBSYSCONF1, 0x00000002);
   WriteReg_32(DBSC_E3_DBDFICNT0, 0x00000010);
   WriteReg_32(DBSC_E3_DBBCAMDIS, 0x00000001);
   WriteReg_32(DBSC_E3_DBSCHRW1, 0x00000046);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_SCFCTST0, 0x0D050B03);
      WriteReg_32(DBSC_E3_SCFCTST1, 0x0306030C);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_SCFCTST0, 0x0C050B03);
      WriteReg_32(DBSC_E3_SCFCTST1, 0x0305030C);
   } /*  ddr_md */

   /****************************************************************************
    *  recovery_Step1(PHY setting 1)
    ***************************************************************************/
   WriteReg_32(DBSC_E3_DBPDLK0, 0x0000A55A);
   WriteReg_32(DBSC_E3_DBCMD, 0x01840001);
   WriteReg_32(DBSC_E3_DBCMD, 0x0A840000);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000008); /*  DDR_PLLCR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x000B8000);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000003); /*  DDR_PGCR1 */
   if (byp_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0780C720);
   } else {
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0780C700);
   }
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000020); /*  DDR_DXCCR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00181884);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x0000001A); /*  DDR_ACIOCR0 */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x33C03C10);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000007);
   while ((BIT30 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000004);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, (uint32_t)(REFRESH_RATE * 792 / 125) - 400 + 0x08B00000);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, (uint32_t)(REFRESH_RATE * 928 / 125) - 400 + 0x0A300000);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000022);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x1000040B);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000023);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x2D9C0B66);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x35A00D77);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000024);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x2A88B400);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x2A8A2C28);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000025);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x30005200);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x30005E00);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000026);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0014A9C9);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0014CB49);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000027);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000D70);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000F14);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000028);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000046);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000029);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      if (REFRESH_RATE > 3900) {
          WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000018);  /*            [7]SRT=0 */
      } else {
          WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000098);  /*            [7]SRT=1 */
      }
   } else {                                        /*  1856Mbps */
      if (REFRESH_RATE > 3900) {
          WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000020);  /*            [7]SRT=0 */
      } else {
          WriteReg_32(DBSC_E3_DBPDRGD0, 0x000000A0);  /*            [7]SRT=1 */
      } /*  REFRESH_RATE */
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x0000002C);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x81003047);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000091);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0007BB6B);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000095);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0007BBAD);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000099);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0007BB6B);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000021); /*  DDR_DSGCR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0024641E);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006); /*  DDR_PGSR0 */
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001); /*  DDR_PIR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x40010000);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006); /*  DDR_PGSR0 */
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000092); /*  DDR_ZQ0DR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0xC2C59AB5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000096); /*  DDR_ZQ1DR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0xC4285FBF);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x0000009A); /*  DDR_ZQ2DR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0xC2C59AB5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000090); /*  DDR_ZQCR */

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0C058900);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0C058A00);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000090); /*  DDR_ZQCR */

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x04058900);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x04058A00);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001); /*  DDR_PIR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00050001);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006); /*  DDR_PGSR0 */
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   /*  ddr backupmode end */
   if (ddrBackup) {
      NOTICE("BL2: [WARM_BOOT]\n");
   } else {
      NOTICE("BL2: [COLD_BOOT]\n");
   } /*  ddrBackup */
   err = rcar_dram_update_boot_status(ddrBackup);
   if (err) {
      NOTICE("BL2: [BOOT_STATUS_UPDATE_ERROR]\n");
      return INITDRAM_ERR_I;
   } /*  err */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000092); /*  DDR_ZQ0DR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x02C59AB5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000096); /*  DDR_ZQ1DR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x04285FBF);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x0000009A); /*  DDR_ZQ2DR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x02C59AB5);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001); /*  DDR_PIR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x08000000);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001); /*  DDR_PIR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000003);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006); /*  DDR_PGSR0 */
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001); /*  DDR_PIR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x80010000);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006); /*  DDR_PGSR0 */
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001); /*  DDR_PIR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00010073);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006); /*  DDR_PGSR0 */
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000090); /*  DDR_ZQCR */

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0C058900);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0C058A00);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000090); /*  DDR_ZQCR */

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x04058900);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x04058A00);
   } /*  ddr_md */

/* rev0.08 */
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x0000000C);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x18000040);

   /****************************************************************************
    *  recovery_Step2(PHY setting 2)
    ***************************************************************************/
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A7);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A8);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A9);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x000D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C7);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C8);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C9);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x000D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E7);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E8);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E9);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x000D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000107);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000108);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0D0D0D0D);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000109);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x000D0D0D);

   WriteReg_32(DBSC_E3_DBCALCNF, (uint32_t)(64000000 / REFRESH_RATE) + 0x01000000);
   WriteReg_32(DBSC_E3_DBBUS0CNF1, 0x00000010);

   /*  Select setting value in bps */
   if (ddr_md == 0) {                                 /*  1584Mbps */
      WriteReg_32(DBSC_E3_DBRFCNF1, (uint32_t)(REFRESH_RATE * 99 / 125) + 0x00080000);
   } else {                                        /*  1856Mbps */
      WriteReg_32(DBSC_E3_DBRFCNF1, (uint32_t)(REFRESH_RATE * 116 / 125) + 0x00080000);
   } /*  ddr_md */

   WriteReg_32(DBSC_E3_DBRFCNF2, 0x00010000);
   WriteReg_32(DBSC_E3_DBRFEN, 0x00000001);
   WriteReg_32(DBSC_E3_DBCMD, 0x0A840001);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBWAIT)) != 0);

   WriteReg_32(DBSC_E3_DBCMD, 0x00000000);

   WriteReg_32(DBSC_E3_DBCMD, 0x04840010);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBWAIT)) != 0);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006); /*  DDR_PGSR0 */
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001); /*  DDR_PIR */
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00010701);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006); /*  DDR_PGSR0 */
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   for (i = 0; i < 4; i++)
   {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B1 + i * 0x20);
      RegVal_R5 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0x0000FF00) >> 0x8;
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B4 + i * 0x20);
      RegVal_R6 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0x000000FF);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B3 + i * 0x20);
      RegVal_R7 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0x00000007);

      if (RegVal_R6 > 0) {
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFFF8);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | ((RegVal_R7 + 0x1) & 0x00000007));
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFF00);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | RegVal_R6);
      } else {
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFFF8);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | RegVal_R7);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFF00);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | ((RegVal_R6 + ((RegVal_R5) << 1)) & 0x000000FF));
      } /*  RegVal_R6 */
   } /*  for i */

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000005);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0xC1AA00C0);

   /* rev.0.08 */
   if (pdqsr_ctl == 1){} else {
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000100);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   }

   /* PDR always off */	/* rev.0.10 */
   if (pdr_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000103);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
   }

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00010801);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000005);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0xC1AA00D8);

   /* rev.0.08 */
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00011001);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

if (pdqsr_ctl == 1) {
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000100);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
}

   /* PDR dynamic */	/* rev.0.10 */
   if (pdr_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000103);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
   }

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00012001);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

if (pdqsr_ctl == 1) {
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000100);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
}

   /* PDR always off */	/* rev.0.10 */
   if (pdr_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000103);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
   }

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00014001);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

if (pdqsr_ctl == 1) {
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000100);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
}

   /* PDR dynamic */	/* rev.0.10 */
   if (pdr_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000103);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
   }

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00018001);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000100);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C000285);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x0000002C);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x81003087);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00010401);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

   for (i = 0; i < 4; i++) {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B1 + i * 0x20);
      RegVal_R5 = ((ReadReg_32(DBSC_E3_DBPDRGD0) & 0x0000FF00) >> 0x8);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B4 + i * 0x20);
      RegVal_R6 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0x000000FF);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B3 + i * 0x20);
      RegVal_R7 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0x00000007);
      RegVal_R12 = (RegVal_R5 >> 0x2);

      if (RegVal_R12 < RegVal_R6) {
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFFF8);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | ((RegVal_R7 + 0x1) & 0x00000007));
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFF00);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | ((RegVal_R6 - (RegVal_R12)) & 0x000000FF));
      } else {
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFFF8);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | (RegVal_R7 & 0x00000007));
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
	 RegVal_R2 = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFF00);
         WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
         WriteReg_32(DBSC_E3_DBPDRGD0, RegVal_R2 | ((RegVal_R6 + (RegVal_R5) + ((RegVal_R5) >> 1) + (RegVal_R12)) & 0x000000FF));
      } /*  RegVal_R12 < RegVal_R6 */
   } /*  for i */

/* rev.0.08 */
   if (pdqsr_ctl == 1){} else {
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000100);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   }

   /* PDR always off */	/* rev.0.10 */
   if (pdr_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000103);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000008);
   }

   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000001);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x00015001);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000006);
   while ((BIT0 & ReadReg_32(DBSC_E3_DBPDRGD0)) == 0);

/* rev.0.08 */
   if (lcdl_ctl == 1) {
       for (i = 0; i < 4; i++) {
          WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
	  dqsgd_0c = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0x000000FF);
          WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B1 + i * 0x20);
	  bdlcount_0c = ((ReadReg_32(DBSC_E3_DBPDRGD0) & 0x0000FF00) >> 8);
	  bdlcount_0c_div2  = (bdlcount_0c >> 1);
	  bdlcount_0c_div4  = (bdlcount_0c >> 2);
	  bdlcount_0c_div8  = (bdlcount_0c >> 3);
	  bdlcount_0c_div16 = (bdlcount_0c >> 4);

          if (ddr_md == 0) {                                 /*  1584Mbps */
	     lcdl_judge1 = bdlcount_0c_div2 + bdlcount_0c_div4 + bdlcount_0c_div8;
	     lcdl_judge2 = bdlcount_0c + bdlcount_0c_div4 + bdlcount_0c_div16;
	  } else {                                        /*  1856Mbps */
	     lcdl_judge1 = bdlcount_0c_div2 + bdlcount_0c_div4;
	     lcdl_judge2 = bdlcount_0c + bdlcount_0c_div4;
	  } /*  ddr_md */

	  if (dqsgd_0c > lcdl_judge1) {
	     if (dqsgd_0c <= lcdl_judge2) {
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFF00);
                WriteReg_32(DBSC_E3_DBPDRGD0, ((dqsgd_0c - bdlcount_0c_div8) | RegVal));
	      } else {
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B0 + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFF00);
		WriteReg_32(DBSC_E3_DBPDRGD0, RegVal);
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
		gatesl_0c = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0x00000007);
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000B2 + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xFFFFFFF8);
                WriteReg_32(DBSC_E3_DBPDRGD0, (RegVal | (gatesl_0c + 1)));
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000AF + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0));
		rdqsd_0c = (RegVal & 0x0000FF00) >> 8;
		rdqsnd_0c = (RegVal & 0x00FF0000) >> 16;
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000AF + i * 0x20);
                WriteReg_32(DBSC_E3_DBPDRGD0, ((RegVal & 0xFF0000FF) | ((rdqsd_0c + bdlcount_0c_div4) << 8) | ((rdqsnd_0c + bdlcount_0c_div4) << 16)));
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000AA + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0));
                rbd_0c[0] = (RegVal) &0x0000001f;
		rbd_0c[1] = (RegVal >>  8) & 0x0000001f;
		rbd_0c[2] = (RegVal >> 16) & 0x0000001f;
		rbd_0c[3] = (RegVal >> 24) & 0x0000001f;
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000AA + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xE0E0E0E0);
                for (j = 0; j < 4; j++) {
		    rbd_0c[j] = (rbd_0c[j] + bdlcount_0c_div4);
		    if (rbd_0c[j] > 0x1F) rbd_0c[j] = 0x1F;
                    RegVal = RegVal | (rbd_0c[j] << 8 * j);
		}
		WriteReg_32(DBSC_E3_DBPDRGD0, RegVal);
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000AB + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0));
                rbd_0c[0] = (RegVal) &0x0000001f;
		rbd_0c[1] = (RegVal >>  8) & 0x0000001f;
		rbd_0c[2] = (RegVal >> 16) & 0x0000001f;
		rbd_0c[3] = (RegVal >> 24) & 0x0000001f;
		WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000AB + i * 0x20);
		RegVal = (ReadReg_32(DBSC_E3_DBPDRGD0) & 0xE0E0E0E0);
                for (j = 0; j < 4; j++) {
		    rbd_0c[j] = (rbd_0c[j] + bdlcount_0c_div4);
		    if (rbd_0c[j] > 0x1F) rbd_0c[j] = 0x1F;
                    RegVal = RegVal | (rbd_0c[j] << 8 * j);
		}
		WriteReg_32(DBSC_E3_DBPDRGD0, RegVal);
	     }
	  }
       }
       WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000002);
       WriteReg_32(DBSC_E3_DBPDRGD0, 0x07D81E37);
   }


   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000003);
   if (byp_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0380C720);
   } else {
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x0380C700);
   }
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000007);
   while ((BIT30 & ReadReg_32(DBSC_E3_DBPDRGD0)) != 0);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000021);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x0024643E);

   /****************************************************************************
    *  recovery_Step3(DBSC Setting 2)
    ***************************************************************************/
   WriteReg_32(DBSC_E3_DBDFICUPDCNF, 0x40100001);
   WriteReg_32(DBSC_E3_DBACEN, 0x00000001);

/* rev.0.08 */
   if (pdqsr_ctl == 1) {
   WriteReg_32(0xE67F0018, 0x00000001);
   RegVal = ReadReg_32(0x40000000);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000000);
   WriteReg_32(DBSC_E3_DBPDRGD0, RegVal);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E0);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000100);
   WriteReg_32(DBSC_E3_DBPDRGD0, 0x7C0002C5);
   }

   /* PDR dynamic */	/* rev.0.10 */
   if (pdr_ctl == 1) {
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000A3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000C3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x000000E3);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
      WriteReg_32(DBSC_E3_DBPDRGA0, 0x00000103);
      WriteReg_32(DBSC_E3_DBPDRGD0, 0x00000000);
   }

   WriteReg_32(DBSC_E3_DBPDLK0, 0x00000000);
   WriteReg_32(DBSC_E3_DBSYSCNT0, 0x00000000);

#ifdef ddr_qos_init_setting /*  only for non qos_init */
   WriteReg_32(DBSC_E3_DBSYSCNT0, 0x00001234);
   WriteReg_32(DBSC_E3_DBCAM0CNF1, 0x00043218);
   WriteReg_32(DBSC_E3_DBCAM0CNF2, 0x000000F4);
   WriteReg_32(DBSC_E3_DBSCHCNT0, 0x000f0037);
   WriteReg_32(DBSC_E3_DBSCHSZ0, 0x00000001);
   WriteReg_32(DBSC_E3_DBSCHRW0, 0x22421111);
   WriteReg_32(DBSC_E3_SCFCTST2, 0x012F1123);
   WriteReg_32(DBSC_E3_DBSCHQOS00, 0x00000F00);
   WriteReg_32(DBSC_E3_DBSCHQOS01, 0x00000B00);
   WriteReg_32(DBSC_E3_DBSCHQOS02, 0x00000000);
   WriteReg_32(DBSC_E3_DBSCHQOS03, 0x00000000);
   WriteReg_32(DBSC_E3_DBSCHQOS40, 0x00000300);
   WriteReg_32(DBSC_E3_DBSCHQOS41, 0x000002F0);
   WriteReg_32(DBSC_E3_DBSCHQOS42, 0x00000200);
   WriteReg_32(DBSC_E3_DBSCHQOS43, 0x00000100);
   WriteReg_32(DBSC_E3_DBSCHQOS90, 0x00000100);
   WriteReg_32(DBSC_E3_DBSCHQOS91, 0x000000F0);
   WriteReg_32(DBSC_E3_DBSCHQOS92, 0x000000A0);
   WriteReg_32(DBSC_E3_DBSCHQOS93, 0x00000040);
   WriteReg_32(DBSC_E3_DBSCHQOS130, 0x00000100);
   WriteReg_32(DBSC_E3_DBSCHQOS131, 0x000000F0);
   WriteReg_32(DBSC_E3_DBSCHQOS132, 0x000000A0);
   WriteReg_32(DBSC_E3_DBSCHQOS133, 0x00000040);
   WriteReg_32(DBSC_E3_DBSCHQOS140, 0x000000C0);
   WriteReg_32(DBSC_E3_DBSCHQOS141, 0x000000B0);
   WriteReg_32(DBSC_E3_DBSCHQOS142, 0x00000080);
   WriteReg_32(DBSC_E3_DBSCHQOS143, 0x00000040);
   WriteReg_32(DBSC_E3_DBSCHQOS150, 0x00000040);
   WriteReg_32(DBSC_E3_DBSCHQOS151, 0x00000030);
   WriteReg_32(DBSC_E3_DBSCHQOS152, 0x00000020);
   WriteReg_32(DBSC_E3_DBSCHQOS153, 0x00000010);

/* rev.0.08 */
   if (pdqsr_ctl == 1){} else {
   WriteReg_32(0xE67F0018, 0x00000001);
   }

   WriteReg_32(DBSC_E3_DBSYSCNT0, 0x00000000);
#endif

   return 1;

} /*  recovery_from_backup_mode */

/*******************************************************************************
 *      init_ddr : MD19=0,DDR3L,1584Mbps / MD19=1,DDR3L,1856Mbps
 ******************************************************************************/

/*******************************************************************************
 *  DDR Initialize entry for IPL
 ******************************************************************************/
int32_t rcar_dram_init(void)
{
    uint32_t dataL;
    uint32_t failcount;
    uint32_t md = 0;
    uint32_t ddr = 0;

    md = *((volatile uint32_t*)RST_MODEMR);
    ddr = (md & 0x00080000) >> 19;
    if (ddr == 0x0) {
	NOTICE("BL2: DDR1584(%s)\n", RCAR_E3_DDR_VERSION);
    } else if(ddr == 0x1){
	NOTICE("BL2: DDR1856(%s)\n", RCAR_E3_DDR_VERSION);
    } /*  ddr */

    rcar_dram_get_boot_status(&ddrBackup);

    if (ddrBackup == DRAM_BOOT_STATUS_WARM) {
        dataL = recovery_from_backup_mode(); /*  WARM boot */
    } else {
        dataL = init_ddr();                  /*  COLD boot */
    } /*  ddrBackup */

    if (dataL == 1) {
        failcount = 0;
    } else {
        failcount = 1;
    } /*  dataL */

    if (failcount == 0) {
	return INITDRAM_OK;
    } else {
	return INITDRAM_NG;
    } /*  failcount */
} /*  InitDram */

/*******************************************************************************
 *  END
 ******************************************************************************/

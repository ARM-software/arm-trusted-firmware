/*
 * Copyright (c) 2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOOT_INIT_DRAM_REGDEF_E3_H
#define BOOT_INIT_DRAM_REGDEF_E3_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define				BIT0						0x00000001U
#define				BIT11						0x00000800U
#define				BIT30						0x40000000U

/* DBSC registers */

#define				DBSC_E3_DBSYSCONF1         0xE6790004U
#define				DBSC_E3_DBPHYCONF0         0xE6790010U
#define				DBSC_E3_DBKIND             0xE6790020U
#define				DBSC_E3_DBMEMCONF00        0xE6790030U
#define				DBSC_E3_DBSYSCNT0          0xE6790100U
#define				DBSC_E3_DBACEN             0xE6790200U
#define				DBSC_E3_DBRFEN             0xE6790204U
#define				DBSC_E3_DBCMD              0xE6790208U
#define				DBSC_E3_DBWAIT             0xE6790210U
#define				DBSC_E3_DBTR0              0xE6790300U
#define				DBSC_E3_DBTR1              0xE6790304U
#define				DBSC_E3_DBTR2              0xE6790308U
#define				DBSC_E3_DBTR3              0xE679030CU
#define				DBSC_E3_DBTR4              0xE6790310U
#define				DBSC_E3_DBTR5              0xE6790314U
#define				DBSC_E3_DBTR6              0xE6790318U
#define				DBSC_E3_DBTR7              0xE679031CU
#define				DBSC_E3_DBTR8              0xE6790320U
#define				DBSC_E3_DBTR9              0xE6790324U
#define				DBSC_E3_DBTR10             0xE6790328U
#define				DBSC_E3_DBTR11             0xE679032CU
#define				DBSC_E3_DBTR12             0xE6790330U
#define				DBSC_E3_DBTR13             0xE6790334U
#define				DBSC_E3_DBTR14             0xE6790338U
#define				DBSC_E3_DBTR15             0xE679033CU
#define				DBSC_E3_DBTR16             0xE6790340U
#define				DBSC_E3_DBTR17             0xE6790344U
#define				DBSC_E3_DBTR18             0xE6790348U
#define				DBSC_E3_DBTR19             0xE679034CU
#define				DBSC_E3_DBTR20             0xE6790350U
#define				DBSC_E3_DBTR21             0xE6790354U
#define				DBSC_E3_DBBL               0xE6790400U
#define				DBSC_E3_DBRFCNF1           0xE6790414U
#define				DBSC_E3_DBRFCNF2           0xE6790418U
#define				DBSC_E3_DBCALCNF           0xE6790424U
#define				DBSC_E3_DBODT0             0xE6790460U
#define				DBSC_E3_DBADJ0             0xE6790500U
#define				DBSC_E3_DBDFICUPDCNF       0xE679052CU
#define				DBSC_E3_DBDFICNT0          0xE6790604U
#define				DBSC_E3_DBPDLK0            0xE6790620U
#define				DBSC_E3_DBPDRGA0           0xE6790624U
#define				DBSC_E3_DBPDRGD0           0xE6790628U
#define				DBSC_E3_DBBUS0CNF1         0xE6790804U
#define				DBSC_E3_DBCAM0CNF1         0xE6790904U
#define				DBSC_E3_DBCAM0CNF2         0xE6790908U
#define				DBSC_E3_DBCAM0STAT0        0xE6790980U
#define				DBSC_E3_DBBCAMDIS          0xE67909FCU
#define				DBSC_E3_DBSCHCNT0          0xE6791000U
#define				DBSC_E3_DBSCHSZ0           0xE6791010U
#define				DBSC_E3_DBSCHRW0           0xE6791020U
#define				DBSC_E3_DBSCHRW1           0xE6791024U
#define				DBSC_E3_DBSCHQOS00         0xE6791030U
#define				DBSC_E3_DBSCHQOS01         0xE6791034U
#define				DBSC_E3_DBSCHQOS02         0xE6791038U
#define				DBSC_E3_DBSCHQOS03         0xE679103CU
#define				DBSC_E3_DBSCHQOS40         0xE6791070U
#define				DBSC_E3_DBSCHQOS41         0xE6791074U
#define				DBSC_E3_DBSCHQOS42         0xE6791078U
#define				DBSC_E3_DBSCHQOS43         0xE679107CU
#define				DBSC_E3_DBSCHQOS90         0xE67910C0U
#define				DBSC_E3_DBSCHQOS91         0xE67910C4U
#define				DBSC_E3_DBSCHQOS92         0xE67910C8U
#define				DBSC_E3_DBSCHQOS93         0xE67910CCU
#define				DBSC_E3_DBSCHQOS130        0xE6791100U
#define				DBSC_E3_DBSCHQOS131        0xE6791104U
#define				DBSC_E3_DBSCHQOS132        0xE6791108U
#define				DBSC_E3_DBSCHQOS133        0xE679110CU
#define				DBSC_E3_DBSCHQOS140        0xE6791110U
#define				DBSC_E3_DBSCHQOS141        0xE6791114U
#define				DBSC_E3_DBSCHQOS142        0xE6791118U
#define				DBSC_E3_DBSCHQOS143        0xE679111CU
#define				DBSC_E3_DBSCHQOS150        0xE6791120U
#define				DBSC_E3_DBSCHQOS151        0xE6791124U
#define				DBSC_E3_DBSCHQOS152        0xE6791128U
#define				DBSC_E3_DBSCHQOS153        0xE679112CU
#define				DBSC_E3_SCFCTST0           0xE6791700U
#define				DBSC_E3_SCFCTST1           0xE6791708U
#define				DBSC_E3_SCFCTST2           0xE679170CU

/* CPG registers */

#define				CPG_SRCR4                  0xE61500BCU
#define				CPG_PLLECR                 0xE61500D0U
#define				CPG_CPGWPR                 0xE6150900U
#define				CPG_CPGWPCR                0xE6150904U
#define				CPG_SRSTCLR4               0xE6150950U

/* MODE Monitor registers */

#define				RST_MODEMR                 0xE6160060U

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BOOT_INIT_DRAM_REGDEF_E3_H */

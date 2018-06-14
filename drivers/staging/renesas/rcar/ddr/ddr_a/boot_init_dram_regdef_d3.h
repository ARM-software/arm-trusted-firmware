/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Revision history
 *
 * rev.0.01    2017/05/22    New
 */

#ifndef BOOT_INIT_DRAM_REGDEF_D3_H_
#define BOOT_INIT_DRAM_REGDEF_D3_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define				BIT0						0x00000001U
#define				BIT30						0x40000000U

/* DBSC registers */

#define				DBSC_D3_DBSYSCONF1         0xE6790004U
#define				DBSC_D3_DBPHYCONF0         0xE6790010U
#define				DBSC_D3_DBKIND             0xE6790020U
#define				DBSC_D3_DBMEMCONF00        0xE6790030U
#define				DBSC_D3_DBMEMCONF01        0xE6790034U
#define				DBSC_D3_DBMEMCONF02        0xE6790038U
#define				DBSC_D3_DBMEMCONF03        0xE679003CU
#define				DBSC_D3_DBMEMCONF10        0xE6790040U
#define				DBSC_D3_DBMEMCONF11        0xE6790044U
#define				DBSC_D3_DBMEMCONF12        0xE6790048U
#define				DBSC_D3_DBMEMCONF13        0xE679004CU
#define				DBSC_D3_DBMEMCONF20        0xE6790050U
#define				DBSC_D3_DBMEMCONF21        0xE6790054U
#define				DBSC_D3_DBMEMCONF22        0xE6790058U
#define				DBSC_D3_DBMEMCONF23        0xE679005CU
#define				DBSC_D3_DBMEMCONF30        0xE6790060U
#define				DBSC_D3_DBMEMCONF31        0xE6790064U
#define				DBSC_D3_DBMEMCONF32        0xE6790068U
#define				DBSC_D3_DBMEMCONF33        0xE679006CU
#define				DBSC_D3_DBSYSCNT0          0xE6790100U
#define				DBSC_D3_DBSVCR1            0xE6790104U
#define				DBSC_D3_DBSTATE0           0xE6790108U
#define				DBSC_D3_DBSTATE1           0xE679010CU
#define				DBSC_D3_DBINTEN            0xE6790180U
#define				DBSC_D3_DBINTSTAT0         0xE6790184U
#define				DBSC_D3_DBACEN             0xE6790200U
#define				DBSC_D3_DBRFEN             0xE6790204U
#define				DBSC_D3_DBCMD              0xE6790208U
#define				DBSC_D3_DBWAIT             0xE6790210U
#define				DBSC_D3_DBSYSCTRL0         0xE6790280U
#define				DBSC_D3_DBTR0              0xE6790300U
#define				DBSC_D3_DBTR1              0xE6790304U
#define				DBSC_D3_DBTR2              0xE6790308U
#define				DBSC_D3_DBTR3              0xE679030CU
#define				DBSC_D3_DBTR4              0xE6790310U
#define				DBSC_D3_DBTR5              0xE6790314U
#define				DBSC_D3_DBTR6              0xE6790318U
#define				DBSC_D3_DBTR7              0xE679031CU
#define				DBSC_D3_DBTR8              0xE6790320U
#define				DBSC_D3_DBTR9              0xE6790324U
#define				DBSC_D3_DBTR10             0xE6790328U
#define				DBSC_D3_DBTR11             0xE679032CU
#define				DBSC_D3_DBTR12             0xE6790330U
#define				DBSC_D3_DBTR13             0xE6790334U
#define				DBSC_D3_DBTR14             0xE6790338U
#define				DBSC_D3_DBTR15             0xE679033CU
#define				DBSC_D3_DBTR16             0xE6790340U
#define				DBSC_D3_DBTR17             0xE6790344U
#define				DBSC_D3_DBTR18             0xE6790348U
#define				DBSC_D3_DBTR19             0xE679034CU
#define				DBSC_D3_DBTR20             0xE6790350U
#define				DBSC_D3_DBTR21             0xE6790354U
#define				DBSC_D3_DBTR22             0xE6790358U
#define				DBSC_D3_DBTR24             0xE6790360U
#define				DBSC_D3_DBTR25             0xE6790364U
#define				DBSC_D3_DBBL               0xE6790400U
#define				DBSC_D3_DBRFCNF1           0xE6790414U
#define				DBSC_D3_DBRFCNF2           0xE6790418U
#define				DBSC_D3_DBCALCNF           0xE6790424U
#define				DBSC_D3_DBRNK2             0xE6790438U
#define				DBSC_D3_DBRNK3             0xE679043CU
#define				DBSC_D3_DBRNK4             0xE6790440U
#define				DBSC_D3_DBRNK5             0xE6790444U
#define				DBSC_D3_DBPDNCNF           0xE6790450U
#define				DBSC_D3_DBODT0             0xE6790460U
#define				DBSC_D3_DBODT1             0xE6790464U
#define				DBSC_D3_DBODT2             0xE6790468U
#define				DBSC_D3_DBODT3             0xE679046CU
#define				DBSC_D3_DBADJ0             0xE6790500U
#define				DBSC_D3_DBDBICNT           0xE6790518U
#define				DBSC_D3_DBDFICUPDCNF       0xE679052CU
#define				DBSC_D3_DBDFICNT0          0xE6790604U
#define				DBSC_D3_DBPDLK0            0xE6790620U
#define				DBSC_D3_DBPDRGA0           0xE6790624U
#define				DBSC_D3_DBPDRGD0           0xE6790628U
#define				DBSC_D3_DBPDSTAT00         0xE6790630U
#define				DBSC_D3_DBDFISTAT1         0xE6790640U
#define				DBSC_D3_DBDFICNT1          0xE6790644U
#define				DBSC_D3_DBPDLK1            0xE6790660U
#define				DBSC_D3_DBPDRGA1           0xE6790664U
#define				DBSC_D3_DBPDRGD1           0xE6790668U
#define				DBSC_D3_DBDFICNT2          0xE6790684U
#define				DBSC_D3_DBPDLK2            0xE67906A0U
#define				DBSC_D3_DBPDRGA2           0xE67906A4U
#define				DBSC_D3_DBPDRGD2           0xE67906A8U
#define				DBSC_D3_DBPDSTAT20         0xE67906B0U
#define				DBSC_D3_DBDFISTAT3         0xE67906C0U
#define				DBSC_D3_DBDFICNT3          0xE67906C4U
#define				DBSC_D3_DBPDLK3            0xE67906E0U
#define				DBSC_D3_DBPDRGA3           0xE67906E4U
#define				DBSC_D3_DBPDRGD3           0xE67906E8U
#define				DBSC_D3_DBBUS0CNF1         0xE6790804U
#define				DBSC_D3_DBCAM0CNF1         0xE6790904U
#define				DBSC_D3_DBCAM0CNF2         0xE6790908U
#define				DBSC_D3_DBCAM0STAT0        0xE6790980U
#define				DBSC_D3_DBCAM1STAT0        0xE6790990U
#define				DBSC_D3_DBBCAMDIS          0xE67909FCU
#define				DBSC_D3_DBSCHCNT0          0xE6791000U
#define				DBSC_D3_DBSCHSZ0           0xE6791010U
#define				DBSC_D3_DBSCHRW0           0xE6791020U
#define				DBSC_D3_DBSCHRW1           0xE6791024U
#define				DBSC_D3_DBSCHQOS00         0xE6791030U
#define				DBSC_D3_DBSCHQOS01         0xE6791034U
#define				DBSC_D3_DBSCHQOS02         0xE6791038U
#define				DBSC_D3_DBSCHQOS03         0xE679103CU
#define				DBSC_D3_DBSCHQOS10         0xE6791040U
#define				DBSC_D3_DBSCHQOS11         0xE6791044U
#define				DBSC_D3_DBSCHQOS12         0xE6791048U
#define				DBSC_D3_DBSCHQOS13         0xE679104CU
#define				DBSC_D3_DBSCHQOS20         0xE6791050U
#define				DBSC_D3_DBSCHQOS21         0xE6791054U
#define				DBSC_D3_DBSCHQOS22         0xE6791058U
#define				DBSC_D3_DBSCHQOS23         0xE679105CU
#define				DBSC_D3_DBSCHQOS30         0xE6791060U
#define				DBSC_D3_DBSCHQOS31         0xE6791064U
#define				DBSC_D3_DBSCHQOS32         0xE6791068U
#define				DBSC_D3_DBSCHQOS33         0xE679106CU
#define				DBSC_D3_DBSCHQOS40         0xE6791070U
#define				DBSC_D3_DBSCHQOS41         0xE6791074U
#define				DBSC_D3_DBSCHQOS42         0xE6791078U
#define				DBSC_D3_DBSCHQOS43         0xE679107CU
#define				DBSC_D3_DBSCHQOS50         0xE6791080U
#define				DBSC_D3_DBSCHQOS51         0xE6791084U
#define				DBSC_D3_DBSCHQOS52         0xE6791088U
#define				DBSC_D3_DBSCHQOS53         0xE679108CU
#define				DBSC_D3_DBSCHQOS60         0xE6791090U
#define				DBSC_D3_DBSCHQOS61         0xE6791094U
#define				DBSC_D3_DBSCHQOS62         0xE6791098U
#define				DBSC_D3_DBSCHQOS63         0xE679109CU
#define				DBSC_D3_DBSCHQOS70         0xE67910A0U
#define				DBSC_D3_DBSCHQOS71         0xE67910A4U
#define				DBSC_D3_DBSCHQOS72         0xE67910A8U
#define				DBSC_D3_DBSCHQOS73         0xE67910ACU
#define				DBSC_D3_DBSCHQOS80         0xE67910B0U
#define				DBSC_D3_DBSCHQOS81         0xE67910B4U
#define				DBSC_D3_DBSCHQOS82         0xE67910B8U
#define				DBSC_D3_DBSCHQOS83         0xE67910BCU
#define				DBSC_D3_DBSCHQOS90         0xE67910C0U
#define				DBSC_D3_DBSCHQOS91         0xE67910C4U
#define				DBSC_D3_DBSCHQOS92         0xE67910C8U
#define				DBSC_D3_DBSCHQOS93         0xE67910CCU
#define				DBSC_D3_DBSCHQOS100        0xE67910D0U
#define				DBSC_D3_DBSCHQOS101        0xE67910D4U
#define				DBSC_D3_DBSCHQOS102        0xE67910D8U
#define				DBSC_D3_DBSCHQOS103        0xE67910DCU
#define				DBSC_D3_DBSCHQOS110        0xE67910E0U
#define				DBSC_D3_DBSCHQOS111        0xE67910E4U
#define				DBSC_D3_DBSCHQOS112        0xE67910E8U
#define				DBSC_D3_DBSCHQOS113        0xE67910ECU
#define				DBSC_D3_DBSCHQOS120        0xE67910F0U
#define				DBSC_D3_DBSCHQOS121        0xE67910F4U
#define				DBSC_D3_DBSCHQOS122        0xE67910F8U
#define				DBSC_D3_DBSCHQOS123        0xE67910FCU
#define				DBSC_D3_DBSCHQOS130        0xE6791100U
#define				DBSC_D3_DBSCHQOS131        0xE6791104U
#define				DBSC_D3_DBSCHQOS132        0xE6791108U
#define				DBSC_D3_DBSCHQOS133        0xE679110CU
#define				DBSC_D3_DBSCHQOS140        0xE6791110U
#define				DBSC_D3_DBSCHQOS141        0xE6791114U
#define				DBSC_D3_DBSCHQOS142        0xE6791118U
#define				DBSC_D3_DBSCHQOS143        0xE679111CU
#define				DBSC_D3_DBSCHQOS150        0xE6791120U
#define				DBSC_D3_DBSCHQOS151        0xE6791124U
#define				DBSC_D3_DBSCHQOS152        0xE6791128U
#define				DBSC_D3_DBSCHQOS153        0xE679112CU
#define				DBSC_D3_SCFCTST0           0xE6791700U
#define				DBSC_D3_SCFCTST1           0xE6791708U
#define				DBSC_D3_SCFCTST2           0xE679170CU
#define				DBSC_D3_DBMRRDR0           0xE6791800U
#define				DBSC_D3_DBMRRDR1           0xE6791804U
#define				DBSC_D3_DBMRRDR2           0xE6791808U
#define				DBSC_D3_DBMRRDR3           0xE679180CU
#define				DBSC_D3_DBMRRDR4           0xE6791810U
#define				DBSC_D3_DBMRRDR5           0xE6791814U
#define				DBSC_D3_DBMRRDR6           0xE6791818U
#define				DBSC_D3_DBMRRDR7           0xE679181CU


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BOOT_INIT_DRAM_REGDEF_D3_H_*/

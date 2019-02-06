/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMMU_H
#define SMMU_H

#include <lib/mmio.h>

#include <memctrl_v2.h>
#include <tegra_def.h>

/*******************************************************************************
 * SMMU Register constants
 ******************************************************************************/
#define SMMU_CBn_SCTLR				(0x0U)
#define SMMU_CBn_SCTLR_STAGE2			(0x0U)
#define SMMU_CBn_ACTLR				(0x4U)
#define SMMU_CBn_RESUME				(0x8U)
#define SMMU_CBn_TCR2				(0x10U)
#define SMMU_CBn_TTBR0_LO			(0x20U)
#define SMMU_CBn_TTBR0_HI			(0x24U)
#define SMMU_CBn_TTBR1_LO			(0x28U)
#define SMMU_CBn_TTBR1_HI			(0x2cU)
#define SMMU_CBn_TCR_LPAE			(0x30U)
#define SMMU_CBn_TCR				(0x30U)
#define SMMU_CBn_TCR_EAE_1			(0x30U)
#define SMMU_CBn_TCR				(0x30U)
#define SMMU_CBn_CONTEXTIDR			(0x34U)
#define SMMU_CBn_CONTEXTIDR_EAE_1		(0x34U)
#define SMMU_CBn_PRRR_MAIR0			(0x38U)
#define SMMU_CBn_NMRR_MAIR1			(0x3cU)
#define SMMU_CBn_SMMU_CBn_PAR			(0x50U)
#define SMMU_CBn_SMMU_CBn_PAR0			(0x50U)
#define SMMU_CBn_SMMU_CBn_PAR1			(0x54U)
/*      SMMU_CBn_SMMU_CBn_PAR0_Fault		(0x50U) */
/*      SMMU_CBn_SMMU_CBn_PAR0_Fault		(0x54U) */
#define SMMU_CBn_FSR				(0x58U)
#define SMMU_CBn_FSRRESTORE			(0x5cU)
#define SMMU_CBn_FAR_LO				(0x60U)
#define SMMU_CBn_FAR_HI				(0x64U)
#define SMMU_CBn_FSYNR0				(0x68U)
#define SMMU_CBn_IPAFAR_LO			(0x70U)
#define SMMU_CBn_IPAFAR_HI			(0x74U)
#define SMMU_CBn_TLBIVA_LO			(0x600U)
#define SMMU_CBn_TLBIVA_HI			(0x604U)
#define SMMU_CBn_TLBIVA_AARCH_32		(0x600U)
#define SMMU_CBn_TLBIVAA_LO			(0x608U)
#define SMMU_CBn_TLBIVAA_HI			(0x60cU)
#define SMMU_CBn_TLBIVAA_AARCH_32		(0x608U)
#define SMMU_CBn_TLBIASID			(0x610U)
#define SMMU_CBn_TLBIALL			(0x618U)
#define SMMU_CBn_TLBIVAL_LO			(0x620U)
#define SMMU_CBn_TLBIVAL_HI			(0x624U)
#define SMMU_CBn_TLBIVAL_AARCH_32		(0x618U)
#define SMMU_CBn_TLBIVAAL_LO			(0x628U)
#define SMMU_CBn_TLBIVAAL_HI			(0x62cU)
#define SMMU_CBn_TLBIVAAL_AARCH_32		(0x628U)
#define SMMU_CBn_TLBIIPAS2_LO			(0x630U)
#define SMMU_CBn_TLBIIPAS2_HI			(0x634U)
#define SMMU_CBn_TLBIIPAS2L_LO			(0x638U)
#define SMMU_CBn_TLBIIPAS2L_HI			(0x63cU)
#define SMMU_CBn_TLBSYNC			(0x7f0U)
#define SMMU_CBn_TLBSTATUS			(0x7f4U)
#define SMMU_CBn_ATSR				(0x800U)
#define SMMU_CBn_PMEVCNTR0			(0xe00U)
#define SMMU_CBn_PMEVCNTR1			(0xe04U)
#define SMMU_CBn_PMEVCNTR2			(0xe08U)
#define SMMU_CBn_PMEVCNTR3			(0xe0cU)
#define SMMU_CBn_PMEVTYPER0			(0xe80U)
#define SMMU_CBn_PMEVTYPER1			(0xe84U)
#define SMMU_CBn_PMEVTYPER2			(0xe88U)
#define SMMU_CBn_PMEVTYPER3			(0xe8cU)
#define SMMU_CBn_PMCFGR				(0xf00U)
#define SMMU_CBn_PMCR				(0xf04U)
#define SMMU_CBn_PMCEID				(0xf20U)
#define SMMU_CBn_PMCNTENSE			(0xf40U)
#define SMMU_CBn_PMCNTENCLR			(0xf44U)
#define SMMU_CBn_PMCNTENSET			(0xf48U)
#define SMMU_CBn_PMINTENCLR			(0xf4cU)
#define SMMU_CBn_PMOVSCLR			(0xf50U)
#define SMMU_CBn_PMOVSSET			(0xf58U)
#define SMMU_CBn_PMAUTHSTATUS			(0xfb8U)
#define SMMU_GNSR0_CR0				(0x0U)
#define SMMU_GNSR0_CR2				(0x8U)
#define SMMU_GNSR0_ACR				(0x10U)
#define SMMU_GNSR0_IDR0				(0x20U)
#define SMMU_GNSR0_IDR1				(0x24U)
#define SMMU_GNSR0_IDR2				(0x28U)
#define SMMU_GNSR0_IDR7				(0x3cU)
#define SMMU_GNSR0_GFAR_LO			(0x40U)
#define SMMU_GNSR0_GFAR_HI			(0x44U)
#define SMMU_GNSR0_GFSR				(0x48U)
#define SMMU_GNSR0_GFSRRESTORE			(0x4cU)
#define SMMU_GNSR0_GFSYNR0			(0x50U)
#define SMMU_GNSR0_GFSYNR1			(0x54U)
#define SMMU_GNSR0_GFSYNR1_v2			(0x54U)
#define SMMU_GNSR0_TLBIVMID			(0x64U)
#define SMMU_GNSR0_TLBIALLNSNH			(0x68U)
#define SMMU_GNSR0_TLBIALLH			(0x6cU)
#define SMMU_GNSR0_TLBGSYNC			(0x70U)
#define SMMU_GNSR0_TLBGSTATUS			(0x74U)
#define SMMU_GNSR0_TLBIVAH_LO			(0x78U)
#define SMMU_GNSR0_TLBIVALH64_LO		(0xb0U)
#define SMMU_GNSR0_TLBIVALH64_HI		(0xb4U)
#define SMMU_GNSR0_TLBIVMIDS1			(0xb8U)
#define SMMU_GNSR0_TLBIVAH64_LO			(0xc0U)
#define SMMU_GNSR0_TLBIVAH64_HI			(0xc4U)
#define SMMU_GNSR0_SMR0				(0x800U)
#define SMMU_GNSR0_SMRn				(0x800U)
#define SMMU_GNSR0_SMR1				(0x804U)
#define SMMU_GNSR0_SMR2				(0x808U)
#define SMMU_GNSR0_SMR3				(0x80cU)
#define SMMU_GNSR0_SMR4				(0x810U)
#define SMMU_GNSR0_SMR5				(0x814U)
#define SMMU_GNSR0_SMR6				(0x818U)
#define SMMU_GNSR0_SMR7				(0x81cU)
#define SMMU_GNSR0_SMR8				(0x820U)
#define SMMU_GNSR0_SMR9				(0x824U)
#define SMMU_GNSR0_SMR10			(0x828U)
#define SMMU_GNSR0_SMR11			(0x82cU)
#define SMMU_GNSR0_SMR12			(0x830U)
#define SMMU_GNSR0_SMR13			(0x834U)
#define SMMU_GNSR0_SMR14			(0x838U)
#define SMMU_GNSR0_SMR15			(0x83cU)
#define SMMU_GNSR0_SMR16			(0x840U)
#define SMMU_GNSR0_SMR17			(0x844U)
#define SMMU_GNSR0_SMR18			(0x848U)
#define SMMU_GNSR0_SMR19			(0x84cU)
#define SMMU_GNSR0_SMR20			(0x850U)
#define SMMU_GNSR0_SMR21			(0x854U)
#define SMMU_GNSR0_SMR22			(0x858U)
#define SMMU_GNSR0_SMR23			(0x85cU)
#define SMMU_GNSR0_SMR24			(0x860U)
#define SMMU_GNSR0_SMR25			(0x864U)
#define SMMU_GNSR0_SMR26			(0x868U)
#define SMMU_GNSR0_SMR27			(0x86cU)
#define SMMU_GNSR0_SMR28			(0x870U)
#define SMMU_GNSR0_SMR29			(0x874U)
#define SMMU_GNSR0_SMR30			(0x878U)
#define SMMU_GNSR0_SMR31			(0x87cU)
#define SMMU_GNSR0_SMR32			(0x880U)
#define SMMU_GNSR0_SMR33			(0x884U)
#define SMMU_GNSR0_SMR34			(0x888U)
#define SMMU_GNSR0_SMR35			(0x88cU)
#define SMMU_GNSR0_SMR36			(0x890U)
#define SMMU_GNSR0_SMR37			(0x894U)
#define SMMU_GNSR0_SMR38			(0x898U)
#define SMMU_GNSR0_SMR39			(0x89cU)
#define SMMU_GNSR0_SMR40			(0x8a0U)
#define SMMU_GNSR0_SMR41			(0x8a4U)
#define SMMU_GNSR0_SMR42			(0x8a8U)
#define SMMU_GNSR0_SMR43			(0x8acU)
#define SMMU_GNSR0_SMR44			(0x8b0U)
#define SMMU_GNSR0_SMR45			(0x8b4U)
#define SMMU_GNSR0_SMR46			(0x8b8U)
#define SMMU_GNSR0_SMR47			(0x8bcU)
#define SMMU_GNSR0_SMR48			(0x8c0U)
#define SMMU_GNSR0_SMR49			(0x8c4U)
#define SMMU_GNSR0_SMR50			(0x8c8U)
#define SMMU_GNSR0_SMR51			(0x8ccU)
#define SMMU_GNSR0_SMR52			(0x8d0U)
#define SMMU_GNSR0_SMR53			(0x8d4U)
#define SMMU_GNSR0_SMR54			(0x8d8U)
#define SMMU_GNSR0_SMR55			(0x8dcU)
#define SMMU_GNSR0_SMR56			(0x8e0U)
#define SMMU_GNSR0_SMR57			(0x8e4U)
#define SMMU_GNSR0_SMR58			(0x8e8U)
#define SMMU_GNSR0_SMR59			(0x8ecU)
#define SMMU_GNSR0_SMR60			(0x8f0U)
#define SMMU_GNSR0_SMR61			(0x8f4U)
#define SMMU_GNSR0_SMR62			(0x8f8U)
#define SMMU_GNSR0_SMR63			(0x8fcU)
#define SMMU_GNSR0_SMR64			(0x900U)
#define SMMU_GNSR0_SMR65			(0x904U)
#define SMMU_GNSR0_SMR66			(0x908U)
#define SMMU_GNSR0_SMR67			(0x90cU)
#define SMMU_GNSR0_SMR68			(0x910U)
#define SMMU_GNSR0_SMR69			(0x914U)
#define SMMU_GNSR0_SMR70			(0x918U)
#define SMMU_GNSR0_SMR71			(0x91cU)
#define SMMU_GNSR0_SMR72			(0x920U)
#define SMMU_GNSR0_SMR73			(0x924U)
#define SMMU_GNSR0_SMR74			(0x928U)
#define SMMU_GNSR0_SMR75			(0x92cU)
#define SMMU_GNSR0_SMR76			(0x930U)
#define SMMU_GNSR0_SMR77			(0x934U)
#define SMMU_GNSR0_SMR78			(0x938U)
#define SMMU_GNSR0_SMR79			(0x93cU)
#define SMMU_GNSR0_SMR80			(0x940U)
#define SMMU_GNSR0_SMR81			(0x944U)
#define SMMU_GNSR0_SMR82			(0x948U)
#define SMMU_GNSR0_SMR83			(0x94cU)
#define SMMU_GNSR0_SMR84			(0x950U)
#define SMMU_GNSR0_SMR85			(0x954U)
#define SMMU_GNSR0_SMR86			(0x958U)
#define SMMU_GNSR0_SMR87			(0x95cU)
#define SMMU_GNSR0_SMR88			(0x960U)
#define SMMU_GNSR0_SMR89			(0x964U)
#define SMMU_GNSR0_SMR90			(0x968U)
#define SMMU_GNSR0_SMR91			(0x96cU)
#define SMMU_GNSR0_SMR92			(0x970U)
#define SMMU_GNSR0_SMR93			(0x974U)
#define SMMU_GNSR0_SMR94			(0x978U)
#define SMMU_GNSR0_SMR95			(0x97cU)
#define SMMU_GNSR0_SMR96			(0x980U)
#define SMMU_GNSR0_SMR97			(0x984U)
#define SMMU_GNSR0_SMR98			(0x988U)
#define SMMU_GNSR0_SMR99			(0x98cU)
#define SMMU_GNSR0_SMR100			(0x990U)
#define SMMU_GNSR0_SMR101			(0x994U)
#define SMMU_GNSR0_SMR102			(0x998U)
#define SMMU_GNSR0_SMR103			(0x99cU)
#define SMMU_GNSR0_SMR104			(0x9a0U)
#define SMMU_GNSR0_SMR105			(0x9a4U)
#define SMMU_GNSR0_SMR106			(0x9a8U)
#define SMMU_GNSR0_SMR107			(0x9acU)
#define SMMU_GNSR0_SMR108			(0x9b0U)
#define SMMU_GNSR0_SMR109			(0x9b4U)
#define SMMU_GNSR0_SMR110			(0x9b8U)
#define SMMU_GNSR0_SMR111			(0x9bcU)
#define SMMU_GNSR0_SMR112			(0x9c0U)
#define SMMU_GNSR0_SMR113			(0x9c4U)
#define SMMU_GNSR0_SMR114			(0x9c8U)
#define SMMU_GNSR0_SMR115			(0x9ccU)
#define SMMU_GNSR0_SMR116			(0x9d0U)
#define SMMU_GNSR0_SMR117			(0x9d4U)
#define SMMU_GNSR0_SMR118			(0x9d8U)
#define SMMU_GNSR0_SMR119			(0x9dcU)
#define SMMU_GNSR0_SMR120			(0x9e0U)
#define SMMU_GNSR0_SMR121			(0x9e4U)
#define SMMU_GNSR0_SMR122			(0x9e8U)
#define SMMU_GNSR0_SMR123			(0x9ecU)
#define SMMU_GNSR0_SMR124			(0x9f0U)
#define SMMU_GNSR0_SMR125			(0x9f4U)
#define SMMU_GNSR0_SMR126			(0x9f8U)
#define SMMU_GNSR0_SMR127			(0x9fcU)
#define SMMU_GNSR0_S2CR0			(0xc00U)
#define SMMU_GNSR0_S2CRn			(0xc00U)
#define SMMU_GNSR0_S2CRn			(0xc00U)
#define SMMU_GNSR0_S2CR1			(0xc04U)
#define SMMU_GNSR0_S2CR2			(0xc08U)
#define SMMU_GNSR0_S2CR3			(0xc0cU)
#define SMMU_GNSR0_S2CR4			(0xc10U)
#define SMMU_GNSR0_S2CR5			(0xc14U)
#define SMMU_GNSR0_S2CR6			(0xc18U)
#define SMMU_GNSR0_S2CR7			(0xc1cU)
#define SMMU_GNSR0_S2CR8			(0xc20U)
#define SMMU_GNSR0_S2CR9			(0xc24U)
#define SMMU_GNSR0_S2CR10			(0xc28U)
#define SMMU_GNSR0_S2CR11			(0xc2cU)
#define SMMU_GNSR0_S2CR12			(0xc30U)
#define SMMU_GNSR0_S2CR13			(0xc34U)
#define SMMU_GNSR0_S2CR14			(0xc38U)
#define SMMU_GNSR0_S2CR15			(0xc3cU)
#define SMMU_GNSR0_S2CR16			(0xc40U)
#define SMMU_GNSR0_S2CR17			(0xc44U)
#define SMMU_GNSR0_S2CR18			(0xc48U)
#define SMMU_GNSR0_S2CR19			(0xc4cU)
#define SMMU_GNSR0_S2CR20			(0xc50U)
#define SMMU_GNSR0_S2CR21			(0xc54U)
#define SMMU_GNSR0_S2CR22			(0xc58U)
#define SMMU_GNSR0_S2CR23			(0xc5cU)
#define SMMU_GNSR0_S2CR24			(0xc60U)
#define SMMU_GNSR0_S2CR25			(0xc64U)
#define SMMU_GNSR0_S2CR26			(0xc68U)
#define SMMU_GNSR0_S2CR27			(0xc6cU)
#define SMMU_GNSR0_S2CR28			(0xc70U)
#define SMMU_GNSR0_S2CR29			(0xc74U)
#define SMMU_GNSR0_S2CR30			(0xc78U)
#define SMMU_GNSR0_S2CR31			(0xc7cU)
#define SMMU_GNSR0_S2CR32			(0xc80U)
#define SMMU_GNSR0_S2CR33			(0xc84U)
#define SMMU_GNSR0_S2CR34			(0xc88U)
#define SMMU_GNSR0_S2CR35			(0xc8cU)
#define SMMU_GNSR0_S2CR36			(0xc90U)
#define SMMU_GNSR0_S2CR37			(0xc94U)
#define SMMU_GNSR0_S2CR38			(0xc98U)
#define SMMU_GNSR0_S2CR39			(0xc9cU)
#define SMMU_GNSR0_S2CR40			(0xca0U)
#define SMMU_GNSR0_S2CR41			(0xca4U)
#define SMMU_GNSR0_S2CR42			(0xca8U)
#define SMMU_GNSR0_S2CR43			(0xcacU)
#define SMMU_GNSR0_S2CR44			(0xcb0U)
#define SMMU_GNSR0_S2CR45			(0xcb4U)
#define SMMU_GNSR0_S2CR46			(0xcb8U)
#define SMMU_GNSR0_S2CR47			(0xcbcU)
#define SMMU_GNSR0_S2CR48			(0xcc0U)
#define SMMU_GNSR0_S2CR49			(0xcc4U)
#define SMMU_GNSR0_S2CR50			(0xcc8U)
#define SMMU_GNSR0_S2CR51			(0xcccU)
#define SMMU_GNSR0_S2CR52			(0xcd0U)
#define SMMU_GNSR0_S2CR53			(0xcd4U)
#define SMMU_GNSR0_S2CR54			(0xcd8U)
#define SMMU_GNSR0_S2CR55			(0xcdcU)
#define SMMU_GNSR0_S2CR56			(0xce0U)
#define SMMU_GNSR0_S2CR57			(0xce4U)
#define SMMU_GNSR0_S2CR58			(0xce8U)
#define SMMU_GNSR0_S2CR59			(0xcecU)
#define SMMU_GNSR0_S2CR60			(0xcf0U)
#define SMMU_GNSR0_S2CR61			(0xcf4U)
#define SMMU_GNSR0_S2CR62			(0xcf8U)
#define SMMU_GNSR0_S2CR63			(0xcfcU)
#define SMMU_GNSR0_S2CR64			(0xd00U)
#define SMMU_GNSR0_S2CR65			(0xd04U)
#define SMMU_GNSR0_S2CR66			(0xd08U)
#define SMMU_GNSR0_S2CR67			(0xd0cU)
#define SMMU_GNSR0_S2CR68			(0xd10U)
#define SMMU_GNSR0_S2CR69			(0xd14U)
#define SMMU_GNSR0_S2CR70			(0xd18U)
#define SMMU_GNSR0_S2CR71			(0xd1cU)
#define SMMU_GNSR0_S2CR72			(0xd20U)
#define SMMU_GNSR0_S2CR73			(0xd24U)
#define SMMU_GNSR0_S2CR74			(0xd28U)
#define SMMU_GNSR0_S2CR75			(0xd2cU)
#define SMMU_GNSR0_S2CR76			(0xd30U)
#define SMMU_GNSR0_S2CR77			(0xd34U)
#define SMMU_GNSR0_S2CR78			(0xd38U)
#define SMMU_GNSR0_S2CR79			(0xd3cU)
#define SMMU_GNSR0_S2CR80			(0xd40U)
#define SMMU_GNSR0_S2CR81			(0xd44U)
#define SMMU_GNSR0_S2CR82			(0xd48U)
#define SMMU_GNSR0_S2CR83			(0xd4cU)
#define SMMU_GNSR0_S2CR84			(0xd50U)
#define SMMU_GNSR0_S2CR85			(0xd54U)
#define SMMU_GNSR0_S2CR86			(0xd58U)
#define SMMU_GNSR0_S2CR87			(0xd5cU)
#define SMMU_GNSR0_S2CR88			(0xd60U)
#define SMMU_GNSR0_S2CR89			(0xd64U)
#define SMMU_GNSR0_S2CR90			(0xd68U)
#define SMMU_GNSR0_S2CR91			(0xd6cU)
#define SMMU_GNSR0_S2CR92			(0xd70U)
#define SMMU_GNSR0_S2CR93			(0xd74U)
#define SMMU_GNSR0_S2CR94			(0xd78U)
#define SMMU_GNSR0_S2CR95			(0xd7cU)
#define SMMU_GNSR0_S2CR96			(0xd80U)
#define SMMU_GNSR0_S2CR97			(0xd84U)
#define SMMU_GNSR0_S2CR98			(0xd88U)
#define SMMU_GNSR0_S2CR99			(0xd8cU)
#define SMMU_GNSR0_S2CR100			(0xd90U)
#define SMMU_GNSR0_S2CR101			(0xd94U)
#define SMMU_GNSR0_S2CR102			(0xd98U)
#define SMMU_GNSR0_S2CR103			(0xd9cU)
#define SMMU_GNSR0_S2CR104			(0xda0U)
#define SMMU_GNSR0_S2CR105			(0xda4U)
#define SMMU_GNSR0_S2CR106			(0xda8U)
#define SMMU_GNSR0_S2CR107			(0xdacU)
#define SMMU_GNSR0_S2CR108			(0xdb0U)
#define SMMU_GNSR0_S2CR109			(0xdb4U)
#define SMMU_GNSR0_S2CR110			(0xdb8U)
#define SMMU_GNSR0_S2CR111			(0xdbcU)
#define SMMU_GNSR0_S2CR112			(0xdc0U)
#define SMMU_GNSR0_S2CR113			(0xdc4U)
#define SMMU_GNSR0_S2CR114			(0xdc8U)
#define SMMU_GNSR0_S2CR115			(0xdccU)
#define SMMU_GNSR0_S2CR116			(0xdd0U)
#define SMMU_GNSR0_S2CR117			(0xdd4U)
#define SMMU_GNSR0_S2CR118			(0xdd8U)
#define SMMU_GNSR0_S2CR119			(0xddcU)
#define SMMU_GNSR0_S2CR120			(0xde0U)
#define SMMU_GNSR0_S2CR121			(0xde4U)
#define SMMU_GNSR0_S2CR122			(0xde8U)
#define SMMU_GNSR0_S2CR123			(0xdecU)
#define SMMU_GNSR0_S2CR124			(0xdf0U)
#define SMMU_GNSR0_S2CR125			(0xdf4U)
#define SMMU_GNSR0_S2CR126			(0xdf8U)
#define SMMU_GNSR0_S2CR127			(0xdfcU)
#define SMMU_GNSR0_PIDR0			(0xfe0U)
#define SMMU_GNSR0_PIDR1			(0xfe4U)
#define SMMU_GNSR0_PIDR2			(0xfe8U)
#define SMMU_GNSR0_PIDR3			(0xfecU)
#define SMMU_GNSR0_PIDR4			(0xfd0U)
#define SMMU_GNSR0_PIDR5			(0xfd4U)
#define SMMU_GNSR0_PIDR6			(0xfd8U)
#define SMMU_GNSR0_PIDR7			(0xfdcU)
#define SMMU_GNSR0_CIDR0			(0xff0U)
#define SMMU_GNSR0_CIDR1			(0xff4U)
#define SMMU_GNSR0_CIDR2			(0xff8U)
#define SMMU_GNSR0_CIDR3			(0xffcU)
#define SMMU_GNSR1_CBAR0			(0x0U)
#define SMMU_GNSR1_CBARn			(0x0U)
#define SMMU_GNSR1_CBFRSYNRA0			(0x400U)
#define SMMU_GNSR1_CBA2R0			(0x800U)
#define SMMU_GNSR1_CBAR1			(0x4U)
#define SMMU_GNSR1_CBFRSYNRA1			(0x404U)
#define SMMU_GNSR1_CBA2R1			(0x804U)
#define SMMU_GNSR1_CBAR2			(0x8U)
#define SMMU_GNSR1_CBFRSYNRA2			(0x408U)
#define SMMU_GNSR1_CBA2R2			(0x808U)
#define SMMU_GNSR1_CBAR3			(0xcU)
#define SMMU_GNSR1_CBFRSYNRA3			(0x40cU)
#define SMMU_GNSR1_CBA2R3			(0x80cU)
#define SMMU_GNSR1_CBAR4			(0x10U)
#define SMMU_GNSR1_CBFRSYNRA4			(0x410U)
#define SMMU_GNSR1_CBA2R4			(0x810U)
#define SMMU_GNSR1_CBAR5			(0x14U)
#define SMMU_GNSR1_CBFRSYNRA5			(0x414U)
#define SMMU_GNSR1_CBA2R5			(0x814U)
#define SMMU_GNSR1_CBAR6			(0x18U)
#define SMMU_GNSR1_CBFRSYNRA6			(0x418U)
#define SMMU_GNSR1_CBA2R6			(0x818U)
#define SMMU_GNSR1_CBAR7			(0x1cU)
#define SMMU_GNSR1_CBFRSYNRA7			(0x41cU)
#define SMMU_GNSR1_CBA2R7			(0x81cU)
#define SMMU_GNSR1_CBAR8			(0x20U)
#define SMMU_GNSR1_CBFRSYNRA8			(0x420U)
#define SMMU_GNSR1_CBA2R8			(0x820U)
#define SMMU_GNSR1_CBAR9			(0x24U)
#define SMMU_GNSR1_CBFRSYNRA9			(0x424U)
#define SMMU_GNSR1_CBA2R9			(0x824U)
#define SMMU_GNSR1_CBAR10			(0x28U)
#define SMMU_GNSR1_CBFRSYNRA10			(0x428U)
#define SMMU_GNSR1_CBA2R10			(0x828U)
#define SMMU_GNSR1_CBAR11			(0x2cU)
#define SMMU_GNSR1_CBFRSYNRA11			(0x42cU)
#define SMMU_GNSR1_CBA2R11			(0x82cU)
#define SMMU_GNSR1_CBAR12			(0x30U)
#define SMMU_GNSR1_CBFRSYNRA12			(0x430U)
#define SMMU_GNSR1_CBA2R12			(0x830U)
#define SMMU_GNSR1_CBAR13			(0x34U)
#define SMMU_GNSR1_CBFRSYNRA13			(0x434U)
#define SMMU_GNSR1_CBA2R13			(0x834U)
#define SMMU_GNSR1_CBAR14			(0x38U)
#define SMMU_GNSR1_CBFRSYNRA14			(0x438U)
#define SMMU_GNSR1_CBA2R14			(0x838U)
#define SMMU_GNSR1_CBAR15			(0x3cU)
#define SMMU_GNSR1_CBFRSYNRA15			(0x43cU)
#define SMMU_GNSR1_CBA2R15			(0x83cU)
#define SMMU_GNSR1_CBAR16			(0x40U)
#define SMMU_GNSR1_CBFRSYNRA16			(0x440U)
#define SMMU_GNSR1_CBA2R16			(0x840U)
#define SMMU_GNSR1_CBAR17			(0x44U)
#define SMMU_GNSR1_CBFRSYNRA17			(0x444U)
#define SMMU_GNSR1_CBA2R17			(0x844U)
#define SMMU_GNSR1_CBAR18			(0x48U)
#define SMMU_GNSR1_CBFRSYNRA18			(0x448U)
#define SMMU_GNSR1_CBA2R18			(0x848U)
#define SMMU_GNSR1_CBAR19			(0x4cU)
#define SMMU_GNSR1_CBFRSYNRA19			(0x44cU)
#define SMMU_GNSR1_CBA2R19			(0x84cU)
#define SMMU_GNSR1_CBAR20			(0x50U)
#define SMMU_GNSR1_CBFRSYNRA20			(0x450U)
#define SMMU_GNSR1_CBA2R20			(0x850U)
#define SMMU_GNSR1_CBAR21			(0x54U)
#define SMMU_GNSR1_CBFRSYNRA21			(0x454U)
#define SMMU_GNSR1_CBA2R21			(0x854U)
#define SMMU_GNSR1_CBAR22			(0x58U)
#define SMMU_GNSR1_CBFRSYNRA22			(0x458U)
#define SMMU_GNSR1_CBA2R22			(0x858U)
#define SMMU_GNSR1_CBAR23			(0x5cU)
#define SMMU_GNSR1_CBFRSYNRA23			(0x45cU)
#define SMMU_GNSR1_CBA2R23			(0x85cU)
#define SMMU_GNSR1_CBAR24			(0x60U)
#define SMMU_GNSR1_CBFRSYNRA24			(0x460U)
#define SMMU_GNSR1_CBA2R24			(0x860U)
#define SMMU_GNSR1_CBAR25			(0x64U)
#define SMMU_GNSR1_CBFRSYNRA25			(0x464U)
#define SMMU_GNSR1_CBA2R25			(0x864U)
#define SMMU_GNSR1_CBAR26			(0x68U)
#define SMMU_GNSR1_CBFRSYNRA26			(0x468U)
#define SMMU_GNSR1_CBA2R26			(0x868U)
#define SMMU_GNSR1_CBAR27			(0x6cU)
#define SMMU_GNSR1_CBFRSYNRA27			(0x46cU)
#define SMMU_GNSR1_CBA2R27			(0x86cU)
#define SMMU_GNSR1_CBAR28			(0x70U)
#define SMMU_GNSR1_CBFRSYNRA28			(0x470U)
#define SMMU_GNSR1_CBA2R28			(0x870U)
#define SMMU_GNSR1_CBAR29			(0x74U)
#define SMMU_GNSR1_CBFRSYNRA29			(0x474U)
#define SMMU_GNSR1_CBA2R29			(0x874U)
#define SMMU_GNSR1_CBAR30			(0x78U)
#define SMMU_GNSR1_CBFRSYNRA30			(0x478U)
#define SMMU_GNSR1_CBA2R30			(0x878U)
#define SMMU_GNSR1_CBAR31			(0x7cU)
#define SMMU_GNSR1_CBFRSYNRA31			(0x47cU)
#define SMMU_GNSR1_CBA2R31			(0x87cU)
#define SMMU_GNSR1_CBAR32			(0x80U)
#define SMMU_GNSR1_CBFRSYNRA32			(0x480U)
#define SMMU_GNSR1_CBA2R32			(0x880U)
#define SMMU_GNSR1_CBAR33			(0x84U)
#define SMMU_GNSR1_CBFRSYNRA33			(0x484U)
#define SMMU_GNSR1_CBA2R33			(0x884U)
#define SMMU_GNSR1_CBAR34			(0x88U)
#define SMMU_GNSR1_CBFRSYNRA34			(0x488U)
#define SMMU_GNSR1_CBA2R34			(0x888U)
#define SMMU_GNSR1_CBAR35			(0x8cU)
#define SMMU_GNSR1_CBFRSYNRA35			(0x48cU)
#define SMMU_GNSR1_CBA2R35			(0x88cU)
#define SMMU_GNSR1_CBAR36			(0x90U)
#define SMMU_GNSR1_CBFRSYNRA36			(0x490U)
#define SMMU_GNSR1_CBA2R36			(0x890U)
#define SMMU_GNSR1_CBAR37			(0x94U)
#define SMMU_GNSR1_CBFRSYNRA37			(0x494U)
#define SMMU_GNSR1_CBA2R37			(0x894U)
#define SMMU_GNSR1_CBAR38			(0x98U)
#define SMMU_GNSR1_CBFRSYNRA38			(0x498U)
#define SMMU_GNSR1_CBA2R38			(0x898U)
#define SMMU_GNSR1_CBAR39			(0x9cU)
#define SMMU_GNSR1_CBFRSYNRA39			(0x49cU)
#define SMMU_GNSR1_CBA2R39			(0x89cU)
#define SMMU_GNSR1_CBAR40			(0xa0U)
#define SMMU_GNSR1_CBFRSYNRA40			(0x4a0U)
#define SMMU_GNSR1_CBA2R40			(0x8a0U)
#define SMMU_GNSR1_CBAR41			(0xa4U)
#define SMMU_GNSR1_CBFRSYNRA41			(0x4a4U)
#define SMMU_GNSR1_CBA2R41			(0x8a4U)
#define SMMU_GNSR1_CBAR42			(0xa8U)
#define SMMU_GNSR1_CBFRSYNRA42			(0x4a8U)
#define SMMU_GNSR1_CBA2R42			(0x8a8U)
#define SMMU_GNSR1_CBAR43			(0xacU)
#define SMMU_GNSR1_CBFRSYNRA43			(0x4acU)
#define SMMU_GNSR1_CBA2R43			(0x8acU)
#define SMMU_GNSR1_CBAR44			(0xb0U)
#define SMMU_GNSR1_CBFRSYNRA44			(0x4b0U)
#define SMMU_GNSR1_CBA2R44			(0x8b0U)
#define SMMU_GNSR1_CBAR45			(0xb4U)
#define SMMU_GNSR1_CBFRSYNRA45			(0x4b4U)
#define SMMU_GNSR1_CBA2R45			(0x8b4U)
#define SMMU_GNSR1_CBAR46			(0xb8U)
#define SMMU_GNSR1_CBFRSYNRA46			(0x4b8U)
#define SMMU_GNSR1_CBA2R46			(0x8b8U)
#define SMMU_GNSR1_CBAR47			(0xbcU)
#define SMMU_GNSR1_CBFRSYNRA47			(0x4bcU)
#define SMMU_GNSR1_CBA2R47			(0x8bcU)
#define SMMU_GNSR1_CBAR48			(0xc0U)
#define SMMU_GNSR1_CBFRSYNRA48			(0x4c0U)
#define SMMU_GNSR1_CBA2R48			(0x8c0U)
#define SMMU_GNSR1_CBAR49			(0xc4U)
#define SMMU_GNSR1_CBFRSYNRA49			(0x4c4U)
#define SMMU_GNSR1_CBA2R49			(0x8c4U)
#define SMMU_GNSR1_CBAR50			(0xc8U)
#define SMMU_GNSR1_CBFRSYNRA50			(0x4c8U)
#define SMMU_GNSR1_CBA2R50			(0x8c8U)
#define SMMU_GNSR1_CBAR51			(0xccU)
#define SMMU_GNSR1_CBFRSYNRA51			(0x4ccU)
#define SMMU_GNSR1_CBA2R51			(0x8ccU)
#define SMMU_GNSR1_CBAR52			(0xd0U)
#define SMMU_GNSR1_CBFRSYNRA52			(0x4d0U)
#define SMMU_GNSR1_CBA2R52			(0x8d0U)
#define SMMU_GNSR1_CBAR53			(0xd4U)
#define SMMU_GNSR1_CBFRSYNRA53			(0x4d4U)
#define SMMU_GNSR1_CBA2R53			(0x8d4U)
#define SMMU_GNSR1_CBAR54			(0xd8U)
#define SMMU_GNSR1_CBFRSYNRA54			(0x4d8U)
#define SMMU_GNSR1_CBA2R54			(0x8d8U)
#define SMMU_GNSR1_CBAR55			(0xdcU)
#define SMMU_GNSR1_CBFRSYNRA55			(0x4dcU)
#define SMMU_GNSR1_CBA2R55			(0x8dcU)
#define SMMU_GNSR1_CBAR56			(0xe0U)
#define SMMU_GNSR1_CBFRSYNRA56			(0x4e0U)
#define SMMU_GNSR1_CBA2R56			(0x8e0U)
#define SMMU_GNSR1_CBAR57			(0xe4U)
#define SMMU_GNSR1_CBFRSYNRA57			(0x4e4U)
#define SMMU_GNSR1_CBA2R57			(0x8e4U)
#define SMMU_GNSR1_CBAR58			(0xe8U)
#define SMMU_GNSR1_CBFRSYNRA58			(0x4e8U)
#define SMMU_GNSR1_CBA2R58			(0x8e8U)
#define SMMU_GNSR1_CBAR59			(0xecU)
#define SMMU_GNSR1_CBFRSYNRA59			(0x4ecU)
#define SMMU_GNSR1_CBA2R59			(0x8ecU)
#define SMMU_GNSR1_CBAR60			(0xf0U)
#define SMMU_GNSR1_CBFRSYNRA60			(0x4f0U)
#define SMMU_GNSR1_CBA2R60			(0x8f0U)
#define SMMU_GNSR1_CBAR61			(0xf4U)
#define SMMU_GNSR1_CBFRSYNRA61			(0x4f4U)
#define SMMU_GNSR1_CBA2R61			(0x8f4U)
#define SMMU_GNSR1_CBAR62			(0xf8U)
#define SMMU_GNSR1_CBFRSYNRA62			(0x4f8U)
#define SMMU_GNSR1_CBA2R62			(0x8f8U)
#define SMMU_GNSR1_CBAR63			(0xfcU)
#define SMMU_GNSR1_CBFRSYNRA63			(0x4fcU)
#define SMMU_GNSR1_CBA2R63			(0x8fcU)

/*******************************************************************************
 * SMMU Global Secure Aux. Configuration Register
 ******************************************************************************/
#define SMMU_GSR0_SECURE_ACR			0x10U
#define SMMU_GNSR_ACR				(SMMU_GSR0_SECURE_ACR + 0x400U)
#define SMMU_GSR0_PGSIZE_SHIFT			16U
#define SMMU_GSR0_PGSIZE_4K			(0U << SMMU_GSR0_PGSIZE_SHIFT)
#define SMMU_GSR0_PGSIZE_64K			(1U << SMMU_GSR0_PGSIZE_SHIFT)
#define SMMU_ACR_CACHE_LOCK_ENABLE_BIT		(1U << 26)

/*******************************************************************************
 * SMMU Global Aux. Control Register
 ******************************************************************************/
#define SMMU_CBn_ACTLR_CPRE_BIT			(1ULL << 1U)

/*******************************************************************************
 * SMMU configuration constants
 ******************************************************************************/
#define ID1_PAGESIZE				(1U << 31U)
#define ID1_NUMPAGENDXB_SHIFT			28U
#define ID1_NUMPAGENDXB_MASK			7U
#define ID1_NUMS2CB_SHIFT			16U
#define ID1_NUMS2CB_MASK			0xffU
#define ID1_NUMCB_SHIFT				0U
#define ID1_NUMCB_MASK				0xffU
#define PGSHIFT					16U
#define CB_SIZE					0x800000U

typedef struct smmu_regs {
	uint32_t reg;
	uint32_t val;
} smmu_regs_t;

#define mc_make_sid_override_cfg(name) \
	{ \
		.reg = TEGRA_MC_STREAMID_BASE + MC_STREAMID_OVERRIDE_CFG_ ## name, \
		.val = 0x00000000U, \
	}

#define mc_make_sid_security_cfg(name) \
	{ \
		.reg = TEGRA_MC_STREAMID_BASE + MC_STREAMID_OVERRIDE_TO_SECURITY_CFG(MC_STREAMID_OVERRIDE_CFG_ ## name), \
		.val = 0x00000000U, \
	}

#define smmu_make_gnsr0_sec_cfg(base_addr, name) \
	{ \
		.reg = base_addr + SMMU_GNSR0_ ## name, \
		.val = 0x00000000U, \
	}

/*
 * On ARM-SMMU, conditional offset to access secure aliases of non-secure registers
 * is 0x400. So, add it to register address
 */
#define smmu_make_gnsr0_nsec_cfg(base_addr, name) \
	{ \
		.reg = base_addr + 0x400U + SMMU_GNSR0_ ## name, \
		.val = 0x00000000U, \
	}

#define smmu_make_gnsr0_smr_cfg(base_addr, n) \
	{ \
		.reg = base_addr + SMMU_GNSR0_SMR ## n, \
		.val = 0x00000000U, \
	}

#define smmu_make_gnsr0_s2cr_cfg(base_addr, n) \
	{ \
		.reg = base_addr + SMMU_GNSR0_S2CR ## n, \
		.val = 0x00000000U, \
	}

#define smmu_make_gnsr1_cbar_cfg(base_addr, n) \
	{ \
		.reg = base_addr + (1U << PGSHIFT) + SMMU_GNSR1_CBAR ## n, \
		.val = 0x00000000U, \
	}

#define smmu_make_gnsr1_cba2r_cfg(base_addr, n) \
	{ \
		.reg = base_addr + (1U << PGSHIFT) + SMMU_GNSR1_CBA2R ## n, \
		.val = 0x00000000U, \
	}

#define smmu_make_cb_cfg(base_addr, name, n) \
	{ \
		.reg = base_addr + (CB_SIZE >> 1) + (n * (1 << PGSHIFT)) \
			+ SMMU_CBn_ ## name, \
		.val = 0x00000000U, \
	}

#define smmu_make_smrg_group(base_addr, n)	\
	smmu_make_gnsr0_smr_cfg(base_addr, n),	\
	smmu_make_gnsr0_s2cr_cfg(base_addr, n),	\
	smmu_make_gnsr1_cbar_cfg(base_addr, n),	\
	smmu_make_gnsr1_cba2r_cfg(base_addr, n)	/* don't put "," here. */

#define smmu_make_cb_group(base_addr, n)		\
	smmu_make_cb_cfg(base_addr, SCTLR, n),	\
	smmu_make_cb_cfg(base_addr, TCR2, n),	\
	smmu_make_cb_cfg(base_addr, TTBR0_LO, n),	\
	smmu_make_cb_cfg(base_addr, TTBR0_HI, n),	\
	smmu_make_cb_cfg(base_addr, TCR, n),	\
	smmu_make_cb_cfg(base_addr, PRRR_MAIR0, n),\
	smmu_make_cb_cfg(base_addr, FSR, n),	\
	smmu_make_cb_cfg(base_addr, FAR_LO, n),	\
	smmu_make_cb_cfg(base_addr, FAR_HI, n),	\
	smmu_make_cb_cfg(base_addr, FSYNR0, n)	/* don't put "," here. */

#define smmu_make_cfg(base_addr)			\
	smmu_make_gnsr0_nsec_cfg(base_addr, CR0),	\
	smmu_make_gnsr0_sec_cfg(base_addr, IDR0),	\
	smmu_make_gnsr0_sec_cfg(base_addr, IDR1),	\
	smmu_make_gnsr0_sec_cfg(base_addr, IDR2),	\
	smmu_make_gnsr0_nsec_cfg(base_addr, GFSR),	\
	smmu_make_gnsr0_nsec_cfg(base_addr, GFSYNR0),	\
	smmu_make_gnsr0_nsec_cfg(base_addr, GFSYNR1),	\
	smmu_make_gnsr0_nsec_cfg(base_addr, TLBGSTATUS),\
	smmu_make_gnsr0_nsec_cfg(base_addr, PIDR2),	\
	smmu_make_smrg_group(base_addr, 0),		\
	smmu_make_smrg_group(base_addr, 1),		\
	smmu_make_smrg_group(base_addr, 2),		\
	smmu_make_smrg_group(base_addr, 3),		\
	smmu_make_smrg_group(base_addr, 4),		\
	smmu_make_smrg_group(base_addr, 5),		\
	smmu_make_smrg_group(base_addr, 6),		\
	smmu_make_smrg_group(base_addr, 7),		\
	smmu_make_smrg_group(base_addr, 8),		\
	smmu_make_smrg_group(base_addr, 9),		\
	smmu_make_smrg_group(base_addr, 10),		\
	smmu_make_smrg_group(base_addr, 11),		\
	smmu_make_smrg_group(base_addr, 12),		\
	smmu_make_smrg_group(base_addr, 13),		\
	smmu_make_smrg_group(base_addr, 14),		\
	smmu_make_smrg_group(base_addr, 15),		\
	smmu_make_smrg_group(base_addr, 16),		\
	smmu_make_smrg_group(base_addr, 17),		\
	smmu_make_smrg_group(base_addr, 18),		\
	smmu_make_smrg_group(base_addr, 19),		\
	smmu_make_smrg_group(base_addr, 20),		\
	smmu_make_smrg_group(base_addr, 21),		\
	smmu_make_smrg_group(base_addr, 22),		\
	smmu_make_smrg_group(base_addr, 23),		\
	smmu_make_smrg_group(base_addr, 24),		\
	smmu_make_smrg_group(base_addr, 25),		\
	smmu_make_smrg_group(base_addr, 26),		\
	smmu_make_smrg_group(base_addr, 27),		\
	smmu_make_smrg_group(base_addr, 28),		\
	smmu_make_smrg_group(base_addr, 29),		\
	smmu_make_smrg_group(base_addr, 30),		\
	smmu_make_smrg_group(base_addr, 31),		\
	smmu_make_smrg_group(base_addr, 32),		\
	smmu_make_smrg_group(base_addr, 33),		\
	smmu_make_smrg_group(base_addr, 34),		\
	smmu_make_smrg_group(base_addr, 35),		\
	smmu_make_smrg_group(base_addr, 36),		\
	smmu_make_smrg_group(base_addr, 37),		\
	smmu_make_smrg_group(base_addr, 38),		\
	smmu_make_smrg_group(base_addr, 39),		\
	smmu_make_smrg_group(base_addr, 40),		\
	smmu_make_smrg_group(base_addr, 41),		\
	smmu_make_smrg_group(base_addr, 42),		\
	smmu_make_smrg_group(base_addr, 43),		\
	smmu_make_smrg_group(base_addr, 44),		\
	smmu_make_smrg_group(base_addr, 45),		\
	smmu_make_smrg_group(base_addr, 46),		\
	smmu_make_smrg_group(base_addr, 47),		\
	smmu_make_smrg_group(base_addr, 48),		\
	smmu_make_smrg_group(base_addr, 49),		\
	smmu_make_smrg_group(base_addr, 50),		\
	smmu_make_smrg_group(base_addr, 51),		\
	smmu_make_smrg_group(base_addr, 52),		\
	smmu_make_smrg_group(base_addr, 53),		\
	smmu_make_smrg_group(base_addr, 54),		\
	smmu_make_smrg_group(base_addr, 55),		\
	smmu_make_smrg_group(base_addr, 56),		\
	smmu_make_smrg_group(base_addr, 57),		\
	smmu_make_smrg_group(base_addr, 58),		\
	smmu_make_smrg_group(base_addr, 59),		\
	smmu_make_smrg_group(base_addr, 60),		\
	smmu_make_smrg_group(base_addr, 61),		\
	smmu_make_smrg_group(base_addr, 62),		\
	smmu_make_smrg_group(base_addr, 63),		\
	smmu_make_cb_group(base_addr, 0),		\
	smmu_make_cb_group(base_addr, 1),		\
	smmu_make_cb_group(base_addr, 2),		\
	smmu_make_cb_group(base_addr, 3),		\
	smmu_make_cb_group(base_addr, 4),		\
	smmu_make_cb_group(base_addr, 5),		\
	smmu_make_cb_group(base_addr, 6),		\
	smmu_make_cb_group(base_addr, 7),		\
	smmu_make_cb_group(base_addr, 8),		\
	smmu_make_cb_group(base_addr, 9),		\
	smmu_make_cb_group(base_addr, 10),		\
	smmu_make_cb_group(base_addr, 11),		\
	smmu_make_cb_group(base_addr, 12),		\
	smmu_make_cb_group(base_addr, 13),		\
	smmu_make_cb_group(base_addr, 14),		\
	smmu_make_cb_group(base_addr, 15),		\
	smmu_make_cb_group(base_addr, 16),		\
	smmu_make_cb_group(base_addr, 17),		\
	smmu_make_cb_group(base_addr, 18),		\
	smmu_make_cb_group(base_addr, 19),		\
	smmu_make_cb_group(base_addr, 20),		\
	smmu_make_cb_group(base_addr, 21),		\
	smmu_make_cb_group(base_addr, 22),		\
	smmu_make_cb_group(base_addr, 23),		\
	smmu_make_cb_group(base_addr, 24),		\
	smmu_make_cb_group(base_addr, 25),		\
	smmu_make_cb_group(base_addr, 26),		\
	smmu_make_cb_group(base_addr, 27),		\
	smmu_make_cb_group(base_addr, 28),		\
	smmu_make_cb_group(base_addr, 29),		\
	smmu_make_cb_group(base_addr, 30),		\
	smmu_make_cb_group(base_addr, 31),		\
	smmu_make_cb_group(base_addr, 32),		\
	smmu_make_cb_group(base_addr, 33),		\
	smmu_make_cb_group(base_addr, 34),		\
	smmu_make_cb_group(base_addr, 35),		\
	smmu_make_cb_group(base_addr, 36),		\
	smmu_make_cb_group(base_addr, 37),		\
	smmu_make_cb_group(base_addr, 38),		\
	smmu_make_cb_group(base_addr, 39),		\
	smmu_make_cb_group(base_addr, 40),		\
	smmu_make_cb_group(base_addr, 41),		\
	smmu_make_cb_group(base_addr, 42),		\
	smmu_make_cb_group(base_addr, 43),		\
	smmu_make_cb_group(base_addr, 44),		\
	smmu_make_cb_group(base_addr, 45),		\
	smmu_make_cb_group(base_addr, 46),		\
	smmu_make_cb_group(base_addr, 47),		\
	smmu_make_cb_group(base_addr, 48),		\
	smmu_make_cb_group(base_addr, 49),		\
	smmu_make_cb_group(base_addr, 50),		\
	smmu_make_cb_group(base_addr, 51),		\
	smmu_make_cb_group(base_addr, 52),		\
	smmu_make_cb_group(base_addr, 53),		\
	smmu_make_cb_group(base_addr, 54),		\
	smmu_make_cb_group(base_addr, 55),		\
	smmu_make_cb_group(base_addr, 56),		\
	smmu_make_cb_group(base_addr, 57),		\
	smmu_make_cb_group(base_addr, 58),		\
	smmu_make_cb_group(base_addr, 59),		\
	smmu_make_cb_group(base_addr, 60),		\
	smmu_make_cb_group(base_addr, 61),		\
	smmu_make_cb_group(base_addr, 62),		\
	smmu_make_cb_group(base_addr, 63)	/* don't put "," here. */

#define smmu_bypass_cfg \
	{ \
		.reg = TEGRA_MC_BASE + MC_SMMU_BYPASS_CONFIG, \
		.val = 0x00000000U, \
	}

#define _START_OF_TABLE_ \
	{ \
		.reg = 0xCAFE05C7U, \
		.val = 0x00000000U, \
	}

#define _END_OF_TABLE_ \
	{ \
		.reg = 0xFFFFFFFFU, \
		.val = 0xFFFFFFFFU, \
	}


void tegra_smmu_init(void);
void tegra_smmu_save_context(uint64_t smmu_ctx_addr);
smmu_regs_t *plat_get_smmu_ctx(void);
uint32_t plat_get_num_smmu_devices(void);

#endif /* SMMU_H */

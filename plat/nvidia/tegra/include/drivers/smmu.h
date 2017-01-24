/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __SMMU_H
#define __SMMU_H

#include <memctrl_v2.h>
#include <mmio.h>
#include <tegra_def.h>

/*******************************************************************************
 * SMMU Register constants
 ******************************************************************************/
#define SMMU_CBn_SCTLR				(0x0)
#define SMMU_CBn_SCTLR_STAGE2			(0x0)
#define SMMU_CBn_ACTLR				(0x4)
#define SMMU_CBn_RESUME				(0x8)
#define SMMU_CBn_TCR2				(0x10)
#define SMMU_CBn_TTBR0_LO			(0x20)
#define SMMU_CBn_TTBR0_HI			(0x24)
#define SMMU_CBn_TTBR1_LO			(0x28)
#define SMMU_CBn_TTBR1_HI			(0x2c)
#define SMMU_CBn_TCR_LPAE			(0x30)
#define SMMU_CBn_TCR				(0x30)
#define SMMU_CBn_TCR_EAE_1			(0x30)
#define SMMU_CBn_TCR				(0x30)
#define SMMU_CBn_CONTEXTIDR			(0x34)
#define SMMU_CBn_CONTEXTIDR_EAE_1		(0x34)
#define SMMU_CBn_PRRR_MAIR0			(0x38)
#define SMMU_CBn_NMRR_MAIR1			(0x3c)
#define SMMU_CBn_SMMU_CBn_PAR			(0x50)
#define SMMU_CBn_SMMU_CBn_PAR0			(0x50)
#define SMMU_CBn_SMMU_CBn_PAR1			(0x54)
/*      SMMU_CBn_SMMU_CBn_PAR0_Fault		(0x50) */
/*      SMMU_CBn_SMMU_CBn_PAR0_Fault		(0x54) */
#define SMMU_CBn_FSR				(0x58)
#define SMMU_CBn_FSRRESTORE			(0x5c)
#define SMMU_CBn_FAR_LO				(0x60)
#define SMMU_CBn_FAR_HI				(0x64)
#define SMMU_CBn_FSYNR0				(0x68)
#define SMMU_CBn_IPAFAR_LO			(0x70)
#define SMMU_CBn_IPAFAR_HI			(0x74)
#define SMMU_CBn_TLBIVA_LO			(0x600)
#define SMMU_CBn_TLBIVA_HI			(0x604)
#define SMMU_CBn_TLBIVA_AARCH_32		(0x600)
#define SMMU_CBn_TLBIVAA_LO			(0x608)
#define SMMU_CBn_TLBIVAA_HI			(0x60c)
#define SMMU_CBn_TLBIVAA_AARCH_32		(0x608)
#define SMMU_CBn_TLBIASID			(0x610)
#define SMMU_CBn_TLBIALL			(0x618)
#define SMMU_CBn_TLBIVAL_LO			(0x620)
#define SMMU_CBn_TLBIVAL_HI			(0x624)
#define SMMU_CBn_TLBIVAL_AARCH_32		(0x618)
#define SMMU_CBn_TLBIVAAL_LO			(0x628)
#define SMMU_CBn_TLBIVAAL_HI			(0x62c)
#define SMMU_CBn_TLBIVAAL_AARCH_32		(0x628)
#define SMMU_CBn_TLBIIPAS2_LO			(0x630)
#define SMMU_CBn_TLBIIPAS2_HI			(0x634)
#define SMMU_CBn_TLBIIPAS2L_LO			(0x638)
#define SMMU_CBn_TLBIIPAS2L_HI			(0x63c)
#define SMMU_CBn_TLBSYNC			(0x7f0)
#define SMMU_CBn_TLBSTATUS			(0x7f4)
#define SMMU_CBn_ATSR				(0x800)
#define SMMU_CBn_PMEVCNTR0			(0xe00)
#define SMMU_CBn_PMEVCNTR1			(0xe04)
#define SMMU_CBn_PMEVCNTR2			(0xe08)
#define SMMU_CBn_PMEVCNTR3			(0xe0c)
#define SMMU_CBn_PMEVTYPER0			(0xe80)
#define SMMU_CBn_PMEVTYPER1			(0xe84)
#define SMMU_CBn_PMEVTYPER2			(0xe88)
#define SMMU_CBn_PMEVTYPER3			(0xe8c)
#define SMMU_CBn_PMCFGR				(0xf00)
#define SMMU_CBn_PMCR				(0xf04)
#define SMMU_CBn_PMCEID				(0xf20)
#define SMMU_CBn_PMCNTENSE			(0xf40)
#define SMMU_CBn_PMCNTENCLR			(0xf44)
#define SMMU_CBn_PMCNTENSET			(0xf48)
#define SMMU_CBn_PMINTENCLR			(0xf4c)
#define SMMU_CBn_PMOVSCLR			(0xf50)
#define SMMU_CBn_PMOVSSET			(0xf58)
#define SMMU_CBn_PMAUTHSTATUS			(0xfb8)
#define SMMU_GNSR0_CR0				(0x0)
#define SMMU_GNSR0_CR2				(0x8)
#define SMMU_GNSR0_ACR				(0x10)
#define SMMU_GNSR0_IDR0				(0x20)
#define SMMU_GNSR0_IDR1				(0x24)
#define SMMU_GNSR0_IDR2				(0x28)
#define SMMU_GNSR0_IDR7				(0x3c)
#define SMMU_GNSR0_GFAR_LO			(0x40)
#define SMMU_GNSR0_GFAR_HI			(0x44)
#define SMMU_GNSR0_GFSR				(0x48)
#define SMMU_GNSR0_GFSRRESTORE			(0x4c)
#define SMMU_GNSR0_GFSYNR0			(0x50)
#define SMMU_GNSR0_GFSYNR1			(0x54)
#define SMMU_GNSR0_GFSYNR1_v2			(0x54)
#define SMMU_GNSR0_TLBIVMID			(0x64)
#define SMMU_GNSR0_TLBIALLNSNH			(0x68)
#define SMMU_GNSR0_TLBIALLH			(0x6c)
#define SMMU_GNSR0_TLBGSYNC			(0x70)
#define SMMU_GNSR0_TLBGSTATUS			(0x74)
#define SMMU_GNSR0_TLBIVAH_LO			(0x78)
#define SMMU_GNSR0_TLBIVALH64_LO		(0xb0)
#define SMMU_GNSR0_TLBIVALH64_HI		(0xb4)
#define SMMU_GNSR0_TLBIVMIDS1			(0xb8)
#define SMMU_GNSR0_TLBIVAH64_LO			(0xc0)
#define SMMU_GNSR0_TLBIVAH64_HI			(0xc4)
#define SMMU_GNSR0_SMR0				(0x800)
#define SMMU_GNSR0_SMRn				(0x800)
#define SMMU_GNSR0_SMR1				(0x804)
#define SMMU_GNSR0_SMR2				(0x808)
#define SMMU_GNSR0_SMR3				(0x80c)
#define SMMU_GNSR0_SMR4				(0x810)
#define SMMU_GNSR0_SMR5				(0x814)
#define SMMU_GNSR0_SMR6				(0x818)
#define SMMU_GNSR0_SMR7				(0x81c)
#define SMMU_GNSR0_SMR8				(0x820)
#define SMMU_GNSR0_SMR9				(0x824)
#define SMMU_GNSR0_SMR10			(0x828)
#define SMMU_GNSR0_SMR11			(0x82c)
#define SMMU_GNSR0_SMR12			(0x830)
#define SMMU_GNSR0_SMR13			(0x834)
#define SMMU_GNSR0_SMR14			(0x838)
#define SMMU_GNSR0_SMR15			(0x83c)
#define SMMU_GNSR0_SMR16			(0x840)
#define SMMU_GNSR0_SMR17			(0x844)
#define SMMU_GNSR0_SMR18			(0x848)
#define SMMU_GNSR0_SMR19			(0x84c)
#define SMMU_GNSR0_SMR20			(0x850)
#define SMMU_GNSR0_SMR21			(0x854)
#define SMMU_GNSR0_SMR22			(0x858)
#define SMMU_GNSR0_SMR23			(0x85c)
#define SMMU_GNSR0_SMR24			(0x860)
#define SMMU_GNSR0_SMR25			(0x864)
#define SMMU_GNSR0_SMR26			(0x868)
#define SMMU_GNSR0_SMR27			(0x86c)
#define SMMU_GNSR0_SMR28			(0x870)
#define SMMU_GNSR0_SMR29			(0x874)
#define SMMU_GNSR0_SMR30			(0x878)
#define SMMU_GNSR0_SMR31			(0x87c)
#define SMMU_GNSR0_SMR32			(0x880)
#define SMMU_GNSR0_SMR33			(0x884)
#define SMMU_GNSR0_SMR34			(0x888)
#define SMMU_GNSR0_SMR35			(0x88c)
#define SMMU_GNSR0_SMR36			(0x890)
#define SMMU_GNSR0_SMR37			(0x894)
#define SMMU_GNSR0_SMR38			(0x898)
#define SMMU_GNSR0_SMR39			(0x89c)
#define SMMU_GNSR0_SMR40			(0x8a0)
#define SMMU_GNSR0_SMR41			(0x8a4)
#define SMMU_GNSR0_SMR42			(0x8a8)
#define SMMU_GNSR0_SMR43			(0x8ac)
#define SMMU_GNSR0_SMR44			(0x8b0)
#define SMMU_GNSR0_SMR45			(0x8b4)
#define SMMU_GNSR0_SMR46			(0x8b8)
#define SMMU_GNSR0_SMR47			(0x8bc)
#define SMMU_GNSR0_SMR48			(0x8c0)
#define SMMU_GNSR0_SMR49			(0x8c4)
#define SMMU_GNSR0_SMR50			(0x8c8)
#define SMMU_GNSR0_SMR51			(0x8cc)
#define SMMU_GNSR0_SMR52			(0x8d0)
#define SMMU_GNSR0_SMR53			(0x8d4)
#define SMMU_GNSR0_SMR54			(0x8d8)
#define SMMU_GNSR0_SMR55			(0x8dc)
#define SMMU_GNSR0_SMR56			(0x8e0)
#define SMMU_GNSR0_SMR57			(0x8e4)
#define SMMU_GNSR0_SMR58			(0x8e8)
#define SMMU_GNSR0_SMR59			(0x8ec)
#define SMMU_GNSR0_SMR60			(0x8f0)
#define SMMU_GNSR0_SMR61			(0x8f4)
#define SMMU_GNSR0_SMR62			(0x8f8)
#define SMMU_GNSR0_SMR63			(0x8fc)
#define SMMU_GNSR0_SMR64			(0x900)
#define SMMU_GNSR0_SMR65			(0x904)
#define SMMU_GNSR0_SMR66			(0x908)
#define SMMU_GNSR0_SMR67			(0x90c)
#define SMMU_GNSR0_SMR68			(0x910)
#define SMMU_GNSR0_SMR69			(0x914)
#define SMMU_GNSR0_SMR70			(0x918)
#define SMMU_GNSR0_SMR71			(0x91c)
#define SMMU_GNSR0_SMR72			(0x920)
#define SMMU_GNSR0_SMR73			(0x924)
#define SMMU_GNSR0_SMR74			(0x928)
#define SMMU_GNSR0_SMR75			(0x92c)
#define SMMU_GNSR0_SMR76			(0x930)
#define SMMU_GNSR0_SMR77			(0x934)
#define SMMU_GNSR0_SMR78			(0x938)
#define SMMU_GNSR0_SMR79			(0x93c)
#define SMMU_GNSR0_SMR80			(0x940)
#define SMMU_GNSR0_SMR81			(0x944)
#define SMMU_GNSR0_SMR82			(0x948)
#define SMMU_GNSR0_SMR83			(0x94c)
#define SMMU_GNSR0_SMR84			(0x950)
#define SMMU_GNSR0_SMR85			(0x954)
#define SMMU_GNSR0_SMR86			(0x958)
#define SMMU_GNSR0_SMR87			(0x95c)
#define SMMU_GNSR0_SMR88			(0x960)
#define SMMU_GNSR0_SMR89			(0x964)
#define SMMU_GNSR0_SMR90			(0x968)
#define SMMU_GNSR0_SMR91			(0x96c)
#define SMMU_GNSR0_SMR92			(0x970)
#define SMMU_GNSR0_SMR93			(0x974)
#define SMMU_GNSR0_SMR94			(0x978)
#define SMMU_GNSR0_SMR95			(0x97c)
#define SMMU_GNSR0_SMR96			(0x980)
#define SMMU_GNSR0_SMR97			(0x984)
#define SMMU_GNSR0_SMR98			(0x988)
#define SMMU_GNSR0_SMR99			(0x98c)
#define SMMU_GNSR0_SMR100			(0x990)
#define SMMU_GNSR0_SMR101			(0x994)
#define SMMU_GNSR0_SMR102			(0x998)
#define SMMU_GNSR0_SMR103			(0x99c)
#define SMMU_GNSR0_SMR104			(0x9a0)
#define SMMU_GNSR0_SMR105			(0x9a4)
#define SMMU_GNSR0_SMR106			(0x9a8)
#define SMMU_GNSR0_SMR107			(0x9ac)
#define SMMU_GNSR0_SMR108			(0x9b0)
#define SMMU_GNSR0_SMR109			(0x9b4)
#define SMMU_GNSR0_SMR110			(0x9b8)
#define SMMU_GNSR0_SMR111			(0x9bc)
#define SMMU_GNSR0_SMR112			(0x9c0)
#define SMMU_GNSR0_SMR113			(0x9c4)
#define SMMU_GNSR0_SMR114			(0x9c8)
#define SMMU_GNSR0_SMR115			(0x9cc)
#define SMMU_GNSR0_SMR116			(0x9d0)
#define SMMU_GNSR0_SMR117			(0x9d4)
#define SMMU_GNSR0_SMR118			(0x9d8)
#define SMMU_GNSR0_SMR119			(0x9dc)
#define SMMU_GNSR0_SMR120			(0x9e0)
#define SMMU_GNSR0_SMR121			(0x9e4)
#define SMMU_GNSR0_SMR122			(0x9e8)
#define SMMU_GNSR0_SMR123			(0x9ec)
#define SMMU_GNSR0_SMR124			(0x9f0)
#define SMMU_GNSR0_SMR125			(0x9f4)
#define SMMU_GNSR0_SMR126			(0x9f8)
#define SMMU_GNSR0_SMR127			(0x9fc)
#define SMMU_GNSR0_S2CR0			(0xc00)
#define SMMU_GNSR0_S2CRn			(0xc00)
#define SMMU_GNSR0_S2CRn			(0xc00)
#define SMMU_GNSR0_S2CR1			(0xc04)
#define SMMU_GNSR0_S2CR2			(0xc08)
#define SMMU_GNSR0_S2CR3			(0xc0c)
#define SMMU_GNSR0_S2CR4			(0xc10)
#define SMMU_GNSR0_S2CR5			(0xc14)
#define SMMU_GNSR0_S2CR6			(0xc18)
#define SMMU_GNSR0_S2CR7			(0xc1c)
#define SMMU_GNSR0_S2CR8			(0xc20)
#define SMMU_GNSR0_S2CR9			(0xc24)
#define SMMU_GNSR0_S2CR10			(0xc28)
#define SMMU_GNSR0_S2CR11			(0xc2c)
#define SMMU_GNSR0_S2CR12			(0xc30)
#define SMMU_GNSR0_S2CR13			(0xc34)
#define SMMU_GNSR0_S2CR14			(0xc38)
#define SMMU_GNSR0_S2CR15			(0xc3c)
#define SMMU_GNSR0_S2CR16			(0xc40)
#define SMMU_GNSR0_S2CR17			(0xc44)
#define SMMU_GNSR0_S2CR18			(0xc48)
#define SMMU_GNSR0_S2CR19			(0xc4c)
#define SMMU_GNSR0_S2CR20			(0xc50)
#define SMMU_GNSR0_S2CR21			(0xc54)
#define SMMU_GNSR0_S2CR22			(0xc58)
#define SMMU_GNSR0_S2CR23			(0xc5c)
#define SMMU_GNSR0_S2CR24			(0xc60)
#define SMMU_GNSR0_S2CR25			(0xc64)
#define SMMU_GNSR0_S2CR26			(0xc68)
#define SMMU_GNSR0_S2CR27			(0xc6c)
#define SMMU_GNSR0_S2CR28			(0xc70)
#define SMMU_GNSR0_S2CR29			(0xc74)
#define SMMU_GNSR0_S2CR30			(0xc78)
#define SMMU_GNSR0_S2CR31			(0xc7c)
#define SMMU_GNSR0_S2CR32			(0xc80)
#define SMMU_GNSR0_S2CR33			(0xc84)
#define SMMU_GNSR0_S2CR34			(0xc88)
#define SMMU_GNSR0_S2CR35			(0xc8c)
#define SMMU_GNSR0_S2CR36			(0xc90)
#define SMMU_GNSR0_S2CR37			(0xc94)
#define SMMU_GNSR0_S2CR38			(0xc98)
#define SMMU_GNSR0_S2CR39			(0xc9c)
#define SMMU_GNSR0_S2CR40			(0xca0)
#define SMMU_GNSR0_S2CR41			(0xca4)
#define SMMU_GNSR0_S2CR42			(0xca8)
#define SMMU_GNSR0_S2CR43			(0xcac)
#define SMMU_GNSR0_S2CR44			(0xcb0)
#define SMMU_GNSR0_S2CR45			(0xcb4)
#define SMMU_GNSR0_S2CR46			(0xcb8)
#define SMMU_GNSR0_S2CR47			(0xcbc)
#define SMMU_GNSR0_S2CR48			(0xcc0)
#define SMMU_GNSR0_S2CR49			(0xcc4)
#define SMMU_GNSR0_S2CR50			(0xcc8)
#define SMMU_GNSR0_S2CR51			(0xccc)
#define SMMU_GNSR0_S2CR52			(0xcd0)
#define SMMU_GNSR0_S2CR53			(0xcd4)
#define SMMU_GNSR0_S2CR54			(0xcd8)
#define SMMU_GNSR0_S2CR55			(0xcdc)
#define SMMU_GNSR0_S2CR56			(0xce0)
#define SMMU_GNSR0_S2CR57			(0xce4)
#define SMMU_GNSR0_S2CR58			(0xce8)
#define SMMU_GNSR0_S2CR59			(0xcec)
#define SMMU_GNSR0_S2CR60			(0xcf0)
#define SMMU_GNSR0_S2CR61			(0xcf4)
#define SMMU_GNSR0_S2CR62			(0xcf8)
#define SMMU_GNSR0_S2CR63			(0xcfc)
#define SMMU_GNSR0_S2CR64			(0xd00)
#define SMMU_GNSR0_S2CR65			(0xd04)
#define SMMU_GNSR0_S2CR66			(0xd08)
#define SMMU_GNSR0_S2CR67			(0xd0c)
#define SMMU_GNSR0_S2CR68			(0xd10)
#define SMMU_GNSR0_S2CR69			(0xd14)
#define SMMU_GNSR0_S2CR70			(0xd18)
#define SMMU_GNSR0_S2CR71			(0xd1c)
#define SMMU_GNSR0_S2CR72			(0xd20)
#define SMMU_GNSR0_S2CR73			(0xd24)
#define SMMU_GNSR0_S2CR74			(0xd28)
#define SMMU_GNSR0_S2CR75			(0xd2c)
#define SMMU_GNSR0_S2CR76			(0xd30)
#define SMMU_GNSR0_S2CR77			(0xd34)
#define SMMU_GNSR0_S2CR78			(0xd38)
#define SMMU_GNSR0_S2CR79			(0xd3c)
#define SMMU_GNSR0_S2CR80			(0xd40)
#define SMMU_GNSR0_S2CR81			(0xd44)
#define SMMU_GNSR0_S2CR82			(0xd48)
#define SMMU_GNSR0_S2CR83			(0xd4c)
#define SMMU_GNSR0_S2CR84			(0xd50)
#define SMMU_GNSR0_S2CR85			(0xd54)
#define SMMU_GNSR0_S2CR86			(0xd58)
#define SMMU_GNSR0_S2CR87			(0xd5c)
#define SMMU_GNSR0_S2CR88			(0xd60)
#define SMMU_GNSR0_S2CR89			(0xd64)
#define SMMU_GNSR0_S2CR90			(0xd68)
#define SMMU_GNSR0_S2CR91			(0xd6c)
#define SMMU_GNSR0_S2CR92			(0xd70)
#define SMMU_GNSR0_S2CR93			(0xd74)
#define SMMU_GNSR0_S2CR94			(0xd78)
#define SMMU_GNSR0_S2CR95			(0xd7c)
#define SMMU_GNSR0_S2CR96			(0xd80)
#define SMMU_GNSR0_S2CR97			(0xd84)
#define SMMU_GNSR0_S2CR98			(0xd88)
#define SMMU_GNSR0_S2CR99			(0xd8c)
#define SMMU_GNSR0_S2CR100			(0xd90)
#define SMMU_GNSR0_S2CR101			(0xd94)
#define SMMU_GNSR0_S2CR102			(0xd98)
#define SMMU_GNSR0_S2CR103			(0xd9c)
#define SMMU_GNSR0_S2CR104			(0xda0)
#define SMMU_GNSR0_S2CR105			(0xda4)
#define SMMU_GNSR0_S2CR106			(0xda8)
#define SMMU_GNSR0_S2CR107			(0xdac)
#define SMMU_GNSR0_S2CR108			(0xdb0)
#define SMMU_GNSR0_S2CR109			(0xdb4)
#define SMMU_GNSR0_S2CR110			(0xdb8)
#define SMMU_GNSR0_S2CR111			(0xdbc)
#define SMMU_GNSR0_S2CR112			(0xdc0)
#define SMMU_GNSR0_S2CR113			(0xdc4)
#define SMMU_GNSR0_S2CR114			(0xdc8)
#define SMMU_GNSR0_S2CR115			(0xdcc)
#define SMMU_GNSR0_S2CR116			(0xdd0)
#define SMMU_GNSR0_S2CR117			(0xdd4)
#define SMMU_GNSR0_S2CR118			(0xdd8)
#define SMMU_GNSR0_S2CR119			(0xddc)
#define SMMU_GNSR0_S2CR120			(0xde0)
#define SMMU_GNSR0_S2CR121			(0xde4)
#define SMMU_GNSR0_S2CR122			(0xde8)
#define SMMU_GNSR0_S2CR123			(0xdec)
#define SMMU_GNSR0_S2CR124			(0xdf0)
#define SMMU_GNSR0_S2CR125			(0xdf4)
#define SMMU_GNSR0_S2CR126			(0xdf8)
#define SMMU_GNSR0_S2CR127			(0xdfc)
#define SMMU_GNSR0_PIDR0			(0xfe0)
#define SMMU_GNSR0_PIDR1			(0xfe4)
#define SMMU_GNSR0_PIDR2			(0xfe8)
#define SMMU_GNSR0_PIDR3			(0xfec)
#define SMMU_GNSR0_PIDR4			(0xfd0)
#define SMMU_GNSR0_PIDR5			(0xfd4)
#define SMMU_GNSR0_PIDR6			(0xfd8)
#define SMMU_GNSR0_PIDR7			(0xfdc)
#define SMMU_GNSR0_CIDR0			(0xff0)
#define SMMU_GNSR0_CIDR1			(0xff4)
#define SMMU_GNSR0_CIDR2			(0xff8)
#define SMMU_GNSR0_CIDR3			(0xffc)
#define SMMU_GNSR1_CBAR0			(0x0)
#define SMMU_GNSR1_CBARn			(0x0)
#define SMMU_GNSR1_CBFRSYNRA0			(0x400)
#define SMMU_GNSR1_CBA2R0			(0x800)
#define SMMU_GNSR1_CBAR1			(0x4)
#define SMMU_GNSR1_CBFRSYNRA1			(0x404)
#define SMMU_GNSR1_CBA2R1			(0x804)
#define SMMU_GNSR1_CBAR2			(0x8)
#define SMMU_GNSR1_CBFRSYNRA2			(0x408)
#define SMMU_GNSR1_CBA2R2			(0x808)
#define SMMU_GNSR1_CBAR3			(0xc)
#define SMMU_GNSR1_CBFRSYNRA3			(0x40c)
#define SMMU_GNSR1_CBA2R3			(0x80c)
#define SMMU_GNSR1_CBAR4			(0x10)
#define SMMU_GNSR1_CBFRSYNRA4			(0x410)
#define SMMU_GNSR1_CBA2R4			(0x810)
#define SMMU_GNSR1_CBAR5			(0x14)
#define SMMU_GNSR1_CBFRSYNRA5			(0x414)
#define SMMU_GNSR1_CBA2R5			(0x814)
#define SMMU_GNSR1_CBAR6			(0x18)
#define SMMU_GNSR1_CBFRSYNRA6			(0x418)
#define SMMU_GNSR1_CBA2R6			(0x818)
#define SMMU_GNSR1_CBAR7			(0x1c)
#define SMMU_GNSR1_CBFRSYNRA7			(0x41c)
#define SMMU_GNSR1_CBA2R7			(0x81c)
#define SMMU_GNSR1_CBAR8			(0x20)
#define SMMU_GNSR1_CBFRSYNRA8			(0x420)
#define SMMU_GNSR1_CBA2R8			(0x820)
#define SMMU_GNSR1_CBAR9			(0x24)
#define SMMU_GNSR1_CBFRSYNRA9			(0x424)
#define SMMU_GNSR1_CBA2R9			(0x824)
#define SMMU_GNSR1_CBAR10			(0x28)
#define SMMU_GNSR1_CBFRSYNRA10			(0x428)
#define SMMU_GNSR1_CBA2R10			(0x828)
#define SMMU_GNSR1_CBAR11			(0x2c)
#define SMMU_GNSR1_CBFRSYNRA11			(0x42c)
#define SMMU_GNSR1_CBA2R11			(0x82c)
#define SMMU_GNSR1_CBAR12			(0x30)
#define SMMU_GNSR1_CBFRSYNRA12			(0x430)
#define SMMU_GNSR1_CBA2R12			(0x830)
#define SMMU_GNSR1_CBAR13			(0x34)
#define SMMU_GNSR1_CBFRSYNRA13			(0x434)
#define SMMU_GNSR1_CBA2R13			(0x834)
#define SMMU_GNSR1_CBAR14			(0x38)
#define SMMU_GNSR1_CBFRSYNRA14			(0x438)
#define SMMU_GNSR1_CBA2R14			(0x838)
#define SMMU_GNSR1_CBAR15			(0x3c)
#define SMMU_GNSR1_CBFRSYNRA15			(0x43c)
#define SMMU_GNSR1_CBA2R15			(0x83c)
#define SMMU_GNSR1_CBAR16			(0x40)
#define SMMU_GNSR1_CBFRSYNRA16			(0x440)
#define SMMU_GNSR1_CBA2R16			(0x840)
#define SMMU_GNSR1_CBAR17			(0x44)
#define SMMU_GNSR1_CBFRSYNRA17			(0x444)
#define SMMU_GNSR1_CBA2R17			(0x844)
#define SMMU_GNSR1_CBAR18			(0x48)
#define SMMU_GNSR1_CBFRSYNRA18			(0x448)
#define SMMU_GNSR1_CBA2R18			(0x848)
#define SMMU_GNSR1_CBAR19			(0x4c)
#define SMMU_GNSR1_CBFRSYNRA19			(0x44c)
#define SMMU_GNSR1_CBA2R19			(0x84c)
#define SMMU_GNSR1_CBAR20			(0x50)
#define SMMU_GNSR1_CBFRSYNRA20			(0x450)
#define SMMU_GNSR1_CBA2R20			(0x850)
#define SMMU_GNSR1_CBAR21			(0x54)
#define SMMU_GNSR1_CBFRSYNRA21			(0x454)
#define SMMU_GNSR1_CBA2R21			(0x854)
#define SMMU_GNSR1_CBAR22			(0x58)
#define SMMU_GNSR1_CBFRSYNRA22			(0x458)
#define SMMU_GNSR1_CBA2R22			(0x858)
#define SMMU_GNSR1_CBAR23			(0x5c)
#define SMMU_GNSR1_CBFRSYNRA23			(0x45c)
#define SMMU_GNSR1_CBA2R23			(0x85c)
#define SMMU_GNSR1_CBAR24			(0x60)
#define SMMU_GNSR1_CBFRSYNRA24			(0x460)
#define SMMU_GNSR1_CBA2R24			(0x860)
#define SMMU_GNSR1_CBAR25			(0x64)
#define SMMU_GNSR1_CBFRSYNRA25			(0x464)
#define SMMU_GNSR1_CBA2R25			(0x864)
#define SMMU_GNSR1_CBAR26			(0x68)
#define SMMU_GNSR1_CBFRSYNRA26			(0x468)
#define SMMU_GNSR1_CBA2R26			(0x868)
#define SMMU_GNSR1_CBAR27			(0x6c)
#define SMMU_GNSR1_CBFRSYNRA27			(0x46c)
#define SMMU_GNSR1_CBA2R27			(0x86c)
#define SMMU_GNSR1_CBAR28			(0x70)
#define SMMU_GNSR1_CBFRSYNRA28			(0x470)
#define SMMU_GNSR1_CBA2R28			(0x870)
#define SMMU_GNSR1_CBAR29			(0x74)
#define SMMU_GNSR1_CBFRSYNRA29			(0x474)
#define SMMU_GNSR1_CBA2R29			(0x874)
#define SMMU_GNSR1_CBAR30			(0x78)
#define SMMU_GNSR1_CBFRSYNRA30			(0x478)
#define SMMU_GNSR1_CBA2R30			(0x878)
#define SMMU_GNSR1_CBAR31			(0x7c)
#define SMMU_GNSR1_CBFRSYNRA31			(0x47c)
#define SMMU_GNSR1_CBA2R31			(0x87c)
#define SMMU_GNSR1_CBAR32			(0x80)
#define SMMU_GNSR1_CBFRSYNRA32			(0x480)
#define SMMU_GNSR1_CBA2R32			(0x880)
#define SMMU_GNSR1_CBAR33			(0x84)
#define SMMU_GNSR1_CBFRSYNRA33			(0x484)
#define SMMU_GNSR1_CBA2R33			(0x884)
#define SMMU_GNSR1_CBAR34			(0x88)
#define SMMU_GNSR1_CBFRSYNRA34			(0x488)
#define SMMU_GNSR1_CBA2R34			(0x888)
#define SMMU_GNSR1_CBAR35			(0x8c)
#define SMMU_GNSR1_CBFRSYNRA35			(0x48c)
#define SMMU_GNSR1_CBA2R35			(0x88c)
#define SMMU_GNSR1_CBAR36			(0x90)
#define SMMU_GNSR1_CBFRSYNRA36			(0x490)
#define SMMU_GNSR1_CBA2R36			(0x890)
#define SMMU_GNSR1_CBAR37			(0x94)
#define SMMU_GNSR1_CBFRSYNRA37			(0x494)
#define SMMU_GNSR1_CBA2R37			(0x894)
#define SMMU_GNSR1_CBAR38			(0x98)
#define SMMU_GNSR1_CBFRSYNRA38			(0x498)
#define SMMU_GNSR1_CBA2R38			(0x898)
#define SMMU_GNSR1_CBAR39			(0x9c)
#define SMMU_GNSR1_CBFRSYNRA39			(0x49c)
#define SMMU_GNSR1_CBA2R39			(0x89c)
#define SMMU_GNSR1_CBAR40			(0xa0)
#define SMMU_GNSR1_CBFRSYNRA40			(0x4a0)
#define SMMU_GNSR1_CBA2R40			(0x8a0)
#define SMMU_GNSR1_CBAR41			(0xa4)
#define SMMU_GNSR1_CBFRSYNRA41			(0x4a4)
#define SMMU_GNSR1_CBA2R41			(0x8a4)
#define SMMU_GNSR1_CBAR42			(0xa8)
#define SMMU_GNSR1_CBFRSYNRA42			(0x4a8)
#define SMMU_GNSR1_CBA2R42			(0x8a8)
#define SMMU_GNSR1_CBAR43			(0xac)
#define SMMU_GNSR1_CBFRSYNRA43			(0x4ac)
#define SMMU_GNSR1_CBA2R43			(0x8ac)
#define SMMU_GNSR1_CBAR44			(0xb0)
#define SMMU_GNSR1_CBFRSYNRA44			(0x4b0)
#define SMMU_GNSR1_CBA2R44			(0x8b0)
#define SMMU_GNSR1_CBAR45			(0xb4)
#define SMMU_GNSR1_CBFRSYNRA45			(0x4b4)
#define SMMU_GNSR1_CBA2R45			(0x8b4)
#define SMMU_GNSR1_CBAR46			(0xb8)
#define SMMU_GNSR1_CBFRSYNRA46			(0x4b8)
#define SMMU_GNSR1_CBA2R46			(0x8b8)
#define SMMU_GNSR1_CBAR47			(0xbc)
#define SMMU_GNSR1_CBFRSYNRA47			(0x4bc)
#define SMMU_GNSR1_CBA2R47			(0x8bc)
#define SMMU_GNSR1_CBAR48			(0xc0)
#define SMMU_GNSR1_CBFRSYNRA48			(0x4c0)
#define SMMU_GNSR1_CBA2R48			(0x8c0)
#define SMMU_GNSR1_CBAR49			(0xc4)
#define SMMU_GNSR1_CBFRSYNRA49			(0x4c4)
#define SMMU_GNSR1_CBA2R49			(0x8c4)
#define SMMU_GNSR1_CBAR50			(0xc8)
#define SMMU_GNSR1_CBFRSYNRA50			(0x4c8)
#define SMMU_GNSR1_CBA2R50			(0x8c8)
#define SMMU_GNSR1_CBAR51			(0xcc)
#define SMMU_GNSR1_CBFRSYNRA51			(0x4cc)
#define SMMU_GNSR1_CBA2R51			(0x8cc)
#define SMMU_GNSR1_CBAR52			(0xd0)
#define SMMU_GNSR1_CBFRSYNRA52			(0x4d0)
#define SMMU_GNSR1_CBA2R52			(0x8d0)
#define SMMU_GNSR1_CBAR53			(0xd4)
#define SMMU_GNSR1_CBFRSYNRA53			(0x4d4)
#define SMMU_GNSR1_CBA2R53			(0x8d4)
#define SMMU_GNSR1_CBAR54			(0xd8)
#define SMMU_GNSR1_CBFRSYNRA54			(0x4d8)
#define SMMU_GNSR1_CBA2R54			(0x8d8)
#define SMMU_GNSR1_CBAR55			(0xdc)
#define SMMU_GNSR1_CBFRSYNRA55			(0x4dc)
#define SMMU_GNSR1_CBA2R55			(0x8dc)
#define SMMU_GNSR1_CBAR56			(0xe0)
#define SMMU_GNSR1_CBFRSYNRA56			(0x4e0)
#define SMMU_GNSR1_CBA2R56			(0x8e0)
#define SMMU_GNSR1_CBAR57			(0xe4)
#define SMMU_GNSR1_CBFRSYNRA57			(0x4e4)
#define SMMU_GNSR1_CBA2R57			(0x8e4)
#define SMMU_GNSR1_CBAR58			(0xe8)
#define SMMU_GNSR1_CBFRSYNRA58			(0x4e8)
#define SMMU_GNSR1_CBA2R58			(0x8e8)
#define SMMU_GNSR1_CBAR59			(0xec)
#define SMMU_GNSR1_CBFRSYNRA59			(0x4ec)
#define SMMU_GNSR1_CBA2R59			(0x8ec)
#define SMMU_GNSR1_CBAR60			(0xf0)
#define SMMU_GNSR1_CBFRSYNRA60			(0x4f0)
#define SMMU_GNSR1_CBA2R60			(0x8f0)
#define SMMU_GNSR1_CBAR61			(0xf4)
#define SMMU_GNSR1_CBFRSYNRA61			(0x4f4)
#define SMMU_GNSR1_CBA2R61			(0x8f4)
#define SMMU_GNSR1_CBAR62			(0xf8)
#define SMMU_GNSR1_CBFRSYNRA62			(0x4f8)
#define SMMU_GNSR1_CBA2R62			(0x8f8)
#define SMMU_GNSR1_CBAR63			(0xfc)
#define SMMU_GNSR1_CBFRSYNRA63			(0x4fc)
#define SMMU_GNSR1_CBA2R63			(0x8fc)

/*******************************************************************************
 * SMMU Global Secure Aux. Configuration Register
 ******************************************************************************/
#define SMMU_GSR0_SECURE_ACR			0x10
#define SMMU_GNSR_ACR				(SMMU_GSR0_SECURE_ACR + 0x400)
#define SMMU_GSR0_PGSIZE_SHIFT			16
#define SMMU_GSR0_PGSIZE_4K			(0 << SMMU_GSR0_PGSIZE_SHIFT)
#define SMMU_GSR0_PGSIZE_64K			(1 << SMMU_GSR0_PGSIZE_SHIFT)
#define SMMU_ACR_CACHE_LOCK_ENABLE_BIT		(1 << 26)

/*******************************************************************************
 * SMMU Global Aux. Control Register
 ******************************************************************************/
#define SMMU_CBn_ACTLR_CPRE_BIT			(1 << 1)

/*******************************************************************************
 * SMMU configuration constants
 ******************************************************************************/
#define ID1_PAGESIZE				(1 << 31)
#define ID1_NUMPAGENDXB_SHIFT			28
#define ID1_NUMPAGENDXB_MASK			7
#define ID1_NUMS2CB_SHIFT			16
#define ID1_NUMS2CB_MASK			0xff
#define ID1_NUMCB_SHIFT				0
#define ID1_NUMCB_MASK				0xff
#define PGSHIFT					16
#define CB_SIZE					0x800000

typedef struct smmu_regs {
	uint32_t reg;
	uint32_t val;
} smmu_regs_t;

#define mc_make_sid_override_cfg(name) \
	{ \
		.reg = TEGRA_MC_STREAMID_BASE + MC_STREAMID_OVERRIDE_CFG_ ## name, \
		.val = 0x00000000, \
	}

#define mc_make_sid_security_cfg(name) \
	{ \
		.reg = TEGRA_MC_STREAMID_BASE + MC_STREAMID_OVERRIDE_TO_SECURITY_CFG(MC_STREAMID_OVERRIDE_CFG_ ## name), \
		.val = 0x00000000, \
	}

#define smmu_make_gnsr0_sec_cfg(name) \
	{ \
		.reg = TEGRA_SMMU0_BASE + SMMU_GNSR0_ ## name, \
		.val = 0x00000000, \
	}

/*
 * On ARM-SMMU, conditional offset to access secure aliases of non-secure registers
 * is 0x400. So, add it to register address
 */
#define smmu_make_gnsr0_nsec_cfg(name) \
	{ \
		.reg = TEGRA_SMMU0_BASE + 0x400 + SMMU_GNSR0_ ## name, \
		.val = 0x00000000, \
	}

#define smmu_make_gnsr0_smr_cfg(n) \
	{ \
		.reg = TEGRA_SMMU0_BASE + SMMU_GNSR0_SMR ## n, \
		.val = 0x00000000, \
	}

#define smmu_make_gnsr0_s2cr_cfg(n) \
	{ \
		.reg = TEGRA_SMMU0_BASE + SMMU_GNSR0_S2CR ## n, \
		.val = 0x00000000, \
	}

#define smmu_make_gnsr1_cbar_cfg(n) \
	{ \
		.reg = TEGRA_SMMU0_BASE + (1 << PGSHIFT) + SMMU_GNSR1_CBAR ## n, \
		.val = 0x00000000, \
	}

#define smmu_make_gnsr1_cba2r_cfg(n) \
	{ \
		.reg = TEGRA_SMMU0_BASE + (1 << PGSHIFT) + SMMU_GNSR1_CBA2R ## n, \
		.val = 0x00000000, \
	}

#define make_smmu_cb_cfg(name, n) \
	{ \
		.reg = TEGRA_SMMU0_BASE + (CB_SIZE >> 1) + (n * (1 << PGSHIFT)) \
			+ SMMU_CBn_ ## name, \
		.val = 0x00000000, \
	}

#define smmu_make_smrg_group(n)	\
	smmu_make_gnsr0_smr_cfg(n),	\
	smmu_make_gnsr0_s2cr_cfg(n),	\
	smmu_make_gnsr1_cbar_cfg(n),	\
	smmu_make_gnsr1_cba2r_cfg(n)	/* don't put "," here. */

#define smmu_make_cb_group(n)		\
	make_smmu_cb_cfg(SCTLR, n),	\
	make_smmu_cb_cfg(TCR2, n),	\
	make_smmu_cb_cfg(TTBR0_LO, n),	\
	make_smmu_cb_cfg(TTBR0_HI, n),	\
	make_smmu_cb_cfg(TCR, n),	\
	make_smmu_cb_cfg(PRRR_MAIR0, n),\
	make_smmu_cb_cfg(FSR, n),	\
	make_smmu_cb_cfg(FAR_LO, n),	\
	make_smmu_cb_cfg(FAR_HI, n),	\
	make_smmu_cb_cfg(FSYNR0, n)	/* don't put "," here. */

#define smmu_bypass_cfg \
	{ \
		.reg = TEGRA_MC_BASE + MC_SMMU_BYPASS_CONFIG, \
		.val = 0x00000000, \
	}

#define _START_OF_TABLE_ \
	{ \
		.reg = 0xCAFE05C7, \
		.val = 0x00000000, \
	}

#define _END_OF_TABLE_ \
	{ \
		.reg = 0xFFFFFFFF, \
		.val = 0xFFFFFFFF, \
	}


void tegra_smmu_init(void);
void tegra_smmu_save_context(uint64_t smmu_ctx_addr);
smmu_regs_t *plat_get_smmu_ctx(void);

#endif /*__SMMU_H */

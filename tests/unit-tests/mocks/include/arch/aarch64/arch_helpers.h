/*
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_HELPERS_H
#define ARCH_HELPERS_H

#include <cdefs.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <arch.h>

typedef unsigned long u_register_t;


/* define the structure that contains all the mock register values */
struct mock_sys_reg {
	u_register_t accdata_el1;
	u_register_t actlr_el1;
	u_register_t actlr_el2;
	u_register_t actlr_el3;
	u_register_t afsr0_el1;
	u_register_t afsr0_el2;
	u_register_t afsr0_el3;
	u_register_t afsr1_el1;
	u_register_t afsr1_el2;
	u_register_t afsr1_el3;
	u_register_t aidr_el1;
	u_register_t amair_el1;
	u_register_t amair_el2;
	u_register_t amair_el3;
	u_register_t amcfgr_el0;
	u_register_t amcg1idr_el0;
	u_register_t amcgcr_el0;
	u_register_t amcntenclr0_el0;
	u_register_t amcntenclr1_el0;
	u_register_t amcntenset0_el0;
	u_register_t amcntenset1_el0;
	u_register_t amcr_el0;
	u_register_t amevcntr01_el0;
	u_register_t amevcntr02_el0;
	u_register_t amevcntr03_el0;
	u_register_t amevcntr04_el0;
	u_register_t amevcntr05_el0;
	u_register_t amevcntr06_el0;
	u_register_t amevcntr07_el0;
	u_register_t amevcntr08_el0;
	u_register_t amevcntr09_el0;
	u_register_t amevcntr010_el0;
	u_register_t amevcntr011_el0;
	u_register_t amevcntr012_el0;
	u_register_t amevcntr013_el0;
	u_register_t amevcntr014_el0;
	u_register_t amevcntr015_el0;
	u_register_t amevcntr11_el0;
	u_register_t amevcntr12_el0;
	u_register_t amevcntr13_el0;
	u_register_t amevcntr14_el0;
	u_register_t amevcntr15_el0;
	u_register_t amevcntr16_el0;
	u_register_t amevcntr17_el0;
	u_register_t amevcntr18_el0;
	u_register_t amevcntr19_el0;
	u_register_t amevcntr110_el0;
	u_register_t amevcntr111_el0;
	u_register_t amevcntr112_el0;
	u_register_t amevcntr113_el0;
	u_register_t amevcntr114_el0;
	u_register_t amevcntr115_el0;
	u_register_t amevcntvoff01_el2;
	u_register_t amevcntvoff02_el2;
	u_register_t amevcntvoff03_el2;
	u_register_t amevcntvoff04_el2;
	u_register_t amevcntvoff05_el2;
	u_register_t amevcntvoff06_el2;
	u_register_t amevcntvoff07_el2;
	u_register_t amevcntvoff08_el2;
	u_register_t amevcntvoff09_el2;
	u_register_t amevcntvoff010_el2;
	u_register_t amevcntvoff011_el2;
	u_register_t amevcntvoff012_el2;
	u_register_t amevcntvoff013_el2;
	u_register_t amevcntvoff014_el2;
	u_register_t amevcntvoff015_el2;
	u_register_t amevcntvoff11_el2;
	u_register_t amevcntvoff12_el2;
	u_register_t amevcntvoff13_el2;
	u_register_t amevcntvoff14_el2;
	u_register_t amevcntvoff15_el2;
	u_register_t amevcntvoff16_el2;
	u_register_t amevcntvoff17_el2;
	u_register_t amevcntvoff18_el2;
	u_register_t amevcntvoff19_el2;
	u_register_t amevcntvoff110_el2;
	u_register_t amevcntvoff111_el2;
	u_register_t amevcntvoff112_el2;
	u_register_t amevcntvoff113_el2;
	u_register_t amevcntvoff114_el2;
	u_register_t amevcntvoff115_el2;
	u_register_t amevtyper01_el0;
	u_register_t amevtyper02_el0;
	u_register_t amevtyper03_el0;
	u_register_t amevtyper04_el0;
	u_register_t amevtyper05_el0;
	u_register_t amevtyper06_el0;
	u_register_t amevtyper07_el0;
	u_register_t amevtyper08_el0;
	u_register_t amevtyper09_el0;
	u_register_t amevtyper010_el0;
	u_register_t amevtyper011_el0;
	u_register_t amevtyper012_el0;
	u_register_t amevtyper013_el0;
	u_register_t amevtyper014_el0;
	u_register_t amevtyper015_el0;
	u_register_t amevtyper11_el0;
	u_register_t amevtyper12_el0;
	u_register_t amevtyper13_el0;
	u_register_t amevtyper14_el0;
	u_register_t amevtyper15_el0;
	u_register_t amevtyper16_el0;
	u_register_t amevtyper17_el0;
	u_register_t amevtyper18_el0;
	u_register_t amevtyper19_el0;
	u_register_t amevtyper110_el0;
	u_register_t amevtyper111_el0;
	u_register_t amevtyper112_el0;
	u_register_t amevtyper113_el0;
	u_register_t amevtyper114_el0;
	u_register_t amevtyper115_el0;
	u_register_t amuserenr_el0;
	u_register_t apdakeyhi_el1;
	u_register_t apdakeylo_el1;
	u_register_t apdbkeyhi_el1;
	u_register_t apdbkeylo_el1;
	u_register_t apgakeyhi_el1;
	u_register_t apgakeylo_el1;
	u_register_t apiakeyhi_el1;
	u_register_t apiakeylo_el1;
	u_register_t apibkeyhi_el1;
	u_register_t apibkeylo_el1;
	u_register_t ccsidr2_el1;
	u_register_t ccsidr_el1;
	u_register_t clidr_el1;
	u_register_t cntfrq_el0;
	u_register_t cnthctl_el2;
	u_register_t cnthps_ctl_el2;
	u_register_t cnthps_cval_el2;
	u_register_t cnthps_tval_el2;
	u_register_t cnthp_ctl_el2;
	u_register_t cnthp_cval_el2;
	u_register_t cnthp_tval_el2;
	u_register_t cnthvs_ctl_el2;
	u_register_t cnthvs_cval_el2;
	u_register_t cnthvs_tval_el2;
	u_register_t cnthv_ctl_el2;
	u_register_t cnthv_cval_el2;
	u_register_t cnthv_tval_el2;
	u_register_t cntkctl_el1;
	u_register_t cntpctss_el0;
	u_register_t cntpct_el0;
	u_register_t cntpoff_el2;
	u_register_t cntps_ctl_el1;
	u_register_t cntps_cval_el1;
	u_register_t cntps_tval_el1;
	u_register_t cntp_ctl_el0;
	u_register_t cntp_cval_el0;
	u_register_t cntp_tval_el0;
	u_register_t cntvctss_el0;
	u_register_t cntvct_el0;
	u_register_t cntvoff_el2;
	u_register_t cntv_ctl_el0;
	u_register_t cntv_cval_el0;
	u_register_t cntv_tval_el0;
	u_register_t contextidr_el1;
	u_register_t contextidr_el2;
	u_register_t cpacr_el1;
	u_register_t cptr_el2;
	u_register_t cptr_el3;
	u_register_t csselr_el1;
	u_register_t ctr_el0;
	u_register_t CurrentEl;
	u_register_t dacr32_el2;
	u_register_t daif;
	u_register_t dbgauthstatus_el1;
	u_register_t dbgbcr1_el1;
	u_register_t dbgbcr2_el1;
	u_register_t dbgbcr3_el1;
	u_register_t dbgbcr4_el1;
	u_register_t dbgbcr5_el1;
	u_register_t dbgbcr6_el1;
	u_register_t dbgbcr7_el1;
	u_register_t dbgbcr8_el1;
	u_register_t dbgbcr9_el1;
	u_register_t dbgbcr10_el1;
	u_register_t dbgbcr11_el1;
	u_register_t dbgbcr12_el1;
	u_register_t dbgbcr13_el1;
	u_register_t dbgbcr14_el1;
	u_register_t dbgbcr15_el1;
	u_register_t dbgbvr1_el1;
	u_register_t dbgbvr2_el1;
	u_register_t dbgbvr3_el1;
	u_register_t dbgbvr4_el1;
	u_register_t dbgbvr5_el1;
	u_register_t dbgbvr6_el1;
	u_register_t dbgbvr7_el1;
	u_register_t dbgbvr8_el1;
	u_register_t dbgbvr9_el1;
	u_register_t dbgbvr10_el1;
	u_register_t dbgbvr11_el1;
	u_register_t dbgbvr12_el1;
	u_register_t dbgbvr13_el1;
	u_register_t dbgbvr14_el1;
	u_register_t dbgbvr15_el1;
	u_register_t dbgclaimclr_el1;
	u_register_t dbgclaimset_el1;
	u_register_t dbgdtrrx_el0;
	u_register_t dbgdtrtx_el0;
	u_register_t dbgdtr_el0;
	u_register_t dbgprcr_el1;
	u_register_t dbgvcr32_el2;
	u_register_t dbgwcr1_el1;
	u_register_t dbgwcr2_el1;
	u_register_t dbgwcr3_el1;
	u_register_t dbgwcr4_el1;
	u_register_t dbgwcr5_el1;
	u_register_t dbgwcr6_el1;
	u_register_t dbgwcr7_el1;
	u_register_t dbgwcr8_el1;
	u_register_t dbgwcr9_el1;
	u_register_t dbgwcr10_el1;
	u_register_t dbgwcr11_el1;
	u_register_t dbgwcr12_el1;
	u_register_t dbgwcr13_el1;
	u_register_t dbgwcr14_el1;
	u_register_t dbgwcr15_el1;
	u_register_t dbgwvr1_el1;
	u_register_t dbgwvr2_el1;
	u_register_t dbgwvr3_el1;
	u_register_t dbgwvr4_el1;
	u_register_t dbgwvr5_el1;
	u_register_t dbgwvr6_el1;
	u_register_t dbgwvr7_el1;
	u_register_t dbgwvr8_el1;
	u_register_t dbgwvr9_el1;
	u_register_t dbgwvr10_el1;
	u_register_t dbgwvr11_el1;
	u_register_t dbgwvr12_el1;
	u_register_t dbgwvr13_el1;
	u_register_t dbgwvr14_el1;
	u_register_t dbgwvr15_el1;
	u_register_t dczid_el0;
	u_register_t disr_el1;
	u_register_t dit;
	u_register_t dlr_el0;
	u_register_t dspsr_el0;
	u_register_t elr_el1;
	u_register_t elr_el2;
	u_register_t elr_el3;
	u_register_t erridr_el1;
	u_register_t errselr_el1;
	u_register_t erxaddr_el1;
	u_register_t erxctlr_el1;
	u_register_t erxfr_el1;
	u_register_t erxmisc0_el1;
	u_register_t erxmisc1_el1;
	u_register_t erxmisc2_el1;
	u_register_t erxmisc3_el1;
	u_register_t erxpfgcdn_el1;
	u_register_t erxpfgctl_el1;
	u_register_t erxpfgf_el1;
	u_register_t erxstatus_el1;
	u_register_t esr_el1;
	u_register_t esr_el2;
	u_register_t esr_el3;
	u_register_t far_el1;
	u_register_t far_el2;
	u_register_t far_el3;
	u_register_t fpcr;
	u_register_t fpexc32_el2;
	u_register_t fpsr;
	u_register_t gcr_el1;
	u_register_t gmid_el1;
	u_register_t hacr_el2;
	u_register_t hafgrtr_el2;
	u_register_t hcrx_el2;
	u_register_t hcr_el2;
	u_register_t hdfgrtr_el2;
	u_register_t hdfgwtr_el2;
	u_register_t hfgitr_el2;
	u_register_t hfgrtr_el2;
	u_register_t hfgwtr_el2;
	u_register_t hpfar_el2;
	u_register_t hstr_el2;
	u_register_t icc_ap0r0_el1;
	u_register_t icc_ap0r1_el1;
	u_register_t icc_ap0r2_el1;
	u_register_t icc_ap0r3_el1;
	u_register_t icc_ap1r0_el1;
	u_register_t icc_ap1r1_el1;
	u_register_t icc_ap1r2_el1;
	u_register_t icc_ap1r3_el1;
	u_register_t icc_asgi1r_el1;
	u_register_t icc_bpr0_el1;
	u_register_t icc_bpr1_el1;
	u_register_t icc_ctlr_el1;
	u_register_t icc_ctlr_el3;
	u_register_t icc_dir_el1;
	u_register_t icc_eoir0_el1;
	u_register_t icc_eoir1_el1;
	u_register_t icc_hppir0_el1;
	u_register_t icc_hppir1_el1;
	u_register_t icc_iar0_el1;
	u_register_t icc_iar1_el1;
	u_register_t icc_igrpen0_el1;
	u_register_t icc_igrpen1_el1;
	u_register_t icc_igrpen1_el3;
	u_register_t icc_pmr_el1;
	u_register_t icc_rpr_el1;
	u_register_t icc_sgi0r_el1;
	u_register_t icc_sgi1r_el1;
	u_register_t icc_sre_el1;
	u_register_t icc_sre_el2;
	u_register_t icc_sre_el3;
	u_register_t ich_ap0r0_el2;
	u_register_t ich_ap0r1_el2;
	u_register_t ich_ap0r2_el2;
	u_register_t ich_ap0r3_el2;
	u_register_t ich_ap1r0_el2;
	u_register_t ich_ap1r1_el2;
	u_register_t ich_ap1r2_el2;
	u_register_t ich_ap1r3_el2;
	u_register_t ich_eisr_el2;
	u_register_t ich_elrsr_el2;
	u_register_t ich_hcr_el2;
	u_register_t ich_lr1_el2;
	u_register_t ich_lr2_el2;
	u_register_t ich_lr3_el2;
	u_register_t ich_lr4_el2;
	u_register_t ich_lr5_el2;
	u_register_t ich_lr6_el2;
	u_register_t ich_lr7_el2;
	u_register_t ich_lr8_el2;
	u_register_t ich_lr9_el2;
	u_register_t ich_lr10_el2;
	u_register_t ich_lr11_el2;
	u_register_t ich_lr12_el2;
	u_register_t ich_lr13_el2;
	u_register_t ich_lr14_el2;
	u_register_t ich_lr15_el2;
	u_register_t ich_misr_el2;
	u_register_t ich_vmcr_el2;
	u_register_t ich_vtr_el2;
	u_register_t icv_ap0r0_el1;
	u_register_t icv_ap0r1_el1;
	u_register_t icv_ap0r2_el1;
	u_register_t icv_ap0r3_el1;
	u_register_t icv_ap1r0_el1;
	u_register_t icv_ap1r1_el1;
	u_register_t icv_ap1r2_el1;
	u_register_t icv_ap1r3_el1;
	u_register_t icv_bpr0_el1;
	u_register_t icv_bpr1_el1;
	u_register_t icv_ctlr_el1;
	u_register_t icv_dir_el1;
	u_register_t icv_eoir0_el1;
	u_register_t icv_eoir1_el1;
	u_register_t icv_hppir0_el1;
	u_register_t icv_hppir1_el1;
	u_register_t icv_iar0_el1;
	u_register_t icv_iar1_el1;
	u_register_t icv_igrpen0_el1;
	u_register_t icv_igrpen1_el1;
	u_register_t icv_pmr_el1;
	u_register_t icv_rpr_el1;
	u_register_t id_aa64afr0_el1;
	u_register_t id_aa64afr1_el1;
	u_register_t id_aa64dfr0_el1;
	u_register_t id_aa64dfr1_el1;
	u_register_t id_aa64isar0_el1;
	u_register_t id_aa64isar1_el1;
	u_register_t id_aa64isar2_el1;
	u_register_t id_aa64mmfr0_el1;
	u_register_t id_aa64mmfr1_el1;
	u_register_t id_aa64mmfr2_el1;
	u_register_t id_aa64pfr0_el1;
	u_register_t id_aa64pfr1_el1;
	u_register_t id_aa64zfr0_el1;
	u_register_t id_afr0_el1;
	u_register_t id_dfr0_el1;
	u_register_t id_dfr1_el1;
	u_register_t id_isar0_el1;
	u_register_t id_isar1_el1;
	u_register_t id_isar2_el1;
	u_register_t id_isar3_el1;
	u_register_t id_isar4_el1;
	u_register_t id_isar5_el1;
	u_register_t id_isar6_el1;
	u_register_t id_mmfr0_el1;
	u_register_t id_mmfr1_el1;
	u_register_t id_mmfr2_el1;
	u_register_t id_mmfr3_el1;
	u_register_t id_mmfr4_el1;
	u_register_t id_mmfr5_el1;
	u_register_t id_pfr0_el1;
	u_register_t id_pfr1_el1;
	u_register_t id_pfr2_el1;
	u_register_t ifsr32_el2;
	u_register_t isr_el1;
	u_register_t lorc_el1;
	u_register_t lorea_el1;
	u_register_t lorid_el1;
	u_register_t lorn_el1;
	u_register_t lorsa_el1;
	u_register_t mair_el1;
	u_register_t mair_el2;
	u_register_t mair_el3;
	u_register_t mdccint_el1;
	u_register_t mdccsr_el0;
	u_register_t mdcr_el2;
	u_register_t mdcr_el3;
	u_register_t mdrar_el1;
	u_register_t mdscr_el1;
	u_register_t midr_el1;
	u_register_t mpam0_el1;
	u_register_t mpam1_el1;
	u_register_t mpam2_el2;
	u_register_t mpam3_el3;
	u_register_t mpamhcr_el2;
	u_register_t mpamidr_el1;
	u_register_t mpamvpm0_el2;
	u_register_t mpamvpm1_el2;
	u_register_t mpamvpm2_el2;
	u_register_t mpamvpm3_el2;
	u_register_t mpamvpm4_el2;
	u_register_t mpamvpm5_el2;
	u_register_t mpamvpm6_el2;
	u_register_t mpamvpm7_el2;
	u_register_t mpamvpmv_el2;
	u_register_t mpidr_el1;
	u_register_t mvfr0_el1;
	u_register_t mvfr1_el1;
	u_register_t mvfr2_el1;
	u_register_t nzcv;
	u_register_t osdlr_el1;
	u_register_t osdtrrx_el1;
	u_register_t osdtrtx_el1;
	u_register_t oseccr_el1;
	u_register_t oslar_el1;
	u_register_t oslsr_el1;
	u_register_t pan;
	u_register_t par_el1;
	u_register_t pmbidr_el1;
	u_register_t pmblimitr_el1;
	u_register_t pmbptr_el1;
	u_register_t pmbsr_el1;
	u_register_t pmccfiltr_el0;
	u_register_t pmccntr_el0;
	u_register_t pmceid0_el0;
	u_register_t pmceid1_el0;
	u_register_t pmcntenclr_el0;
	u_register_t pmcntenset_el0;
	u_register_t pmcr_el0;
	u_register_t pmevcntr1_el0;
	u_register_t pmevcntr2_el0;
	u_register_t pmevcntr3_el0;
	u_register_t pmevcntr4_el0;
	u_register_t pmevcntr5_el0;
	u_register_t pmevcntr6_el0;
	u_register_t pmevcntr7_el0;
	u_register_t pmevcntr8_el0;
	u_register_t pmevcntr9_el0;
	u_register_t pmevcntr10_el0;
	u_register_t pmevcntr11_el0;
	u_register_t pmevcntr12_el0;
	u_register_t pmevcntr13_el0;
	u_register_t pmevcntr14_el0;
	u_register_t pmevcntr15_el0;
	u_register_t pmevcntr16_el0;
	u_register_t pmevcntr17_el0;
	u_register_t pmevcntr18_el0;
	u_register_t pmevcntr19_el0;
	u_register_t pmevcntr20_el0;
	u_register_t pmevcntr21_el0;
	u_register_t pmevcntr22_el0;
	u_register_t pmevcntr23_el0;
	u_register_t pmevcntr24_el0;
	u_register_t pmevcntr25_el0;
	u_register_t pmevcntr26_el0;
	u_register_t pmevcntr27_el0;
	u_register_t pmevcntr28_el0;
	u_register_t pmevcntr29_el0;
	u_register_t pmevcntr30_el0;
	u_register_t pmevtyper1_el0;
	u_register_t pmevtyper2_el0;
	u_register_t pmevtyper3_el0;
	u_register_t pmevtyper4_el0;
	u_register_t pmevtyper5_el0;
	u_register_t pmevtyper6_el0;
	u_register_t pmevtyper7_el0;
	u_register_t pmevtyper8_el0;
	u_register_t pmevtyper9_el0;
	u_register_t pmevtyper10_el0;
	u_register_t pmevtyper11_el0;
	u_register_t pmevtyper12_el0;
	u_register_t pmevtyper13_el0;
	u_register_t pmevtyper14_el0;
	u_register_t pmevtyper15_el0;
	u_register_t pmevtyper16_el0;
	u_register_t pmevtyper17_el0;
	u_register_t pmevtyper18_el0;
	u_register_t pmevtyper19_el0;
	u_register_t pmevtyper20_el0;
	u_register_t pmevtyper21_el0;
	u_register_t pmevtyper22_el0;
	u_register_t pmevtyper23_el0;
	u_register_t pmevtyper24_el0;
	u_register_t pmevtyper25_el0;
	u_register_t pmevtyper26_el0;
	u_register_t pmevtyper27_el0;
	u_register_t pmevtyper28_el0;
	u_register_t pmevtyper29_el0;
	u_register_t pmevtyper30_el0;
	u_register_t pmintenclr_el1;
	u_register_t pmintenset_el1;
	u_register_t pmmir_el1;
	u_register_t pmovsclr_el0;
	u_register_t pmovsset_el0;
	u_register_t pmscr_el1;
	u_register_t pmscr_el2;
	u_register_t pmselr_el0;
	u_register_t pmsevfr_el1;
	u_register_t pmsfcr_el1;
	u_register_t pmsicr_el1;
	u_register_t pmsidr_el1;
	u_register_t pmsirr_el1;
	u_register_t pmslatfr_el1;
	u_register_t pmsnevfr_el1;
	u_register_t pmswinc_el0;
	u_register_t pmuserenr_el0;
	u_register_t pmxevcntr_el0;
	u_register_t pmxevtyper_el0;
	u_register_t revidr_el1;
	u_register_t rgsr_el1;
	u_register_t rmr_el1;
	u_register_t rmr_el2;
	u_register_t rmr_el3;
	u_register_t rndr;
	u_register_t rndrrs;
	u_register_t rvbar_el1;
	u_register_t rvbar_el2;
	u_register_t rvbar_el3;
	u_register_t scr_el3;
	u_register_t sctlr_el1;
	u_register_t sctlr_el2;
	u_register_t sctlr_el3;
	u_register_t scxtnum_el0;
	u_register_t scxtnum_el1;
	u_register_t scxtnum_el2;
	u_register_t scxtnum_el3;
	u_register_t sder32_el2;
	u_register_t sder32_el3;
	u_register_t spsel;
	u_register_t spsr_abt;
	u_register_t spsr_el1;
	u_register_t spsr_el2;
	u_register_t spsr_el3;
	u_register_t spsr_fiq;
	u_register_t spsr_irq;
	u_register_t spsr_und;
	u_register_t sp_el0;
	u_register_t sp_el1;
	u_register_t sp_el2;
	u_register_t sp_el3;
	u_register_t ssbs;
	u_register_t tco;
	u_register_t tcr_el1;
	u_register_t tcr_el2;
	u_register_t tcr_el3;
	u_register_t tfsre0_el1;
	u_register_t tfsr_el1;
	u_register_t tfsr_el2;
	u_register_t tfsr_el3;
	u_register_t tpidrro_el0;
	u_register_t tpidr_el0;
	u_register_t tpidr_el1;
	u_register_t tpidr_el2;
	u_register_t tpidr_el3;
	u_register_t trfcr_el1;
	u_register_t trfcr_el2;
	u_register_t ttbr0_el1;
	u_register_t ttbr0_el2;
	u_register_t ttbr0_el3;
	u_register_t ttbr1_el1;
	u_register_t ttbr1_el2;
	u_register_t uao;
	u_register_t vbar_el1;
	u_register_t vbar_el2;
	u_register_t vbar_el3;
	u_register_t vdisr_el2;
	u_register_t vmpidr_el2;
	u_register_t vncr_el2;
	u_register_t vpidr_el2;
	u_register_t vsesr_el2;
	u_register_t vstcr_el2;
	u_register_t vsttbr_el2;
	u_register_t vtcr_el2;
	u_register_t vttbr_el2;
	u_register_t zcr_el1;
	u_register_t zcr_el2;
	u_register_t zcr_el3;

	u_register_t clusterpwrdn_el1;

	u_register_t gptbr_el3;
	u_register_t gpccr_el3;
};

extern struct mock_sys_reg sysreg;

/* rename some registers */
#define icc_sgi1r icc_sgi1r_el1

/**********************************************************************
 * Macros which create inline functions to read or write CPU system
 * registers
 *********************************************************************/

#define _DEFINE_SYSREG_READ_FUNC(_name, _reg_name)		\
static inline u_register_t read_ ## _name(void)			\
{								\
	return sysreg._name;					\
}

#define _DEFINE_SYSREG_WRITE_FUNC(_name, _reg_name)			\
static inline void write_ ## _name(u_register_t v)			\
{									\
	sysreg._name = v;								\
}

#define SYSREG_WRITE_CONST(reg_name, v)				\
	__asm__ volatile ("msr " #reg_name ", %0" : : "i" (v))

/* Define read function for system register */
#define DEFINE_SYSREG_READ_FUNC(_name) 			\
	_DEFINE_SYSREG_READ_FUNC(_name, _name)

/* Define read & write function for system register */
#define DEFINE_SYSREG_RW_FUNCS(_name)			\
	_DEFINE_SYSREG_READ_FUNC(_name, _name)		\
	_DEFINE_SYSREG_WRITE_FUNC(_name, _name)

/* Define read & write function for renamed system register */
#define DEFINE_RENAME_SYSREG_RW_FUNCS(_name, _reg_name)	\
	_DEFINE_SYSREG_READ_FUNC(_name, _reg_name)	\
	_DEFINE_SYSREG_WRITE_FUNC(_name, _reg_name)

/* Define read function for renamed system register */
#define DEFINE_RENAME_SYSREG_READ_FUNC(_name, _reg_name)	\
	_DEFINE_SYSREG_READ_FUNC(_name, _reg_name)

/* Define write function for renamed system register */
#define DEFINE_RENAME_SYSREG_WRITE_FUNC(_name, _reg_name)	\
	_DEFINE_SYSREG_WRITE_FUNC(_name, _reg_name)

/**********************************************************************
 * Macros to create inline functions for system instructions
 *********************************************************************/

/* Define function for simple system instruction */
#define DEFINE_SYSOP_FUNC(_op)				\
static inline void _op(void)				\
{							\
	return;						\
}

/* Define function for system instruction with register parameter */
#define DEFINE_SYSOP_PARAM_FUNC(_op)			\
static inline void _op(uint64_t v)			\
{							\
	 return;					\
}

/* Define function for system instruction with type specifier */
#define DEFINE_SYSOP_TYPE_FUNC(_op, _type)		\
static inline void _op ## _type(void)			\
{							\
	return;						\
}

/* Define function for system instruction with register parameter */
#define DEFINE_SYSOP_TYPE_PARAM_FUNC(_op, _type)	\
static inline void _op ## _type(uint64_t v)		\
{							\
	return;						\
}

/*******************************************************************************
 * TLB maintenance accessor prototypes
 ******************************************************************************/

#if ERRATA_A57_813419 || ERRATA_A76_1286807
/*
 * Define function for TLBI instruction with type specifier that implements
 * the workaround for errata 813419 of Cortex-A57 or errata 1286807 of
 * Cortex-A76.
 */
#define DEFINE_TLBIOP_ERRATA_TYPE_FUNC(_type)\
static inline void tlbi ## _type(void)			\
{							\
	return;						\
}

/*
 * Define function for TLBI instruction with register parameter that implements
 * the workaround for errata 813419 of Cortex-A57 or errata 1286807 of
 * Cortex-A76.
 */
#define DEFINE_TLBIOP_ERRATA_TYPE_PARAM_FUNC(_type)	\
static inline void tlbi ## _type(uint64_t v)			\
{								\
	return;							\
}
#endif /* ERRATA_A57_813419 */

#if ERRATA_A53_819472 || ERRATA_A53_824069 || ERRATA_A53_827319
/*
 * Define function for DC instruction with register parameter that enables
 * the workaround for errata 819472, 824069 and 827319 of Cortex-A53.
 */
#define DEFINE_DCOP_ERRATA_A53_TYPE_PARAM_FUNC(_name, _type)	\
static inline void dc ## _name(uint64_t v)			\
{								\
	return;							\
}
#endif /* ERRATA_A53_819472 || ERRATA_A53_824069 || ERRATA_A53_827319 */

#if ERRATA_A57_813419
DEFINE_SYSOP_TYPE_FUNC(tlbi, alle1)
DEFINE_SYSOP_TYPE_FUNC(tlbi, alle1is)
DEFINE_SYSOP_TYPE_FUNC(tlbi, alle2)
DEFINE_SYSOP_TYPE_FUNC(tlbi, alle2is)
DEFINE_TLBIOP_ERRATA_TYPE_FUNC(alle3)
DEFINE_TLBIOP_ERRATA_TYPE_FUNC(alle3is)
DEFINE_SYSOP_TYPE_FUNC(tlbi, vmalle1)
#elif ERRATA_A76_1286807
DEFINE_TLBIOP_ERRATA_TYPE_FUNC(alle1)
DEFINE_TLBIOP_ERRATA_TYPE_FUNC(alle1is)
DEFINE_TLBIOP_ERRATA_TYPE_FUNC(alle2)
DEFINE_TLBIOP_ERRATA_TYPE_FUNC(alle2is)
DEFINE_TLBIOP_ERRATA_TYPE_FUNC(alle3)
DEFINE_TLBIOP_ERRATA_TYPE_FUNC(alle3is)
DEFINE_TLBIOP_ERRATA_TYPE_FUNC(vmalle1)
#else
DEFINE_SYSOP_TYPE_FUNC(tlbi, alle1)
DEFINE_SYSOP_TYPE_FUNC(tlbi, alle1is)
DEFINE_SYSOP_TYPE_FUNC(tlbi, alle2)
DEFINE_SYSOP_TYPE_FUNC(tlbi, alle2is)
DEFINE_SYSOP_TYPE_FUNC(tlbi, alle3)
DEFINE_SYSOP_TYPE_FUNC(tlbi, alle3is)
DEFINE_SYSOP_TYPE_FUNC(tlbi, vmalle1)
#endif

#if ERRATA_A57_813419
DEFINE_SYSOP_TYPE_PARAM_FUNC(tlbi, vaae1is)
DEFINE_SYSOP_TYPE_PARAM_FUNC(tlbi, vaale1is)
DEFINE_SYSOP_TYPE_PARAM_FUNC(tlbi, vae2is)
DEFINE_SYSOP_TYPE_PARAM_FUNC(tlbi, vale2is)
DEFINE_TLBIOP_ERRATA_TYPE_PARAM_FUNC(vae3is)
DEFINE_TLBIOP_ERRATA_TYPE_PARAM_FUNC(vale3is)
#elif ERRATA_A76_1286807
DEFINE_TLBIOP_ERRATA_TYPE_PARAM_FUNC(vaae1is)
DEFINE_TLBIOP_ERRATA_TYPE_PARAM_FUNC(vaale1is)
DEFINE_TLBIOP_ERRATA_TYPE_PARAM_FUNC(vae2is)
DEFINE_TLBIOP_ERRATA_TYPE_PARAM_FUNC(vale2is)
DEFINE_TLBIOP_ERRATA_TYPE_PARAM_FUNC(vae3is)
DEFINE_TLBIOP_ERRATA_TYPE_PARAM_FUNC(vale3is)
#else
DEFINE_SYSOP_TYPE_PARAM_FUNC(tlbi, vaae1is)
DEFINE_SYSOP_TYPE_PARAM_FUNC(tlbi, vaale1is)
DEFINE_SYSOP_TYPE_PARAM_FUNC(tlbi, vae2is)
DEFINE_SYSOP_TYPE_PARAM_FUNC(tlbi, vale2is)
DEFINE_SYSOP_TYPE_PARAM_FUNC(tlbi, vae3is)
DEFINE_SYSOP_TYPE_PARAM_FUNC(tlbi, vale3is)
#endif

/*******************************************************************************
 * Cache maintenance accessor prototypes
 ******************************************************************************/
DEFINE_SYSOP_TYPE_PARAM_FUNC(dc, isw)
DEFINE_SYSOP_TYPE_PARAM_FUNC(dc, cisw)
#if ERRATA_A53_827319
DEFINE_DCOP_ERRATA_A53_TYPE_PARAM_FUNC(csw, cisw)
#else
DEFINE_SYSOP_TYPE_PARAM_FUNC(dc, csw)
#endif
#if ERRATA_A53_819472 || ERRATA_A53_824069 || ERRATA_A53_827319
DEFINE_DCOP_ERRATA_A53_TYPE_PARAM_FUNC(cvac, civac)
#else
DEFINE_SYSOP_TYPE_PARAM_FUNC(dc, cvac)
#endif
DEFINE_SYSOP_TYPE_PARAM_FUNC(dc, ivac)
DEFINE_SYSOP_TYPE_PARAM_FUNC(dc, civac)
#if ERRATA_A53_819472 || ERRATA_A53_824069 || ERRATA_A53_827319
DEFINE_DCOP_ERRATA_A53_TYPE_PARAM_FUNC(cvau, civac)
#else
DEFINE_SYSOP_TYPE_PARAM_FUNC(dc, cvau)
#endif
DEFINE_SYSOP_TYPE_PARAM_FUNC(dc, zva)

/*******************************************************************************
 * Address translation accessor prototypes
 ******************************************************************************/
DEFINE_SYSOP_TYPE_PARAM_FUNC(at, s12e1r)
DEFINE_SYSOP_TYPE_PARAM_FUNC(at, s12e1w)
DEFINE_SYSOP_TYPE_PARAM_FUNC(at, s12e0r)
DEFINE_SYSOP_TYPE_PARAM_FUNC(at, s12e0w)
DEFINE_SYSOP_TYPE_PARAM_FUNC(at, s1e1r)
DEFINE_SYSOP_TYPE_PARAM_FUNC(at, s1e2r)
DEFINE_SYSOP_TYPE_PARAM_FUNC(at, s1e3r)

/*******************************************************************************
 * Strip Pointer Authentication Code
 ******************************************************************************/
DEFINE_SYSOP_PARAM_FUNC(xpaci)

void flush_dcache_range(uintptr_t addr, size_t size);
void flush_dcache_to_popa_range(uintptr_t addr, size_t size);
void flush_dcache_to_popa_range_mte2(uintptr_t addr, size_t size);
void clean_dcache_range(uintptr_t addr, size_t size);
void inv_dcache_range(uintptr_t addr, size_t size);
bool is_dcache_enabled(void);

void dcsw_op_louis(u_register_t op_type);
void dcsw_op_all(u_register_t op_type);

void disable_mmu_el1(void);
void disable_mmu_el3(void);
void disable_mmu_icache_el1(void);
void disable_mmu_icache_el3(void);

/*******************************************************************************
 * Misc. accessor prototypes
 ******************************************************************************/

#define write_daifclr(val) SYSREG_WRITE_CONST(daifclr, val)
#define write_daifset(val) SYSREG_WRITE_CONST(daifset, val)

DEFINE_SYSREG_RW_FUNCS(par_el1)
DEFINE_SYSREG_READ_FUNC(id_pfr1_el1)
DEFINE_SYSREG_READ_FUNC(id_aa64isar0_el1)
DEFINE_SYSREG_READ_FUNC(id_aa64isar1_el1)
DEFINE_SYSREG_READ_FUNC(id_aa64pfr0_el1)
DEFINE_SYSREG_READ_FUNC(id_aa64pfr1_el1)
DEFINE_SYSREG_READ_FUNC(id_aa64dfr0_el1)
DEFINE_SYSREG_READ_FUNC(id_afr0_el1)
DEFINE_SYSREG_READ_FUNC(CurrentEl)
DEFINE_SYSREG_READ_FUNC(ctr_el0)
DEFINE_SYSREG_RW_FUNCS(daif)
DEFINE_SYSREG_RW_FUNCS(spsr_el1)
DEFINE_SYSREG_RW_FUNCS(spsr_el2)
DEFINE_SYSREG_RW_FUNCS(spsr_el3)
DEFINE_SYSREG_RW_FUNCS(elr_el1)
DEFINE_SYSREG_RW_FUNCS(elr_el2)
DEFINE_SYSREG_RW_FUNCS(elr_el3)
DEFINE_SYSREG_RW_FUNCS(mdccsr_el0)
DEFINE_SYSREG_RW_FUNCS(dbgdtrrx_el0)
DEFINE_SYSREG_RW_FUNCS(dbgdtrtx_el0)

DEFINE_SYSOP_FUNC(wfi)
DEFINE_SYSOP_FUNC(wfe)
DEFINE_SYSOP_FUNC(sev)
DEFINE_SYSOP_TYPE_FUNC(dsb, sy)
DEFINE_SYSOP_TYPE_FUNC(dmb, sy)
DEFINE_SYSOP_TYPE_FUNC(dmb, st)
DEFINE_SYSOP_TYPE_FUNC(dmb, ld)
DEFINE_SYSOP_TYPE_FUNC(dsb, ish)
DEFINE_SYSOP_TYPE_FUNC(dsb, osh)
DEFINE_SYSOP_TYPE_FUNC(dsb, nsh)
DEFINE_SYSOP_TYPE_FUNC(dsb, ishst)
DEFINE_SYSOP_TYPE_FUNC(dsb, oshst)
DEFINE_SYSOP_TYPE_FUNC(dmb, oshld)
DEFINE_SYSOP_TYPE_FUNC(dmb, oshst)
DEFINE_SYSOP_TYPE_FUNC(dmb, osh)
DEFINE_SYSOP_TYPE_FUNC(dmb, nshld)
DEFINE_SYSOP_TYPE_FUNC(dmb, nshst)
DEFINE_SYSOP_TYPE_FUNC(dmb, nsh)
DEFINE_SYSOP_TYPE_FUNC(dmb, ishld)
DEFINE_SYSOP_TYPE_FUNC(dmb, ishst)
DEFINE_SYSOP_TYPE_FUNC(dmb, ish)
DEFINE_SYSOP_FUNC(isb)

static inline void enable_irq(void)
{
	/*
	 * The compiler memory barrier will prevent the compiler from
	 * scheduling non-volatile memory access after the write to the
	 * register.
	 *
	 * This could happen if some initialization code issues non-volatile
	 * accesses to an area used by an interrupt handler, in the assumption
	 * that it is safe as the interrupts are disabled at the time it does
	 * that (according to program order). However, non-volatile accesses
	 * are not necessarily in program order relatively with volatile inline
	 * assembly statements (and volatile accesses).
	 */
	return;
}

static inline void enable_fiq(void)
{
	return;
}

static inline void enable_serror(void)
{
	return;
}

static inline void enable_debug_exceptions(void)
{
	return;
}

static inline void disable_irq(void)
{
	return;
}

static inline void disable_fiq(void)
{
	return;
}

static inline void disable_serror(void)
{
	return;
}

static inline void disable_debug_exceptions(void)
{
	return;
}

void __dead2 smc(uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3,
		 uint64_t x4, uint64_t x5, uint64_t x6, uint64_t x7);

/*******************************************************************************
 * System register accessor prototypes
 ******************************************************************************/
DEFINE_SYSREG_READ_FUNC(midr_el1)
DEFINE_SYSREG_READ_FUNC(mpidr_el1)
DEFINE_SYSREG_READ_FUNC(id_aa64mmfr0_el1)
DEFINE_SYSREG_READ_FUNC(id_aa64mmfr1_el1)

DEFINE_SYSREG_RW_FUNCS(scr_el3)
DEFINE_SYSREG_RW_FUNCS(hcr_el2)

DEFINE_SYSREG_RW_FUNCS(vbar_el1)
DEFINE_SYSREG_RW_FUNCS(vbar_el2)
DEFINE_SYSREG_RW_FUNCS(vbar_el3)

DEFINE_SYSREG_RW_FUNCS(sctlr_el1)
DEFINE_SYSREG_RW_FUNCS(sctlr_el2)
DEFINE_SYSREG_RW_FUNCS(sctlr_el3)

DEFINE_SYSREG_RW_FUNCS(actlr_el1)
DEFINE_SYSREG_RW_FUNCS(actlr_el2)
DEFINE_SYSREG_RW_FUNCS(actlr_el3)

DEFINE_SYSREG_RW_FUNCS(esr_el1)
DEFINE_SYSREG_RW_FUNCS(esr_el2)
DEFINE_SYSREG_RW_FUNCS(esr_el3)

DEFINE_SYSREG_RW_FUNCS(afsr0_el1)
DEFINE_SYSREG_RW_FUNCS(afsr0_el2)
DEFINE_SYSREG_RW_FUNCS(afsr0_el3)

DEFINE_SYSREG_RW_FUNCS(afsr1_el1)
DEFINE_SYSREG_RW_FUNCS(afsr1_el2)
DEFINE_SYSREG_RW_FUNCS(afsr1_el3)

DEFINE_SYSREG_RW_FUNCS(far_el1)
DEFINE_SYSREG_RW_FUNCS(far_el2)
DEFINE_SYSREG_RW_FUNCS(far_el3)

DEFINE_SYSREG_RW_FUNCS(mair_el1)
DEFINE_SYSREG_RW_FUNCS(mair_el2)
DEFINE_SYSREG_RW_FUNCS(mair_el3)

DEFINE_SYSREG_RW_FUNCS(amair_el1)
DEFINE_SYSREG_RW_FUNCS(amair_el2)
DEFINE_SYSREG_RW_FUNCS(amair_el3)

DEFINE_SYSREG_READ_FUNC(rvbar_el1)
DEFINE_SYSREG_READ_FUNC(rvbar_el2)
DEFINE_SYSREG_READ_FUNC(rvbar_el3)

DEFINE_SYSREG_RW_FUNCS(rmr_el1)
DEFINE_SYSREG_RW_FUNCS(rmr_el2)
DEFINE_SYSREG_RW_FUNCS(rmr_el3)

DEFINE_SYSREG_RW_FUNCS(tcr_el1)
DEFINE_SYSREG_RW_FUNCS(tcr_el2)
DEFINE_SYSREG_RW_FUNCS(tcr_el3)

DEFINE_SYSREG_RW_FUNCS(ttbr0_el1)
DEFINE_SYSREG_RW_FUNCS(ttbr0_el2)
DEFINE_SYSREG_RW_FUNCS(ttbr0_el3)

DEFINE_SYSREG_RW_FUNCS(ttbr1_el1)

DEFINE_SYSREG_RW_FUNCS(vttbr_el2)

DEFINE_SYSREG_RW_FUNCS(cptr_el2)
DEFINE_SYSREG_RW_FUNCS(cptr_el3)

DEFINE_SYSREG_RW_FUNCS(cpacr_el1)
DEFINE_SYSREG_RW_FUNCS(cntfrq_el0)
DEFINE_SYSREG_RW_FUNCS(cnthp_ctl_el2)
DEFINE_SYSREG_RW_FUNCS(cnthp_tval_el2)
DEFINE_SYSREG_RW_FUNCS(cnthp_cval_el2)
DEFINE_SYSREG_RW_FUNCS(cntps_ctl_el1)
DEFINE_SYSREG_RW_FUNCS(cntps_tval_el1)
DEFINE_SYSREG_RW_FUNCS(cntps_cval_el1)
DEFINE_SYSREG_RW_FUNCS(cntp_ctl_el0)
DEFINE_SYSREG_RW_FUNCS(cntp_tval_el0)
DEFINE_SYSREG_RW_FUNCS(cntp_cval_el0)
DEFINE_SYSREG_READ_FUNC(cntpct_el0)
DEFINE_SYSREG_RW_FUNCS(cnthctl_el2)

#define get_cntp_ctl_enable(x)  (((x) >> CNTP_CTL_ENABLE_SHIFT) & \
					CNTP_CTL_ENABLE_MASK)
#define get_cntp_ctl_imask(x)   (((x) >> CNTP_CTL_IMASK_SHIFT) & \
					CNTP_CTL_IMASK_MASK)
#define get_cntp_ctl_istatus(x) (((x) >> CNTP_CTL_ISTATUS_SHIFT) & \
					CNTP_CTL_ISTATUS_MASK)

#define set_cntp_ctl_enable(x)  ((x) |= (U(1) << CNTP_CTL_ENABLE_SHIFT))
#define set_cntp_ctl_imask(x)   ((x) |= (U(1) << CNTP_CTL_IMASK_SHIFT))

#define clr_cntp_ctl_enable(x)  ((x) &= ~(U(1) << CNTP_CTL_ENABLE_SHIFT))
#define clr_cntp_ctl_imask(x)   ((x) &= ~(U(1) << CNTP_CTL_IMASK_SHIFT))

DEFINE_SYSREG_RW_FUNCS(tpidr_el3)

DEFINE_SYSREG_RW_FUNCS(cntvoff_el2)

DEFINE_SYSREG_RW_FUNCS(vpidr_el2)
DEFINE_SYSREG_RW_FUNCS(vmpidr_el2)

DEFINE_SYSREG_READ_FUNC(isr_el1)

DEFINE_SYSREG_RW_FUNCS(mdcr_el2)
DEFINE_SYSREG_RW_FUNCS(mdcr_el3)
DEFINE_SYSREG_RW_FUNCS(hstr_el2)
DEFINE_SYSREG_RW_FUNCS(pmcr_el0)

/* GICv3 System Registers */

DEFINE_RENAME_SYSREG_RW_FUNCS(icc_sre_el1, ICC_SRE_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(icc_sre_el2, ICC_SRE_EL2)
DEFINE_RENAME_SYSREG_RW_FUNCS(icc_sre_el3, ICC_SRE_EL3)
DEFINE_RENAME_SYSREG_RW_FUNCS(icc_pmr_el1, ICC_PMR_EL1)
DEFINE_RENAME_SYSREG_READ_FUNC(icc_rpr_el1, ICC_RPR_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(icc_igrpen1_el3, ICC_IGRPEN1_EL3)
DEFINE_RENAME_SYSREG_RW_FUNCS(icc_igrpen1_el1, ICC_IGRPEN1_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(icc_igrpen0_el1, ICC_IGRPEN0_EL1)
DEFINE_RENAME_SYSREG_READ_FUNC(icc_hppir0_el1, ICC_HPPIR0_EL1)
DEFINE_RENAME_SYSREG_READ_FUNC(icc_hppir1_el1, ICC_HPPIR1_EL1)
DEFINE_RENAME_SYSREG_READ_FUNC(icc_iar0_el1, ICC_IAR0_EL1)
DEFINE_RENAME_SYSREG_READ_FUNC(icc_iar1_el1, ICC_IAR1_EL1)
DEFINE_RENAME_SYSREG_WRITE_FUNC(icc_eoir0_el1, ICC_EOIR0_EL1)
DEFINE_RENAME_SYSREG_WRITE_FUNC(icc_eoir1_el1, ICC_EOIR1_EL1)
DEFINE_RENAME_SYSREG_WRITE_FUNC(icc_sgi0r_el1, ICC_SGI0R_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(icc_sgi1r, ICC_SGI1R)

DEFINE_RENAME_SYSREG_READ_FUNC(amcfgr_el0, AMCFGR_EL0)
DEFINE_RENAME_SYSREG_READ_FUNC(amcgcr_el0, AMCGCR_EL0)
DEFINE_RENAME_SYSREG_READ_FUNC(amcg1idr_el0, AMCG1IDR_EL0)
DEFINE_RENAME_SYSREG_RW_FUNCS(amcr_el0, AMCR_EL0)
DEFINE_RENAME_SYSREG_RW_FUNCS(amcntenclr0_el0, AMCNTENCLR0_EL0)
DEFINE_RENAME_SYSREG_RW_FUNCS(amcntenset0_el0, AMCNTENSET0_EL0)
DEFINE_RENAME_SYSREG_RW_FUNCS(amcntenclr1_el0, AMCNTENCLR1_EL0)
DEFINE_RENAME_SYSREG_RW_FUNCS(amcntenset1_el0, AMCNTENSET1_EL0)

DEFINE_RENAME_SYSREG_READ_FUNC(mpamidr_el1, MPAMIDR_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(mpam3_el3, MPAM3_EL3)
DEFINE_RENAME_SYSREG_RW_FUNCS(mpam2_el2, MPAM2_EL2)
DEFINE_RENAME_SYSREG_RW_FUNCS(mpamhcr_el2, MPAMHCR_EL2)

DEFINE_RENAME_SYSREG_RW_FUNCS(pmblimitr_el1, PMBLIMITR_EL1)

DEFINE_RENAME_SYSREG_WRITE_FUNC(zcr_el3, ZCR_EL3)
DEFINE_RENAME_SYSREG_WRITE_FUNC(zcr_el2, ZCR_EL2)

DEFINE_RENAME_SYSREG_READ_FUNC(erridr_el1, ERRIDR_EL1)
DEFINE_RENAME_SYSREG_WRITE_FUNC(errselr_el1, ERRSELR_EL1)

DEFINE_RENAME_SYSREG_READ_FUNC(erxfr_el1, ERXFR_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(erxctlr_el1, ERXCTLR_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(erxstatus_el1, ERXSTATUS_EL1)
DEFINE_RENAME_SYSREG_READ_FUNC(erxaddr_el1, ERXADDR_EL1)
DEFINE_RENAME_SYSREG_READ_FUNC(erxmisc0_el1, ERXMISC0_EL1)
DEFINE_RENAME_SYSREG_READ_FUNC(erxmisc1_el1, ERXMISC1_EL1)

/* Armv8.2 Registers */
DEFINE_RENAME_SYSREG_READ_FUNC(id_aa64mmfr2_el1, ID_AA64MMFR2_EL1)

/* Armv8.3 Pointer Authentication Registers */
DEFINE_RENAME_SYSREG_RW_FUNCS(apiakeyhi_el1, APIAKeyHi_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(apiakeylo_el1, APIAKeyLo_EL1)

/* Armv8.5 MTE Registers */
DEFINE_RENAME_SYSREG_RW_FUNCS(tfsre0_el1, TFSRE0_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(tfsr_el1, TFSR_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(rgsr_el1, RGSR_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(gcr_el1, GCR_EL1)

/* Armv8.5 FEAT_RNG Registers */
DEFINE_SYSREG_READ_FUNC(rndr)
DEFINE_SYSREG_READ_FUNC(rndrrs)

/* DynamIQ Shared Unit power management */
DEFINE_RENAME_SYSREG_RW_FUNCS(clusterpwrdn_el1, CLUSTERPWRDN_EL1)

/* Armv9.2 RME Registers */
DEFINE_RENAME_SYSREG_RW_FUNCS(gptbr_el3, GPTBR_EL3)
DEFINE_RENAME_SYSREG_RW_FUNCS(gpccr_el3, GPCCR_EL3)

#define IS_IN_EL(x) \
	(GET_EL(read_CurrentEl()) == MODE_EL##x)

#define IS_IN_EL1() IS_IN_EL(1)
#define IS_IN_EL2() IS_IN_EL(2)
#define IS_IN_EL3() IS_IN_EL(3)

static inline unsigned int get_current_el(void)
{
	return GET_EL(read_CurrentEl());
}

static inline unsigned int get_current_el_maybe_constant(void)
{
#if defined(IMAGE_AT_EL1)
	return 1;
#elif defined(IMAGE_AT_EL2)
	return 2;	/* no use-case in TF-A */
#elif defined(IMAGE_AT_EL3)
	return 3;
#else
	/*
	 * If we do not know which exception level this is being built for
	 * (e.g. built for library), fall back to run-time detection.
	 */
	return get_current_el();
#endif
}

/*
 * Check if an EL is implemented from AA64PFR0 register fields.
 */
static inline uint64_t el_implemented(unsigned int el)
{
	if (el > 3U) {
		return EL_IMPL_NONE;
	} else {
	/*	unsigned int shift = ID_AA64PFR0_EL1_SHIFT * el;

		return (read_id_aa64pfr0_el1() >> shift) & ID_AA64PFR0_ELX_MASK;*/
		return 0;
	}
}

/*
 * TLBIPAALLOS instruction
 * (TLB Inivalidate GPT Information by PA,
 * All Entries, Outer Shareable)
 */
static inline void tlbipaallos(void)
{
	return;
}

/*
 * Invalidate TLBs of GPT entries by Physical address, last level.
 *
 * @pa: the starting address for the range
 *      of invalidation
 * @size: size of the range of invalidation
 */
void gpt_tlbi_by_pa_ll(uint64_t pa, size_t size);

/*
 * TLBI RPALOS instructions
 * (TLB Range Invalidate GPT Information by PA, Last level, Outer Shareable)
 *
 * command SIZE, bits [47:44] field:
 * 0b0000	4KB
 * 0b0001	16KB
 * 0b0010	64KB
 * 0b0011	2MB
 * 0b0100	32MB
 * 0b0101	512MB
 * 0b0110	1GB
 * 0b0111	16GB
 * 0b1000	64GB
 * 0b1001	512GB
 */
#define TLBI_SZ_4K		0UL
#define TLBI_SZ_16K		1UL
#define TLBI_SZ_64K		2UL
#define TLBI_SZ_2M		3UL
#define TLBI_SZ_32M		4UL
#define TLBI_SZ_512M		5UL
#define TLBI_SZ_1G		6UL
#define TLBI_SZ_16G		7UL
#define TLBI_SZ_64G		8UL
#define TLBI_SZ_512G		9UL

#define	TLBI_ADDR_SHIFT		U(12)
#define	TLBI_SIZE_SHIFT		U(44)

#define TLBIRPALOS(_addr, _size)				\
{								\
	u_register_t arg = ((_addr) >> TLBI_ADDR_SHIFT) |	\
			   ((_size) << TLBI_SIZE_SHIFT);	\
}

/* Note: addr must be aligned to 4KB */
static inline void tlbirpalos_4k(uintptr_t addr)
{
	TLBIRPALOS(addr, TLBI_SZ_4K);
}

/* Note: addr must be aligned to 16KB */
static inline void tlbirpalos_16k(uintptr_t addr)
{
	TLBIRPALOS(addr, TLBI_SZ_16K);
}

/* Note: addr must be aligned to 64KB */
static inline void tlbirpalos_64k(uintptr_t addr)
{
	TLBIRPALOS(addr, TLBI_SZ_64K);
}

/* Note: addr must be aligned to 2MB */
static inline void tlbirpalos_2m(uintptr_t addr)
{
	TLBIRPALOS(addr, TLBI_SZ_2M);
}

/* Note: addr must be aligned to 32MB */
static inline void tlbirpalos_32m(uintptr_t addr)
{
	TLBIRPALOS(addr, TLBI_SZ_32M);
}

/* Note: addr must be aligned to 512MB */
static inline void tlbirpalos_512m(uintptr_t addr)
{
	TLBIRPALOS(addr, TLBI_SZ_512M);
}


/* Previously defined accessor functions with incomplete register names  */

#define read_current_el()	read_CurrentEl()

#define dsb()			dsbsy()

#define read_midr()		read_midr_el1()

#define read_mpidr()		read_mpidr_el1()

#define read_scr()		read_scr_el3()
#define write_scr(_v)		write_scr_el3(_v)

#define read_hcr()		read_hcr_el2()
#define write_hcr(_v)		write_hcr_el2(_v)

#define read_cpacr()		read_cpacr_el1()
#define write_cpacr(_v)		write_cpacr_el1(_v)

#define read_clusterpwrdn()	read_clusterpwrdn_el1()
#define write_clusterpwrdn(_v)	write_clusterpwrdn_el1(_v)

#if ERRATA_SPECULATIVE_AT
/*
 * Assuming SCTLR.M bit is already enabled
 * 1. Enable page table walk by clearing TCR_EL1.EPDx bits
 * 2. Execute AT instruction for lower EL1/0
 * 3. Disable page table walk by setting TCR_EL1.EPDx bits
 */
#define AT(_at_inst, _va)	\
{	\
	return;	\
}
#else
#define AT(_at_inst, _va)	_at_inst(_va);
#endif

#endif /* ARCH_HELPERS_H */

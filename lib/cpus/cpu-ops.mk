#
# Copyright (c) 2014-2023, Arm Limited and Contributors. All rights reserved.
# Copyright (c) 2020-2022, NVIDIA Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include ${MAKE_HELPERS_DIRECTORY}$/build_macros.mk

# Cortex A57 specific optimisation to skip L1 cache flush when
# cluster is powered down.
CPU_FLAG_LIST += SKIP_A57_L1_FLUSH_PWR_DWN

# Flag to disable the cache non-temporal hint.
# It is enabled by default.
A53_DISABLE_NON_TEMPORAL_HINT		?=1
CPU_FLAG_LIST += A53_DISABLE_NON_TEMPORAL_HINT

# Flag to disable the cache non-temporal hint.
# It is enabled by default.
A57_DISABLE_NON_TEMPORAL_HINT		?=1
CPU_FLAG_LIST += A57_DISABLE_NON_TEMPORAL_HINT

# Flag to enable higher performance non-cacheable load forwarding.
# It is disabled by default.
CPU_FLAG_LIST += A57_ENABLE_NONCACHEABLE_LOAD_FWD

WORKAROUND_CVE_2017_5715		?=1
CPU_FLAG_LIST += WORKAROUND_CVE_2017_5715
WORKAROUND_CVE_2018_3639		?=1
CPU_FLAG_LIST += WORKAROUND_CVE_2018_3639
CPU_FLAG_LIST += DYNAMIC_WORKAROUND_CVE_2018_3639
WORKAROUND_CVE_2022_23960		?=1
CPU_FLAG_LIST += WORKAROUND_CVE_2022_23960

# Flags to indicate internal or external Last level cache
# By default internal
CPU_FLAG_LIST += NEOVERSE_Nx_EXTERNAL_LLC

# CPU Errata Build flags.
# These should be enabled by the platform if the erratum workaround needs to be
# applied.

# Flag to apply erratum 794073 workaround when disabling mmu.
CPU_FLAG_LIST += ERRATA_A9_794073

# Flag to apply erratum 816470 workaround during power down. This erratum
# applies only to revision >= r3p0 of the Cortex A15 cpu.
CPU_FLAG_LIST += ERRATA_A15_816470

# Flag to apply erratum 827671 workaround during reset. This erratum applies
# only to revision >= r3p0 of the Cortex A15 cpu.
CPU_FLAG_LIST += ERRATA_A15_827671

# Flag to apply erratum 852421 workaround during reset. This erratum applies
# only to revision <= r1p2 of the Cortex A17 cpu.
CPU_FLAG_LIST += ERRATA_A17_852421

# Flag to apply erratum 852423 workaround during reset. This erratum applies
# only to revision <= r1p2 of the Cortex A17 cpu.
CPU_FLAG_LIST += ERRATA_A17_852423

# Flag to apply erratum 855472 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A35 cpu.
CPU_FLAG_LIST += ERRATA_A35_855472

# Flag to apply erratum 819472 workaround during reset. This erratum applies
# only to revision <= r0p1 of the Cortex A53 cpu.
CPU_FLAG_LIST += ERRATA_A53_819472

# Flag to apply erratum 824069 workaround during reset. This erratum applies
# only to revision <= r0p2 of the Cortex A53 cpu.
CPU_FLAG_LIST += ERRATA_A53_824069

# Flag to apply erratum 826319 workaround during reset. This erratum applies
# only to revision <= r0p2 of the Cortex A53 cpu.
CPU_FLAG_LIST += ERRATA_A53_826319

# Flag to apply erratum 827319 workaround during reset. This erratum applies
# only to revision <= r0p2 of the Cortex A53 cpu.
CPU_FLAG_LIST += ERRATA_A53_827319

# Flag to apply erratum 835769 workaround at compile and link time. This
# erratum applies to revision <= r0p4 of the Cortex A53 cpu. Enabling this
# workaround can lead the linker to create "*.stub" sections.
CPU_FLAG_LIST += ERRATA_A53_835769

# Flag to apply erratum 836870 workaround during reset. This erratum applies
# only to revision <= r0p3 of the Cortex A53 cpu. From r0p4 and onwards, this
# erratum workaround is enabled by default in hardware.
CPU_FLAG_LIST += ERRATA_A53_836870

# Flag to apply erratum 843419 workaround at link time.
# This erratum applies to revision <= r0p4 of the Cortex A53 cpu. Enabling this
# workaround could lead the linker to emit "*.stub" sections which are 4kB
# aligned.
CPU_FLAG_LIST += ERRATA_A53_843419

# Flag to apply errata 855873 during reset. This errata applies to all
# revisions of the Cortex A53 CPU, but this firmware workaround only works
# for revisions r0p3 and higher. Earlier revisions are taken care
# of by the rich OS.
CPU_FLAG_LIST += ERRATA_A53_855873

# Flag to apply erratum 1530924 workaround during reset. This erratum applies
# to all revisions of Cortex A53 cpu.
CPU_FLAG_LIST += ERRATA_A53_1530924

# Flag to apply erratum 768277 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A55 cpu.
CPU_FLAG_LIST += ERRATA_A55_768277

# Flag to apply erratum 778703 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A55 cpu.
CPU_FLAG_LIST += ERRATA_A55_778703

# Flag to apply erratum 798797 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A55 cpu.
CPU_FLAG_LIST += ERRATA_A55_798797

# Flag to apply erratum 846532 workaround during reset. This erratum applies
# only to revision <= r0p1 of the Cortex A55 cpu.
CPU_FLAG_LIST += ERRATA_A55_846532

# Flag to apply erratum 903758 workaround during reset. This erratum applies
# only to revision <= r0p1 of the Cortex A55 cpu.
CPU_FLAG_LIST += ERRATA_A55_903758

# Flag to apply erratum 1221012 workaround during reset. This erratum applies
# only to revision <= r1p0 of the Cortex A55 cpu.
CPU_FLAG_LIST += ERRATA_A55_1221012

# Flag to apply erratum 1530923 workaround during reset. This erratum applies
# to all revisions of Cortex A55 cpu.
CPU_FLAG_LIST += ERRATA_A55_1530923

# Flag to apply erratum 806969 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A57 cpu.
CPU_FLAG_LIST += ERRATA_A57_806969

# Flag to apply erratum 813419 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A57 cpu.
CPU_FLAG_LIST += ERRATA_A57_813419

# Flag to apply erratum 813420 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A57 cpu.
CPU_FLAG_LIST += ERRATA_A57_813420

# Flag to apply erratum 814670 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A57 cpu.
CPU_FLAG_LIST += ERRATA_A57_814670

# Flag to apply erratum 817169 workaround during power down. This erratum
# applies only to revision <= r0p1 of the Cortex A57 cpu.
CPU_FLAG_LIST += ERRATA_A57_817169

# Flag to apply erratum 826974 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A57 cpu.
CPU_FLAG_LIST += ERRATA_A57_826974

# Flag to apply erratum 826977 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A57 cpu.
CPU_FLAG_LIST += ERRATA_A57_826977

# Flag to apply erratum 828024 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A57 cpu.
CPU_FLAG_LIST += ERRATA_A57_828024

# Flag to apply erratum 829520 workaround during reset. This erratum applies
# only to revision <= r1p2 of the Cortex A57 cpu.
CPU_FLAG_LIST += ERRATA_A57_829520

# Flag to apply erratum 833471 workaround during reset. This erratum applies
# only to revision <= r1p2 of the Cortex A57 cpu.
CPU_FLAG_LIST += ERRATA_A57_833471

# Flag to apply erratum 855972 workaround during reset. This erratum applies
# only to revision <= r1p3 of the Cortex A57 cpu.
CPU_FLAG_LIST += ERRATA_A57_859972

# Flag to apply erratum 1319537 workaround during reset. This erratum applies
# to all revisions of Cortex A57 cpu.
CPU_FLAG_LIST += ERRATA_A57_1319537

# Flag to apply erratum 855971 workaround during reset. This erratum applies
# only to revision <= r0p3 of the Cortex A72 cpu.
CPU_FLAG_LIST += ERRATA_A72_859971

# Flag to apply erratum 1319367 workaround during reset. This erratum applies
# to all revisions of Cortex A72 cpu.
CPU_FLAG_LIST += ERRATA_A72_1319367

# Flag to apply erratum 852427 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A73 cpu.
CPU_FLAG_LIST += ERRATA_A73_852427

# Flag to apply erratum 855423 workaround during reset. This erratum applies
# only to revision <= r0p1 of the Cortex A73 cpu.
CPU_FLAG_LIST += ERRATA_A73_855423

# Flag to apply erratum 764081 workaround during reset. This erratum applies
# only to revision <= r0p0 of the Cortex A75 cpu.
CPU_FLAG_LIST += ERRATA_A75_764081

# Flag to apply erratum 790748 workaround during reset. This erratum applies
# only to revision <= r0p0 of the Cortex A75 cpu.
CPU_FLAG_LIST += ERRATA_A75_790748

# Flag to apply erratum 1073348 workaround during reset. This erratum applies
# only to revision <= r1p0 of the Cortex A76 cpu.
CPU_FLAG_LIST += ERRATA_A76_1073348

# Flag to apply erratum 1130799 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Cortex A76 cpu.
CPU_FLAG_LIST += ERRATA_A76_1130799

# Flag to apply erratum 1220197 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Cortex A76 cpu.
CPU_FLAG_LIST += ERRATA_A76_1220197

# Flag to apply erratum 1257314 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Cortex A76 cpu.
CPU_FLAG_LIST += ERRATA_A76_1257314

# Flag to apply erratum 1262606 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Cortex A76 cpu.
CPU_FLAG_LIST += ERRATA_A76_1262606

# Flag to apply erratum 1262888 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Cortex A76 cpu.
CPU_FLAG_LIST += ERRATA_A76_1262888

# Flag to apply erratum 1275112 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Cortex A76 cpu.
CPU_FLAG_LIST += ERRATA_A76_1275112

# Flag to apply erratum 1286807 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Cortex A76 cpu.
CPU_FLAG_LIST += ERRATA_A76_1286807

# Flag to apply erratum 1791580 workaround during reset. This erratum applies
# only to revision <= r4p0 of the Cortex A76 cpu.
CPU_FLAG_LIST += ERRATA_A76_1791580

# Flag to apply erratum 1165522 workaround during reset. This erratum applies
# to all revisions of Cortex A76 cpu.
CPU_FLAG_LIST += ERRATA_A76_1165522

# Flag to apply erratum 1868343 workaround during reset. This erratum applies
# only to revision <= r4p0 of the Cortex A76 cpu.
CPU_FLAG_LIST += ERRATA_A76_1868343

# Flag to apply erratum 1946160 workaround during reset. This erratum applies
# only to revisions r3p0 - r4p1 of the Cortex A76 cpu.
CPU_FLAG_LIST += ERRATA_A76_1946160

# Flag to apply erratum 2743102 workaround during powerdown. This erratum
# applies to all revisions <= r4p1 of the Cortex A76 cpu and is still open.
CPU_FLAG_LIST += ERRATA_A76_2743102

# Flag to apply erratum 1508412 workaround during reset. This erratum applies
# only to revision <= r1p0 of the Cortex A77 cpu.
CPU_FLAG_LIST += ERRATA_A77_1508412

# Flag to apply erratum 1925769 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A77 cpu.
CPU_FLAG_LIST += ERRATA_A77_1925769

# Flag to apply erratum 1946167 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A77 cpu.
CPU_FLAG_LIST += ERRATA_A77_1946167

# Flag to apply erratum 1791578 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, and r1p1, it is still open.
CPU_FLAG_LIST += ERRATA_A77_1791578

# Flag to apply erratum 2356587 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, and r1p1, it is still open.
CPU_FLAG_LIST += ERRATA_A77_2356587

# Flag to apply erratum 1800714 workaround during reset. This erratum applies
# to revisions <= r1p1 of the Cortex A77 cpu.
CPU_FLAG_LIST += ERRATA_A77_1800714

# Flag to apply erratum 2743100 workaround during power down. This erratum
# applies to revisions r0p0, r1p0, and r1p1, it is still open.
CPU_FLAG_LIST += ERRATA_A77_2743100

# Flag to apply erratum 1688305 workaround during reset. This erratum applies
# to revisions r0p0 - r1p0 of the A78 cpu.
CPU_FLAG_LIST += ERRATA_A78_1688305

# Flag to apply erratum 1941498 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, and r1p1 of the A78 cpu.
CPU_FLAG_LIST += ERRATA_A78_1941498

# Flag to apply erratum 1951500 workaround during reset. This erratum applies
# to revisions r1p0 and r1p1 of the A78 cpu. The issue is present in r0p0 as
# well but there is no workaround for that revision.
CPU_FLAG_LIST += ERRATA_A78_1951500

# Flag to apply erratum 1821534 workaround during reset. This erratum applies
# to revisions r0p0 and r1p0 of the A78 cpu.
CPU_FLAG_LIST += ERRATA_A78_1821534

# Flag to apply erratum 1952683 workaround during reset. This erratum applies
# to revision r0p0 of the A78 cpu and was fixed in the revision r1p0.
CPU_FLAG_LIST += ERRATA_A78_1952683

# Flag to apply erratum 2132060 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, r1p1, and r1p2 of the A78 cpu. It is still open.
CPU_FLAG_LIST += ERRATA_A78_2132060

# Flag to apply erratum 2242635 workaround during reset. This erratum applies
# to revisions r1p0, r1p1, and r1p2 of the A78 cpu and is open. The issue is
# present in r0p0 as well but there is no workaround for that revision.
CPU_FLAG_LIST += ERRATA_A78_2242635

# Flag to apply erratum 2376745 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, r1p1, and r1p2 of the A78 cpu. It is still open.
CPU_FLAG_LIST += ERRATA_A78_2376745

# Flag to apply erratum 2395406 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, r1p1, and r1p2 of the A78 cpu. It is still open.
CPU_FLAG_LIST += ERRATA_A78_2395406

# Flag to apply erratum 2712571 workaround for non-arm interconnect ip. This
# erratum applies to revisions r0p0, r1p0, r1p1, and r1p2 of the A78 cpu.
# It is fixed in r1p2.
CPU_FLAG_LIST += ERRATA_A78_2712571

# Flag to apply erratum 2742426 workaround during reset. This erratum
# applies to revisions r0p0, r1p0, r1p1 and r1p2 of the A78 cpu. It is still
# open.
CPU_FLAG_LIST += ERRATA_A78_2742426

# Flag to apply erratum 2772019 workaround during powerdown. This erratum
# applies to revisions r0p0, r1p0, r1p1 and r1p2 of the A78 cpu. It is still
# open.
CPU_FLAG_LIST += ERRATA_A78_2772019

# Flag to apply erratum 2779479 workaround during reset. This erratum applies
# to revision r0p0, r1p0, r1p1 and r1p2 of the A78 cpu. It is still open.
CPU_FLAG_LIST += ERRATA_A78_2779479

# Flag to apply erratum 1941500 workaround during reset. This erratum applies
# to revisions r0p0 and r0p1 of the A78 AE cpu. It is still open.
CPU_FLAG_LIST += ERRATA_A78_AE_1941500

# Flag to apply erratum 1951502 workaround during reset. This erratum applies
# to revisions r0p0 and r0p1 of the A78 AE cpu. It is still open.
CPU_FLAG_LIST += ERRATA_A78_AE_1951502

# Flag to apply erratum 2376748 workaround during reset. This erratum applies
# to revisions r0p0, r0p1 and r0p2 of the A78 AE cpu. It is still open.
CPU_FLAG_LIST += ERRATA_A78_AE_2376748

# Flag to apply erratum 2395408 workaround during reset. This erratum applies
# to revisions r0p0 and r0p1 of the A78 AE cpu. It is still open.
CPU_FLAG_LIST += ERRATA_A78_AE_2395408

# Flag to apply erratum 1827430 workaround during reset. This erratum applies
# to revision r0p0 of the A78C cpu. It is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_A78C_1827430

# Flag to apply erratum 1827440 workaround during reset. This erratum applies
# to revision r0p0 of the A78C cpu. It is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_A78C_1827440

# Flag to apply erratum 2712574 workaround for non-arm interconnect ip. This
# erratum applies to revisions r0p0, r0p1 and r0p2 of the A78 AE cpu.
# It is still open.
CPU_FLAG_LIST += ERRATA_A78_AE_2712574

# Flag to apply erratum 2132064 workaround during reset. This erratum applies
# to revisions r0p1 and r0p2 of the A78C cpu. It is still open.
CPU_FLAG_LIST += ERRATA_A78C_2132064

# Flag to apply erratum 2242638 workaround during reset. This erratum applies
# to revisions r0p1 and r0p2 of the A78C cpu. It is still open.
CPU_FLAG_LIST += ERRATA_A78C_2242638

# Flag to apply erratum 2376749 workaround during reset. This erratum applies
# to revisions r0p1 and r0p2 of the A78C cpu. It is still open.
CPU_FLAG_LIST += ERRATA_A78C_2376749

# Flag to apply erratum 2395411 workaround during reset. This erratum applies
# to revisions r0p1 and r0p2 of the A78C cpu. It is still open.
CPU_FLAG_LIST += ERRATA_A78C_2395411

# Flag to apply erratum 2712575 workaround for non-arm interconnect ip. This
# erratum applies to revisions r0p1 and r0p2 of the A78C cpu.
# It is still open.
CPU_FLAG_LIST += ERRATA_A78C_2712575

# Flag to apply erratum 2772121 workaround during powerdown. This erratum
# applies to revisions r0p0, r0p1 and r0p2 of the A78C cpu. It is still open.
CPU_FLAG_LIST += ERRATA_A78C_2772121

# Flag to apply erratum 2779484 workaround during reset. This erratum
# applies to revisions r0p1 and r0p2 of the A78C cpu. It is still open.
CPU_FLAG_LIST += ERRATA_A78C_2779484

# Flag to apply erratum 1821534 workaround during reset. This erratum applies
# to revisions r0p0 - r1p0 of the X1 cpu and fixed in r1p1.
CPU_FLAG_LIST += ERRATA_X1_1821534

# Flag to apply erratum 1688305 workaround during reset. This erratum applies
# to revisions r0p0 - r1p0 of the X1 cpu and fixed in r1p1.
CPU_FLAG_LIST += ERRATA_X1_1688305

# Flag to apply erratum 1827429 workaround during reset. This erratum applies
# to revisions r0p0 - r1p0 of the X1 cpu and fixed in r1p1.
CPU_FLAG_LIST += ERRATA_X1_1827429

# Flag to apply T32 CLREX workaround during reset. This erratum applies
# only to r0p0 and r1p0 of the Neoverse N1 cpu.
CPU_FLAG_LIST += ERRATA_N1_1043202

# Flag to apply erratum 1073348 workaround during reset. This erratum applies
# only to revision r0p0 and r1p0 of the Neoverse N1 cpu.
CPU_FLAG_LIST += ERRATA_N1_1073348

# Flag to apply erratum 1130799 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Neoverse N1 cpu.
CPU_FLAG_LIST += ERRATA_N1_1130799

# Flag to apply erratum 1165347 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Neoverse N1 cpu.
CPU_FLAG_LIST += ERRATA_N1_1165347

# Flag to apply erratum 1207823 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Neoverse N1 cpu.
CPU_FLAG_LIST += ERRATA_N1_1207823

# Flag to apply erratum 1220197 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Neoverse N1 cpu.
CPU_FLAG_LIST += ERRATA_N1_1220197

# Flag to apply erratum 1257314 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Neoverse N1 cpu.
CPU_FLAG_LIST += ERRATA_N1_1257314

# Flag to apply erratum 1262606 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Neoverse N1 cpu.
CPU_FLAG_LIST += ERRATA_N1_1262606

# Flag to apply erratum 1262888 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Neoverse N1 cpu.
CPU_FLAG_LIST += ERRATA_N1_1262888

# Flag to apply erratum 1275112 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Neoverse N1 cpu.
CPU_FLAG_LIST += ERRATA_N1_1275112

# Flag to apply erratum 1315703 workaround during reset. This erratum applies
# to revisions before r3p1 of the Neoverse N1 cpu.
CPU_FLAG_LIST += ERRATA_N1_1315703

# Flag to apply erratum 1542419 workaround during reset. This erratum applies
# to revisions r3p0 - r4p0 of the Neoverse N1 cpu.
CPU_FLAG_LIST += ERRATA_N1_1542419

# Flag to apply erratum 1868343 workaround during reset. This erratum applies
# to revision <= r4p0 of the Neoverse N1 cpu.
CPU_FLAG_LIST += ERRATA_N1_1868343

# Flag to apply erratum 1946160 workaround during reset. This erratum applies
# to revisions r3p0, r3p1, r4p0, and r4p1 of the Neoverse N1 cpu. The issue
# exists in revisions r0p0, r1p0, and r2p0 as well but there is no workaround.
CPU_FLAG_LIST += ERRATA_N1_1946160

# Flag to apply erratum 2743102 workaround during powerdown. This erratum
# applies to all revisions <= r4p1 of the Neoverse N1 cpu and is still open.
CPU_FLAG_LIST += ERRATA_N1_2743102

# Flag to apply erratum 1618635 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse V1 cpu and was fixed in the revision r1p0.
CPU_FLAG_LIST += ERRATA_V1_1618635

# Flag to apply erratum 1774420 workaround during reset. This erratum applies
# to revisions r0p0 and r1p0 of the Neoverse V1 core, and was fixed in r1p1.
CPU_FLAG_LIST += ERRATA_V1_1774420

# Flag to apply erratum 1791573 workaround during reset. This erratum applies
# to revisions r0p0 and r1p0 of the Neoverse V1 core, and was fixed in r1p1.
CPU_FLAG_LIST += ERRATA_V1_1791573

# Flag to apply erratum 1852267 workaround during reset. This erratum applies
# to revisions r0p0 and r1p0 of the Neoverse V1 core, and was fixed in r1p1.
CPU_FLAG_LIST += ERRATA_V1_1852267

# Flag to apply erratum 1925756 workaround during reset. This needs to be
# enabled for r0p0, r1p0, and r1p1 of the Neoverse V1 core, it is still open.
CPU_FLAG_LIST += ERRATA_V1_1925756

# Flag to apply erratum 1940577 workaround during reset. This erratum applies
# to revisions r1p0 and r1p1 of the Neoverse V1 cpu.
CPU_FLAG_LIST += ERRATA_V1_1940577

# Flag to apply erratum 1966096 workaround during reset. This erratum applies
# to revisions r1p0 and r1p1 of the Neoverse V1 CPU and is open. This issue
# exists in r0p0 as well but there is no workaround for that revision.
CPU_FLAG_LIST += ERRATA_V1_1966096

# Flag to apply erratum 2139242 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, and r1p1 of the Neoverse V1 cpu and is still open.
CPU_FLAG_LIST += ERRATA_V1_2139242

# Flag to apply erratum 2108267 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, and r1p1 of the Neoverse V1 cpu and is still open.
CPU_FLAG_LIST += ERRATA_V1_2108267

# Flag to apply erratum 2216392 workaround during reset. This erratum applies
# to revisions r1p0 and r1p1 of the Neoverse V1 cpu and is still open. This
# issue exists in r0p0 as well but there is no workaround for that revision.
CPU_FLAG_LIST += ERRATA_V1_2216392

# Flag to apply erratum 2294912 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, and r1p1 and r1p2 of the Neoverse V1 cpu and is still open.
CPU_FLAG_LIST += ERRATA_V1_2294912

# Flag to apply erratum 2372203 workaround during reset. This erratum applies
# to revisions r0p0, r1p0 and r1p1 of the Neoverse V1 cpu and is still open.
CPU_FLAG_LIST += ERRATA_V1_2372203

# Flag to apply erratum 2701953 workaround to non-arm interconnect ip. This
# erratum applies to revisions r0p0, r1p0, r1p1 of the Neoverse V1 cpu,
# it is fixed in r1p2.
CPU_FLAG_LIST += ERRATA_V1_2701953

# Flag to apply erratum 2743093 workaround during powerdown. This erratum
# applies to revisions r0p0, r1p0, r1p1 and r1p2  of the Neoverse V1 cpu and is
# still open.
CPU_FLAG_LIST += ERRATA_V1_2743093

# Flag to apply erratum 2743233 workaround during powerdown. This erratum
# applies to revisions r0p0, r1p0, r1p1 and r1p2 of the Neoverse V1 cpu and is
# still open.
CPU_FLAG_LIST += ERRATA_V1_2743233

# Flag to apply erratum 2779461 workaround during powerdown. This erratum
# applies to revisions r0p0, r1p0, r1p1 and r1p2  of the Neoverse V1 cpu and is
# still open.
CPU_FLAG_LIST += ERRATA_V1_2779461

# Flag to apply erratum 1987031 workaround during reset. This erratum applies
# to revisions r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is still open.
CPU_FLAG_LIST += ERRATA_A710_1987031

# Flag to apply erratum 2081180 workaround during reset. This erratum applies
# to revisions r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is still open.
CPU_FLAG_LIST += ERRATA_A710_2081180

# Flag to apply erratum 2083908 workaround during reset. This erratum applies
# to revision r2p0 of the Cortex-A710 cpu and is still open.
CPU_FLAG_LIST += ERRATA_A710_2083908

# Flag to apply erratum 2058056 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, r2p0 and r2p1 of the Cortex-A710 cpu and is still
# open.
CPU_FLAG_LIST += ERRATA_A710_2058056

# Flag to apply erratum 2055002 workaround during reset. This erratum applies
# to revision r1p0, r2p0 of the Cortex-A710 cpu and is still open.
CPU_FLAG_LIST += ERRATA_A710_2055002

# Flag to apply erratum 2017096 workaround during reset. This erratum applies
# to revision r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is still open.
CPU_FLAG_LIST += ERRATA_A710_2017096

# Flag to apply erratum 2267065 workaround during reset. This erratum applies
# to revision r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is fixed in r2p1.
CPU_FLAG_LIST += ERRATA_A710_2267065

# Flag to apply erratum 2136059 workaround during reset. This erratum applies
# to revision r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is fixed in r2p1.
CPU_FLAG_LIST += ERRATA_A710_2136059

# Flag to apply erratum 2147715 workaround during reset. This erratum applies
# to revision r2p0 of the Cortex-A710 CPU and is fixed in revision r2p1.
CPU_FLAG_LIST += ERRATA_A710_2147715

# Flag to apply erratum 2216384 workaround during reset. This erratum applies
# to revision r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is fixed in r2p1.
CPU_FLAG_LIST += ERRATA_A710_2216384

# Flag to apply erratum 2282622 workaround during reset. This erratum applies
# to revision r0p0, r1p0, r2p0 and r2p1 of the Cortex-A710 cpu and is still
# open.
CPU_FLAG_LIST += ERRATA_A710_2282622

# Flag to apply erratum 2291219 workaround during reset. This erratum applies
# to revision r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is fixed in r2p1.
CPU_FLAG_LIST += ERRATA_A710_2291219

# Flag to apply erratum 2008768 workaround during reset. This erratum applies
# to revision r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is fixed in r2p1.
CPU_FLAG_LIST += ERRATA_A710_2008768

# Flag to apply erratum 2371105 workaround during reset. This erratum applies
# to revision r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is fixed in r2p1.
CPU_FLAG_LIST += ERRATA_A710_2371105

# Flag to apply erratum 2701952 workaround for non-arm interconnect ip. This
# erratum applies to revision r0p0, r1p0, r2p0, r2p1 of the Cortex-A710 cpu
# and is still open.
CPU_FLAG_LIST += ERRATA_A710_2701952

# Flag to apply erratum 2742423 workaround during reset. This erratum applies
# to revision r0p0, r1p0, r2p0 and r2p1 of the Cortex-A710 cpu and is still
# open.
CPU_FLAG_LIST += ERRATA_A710_2742423

# Flag to apply erratum 2768515 workaround during power down. This erratum
# applies to revision r0p0, r1p0, r2p0 and r2p1 of the Cortex-A710 cpu and is
# still open.
CPU_FLAG_LIST += ERRATA_A710_2768515

# Flag to apply erratum 2002655 workaround during reset. This erratum applies
# to revisions r0p0 of the Neoverse-N2 cpu and is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_N2_2002655

# Flag to apply erratum 2009478 workaround during powerdown. This erratum
# applies to revision r0p0 of the Neoverse N2 cpu, it is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_N2_2009478

# Flag to apply erratum 2067956 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_N2_2067956

# Flag to apply erratum 2025414 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_N2_2025414

# Flag to apply erratum 2189731 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_N2_2189731

# Flag to apply erratum 2138956 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_N2_2138956

# Flag to apply erratum 2138953 workaround during reset. This erratum applies
# to revision r0p0, r0p1, r0p2, r0p3 of the Neoverse N2 cpu and is still open.
CPU_FLAG_LIST += ERRATA_N2_2138953

# Flag to apply erratum 2242415 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_N2_2242415

# Flag to apply erratum 2138958 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_N2_2138958

# Flag to apply erratum 2242400 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_N2_2242400

# Flag to apply erratum 2280757 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_N2_2280757

# Flag to apply erratum 2326639 workaroud during powerdown. This erratum
# applies to revision r0p0 of the Neoverse N2 cpu and is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_N2_2326639

# Flag to apply erratum 2340933 workaroud during reset. This erratum
# applies to revision r0p0 of the Neoverse N2 cpu and is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_N2_2340933

# Flag to apply erratum 2346952 workaround during reset. This erratum applies
# to r0p0, r0p1, r0p2 of the Neoverse N2 cpu, it is fixed in r0p3.
CPU_FLAG_LIST += ERRATA_N2_2346952

# Flag to apply erratum 2376738 workaround during reset. This erratum applies
# to revision r0p0, r0p1, r0p2, r0p3 of the Neoverse N2 cpu and is still open.
CPU_FLAG_LIST += ERRATA_N2_2376738

# Flag to apply erratum 2388450 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu, it is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_N2_2388450

# Flag to apply erratum 2728475 workaround for non-arm interconnect ip. This
# erratum applies to r0p0, r0p1, r0p2 of the Neoverse N2 cpu, it is fixed in
# r0p3.
CPU_FLAG_LIST += ERRATA_N2_2728475

# Flag to apply erratum 2743014 workaround during reset. This erratum applies
# to r0p0, r0p1, r0p2 of the Neoverse N2 cpu, it is fixed in r0p3.
CPU_FLAG_LIST += ERRATA_N2_2743014

# Flag to apply erratum 2743089 workaround during during powerdown. This erratum
# applies to all revisions <= r0p2 of the Neoverse N2 cpu, it is fixed in r0p3.
CPU_FLAG_LIST += ERRATA_N2_2743089

# Flag to apply erratum 2779511 workaround during reset. This erratum applies
# to r0p0, r0p1, r0p2 of the Neoverse N2 cpu, it is fixed in r0p3.
CPU_FLAG_LIST += ERRATA_N2_2779511

# Flag to apply erratum 2002765 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, and r2p0 of the Cortex-X2 cpu and is still open.
CPU_FLAG_LIST += ERRATA_X2_2002765

# Flag to apply erratum 2058056 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, r2p0 and r2p1 of the Cortex-X2 cpu and is still open.
CPU_FLAG_LIST += ERRATA_X2_2058056

# Flag to apply erratum 2083908 workaround during reset. This erratum applies
# to revision r2p0 of the Cortex-X2 cpu and is still open.
CPU_FLAG_LIST += ERRATA_X2_2083908

# Flag to apply erratum 2017096 workaround during reset. This erratum applies
# only to revisions r0p0, r1p0 and r2p0 of the Cortex-X2 cpu, it is fixed in
# r2p1.
CPU_FLAG_LIST += ERRATA_X2_2017096

# Flag to apply erratum 2081180 workaround during reset. This erratum applies
# only to revisions r0p0, r1p0 and r2p0 of the Cortex-X2 cpu, it is fixed in
# r2p1.
CPU_FLAG_LIST += ERRATA_X2_2081180

# Flag to apply erratum 2216384 workaround during reset. This erratum applies
# only to revisions r0p0, r1p0 and r2p0 of the Cortex-X2 cpu, it is fixed in
# r2p1.
CPU_FLAG_LIST += ERRATA_X2_2216384

# Flag to apply erratum 2147715 workaround during reset. This erratum applies
# only to revision r2p0 of the Cortex-X2 cpu, it is fixed in r2p1.
CPU_FLAG_LIST += ERRATA_X2_2147715

# Flag to apply erratum 2282622 workaround during reset. This erratum applies
# to revision r0p0, r1p0, r2p0 and r2p1 of the Cortex-X2 cpu and is still
# open.
CPU_FLAG_LIST += ERRATA_X2_2282622

# Flag to apply erratum 2371105 workaround during reset. This erratum applies
# to revision r0p0, r1p0 and r2p0 of the Cortex-X2 cpu and is fixed in r2p1.
CPU_FLAG_LIST += ERRATA_X2_2371105

# Flag to apply erratum 2701952 workaround for non-arm interconnect ip. This
# erratum applies to revisions r0p0, r1p0, r2p0, r2p1 of the Cortex-x2 cpu
# and is still open.
CPU_FLAG_LIST += ERRATA_X2_2701952

# Flag to apply erratum 2742423 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, r2p0 and r2p1 of the Cortex-X2 cpu and is still open.
CPU_FLAG_LIST += ERRATA_X2_2742423

# Flag to apply erratum 2768515 workaround during power down. This erratum
# applies to revision r0p0, r1p0, r2p0 and r2p1 of the Cortex-X2 cpu and is
# still open.
CPU_FLAG_LIST += ERRATA_X2_2768515

# Flag to apply erratum 2070301 workaround on reset. This erratum applies
# to revisions r0p0, r1p0, r1p1 and r1p2 of the Cortex-X3 cpu and is
# still open.
CPU_FLAG_LIST += ERRATA_X3_2070301

# Flag to apply erratum 2313909 workaround on powerdown. This erratum applies
# to revisions r0p0 and r1p0 of the Cortex-X3 cpu, it is fixed in r1p1.
CPU_FLAG_LIST += ERRATA_X3_2313909

# Flag to apply erratum 2615812 workaround on powerdown. This erratum applies
# to revisions r0p0, r1p0, r1p1 of the Cortex-X3 cpu, it is still open.
CPU_FLAG_LIST += ERRATA_X3_2615812

# Flag to apply erratum 2742421 workaround on reset. This erratum applies
# to revisions r0p0, r1p0 and r1p1 of the Cortex-X3 cpu, it is fixed in r1p2.
CPU_FLAG_LIST += ERRATA_X3_2742421

# Flag to apply erratum 1922240 workaround during reset. This erratum applies
# to revision r0p0 of the Cortex-A510 cpu and is fixed in r0p1.
CPU_FLAG_LIST += ERRATA_A510_1922240

# Flag to apply erratum 2288014 workaround during reset. This erratum applies
# to revisions r0p0, r0p1, r0p2, r0p3 and r1p0 of the Cortex-A510 cpu and is
# fixed in r1p1.
CPU_FLAG_LIST += ERRATA_A510_2288014

# Flag to apply erratum 2042739 workaround during reset. This erratum applies
# to revisions r0p0, r0p1 and r0p2 of the Cortex-A510 cpu and is fixed in r0p3.
CPU_FLAG_LIST += ERRATA_A510_2042739

# Flag to apply erratum 2041909 workaround during reset. This erratum applies
# to revision r0p2 of the Cortex-A510 cpu and is fixed in r0p3. The issue is
# present in r0p0 and r0p1 but there is no workaround for those revisions.
CPU_FLAG_LIST += ERRATA_A510_2041909

# Flag to aply erratum 2080326 workaround during reset. This erratum applies
# to revision r0p2 of the Cortex-A510 cpu and is fixed in r0p3. The issue is
# also present in r0p0 and r0p1 but there is no workaround for those revisions.
CPU_FLAG_LIST += ERRATA_A510_2080326

# Flag to apply erratum 2250311 workaround during reset. This erratum applies
# to revisions r0p0, r0p1, r0p2, r0p3 and r1p0, and is fixed in r1p1.
CPU_FLAG_LIST += ERRATA_A510_2250311

# Flag to apply erratum 2218950 workaround during reset. This erratum applies
# to revisions r0p0, r0p1, r0p2, r0p3 and r1p0, and is fixed in r1p1.
CPU_FLAG_LIST += ERRATA_A510_2218950

# Flag to apply erratum 2172148 workaround during reset. This erratum applies
# to revisions r0p0, r0p1, r0p2, r0p3 and r1p0, and is fixed in r1p1.
CPU_FLAG_LIST += ERRATA_A510_2172148

# Flag to apply erratum 2347730 workaround during reset. This erratum applies
# to revisions r0p0, r0p1, r0p2, r0p3, r1p0 and r1p1 of the Cortex-A510 CPU,
# and is fixed in r1p2.
CPU_FLAG_LIST += ERRATA_A510_2347730

# Flag to apply erratum 2371937 workaround during reset. This erratum applies
# to revisions r0p0, r0p1, r0p2, r0p3, r1p0, and r1p1. It is fixed in r1p2.
CPU_FLAG_LIST += ERRATA_A510_2371937

# Flag to apply erratum 2666669 workaround during reset. This erratum applies
# to revisions r0p0, r0p1, r0p2, r0p3, r1p0, and r1p1. It is fixed in r1p2.
CPU_FLAG_LIST += ERRATA_A510_2666669

# Flag to apply erratum 2684597 workaround during powerdown. This erratum
# applies to revision r0p0, r0p1, r0p2, r0p3, r1p0, r1p1 and r1p2 of the
# Cortex-A510 cpu and is fixed in r1p3.
CPU_FLAG_LIST += ERRATA_A510_2684597

# Flag to apply erratum 2331132 workaround during reset. This erratum applies
# to revisions r0p0, r0p1 and r0p2. It is still open.
CPU_FLAG_LIST += ERRATA_V2_2331132

# Flag to apply erratum 2719103 workaround for non-arm interconnect ip. This
# erratum applies to revisions r0p0, rop1. Fixed in r0p2.
CPU_FLAG_LIST += ERRATA_V2_2719103

# Flag to apply erratum 2719105 workaround during reset. This erratum applies
# to revisions r0p0 and r0p1. It is fixed in r0p2.
CPU_FLAG_LIST += ERRATA_V2_2719105

# Flag to apply erratum 2743011 workaround during reset. This erratum applies
# to revisions r0p0 and r0p1. It is fixed in r0p2.
CPU_FLAG_LIST += ERRATA_V2_2743011

# Flag to apply erratum 2779510 workaround during reset. This erratum applies
# to revisions r0p0 and r0p1. It is fixed in r0p2.
CPU_FLAG_LIST += ERRATA_V2_2779510

# Flag to apply erratum 2801372 workaround for all configurations.
# This erratum applies to revisions r0p0, r0p1. Fixed in r0p2.
CPU_FLAG_LIST += ERRATA_V2_2801372

# Flag to apply erratum 2701951 workaround for non-arm interconnect ip.
# This erratum applies to revisions r0p0, r1p0, and r1p1. Its is fixed in r1p2.
CPU_FLAG_LIST += ERRATA_A715_2701951

# Flag to apply DSU erratum 798953. This erratum applies to DSUs revision r0p0.
# Applying the workaround results in higher DSU power consumption on idle.
CPU_FLAG_LIST += ERRATA_DSU_798953

# Flag to apply DSU erratum 936184. This erratum applies to DSUs containing
# the ACP interface and revision < r2p0. Applying the workaround results in
# higher DSU power consumption on idle.
CPU_FLAG_LIST += ERRATA_DSU_936184

# Flag to apply DSU erratum 2313941. This erratum applies to DSUs revisions
# r0p0, r1p0, r2p0, r2p1, r3p0, r3p1 and is still open. Applying the workaround
# results in higher DSU power consumption on idle.
CPU_FLAG_LIST += ERRATA_DSU_2313941

ifneq (${DYNAMIC_WORKAROUND_CVE_2018_3639},0)
	ifeq (${WORKAROUND_CVE_2018_3639},0)
		$(error "Error: WORKAROUND_CVE_2018_3639 must be 1 if DYNAMIC_WORKAROUND_CVE_2018_3639 is 1")
	endif
endif

# process all flags
$(eval $(call default_zeros, $(CPU_FLAG_LIST)))
$(eval $(call add_defines, $(CPU_FLAG_LIST)))
$(eval $(call assert_booleans, $(CPU_FLAG_LIST)))

# Errata build flags
ifneq (${ERRATA_A53_843419},0)
TF_LDFLAGS_aarch64	+= --fix-cortex-a53-843419
endif

ifneq (${ERRATA_A53_835769},0)
TF_CFLAGS_aarch64	+= -mfix-cortex-a53-835769
TF_LDFLAGS_aarch64	+= --fix-cortex-a53-835769
endif

ifneq ($(filter 1,${ERRATA_A53_1530924} ${ERRATA_A55_1530923}	\
	${ERRATA_A57_1319537} ${ERRATA_A72_1319367} ${ERRATA_A76_1165522}),)
ERRATA_SPECULATIVE_AT	:= 1
else
ERRATA_SPECULATIVE_AT	:= 0
endif

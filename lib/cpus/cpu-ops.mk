#
# Copyright (c) 2014-2022, Arm Limited and Contributors. All rights reserved.
# Copyright (c) 2020-2022, NVIDIA Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Cortex A57 specific optimisation to skip L1 cache flush when
# cluster is powered down.
SKIP_A57_L1_FLUSH_PWR_DWN		?=0

# Flag to disable the cache non-temporal hint.
# It is enabled by default.
A53_DISABLE_NON_TEMPORAL_HINT		?=1

# Flag to disable the cache non-temporal hint.
# It is enabled by default.
A57_DISABLE_NON_TEMPORAL_HINT		?=1

# Flag to enable higher performance non-cacheable load forwarding.
# It is disabled by default.
A57_ENABLE_NONCACHEABLE_LOAD_FWD	?= 0

WORKAROUND_CVE_2017_5715		?=1
WORKAROUND_CVE_2018_3639		?=1
DYNAMIC_WORKAROUND_CVE_2018_3639	?=0
WORKAROUND_CVE_2022_23960		?=1

# Flags to indicate internal or external Last level cache
# By default internal
NEOVERSE_Nx_EXTERNAL_LLC		?=0

# Process A57_ENABLE_NONCACHEABLE_LOAD_FWD flag
$(eval $(call assert_boolean,A57_ENABLE_NONCACHEABLE_LOAD_FWD))
$(eval $(call add_define,A57_ENABLE_NONCACHEABLE_LOAD_FWD))

# Process SKIP_A57_L1_FLUSH_PWR_DWN flag
$(eval $(call assert_boolean,SKIP_A57_L1_FLUSH_PWR_DWN))
$(eval $(call add_define,SKIP_A57_L1_FLUSH_PWR_DWN))

# Process A53_DISABLE_NON_TEMPORAL_HINT flag
$(eval $(call assert_boolean,A53_DISABLE_NON_TEMPORAL_HINT))
$(eval $(call add_define,A53_DISABLE_NON_TEMPORAL_HINT))

# Process A57_DISABLE_NON_TEMPORAL_HINT flag
$(eval $(call assert_boolean,A57_DISABLE_NON_TEMPORAL_HINT))
$(eval $(call add_define,A57_DISABLE_NON_TEMPORAL_HINT))

# Process WORKAROUND_CVE_2017_5715 flag
$(eval $(call assert_boolean,WORKAROUND_CVE_2017_5715))
$(eval $(call add_define,WORKAROUND_CVE_2017_5715))

# Process WORKAROUND_CVE_2018_3639 flag
$(eval $(call assert_boolean,WORKAROUND_CVE_2018_3639))
$(eval $(call add_define,WORKAROUND_CVE_2018_3639))

$(eval $(call assert_boolean,DYNAMIC_WORKAROUND_CVE_2018_3639))
$(eval $(call add_define,DYNAMIC_WORKAROUND_CVE_2018_3639))

# Process WORKAROUND_CVE_2022_23960 flag
$(eval $(call assert_boolean,WORKAROUND_CVE_2022_23960))
$(eval $(call add_define,WORKAROUND_CVE_2022_23960))

$(eval $(call assert_boolean,NEOVERSE_Nx_EXTERNAL_LLC))
$(eval $(call add_define,NEOVERSE_Nx_EXTERNAL_LLC))

ifneq (${DYNAMIC_WORKAROUND_CVE_2018_3639},0)
	ifeq (${WORKAROUND_CVE_2018_3639},0)
		$(error "Error: WORKAROUND_CVE_2018_3639 must be 1 if DYNAMIC_WORKAROUND_CVE_2018_3639 is 1")
	endif
endif

# CPU Errata Build flags.
# These should be enabled by the platform if the erratum workaround needs to be
# applied.

# Flag to apply erratum 794073 workaround when disabling mmu.
ERRATA_A9_794073	?=0

# Flag to apply erratum 816470 workaround during power down. This erratum
# applies only to revision >= r3p0 of the Cortex A15 cpu.
ERRATA_A15_816470	?=0

# Flag to apply erratum 827671 workaround during reset. This erratum applies
# only to revision >= r3p0 of the Cortex A15 cpu.
ERRATA_A15_827671	?=0

# Flag to apply erratum 852421 workaround during reset. This erratum applies
# only to revision <= r1p2 of the Cortex A17 cpu.
ERRATA_A17_852421	?=0

# Flag to apply erratum 852423 workaround during reset. This erratum applies
# only to revision <= r1p2 of the Cortex A17 cpu.
ERRATA_A17_852423	?=0

# Flag to apply erratum 855472 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A35 cpu.
ERRATA_A35_855472	?=0

# Flag to apply erratum 819472 workaround during reset. This erratum applies
# only to revision <= r0p1 of the Cortex A53 cpu.
ERRATA_A53_819472	?=0

# Flag to apply erratum 824069 workaround during reset. This erratum applies
# only to revision <= r0p2 of the Cortex A53 cpu.
ERRATA_A53_824069	?=0

# Flag to apply erratum 826319 workaround during reset. This erratum applies
# only to revision <= r0p2 of the Cortex A53 cpu.
ERRATA_A53_826319	?=0

# Flag to apply erratum 827319 workaround during reset. This erratum applies
# only to revision <= r0p2 of the Cortex A53 cpu.
ERRATA_A53_827319	?=0

# Flag to apply erratum 835769 workaround at compile and link time. This
# erratum applies to revision <= r0p4 of the Cortex A53 cpu. Enabling this
# workaround can lead the linker to create "*.stub" sections.
ERRATA_A53_835769	?=0

# Flag to apply erratum 836870 workaround during reset. This erratum applies
# only to revision <= r0p3 of the Cortex A53 cpu. From r0p4 and onwards, this
# erratum workaround is enabled by default in hardware.
ERRATA_A53_836870	?=0

# Flag to apply erratum 843419 workaround at link time.
# This erratum applies to revision <= r0p4 of the Cortex A53 cpu. Enabling this
# workaround could lead the linker to emit "*.stub" sections which are 4kB
# aligned.
ERRATA_A53_843419	?=0

# Flag to apply errata 855873 during reset. This errata applies to all
# revisions of the Cortex A53 CPU, but this firmware workaround only works
# for revisions r0p3 and higher. Earlier revisions are taken care
# of by the rich OS.
ERRATA_A53_855873	?=0

# Flag to apply erratum 1530924 workaround during reset. This erratum applies
# to all revisions of Cortex A53 cpu.
ERRATA_A53_1530924	?=0

# Flag to apply erratum 768277 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A55 cpu.
ERRATA_A55_768277	?=0

# Flag to apply erratum 778703 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A55 cpu.
ERRATA_A55_778703	?=0

# Flag to apply erratum 798797 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A55 cpu.
ERRATA_A55_798797	?=0

# Flag to apply erratum 846532 workaround during reset. This erratum applies
# only to revision <= r0p1 of the Cortex A55 cpu.
ERRATA_A55_846532	?=0

# Flag to apply erratum 903758 workaround during reset. This erratum applies
# only to revision <= r0p1 of the Cortex A55 cpu.
ERRATA_A55_903758	?=0

# Flag to apply erratum 1221012 workaround during reset. This erratum applies
# only to revision <= r1p0 of the Cortex A55 cpu.
ERRATA_A55_1221012	?=0

# Flag to apply erratum 1530923 workaround during reset. This erratum applies
# to all revisions of Cortex A55 cpu.
ERRATA_A55_1530923	?=0

# Flag to apply erratum 806969 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A57 cpu.
ERRATA_A57_806969	?=0

# Flag to apply erratum 813419 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A57 cpu.
ERRATA_A57_813419	?=0

# Flag to apply erratum 813420 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A57 cpu.
ERRATA_A57_813420	?=0

# Flag to apply erratum 814670 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A57 cpu.
ERRATA_A57_814670	?=0

# Flag to apply erratum 817169 workaround during power down. This erratum
# applies only to revision <= r0p1 of the Cortex A57 cpu.
ERRATA_A57_817169	?=0

# Flag to apply erratum 826974 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A57 cpu.
ERRATA_A57_826974	?=0

# Flag to apply erratum 826977 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A57 cpu.
ERRATA_A57_826977	?=0

# Flag to apply erratum 828024 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A57 cpu.
ERRATA_A57_828024	?=0

# Flag to apply erratum 829520 workaround during reset. This erratum applies
# only to revision <= r1p2 of the Cortex A57 cpu.
ERRATA_A57_829520	?=0

# Flag to apply erratum 833471 workaround during reset. This erratum applies
# only to revision <= r1p2 of the Cortex A57 cpu.
ERRATA_A57_833471	?=0

# Flag to apply erratum 855972 workaround during reset. This erratum applies
# only to revision <= r1p3 of the Cortex A57 cpu.
ERRATA_A57_859972	?=0

# Flag to apply erratum 1319537 workaround during reset. This erratum applies
# to all revisions of Cortex A57 cpu.
ERRATA_A57_1319537	?=0

# Flag to apply erratum 855971 workaround during reset. This erratum applies
# only to revision <= r0p3 of the Cortex A72 cpu.
ERRATA_A72_859971	?=0

# Flag to apply erratum 1319367 workaround during reset. This erratum applies
# to all revisions of Cortex A72 cpu.
ERRATA_A72_1319367	?=0

# Flag to apply erratum 852427 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A73 cpu.
ERRATA_A73_852427	?=0

# Flag to apply erratum 855423 workaround during reset. This erratum applies
# only to revision <= r0p1 of the Cortex A73 cpu.
ERRATA_A73_855423	?=0

# Flag to apply erratum 764081 workaround during reset. This erratum applies
# only to revision <= r0p0 of the Cortex A75 cpu.
ERRATA_A75_764081	?=0

# Flag to apply erratum 790748 workaround during reset. This erratum applies
# only to revision <= r0p0 of the Cortex A75 cpu.
ERRATA_A75_790748	?=0

# Flag to apply erratum 1073348 workaround during reset. This erratum applies
# only to revision <= r1p0 of the Cortex A76 cpu.
ERRATA_A76_1073348	?=0

# Flag to apply erratum 1130799 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Cortex A76 cpu.
ERRATA_A76_1130799	?=0

# Flag to apply erratum 1220197 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Cortex A76 cpu.
ERRATA_A76_1220197	?=0

# Flag to apply erratum 1257314 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Cortex A76 cpu.
ERRATA_A76_1257314	?=0

# Flag to apply erratum 1262606 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Cortex A76 cpu.
ERRATA_A76_1262606	?=0

# Flag to apply erratum 1262888 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Cortex A76 cpu.
ERRATA_A76_1262888	?=0

# Flag to apply erratum 1275112 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Cortex A76 cpu.
ERRATA_A76_1275112	?=0

# Flag to apply erratum 1286807 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Cortex A76 cpu.
ERRATA_A76_1286807	?=0

# Flag to apply erratum 1791580 workaround during reset. This erratum applies
# only to revision <= r4p0 of the Cortex A76 cpu.
ERRATA_A76_1791580	?=0

# Flag to apply erratum 1165522 workaround during reset. This erratum applies
# to all revisions of Cortex A76 cpu.
ERRATA_A76_1165522	?=0

# Flag to apply erratum 1868343 workaround during reset. This erratum applies
# only to revision <= r4p0 of the Cortex A76 cpu.
ERRATA_A76_1868343	?=0

# Flag to apply erratum 1946160 workaround during reset. This erratum applies
# only to revisions r3p0 - r4p1 of the Cortex A76 cpu.
ERRATA_A76_1946160	?=0

# Flag to apply erratum 1508412 workaround during reset. This erratum applies
# only to revision <= r1p0 of the Cortex A77 cpu.
ERRATA_A77_1508412	?=0

# Flag to apply erratum 1925769 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A77 cpu.
ERRATA_A77_1925769	?=0

# Flag to apply erratum 1946167 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A77 cpu.
ERRATA_A77_1946167	?=0

# Flag to apply erratum 1791578 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, and r1p1, it is still open.
ERRATA_A77_1791578	?=0

# Flag to apply erratum 1688305 workaround during reset. This erratum applies
# to revisions r0p0 - r1p0 of the A78 cpu.
ERRATA_A78_1688305	?=0

# Flag to apply erratum 1941498 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, and r1p1 of the A78 cpu.
ERRATA_A78_1941498	?=0

# Flag to apply erratum 1951500 workaround during reset. This erratum applies
# to revisions r1p0 and r1p1 of the A78 cpu. The issue is present in r0p0 as
# well but there is no workaround for that revision.
ERRATA_A78_1951500	?=0

# Flag to apply erratum 1821534 workaround during reset. This erratum applies
# to revisions r0p0 and r1p0 of the A78 cpu.
ERRATA_A78_1821534	?=0

# Flag to apply erratum 1952683 workaround during reset. This erratum applies
# to revision r0p0 of the A78 cpu and was fixed in the revision r1p0.
ERRATA_A78_1952683	?=0

# Flag to apply erratum 2132060 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, r1p1, and r1p2 of the A78 cpu. It is still open.
ERRATA_A78_2132060	?=0

# Flag to apply erratum 2242635 workaround during reset. This erratum applies
# to revisions r1p0, r1p1, and r1p2 of the A78 cpu and is open. The issue is
# present in r0p0 as well but there is no workaround for that revision.
ERRATA_A78_2242635	?=0

# Flag to apply erratum 2376745 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, r1p1, and r1p2 of the A78 cpu. It is still open.
ERRATA_A78_2376745	?=0

# Flag to apply erratum 2395406 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, r1p1, and r1p2 of the A78 cpu. It is still open.
ERRATA_A78_2395406	?=0

# Flag to apply erratum 1941500 workaround during reset. This erratum applies
# to revisions r0p0 and r0p1 of the A78 AE cpu. It is still open.
ERRATA_A78_AE_1941500	?=0

# Flag to apply erratum 1951502 workaround during reset. This erratum applies
# to revisions r0p0 and r0p1 of the A78 AE cpu. It is still open.
ERRATA_A78_AE_1951502	?=0

# Flag to apply erratum 2376748 workaround during reset. This erratum applies
# to revisions r0p0 and r0p1 of the A78 AE cpu. It is still open.
ERRATA_A78_AE_2376748	?=0

# Flag to apply erratum 2395408 workaround during reset. This erratum applies
# to revisions r0p0 and r0p1 of the A78 AE cpu. It is still open.
ERRATA_A78_AE_2395408	?=0

# Flag to apply erratum 1821534 workaround during reset. This erratum applies
# to revisions r0p0 - r1p0 of the X1 cpu and fixed in r1p1.
ERRATA_X1_1821534	?=0

# Flag to apply erratum 1688305 workaround during reset. This erratum applies
# to revisions r0p0 - r1p0 of the X1 cpu and fixed in r1p1.
ERRATA_X1_1688305	?=0

# Flag to apply erratum 1827429 workaround during reset. This erratum applies
# to revisions r0p0 - r1p0 of the X1 cpu and fixed in r1p1.
ERRATA_X1_1827429	?=0

# Flag to apply T32 CLREX workaround during reset. This erratum applies
# only to r0p0 and r1p0 of the Neoverse N1 cpu.
ERRATA_N1_1043202	?=0

# Flag to apply erratum 1073348 workaround during reset. This erratum applies
# only to revision r0p0 and r1p0 of the Neoverse N1 cpu.
ERRATA_N1_1073348	?=0

# Flag to apply erratum 1130799 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Neoverse N1 cpu.
ERRATA_N1_1130799	?=0

# Flag to apply erratum 1165347 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Neoverse N1 cpu.
ERRATA_N1_1165347	?=0

# Flag to apply erratum 1207823 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Neoverse N1 cpu.
ERRATA_N1_1207823	?=0

# Flag to apply erratum 1220197 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Neoverse N1 cpu.
ERRATA_N1_1220197	?=0

# Flag to apply erratum 1257314 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Neoverse N1 cpu.
ERRATA_N1_1257314	?=0

# Flag to apply erratum 1262606 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Neoverse N1 cpu.
ERRATA_N1_1262606	?=0

# Flag to apply erratum 1262888 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Neoverse N1 cpu.
ERRATA_N1_1262888	?=0

# Flag to apply erratum 1275112 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Neoverse N1 cpu.
ERRATA_N1_1275112	?=0

# Flag to apply erratum 1315703 workaround during reset. This erratum applies
# to revisions before r3p1 of the Neoverse N1 cpu.
ERRATA_N1_1315703	?=0

# Flag to apply erratum 1542419 workaround during reset. This erratum applies
# to revisions r3p0 - r4p0 of the Neoverse N1 cpu.
ERRATA_N1_1542419	?=0

# Flag to apply erratum 1868343 workaround during reset. This erratum applies
# to revision <= r4p0 of the Neoverse N1 cpu.
ERRATA_N1_1868343	?=0

# Flag to apply erratum 1946160 workaround during reset. This erratum applies
# to revisions r3p0, r3p1, r4p0, and r4p1 of the Neoverse N1 cpu. The issue
# exists in revisions r0p0, r1p0, and r2p0 as well but there is no workaround.
ERRATA_N1_1946160	?=0

# Flag to apply erratum 2002655 workaround during reset. This erratum applies
# to revisions r0p0 of the Neoverse-N2 cpu, it is still open.
ERRATA_N2_2002655	?=0

# Flag to apply erratum 1774420 workaround during reset. This erratum applies
# to revisions r0p0 and r1p0 of the Neoverse V1 core, and was fixed in r1p1.
ERRATA_V1_1774420	?=0

# Flag to apply erratum 1791573 workaround during reset. This erratum applies
# to revisions r0p0 and r1p0 of the Neoverse V1 core, and was fixed in r1p1.
ERRATA_V1_1791573	?=0

# Flag to apply erratum 1852267 workaround during reset. This erratum applies
# to revisions r0p0 and r1p0 of the Neoverse V1 core, and was fixed in r1p1.
ERRATA_V1_1852267	?=0

# Flag to apply erratum 1925756 workaround during reset. This needs to be
# enabled for r0p0, r1p0, and r1p1 of the Neoverse V1 core, it is still open.
ERRATA_V1_1925756	?=0

# Flag to apply erratum 1940577 workaround during reset. This erratum applies
# to revisions r1p0 and r1p1 of the Neoverse V1 cpu.
ERRATA_V1_1940577	?=0

# Flag to apply erratum 1966096 workaround during reset. This erratum applies
# to revisions r1p0 and r1p1 of the Neoverse V1 CPU and is open. This issue
# exists in r0p0 as well but there is no workaround for that revision.
ERRATA_V1_1966096	?=0

# Flag to apply erratum 2139242 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, and r1p1 of the Neoverse V1 cpu and is still open.
ERRATA_V1_2139242	?=0

# Flag to apply erratum 2108267 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, and r1p1 of the Neoverse V1 cpu and is still open.
ERRATA_V1_2108267	?=0

# Flag to apply erratum 2216392 workaround during reset. This erratum applies
# to revisions r1p0 and r1p1 of the Neoverse V1 cpu and is still open. This
# issue exists in r0p0 as well but there is no workaround for that revision.
ERRATA_V1_2216392	?=0

# Flag to apply erratum 1987031 workaround during reset. This erratum applies
# to revisions r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is still open.
ERRATA_A710_1987031	?=0

# Flag to apply erratum 2081180 workaround during reset. This erratum applies
# to revisions r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is still open.
ERRATA_A710_2081180	?=0

# Flag to apply erratum 2083908 workaround during reset. This erratum applies
# to revision r2p0 of the Cortex-A710 cpu and is still open.
ERRATA_A710_2083908	?=0

# Flag to apply erratum 2058056 workaround during reset. This erratum applies
# to revisions r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is still open.
ERRATA_A710_2058056	?=0

# Flag to apply erratum 2055002 workaround during reset. This erratum applies
# to revision r1p0, r2p0 of the Cortex-A710 cpu and is still open.
ERRATA_A710_2055002	?=0

# Flag to apply erratum 2017096 workaround during reset. This erratum applies
# to revision r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is still open.
ERRATA_A710_2017096	?=0

# Flag to apply erratum 2267065 workaround during reset. This erratum applies
# to revision r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is fixed in r2p1.
ERRATA_A710_2267065	?=0

# Flag to apply erratum 2136059 workaround during reset. This erratum applies
# to revision r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is fixed in r2p1.
ERRATA_A710_2136059	?=0

# Flag to apply erratum 2282622 workaround during reset. This erratum applies
# to revision r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is fixed in r2p1.
ERRATA_A710_2282622	?=0

# Flag to apply erratum 2008768 workaround during reset. This erratum applies
# to revision r0p0, r1p0 and r2p0 of the Cortex-A710 cpu and is fixed in r2p1.
ERRATA_A710_2008768	?=0

# Flag to apply erratum 2067956 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is still open.
ERRATA_N2_2067956	?=0

# Flag to apply erratum 2025414 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is still open.
ERRATA_N2_2025414	?=0

# Flag to apply erratum 2189731 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is still open.
ERRATA_N2_2189731	?=0

# Flag to apply erratum 2138956 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is still open.
ERRATA_N2_2138956	?=0

# Flag to apply erratum 2138953 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is still open.
ERRATA_N2_2138953	?=0

# Flag to apply erratum 2242415 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is still open.
ERRATA_N2_2242415	?=0

# Flag to apply erratum 2138958 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is still open.
ERRATA_N2_2138958	?=0

# Flag to apply erratum 2242400 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is still open.
ERRATA_N2_2242400	?=0

# Flag to apply erratum 2280757 workaround during reset. This erratum applies
# to revision r0p0 of the Neoverse N2 cpu and is still open.
ERRATA_N2_2280757	?=0

# Flag to apply erratum 2002765 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, and r2p0 of the Cortex-X2 cpu and is still open.
ERRATA_X2_2002765	?=0

# Flag to apply erratum 2058056 workaround during reset. This erratum applies
# to revisions r0p0, r1p0, and r2p0 of the Cortex-X2 cpu and is still open.
ERRATA_X2_2058056	?=0

# Flag to apply erratum 2083908 workaround during reset. This erratum applies
# to revision r2p0 of the Cortex-X2 cpu and is still open.
ERRATA_X2_2083908	?=0

# Flag to apply erratum 2017096 workaround during reset. This erratum applies
# only to revisions r0p0, r1p0 and r2p0 of the Cortex-X2 cpu, it is fixed in
# r2p1.
ERRATA_X2_2017096	?=0

# Flag to apply erratum 2081180 workaround during reset. This erratum applies
# only to revisions r0p0, r1p0 and r2p0 of the Cortex-X2 cpu, it is fixed in
# r2p1.
ERRATA_X2_2081180	?=0

# Flag to apply erratum 2216384 workaround during reset. This erratum applies
# only to revisions r0p0, r1p0 and r2p0 of the Cortex-X2 cpu, it is fixed in
# r2p1.
ERRATA_X2_2216384	?=0

# Flag to apply erratum 2147715 workaround during reset. This erratum applies
# only to revision r2p0 of the Cortex-X2 cpu, it is fixed in r2p1.
ERRATA_X2_2147715	?=0

# Flag to apply erratum 1922240 workaround during reset. This erratum applies
# to revision r0p0 of the Cortex-A510 cpu and is fixed in r0p1.
ERRATA_A510_1922240	?=0

# Flag to apply erratum 2288014 workaround during reset. This erratum applies
# to revisions r0p0, r0p1, r0p2, r0p3 and r1p0 of the Cortex-A510 cpu and is
# fixed in r1p1.
ERRATA_A510_2288014	?=0

# Flag to apply erratum 2042739 workaround during reset. This erratum applies
# to revisions r0p0, r0p1 and r0p2 of the Cortex-A510 cpu and is fixed in r0p3.
ERRATA_A510_2042739	?=0

# Flag to apply erratum 2041909 workaround during reset. This erratum applies
# to revision r0p2 of the Cortex-A510 cpu and is fixed in r0p3. The issue is
# present in r0p0 and r0p1 but there is no workaround for those revisions.
ERRATA_A510_2041909	?=0

# Flag to apply erratum 2250311 workaround during reset. This erratum applies
# to revisions r0p0, r0p1, r0p2, r0p3 and r1p0, and is fixed in r1p1.
ERRATA_A510_2250311	?=0

# Flag to apply erratum 2218950 workaround during reset. This erratum applies
# to revisions r0p0, r0p1, r0p2, r0p3 and r1p0, and is fixed in r1p1.
ERRATA_A510_2218950	?=0

# Flag to apply erratum 2172148 workaround during reset. This erratum applies
# to revisions r0p0, r0p1, r0p2, r0p3 and r1p0, and is fixed in r1p1.
ERRATA_A510_2172148	?=0

# Flag to apply DSU erratum 798953. This erratum applies to DSUs revision r0p0.
# Applying the workaround results in higher DSU power consumption on idle.
ERRATA_DSU_798953	?=0

# Flag to apply DSU erratum 936184. This erratum applies to DSUs containing
# the ACP interface and revision < r2p0. Applying the workaround results in
# higher DSU power consumption on idle.
ERRATA_DSU_936184	?=0

# Flag to apply DSU erratum 2313941. This erratum applies to DSUs revisions
# r0p0, r1p0, r2p0, r2p1, r3p0, r3p1 and is still open. Applying the workaround
# results in higher DSU power consumption on idle.
ERRATA_DSU_2313941	?=0

# Process ERRATA_A9_794073 flag
$(eval $(call assert_boolean,ERRATA_A9_794073))
$(eval $(call add_define,ERRATA_A9_794073))

# Process ERRATA_A15_816470 flag
$(eval $(call assert_boolean,ERRATA_A15_816470))
$(eval $(call add_define,ERRATA_A15_816470))

# Process ERRATA_A15_827671 flag
$(eval $(call assert_boolean,ERRATA_A15_827671))
$(eval $(call add_define,ERRATA_A15_827671))

# Process ERRATA_A17_852421 flag
$(eval $(call assert_boolean,ERRATA_A17_852421))
$(eval $(call add_define,ERRATA_A17_852421))

# Process ERRATA_A17_852423 flag
$(eval $(call assert_boolean,ERRATA_A17_852423))
$(eval $(call add_define,ERRATA_A17_852423))

# Process ERRATA_A35_855472 flag
$(eval $(call assert_boolean,ERRATA_A35_855472))
$(eval $(call add_define,ERRATA_A35_855472))

# Process ERRATA_A53_819472 flag
$(eval $(call assert_boolean,ERRATA_A53_819472))
$(eval $(call add_define,ERRATA_A53_819472))

# Process ERRATA_A53_824069 flag
$(eval $(call assert_boolean,ERRATA_A53_824069))
$(eval $(call add_define,ERRATA_A53_824069))

# Process ERRATA_A53_826319 flag
$(eval $(call assert_boolean,ERRATA_A53_826319))
$(eval $(call add_define,ERRATA_A53_826319))

# Process ERRATA_A53_827319 flag
$(eval $(call assert_boolean,ERRATA_A53_827319))
$(eval $(call add_define,ERRATA_A53_827319))

# Process ERRATA_A53_835769 flag
$(eval $(call assert_boolean,ERRATA_A53_835769))
$(eval $(call add_define,ERRATA_A53_835769))

# Process ERRATA_A53_836870 flag
$(eval $(call assert_boolean,ERRATA_A53_836870))
$(eval $(call add_define,ERRATA_A53_836870))

# Process ERRATA_A53_843419 flag
$(eval $(call assert_boolean,ERRATA_A53_843419))
$(eval $(call add_define,ERRATA_A53_843419))

# Process ERRATA_A53_855873 flag
$(eval $(call assert_boolean,ERRATA_A53_855873))
$(eval $(call add_define,ERRATA_A53_855873))

# Process ERRATA_A53_1530924 flag
$(eval $(call assert_boolean,ERRATA_A53_1530924))
$(eval $(call add_define,ERRATA_A53_1530924))

# Process ERRATA_A55_768277 flag
$(eval $(call assert_boolean,ERRATA_A55_768277))
$(eval $(call add_define,ERRATA_A55_768277))

# Process ERRATA_A55_778703 flag
$(eval $(call assert_boolean,ERRATA_A55_778703))
$(eval $(call add_define,ERRATA_A55_778703))

# Process ERRATA_A55_798797 flag
$(eval $(call assert_boolean,ERRATA_A55_798797))
$(eval $(call add_define,ERRATA_A55_798797))

# Process ERRATA_A55_846532 flag
$(eval $(call assert_boolean,ERRATA_A55_846532))
$(eval $(call add_define,ERRATA_A55_846532))

# Process ERRATA_A55_903758 flag
$(eval $(call assert_boolean,ERRATA_A55_903758))
$(eval $(call add_define,ERRATA_A55_903758))

# Process ERRATA_A55_1221012 flag
$(eval $(call assert_boolean,ERRATA_A55_1221012))
$(eval $(call add_define,ERRATA_A55_1221012))

# Process ERRATA_A55_1530923 flag
$(eval $(call assert_boolean,ERRATA_A55_1530923))
$(eval $(call add_define,ERRATA_A55_1530923))

# Process ERRATA_A57_806969 flag
$(eval $(call assert_boolean,ERRATA_A57_806969))
$(eval $(call add_define,ERRATA_A57_806969))

# Process ERRATA_A57_813419 flag
$(eval $(call assert_boolean,ERRATA_A57_813419))
$(eval $(call add_define,ERRATA_A57_813419))

# Process ERRATA_A57_813420 flag
$(eval $(call assert_boolean,ERRATA_A57_813420))
$(eval $(call add_define,ERRATA_A57_813420))

# Process ERRATA_A57_814670 flag
$(eval $(call assert_boolean,ERRATA_A57_814670))
$(eval $(call add_define,ERRATA_A57_814670))

# Process ERRATA_A57_817169 flag
$(eval $(call assert_boolean,ERRATA_A57_817169))
$(eval $(call add_define,ERRATA_A57_817169))

# Process ERRATA_A57_826974 flag
$(eval $(call assert_boolean,ERRATA_A57_826974))
$(eval $(call add_define,ERRATA_A57_826974))

# Process ERRATA_A57_826977 flag
$(eval $(call assert_boolean,ERRATA_A57_826977))
$(eval $(call add_define,ERRATA_A57_826977))

# Process ERRATA_A57_828024 flag
$(eval $(call assert_boolean,ERRATA_A57_828024))
$(eval $(call add_define,ERRATA_A57_828024))

# Process ERRATA_A57_829520 flag
$(eval $(call assert_boolean,ERRATA_A57_829520))
$(eval $(call add_define,ERRATA_A57_829520))

# Process ERRATA_A57_833471 flag
$(eval $(call assert_boolean,ERRATA_A57_833471))
$(eval $(call add_define,ERRATA_A57_833471))

# Process ERRATA_A57_859972 flag
$(eval $(call assert_boolean,ERRATA_A57_859972))
$(eval $(call add_define,ERRATA_A57_859972))

# Process ERRATA_A57_1319537 flag
$(eval $(call assert_boolean,ERRATA_A57_1319537))
$(eval $(call add_define,ERRATA_A57_1319537))

# Process ERRATA_A72_859971 flag
$(eval $(call assert_boolean,ERRATA_A72_859971))
$(eval $(call add_define,ERRATA_A72_859971))

# Process ERRATA_A72_1319367 flag
$(eval $(call assert_boolean,ERRATA_A72_1319367))
$(eval $(call add_define,ERRATA_A72_1319367))

# Process ERRATA_A73_852427 flag
$(eval $(call assert_boolean,ERRATA_A73_852427))
$(eval $(call add_define,ERRATA_A73_852427))

# Process ERRATA_A73_855423 flag
$(eval $(call assert_boolean,ERRATA_A73_855423))
$(eval $(call add_define,ERRATA_A73_855423))

# Process ERRATA_A75_764081 flag
$(eval $(call assert_boolean,ERRATA_A75_764081))
$(eval $(call add_define,ERRATA_A75_764081))

# Process ERRATA_A75_790748 flag
$(eval $(call assert_boolean,ERRATA_A75_790748))
$(eval $(call add_define,ERRATA_A75_790748))

# Process ERRATA_A76_1073348 flag
$(eval $(call assert_boolean,ERRATA_A76_1073348))
$(eval $(call add_define,ERRATA_A76_1073348))

# Process ERRATA_A76_1130799 flag
$(eval $(call assert_boolean,ERRATA_A76_1130799))
$(eval $(call add_define,ERRATA_A76_1130799))

# Process ERRATA_A76_1220197 flag
$(eval $(call assert_boolean,ERRATA_A76_1220197))
$(eval $(call add_define,ERRATA_A76_1220197))

# Process ERRATA_A76_1257314 flag
$(eval $(call assert_boolean,ERRATA_A76_1257314))
$(eval $(call add_define,ERRATA_A76_1257314))

# Process ERRATA_A76_1262606 flag
$(eval $(call assert_boolean,ERRATA_A76_1262606))
$(eval $(call add_define,ERRATA_A76_1262606))

# Process ERRATA_A76_1262888 flag
$(eval $(call assert_boolean,ERRATA_A76_1262888))
$(eval $(call add_define,ERRATA_A76_1262888))

# Process ERRATA_A76_1275112 flag
$(eval $(call assert_boolean,ERRATA_A76_1275112))
$(eval $(call add_define,ERRATA_A76_1275112))

# Process ERRATA_A76_1286807 flag
$(eval $(call assert_boolean,ERRATA_A76_1286807))
$(eval $(call add_define,ERRATA_A76_1286807))

# Process ERRATA_A76_1791580 flag
$(eval $(call assert_boolean,ERRATA_A76_1791580))
$(eval $(call add_define,ERRATA_A76_1791580))

# Process ERRATA_A76_1165522 flag
$(eval $(call assert_boolean,ERRATA_A76_1165522))
$(eval $(call add_define,ERRATA_A76_1165522))

# Process ERRATA_A76_1868343 flag
$(eval $(call assert_boolean,ERRATA_A76_1868343))
$(eval $(call add_define,ERRATA_A76_1868343))

# Process ERRATA_A76_1946160 flag
$(eval $(call assert_boolean,ERRATA_A76_1946160))
$(eval $(call add_define,ERRATA_A76_1946160))

# Process ERRATA_A77_1508412 flag
$(eval $(call assert_boolean,ERRATA_A77_1508412))
$(eval $(call add_define,ERRATA_A77_1508412))

# Process ERRATA_A77_1925769 flag
$(eval $(call assert_boolean,ERRATA_A77_1925769))
$(eval $(call add_define,ERRATA_A77_1925769))

# Process ERRATA_A77_1946167 flag
$(eval $(call assert_boolean,ERRATA_A77_1946167))
$(eval $(call add_define,ERRATA_A77_1946167))

# Process ERRATA_A77_1791578 flag
$(eval $(call assert_boolean,ERRATA_A77_1791578))
$(eval $(call add_define,ERRATA_A77_1791578))

# Process ERRATA_A78_1688305 flag
$(eval $(call assert_boolean,ERRATA_A78_1688305))
$(eval $(call add_define,ERRATA_A78_1688305))

# Process ERRATA_A78_1941498 flag
$(eval $(call assert_boolean,ERRATA_A78_1941498))
$(eval $(call add_define,ERRATA_A78_1941498))

# Process ERRATA_A78_1951500 flag
$(eval $(call assert_boolean,ERRATA_A78_1951500))
$(eval $(call add_define,ERRATA_A78_1951500))

# Process ERRATA_A78_1821534 flag
$(eval $(call assert_boolean,ERRATA_A78_1821534))
$(eval $(call add_define,ERRATA_A78_1821534))

# Process ERRATA_A78_1952683 flag
$(eval $(call assert_boolean,ERRATA_A78_1952683))
$(eval $(call add_define,ERRATA_A78_1952683))

# Process ERRATA_A78_2132060 flag
$(eval $(call assert_boolean,ERRATA_A78_2132060))
$(eval $(call add_define,ERRATA_A78_2132060))

# Process ERRATA_A78_2242635 flag
$(eval $(call assert_boolean,ERRATA_A78_2242635))
$(eval $(call add_define,ERRATA_A78_2242635))

# Process ERRATA_A78_2376745 flag
$(eval $(call assert_boolean,ERRATA_A78_2376745))
$(eval $(call add_define,ERRATA_A78_2376745))

# Process ERRATA_A78_2395406 flag
$(eval $(call assert_boolean,ERRATA_A78_2395406))
$(eval $(call add_define,ERRATA_A78_2395406))

# Process ERRATA_A78_AE_1941500 flag
$(eval $(call assert_boolean,ERRATA_A78_AE_1941500))
$(eval $(call add_define,ERRATA_A78_AE_1941500))

# Process ERRATA_A78_AE_1951502 flag
$(eval $(call assert_boolean,ERRATA_A78_AE_1951502))
$(eval $(call add_define,ERRATA_A78_AE_1951502))

# Process ERRATA_A78_AE_2376748 flag
$(eval $(call assert_boolean,ERRATA_A78_AE_2376748))
$(eval $(call add_define,ERRATA_A78_AE_2376748))

# Process ERRATA_A78_AE_2395408 flag
$(eval $(call assert_boolean,ERRATA_A78_AE_2395408))
$(eval $(call add_define,ERRATA_A78_AE_2395408))

# Process ERRATA_X1_1821534 flag
$(eval $(call assert_boolean,ERRATA_X1_1821534))
$(eval $(call add_define,ERRATA_X1_1821534))

# Process ERRATA_X1_1688305 flag
$(eval $(call assert_boolean,ERRATA_X1_1688305))
$(eval $(call add_define,ERRATA_X1_1688305))

# Process ERRATA_X1_1827429 flag
$(eval $(call assert_boolean,ERRATA_X1_1827429))
$(eval $(call add_define,ERRATA_X1_1827429))

# Process ERRATA_N1_1043202 flag
$(eval $(call assert_boolean,ERRATA_N1_1043202))
$(eval $(call add_define,ERRATA_N1_1043202))

# Process ERRATA_N1_1073348 flag
$(eval $(call assert_boolean,ERRATA_N1_1073348))
$(eval $(call add_define,ERRATA_N1_1073348))

# Process ERRATA_N1_1130799 flag
$(eval $(call assert_boolean,ERRATA_N1_1130799))
$(eval $(call add_define,ERRATA_N1_1130799))

# Process ERRATA_N1_1165347 flag
$(eval $(call assert_boolean,ERRATA_N1_1165347))
$(eval $(call add_define,ERRATA_N1_1165347))

# Process ERRATA_N1_1207823 flag
$(eval $(call assert_boolean,ERRATA_N1_1207823))
$(eval $(call add_define,ERRATA_N1_1207823))

# Process ERRATA_N1_1220197 flag
$(eval $(call assert_boolean,ERRATA_N1_1220197))
$(eval $(call add_define,ERRATA_N1_1220197))

# Process ERRATA_N1_1257314 flag
$(eval $(call assert_boolean,ERRATA_N1_1257314))
$(eval $(call add_define,ERRATA_N1_1257314))

# Process ERRATA_N1_1262606 flag
$(eval $(call assert_boolean,ERRATA_N1_1262606))
$(eval $(call add_define,ERRATA_N1_1262606))

# Process ERRATA_N1_1262888 flag
$(eval $(call assert_boolean,ERRATA_N1_1262888))
$(eval $(call add_define,ERRATA_N1_1262888))

# Process ERRATA_N1_1275112 flag
$(eval $(call assert_boolean,ERRATA_N1_1275112))
$(eval $(call add_define,ERRATA_N1_1275112))

# Process ERRATA_N1_1315703 flag
$(eval $(call assert_boolean,ERRATA_N1_1315703))
$(eval $(call add_define,ERRATA_N1_1315703))

# Process ERRATA_N1_1542419 flag
$(eval $(call assert_boolean,ERRATA_N1_1542419))
$(eval $(call add_define,ERRATA_N1_1542419))

# Process ERRATA_N1_1868343 flag
$(eval $(call assert_boolean,ERRATA_N1_1868343))
$(eval $(call add_define,ERRATA_N1_1868343))

# Process ERRATA_N1_1946160 flag
$(eval $(call assert_boolean,ERRATA_N1_1946160))
$(eval $(call add_define,ERRATA_N1_1946160))

# Process ERRATA_N2_2002655 flag
$(eval $(call assert_boolean,ERRATA_N2_2002655))
$(eval $(call add_define,ERRATA_N2_2002655))

# Process ERRATA_V1_1774420 flag
$(eval $(call assert_boolean,ERRATA_V1_1774420))
$(eval $(call add_define,ERRATA_V1_1774420))

# Process ERRATA_V1_1791573 flag
$(eval $(call assert_boolean,ERRATA_V1_1791573))
$(eval $(call add_define,ERRATA_V1_1791573))

# Process ERRATA_V1_1852267 flag
$(eval $(call assert_boolean,ERRATA_V1_1852267))
$(eval $(call add_define,ERRATA_V1_1852267))

# Process ERRATA_V1_1925756 flag
$(eval $(call assert_boolean,ERRATA_V1_1925756))
$(eval $(call add_define,ERRATA_V1_1925756))

# Process ERRATA_V1_1940577 flag
$(eval $(call assert_boolean,ERRATA_V1_1940577))
$(eval $(call add_define,ERRATA_V1_1940577))

# Process ERRATA_V1_1966096 flag
$(eval $(call assert_boolean,ERRATA_V1_1966096))
$(eval $(call add_define,ERRATA_V1_1966096))

# Process ERRATA_V1_2139242 flag
$(eval $(call assert_boolean,ERRATA_V1_2139242))
$(eval $(call add_define,ERRATA_V1_2139242))

# Process ERRATA_V1_2108267 flag
$(eval $(call assert_boolean,ERRATA_V1_2108267))
$(eval $(call add_define,ERRATA_V1_2108267))

# Process ERRATA_V1_2216392 flag
$(eval $(call assert_boolean,ERRATA_V1_2216392))
$(eval $(call add_define,ERRATA_V1_2216392))

# Process ERRATA_A710_1987031 flag
$(eval $(call assert_boolean,ERRATA_A710_1987031))
$(eval $(call add_define,ERRATA_A710_1987031))

# Process ERRATA_A710_2081180 flag
$(eval $(call assert_boolean,ERRATA_A710_2081180))
$(eval $(call add_define,ERRATA_A710_2081180))

# Process ERRATA_A710_2083908 flag
$(eval $(call assert_boolean,ERRATA_A710_2083908))
$(eval $(call add_define,ERRATA_A710_2083908))

# Process ERRATA_A710_2058056 flag
$(eval $(call assert_boolean,ERRATA_A710_2058056))
$(eval $(call add_define,ERRATA_A710_2058056))

# Process ERRATA_A710_2055002 flag
$(eval $(call assert_boolean,ERRATA_A710_2055002))
$(eval $(call add_define,ERRATA_A710_2055002))

# Process ERRATA_A710_2017096 flag
$(eval $(call assert_boolean,ERRATA_A710_2017096))
$(eval $(call add_define,ERRATA_A710_2017096))

# Process ERRATA_A710_2267065 flag
$(eval $(call assert_boolean,ERRATA_A710_2267065))
$(eval $(call add_define,ERRATA_A710_2267065))

# Process ERRATA_A710_2136059 flag
$(eval $(call assert_boolean,ERRATA_A710_2136059))
$(eval $(call add_define,ERRATA_A710_2136059))

# Process ERRATA_A710_2282622 flag
$(eval $(call assert_boolean,ERRATA_A710_2282622))
$(eval $(call add_define,ERRATA_A710_2282622))

# Process ERRATA_A710_2008768 flag
$(eval $(call assert_boolean,ERRATA_A710_2008768))
$(eval $(call add_define,ERRATA_A710_2008768))

# Process ERRATA_N2_2067956 flag
$(eval $(call assert_boolean,ERRATA_N2_2067956))
$(eval $(call add_define,ERRATA_N2_2067956))

# Process ERRATA_N2_2025414 flag
$(eval $(call assert_boolean,ERRATA_N2_2025414))
$(eval $(call add_define,ERRATA_N2_2025414))

# Process ERRATA_N2_2189731 flag
$(eval $(call assert_boolean,ERRATA_N2_2189731))
$(eval $(call add_define,ERRATA_N2_2189731))

# Process ERRATA_N2_2138956 flag
$(eval $(call assert_boolean,ERRATA_N2_2138956))
$(eval $(call add_define,ERRATA_N2_2138956))

# Process ERRATA_N2_2138953 flag
$(eval $(call assert_boolean,ERRATA_N2_2138953))
$(eval $(call add_define,ERRATA_N2_2138953))

# Process ERRATA_N2_2242415 flag
$(eval $(call assert_boolean,ERRATA_N2_2242415))
$(eval $(call add_define,ERRATA_N2_2242415))

# Process ERRATA_N2_2138958 flag
$(eval $(call assert_boolean,ERRATA_N2_2138958))
$(eval $(call add_define,ERRATA_N2_2138958))

# Process ERRATA_N2_2242400 flag
$(eval $(call assert_boolean,ERRATA_N2_2242400))
$(eval $(call add_define,ERRATA_N2_2242400))

# Process ERRATA_N2_2280757 flag
$(eval $(call assert_boolean,ERRATA_N2_2280757))
$(eval $(call add_define,ERRATA_N2_2280757))

# Process ERRATA_X2_2002765 flag
$(eval $(call assert_boolean,ERRATA_X2_2002765))
$(eval $(call add_define,ERRATA_X2_2002765))

# Process ERRATA_X2_2058056 flag
$(eval $(call assert_boolean,ERRATA_X2_2058056))
$(eval $(call add_define,ERRATA_X2_2058056))

# Process ERRATA_X2_2083908 flag
$(eval $(call assert_boolean,ERRATA_X2_2083908))
$(eval $(call add_define,ERRATA_X2_2083908))

# Process ERRATA_X2_2017096 flag
$(eval $(call assert_boolean,ERRATA_X2_2017096))
$(eval $(call add_define,ERRATA_X2_2017096))

# Process ERRATA_X2_2081180 flag
$(eval $(call assert_boolean,ERRATA_X2_2081180))
$(eval $(call add_define,ERRATA_X2_2081180))

# Process ERRATA_X2_2216384 flag
$(eval $(call assert_boolean,ERRATA_X2_2216384))
$(eval $(call add_define,ERRATA_X2_2216384))

# Process ERRATA_X2_2147715 flag
$(eval $(call assert_boolean,ERRATA_X2_2147715))
$(eval $(call add_define,ERRATA_X2_2147715))

# Process ERRATA_A510_1922240 flag
$(eval $(call assert_boolean,ERRATA_A510_1922240))
$(eval $(call add_define,ERRATA_A510_1922240))

# Process ERRATA_A510_2288014 flag
$(eval $(call assert_boolean,ERRATA_A510_2288014))
$(eval $(call add_define,ERRATA_A510_2288014))

# Process ERRATA_A510_2042739 flag
$(eval $(call assert_boolean,ERRATA_A510_2042739))
$(eval $(call add_define,ERRATA_A510_2042739))

# Process ERRATA_A510_2041909 flag
$(eval $(call assert_boolean,ERRATA_A510_2041909))
$(eval $(call add_define,ERRATA_A510_2041909))

# Process ERRATA_A510_2250311 flag
$(eval $(call assert_boolean,ERRATA_A510_2250311))
$(eval $(call add_define,ERRATA_A510_2250311))

# Process ERRATA_A510_2218950 flag
$(eval $(call assert_boolean,ERRATA_A510_2218950))
$(eval $(call add_define,ERRATA_A510_2218950))

# Process ERRATA_A510_2172148 flag
$(eval $(call assert_boolean,ERRATA_A510_2172148))
$(eval $(call add_define,ERRATA_A510_2172148))

# Process ERRATA_DSU_798953 flag
$(eval $(call assert_boolean,ERRATA_DSU_798953))
$(eval $(call add_define,ERRATA_DSU_798953))

# Process ERRATA_DSU_936184 flag
$(eval $(call assert_boolean,ERRATA_DSU_936184))
$(eval $(call add_define,ERRATA_DSU_936184))

# Process ERRATA_DSU_2313941 flag
$(eval $(call assert_boolean,ERRATA_DSU_2313941))
$(eval $(call add_define,ERRATA_DSU_2313941))

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

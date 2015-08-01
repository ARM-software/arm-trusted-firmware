Tegra SoCs - Overview
======================

* T210
-------

T210 has Quad ARM® Cortex®-A57 cores in a switched configuration with a
companion set of quad ARM Cortex-A53 cores. The Cortex-A57 and A53 cores
support ARMv8, executing both 64-bit Aarch64 code, and 32-bit Aarch32 code
including legacy ARMv7 applications. The Cortex-A57 processors each have
48 KB Instruction and 32 KB Data Level 1 caches; and have a 2 MB shared
Level 2 unified cache. The Cortex-A53 processors each have 32 KB Instruction
and 32 KB Data Level 1 caches; and have a 512 KB shared Level 2 unified cache.

* T132
-------

Denver is NVIDIA's own custom-designed, 64-bit, dual-core CPU which is
fully ARMv8 architecture compatible.  Each of the two Denver cores
implements a 7-way superscalar microarchitecture (up to 7 concurrent
micro-ops can be executed per clock), and includes a 128KB 4-way L1
instruction cache, a 64KB 4-way L1 data cache, and a 2MB 16-way L2
cache, which services both cores.

Denver implements an innovative process called Dynamic Code Optimization,
which optimizes frequently used software routines at runtime into dense,
highly tuned microcode-equivalent routines. These are stored in a
dedicated, 128MB main-memory-based optimization cache. After being read
into the instruction cache, the optimized micro-ops are executed,
re-fetched and executed from the instruction cache as long as needed and
capacity allows.

Effectively, this reduces the need to re-optimize the software routines.
Instead of using hardware to extract the instruction-level parallelism
(ILP) inherent in the code, Denver extracts the ILP once via software
techniques, and then executes those routines repeatedly, thus amortizing
the cost of ILP extraction over the many execution instances.

Denver also features new low latency power-state transitions, in addition
to extensive power-gating and dynamic voltage and clock scaling based on
workloads.

Directory structure
====================

* plat/nvidia/tegra/common - Common code for all Tegra SoCs
* plat/nvidia/tegra/soc/txxx - Chip specific code

Trusted OS dispatcher
=====================
Tegra supports multiple Trusted OS', Trusted Little Kernel (TLK) being one of
them. In order to include the 'tlkd' dispatcher in the image, pass 'SPD=tlkd'
on the command line while preparing a bl31 image. This allows other Trusted OS
vendors to use the upstream code and include their dispatchers in the image
without changing any makefiles.

Preparing the BL31 image to run on Tegra SoCs
===================================================
'CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- make PLAT=tegra \
TARGET_SOC=<target-soc e.g. t210|t132> SPD=<dispatcher e.g. tlkd> bl31'

Platforms wanting to use different TZDRAM_BASE, can add 'TZDRAM_BASE=<value>'
to the build command line.

Power Management
================
The PSCI implementation expects each platform to expose the 'power state'
parameter to be used during the 'SYSTEM SUSPEND' call. The state-id field
is implementation defined on Tegra SoCs and is preferably defined by
tegra_def.h.

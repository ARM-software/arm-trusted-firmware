Tegra-T210 Overview
====================

T210 has Quad ARM® Cortex®-A57 cores in a switched configuration with a
companion set of quad ARM Cortex-A53 cores. The Cortex-A57 and A53 cores
support ARMv8, executing both 64-bit Aarch64 code, and 32-bit Aarch32 code
including legacy ARMv7 applications. The Cortex-A57 processors each have
48 KB Instruction and 32 KB Data Level 1 caches; and have a 2 MB shared
Level 2 unified cache. The Cortex-A53 processors each have 32 KB Instruction
and 32 KB Data Level 1 caches; and have a 512 KB shared Level 2 unified cache.

Directory structure
====================

* plat/nvidia/tegra/common - Common code for all Tegra SoCs
* plat/nvidia/tegra/soc/txxx - Chip specific code

Preparing the BL31 image to run on Tegra SoCs
===================================================
CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- make PLAT=tegra \
TARGET_SOC=<target-soc e.g. t210> all

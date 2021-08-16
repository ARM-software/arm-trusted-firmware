NVIDIA Tegra
============

-  .. rubric:: T194
      :name: t194

T194 has eight NVIDIA Carmel CPU cores in a coherent multi-processor
configuration. The Carmel cores support the ARM Architecture version 8.2,
executing both 64-bit AArch64 code, and 32-bit AArch32 code. The Carmel
processors are organized as four dual-core clusters, where each cluster has
a dedicated 2 MiB Level-2 unified cache. A high speed coherency fabric connects
these processor complexes and allows heterogeneous multi-processing with all
eight cores if required.

-  .. rubric:: T186
      :name: t186

The NVIDIA® Parker (T186) series system-on-chip (SoC) delivers a heterogeneous
multi-processing (HMP) solution designed to optimize performance and
efficiency.

T186 has Dual NVIDIA Denver2 ARM® CPU cores, plus Quad ARM Cortex®-A57 cores,
in a coherent multiprocessor configuration. The Denver 2 and Cortex-A57 cores
support ARMv8, executing both 64-bit Aarch64 code, and 32-bit Aarch32 code
including legacy ARMv7 applications. The Denver 2 processors each have 128 KB
Instruction and 64 KB Data Level 1 caches; and have a 2MB shared Level 2
unified cache. The Cortex-A57 processors each have 48 KB Instruction and 32 KB
Data Level 1 caches; and also have a 2 MB shared Level 2 unified cache. A
high speed coherency fabric connects these two processor complexes and allows
heterogeneous multi-processing with all six cores if required.

Denver is NVIDIA's own custom-designed, 64-bit, dual-core CPU which is
fully Armv8-A architecture compatible. Each of the two Denver cores
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

-  .. rubric:: T210
      :name: t210

T210 has Quad Arm® Cortex®-A57 cores in a switched configuration with a
companion set of quad Arm Cortex-A53 cores. The Cortex-A57 and A53 cores
support Armv8-A, executing both 64-bit Aarch64 code, and 32-bit Aarch32 code
including legacy Armv7-A applications. The Cortex-A57 processors each have
48 KB Instruction and 32 KB Data Level 1 caches; and have a 2 MB shared
Level 2 unified cache. The Cortex-A53 processors each have 32 KB Instruction
and 32 KB Data Level 1 caches; and have a 512 KB shared Level 2 unified cache.

Directory structure
-------------------

-  plat/nvidia/tegra/common - Common code for all Tegra SoCs
-  plat/nvidia/tegra/soc/txxx - Chip specific code

Trusted OS dispatcher
---------------------

Tegra supports multiple Trusted OS'.

- Trusted Little Kernel (TLK): In order to include the 'tlkd' dispatcher in
  the image, pass 'SPD=tlkd' on the command line while preparing a bl31 image.
- Trusty: In order to include the 'trusty' dispatcher in the image, pass
  'SPD=trusty' on the command line while preparing a bl31 image.

This allows other Trusted OS vendors to use the upstream code and include
their dispatchers in the image without changing any makefiles.

These are the supported Trusted OS' by Tegra platforms.

- Tegra210: TLK and Trusty
- Tegra186: Trusty
- Tegra194: Trusty

Scatter files
-------------

Tegra platforms currently support scatter files and ld.S scripts. The scatter
files help support ARMLINK linker to generate BL31 binaries. For now, there
exists a common scatter file, plat/nvidia/tegra/scat/bl31.scat, for all Tegra
SoCs. The `LINKER` build variable needs to point to the ARMLINK binary for
the scatter file to be used. Tegra platforms have verified BL31 image generation
with ARMCLANG (compilation) and ARMLINK (linking) for the Tegra186 platforms.

Preparing the BL31 image to run on Tegra SoCs
---------------------------------------------

.. code:: shell

    CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- make PLAT=tegra \
    TARGET_SOC=<target-soc e.g. t194|t186|t210> SPD=<dispatcher e.g. trusty|tlkd>
    bl31

Platforms wanting to use different TZDRAM\_BASE, can add ``TZDRAM_BASE=<value>``
to the build command line.

The Tegra platform code expects a pointer to the following platform specific
structure via 'x1' register from the BL2 layer which is used by the
bl31\_early\_platform\_setup() handler to extract the TZDRAM carveout base and
size for loading the Trusted OS and the UART port ID to be used. The Tegra
memory controller driver programs this base/size in order to restrict NS
accesses.

typedef struct plat\_params\_from\_bl2 {
/\* TZ memory size */
uint64\_t tzdram\_size;
/* TZ memory base */
uint64\_t tzdram\_base;
/* UART port ID \*/
int uart\_id;
/* L2 ECC parity protection disable flag \*/
int l2\_ecc\_parity\_prot\_dis;
/* SHMEM base address for storing the boot logs \*/
uint64\_t boot\_profiler\_shmem\_base;
} plat\_params\_from\_bl2\_t;

Power Management
----------------

The PSCI implementation expects each platform to expose the 'power state'
parameter to be used during the 'SYSTEM SUSPEND' call. The state-id field
is implementation defined on Tegra SoCs and is preferably defined by
tegra\_def.h.

Tegra configs
-------------

-  'tegra\_enable\_l2\_ecc\_parity\_prot': This flag enables the L2 ECC and Parity
   Protection bit, for Arm Cortex-A57 CPUs, during CPU boot. This flag will
   be enabled by Tegrs SoCs during 'Cluster power up' or 'System Suspend' exit.

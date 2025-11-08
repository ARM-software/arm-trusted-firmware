NUMA-Aware PER-CPU Framework
============================

.. contents::
   :local:
   :depth: 2

Introduction
============

Modern System designs increasingly adopt multi-node architectures, where the
system is divided into multiple topological units such as chiplet, socket, or
any other isolated unit of compute and memory. Each node typically has its own
local memory, and CPUs within a node can access this memory with lower latency
compared to memory located on remote nodes. In TF-A's current implementation,
per-cpu data (such as PSCI context, SPM context, etc.) is stored in a global
array or contiguous region, usually located in the memory of a single node. This
approach introduces two key issues in multi-node systems:

- **Storage Constraints:** As systems scale to include more CPUs and nodes, this
  centralized allocation becomes a bottleneck.  The memory capacity of a single
  node may be insufficient to hold per-cpu data for all CPUs. This constraint
  limits scalability in systems where each node has limited local memory.

  .. figure:: ../resources/diagrams/per_cpu_numa_numa_disabled.png
     :alt: Storage Problem in Multi-node Systems
     :align: right
     :width: 500px

  *Figure: Typical BL31/BL32 binary storage in local memory*

- **Non-Uniform Memory Access (NUMA) Latency:** In multi-node systems, memory
  access across nodes incurs additional latency due to interconnect traversal.
  When per-cpu data is centralized on a single node, CPUs on remote nodes must
  access their per-cpu data via the interconnect, leading to increased latency
  for frequent operations like context switching, exception handling, and crash
  reporting. This violates NUMA design principles, where data locality is
  critical to achieving performance and scalability.

To address these challenges, the NUMA-Aware per-cpu framework has been
introduced. This framework optimizes the allocation and access of per-cpu
objects by allowing platforms to place them in nodes with least access latency.

Design
======

To address these architectural challenges, TF-A introduces the NUMA-aware per
cpu framework. This framework is designed to give platforms the opportunity to
allocate per-cpu data as close to the calling CPU as possible, ideally within
the same NUMA node, thereby reducing access latency and improving overall memory
scalability.

The framework provides standardized interfaces and mechanisms for
**allocating**, **defining**, and **accessing** per-cpu data in a NUMA-aware
environment. This ensures portability and maintainability across different
platforms while optimizing for performance in multi-node systems.

.per_cpu Section
----------------

A dedicated .per_cpu section to **allocate** per-cpu global variables, ensuring
that these objects are allocated in the local memory of each NUMA node. Figure
illustrates how per-cpu objects are allocated in the local memory of their
respective nodes. The necessary linker modifications to support this layout are
shown in the accompanying snippet.

.. figure:: ../resources/diagrams/per_cpu_numa_numa_enabled.png
   :alt: NUMA-Aware PER-CPU Framework Overview
   :align: center
   :width: 2000px

*Figure: BL31/BL32 binary storage in local memory of per node when per-cpu NUMA
framework is enabled*

.. code-block:: text

	/* The .per_cpu section gets initialised to 0 at runtime. */	\
	.per_cpu (NOLOAD) : ALIGN(CACHE_WRITEBACK_GRANULE) {		\
		__PER_CPU_START__ = .;					\
		__PER_CPU_UNIT_START__ = .;				\
		*(SORT_BY_ALIGNMENT(.per_cpu*))				\
		__PER_CPU_UNIT_UNALIGNED_END_UNIT__ = .;		\
		. = ALIGN(CACHE_WRITEBACK_GRANULE);			\
		__PER_CPU_UNIT_END__ = .;				\
		__PER_CPU_UNIT_SECTION_SIZE__ =				\
		ABSOLUTE(__PER_CPU_UNIT_END__ - __PER_CPU_UNIT_START__);\
		. = . + (PER_CPU_NODE_CORE_COUNT - 1) *			\
		__PER_CPU_UNIT_SECTION_SIZE__;				\
		__PER_CPU_END__ = .;					\
	}

The newly introduced linker changes also addresses a common performance issue in
modern multi-cpu systems—**cache thrashing**.

A performance issue known as **cache thrashing** arises when multiple CPUs
access different addresses that are on the same cache line. Although the
accessed variables may be logically independent, their proximity in memory can
result in repeated cache invalidations and reloads. This is because cache
coherency mechanisms operate at the granularity of cache lines (typically 64
bytes). If two CPUs attempt to write to two different addresses that fall within
the same cache line, the cache line is bounced back and forth between the cores,
incurring unnecessary overhead.

.. figure:: ../resources/diagrams/per_cpu_numa_cache_thrashing.png
   :alt: Illustration of Cache Thrashing from Per-CPU Data Collisions
   :align: center
   :width: 600px

*Figure: Two processors modifying different variables placed too closely in
memory, leading to cache thrashing*

To eliminate cache thrashing, this framework employs **linker-script-based
alignment**. It ensures:

- Placing all per-cpu variables into a **dedicated, aligned** section:
  `.per_cpu`
- Aligning that section using the cache granularity size
  (`CACHE_WRITEBACK_GRANULE`)

Definer Interfaces
------------------

The NUMA-Aware PER-CPU framework provides set of macros to define and declare
per-cpu objects efficiently in multi-node systems.

- **PER_CPU_DECLARE**

  Declares an external per-cpu object.

  .. code-block:: c

      #define PER_CPU_DECLARE(TYPE, NAME) \
          extern typeof(TYPE) NAME

- **PER_CPU_DEFINE**

  Defines a per-cpu object and places it in the `.per_cpu` section.

  .. code-block:: c

      #define PER_CPU_DEFINE(TYPE, NAME) \
          typeof(TYPE) NAME \
          __section(PER_CPU_SECTION_NAME)

Accessor Interfaces
-------------------

The NUMA-Aware PER-CPU framework provides set of macros to access per-cpu
objects efficiently in multi-node systems.

- **PER_CPU_BY_INDEX(NAME, CPU)**
  Returns a pointer to the per-cpu object `NAME` for the specified CPU.

  .. code-block:: c

      #define PER_CPU_BY_INDEX(NAME, CPU)			\
          ((__typeof__(&NAME))					\
          (per_cpu_by_index_compute((CPU), (void *)&(NAME))))

- **PER_CPU_CUR(NAME)**
  Returns a pointer to the per-cpu object `NAME` for the current CPU.

  .. code-block:: c

      #define PER_CPU_CUR(NAME) 			\
      ((__typeof__(&(NAME)))				\
      (per_cpu_cur_compute((void *)&(NAME))))

For use in assembly routines, a corresponding macro version is provided:

.. code-block:: text

   .macro  per_cpu_cur label, dst=x0, clobber=x1
       /* Safety checks */
       .ifc \dst,\clobber
       .error "per_cpu_cur: dst and clobber must be different"
       .endif

       /* dst = absolute address of label */
       adr_l	\dst, \label

       /* clobber = absolute address of __PER_CPU_START__ */
       	adr_l	\clobber, __PER_CPU_START__

       /* dst = (label - __PER_CPU_START__) */
       sub     \dst, \dst, \clobber

       /* clobber = per-cpu base (TPIDR_EL3) */
       mrs     \clobber, tpidr_el3

       /* dst = base + offset */
       add     \dst, \clobber, \dst
   .endm


The accessor interfaces take advantage of using `tpidr_el3` system register
(Thread ID Register at EL3). It stores the **base address of the current CPU's
`.per_cpu` section**. By setting up this register during early CPU
initialization (e.g., in the el3_entrypoint_common path), TF-A can avoid
repeated calculations or memory lookups when accessing per-cpu objects.

Instead of computing the per-cpu address dynamically using platform-level
functions (which could involve node discovery, offset arithmetic, and memory
dereferencing), TF-A can simply:

- Read `tpidr_el3` to get the base address of the current CPU's per-cpu data.
- Add the relative offset of the desired object within the `.per_cpu` section.
- Access the target object directly using this computed address.

This strategy significantly reduces access time by replacing a potentially
expensive memory access path with a single register read and offset addition. It
improves performance—particularly in hot paths like PSCI operations and context
switching taking advantage of fast-access system registers instead of traversing
interconnects.

Usage Example
=============

Platform Responsibilities
-------------------------

To integrate the NUMA-Aware PER-CPU Framework into a platform, the following
steps must be taken:

1. Enable the Framework
-------------------------

Set the PLATFORM_NODE_COUNT to greater than 1 (>=2) in the platform
makefile to enable NUMA-aware per-cpu support:

.. code-block:: text

    PLATFORM_NODE_COUNT := 1 (>=2 for enabling NUMA-aware per-cpu support)

Platforms that are not multi-node needn't do anything as
PLATFORM_NODE_COUNT = 1 (NODE COUNT) by default.
In the case of 32-bit Images such as BL32 sp_min NUMA framework is not supported.

2. Provide Per-CPU Section Base Address Table
---------------------------------------------

Declare and initialize an array holding the base address of the `.per_cpu`
section for each node:

.. code-block:: c

    const uintptr_t per_cpu_nodes_base[] = {
        /* Base addresses per node (platform-specific) */
    };

This array allows efficient mapping from logical CPU IDs to physical memory
regions in multi-node systems.  Note: This is one example of how platforms can
define .per_cpu section base addresses.  Platforms are free to determine and
provide these addresses using other methods, such as device tree parsing,
platform-specific tables, or dynamic discovery logic. It is important to note
that the platform defined regions for holding remote per-cpu section should have
a page aligned base and size for page table mapping via the xlat library. This
is simply due to the fact that xlat requires page aligned address and size for
mapping an entry. per-cpu section by itself requires only CACHE_WRITEBACK_GRANULE
alignment for its base.

3. Implement Required Platform Hooks
------------------------------------

Provide the following platform-specific functions:

- **`plat_per_cpu_base(int cpu)`**
  Returns the base address of the `.per_cpu` section for the specified CPU.

- **`plat_per_cpu_node_base(void)`**
  Returns the node base address of the `.per_cpu` section.

- **`plat_per_cpu_dcache_clean(void)`**
  Cleans the entire per-cpu section from the data cache. This ensures that any
  modifications made to per-cpu data are written back to memory, making them
  visible to other CPUs or system components that may access this memory. It is
  especially important on platforms that do not support hardware managed
  coherency early in the boot.

References
==========

- Original Presentation: https://www.trustedfirmware.org/docs/NUMA-aware-PER-CPU-framework-18Jul24.pdf

--------------

*Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.*

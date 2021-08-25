Granule Protection Tables Library
=================================

This document describes the design of the granule protection tables (GPT)
library used by Trusted Firmware-A (TF-A). This library provides the APIs needed
to initialize the GPTs based on a data structure containing information about
the systems memory layout, configure the system registers to enable granule
protection checks based on these tables, and transition granules between
different PAS (physical address spaces) at runtime.

Arm CCA adds two new security states for a total of four: root, realm, secure, and
non-secure. In addition to new security states, corresponding physical address
spaces have been added to control memory access for each state. The PAS access
allowed to each security state can be seen in the table below.

.. list-table:: Security states and PAS access rights
   :widths: 25 25 25 25 25
   :header-rows: 1

   * -
     - Root state
     - Realm state
     - Secure state
     - Non-secure state
   * - Root PAS
     - yes
     - no
     - no
     - no
   * - Realm PAS
     - yes
     - yes
     - no
     - no
   * - Secure PAS
     - yes
     - no
     - yes
     - no
   * - Non-secure PAS
     - yes
     - yes
     - yes
     - yes

The GPT can function as either a 1 level or 2 level lookup depending on how a
PAS region is configured. The first step is the level 0 table, each entry in the
level 0 table controls access to a relatively large region in memory (block
descriptor), and the entire region can belong to a single PAS when a one step
mapping is used, or a level 0 entry can link to a level 1 table where relatively
small regions (granules) of memory can be assigned to different PAS with a 2
step mapping. The type of mapping used for each PAS is determined by the user
when setting up the configuration structure.

Design Concepts and Interfaces
------------------------------

This section covers some important concepts and data structures used in the GPT
library.

There are three main parameters that determine how the tables are organized and
function: the PPS (protected physical space) which is the total amount of
protected physical address space in the system, PGS (physical granule size)
which is how large each level 1 granule is, and L0GPTSZ (level 0 GPT size) which
determines how much physical memory is governed by each level 0 entry. A granule
is the smallest unit of memory that can be independently assigned to a PAS.

L0GPTSZ is determined by the hardware and is read from the GPCCR_EL3 register.
PPS and PGS are passed into the APIs at runtime and can be determined in
whatever way is best for a given platform, either through some algorithm or hard
coded in the firmware.

GPT setup is split into two parts: table creation and runtime initialization. In
the table creation step, a data structure containing information about the
desired PAS regions is passed into the library which validates the mappings,
creates the tables in memory, and enables granule protection checks. In the
runtime initialization step, the runtime firmware locates the existing tables in
memory using the GPT register configuration and saves important data to a
structure used by the granule transition service which will be covered more
below.

In the reference implementation for FVP models, you can find an example of PAS
region definitions in the file ``include/plat/arm/common/arm_pas_def.h``. Table
creation API calls can be found in ``plat/arm/common/arm_bl2_setup.c`` and
runtime initialization API calls can be seen in
``plat/arm/common/arm_bl31_setup.c``.

Defining PAS regions
~~~~~~~~~~~~~~~~~~~~

A ``pas_region_t`` structure is a way to represent a physical address space and
its attributes that can be used by the GPT library to initialize the tables.

This structure is composed of the following:

#. The base physical address
#. The region size
#. The desired attributes of this memory region (mapping type, PAS type)

See the ``pas_region_t`` type in ``include/lib/gpt_rme/gpt_rme.h``.

The programmer should provide the API with an array containing ``pas_region_t``
structures, then the library will check the desired memory access layout for
validity and create tables to implement it.

``pas_region_t`` is a public type, however it is recommended that the macros
``GPT_MAP_REGION_BLOCK`` and ``GPT_MAP_REGION_GRANULE`` be used to populate
these structures instead of doing it manually to reduce the risk of future
compatibility issues. These macros take the base physical address, region size,
and PAS type as arguments to generate the pas_region_t structure. As the names
imply, ``GPT_MAP_REGION_BLOCK`` creates a region using only L0 mapping while
``GPT_MAP_REGION_GRANULE`` creates a region using L0 and L1 mappings.

Level 0 and Level 1 Tables
~~~~~~~~~~~~~~~~~~~~~~~~~~

The GPT initialization APIs require memory to be passed in for the tables to be
constructed, ``gpt_init_l0_tables`` takes a memory address and size for building
the level 0 tables and ``gpt_init_pas_l1_tables`` takes an address and size for
building the level 1 tables which are linked from level 0 descriptors. The
tables should have PAS type ``GPT_GPI_ROOT`` and a typical system might place
its level 0 table in SRAM and its level 1 table(s) in DRAM.

Granule Transition Service
~~~~~~~~~~~~~~~~~~~~~~~~~~

The Granule Transition Service allows memory mapped with GPT_MAP_REGION_GRANULE
ownership to be changed using SMC calls. Non-secure granules can be transitioned
to either realm or secure space, and realm and secure granules can be
transitioned back to non-secure. This library only allows memory mapped as
granules to be transitioned, memory mapped as blocks have their GPIs fixed after
table creation.

Library APIs
------------

The public APIs and types can be found in ``include/lib/gpt_rme/gpt_rme.h`` and this
section is intended to provide additional details and clarifications.

To create the GPTs and enable granule protection checks the APIs need to be
called in the correct order and at the correct time during the system boot
process.

#. Firmware must enable the MMU.
#. Firmware must call ``gpt_init_l0_tables`` to initialize the level 0 tables to
   a default state, that is, initializing all of the L0 descriptors to allow all
   accesses to all memory. The PPS is provided to this function as an argument.
#. DDR discovery and initialization by the system, the discovered DDR region(s)
   are then added to the L1 PAS regions to be initialized in the next step and
   used by the GTSI at runtime.
#. Firmware must call ``gpt_init_pas_l1_tables`` with a pointer to an array of
   ``pas_region_t`` structures containing the desired memory access layout. The
   PGS is provided to this function as an argument.
#. Firmware must call ``gpt_enable`` to enable granule protection checks by
   setting the correct register values.
#. In systems that make use of the granule transition service, runtime
   firmware must call ``gpt_runtime_init`` to set up the data structures needed
   by the GTSI to find the tables and transition granules between PAS types.

API Constraints
~~~~~~~~~~~~~~~

The values allowed by the API for PPS and PGS are enumerated types
defined in the file ``include/lib/gpt_rme/gpt_rme.h``.

Allowable values for PPS along with their corresponding size.

* ``GPCCR_PPS_4GB`` (4GB protected space, 0x100000000 bytes)
* ``GPCCR_PPS_64GB`` (64GB protected space, 0x1000000000 bytes)
* ``GPCCR_PPS_1TB`` (1TB protected space, 0x10000000000 bytes)
* ``GPCCR_PPS_4TB`` (4TB protected space, 0x40000000000 bytes)
* ``GPCCR_PPS_16TB`` (16TB protected space, 0x100000000000 bytes)
* ``GPCCR_PPS_256TB`` (256TB protected space, 0x1000000000000 bytes)
* ``GPCCR_PPS_4PB`` (4PB protected space, 0x10000000000000 bytes)

Allowable values for PGS along with their corresponding size.

* ``GPCCR_PGS_4K`` (4KB granules, 0x1000 bytes)
* ``GPCCR_PGS_16K`` (16KB granules, 0x4000 bytes)
* ``GPCCR_PGS_64K`` (64KB granules, 0x10000 bytes)

Allowable values for L0GPTSZ along with the corresponding size.

* ``GPCCR_L0GPTSZ_30BITS`` (1GB regions, 0x40000000 bytes)
* ``GPCCR_L0GPTSZ_34BITS`` (16GB regions, 0x400000000 bytes)
* ``GPCCR_L0GPTSZ_36BITS`` (64GB regions, 0x1000000000 bytes)
* ``GPCCR_L0GPTSZ_39BITS`` (512GB regions, 0x8000000000 bytes)

Note that the value of the PPS, PGS, and L0GPTSZ definitions is an encoded value
corresponding to the size, not the size itself. The decoded hex representations
of the sizes have been provided for convenience.

The L0 table memory has some constraints that must be taken into account.

* The L0 table must be aligned to either the table size or 4096 bytes, whichever
  is greater. L0 table size is the total protected space (PPS) divided by the
  size of each L0 region (L0GPTSZ) multiplied by the size of each L0 descriptor
  (8 bytes). ((PPS / L0GPTSZ) * 8)
* The L0 memory size must be greater than or equal to the table size.
* The L0 memory must fall within a PAS of type GPT_GPI_ROOT.

The L1 memory also has some constraints.

* The L1 tables must be aligned to their size. The size of each L1 table is the
  size of each L0 region (L0GPTSZ) divided by the granule size (PGS) divided by
  the granules controlled in each byte (2). ((L0GPTSZ / PGS) / 2)
* There must be enough L1 memory supplied to build all requested L1 tables.
* The L1 memory must fall within a PAS of type GPT_GPI_ROOT.

If an invalid combination of parameters is supplied, the APIs will print an
error message and return a negative value. The return values of APIs should be
checked to ensure successful configuration.

Sample Calculation for L0 memory size and alignment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Let PPS=GPCCR_PPS_4GB and L0GPTSZ=GPCCR_L0GPTSZ_30BITS

We can find the total L0 table size with ((PPS / L0GPTSZ) * 8)

Substitute values to get this: ((0x100000000 / 0x40000000) * 8)

And solve to get 32 bytes. In this case, 4096 is greater than 32, so the L0
tables must be aligned to 4096 bytes.

Sample calculation for L1 table size and alignment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Let PGS=GPCCR_PGS_4K and L0GPTSZ=GPCCR_L0GPTSZ_30BITS

We can find the size of each L1 table with ((L0GPTSZ / PGS) / 2).

Substitute values: ((0x40000000 / 0x1000) / 2)

And solve to get 0x20000 bytes per L1 table.

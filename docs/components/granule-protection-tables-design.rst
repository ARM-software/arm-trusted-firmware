Granule Protection Tables Library
=================================

This document describes the design of the Granule Protection Tables (GPT)
library used by Trusted Firmware-A (TF-A). This library provides the APIs needed
to transition granules between different PAS (physical address spaces) at
runtime.

Design Concepts and Interfaces
------------------------------

In the runtime initialization step, the runtime firmware locates the existing
tables in memory using the GPT register configuration and saves important data
to a structure used by the granule transition service which will be covered more
below.

In the reference implementation for FVP models, you can find an example runtime
initialization API call ``gpt_runtime_init`` in
``plat/arm/common/arm_bl31_setup.c``.

During the table creation time, the GPT lib opportunistically fuses contiguous
GPT L1 entries having the same PAS. The maximum size of supported contiguous
blocks is defined by ``RME_GPT_MAX_BLOCK`` build option.

Granule Transition Service
~~~~~~~~~~~~~~~~~~~~~~~~~~

The Granule Transition Service allows memory mapped with
``GPT_MAP_REGION_GRANULE`` ownership to be changed using SMC calls. Non-secure
granules can be transitioned to either realm or secure space, and realm and
secure granules can be transitioned back to non-secure. This library only
allows Level 1 entries to be transitioned. The lib may either shatter
contiguous blocks or fuse adjacent GPT entries to form a contiguous block
opportunistically. Depending on the maximum block size, the fuse operation may
propogate to higher block sizes as allowed by RME Architecture. Thus a higher
maximum block size may have a higher runtime cost due to software operations
that need to be performed for fuse to bigger block sizes. This cost may
be offset by better TLB performance due to the higher block size and platforms
need to make the trade-off decision based on their particular workload.

Locking Scheme
~~~~~~~~~~~~~~

During Granule Transition access to L1 tables is controlled by a lock to ensure
that no more than one CPU is allowed to make changes at any given time.
The granularity of the lock is defined by ``RME_GPT_BITLOCK_BLOCK`` build option
which defines the size of the memory block protected by one bit of ``bitlock_t``
structure. Setting this option to 0 chooses a single spinlock for all GPT L1
table entries.

Library APIs
------------

The public APIs and types can be found in ``include/lib/gpt_rme/gpt_rme.h`` and
this section is intended to provide additional details.

To enable runtime granule transition below APIs must be used:

#. Firmware must use these APIs after successful RME initiazation.
#. Firmware must call ``gpt_runtime_init`` to initialize the bitlocks using by
   legacy GPT library to transition GPI at runtime using delegate/undelegate
   calls. Granule protection checks must be enabled already or this function
   will return an error.
#. Firmware must call ``gpt_delegate_pas`` transition a granule from NS PAS to
   Realm or Secure PAS.
#. Firmware must call ``gpt_undelegate_pas`` transition a granule from Realm or
   Secure PAS to NS PAS.

API Constraints
~~~~~~~~~~~~~~~

* The platform allocates the bitlock array which contains fine-grained
  ``bitlock_t`` data structures. The RME GPT library will check that the array
  has at least the amount of memory defined by PPS and ``RME_GPT_BITLOCK_BLOCK``
  value.

Sample calculation for bitlocks array size
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Let PPS=256TB and RME_GPT_BITLOCK_BLOCK=1

The size of bitlocks array in bits is the total protected space (PPS) divided
by the size of memory block per bit. The size of memory block
is ``RME_GPT_BITLOCK_BLOCK`` (number of 512MB blocks per bit) times
512MB (0x20000000). This is then divided by the number of bits in ``bitlock_t``
structure (8) to get the size of array in bytes.

In other words, we can find the total size of ``bitlock_t`` array
in bytes with PPS / (RME_GPT_BITLOCK_BLOCK * 0x20000000 * 8).

Substitute values to get this: 0x1000000000000 / (1 * 0x20000000 * 8)

And solve to get 0x10000 bytes.

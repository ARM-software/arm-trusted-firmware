Alternative Boot Flows
======================

EL3 payloads alternative boot flow
----------------------------------

On a pre-production system, the ability to execute arbitrary, bare-metal code at
the highest exception level is required. It allows full, direct access to the
hardware, for example to run silicon soak tests.

Although it is possible to implement some baremetal secure firmware from
scratch, this is a complex task on some platforms, depending on the level of
configuration required to put the system in the expected state.

Rather than booting a baremetal application, a possible compromise is to boot
``EL3 payloads`` through TF-A instead. This is implemented as an alternative
boot flow, where a modified BL2 boots an EL3 payload, instead of loading the
other BL images and passing control to BL31. It reduces the complexity of
developing EL3 baremetal code by:

-  putting the system into a known architectural state;
-  taking care of platform secure world initialization;
-  loading the SCP_BL2 image if required by the platform.

When booting an EL3 payload on Arm standard platforms, the configuration of the
TrustZone controller is simplified such that only region 0 is enabled and is
configured to permit secure access only. This gives full access to the whole
DRAM to the EL3 payload.

The system is left in the same state as when entering BL31 in the default boot
flow. In particular:

-  Running in EL3;
-  Current state is AArch64;
-  Little-endian data access;
-  All exceptions disabled;
-  MMU disabled;
-  Caches disabled.

.. _alt_boot_flows_el3_payload:

Booting an EL3 payload
~~~~~~~~~~~~~~~~~~~~~~

The EL3 payload image is a standalone image and is not part of the FIP. It is
not loaded by TF-A. Therefore, there are 2 possible scenarios:

-  The EL3 payload may reside in non-volatile memory (NVM) and execute in
   place. In this case, booting it is just a matter of specifying the right
   address in NVM through ``EL3_PAYLOAD_BASE`` when building TF-A.

-  The EL3 payload needs to be loaded in volatile memory (e.g. DRAM) at
   run-time.

To help in the latter scenario, the ``SPIN_ON_BL1_EXIT=1`` build option can be
used. The infinite loop that it introduces in BL1 stops execution at the right
moment for a debugger to take control of the target and load the payload (for
example, over JTAG).

It is expected that this loading method will work in most cases, as a debugger
connection is usually available in a pre-production system. The user is free to
use any other platform-specific mechanism to load the EL3 payload, though.


Preloaded BL33 alternative boot flow
------------------------------------

Some platforms have the ability to preload BL33 into memory instead of relying
on TF-A to load it. This may simplify packaging of the normal world code and
improve performance in a development environment. When secure world cold boot
is complete, TF-A simply jumps to a BL33 base address provided at build time.

For this option to be used, the ``PRELOADED_BL33_BASE`` build option has to be
used when compiling TF-A. For example, the following command will create a FIP
without a BL33 and prepare to jump to a BL33 image loaded at address
0x80000000:

.. code:: shell

    make PRELOADED_BL33_BASE=0x80000000 PLAT=fvp all fip

--------------

*Copyright (c) 2019, Arm Limited. All rights reserved.*

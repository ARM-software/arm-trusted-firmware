ARM Trusted Firmware - SPM User Guide
=====================================

.. section-numbering::
    :suffix: .

.. contents::


This document briefly presents the Secure Partition Management (SPM) support in
the Arm Trusted Firmware (TF), specifically focusing on how to build Arm TF with
SPM support.

Overview of the SPM software stack
----------------------------------

SPM is supported on the Arm FVP exclusively at the moment.

It is not currently possible for BL31 to integrate SPM support and a Secure
Payload Dispatcher (SPD) at the same time; they are mutually exclusive. In the
SPM bootflow, a Secure Partition (SP) image executing at Secure-EL0 replaces the
Secure Payload image executing at Secure-EL1 (e.g. a Trusted OS). Both are
referred to as BL32.

A working prototype of a SP has been implemented by repurposing the EDK2 code
and tools, leveraging the concept of the *Standalone Management Mode (MM)* in
the UEFI specification (see the PI v1.6 Volume 4: Management Mode Core
Interface). This will be referred to as the *Standalone MM Secure Partition* in
the rest of this document.


Building TF with SPM support
----------------------------

To enable SPM support in the TF, the source code must be compiled with the build
flag ``ENABLE_SPM=1``. On Arm platforms the build option ``ARM_BL31_IN_DRAM``
can be used to select the location of BL31, both SRAM and DRAM are supported.


Using the Standalone MM SP
~~~~~~~~~~~~~~~~~~~~~~~~~~

First, build the Standalone MM Secure Partition. To build it, refer to the
`instructions in the EDK2 repository`_.

Then build TF with SPM support and include the Standalone MM Secure Partition
image in the FIP:

::

    BL32=path/to/standalone/mm/sp BL33=path/to/bl33.bin \
    make PLAT=fvp ENABLE_SPM=1 fip all


--------------

*Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.*

.. _instructions in the EDK2 repository: https://github.com/tianocore/edk2-staging/blob/AArch64StandaloneMm/HowtoBuild.MD

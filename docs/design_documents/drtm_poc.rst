DRTM Proof of Concept
=====================

Dynamic Root of Trust for Measurement (DRTM) begins a new trust environment
by measuring and executing a protected payload. The architectural requirements
and formal definition of DRTM for Arm-based systems are detailed in the
`DRTM Architecture for Arm`_

Static Root of Trust for Measurement (SRTM)/Measured Boot implementation,
currently used by TF-A covers all firmwares, from the boot ROM to the normal
world bootloader. As a whole, they make up the system's TCB. These boot
measurements allow attesting to what software is running on the system and
enable enforcing security policies.

As the boot chain grows or firmware becomes dynamically extensible,
establishing an attestable TCB becomes more challenging. DRTM  provides a
solution to this problem by allowing measurement chains to be started at
any time. As these measurements are stored separately from the boot-time
measurements, they reduce the size of the TCB, which helps reduce the attack
surface and the risk of untrusted code executing, which could compromise
the security of the system.

Components
~~~~~~~~~~

   - **DCE-Preamble**: The DCE Preamble prepares the platform for DRTM by
     doing any needed configuration, loading the target payload image(DLME),
     and preparing input parameters needed by DRTM. Finally, it invokes the
     DL Event to start the dynamic launch.

   - **D-CRTM**: The D-CRTM is the trust anchor (or root of trust) for the
     DRTM boot sequence and is where the dynamic launch starts. The D-CRTM
     must be implemented as a trusted agent in the system. The D-CRTM
     initializes the TPM for DRTM and prepares the environment for the next
     stage of DRTM, the DCE. The D-CRTM measures the DCE, verifies its
     signature, and transfers control to it.

   - **DCE**: The DCE executes on an application core. The DCE verifies the
     system’s state, measures security-critical attributes of the system,
     prepares the memory region for the target payload, measures the payload,
     and finally transfers control to the payload.

   - **DLME**: The protected payload is referred to as the Dynamically Launched
     Measured Environment, or DLME. The DLME begins execution in a safe state,
     with a single thread of execution, DMA protections, and interrupts
     disabled. The DCE provides data to the DLME that it can use to verify the
     configuration of the system.

In this proof of concept, DCE and D-CRTM are implemented in BL31 and
DCE-Preamble and DLME are implemented in UEFI application. A DL Event is
triggered as a SMC by DCE-Preamble and handled by D-CRTM, which launches the
DLME via DCE.

TF-A CI pipeline already includes coverage for building TF-A with a prebuilt
EDK2 and the DRTM UEFI application.

--------------

*Copyright (c) 2022-2025, Arm Limited. All rights reserved.*

.. _DRTM Architecture for Arm: https://developer.arm.com/documentation/den0113/latest

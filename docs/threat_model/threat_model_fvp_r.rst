fvp_r-Platform Threat Model
***************************

************************
Introduction
************************
This document provides a threat model for TF-A fvp_r platform.

************************
Target of Evaluation
************************
In this threat model, the target of evaluation is the fvp_r platform of Trusted
Firmware for A-class Processors (TF-A).  The fvp_r platform provides limited
support of AArch64 R-class Processors (v8-R64).

This is a delta document, only pointing out differences from the general TF-A
threat-model document, :ref:`Generic Threat Model`

BL1 Only
========
The most fundamental difference between the threat model for the current fvp_r
implementation compared to the general TF-A threat model, is that fvp_r is
currently limited to BL1 only.  Any threats from the general TF-A threat model
unrelated to BL1 are therefore not relevant to the fvp_r implementation.

The fvp_r BL1 implementation directly loads a customer/partner-defined runtime
system.  The threat model for that runtime system, being partner-defined, is
out-of-scope for this threat-model.

Relatedly, all exceptions, synchronous and asynchronous, are disabled during BL1
execution.  So, any references to exceptions are not relevant.

EL3 is Unsupported and All Secure
=================================
v8-R64 cores do not support EL3, and (essentially) all operation is defined as
Secure-mode.  Therefore:

    - Any threats regarding NS operation are not relevant.

    - Any mentions of SMCs are also not relevant.

    - Anything otherwise-relevant code running in EL3 is instead run in EL2.

MPU instead of MMU
==================
v8-R64 cores, running in EL2, use an MPU for memory management, rather than an
MMU.  The MPU in the fvp_r implementation is configured to function effectively
identically with the MMU for the usual BL1 implementation.  There are
memory-map differences, but the MPU configuration is functionally equivalent.

No AArch32 Support
==================
Another substantial difference between v8-A and v8-R64 cores is that v8-R64 does
not support AArch32.  However, this is not believed to have any threat-modeling
ramifications.


Threat Assessment
=================
For this section, please reference the Threat Assessment under the general TF-A
threat-model document, :ref:`Generic Threat Model`

The following threats from that document are still relevant to the fvp_r
implementation:

    - ID 01:  An attacker can mangle firmware images to execute arbitrary code.

    - ID 03:  An attacker can use Time-of-Check-Time-of-Use (TOCTOU) attack to
      bypass image authentication during the boot process.

    - ID 04:  An attacker with physical access can execute arbitrary image by
      bypassing the signature verification stage using clock- or power-glitching
      techniques.

    - ID 05:  Information leak via UART logs such as crashes

    - ID 06:  An attacker can read sensitive data and execute arbitrary code
      through the external debug and trace interface.

    - ID 08:  Memory corruption due to memory overflows and lack of boundary
      checking when accessing resources could allow an attacker to execute 
      arbitrary code, modify some state variable to change the normal flow of
      the program, or leak sensitive.

    - ID 11:  Misconfiguration of the Memory Protection Unit (MPU) may allow
      normal world software to access sensitive data or execute arbitrary code.
      Arguably, MPUs having fewer memory regions, there may be a temptation to
      share memory regions, making this a greater threat.  However, since the
      fvp_r implementation is limited to BL1, since BL1's regions are fixed,
      and since the MPU configuration is equivalent with that for the fvp
      platform and others, this is not expected to be a concern.



--------------

*Copyright (c) 2021, Arm Limited. All rights reserved.*

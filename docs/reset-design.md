ARM Trusted Firmware Reset Design
=================================

1.  [Introduction](#1--introduction)
2.  [General reset code flow](#2--general-reset-code-flow)
3.  [Programmable CPU reset address](#3--programmable-cpu-reset-address)
4.  [Cold boot on a single CPU](#4--cold-boot-on-a-single-cpu)
5.  [Programmable CPU reset address, Cold boot on a single CPU](#5--programmable-cpu-reset-address-cold-boot-on-a-single-cpu)
6.  [Using BL31 as the reset image](#6--using-bl31-as-the-reset-image)


1.  Introduction
----------------

This document provides a high-level overview of the CPU reset code design in
the ARM Trusted Firmware. It also describes how the platform integrator can
tailor this code to the system configuration to some extent, resulting in a
simplified and more optimised boot flow, both in terms of code size and
performance.

For a more in-depth, lower-level understanding of the reset code, please refer
to the [Firmware Design], specifically section 2 "Cold boot".


2.  General reset code flow
---------------------------

The ARM Trusted Firmware (TF) reset code is implemented in BL1 by default. The
following high-level diagram illustrates this:

![Default reset code flow](diagrams/default_reset_code.png?raw=true)

This diagram shows the default, unoptimised reset flow. Depending on the system
configuration, some of these steps might be unnecessary. The following sections
guide the platform integrator by indicating which build options exclude which
steps, depending on the capability of the platform.

Note: If BL31 is used as the Trusted Firmware entry point instead of BL1, the
diagram above is still relevant, as all these operations will occur in BL31 in
this case. Please refer to section 6 "Using BL31 as the reset image" for more
information.


3.  Programmable CPU reset address
----------------------------------

By default, the TF assumes that the CPU reset address is not programmable.
Therefore, all CPUs start at the same address (typically address 0) whenever
they reset. Further logic is then required to identify whether it is a cold or
warm boot to direct CPUs to the right execution path.

If the reset vector base address register `RVBAR_EL3` is programmable then it is
possible to make each CPU start directly at the right address, both on a cold
and warm reset. Therefore, the boot type detection can be skipped, resulting
in the following boot flow:

![Reset code flow with programmable reset address](
diagrams/reset_code_no_boot_type_check.png?raw=true)

To enable this boot flow, compile the TF with `PROGRAMMABLE_RESET_ADDRESS=1`.
This option only affects the TF reset image, which is BL1 by default or BL31 if
`RESET_TO_BL31=1`.

On both the FVP and Juno platforms, the `RVBAR_EL3` register is not programmable
so both ports use `PROGRAMMABLE_RESET_ADDRESS=0`.


4.  Cold boot on a single CPU
-----------------------------

By default, the TF assumes that several CPUs may be released out of reset.
Therefore, the cold boot code has to arbitrate access to hardware resources
shared amongst CPUs. This is done by nominating one of the CPUs as the primary,
which is responsible for initialising shared hardware and coordinating the boot
flow with the other CPUs.

If the platform guarantees that only a single CPU will ever be brought up then
no arbitration is required. The notion of primary/secondary CPU itself no longer
applies. This results in the following boot flow:

![Reset code flow with single CPU released out of reset](
diagrams/reset_code_no_cpu_check.png?raw=true)

To enable this boot flow, compile the TF with `COLD_BOOT_SINGLE_CPU=1`. This
option only affects the TF reset image, which is BL1 by default or BL31 if
`RESET_TO_BL31=1`.

On both the FVP and Juno platforms, although only one core is powered up by
default, there are platform-specific ways to release any number of cores out of
reset. Therefore, both platform ports use `COLD_BOOT_SINGLE_CPU=0`.


5.  Programmable CPU reset address, Cold boot on a single CPU
-------------------------------------------------------------

It is obviously possible to combine both optimisations on platforms that have
a programmable CPU reset address and which release a single CPU out of reset.
This results in the following boot flow:

![Reset code flow with programmable reset address and single CPU released out of
reset](diagrams/reset_code_no_checks.png?raw=true)

To enable this boot flow, compile the TF with both `COLD_BOOT_SINGLE_CPU=1`
and `PROGRAMMABLE_RESET_ADDRESS=1`. These options only affect the TF reset
image, which is BL1 by default or BL31 if `RESET_TO_BL31=1`.


6.  Using BL31 as the reset image
---------------------------------

On some platforms the runtime firmware (BL3x images) for the application
processors are loaded by some firmware running on a secure system processor
on the SoC, rather than by BL1 and BL2 running on the primary application
processor. For this type of SoC it is desirable for the application processor
to always reset to BL31 which eliminates the need for BL1 and BL2.

TF provides a build-time option `RESET_TO_BL31` that includes some additional
logic in the BL31 entry point to support this use case.

In this configuration, the platform's Trusted Boot Firmware must ensure that
BL31 is loaded to its runtime address, which must match the CPU's `RVBAR_EL3`
reset vector base address, before the application processor is powered on.
Additionally, platform software is responsible for loading the other BL3x images
required and providing entry point information for them to BL31. Loading these
images might be done by the Trusted Boot Firmware or by platform code in BL31.

Although the ARM FVP platform does not support programming the `RVBAR_EL3`
register dynamically at run-time, it is possible to set its initial value
at start-up. This feature is provided on the Base FVP only. It allows the ARM
FVP port to support the `RESET_TO_BL31` configuration, in which case the
`bl31.bin` image must be loaded to its run address in Trusted SRAM and all CPU
reset vectors be changed from the default `0x0` to this run address. See the
[User Guide] for details of running the FVP models in this way.

Although technically it would be possible to program the `RVBAR_EL3` register
with the right support in the SCP firmware, this is currently not implemented so
the Juno port doesn't support the `RESET_TO_BL31` configuration.

The `RESET_TO_BL31` configuration requires some additions and changes in the
BL31 functionality:

#### Determination of boot path

In this configuration, BL31 uses the same reset framework and code as the one
described for BL1 above. Therefore, it is affected by the
`PROGRAMMABLE_RESET_ADDRESS` and `COLD_BOOT_SINGLE_CPU` build options in the
same way.

In the default, unoptimised BL31 reset flow, on a warm boot a CPU is directed
to the PSCI implementation via a platform defined mechanism. On a cold boot,
the platform must place any secondary CPUs into a safe state while the primary
CPU executes a modified BL31 initialization, as described below.

#### Platform initialization

In this configuration, when the CPU resets to BL31 there are no parameters that
can be passed in registers by previous boot stages. Instead, the platform code
in BL31 needs to know, or be able to determine, the location of the BL32 (if
required) and BL33 images and provide this information in response to the
`bl31_plat_get_next_image_ep_info()` function.

Additionally, platform software is responsible for carrying out any security
initialisation, for example programming a TrustZone address space controller.
This might be done by the Trusted Boot Firmware or by platform code in BL31.

- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2015, ARM Limited and Contributors. All rights reserved._


[User Guide]:       user-guide.md
[Firmware Design]:  firmware-design.md

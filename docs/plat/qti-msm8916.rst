Qualcomm MSM8916
================
The MSM8916 platform port in TF-A supports multiple similar Qualcomm SoCs:

+-----------------------+----------------+-------------------+-----------------+
| System-on-Chip (SoC)  | TF-A Platform  | Application CPU   | Supports        |
+=======================+================+===================+=================+
| `Snapdragon 410`_     |``PLAT=msm8916``| 4x ARM Cortex-A53 | AArch64/AArch32 |
| (MSM8x16, APQ8016(E)) |                |                   |                 |
| (`DragonBoard 410c`_) |                |                   |                 |
+-----------------------+----------------+-------------------+-----------------+
| `Snapdragon 615`_     |``PLAT=msm8939``| 4x ARM Cortex-A53 | AArch64/AArch32 |
| (MSM8x39, APQ8039)    |                | 4x ARM Cortex-A53 |                 |
+-----------------------+----------------+-------------------+-----------------+
| `Snapdragon 210`_     |``PLAT=msm8909``| 4x ARM Cortex-A7  | AArch32 only    |
| (MSM8x09, APQ8009)    |                |                   |                 |
+-----------------------+----------------+-------------------+-----------------+
| `Snapdragon X5 Modem`_|``PLAT=mdm9607``| 1x ARM Cortex-A7  | AArch32 only    |
| (MDM9x07)             |                |                   |                 |
+-----------------------+----------------+-------------------+-----------------+

It provides a minimal, community-maintained EL3 firmware and PSCI implementation,
based on information from the public `Snapdragon 410E Technical Reference Manual`_
combined with a lot of trial and error to actually make it work.

.. note::
	Unlike the :doc:`QTI SC7180/SC7280 <qti>` ports, this port does **not**
	make use of a proprietary binary components (QTISECLIB). It is fully
	open-source but therefore limited to publicly documented hardware
	components.

Functionality
-------------
The TF-A port is much more minimal compared to the original firmware and
therefore expects the non-secure world (e.g. Linux) to manage more hardware,
such as the SMMUs and all remote processors (RPM, WCNSS, Venus, Modem).
Everything except modem is currently functional with a slightly modified version
of mainline Linux.

.. warning::
	This port is **not secure**. There is no special secure memory and the
	used DRAM is available from both the non-secure and secure worlds.
	Unfortunately, the hardware used for memory protection is not described
	in the APQ8016E documentation.

The port is primarily intended as a minimal PSCI implementation (without a
separate secure world) where this limitation is not a big problem. Booting
secondary CPU cores (PSCI ``CPU_ON``) is supported. Basic CPU core power
management (``CPU_SUSPEND``) is functional but still work-in-progress and
will be added later once ready.

Boot Flow
---------
BL31 (AArch64) or BL32/SP_MIN (AArch32) replaces the original ``tz`` firmware
in the boot flow::

	Boot ROM (PBL) -> SBL -> BL31 (EL3) -> U-Boot (EL2) -> Linux (EL2)

After initialization the normal world starts at a fixed entry address in EL2/HYP
mode, configured using ``PRELOADED_BL33_BASE``. At runtime, it is expected that
the normal world bootloader was already loaded into RAM by a previous firmware
component (usually SBL) and that it is capable of running in EL2/HYP mode.

`U-Boot for DragonBoard 410c`_ is recommended if possible. The original Little
Kernel-based bootloader from Qualcomm does not support EL2/HYP, but can be
booted using an additional shim loader such as `tfalkstub`_.

Build
-----
It is possible to build for either AArch64 or AArch32. Some platforms use 32-bit
CPUs that only support AArch32 (see table above). For all others AArch64 is the
preferred build option.

AArch64 (BL31)
^^^^^^^^^^^^^^
Setup the cross compiler for AArch64 and build BL31 for one of the platforms in
the table above::

	$ make CROSS_COMPILE=aarch64-none-elf- PLAT=...

The BL31 ELF image is generated in ``build/$PLAT/release/bl31/bl31.elf``.

AArch32 (BL32/SP_MIN)
^^^^^^^^^^^^^^^^^^^^^
Setup the cross compiler for AArch32 and build BL32 with SP_MIN for one of the
platforms in the table above::

	$ make CROSS_COMPILE=arm-none-eabi- PLAT=... ARCH=aarch32 AARCH32_SP=sp_min

The BL32 ELF image is generated in ``build/$PLAT/release/bl32/bl32.elf``.

Build Options
-------------
Some options can be changed at build time by adding them to the make command line:

 * ``QTI_UART_NUM``: Number of UART controller to use for debug output and crash
   reports. This must be the same UART as used by earlier boot firmware since
   the UART controller does not get fully initialized at the moment. Defaults to
   the usual debug UART used for the platform (see ``platform.mk``).
 * ``QTI_RUNTIME_UART``: By default (``0``) the UART is only used for the boot
   process and critical crashes. If set to ``1`` it is also used for runtime
   messages. Note that this option can only be used if the UART is reserved in
   the normal world and the necessary clocks remain enabled.

The memory region used for the different firmware components is not fixed and
can be changed on the make command line. The default values match the addresses
used by the original firmware (see ``platform.mk``):

 * ``PRELOADED_BL33_BASE``: The entry address for the normal world. Usually
   refers to the first bootloader (e.g. U-Boot).
 * ``BL31_BASE``: Base address for the BL31 firmware component. Must point to
   a 64K-aligned memory region with at least 128 KiB space that is permanently
   reserved in the normal world.
 * ``BL32_BASE``: Base address for the BL32 firmware component.

   * **AArch32:** BL32 is used in place of BL31, so the option is equivalent to
     ``BL31_BASE``.
   * **AArch64:** Secure-EL1 Payload. Defaults to using 128 KiB of space
     directly after BL31. For testing only, the port is primarily intended as
     a minimal PSCI implementation without a separate secure world.

Installation
------------
The ELF image must be "signed" before flashing it, even if the board has secure
boot disabled. In this case the signature does not provide any security,
but it provides the firmware with required metadata.

The `DragonBoard 410c`_ does not have secure boot enabled by default. In this
case you can simply sign the ELF image using a randomly generated key. You can
use e.g. `qtestsign`_::

	$ ./qtestsign.py tz build/msm8916/release/bl31/bl31.elf

Then install the resulting ``build/msm8916/release/bl31/bl31-test-signed.mbn``
to the ``tz`` partition on the device. BL31 should be running after a reboot.

.. note::
	On AArch32 the ELF image is called ``bl32.elf``.
	The installation procedure is identical.

.. warning::
	Do not flash incorrectly signed firmware on devices that have secure
	boot enabled! Make sure that you have a way to recover the board in case
	of problems (e.g. using EDL).

Boot Trace
----------

AArch64 (BL31)
^^^^^^^^^^^^^^
BL31 prints some lines on the debug console, which will usually look like this
(with ``DEBUG=1``, otherwise only the ``NOTICE`` lines are shown)::

	...
	S - DDR Frequency, 400 MHz
	NOTICE:  BL31: v2.6(debug):v2.6
	NOTICE:  BL31: Built : 20:00:00, Dec 01 2021
	INFO:    BL31: Platform setup start
	INFO:    ARM GICv2 driver initialized
	INFO:    BL31: Platform setup done
	INFO:    BL31: Initializing runtime services
	INFO:    BL31: cortex_a53: CPU workaround for 819472 was applied
	INFO:    BL31: cortex_a53: CPU workaround for 824069 was applied
	INFO:    BL31: cortex_a53: CPU workaround for 826319 was applied
	INFO:    BL31: cortex_a53: CPU workaround for 827319 was applied
	INFO:    BL31: cortex_a53: CPU workaround for 835769 was applied
	INFO:    BL31: cortex_a53: CPU workaround for disable_non_temporal_hint was applied
	INFO:    BL31: cortex_a53: CPU workaround for 843419 was applied
	INFO:    BL31: cortex_a53: CPU workaround for 1530924 was applied
	INFO:    BL31: Preparing for EL3 exit to normal world
	INFO:    Entry point address = 0x8f600000
	INFO:    SPSR = 0x3c9

	U-Boot 2021.10 (Dec 01 2021 - 20:00:00 +0000)
	Qualcomm-DragonBoard 410C
	...

AArch32 (BL32/SP_MIN)
^^^^^^^^^^^^^^^^^^^^^
BL32/SP_MIN prints some lines on the debug console, which will usually look like
this (with ``DEBUG=1``, otherwise only the ``NOTICE`` lines are shown)::

	...
	S - DDR Frequency, 400 MHz
	NOTICE:  SP_MIN: v2.8(debug):v2.8
	NOTICE:  SP_MIN: Built : 23:03:31, Mar 31 2023
	INFO:    SP_MIN: Platform setup start
	INFO:    ARM GICv2 driver initialized
	INFO:    SP_MIN: Platform setup done
	INFO:    SP_MIN: Initializing runtime services
	INFO:    BL32: cortex_a53: CPU workaround for 819472 was applied
	INFO:    BL32: cortex_a53: CPU workaround for 824069 was applied
	INFO:    BL32: cortex_a53: CPU workaround for 826319 was applied
	INFO:    BL32: cortex_a53: CPU workaround for 827319 was applied
	INFO:    BL32: cortex_a53: CPU workaround for disable_non_temporal_hint was applied
	INFO:    SP_MIN: Preparing exit to normal world
	INFO:    Entry point address = 0x86400000
	INFO:    SPSR = 0x1da
	Android Bootloader - UART_DM Initialized!!!
	[0] welcome to lk
	...

.. _Snapdragon 210: https://www.qualcomm.com/products/snapdragon-processors-210
.. _Snapdragon 410: https://www.qualcomm.com/products/snapdragon-processors-410
.. _Snapdragon 615: https://www.qualcomm.com/products/snapdragon-processors-615
.. _Snapdragon X5 Modem: https://www.qualcomm.com/products/snapdragon-modems-4g-lte-x5
.. _DragonBoard 410c: https://www.96boards.org/product/dragonboard410c/
.. _Snapdragon 410E Technical Reference Manual: https://developer.qualcomm.com/download/sd410/snapdragon-410e-technical-reference-manual.pdf
.. _U-Boot for DragonBoard 410c: https://u-boot.readthedocs.io/en/latest/board/qualcomm/dragonboard410c.html
.. _qtestsign: https://github.com/msm8916-mainline/qtestsign
.. _tfalkstub: https://github.com/msm8916-mainline/tfalkstub

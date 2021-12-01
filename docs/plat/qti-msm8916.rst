Qualcomm Snapdragon 410 (MSM8916/APQ8016)
=========================================

The `Qualcomm Snapdragon 410`_ is Qualcomm's first 64-bit SoC, released in 2014
with four ARM Cortex-A53 cores. There are differents variants (MSM8916,
APQ8016(E), ...) that are all very similar. A popular device based on APQ8016E
is the `DragonBoard 410c`_ single-board computer, but the SoC is also used in
various mid-range smartphones/tablets.

The TF-A/BL31 port for MSM8916 provides a minimal, community-maintained
EL3 firmware. It is primarily based on information from the public
`Snapdragon 410E Technical Reference Manual`_ combined with a lot of
trial and error to actually make it work.

.. note::
	Unlike the :doc:`QTI SC7180/SC7280 <qti>` ports, this port does **not**
	make use of a proprietary binary components (QTISECLIB). It is fully
	open-source but therefore limited to publicly documented hardware
	components.

Functionality
-------------

The BL31 port is much more minimal compared to the original firmware and
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
BL31 replaces the original ``tz`` firmware in the boot flow::

	Boot ROM (PBL) -> SBL -> BL31 (EL3) -> U-Boot (EL2) -> Linux (EL2)

By default, BL31 enters the non-secure world in EL2 AArch64 state at address
``0x8f600000``. The original hypervisor firmware (``hyp``) is not used, you can
use KVM or another hypervisor. The entry address is fixed in the BL31 binary
but can be changed using the ``PRELOADED_BL33_BASE`` make file parameter.

Using an AArch64 bootloader (such as `U-Boot for DragonBoard 410c`_) is
recommended. AArch32 bootloaders (such as the original Little Kernel bootloader
from Qualcomm) are not directly supported, although it is possible to use an EL2
shim loader to temporarily switch to AArch32 state.

Installation
------------
First, setup the cross compiler for AArch64 and build TF-A for ``msm8916``::

	$ make CROSS_COMPILE=aarch64-linux-gnu- PLAT=msm8916

The BL31 ELF image is generated in ``build/msm8916/release/bl31/bl31.elf``.
This image must be "signed" before flashing it, even if the board has secure
boot disabled. In this case the signature does not provide any security,
but it provides the firmware with required metadata.

The `DragonBoard 410c`_ does not have secure boot enabled by default. In this
case you can simply sign the ELF image using a randomly generated key. You can
use e.g. `qtestsign`_::

	$ ./qtestsign.py tz build/msm8916/release/bl31/bl31.elf

Then install the resulting ``build/msm8916/release/bl31/bl31-test-signed.mbn``
to the ``tz`` partition on the device. BL31 should be running after a reboot.

.. warning::
	Do not flash incorrectly signed firmware on devices that have secure
	boot enabled! Make sure that you have a way to recover the board in case
	of problems (e.g. using EDL).

Boot Trace
----------
BL31 prints some lines on the debug console UART2, which will usually look like
this (with ``DEBUG=1``, otherwise only the ``NOTICE`` lines are shown)::

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

.. _Qualcomm Snapdragon 410: https://www.qualcomm.com/products/snapdragon-processors-410
.. _DragonBoard 410c: https://www.96boards.org/product/dragonboard410c/
.. _Snapdragon 410E Technical Reference Manual: https://developer.qualcomm.com/download/sd410/snapdragon-410e-technical-reference-manual.pdf
.. _U-Boot for DragonBoard 410c: https://u-boot.readthedocs.io/en/latest/board/qualcomm/dragonboard410c.html
.. _qtestsign: https://github.com/msm8916-mainline/qtestsign

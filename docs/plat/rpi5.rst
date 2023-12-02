Raspberry Pi 5
==============

The `Raspberry Pi 5`_ is a single-board computer that contains four
Arm Cortex-A76 cores.

This port is a minimal BL31 implementation capable of booting 64-bit EL2
payloads such as Linux and EDK2.

**IMPORTANT NOTE**: This port isn't secure. All of the memory used is DRAM,
which is available from both the Non-secure and Secure worlds. The SoC does
not seem to feature a secure memory controller of any kind, so portions of
DRAM can't be protected properly from the Non-secure world.

Build
------------------

To build this platform, run:

.. code:: shell

    CROSS_COMPILE=aarch64-linux-gnu- make PLAT=rpi5 DEBUG=1

The firmware will be generated at ``build/rpi5/debug/bl31.bin``.

The following build options are supported:

- ``RPI3_DIRECT_LINUX_BOOT``: Enabled by default. Allows direct boot of the Linux
  kernel from the firmware.

- ``PRELOADED_BL33_BASE``: Used to specify the fixed address of a BL33 binary
  that has been preloaded by earlier boot stages (VPU). Useful for bundling
  BL31 and BL33 in the same ``armstub`` image (e.g. TF-A + EDK2).

- ``RPI3_PRELOADED_DTB_BASE``: This option allows to specify the fixed address of
  a DTB in memory. Can only be used if ``device_tree_address=`` is present in
  config.txt.

- ``RPI3_RUNTIME_UART``: Indicates whether TF-A should use the debug UART for
  runtime messages or not. ``-1`` (default) disables the option, any other value
  enables it.

Usage
------------------

Copy the firmware binary to the first FAT32 partition of a supported boot media
(SD, USB) and append ``armstub=bl31.bin`` to config.txt, or just rename the
file to ``armstub8-2712.bin``.

No other config options or files are required by the firmware alone, this will
depend on the payload you intend to run.

For Linux, you must also place an appropriate DTB and kernel in the boot
partition. This has been validated with a copy of Raspberry Pi OS.

The VPU will preload a BL33 AArch64 image named either ``kernel_2712.img`` or
``kernel8.img``, which can be overridden by adding a ``kernel=filename`` option
to config.txt.

Kernel and DTB load addresses are also chosen by the VPU and can be changed with
``kernel_address=`` and ``device_tree_address=`` in config.txt. If TF-A was built
with ``PRELOADED_BL33_BASE`` or ``RPI3_PRELOADED_DTB_BASE``, setting those config
options may be necessary.

By default, all boot stages print messages to the dedicated UART debug port.
Configuration is ``115200 8n1``.

Design
------------------

This port is largely based on the RPi 4 one.

The boot process is essentially the same, the only notable difference being that
all VPU blobs have been moved into EEPROM (former start4.elf & fixup4.dat). There's
also a custom BL31 TF-A armstub included for PSCI, which can be replaced with this
port.

.. _Raspberry Pi 5: https://www.raspberrypi.com/products/raspberry-pi-5/

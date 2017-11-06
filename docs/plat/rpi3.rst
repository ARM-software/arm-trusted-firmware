ARM Trusted Firmware for Raspberry Pi 3
=======================================

.. section-numbering::
    :suffix: .

.. contents::

The Raspberry Pi 3 is an inexpensive single-board computer that contains four
Cortex-A53 cores. This makes it possible to have a port of the Arm Trusted
Firmware.

IMPORTANT NOTE: This port isn't secure. All of the memory used is DRAM, which is
available from both the Non-secure and Secure worlds. This port shouldn't be
considered more than a prototype to play with and implement elements like PSCI
to support the Linux kernel.

Design
------

The SoC used by the Raspberry Pi 3 is the Broadcom BCM2837. It is a strange SoC
with a VideoCore IV that acts as primary processor (and loads everything from
the SD card) and is located between all Arm cores and the DRAM.

This explains why it is possible to change the execution state (AArch64/AArch32)
depending on a few files on the SD card. We only care about the cases in which
the cores boot in AArch64 mode.

The rules are simple:

- If a file called ``kernel8.img`` is located on the ``boot`` partition of the
  SD card, it will load it and execute in EL2 in AArch64. Basically, it executes
  a `default AArch64 stub`_ at address ``0x0``.

- If there is also a file called ``armstub8.bin``, it will load it at address
  ``0x0`` (instead of the default one) and execute it in EL3 in AArch64. All the
  cores are powered on at the same time and start at address ``0x0``.

This is ideal for us, as we can save the default AArch32 kernel in
``kernel8.img`` and the Trusted Firmware and anything we need in
``armstub8.bin``, and forget about the default bootstrap code.

However, we still want to load the kernel and have all cores available, which
means that we need to make the secondary cores work in the way the kernel
expects, as explained in `Secondary cores`_.

Ideally, we want to boot the kernel in Hypervisor mode in AArch32. This means
that BL33 can't be in EL2 in AArch64 mode. The architecture specifies that
AArch32 Hypervisor mode isn't present when AArch64 is used for EL2.

Placement of images
~~~~~~~~~~~~~~~~~~~

The file ``armstub8.bin`` contains BL1 and the FIP. It is needed to add padding
between them so that the addresses they are loaded to match the ones specified
when compiling the Trusted Firmware.

The device tree block is loaded by the VideoCore loader from an appropriate
file, but we can specify the address it is loaded to in ``config.txt``.

The file ``kernel8.img`` contains an AArch32 kernel image, that is loaded to the
address that we specify in ``config.txt`` as well The `Linux kernel tree`_ has
information about how the Linux kernel is loaded in the file
``Documentation/arm/Booting``:

.. code::

    The zImage may also be placed in system RAM and called there.  The
    kernel should be placed in the first 128MiB of RAM.  It is recommended
    that it is loaded above 32MiB in order to avoid the need to relocate
    prior to decompression, which will make the boot process slightly
    faster.

This means that we need to avoid the first 128 MiB of RAM when placing the
Trusted Firmware images (and specially the first 32 MiB, as they are directly
used to place the uncompressed kernel image.

In the end, the images look like the following diagram when placed in memory.
All addresses are Physical Addresses from the point of view of the Arm cores.
Again, note that this is all just part of the same DRAM that goes from
``0x00000000`` to ``0x3F000000``, it just has different names to simulate a real
secure platform!

.. code::

    0x00000000 +-----------------+
               |       ROM       | BL1
    0x00010000 +-----------------+
               |       FIP       |
    0x00200000 +-----------------+
               |                 |
               |       ...       |
               |                 |
    0x01000000 +-----------------+
               |     Kernel      |
               +-----------------+
               |                 |
               |       ...       |
               |                 |
    0x02000000 +-----------------+
               |       DTB       |
               +-----------------+
               |                 |
               |       ...       |
               |                 |
    0x10000000 +-----------------+
               |   Secure SRAM   | BL2, BL31
    0x10100000 +-----------------+
               |   Secure DRAM   |
    0x10300000 +-----------------+
               | Non-secure DRAM | BL33
    0x11000000 +-----------------+
               |                 |
               |       ...       |
               |                 |
    0x3F000000 +-----------------+
               |       I/O       |
    0x40000000 +-----------------+

The area between ``0x10000000`` and ``0x11000000`` has to be protected so that
the kernel doesn't use it. That is done by adding ``memmap=256M$16M`` to the
command line passed to the kernel. See the `Setup SD card`_ instructions to see
how to do it.

The last 16 MiB of DRAM can only be accessed by the VideoCore, that has
different mappings than the Arm cores, where the I/O addresses don't overlap the
DRAM. The memory used by the VideoCore is always placed at the end of the DRAM,
so this space isn't wasted.

Considering the 128 MiB allocated to the GPU and the 16 MiB allocated for the
Trusted Firmware, there are 880 MiB available for the kernel.

Boot sequence
~~~~~~~~~~~~~

The boot sequence of the Trusted Firmware is the usual one. The only difference
is BL33, which is booted in AArch32 Hypervisor mode so that it can jump to the
kernel in the same mode and let it take over that privilege level. If BL33 was
running in EL2 in AArch64, it could only jump to the kernel in AArch32 in
Supervisor mode.

Jumping to the kernel
~~~~~~~~~~~~~~~~~~~~~

The `Linux kernel tree`_ has instructions on how to jump to the Linux kernel
correctly in the file ``Documentation/arm/Booting``. In short:

.. code::

    CPU register settings:
      r0 = 0
      r1 = Machine type number (3138 for the Rasbperry Pi 3)
      r2 = Physical address of the device tree block (dtb) in RAM

    CPU mode:
      All forms of interrupts must be disabled (IRQs and FIQs)

The AArch32 bootstrap takes care of the setup of the registers, and BL2 setups
the BL33 context so that all interrupts are disabled.

Secondary cores
~~~~~~~~~~~~~~~

The 32-bit kernel used by Raspbian doesn't have support for PSCI, so it is
needed to use mailboxes to trap the secondary cores until they are ready to jump
to the kernel.

The port of the Raspberry Pi 3 of the Trusted Firmware has a Trusted Mailbox in
Shared BL RAM. During cold boot, all secondary cores wait in a loop until they
are given given an address to jump to (``bl31_warm_entrypoint``).

Once BL31 has finished and the primary core has jumped to the BL33 AArch32
payload, it calls ``PSCI_CPU_ON_AARCH32`` to release the secondary CPUs from the
wait loop. The payload then makes them wait in another waitloop listening from
messages from the kernel. When the primary CPU jumps into the kernel, it will
send an address to the mailbox so that the secondary CPUs jump to it and are
recognised by the kernel.

Build Instructions
------------------

This port requires both AArch64 and AArch32 toolchains.

The AArch32 toolchain is needed for the AArch32 bootstrap needed to load a
32-bit kernel like Raspbian.

First, clone and compile `Raspberry Pi 3 Arm Trusted Firmware bootstrap`_.

Then compile the Arm Trusted Firmware like this:

.. code:: bash

    CROSS_COMPILE=aarch64-linux-gnu-                            \
    make PLAT=rpi3                                              \
    RPI3_BL33_IN_AARCH32=1                                      \
    RESET_TO_BL31=1                                             \
    BL33=../rpi3-arm-tf-bootstrap/aarch32/el2-bootstrap.bin     \
    all fip

Then, join BL1 and the FIP with the following instructions (replace ``release``
by ``debug`` if you set the build option ``DEBUG=1``):

.. code:: bash

    cp build/rpi3/release/bl1.bin bl1.pad.bin
    truncate --size=65536 bl1.pad.bin
    cat bl1.pad.bin build/rpi3/release/fip.bin > armstub8.bin

The resulting file, ``armstub8.bin``, contains BL1 and the FIP in the place they
need to be for the Trusted Firmware to boot correctly. Now, follow the
instructions in `Setup SD card`_.

The following build options are supported:

- ``PRELOADED_BL33_BASE``: Specially useful because the file ``kernel8.img`` can
  be loaded anywhere by modifying the file ``config.txt``.

- ``RESET_TO_BL31``: Set to 0 by default. If using a 32-bit kernel like
  Raspbian, the space used by BL1 can overwriten by the kernel, but the space
  used by BL31 is reserved. This option should be set to 1 in that case.

- ``RPI3_BL33_IN_AARCH32``: This port can load an AArch64 or AArch32 BL33 image.
  By default this option is 0, which means that the Trusted Firmware will jump
  to BL33 in EL2 in AArch64 mode. If set to 1, it will jump to BL33 in
  Hypervisor in AArch32 mode.

The following is not currently supported:

- AArch32 for the Trusted Firmware itself.

- ``EL3_PAYLOAD_BASE``: The reason is that you can already load anything to any
  address by changing the file ``armstub8.bin``, so there's no point in adding
  the Trusted Firmware.

- ``LOAD_IMAGE_V2=0``: Only version 2 is supported.

Setup SD card
-------------

The instructions assume that you have an SD card with a fresh install of
`Raspbian`_ (or that, at least, the ``boot`` partition is untouched, or nearly
untouched). They have been tested with the image available in 2017-09-07.

1. Insert the SD card and open the ``boot`` partition.

2. Rename ``kernel7.img`` to ``kernel8.img``. This tricks the VideoCore
   bootloader into booting the Arm cores in AArch64 mode, like the Trusted
   Firmware needs, even though the kernel is not compiled for AArch64.

3. Copy ``armstub8.bin`` here. When ``kernel8.img`` is available, The VideoCore
   bootloader will look for a file called ``armstub8.bin`` and load it at
   address ``0x0`` instead of a predefined one.

4. Open ``cmdline.txt`` and add ``memmap=256M$16M`` to prevent the kernel from
   using the memory needed by the Trusted Firmware. If you want to enable the
   serial port "Mini UART", make sure that this file also contains
   ``console=serial0,115200 console=tty1``.

   Note that the 16 MiB reserved this way won't be available for Linux, the same
   way as the memory reserved in the DRAM for the GPU isn't available.

5. Open ``config.txt`` and add the following lines at the end (``enable_uart=1``
   is only needed to enable debugging throug the Mini UART):

.. code::

    enable_uart=1
    kernel_address=0x01000000
    device_tree_address=0x02000000

If you connect a serial cable to the Mini UART and connect to it (for example,
with ``screen /dev/ttyUSB0 115200``) you should see text that looks like this:

.. code::

    NOTICE:  Booting Trusted Firmware
    NOTICE:  BL1: v1.4(release):v1.4-329-g61e94684-dirty
    NOTICE:  BL1: Built : 00:09:25, Nov  6 2017
    NOTICE:  BL1: Booting BL2
    NOTICE:  BL2: v1.4(release):v1.4-329-g61e94684-dirty
    NOTICE:  BL2: Built : 00:09:25, Nov  6 2017
    NOTICE:  BL1: Booting BL31
    NOTICE:  BL31: v1.4(release):v1.4-329-g61e94684-dirty
    NOTICE:  BL31: Built : 00:09:25, Nov  6 2017
    [    0.266484] bcm2835-aux-uart 3f215040.serial: could not get clk: -517

    Raspbian GNU/Linux 9 raspberrypi ttyS0
    raspberrypi login:

Just enter your credentials, everything should work as expected. Note that the
HDMI output won't show any text during boot.

.. _Raspbian: https://www.raspberrypi.org/downloads/raspbian/
.. _Linux kernel tree: https://github.com/torvalds/linux
.. _default Arm stub: https://github.com/raspberrypi/tools/blob/master/armstubs/armstub7.S
.. _default AArch64 stub: https://github.com/raspberrypi/tools/blob/master/armstubs/armstub8.S
.. _Raspberry Pi 3 Arm Trusted Firmware bootstrap: https://github.com/AntonioND/rpi3-arm-tf-bootstrap

Arm Trusted Firmware for Raspberry Pi 3
=======================================

.. section-numbering::
    :suffix: .

.. contents::

The `Raspberry Pi 3`_ is an inexpensive single-board computer that contains four
Cortex-A53 cores, which makes it possible to have a port of the Arm Trusted
Firmware.

The following instructions explain how to use this port of the Trusted Firmware
with the default distribution of `Raspbian`_ because that's the distribution
officially supported by the Raspberry Pi Foundation. At the moment of writing
this, the officially supported kernel is a AArch32 kernel. This doesn't mean
that this port of the Trusted Firmware can't boot a AArch64 kernel. The `Linux
tree fork`_ maintained by the Foundation can be compiled for AArch64 by
following the steps in `AArch64 kernel build instructions`_.

**IMPORTANT NOTE**: This port isn't secure. All of the memory used is DRAM,
which is available from both the Non-secure and Secure worlds. This port
shouldn't be considered more than a prototype to play with and implement
elements like PSCI to support the Linux kernel.

Design
------

The SoC used by the Raspberry Pi 3 is the Broadcom BCM2837. It is a SoC with a
VideoCore IV that acts as primary processor (and loads everything from the SD
card) and is located between all Arm cores and the DRAM. Check the `Raspberry Pi
3 documentation`_ for more information.

This explains why it is possible to change the execution state (AArch64/AArch32)
depending on a few files on the SD card. We only care about the cases in which
the cores boot in AArch64 mode.

The rules are simple:

- If a file called ``kernel8.img`` is located on the ``boot`` partition of the
  SD card, it will load it and execute in EL2 in AArch64. Basically, it executes
  a `default AArch64 stub`_ at address **0x0** that jumps to the kernel.

- If there is also a file called ``armstub8.bin``, it will load it at address
  **0x0** (instead of the default stub) and execute it in EL3 in AArch64. All
  the cores are powered on at the same time and start at address **0x0**.

This means that we can use the default AArch32 kernel provided in the official
`Raspbian`_ distribution by renaming it to ``kernel8.img``, while the Trusted
Firmware and anything else we need is in ``armstub8.bin``. This way we can
forget about the default bootstrap code. When using a AArch64 kernel, it is only
needed to make sure that the name on the SD card is ``kernel8.img``.

Ideally, we want to load the kernel and have all cores available, which means
that we need to make the secondary cores work in the way the kernel expects, as
explained in `Secondary cores`_. In practice, a small bootstrap is needed
between the Trusted Firmware and the kernel.

To get the most out of a AArch32 kernel, we want to boot it in Hypervisor mode
in AArch32. This means that BL33 can't be in EL2 in AArch64 mode. The
architecture specifies that AArch32 Hypervisor mode isn't present when AArch64
is used for EL2. When using a AArch64 kernel, it should simply start in EL2.

Placement of images
~~~~~~~~~~~~~~~~~~~

The file ``armstub8.bin`` contains BL1 and the FIP. It is needed to add padding
between them so that the addresses they are loaded to match the ones specified
when compiling the Trusted Firmware.

The device tree block is loaded by the VideoCore loader from an appropriate
file, but we can specify the address it is loaded to in ``config.txt``.

The file ``kernel8.img`` contains a kernel image that is loaded to the address
specified in ``config.txt``. The `Linux kernel tree`_ has information about how
a AArch32 Linux kernel image is loaded in ``Documentation/arm/Booting``:

::

    The zImage may also be placed in system RAM and called there.  The
    kernel should be placed in the first 128MiB of RAM.  It is recommended
    that it is loaded above 32MiB in order to avoid the need to relocate
    prior to decompression, which will make the boot process slightly
    faster.

There are no similar restrictions for AArch64 kernels, as specified in the file
``Documentation/arm64/booting.txt``.

This means that we need to avoid the first 128 MiB of RAM when placing the
Trusted Firmware images (and specially the first 32 MiB, as they are directly
used to place the uncompressed AArch32 kernel image. This way, both AArch32 and
AArch64 kernels can be placed at the same address.

In the end, the images look like the following diagram when placed in memory.
All addresses are Physical Addresses from the point of view of the Arm cores.
Again, note that this is all just part of the same DRAM that goes from
**0x00000000** to **0x3F000000**, it just has different names to simulate a real
secure platform!

::

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

The area between **0x10000000** and **0x11000000** has to be protected so that
the kernel doesn't use it. That is done by adding ``memmap=256M$16M`` to the
command line passed to the kernel. See the `Setup SD card`_ instructions to see
how to do it.

The last 16 MiB of DRAM can only be accessed by the VideoCore, that has
different mappings than the Arm cores in which the I/O addresses don't overlap
the DRAM. The memory reserved to be used by the VideoCore is always placed at
the end of the DRAM, so this space isn't wasted.

Considering the 128 MiB allocated to the GPU and the 16 MiB allocated for the
Trusted Firmware, there are 880 MiB available for Linux.

Boot sequence
~~~~~~~~~~~~~

The boot sequence of the Trusted Firmware is the usual one except when booting
a AArch32 kernel. In that case, BL33 is booted in AArch32 Hypervisor mode so
that it can jump to the kernel in the same mode and let it take over that
privilege level. If BL33 was running in EL2 in AArch64 (as in the default
bootflow of the Trusted Firmware) it could only jump to the kernel in AArch32 in
Supervisor mode.

The `Linux kernel tree`_ has instructions on how to jump to the Linux kernel
in ``Documentation/arm/Booting`` and ``Documentation/arm64/booting.txt``. The
bootstrap should take care of this.

Secondary cores
~~~~~~~~~~~~~~~

The kernel used by `Raspbian`_ doesn't have support for PSCI, so it is needed to
use mailboxes to trap the secondary cores until they are ready to jump to the
kernel. This mailbox is located at a different address in the AArch32 default
kernel than in the AArch64 kernel.

Also, this port of the Trusted Firmware has another Trusted Mailbox in Shared BL
RAM. During cold boot, all secondary cores wait in a loop until they are given
given an address to jump to in this Mailbox (``bl31_warm_entrypoint``).

Once BL31 has finished and the primary core has jumped to the BL33 payload, it
has to call ``PSCI_CPU_ON`` to release the secondary CPUs from the wait loop.
The payload then makes them wait in another waitloop listening from messages
from the kernel. When the primary CPU jumps into the kernel, it will send an
address to the mailbox so that the secondary CPUs jump to it and are recognised
by the kernel.

Build Instructions
------------------

To boot a AArch64 kernel, only the AArch64 toolchain is required.

To boot a AArch32 kernel, both AArch64 and AArch32 toolchains are required. The
AArch32 toolchain is needed for the AArch32 bootstrap needed to load a 32-bit
kernel.

First, clone and compile `Raspberry Pi 3 Arm Trusted Firmware bootstrap`_.
Choose the one needed for the architecture of your kernel.

Then compile the Arm Trusted Firmware. For a AArch32 kernel, use the following
command line:

.. code:: shell

    CROSS_COMPILE=aarch64-linux-gnu- make PLAT=rpi3             \
    RPI3_BL33_IN_AARCH32=1                                      \
    BL33=../rpi3-arm-tf-bootstrap/aarch32/el2-bootstrap.bin     \
    all fip

For a AArch64 kernel, use this other command line:

.. code:: shell

    CROSS_COMPILE=aarch64-linux-gnu- make PLAT=rpi3             \
    BL33=../rpi3-arm-tf-bootstrap/aarch64/el2-bootstrap.bin     \
    all fip

Then, join BL1 and the FIP with the following instructions (replace ``release``
by ``debug`` if you set the build option ``DEBUG=1``):

.. code:: shell

    cp build/rpi3/release/bl1.bin bl1.pad.bin
    truncate --size=65536 bl1.pad.bin
    cat bl1.pad.bin build/rpi3/release/fip.bin > armstub8.bin

The resulting file, ``armstub8.bin``, contains BL1 and the FIP in the place they
need to be for the Trusted Firmware to boot correctly. Now, follow the
instructions in `Setup SD card`_.

The following build options are supported:

- ``PRELOADED_BL33_BASE``: Specially useful because the file ``kernel8.img`` can
  be loaded anywhere by modifying the file ``config.txt``. It doesn't have to
  contain a kernel, it could have any arbitrary payload.

- ``RESET_TO_BL31``: Set to 1 by default. If using a 32-bit kernel like
  `Raspbian`_, the space used by BL1 can overwritten by the kernel when it is
  being loaded. Even when using a AArch64 kernel the region used by
  BL1 isn't protected and the kernel could overwrite it. The space used by BL31
  is reserved by the command line passed to the kernel.

- ``RPI3_BL33_IN_AARCH32``: This port can load a AArch64 or AArch32 BL33 image.
  By default this option is 0, which means that the Trusted Firmware will jump
  to BL33 in EL2 in AArch64 mode. If set to 1, it will jump to BL33 in
  Hypervisor in AArch32 mode.

The following is not currently supported:

- AArch32 for the Trusted Firmware itself.

- ``EL3_PAYLOAD_BASE``: The reason is that you can already load anything to any
  address by changing the file ``armstub8.bin``, so there's no point in using
  the Trusted Firmware in this case.

- ``LOAD_IMAGE_V2=0``: Only version 2 is supported.

AArch64 kernel build instructions
---------------------------------

The following instructions show how to install and run a AArch64 kernel by
using a SD card with the default `Raspbian`_ install as base. Skip them if you
want to use the default 32-bit kernel.

Note that this system won't be fully 64-bit because all the tools in the
filesystem are 32-bit binaries, but it's a quick way to get it working, and it
allows the user to run 64-bit binaries in addition to 32-bit binaries.

1. Clone the `Linux tree fork`_ maintained by the Raspberry Pi Foundation. To
   speed things up, do a shallow clone of the desired branch.

.. code:: shell

    git clone --depth=1 -b rpi-4.14.y https://github.com/raspberrypi/linux
    cd linux

2. Configure and compile the kernel. Adapt the number after ``-j`` so that it is
   1.5 times the number of CPUs in your computer. This may take some time to
   finish.

.. code:: shell

    make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- bcmrpi3_defconfig
    make -j 6 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-

3. Copy the kernel image and the device tree to the SD card. Replace the path
   by the corresponding path in your computers to the ``boot`` partition of the
   SD card.

.. code:: shell

    cp arch/arm64/boot/Image /path/to/boot/kernel8.img
    cp arch/arm64/boot/dts/broadcom/bcm2710-rpi-3-b.dtb /path/to/boot/

4. Install the kernel modules. Replace the path by the corresponding path to the
   filesystem partition of the SD card on your computer.

.. code:: shell

    make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- \
    INSTALL_MOD_PATH=/path/to/filesystem modules_install

5. Follow the instructions in `Setup SD card`_ except for the step of renaming
   the existing ``kernel7.img`` (we have already copied a AArch64 kernel).

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
   address **0x0** instead of a predefined one.

4. Open ``cmdline.txt`` and add ``memmap=256M$16M`` to prevent the kernel from
   using the memory needed by the Trusted Firmware. If you want to enable the
   serial port "Mini UART", make sure that this file also contains
   ``console=serial0,115200 console=tty1``.

   Note that the 16 MiB reserved this way won't be available for Linux, the same
   way as the memory reserved in DRAM for the GPU isn't available.

5. Open ``config.txt`` and add the following lines at the end (``enable_uart=1``
   is only needed to enable debugging through the Mini UART):

::

    enable_uart=1
    kernel_address=0x01000000
    device_tree_address=0x02000000

If you connect a serial cable to the Mini UART and your computer, and connect
to it (for example, with ``screen /dev/ttyUSB0 115200``) you should see some
text. In the case of an AArch32 kernel, you should see something like this:

::

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

.. _default Arm stub: https://github.com/raspberrypi/tools/blob/master/armstubs/armstub7.S
.. _default AArch64 stub: https://github.com/raspberrypi/tools/blob/master/armstubs/armstub8.S
.. _Linux kernel tree: https://github.com/torvalds/linux
.. _Linux tree fork: https://github.com/raspberrypi/linux
.. _Raspberry Pi 3: https://www.raspberrypi.org/products/raspberry-pi-3-model-b/
.. _Raspberry Pi 3 Arm Trusted Firmware bootstrap: https://github.com/AntonioND/rpi3-arm-tf-bootstrap
.. _Raspberry Pi 3 documentation: https://www.raspberrypi.org/documentation/
.. _Raspbian: https://www.raspberrypi.org/downloads/raspbian/

Raspberry Pi 3
==============

The `Raspberry Pi 3`_ is an inexpensive single-board computer that contains four
Arm Cortex-A53 cores.

The following instructions explain how to use this port of the TF-A with the
default distribution of `Raspbian`_ because that's the distribution officially
supported by the Raspberry Pi Foundation. At the moment of writing this, the
officially supported kernel is a AArch32 kernel. This doesn't mean that this
port of TF-A can't boot a AArch64 kernel. The `Linux tree fork`_ maintained by
the Foundation can be compiled for AArch64 by following the steps in
`AArch64 kernel build instructions`_.

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
`Raspbian`_ distribution by renaming it to ``kernel8.img``, while TF-A and
anything else we need is in ``armstub8.bin``. This way we can forget about the
default bootstrap code. When using a AArch64 kernel, it is only needed to make
sure that the name on the SD card is ``kernel8.img``.

Ideally, we want to load the kernel and have all cores available, which means
that we need to make the secondary cores work in the way the kernel expects, as
explained in `Secondary cores`_. In practice, a small bootstrap is needed
between TF-A and the kernel.

To get the most out of a AArch32 kernel, we want to boot it in Hypervisor mode
in AArch32. This means that BL33 can't be in EL2 in AArch64 mode. The
architecture specifies that AArch32 Hypervisor mode isn't present when AArch64
is used for EL2. When using a AArch64 kernel, it should simply start in EL2.

Placement of images
~~~~~~~~~~~~~~~~~~~

The file ``armstub8.bin`` contains BL1 and the FIP. It is needed to add padding
between them so that the addresses they are loaded to match the ones specified
when compiling TF-A. This is done automatically by the build system.

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
TF-A images (and specially the first 32 MiB, as they are directly used to
place the uncompressed AArch32 kernel image. This way, both AArch32 and
AArch64 kernels can be placed at the same address.

In the end, the images look like the following diagram when placed in memory.
All addresses are Physical Addresses from the point of view of the Arm cores.
Again, note that this is all just part of the same DRAM that goes from
**0x00000000** to **0x3F000000**, it just has different names to simulate a real
secure platform!

::

    0x00000000 +-----------------+
               |       ROM       | BL1
    0x00020000 +-----------------+
               |       FIP       |
    0x00200000 +-----------------+
               |                 |
               |       ...       |
               |                 |
    0x01000000 +-----------------+
               |       DTB       | (Loaded by the VideoCore)
               +-----------------+
               |                 |
               |       ...       |
               |                 |
    0x02000000 +-----------------+
               |     Kernel      | (Loaded by the VideoCore)
               +-----------------+
               |                 |
               |       ...       |
               |                 |
    0x10000000 +-----------------+
               |   Secure SRAM   | BL2, BL31
    0x10100000 +-----------------+
               |   Secure DRAM   | BL32 (Secure payload)
    0x11000000 +-----------------+
               | Non-secure DRAM | BL33
               +-----------------+
               |                 |
               |       ...       |
               |                 |
    0x3F000000 +-----------------+
               |       I/O       |
    0x40000000 +-----------------+

The area between **0x10000000** and **0x11000000** has to be manually protected
so that the kernel doesn't use it. The current port tries to modify the live DTB
to add a memreserve region that reserves the previously mentioned area.

If this is not possible, the user may manually add ``memmap=16M$256M`` to the
command line passed to the kernel in ``cmdline.txt``. See the `Setup SD card`_
instructions to see how to do it. This system is strongly discouraged.

The last 16 MiB of DRAM can only be accessed by the VideoCore, that has
different mappings than the Arm cores in which the I/O addresses don't overlap
the DRAM. The memory reserved to be used by the VideoCore is always placed at
the end of the DRAM, so this space isn't wasted.

Considering the 128 MiB allocated to the GPU and the 16 MiB allocated for
TF-A, there are 880 MiB available for Linux.

Boot sequence
~~~~~~~~~~~~~

The boot sequence of TF-A is the usual one except when booting an AArch32
kernel. In that case, BL33 is booted in AArch32 Hypervisor mode so that it
can jump to the kernel in the same mode and let it take over that privilege
level. If BL33 was running in EL2 in AArch64 (as in the default bootflow of
TF-A) it could only jump to the kernel in AArch32 in Supervisor mode.

The `Linux kernel tree`_ has instructions on how to jump to the Linux kernel
in ``Documentation/arm/Booting`` and ``Documentation/arm64/booting.txt``. The
bootstrap should take care of this.

This port support a direct boot of the Linux kernel from the firmware (as a BL33
image). Alternatively, U-Boot or other bootloaders may be used.

Secondary cores
~~~~~~~~~~~~~~~

This port of the Trusted Firmware-A supports ``PSCI_CPU_ON``,
``PSCI_SYSTEM_RESET`` and ``PSCI_SYSTEM_OFF``. The last one doesn't really turn
the system off, it simply reboots it and asks the VideoCore firmware to keep it
in a low power mode permanently.

The kernel used by `Raspbian`_ doesn't have support for PSCI, so it is needed to
use mailboxes to trap the secondary cores until they are ready to jump to the
kernel. This mailbox is located at a different address in the AArch32 default
kernel than in the AArch64 kernel.

Kernels with PSCI support can use the PSCI calls instead for a cleaner boot.

Also, this port of TF-A has another Trusted Mailbox in Shared BL RAM. During
cold boot, all secondary cores wait in a loop until they are given given an
address to jump to in this Mailbox (``bl31_warm_entrypoint``).

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

The build system concatenates BL1 and the FIP so that the addresses match the
ones in the memory map. The resulting file is ``armstub8.bin``, located in the
build folder (e.g. ``build/rpi3/debug/armstub8.bin``). To know how to use this
file, follow the instructions in `Setup SD card`_.

The following build options are supported:

- ``RPI3_BL33_IN_AARCH32``: This port can load a AArch64 or AArch32 BL33 image.
  By default this option is 0, which means that TF-A will jump to BL33 in EL2
  in AArch64 mode. If set to 1, it will jump to BL33 in Hypervisor in AArch32
  mode.

- ``PRELOADED_BL33_BASE``: Used to specify the address of a BL33 binary that has
  been preloaded by any other system than using the firmware. ``BL33`` isn't
  needed in the build command line if this option is used. Specially useful
  because the file ``kernel8.img`` can be loaded anywhere by modifying the file
  ``config.txt``. It doesn't have to contain a kernel, it could have any
  arbitrary payload.

- ``RPI3_DIRECT_LINUX_BOOT``: Disabled by default. Set to 1 to enable the direct
  boot of the Linux kernel from the firmware. Option ``RPI3_PRELOADED_DTB_BASE``
  is mandatory when the direct Linux kernel boot is used. Options
  ``PRELOADED_BL33_BASE`` will most likely be needed as well because it is
  unlikely that the kernel image will fit in the space reserved for BL33 images.
  This option can be combined with ``RPI3_BL33_IN_AARCH32`` in order to boot a
  32-bit kernel. The only thing this option does is to set the arguments in
  registers x0-x3 or r0-r2 as expected by the kernel.

- ``RPI3_PRELOADED_DTB_BASE``: Auxiliary build option needed when using
  ``RPI3_DIRECT_LINUX_BOOT=1``. This option allows to specify the location of a
  DTB in memory.

- ``RPI3_RUNTIME_UART``: Indicates whether the UART should be used at runtime
  or disabled. ``-1`` (default) disables the runtime UART. Any other value
  enables the default UART (currently UART1) for runtime messages.

- ``RPI3_USE_UEFI_MAP``: Set to 1 to build ATF with the altername memory
  mapping required for an UEFI firmware payload. These changes are needed
  to be able to run Windows on ARM64. This option, which is disabled by
  default, results in the following memory mappings:

::

    0x00000000 +-----------------+
               |       ROM       | BL1
    0x00010000 +-----------------+
               |       DTB       | (Loaded by the VideoCore)
    0x00020000 +-----------------+
               |       FIP       |
    0x00030000 +-----------------+
               |                 |
               |  UEFI PAYLOAD   |
               |                 |
    0x00200000 +-----------------+
               |   Secure SRAM   | BL2, BL31
    0x00300000 +-----------------+
               |   Secure DRAM   | BL32 (Secure payload)
    0x00400000 +-----------------+
               |                 |
               |                 |
               | Non-secure DRAM | BL33
               |                 |
               |                 |
    0x01000000 +-----------------+
               |                 |
               |       ...       |
               |                 |
    0x3F000000 +-----------------+
               |       I/O       |

- ``BL32``: This port can load and run OP-TEE. The OP-TEE image is optional.
  Please use the code from `here <https://github.com/OP-TEE/optee_os>`__.
  Build the Trusted Firmware with option ``BL32=tee-header_v2.bin
  BL32_EXTRA1=tee-pager_v2.bin  BL32_EXTRA2=tee-pageable_v2.bin``
  to put the binaries into the FIP.

  .. warning::
     If OP-TEE is used it may be needed to add the following options to the
     Linux command line so that the USB driver doesn't use FIQs:
     ``dwc_otg.fiq_enable=0 dwc_otg.fiq_fsm_enable=0 dwc_otg.nak_holdoff=0``.
     This will unfortunately reduce the performance of the USB driver. It is
     needed when using Raspbian, for example.

- ``TRUSTED_BOARD_BOOT``: This port supports TBB. Set this option to 1 to enable
  it. In order to use TBB, you might want to set ``GENERATE_COT=1`` to let the
  contents of the FIP automatically signed by the build process. The ROT key
  will be generated and output to ``rot_key.pem`` in the build directory. It is
  able to set ROT_KEY to your own key in PEM format.  Also in order to build,
  you need to clone mbed TLS from `here <https://github.com/ARMmbed/mbedtls>`__.
  ``MBEDTLS_DIR`` must point at the mbed TLS source directory.

- ``ENABLE_STACK_PROTECTOR``: Disabled by default. It uses the hardware RNG of
  the board.

The following is not currently supported:

- AArch32 for TF-A itself.

- ``EL3_PAYLOAD_BASE``: The reason is that you can already load anything to any
  address by changing the file ``armstub8.bin``, so there's no point in using
  TF-A in this case.

- ``MULTI_CONSOLE_API=0``: The multi console API must be enabled. Note that the
  crash console uses the internal 16550 driver functions directly in order to be
  able to print error messages during early crashes before setting up the
  multi console API.

Building the firmware for kernels that don't support PSCI
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is the case for the 32-bit image of Raspbian, for example. 64-bit kernels
always support PSCI, but they may not know that the system understands PSCI due
to an incorrect DTB file.

First, clone and compile the 32-bit version of the `Raspberry Pi 3 TF-A
bootstrap`_. Choose the one needed for the architecture of your kernel.

Then compile TF-A. For a 32-bit kernel, use the following command line:

.. code:: shell

    CROSS_COMPILE=aarch64-linux-gnu- make PLAT=rpi3             \
    RPI3_BL33_IN_AARCH32=1                                      \
    BL33=../rpi3-arm-tf-bootstrap/aarch32/el2-bootstrap.bin

For a 64-bit kernel, use this other command line:

.. code:: shell

    CROSS_COMPILE=aarch64-linux-gnu- make PLAT=rpi3             \
    BL33=../rpi3-arm-tf-bootstrap/aarch64/el2-bootstrap.bin

However, enabling PSCI support in a 64-bit kernel is really easy. In the
repository `Raspberry Pi 3 TF-A bootstrap`_ there is a patch that can be applied
to the Linux kernel tree maintained by the Raspberry Pi foundation. It modifes
the DTS to tell the kernel to use PSCI. Once this patch is applied, follow the
instructions in `AArch64 kernel build instructions`_ to get a working 64-bit
kernel image and supporting files.

Building the firmware for kernels that support PSCI
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For a 64-bit kernel:

.. code:: shell

    CROSS_COMPILE=aarch64-linux-gnu- make PLAT=rpi3             \
    PRELOADED_BL33_BASE=0x02000000                              \
    RPI3_PRELOADED_DTB_BASE=0x01000000                          \
    RPI3_DIRECT_LINUX_BOOT=1

For a 32-bit kernel:

.. code:: shell

    CROSS_COMPILE=aarch64-linux-gnu- make PLAT=rpi3             \
    PRELOADED_BL33_BASE=0x02000000                              \
    RPI3_PRELOADED_DTB_BASE=0x01000000                          \
    RPI3_DIRECT_LINUX_BOOT=1                                    \
    RPI3_BL33_IN_AARCH32=1

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

    git clone --depth=1 -b rpi-4.18.y https://github.com/raspberrypi/linux
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
    cp arch/arm64/boot/dts/broadcom/bcm2710-rpi-3-b-plus.dtb /path/to/boot/

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
untouched). They have been tested with the image available in 2018-03-13.

1. Insert the SD card and open the ``boot`` partition.

2. Rename ``kernel7.img`` to ``kernel8.img``. This tricks the VideoCore
   bootloader into booting the Arm cores in AArch64 mode, like TF-A needs,
   even though the kernel is not compiled for AArch64.

3. Copy ``armstub8.bin`` here. When ``kernel8.img`` is available, The VideoCore
   bootloader will look for a file called ``armstub8.bin`` and load it at
   address **0x0** instead of a predefined one.

4. To enable the serial port "Mini UART" in Linux, open ``cmdline.txt`` and add
   ``console=serial0,115200 console=tty1``.

5. Open ``config.txt`` and add the following lines at the end (``enable_uart=1``
   is only needed to enable debugging through the Mini UART):

::

    enable_uart=1
    kernel_address=0x02000000
    device_tree_address=0x01000000

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
.. _Raspberry Pi 3 TF-A bootstrap: https://github.com/AntonioND/rpi3-arm-tf-bootstrap
.. _Raspberry Pi 3 documentation: https://www.raspberrypi.org/documentation/
.. _Raspbian: https://www.raspberrypi.org/downloads/raspbian/

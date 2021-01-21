Raspberry Pi 4
==============

The `Raspberry Pi 4`_ is an inexpensive single-board computer that contains four
Arm Cortex-A72 cores. Also in contrast to previous Raspberry Pi versions this
model has a GICv2 interrupt controller.

This port is a minimal port to support loading non-secure EL2 payloads such
as a 64-bit Linux kernel. Other payloads such as U-Boot or EDK-II should work
as well, but have not been tested at this point.

**IMPORTANT NOTE**: This port isn't secure. All of the memory used is DRAM,
which is available from both the Non-secure and Secure worlds. The SoC does
not seem to feature a secure memory controller of any kind, so portions of
DRAM can't be protected properly from the Non-secure world.

Build Instructions
------------------

There are no real configuration options at this point, so there is only
one universal binary (bl31.bin), which can be built with:

.. code:: shell

    CROSS_COMPILE=aarch64-linux-gnu- make PLAT=rpi4 DEBUG=1

Copy the generated build/rpi4/debug/bl31.bin to the SD card, adding an entry
starting with ``armstub=``, then followed by the respective file name to
``config.txt``. You should have AArch64 code in the file loaded as the
"kernel", as BL31 will drop into AArch64/EL2 to the respective load address.
arm64 Linux kernels are known to work this way.

Other options that should be set in ``config.txt`` to properly boot 64-bit
kernels are:

::

    enable_uart=1
    arm_64bit=1
    enable_gic=1

The BL31 code will patch the provided device tree blob in memory to advertise
PSCI support, also will add a reserved-memory node to the DT to tell the
non-secure payload to not touch the resident TF-A code.

If you connect a serial cable between the Mini UART and your computer, and
connect to it (for example, with ``screen /dev/ttyUSB0 115200``) you should
see some text from BL31, followed by the output of the EL2 payload.
The command line provided is read from the ``cmdline.txt`` file on the SD card.

TF-A port design
----------------

In contrast to the existing Raspberry Pi 3 port this one here is a BL31-only
port, also it deviates quite a lot from the RPi3 port in many other ways.
There is not so much difference between the two models, so eventually those
two could be (more) unified in the future.

As with the previous models, the GPU and its firmware are the first entity to
run after the SoC gets its power. The on-chip Boot ROM loads the next stage
(bootcode.bin) from flash (EEPROM), which is again GPU code.
This part knows how to access the MMC controller and how to parse a FAT
filesystem, so it will load further components and configuration files
from the first FAT partition on the SD card.

To accommodate this existing way of configuring and setting up the board,
we use as much of this workflow as possible.
If bootcode.bin finds a file called ``armstub8.bin`` on the SD card or it gets
pointed to such code by finding a ``armstub=`` key in ``config.txt``, it will
load this file to the beginning of DRAM (address 0) and execute it in
AArch64 EL3.
But before doing that, it will also load a "kernel" and the device tree into
memory. The load addresses have a default, but can also be changed by
setting them in ``config.txt``. If the GPU firmware finds a magic value in the
armstub image file, it will put those two load addresses in memory locations
near the beginning of memory, where TF-A code picks them up.

To keep things simple, we will just use the kernel load address as the BL33
entry point, also put the DTB address in the x0 register, as requested by
the arm64 Linux kernel boot protocol. This does not necessarily mean that
the EL2 payload needs to be a Linux kernel, a bootloader or any other kernel
would work as well, as long as it can cope with having the DT address in
register x0. If the payload has other means of finding the device tree, it
could ignore this address as well.

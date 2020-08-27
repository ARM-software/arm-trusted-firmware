Arm FPGA Platform
=================

This platform supports FPGA images used internally in Arm Ltd., for
testing and bringup of new cores. With that focus, peripheral support is
minimal: there is no mass storage or display output, for instance. Also
this port ignores any power management features of the platform.
Some interconnect setup is done internally by the platform, so the TF-A code
just needs to setup UART and GIC.

The FPGA platform requires to pass on a DTB for the non-secure payload
(mostly Linux), so we let TF-A use information from the DTB for dynamic
configuration: the UART and GIC base addresses are read from there.

As a result this port is a fairly generic BL31-only port, which can serve
as a template for a minimal new (and possibly DT-based) platform port.

The aim of this port is to support as many FPGA images as possible with
a single build. Image specific data must be described in the DTB or should
be auto-detected at runtime.

As the number and topology layout of the CPU cores differs significantly
across the various images, this is detected at runtime by BL31.
The /cpus node in the DT will be added and filled accordingly, as long as
it does not exist already.

Platform-specific build options
-------------------------------

-  ``SUPPORT_UNKNOWN_MPID`` : Boolean option to allow unknown MPIDR registers.
   Normally TF-A panics if it encounters a MPID value not matched to its
   internal list, but for new or experimental cores this creates a lot of
   churn. With this option, the code will fall back to some basic CPU support
   code (only architectural system registers, and no errata).
   Default value of this flag is 1.

-  ``PRELOADED_BL33_BASE`` : Physical address of the BL33 non-secure payload.
   It must have been loaded into DRAM already, typically this is done by
   the script that also loads BL31 and the DTB.
   It defaults to 0x80080000, which is the traditional load address for an
   arm64 Linux kernel.

-  ``FPGA_PRELOADED_DTB_BASE`` : Physical address of the flattened device
   tree blob (DTB). This DT will be used by TF-A for dynamic configuration,
   so it must describe at least the UART and a GICv3 interrupt controller.
   The DT gets amended by the code, to potentially add a command line and
   fill the CPU topology nodes. It will also be passed on to BL33, by
   putting its address into the x0 register before jumping to the entry
   point (following the Linux kernel boot protocol).
   It defaults to 0x80070000, which is 64KB before the BL33 load address.

-  ``FPGA_PRELOADED_CMD_LINE`` : Physical address of the command line to
   put into the devicetree blob. Due to the lack of a proper bootloader,
   a command line can be put somewhere into memory, so that BL31 will
   detect it and copy it into the DTB passed on to BL33.
   To avoid random garbage, there needs to be a "CMD:" signature before the
   actual command line.
   Defaults to 0x1000, which is normally in the "ROM" space of the typical
   FPGA image (which can be written by the FPGA payload uploader, but is
   read-only to the CPU). The FPGA payload tool should be given a text file
   containing the desired command line, prefixed by the "CMD:" signature.

Building the TF-A image
-----------------------

   .. code:: shell

       make PLAT=arm_fgpa DEBUG=1

   This will use the default load addresses as described above. When those
   addresses need to differ for a certain setup, they can be passed on the
   make command line:

   .. code:: shell

       make PLAT=arm_fgpa DEBUG=1 PRELOADED_BL33_BASE=0x80200000 FPGA_PRELOADED_DTB_BASE=0x80180000 bl31

Running the TF-A image
----------------------

After building TF-A, the actual TF-A code will be located in ``bl31.bin`` in
the build directory.
Additionally there is a ``bl31.axf`` ELF file, which contains BL31, as well
as some simple ROM trampoline code (required by the Arm FPGA boot flow) and
a generic DTB to support most of the FPGA images. This can be simply handed
over to the FPGA payload uploader, which will take care of loading the
components at their respective load addresses. In addition to this file
you need at least a BL33 payload (typically a Linux kernel image), optionally
a Linux initrd image file and possibly a command line:

   .. code:: shell

       fpga-run ... -m bl31.axf -l auto -m Image -l 0x80080000 -m initrd.gz -l 0x84000000 -m cmdline.txt -l 0x1000

--------------

*Copyright (c) 2020, Arm Limited. All rights reserved.*

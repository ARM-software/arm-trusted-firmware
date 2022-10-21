NXP i.MX 8M Series
==================

The i.MX 8M family of applications processors based on Arm Corte-A53 and Cortex-M4
cores provide high-performance computing, power efficiency, enhanced system
reliability and embedded security needed to drive the growth of fast-growing
edge node computing, streaming multimedia, and machine learning applications.

imx8mq is dropped in TF-A CI build due to the small OCRAM size, but still actively
maintained in NXP official release.

Boot Sequence
-------------

Bootrom --> SPL --> BL31 --> BL33(u-boot) --> Linux kernel

How to build
------------

Build Procedure
~~~~~~~~~~~~~~~

-  Prepare AARCH64 toolchain.

-  Build spl and u-boot firstly, and get binary images: u-boot-spl.bin,
   u-boot-nodtb.bin and dtb for the target board.

-  Build TF-A

   Build bl31:

   .. code:: shell

       CROSS_COMPILE=aarch64-linux-gnu- make PLAT=<Target_SoC> bl31

   Target_SoC should be "imx8mq" for i.MX8MQ SoC.
   Target_SoC should be "imx8mm" for i.MX8MM SoC.
   Target_SoC should be "imx8mn" for i.MX8MN SoC.
   Target_SoC should be "imx8mp" for i.MX8MP SoC.

Deploy TF-A Images
~~~~~~~~~~~~~~~~~~

TF-A binary(bl31.bin), u-boot-spl.bin u-boot-nodtb.bin and dtb are combined
together to generate a binary file called flash.bin, the imx-mkimage tool is
used to generate flash.bin, and flash.bin needs to be flashed into SD card
with certain offset for BOOT ROM. the u-boot and imx-mkimage will be upstreamed
soon, this doc will be updated once they are ready, and the link will be posted.

TBBR Boot Sequence
------------------

When setting NEED_BL2=1 on imx8mm. We support an alternative way of
boot sequence to support TBBR.

Bootrom --> SPL --> BL2 --> BL31 --> BL33(u-boot with UEFI) --> grub

This helps us to fulfill the SystemReady EBBR standard.
BL2 will be in the FIT image and SPL will verify it.
All of the BL3x will be put in the FIP image. BL2 will verify them.
In U-boot we turn on the UEFI secure boot features so it can verify
grub. And we use grub to verify linux kernel.

Measured Boot
-------------

When setting MEASURED_BOOT=1 on imx8mm we can let TF-A generate event logs
with a DTB overlay. The overlay will be put at PLAT_IMX8M_DTO_BASE with
maximum size PLAT_IMX8M_DTO_MAX_SIZE. Then in U-boot we can apply the DTB
overlay and let U-boot to parse the event log and update the PCRs.

High Assurance Boot (HABv4)
---------------------------

All actively maintained platforms have a support for High Assurance
Boot (HABv4), which is implemented via ROM Vector Table (RVT) API to
extend the Root-of-Trust beyond the SPL. Those calls are done via SMC
and are executed in EL3, with results returned back to original caller.

Note on DRAM Memory Mapping
~~~~~~~~~~~~~~~~~~~~~~~~~~~

There is a special case of mapping the DRAM: entire DRAM available on the
platform is mapped into the EL3 with MT_RW attributes.

Mapping the entire DRAM allows the usage of 2MB block mapping in Level-2
Translation Table entries, which use less Page Table Entries (PTEs). If
Level-3 PTE mapping is used instead then additional PTEs would be required,
which leads to the increase of translation table size.

Due to the fact that the size of SRAM is limited on some platforms in the
family it should rather be avoided creating additional Level-3 mapping and
introduce more PTEs, hence the implementation uses Level-2 mapping which
maps entire DRAM space.

The reason for the MT_RW attribute mapping scheme is the fact that the SMC
API to get the status and events is called from NS world passing destination
pointers which are located in DRAM. Mapping DRAM without MT_RW permissions
causes those locations not to be filled, which in turn causing EL1&0 software
not to receive replies.

Therefore, DRAM mapping is done with MT_RW attributes, as it is required for
data exchange between EL3 and EL1&0 software.

Reference Documentation
~~~~~~~~~~~~~~~~~~~~~~~

Details on HABv4 usage and implementation could be found in following documents:

- AN4581: "i.MX Secure Boot on HABv4 Supported Devices",  Rev. 4 - June 2020
- AN12263: "HABv4 RVT Guidelines and Recommendations", Rev. 1 - 06/2020
- "HABv4 API Reference Manual". This document in the part of NXP Code Signing Tool (CST) distribution.


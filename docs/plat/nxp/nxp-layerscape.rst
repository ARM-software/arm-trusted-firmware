NXP SoCs - Overview
=====================

The QorIQ family of ARM based SoCs that are supported on TF-A are:

LS1043ARDB:
	Platform Name:
	a. ls1043ardb (Board details can be fetched from the link: `ls1043ardb`_)


Table of Supported Boot-Modes by each platform:
==============================================

                    |  SD   |  QSPI | NOR   | NAND  | eMMC  | Flexspi-NOR | Flexspi-NAND |
------------------------------------------------------------------------------------------
1.| ls1043ardb      | yes   |       |  yes  |  yes  |       |             |              |
------------------------------------------------------------------------------------------
2.| ls1043aqds      | yes   | yes   |  yes  |  yes  |       |             |              |
------------------------------------------------------------------------------------------


Boot Sequence
=============
                          Secure World        |   Normal World
EL0                                           |
                                              |
EL1                           BL32(Tee OS)    |   kernel
                               ^ |            |     ^
		               | |            |     |
EL2                            | |            |   BL33(u-boot)
	                       | |            |    ^
                               | v            |   /
EL3        BootROM --> BL2 --> BL31 -------------/


How to build
============

Code Locations
--------------

-  OP-TEE:
   `link <https://source.codeaurora.org/external/qoriq/qoriq-components/optee_os>`__

-  U-Boot:
   `link <https://source.codeaurora.org/external/qoriq/qoriq-components/u-boot>`__

-  RCW:
   `link <https://source.codeaurora.org/external/qoriq/qoriq-components/rcw>`__

-  CST:
   `link <https://source.codeaurora.org/external/qoriq/qoriq-components/cst>`__

Build Procedure
---------------

-  Fetch all the above repositories into local host.

-  Prepare AARCH64 toolchain and set the environment variable "CROSS_COMPILE".
   .. code:: shell

       CROSS_COMPILE=.../bin/aarch64-linux-gnu-

-  Build RCW. Refer README from the respective cloned folder for more details.

-  Build CST in case of secure-boot. Refer README from the respective cloned folder for more details.

-  Build u-boot and OPTee firstly, and get binary images: u-boot.bin and tee.bin
   For u-boot you can use the <platform>_tfa_defconfig for build

-  Below are the steps to build TF-A images for the supported platforms.

BUILD BL2:

-To compile
   .. code:: shell

       make PLAT=<platform_name>ardb bl2 BOOT_MODE=<any_one_of_the_supported_boot_mode_by_the_platform> pbl RCW_PATH=<RCW_file_name_with_path>

BUILD FIP:

-To compile without OPTEE and without Trusted Board Boot.
   .. code:: shell

	make PLAT=<platform_name> fip BOOT_MODE=<any_one_of_the_supported_boot_mode_by_the_platform> BL33=u-boot-dtb.bin

-To compile with OPTEE and without Trusted Board Boot.
   .. code:: shell

	make PLAT=<platform_name> fip BOOT_MODE=<any_one_of_the_supported_boot_mode_by_the_platform> BL33=u-boot-dtb.bin SPD=opteed BL32=<tee.bin>

-To compile with Trusted Board Boot.
	Refer the readme at ./plat/nxp/README.TRUSTED_BOOT


Deploy ATF Images
-----------------
Note: The size in the standard uboot commands for copy to nor, qspi, nand or sd
should be modified based on the binary size of the image to be copied.

-  Deploy ATF images on Nor flash Alt Bank from U-Boot prompt.

   .. code:: shell

       => tftp 82000000  $path/bl2_nor.pbl;
       =>pro off all;era 64000000 +$filesize;cp.b 82000000 64000000 $filesize

       => tftp 82000000  $path/fip.bin;
       =>pro off all;era 64100000 +$filesize;cp.b 82000000 64100000 $filesize

    Then change to Alt bank and boot up ATF:

    .. code:: shell

       => cpld reset altbank

-  Deploy ATF images on SD card from U-Boot prompt.

   .. code:: shell

       => tftp 82000000 $path/bl2_sd.pbl
       => mmc write 82000000 8 150

       => tftp 82000000  $path/fip.bin; mmc write 82000000 800 B00

    Then change to SD boot and boot up ATF:

    .. code:: shell

       => cpld reset sd (For platforms with cpld as FPGA)
       or
       => qixis reset sd (For platforms with Qixis)

       Please refer to the board manual for the right command.

-  Deploy ATF images on NAND car from U-Boot prompt.

   .. code:: shell

	=> tftp 82000000 $path/bl2_nand.pbl
	=> nand erase 0x0 0x100000;nand write 0x82000000 0x0 0x100000;
	=> tftp 82000000  $path/fip.bin
	=> nand erase 0x100000 0x100000;nand write 0x82000000 0x100000 0x100000;
    Then change to SD boot and boot up ATF:

    .. code:: shell

	=> cpld reset nand
        or
        => qixis reset nand (For platforms with Qixis)

-  Deploy ATF images on QSPI flash from U-Boot prompt.

   .. code:: shell

       => tftp 82000000 $path/bl2_qspi.pbl
       => sf erase 0x0 0x40000; sf write 0x80000000 0x0 0x40000;


       => tftp 82000000  $path/fip.bin;
       => sf erase 0x100000 0x100000; sf write 0x80000000 0x100000 0x100000;

        Note : Please select the flash you want to write to by doing sf probe
        as required on the platform.

    Then change to QSPI boot and boot up ATF:

    .. code:: shell

       => cpld reset qspi (For platforms with cpld as FPGA)
       or
       => qixis reset qspi (For platforms with Qixis)

For trusted boot, please refer to docs/plat/nxp-layerscape-tb.rst

.. _ls1043ardb:https://www.nxp.com/support/developer-resources/software-development-tools/qoriq-developer-resources/qoriq-ls1043a-reference-design-board:LS1043A-RDB


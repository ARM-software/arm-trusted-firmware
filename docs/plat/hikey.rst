Description
===========

HiKey is one of 96boards. Hisilicon Kirin6220 processor is installed on HiKey.

More information are listed in `link`_.

How to build
============

Code Locations
--------------

-  ARM Trusted Firmware:
   `link <https://github.com/ARM-software/arm-trusted-firmware>`__

-  edk2:
   `link <https://github.com/96boards-hikey/edk2/tree/testing/hikey960_v2.5>`__

-  OpenPlatformPkg:
   `link <https://github.com/96boards-hikey/OpenPlatformPkg/tree/testing/hikey960_v1.3.4>`__

-  l-loader:
   `link <https://github.com/96boards-hikey/l-loader/tree/testing/hikey960_v1.2>`__

-  uefi-tools:
   `link <https://github.com/96boards-hikey/uefi-tools/tree/testing/hikey960_v1>`__

-  atf-fastboot:
   `link <https://github.com/96boards-hikey/atf-fastboot/tree/master>`__

Build Procedure
---------------

-  Fetch all the above repositories into local host.
   Make all the repositories in the same ${BUILD\_PATH}.

-  Create the symbol link to OpenPlatformPkg in edk2.

   .. code:: shell

       $cd ${BUILD_PATH}/edk2
       $ln -sf ../OpenPlatformPkg

-  Prepare AARCH64 && AARCH32 toolchain. Prepare python.

-  If your hikey hardware is built by CircuitCo, update *uefi-tools/platform.config* first. *(optional)*
   **Uncomment the below sentence. Otherwise, UEFI can't output messages on serial
   console on hikey.**

   .. code:: shell

       BUILDFLAGS=-DSERIAL_BASE=0xF8015000

   If your hikey hardware is built by LeMarker, nothing to do.

-  Build it as debug mode. Create your own build script file or you could refer to **build\_uefi.sh** in l-loader git repository.

   .. code:: shell

       BUILD_OPTION=DEBUG
       export AARCH64_TOOLCHAIN=GCC5
       export UEFI_TOOLS_DIR=${BUILD_PATH}/uefi-tools
       export EDK2_DIR=${BUILD_PATH}/edk2
       EDK2_OUTPUT_DIR=${EDK2_DIR}/Build/HiKey/${BUILD_OPTION}_${AARCH64_TOOLCHAIN}
       # Build fastboot for ARM Trust Firmware. It's used for recovery mode.
       cd ${BUILD_PATH}/atf-fastboot
       CROSS_COMPILE=aarch64-linux-gnu- make PLAT=hikey DEBUG=1
       # Convert DEBUG/RELEASE to debug/release
       FASTBOOT_BUILD_OPTION=$(echo ${BUILD_OPTION} | tr '[A-Z]' '[a-z]')
       cd ${EDK2_DIR}
       # Build UEFI & ARM Trust Firmware
       ${UEFI_TOOLS_DIR}/uefi-build.sh -b ${BUILD_OPTION} -a ../arm-trusted-firmware hikey
       # Generate l-loader.bin
       cd ${BUILD_PATH}/l-loader
       ln -sf ${EDK2_OUTPUT_DIR}/FV/bl1.bin
       ln -sf ${EDK2_OUTPUT_DIR}/FV/fip.bin
       ln -sf ${BUILD_PATH}/atf-fastboot/build/hikey/${FASTBOOT_BUILD_OPTION}/bl1.bin fastboot.bin
       python gen_loader.py -o l-loader.bin --img_bl1=bl1.bin --img_ns_bl1u=BL33_AP_UEFI.fd
       arm-linux-gnueabihf-gcc -c -o start.o start.S
       arm-linux-gnueabihf-ld -Bstatic -Tl-loader.lds -Ttext 0xf9800800 start.o -o loader
       arm-linux-gnueabihf-objcopy -O binary loader temp
       python gen_loader_hikey.py -o l-loader.bin --img_loader=temp --img_bl1=bl1.bin --img_ns_bl1u=fastboot.bin

-  Generate partition table for aosp. The eMMC capacity is either 4GB or 8GB. Just change "aosp-4g" to "linux-4g" for debian.

   .. code:: shell

       $PTABLE=aosp-4g SECTOR_SIZE=512 bash -x generate_ptable.sh

Setup Console
-------------

-  Install ser2net. Use telnet as the console since UEFI fails to display Boot Manager GUI in minicom. **If you don't need Boot Manager GUI, just ignore this section.**

   .. code:: shell

       $sudo apt-get install ser2net

-  Configure ser2net.

   .. code:: shell

       $sudo vi /etc/ser2net.conf

   Append one line for serial-over-USB in below.
   *#ser2net.conf*

   .. code:: shell

       2004:telnet:0:/dev/ttyUSB0:115200 8DATABITS NONE 1STOPBIT banner

-  Open the console.

   .. code:: shell

       $telnet localhost 2004

   And you could open the console remotely, too.

Flush images in recovery mode
-----------------------------

-  Make sure Pin3-Pin4 on J15 are connected for recovery mode. Then power on HiKey.

-  Remove the modemmanager package. This package may cause the idt tool failure.

   .. code:: shell

       $sudo apt-get purge modemmanager

-  Run the command to download l-loader.bin into HiKey.

   .. code:: shell

       $sudo python hisi-idt.py -d /dev/ttyUSB1 --img1 l-loader.bin

-  Update images. All aosp or debian images could be fetched from `link <https://builds.96boards.org/>`__.

   .. code:: shell

       $sudo fastboot flash ptable prm_ptable.img
       $sudo fastboot flash fastboot fip.bin
       $sudo fastboot flash boot boot.img
       $sudo fastboot flash cache cache.img
       $sudo fastboot flash system system.img
       $sudo  fastboot flash userdata userdata.img

Boot UEFI in normal mode
------------------------

-  Make sure Pin3-Pin4 on J15 are open for normal boot mode. Then power on HiKey.

-  Reference `link <https://github.com/96boards-hikey/tools-images-hikey960/blob/master/build-from-source/README-ATF-UEFI-build-from-source.md>`__

.. _link: https://github.com/96boards/documentation/blob/master/ConsumerEdition/HiKey/Quickstart/README.md

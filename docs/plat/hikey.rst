Description
===========

HiKey is one of 96boards. Hisilicon Kirin6220 processor is installed on HiKey.

More information are listed in `link`_.

How to build
============

Code Locations
--------------

-  Trusted Firmware-A:
   `link <https://github.com/ARM-software/arm-trusted-firmware>`__

-  OP-TEE
   `link <https://github.com/OP-TEE/optee_os>`__

-  edk2:
   `link <https://github.com/96boards-hikey/edk2/tree/testing/hikey960_v2.5>`__

-  OpenPlatformPkg:
   `link <https://github.com/96boards-hikey/OpenPlatformPkg/tree/testing/hikey960_v1.3.4>`__

-  l-loader:
   `link <https://github.com/96boards-hikey/l-loader/tree/testing/hikey960_v1.2>`__

-  uefi-tools:
   `link <https://git.linaro.org/uefi/uefi-tools.git>`__

-  atf-fastboot:
   `link <https://github.com/96boards-hikey/atf-fastboot/tree/master>`__

Build Procedure
---------------

-  Fetch all the above repositories into local host.
   Make all the repositories in the same ${BUILD\_PATH}.

  .. code:: shell

       git clone https://github.com/ARM-software/arm-trusted-firmware -b integration
       git clone https://github.com/OP-TEE/optee_os
       git clone https://github.com/96boards-hikey/edk2 -b testing/hikey960_v2.5
       git clone https://github.com/96boards-hikey/OpenPlatformPkg -b testing/hikey960_v1.3.4
       git clone https://github.com/96boards-hikey/l-loader -b testing/hikey960_v1.2
       git clone https://git.linaro.org/uefi/uefi-tools
       git clone https://github.com/96boards-hikey/atf-fastboot

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

   If your hikey hardware is built by LeMaker, nothing to do.

-  Build it as debug mode. Create your own build script file or you could refer to **build\_uefi.sh** in l-loader git repository.

   .. code:: shell

       BUILD_OPTION=DEBUG
       export AARCH64_TOOLCHAIN=GCC5
       export UEFI_TOOLS_DIR=${BUILD_PATH}/uefi-tools
       export EDK2_DIR=${BUILD_PATH}/edk2
       EDK2_OUTPUT_DIR=${EDK2_DIR}/Build/HiKey/${BUILD_OPTION}_${AARCH64_TOOLCHAIN}
       # Build fastboot for Trusted Firmware-A. It's used for recovery mode.
       cd ${BUILD_PATH}/atf-fastboot
       CROSS_COMPILE=aarch64-linux-gnu- make PLAT=hikey DEBUG=1
       # Convert DEBUG/RELEASE to debug/release
       FASTBOOT_BUILD_OPTION=$(echo ${BUILD_OPTION} | tr '[A-Z]' '[a-z]')
       cd ${EDK2_DIR}
       # Build UEFI & Trusted Firmware-A
       ${UEFI_TOOLS_DIR}/uefi-build.sh -b ${BUILD_OPTION} -a ../arm-trusted-firmware -s ../optee_os hikey

-  Generate l-loader.bin and partition table for aosp. The eMMC capacity is either 8GB or 4GB. Just change "aosp-8g" to "linux-8g" for debian.

   .. code:: shell

       cd ${BUILD_PATH}/l-loader
       ln -sf ${EDK2_OUTPUT_DIR}/FV/bl1.bin
       ln -sf ${EDK2_OUTPUT_DIR}/FV/bl2.bin
       ln -sf ${BUILD_PATH}/atf-fastboot/build/hikey/${FASTBOOT_BUILD_OPTION}/bl1.bin fastboot.bin
       make hikey PTABLE_LST=aosp-8g

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

-  Start ser2net

   .. code:: shell

       $sudo killall ser2net
       $sudo ser2net -u

-  Open the console.

   .. code:: shell

       $telnet localhost 2004

   And you could open the console remotely, too.

Flash images in recovery mode
-----------------------------

-  Make sure Pin3-Pin4 on J15 are connected for recovery mode. Then power on HiKey.

-  Remove the modemmanager package. This package may cause the idt tool failure.

   .. code:: shell

       $sudo apt-get purge modemmanager

-  Run the command to download recovery.bin into HiKey.

   .. code:: shell

       $sudo python hisi-idt.py -d /dev/ttyUSB1 --img1 recovery.bin

-  Update images. All aosp or debian images could be fetched from `link <http://releases.linaro.org/96boards/>`__.

   .. code:: shell

       $sudo fastboot flash ptable prm_ptable.img
       $sudo fastboot flash loader l-loader.bin
       $sudo fastboot flash fastboot fip.bin
       $sudo fastboot flash boot boot.img
       $sudo fastboot flash cache cache.img
       $sudo fastboot flash system system.img
       $sudo  fastboot flash userdata userdata.img

Boot UEFI in normal mode
------------------------

-  Make sure Pin3-Pin4 on J15 are open for normal boot mode. Then power on HiKey.

-  Reference `link <https://github.com/96boards-hikey/tools-images-hikey960/blob/master/build-from-source/README-ATF-UEFI-build-from-source.md>`__

.. _link: https://www.96boards.org/documentation/consumer/hikey/

Description
===========

HiKey960 is one of 96boards. Hisilicon Hi3660 processor is installed on HiKey960.

More information are listed in `link`_.

How to build
============

Code Locations
--------------

-  ARM Trusted Firmware:
   `link <https://github.com/ARM-software/arm-trusted-firmware>`__

-  OP-TEE:
   `link <https://github.com/OP-TEE/optee_os>`__

-  edk2:
   `link <https://github.com/96boards-hikey/edk2/tree/testing/hikey960_v2.5>`__

-  OpenPlatformPkg:
   `link <https://github.com/96boards-hikey/OpenPlatformPkg/tree/testing/hikey960_v1.3.4>`__

-  l-loader:
   `link <https://github.com/96boards-hikey/l-loader/tree/testing/hikey960_v1.2>`__

-  uefi-tools:
   `link <https://git.linaro.org/uefi/uefi-tools.git>`__

Build Procedure
---------------

-  Fetch all the above 5 repositories into local host.
   Make all the repositories in the same ${BUILD\_PATH}.

  .. code:: shell

       git clone https://github.com/ARM-software/arm-trusted-firmware -b integration
       git clone https://github.com/OP-TEE/optee_os
       git clone https://github.com/96boards-hikey/edk2 -b testing/hikey960_v2.5
       git clone https://github.com/96boards-hikey/OpenPlatformPkg -b testing/hikey960_v1.3.4
       git clone https://github.com/96boards-hikey/l-loader -b testing/hikey960_v1.2
       git clone https://git.linaro.org/uefi/uefi-tools

-  Create the symbol link to OpenPlatformPkg in edk2.

   .. code:: shell

       $cd ${BUILD_PATH}/edk2
       $ln -sf ../OpenPlatformPkg

-  Prepare AARCH64 toolchain.

-  If your hikey960 hardware is v1, update *uefi-tools/platform.config* first. *(optional)*
   **Uncomment the below sentence. Otherwise, UEFI can't output messages on serial
   console on hikey960 v1.**

   .. code:: shell

       BUILDFLAGS=-DSERIAL_BASE=0xFDF05000

   If your hikey960 hardware is v2 or newer, nothing to do.

-  Build it as debug mode. Create script file for build.

   .. code:: shell

       BUILD_OPTION=DEBUG
       export AARCH64_TOOLCHAIN=GCC5
       export UEFI_TOOLS_DIR=${BUILD_PATH}/uefi-tools
       export EDK2_DIR=${BUILD_PATH}/edk2
       EDK2_OUTPUT_DIR=${EDK2_DIR}/Build/HiKey960/${BUILD_OPTION}_${AARCH64_TOOLCHAIN}
       cd ${EDK2_DIR}
       # Build UEFI & ARM Trust Firmware
       ${UEFI_TOOLS_DIR}/uefi-build.sh -b ${BUILD_OPTION} -a ../arm-trusted-firmware -s ../optee_os hikey960

-  Generate l-loader.bin and partition table.
   *Make sure that you're using the sgdisk in the l-loader directory.*

   .. code:: shell

       cd ${BUILD_PATH}/l-loader
       ln -sf ${EDK2_OUTPUT_DIR}/FV/bl1.bin
       ln -sf ${EDK2_OUTPUT_DIR}/FV/fip.bin
       ln -sf ${EDK2_OUTPUT_DIR}/FV/BL33_AP_UEFI.fd
       make hikey960

Setup Console
-------------

-  Install ser2net. Use telnet as the console since UEFI will output window
   that fails to display in minicom.

   .. code:: shell

       $sudo apt-get install ser2net

-  Configure ser2net.

   .. code:: shell

       $sudo vi /etc/ser2net.conf

   Append one line for serial-over-USB in *#ser2net.conf*

   ::

       2004:telnet:0:/dev/ttyUSB0:115200 8DATABITS NONE 1STOPBIT banner

-  Start ser2net

   .. code:: shell

       $sudo killall ser2net
       $sudo ser2net -u

-  Open the console.

   .. code:: shell

       $telnet localhost 2004

   And you could open the console remotely, too.

Boot UEFI in recovery mode
--------------------------

-  Fetch that are used in recovery mode. The code location is in below.
   `link <https://github.com/96boards-hikey/tools-images-hikey960>`__

-  Generate l-loader.bin.

   .. code:: shell

       $cd tools-images-hikey960
       $ln -sf ${BUILD_PATH}/l-loader/l-loader.bin
       $ln -sf ${BUILD_PATH}/l-loader/fip.bin

-  Prepare config file.

   .. code:: shell

       $vi config
       # The content of config file
       ./sec_usb_xloader.img 0x00020000
       ./sec_uce_boot.img 0x6A908000
       ./l-loader.bin 0x1AC00000

-  Remove the modemmanager package. This package may causes hikey\_idt tool failure.

   .. code:: shell

       $sudo apt-get purge modemmanager

-  Run the command to download l-loader.bin into HiKey960.

   .. code:: shell

       $sudo ./hikey_idt -c config -p /dev/ttyUSB1

-  UEFI running in recovery mode.
   When prompt '.' is displayed on console, press hotkey 'f' in keyboard. Then Android fastboot app is running.
   The timeout of prompt '.' is 10 seconds.

-  Update images.

   .. code:: shell

       $sudo fastboot flash ptable prm_ptable.img
       $sudo fastboot flash xloader sec_xloader.img
       $sudo fastboot flash fastboot l-loader.bin
       $sudo fastboot flash fip fip.bin
       $sudo fastboot flash boot boot.img
       $sudo fastboot flash cache cache.img
       $sudo fastboot flash system system.img
       $sudo fastboot flash userdata userdata.img

-  Notice: UEFI could also boot kernel in recovery mode, but BL31 isn't loaded in
   recovery mode.

Boot UEFI in normal mode
------------------------

-  Make sure "Boot Mode" switch is OFF for normal boot mode. Then power on HiKey960.

-  Reference `link <https://github.com/96boards-hikey/tools-images-hikey960/blob/master/build-from-source/README-ATF-UEFI-build-from-source.md>`__

.. _link: http://www.96boards.org/documentation/ConsumerEdition/HiKey960/README.md

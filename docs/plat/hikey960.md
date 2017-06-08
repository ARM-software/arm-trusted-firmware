
Description
====================
  HiKey960 is one of 96boards. Hisilicon Hi3660 processor is installed on HiKey960.

  More information are listed in [link](http://www.96boards.org/documentation/ConsumerEdition/HiKey960/README.md).


How to build
====================

1. Code Locations
-----------------

   * ARM Trusted Firmware:
   [link](https://github.com/ARM-software/arm-trusted-firmware)

   * edk2:
   [link](https://github.com/96boards-hikey/edk2/tree/testing/hikey960_v2.5)

   * OpenPlatformPkg:
   [link](https://github.com/96boards-hikey/OpenPlatformPkg/tree/testing/hikey960_v1.3.4)

   * l-loader:
   [link](https://github.com/96boards-hikey/l-loader/tree/testing/hikey960_v1.2)

   * uefi-tools:
   [link](https://github.com/96boards-hikey/uefi-tools/tree/hikey960_v1)


2. Build Procedure
------------------

   * Fetch all the above 5 repositories into local host.
     Make all the repositories in the same ${BUILD_PATH}.

   * Create the symbol link to OpenPlatformPkg in edk2.
     <br>`$cd ${BUILD_PATH}/edk2`</br>
     <br>`$ln -sf ../OpenPlatformPkg`</br>

   * Prepare AARCH64 toolchain.

   * If your hikey960 hardware is v1, update _uefi-tools/platform.config_ first. _(optional)_
     <br>__Uncomment the below sentence. Otherwise, UEFI can't output messages on serial
     console on hikey960 v1.__</br>
     <br>`BUILDFLAGS=-DSERIAL_BASE=0xFDF05000`</br>
     <br>If your hikey960 hardware is v2 or newer, nothing to do.</br>

   * Build it as debug mode. Create script file for build.
     <br>`BUILD_OPTION=DEBUG`</br>
     <br>`export AARCH64_TOOLCHAIN=GCC48`</br>
     <br>`export UEFI_TOOLS_DIR=${BUILD_PATH}/uefi-tools`<br>
     <br>`export EDK2_DIR=${BUILD_PATH}/edk2`</br>
     <br>`EDK2_OUTPUT_DIR=${EDK2_DIR}/Build/HiKey960/${BUILD_OPTION}_${AARCH64_TOOLCHAIN}`</br>
     <br>`cd ${EDK2_DIR}`</br>
     <br>`# Build UEFI & ARM Trust Firmware`</br>
     <br>`${UEFI_TOOLS_DIR}/uefi-build.sh -b ${BUILD_OPTION} -a ../arm-trusted-firmware hikey960`</br>
     <br>`# Generate l-loader.bin`</br>
     <br>`cd ${BUILD_PATH}/l-loader`</br>
     <br>`ln -sf ${EDK2_OUTPUT_DIR}/FV/bl1.bin`</br>
     <br>`ln -sf ${EDK2_OUTPUT_DIR}/FV/fip.bin`</br>
     <br>`ln -sf ${EDK2_OUTPUT_DIR}/FV/BL33_AP_UEFI.fd`</br>
     <br>`python gen_loader.py -o l-loader.bin --img_bl1=bl1.bin --img_ns_bl1u=BL33_AP_UEFI.fd`</br>

   * Generate partition table.
     <br>_Make sure that you're using the sgdisk in the l-loader directory._</br>
     <br>`$PTABLE=aosp-32g SECTOR_SIZE=4096 SGDISK=./sgdisk bash -x generate_ptable.sh`</br>


3. Setup Console
----------------

   * Install ser2net. Use telnet as the console since UEFI will output window
     that fails to display in minicom.
     <br>`$sudo apt-get install ser2net`</br>

   * Configure ser2net.
     <br>`$sudo vi /etc/ser2net.conf`</br>
     <br>Append one line for serial-over-USB in below.</br>
     <br>_#ser2net.conf_</br>
     <br>`2004:telnet:0:/dev/ttyUSB0:115200 8DATABITS NONE 1STOPBIT banner`</br>

   * Open the console.
     <br>`$telnet localhost 2004`</br>
     <br>And you could open the console remotely, too.</br>


4. Boot UEFI in recovery mode
-----------------------------

   * Fetch that are used in recovery mode. The code location is in below.
     [link](https://github.com/96boards-hikey/tools-images-hikey960)

   * Generate l-loader.bin.
     <br>`$cd tools-images-hikey960`</br>
     <br>`$ln -sf ${BUILD_PATH}/l-loader/l-loader.bin`</br>

   * Prepare config file.
     <br>_$vi config_</br>
     <br>_# The content of config file_</br>
     <br>`./sec_user_xloader.img 0x00020000`</br>
     <br>`./sec_uce_boot.img 0x6A908000`</br>
     <br>`./l-loader.bin 0x1AC00000`</br>

   * Remove the modemmanager package. This package may causes hikey_idt tool failure.
     <br>`$sudo apt-get purge modemmanager`</br>

   * Run the command to download l-loader.bin into HiKey960.
     <br>`$sudo ./hikey_idt -c config -p /dev/ttyUSB1`</br>

   * UEFI running in recovery mode.
     <br>When prompt '.' is displayed on console, press hotkey 'f' in keyboard. Then Android fastboot app is running.</br>
     <br>The timeout of prompt '.' is 10 seconds.</br>

   * Update images.
     <br>`$sudo fastboot flash ptable prm_ptable.img`</br>
     <br>`$sudo fastboot flash xloader sec_xloader.img`</br>
     <br>`$sudo fastboot flash fastboot l-loader.bin`</br>
     <br>`$sudo fastboot flash fip fip.bin`</br>
     <br>`$sudo fastboot flash boot boot.img`</br>
     <br>`$sudo fastboot flash cache cache.img`</br>
     <br>`$sudo fastboot flash system system.img`</br>
     <br>`$sudo fastboot flash userdata userdata.img`</br>

   * Notice: UEFI could also boot kernel in recovery mode, but BL31 isn't loaded in
     recovery mode.


5. Boot UEFI in normal mode
-----------------------------

   * Make sure "Boot Mode" switch is OFF for normal boot mode. Then power on HiKey960.

   * Reference [link](https://github.com/96boards-hikey/tools-images-hikey960/blob/master/build-from-source/README-ATF-UEFI-build-from-source.md)

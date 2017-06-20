
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
    ```shell
    $cd ${BUILD_PATH}/edk2
    $ln -sf ../OpenPlatformPkg
    ```

  * Prepare AARCH64 toolchain.

  * If your hikey960 hardware is v1, update _uefi-tools/platform.config_ first. _(optional)_
    __Uncomment the below sentence. Otherwise, UEFI can't output messages on serial
    console on hikey960 v1.__
    ```shell
    BUILDFLAGS=-DSERIAL_BASE=0xFDF05000
    ```
    If your hikey960 hardware is v2 or newer, nothing to do.

  * Build it as debug mode. Create script file for build.
    ```shell
    BUILD_OPTION=DEBUG
    export AARCH64_TOOLCHAIN=GCC48
    export UEFI_TOOLS_DIR=${BUILD_PATH}/uefi-tools
    export EDK2_DIR=${BUILD_PATH}/edk2
    EDK2_OUTPUT_DIR=${EDK2_DIR}/Build/HiKey960/${BUILD_OPTION}_${AARCH64_TOOLCHAIN}
    cd ${EDK2_DIR}
    # Build UEFI & ARM Trust Firmware
    ${UEFI_TOOLS_DIR}/uefi-build.sh -b ${BUILD_OPTION} -a ../arm-trusted-firmware hikey960
    # Generate l-loader.bin
    cd ${BUILD_PATH}/l-loader
    ln -sf ${EDK2_OUTPUT_DIR}/FV/bl1.bin
    ln -sf ${EDK2_OUTPUT_DIR}/FV/fip.bin
    ln -sf ${EDK2_OUTPUT_DIR}/FV/BL33_AP_UEFI.fd
    python gen_loader.py -o l-loader.bin --img_bl1=bl1.bin --img_ns_bl1u=BL33_AP_UEFI.fd
    ```

  * Generate partition table.
    _Make sure that you're using the sgdisk in the l-loader directory._
    ```shell
    $PTABLE=aosp-32g SECTOR_SIZE=4096 SGDISK=./sgdisk bash -x generate_ptable.sh
    ```


3. Setup Console
----------------

  * Install ser2net. Use telnet as the console since UEFI will output window
    that fails to display in minicom.
    ```shell
    $sudo apt-get install ser2net
    ```

  * Configure ser2net.
    ```shell
    $sudo vi /etc/ser2net.conf
    ```
    Append one line for serial-over-USB in _#ser2net.conf_
    ```
    2004:telnet:0:/dev/ttyUSB0:115200 8DATABITS NONE 1STOPBIT banner
    ```

  * Open the console.
    ```shell
    $telnet localhost 2004
    ```
    And you could open the console remotely, too.


4. Boot UEFI in recovery mode
-----------------------------

  * Fetch that are used in recovery mode. The code location is in below.
    [link](https://github.com/96boards-hikey/tools-images-hikey960)

  * Generate l-loader.bin.
    ```shell
    $cd tools-images-hikey960
    $ln -sf ${BUILD_PATH}/l-loader/l-loader.bin
    ```

  * Prepare config file.
    ```shell
    $vi config
    # The content of config file
    ./sec_user_xloader.img 0x00020000
    ./sec_uce_boot.img 0x6A908000
    ./l-loader.bin 0x1AC00000
    ```

  * Remove the modemmanager package. This package may causes hikey_idt tool failure.
    ```shell
    $sudo apt-get purge modemmanager
    ```

  * Run the command to download l-loader.bin into HiKey960.
    ```shell
    $sudo ./hikey_idt -c config -p /dev/ttyUSB1
    ```

  * UEFI running in recovery mode.
    When prompt '.' is displayed on console, press hotkey 'f' in keyboard. Then Android fastboot app is running.
    The timeout of prompt '.' is 10 seconds.

  * Update images.
    ```shell
    $sudo fastboot flash ptable prm_ptable.img
    $sudo fastboot flash xloader sec_xloader.img
    $sudo fastboot flash fastboot l-loader.bin
    $sudo fastboot flash fip fip.bin
    $sudo fastboot flash boot boot.img
    $sudo fastboot flash cache cache.img
    $sudo fastboot flash system system.img
    $sudo fastboot flash userdata userdata.img
    ```

  * Notice: UEFI could also boot kernel in recovery mode, but BL31 isn't loaded in
  recovery mode.


5. Boot UEFI in normal mode
-----------------------------

  * Make sure "Boot Mode" switch is OFF for normal boot mode. Then power on HiKey960.

  * Reference [link](https://github.com/96boards-hikey/tools-images-hikey960/blob/master/build-from-source/README-ATF-UEFI-build-from-source.md)

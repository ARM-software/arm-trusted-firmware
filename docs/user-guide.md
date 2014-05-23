ARM Trusted Firmware User Guide
===============================

Contents :

1.  Introduction
2.  Host machine requirements
3.  Tools
4.  Building the Trusted Firmware
5.  Obtaining the normal world software
6.  Running the software


1.  Introduction
----------------
This document describes how to build ARM Trusted Firmware and run it with a
tested set of other software components using defined configurations on ARM
Fixed Virtual Platform (FVP) models. It is possible to use other software
components, configurations and platforms but that is outside the scope of this
document.

This document should be used in conjunction with the [Firmware Design].


2.  Host machine requirements
-----------------------------

The minimum recommended machine specification for building the software and
running the FVP models is a dual-core processor running at 2GHz with 12GB of
RAM.  For best performance, use a machine with a quad-core processor running at
2.6GHz with 16GB of RAM.

The software has been tested on Ubuntu 12.04.04 (64-bit).  Packages used
for building the software were installed from that distribution unless
otherwise specified.


3.  Tools
---------

The following tools are required to use the ARM Trusted Firmware:

*   `git` package to obtain source code

*   `ia32-libs` package

*   `build-essential` and `uuid-dev` packages for building UEFI and the Firmware
    Image Package(FIP) tool

*   `bc` and `ncurses-dev` packages for building Linux

*   Baremetal GNU GCC tools. Verified packages can be downloaded from [Linaro]
    [Linaro Toolchain]. The rest of this document assumes that the
    `gcc-linaro-aarch64-none-elf-4.8-2013.11_linux.tar.xz` tools are used.

        wget http://releases.linaro.org/13.11/components/toolchain/binaries/gcc-linaro-aarch64-none-elf-4.8-2013.11_linux.tar.xz
        tar -xf gcc-linaro-aarch64-none-elf-4.8-2013.11_linux.tar.xz

*   The Device Tree Compiler (DTC) included with Linux kernel 3.15-rc6 is used
    to build the Flattened Device Tree (FDT) source files (`.dts` files)
    provided with this software.

*   (Optional) For debugging, ARM [Development Studio 5 (DS-5)][DS-5] v5.18.


4.  Building the Trusted Firmware
---------------------------------

To build the software for the FVPs, follow these steps:

1.  Clone the ARM Trusted Firmware repository from GitHub:

        git clone https://github.com/ARM-software/arm-trusted-firmware.git

2.  Change to the trusted firmware directory:

        cd arm-trusted-firmware

3.  Set the compiler path, specify a Non-trusted Firmware image (BL3-3) and
    build:

        CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- \
        BL33=<path-to>/<bl33_image>                               \
        make PLAT=fvp all fip

    See the "Summary of build options" for information on available build
    options.

    By default this produces a release version of the build. To produce a debug
    version instead, refer to the "Debugging options" section below. UEFI can be
    used as the BL3-3 image, refer to the "Obtaining the normal world software"
    section below. By default this won't compile the TSP in, refer to the
    "Building the Test Secure Payload" section below.

    The build process creates products in a `build` directory tree, building
    the objects and binaries for each boot loader stage in separate
    sub-directories.  The following boot loader binary files are created from
    the corresponding ELF files:

    *   `build/<platform>/<build-type>/bl1.bin`
    *   `build/<platform>/<build-type>/bl2.bin`
    *   `build/<platform>/<build-type>/bl31.bin`

    ... where `<platform>` currently defaults to `fvp` and `<build-type>` is
    either `debug` or `release`. A Firmare Image Package(FIP) will be created as
    part of the build. It contains all boot loader images except for `bl1.bin`.

    *   `build/<platform>/<build-type>/fip.bin`

    For more information on FIPs, see the "Firmware Image Package" section in
    the [Firmware Design].

4.  Copy the `bl1.bin` and `fip.bin` binary files to the directory from which
    the FVP will be launched. Symbolic links of the same names may be created
    instead.

5.  (Optional) Build products for a specific build variant can be removed using:

        make DEBUG=<D> PLAT=fvp clean

    ... where `<D>` is `0` or `1`, as specified when building.

    The build tree can be removed completely using:

        make realclean

### Summary of build options

ARM Trusted Firmware build system supports the following build options. Unless
mentioned otherwise, these options are expected to be specified at the build
command line and are not to be modified in any component makefiles. Note that
the build system doesn't track dependency for build options. Therefore, if any
of the build options are changed from a previous build, a clean build must be
performed.

*   `BL33`: Path to BL33 image in the host file system. This is mandatory for
    `fip` target

*   `CROSS_COMPILE`: Prefix to tool chain binaries. Please refer to examples in
    this document for usage

*   `DEBUG`: Chooses between a debug and release build. It can take either 0
    (release) or 1 (debug) as values. 0 is the default

*   `NS_TIMER_SWITCH`: Enable save and restore for non-secure timer register
    contents upon world switch. It can take either 0 (don't save and restore) or
    1 (do save and restore). 0 is the default. An SPD could set this to 1 if it
    wants the timer registers to be saved and restored

*   `PLAT`: Choose a platform to build ARM Trusted Firmware for. The chosen
    platform name must be the name of one of the directories under the `plat/`
    directory other than `common`

*   `SPD`: Choose a Secure Payload Dispatcher component to be built into the
    Trusted Firmware. The value should be the path to the directory containing
    SPD source; the directory is expected to contain `spd.mk` makefile

*   `V`: Verbose build. If assigned anything other than 0, the build commands
    are printed. Default is 0

*   `FVP_GIC_ARCH`: Choice of ARM GIC architecture version used by the FVP port
    for implementing the platform GIC API. This API is used by the interrupt
    management framework. Default is 2 i.e. version 2.0

*   `IMF_READ_INTERRUPT_ID`: Boolean flag used by the interrupt management
    framework to enable passing of the interrupt id to its handler. The id is
    read using a platform GIC API. `INTR_ID_UNAVAILABLE` is passed instead if
    this option set to 0. Default is 0.

*   `RESET_TO_BL31`: Enable BL3-1 entrypoint as the CPU reset vector in place
    of the BL1 entrypoint. It can take the value 0 (CPU reset to BL1
    entrypoint) or 1 (CPU reset to BL3-1 entrypoint).
    The default value is 0.


### Creating a Firmware Image Package

FIPs are automatically created as part of the build instructions described in
the previous section. It is also possible to independently build the FIP
creation tool and FIPs if required. To do this, follow these steps:

Build the tool:

    make -C tools/fip_create

It is recommended to remove the build artifacts before rebuilding:

    make -C tools/fip_create clean

Create a Firmware package that contains existing FVP BL2 and BL3-1 images:

    # fip_create --help to print usage information
    # fip_create <fip_name> <images to add> [--dump to show result]
    ./tools/fip_create/fip_create fip.bin --dump \
       --bl2 build/fvp/debug/bl2.bin --bl31 build/fvp/debug/bl31.bin

     Firmware Image Package ToC:
    ---------------------------
    - Trusted Boot Firmware BL2: offset=0x88, size=0x81E8
      file: 'build/fvp/debug/bl2.bin'
    - EL3 Runtime Firmware BL3-1: offset=0x8270, size=0xC218
      file: 'build/fvp/debug/bl31.bin'
    ---------------------------
    Creating "fip.bin"

View the contents of an existing Firmware package:

    ./tools/fip_create/fip_create fip.bin --dump

     Firmware Image Package ToC:
    ---------------------------
    - Trusted Boot Firmware BL2: offset=0x88, size=0x81E8
    - EL3 Runtime Firmware BL3-1: offset=0x8270, size=0xC218
    ---------------------------

Existing package entries can be individially updated:

    # Change the BL2 from Debug to Release version
    ./tools/fip_create/fip_create fip.bin --dump \
      --bl2 build/fvp/release/bl2.bin

    Firmware Image Package ToC:
    ---------------------------
    - Trusted Boot Firmware BL2: offset=0x88, size=0x7240
      file: 'build/fvp/release/bl2.bin'
    - EL3 Runtime Firmware BL3-1: offset=0x72C8, size=0xC218
    ---------------------------
    Updating "fip.bin"


### Debugging options

To compile a debug version and make the build more verbose use

    CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- \
    BL33=<path-to>/<bl33_image>                               \
    make PLAT=fvp DEBUG=1 V=1 all fip

AArch64 GCC uses DWARF version 4 debugging symbols by default. Some tools (for
example DS-5) might not support this and may need an older version of DWARF
symbols to be emitted by GCC. This can be achieved by using the
`-gdwarf-<version>` flag, with the version being set to 2 or 3. Setting the
version to 2 is recommended for DS-5 versions older than 5.16.

When debugging logic problems it might also be useful to disable all compiler
optimizations by using `-O0`.

NOTE: Using `-O0` could cause output images to be larger and base addresses
might need to be recalculated (see the later memory layout section).

Extra debug options can be passed to the build system by setting `CFLAGS`:

    CFLAGS='-O0 -gdwarf-2'                                    \
    CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- \
    BL33=<path-to>/<bl33_image>                               \
    make PLAT=fvp DEBUG=1 V=1 all fip


NOTE: The Foundation FVP does not provide a debugger interface.


### Building the Test Secure Payload

The TSP is coupled with a companion runtime service in the BL3-1 firmware,
called the TSPD. Therefore, if you intend to use the TSP, the BL3-1 image
must be recompiled as well. For more information on SPs and SPDs, see the
"Secure-EL1 Payloads and Dispatchers" section in the [Firmware Design].

First clean the Trusted Firmware build directory to get rid of any previous
BL3-1 binary. Then to build the TSP image and include it into the FIP use:

    CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- \
    BL33=<path-to>/<bl33_image>                               \
    make PLAT=fvp SPD=tspd all fip

An additional boot loader binary file is created in the `build` directory:

    *   `build/<platform>/<build-type>/bl32.bin`

The Firmware Package contains this new image:

    Firmware Image Package ToC:
    ---------------------------
    - Trusted Boot Firmware BL2: offset=0xD8, size=0x6000
      file: './build/fvp/release/bl2.bin'
    - EL3 Runtime Firmware BL3-1: offset=0x60D8, size=0x9000
      file: './build/fvp/release/bl31.bin'
    - Secure Payload BL3-2 (Trusted OS): offset=0xF0D8, size=0x3000
      file: './build/fvp/release/bl32.bin'
    - Non-Trusted Firmware BL3-3: offset=0x120D8, size=0x280000
      file: '../FVP_AARCH64_EFI.fd'
    ---------------------------
    Creating "build/fvp/release/fip.bin"

On FVP, the TSP binary runs from Trusted SRAM by default. It is also possible
to run it from Trusted DRAM. This is controlled by the build configuration
`TSP_RAM_LOCATION`:

    CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- \
    BL33=<path-to>/<bl33_image>                               \
    make PLAT=fvp SPD=tspd TSP_RAM_LOCATION=tdram all fip


### Checking source code style

When making changes to the source for submission to the project, the source
must be in compliance with the Linux style guide, and to assist with this check
the project Makefile contains two targets, which both utilise the checkpatch.pl
script that ships with the Linux source tree.

To check the entire source tree, you must first download a copy of checkpatch.pl
(or the full Linux source), set the CHECKPATCH environment variable to point to
the script and build the target checkcodebase:

    make CHECKPATCH=../linux/scripts/checkpatch.pl checkcodebase

To just check the style on the files that differ between your local branch and
the remote master, use:

    make CHECKPATCH=../linux/scripts/checkpatch.pl checkpatch

If you wish to check your patch against something other than the remote master,
set the BASE_COMMIT variable to your desired branch.  By default, BASE_COMMIT
is set to 'origin/master'.


5.  Obtaining the normal world software
---------------------------------------

### Obtaining EDK2

Potentially any kind of non-trusted firmware may be used with the ARM Trusted
Firmware but the software has only been tested with the EFI Development Kit 2
(EDK2) open source implementation of the UEFI specification.

Clone the [EDK2 source code][EDK2] from GitHub. This version supports the Base
and Foundation FVPs:

    git clone -n https://github.com/tianocore/edk2.git
    cd edk2
    git checkout 129ff94661bd3a6c759b1e154c143d0136bedc7d


To build the software to be compatible with Foundation and Base FVPs, follow
these steps:

1.  Copy build config templates to local workspace

        # in edk2/
        . edksetup.sh

2.  Build the EDK2 host tools

        make -C BaseTools clean
        make -C BaseTools

3.  Build the EDK2 software

        CROSS_COMPILE=<absolute-path-to-aarch64-gcc>/bin/aarch64-none-elf- \
        make -f ArmPlatformPkg/Scripts/Makefile EDK2_ARCH=AARCH64          \
        EDK2_DSC=ArmPlatformPkg/ArmVExpressPkg/ArmVExpress-FVP-AArch64.dsc \
        EDK2_TOOLCHAIN=ARMGCC EDK2_MACROS="-n 6 -D ARM_FOUNDATION_FVP=1"

    The EDK2 binary for use with the ARM Trusted Firmware can then be found
    here:

        Build/ArmVExpress-FVP-AArch64/DEBUG_ARMGCC/FV/FVP_AARCH64_EFI.fd

    This will build EDK2 for the default settings as used by the FVPs. The EDK2
    binary `FVP_AARCH64_EFI.fd` should be specified as `BL33` in in the `make`
    command line when building the Trusted Firmware. See the "Building the
    Trusted Firmware" section above.

4.  (Optional) To boot Linux using a VirtioBlock file-system, the command line
    passed from EDK2 to the Linux kernel must be modified as described in the
    "Obtaining a root file-system" section below.

5.  (Optional) If legacy GICv2 locations are used, the EDK2 platform description
    must be updated. This is required as EDK2 does not support probing for the
    GIC location. To do this, first clean the EDK2 build directory.

        make -f ArmPlatformPkg/Scripts/Makefile EDK2_ARCH=AARCH64          \
        EDK2_DSC=ArmPlatformPkg/ArmVExpressPkg/ArmVExpress-FVP-AArch64.dsc \
        EDK2_TOOLCHAIN=ARMGCC clean

    Then rebuild EDK2 as described in step 3, using the following flag:

        -D ARM_FVP_LEGACY_GICV2_LOCATION=1

    Finally rebuild the Trusted Firmware to generate a new FIP using the
    instructions in the "Building the Trusted Firmware" section.


### Obtaining a Linux kernel

The software has been verified using a Linux kernel based on version 3.15-rc6.
Patches have been applied in order to enable the CPU idle feature.

Preparing a Linux kernel for use on the FVPs with CPU idle support can
be done as follows (GICv2 support only):

1.  Clone Linux:

        git clone git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git

    Not all CPU idle features are included in the mainline kernel yet. To
    use these, add the patches from Sudeep Holla's kernel:

        cd linux
        git remote add -f --tags arm64_idle_v3.15-rc6 git://linux-arm.org/linux-skn.git
        git checkout -b cpuidle arm64_idle_v3.15-rc6

2.  Build with the Linaro GCC tools.

        # in linux/
        make mrproper
        make ARCH=arm64 defconfig

        # Enable CPU idle
        make ARCH=arm64 menuconfig
        # CPU Power Management ---> CPU Idle ---> [*] CPU idle PM support
        # CPU Power Management ---> CPU Idle ---> ARM64 CPU Idle Drivers ---> [*] Generic ARM64 CPU idle Driver

        CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- \
        make -j6 ARCH=arm64

3.  Copy the Linux image `arch/arm64/boot/Image` to the working directory from
    where the FVP is launched. Alternatively a symbolic link may be used.

### Obtaining the Flattened Device Trees

Depending on the FVP configuration and Linux configuration used, different
FDT files are required. FDTs for the Foundation and Base FVPs can be found in
the Trusted Firmware source directory under `fdts/`. The Foundation FVP has a
subset of the Base FVP components. For example, the Foundation FVP lacks CLCD
and MMC support, and has only one CPU cluster.

*   `fvp-base-gicv2-psci.dtb`

    (Default) For use with both AEMv8 and Cortex-A57-A53 Base FVPs with
    Base memory map configuration.

*   `fvp-base-gicv2legacy-psci.dtb`

    For use with AEMv8 Base FVP with legacy VE GIC memory map configuration.

*   `fvp-base-gicv3-psci.dtb`

    For use with both AEMv8 and Cortex-A57-A53 Base FVPs with Base memory map
    configuration and Linux GICv3 support.

*   `fvp-foundation-gicv2-psci.dtb`

    (Default) For use with Foundation FVP with Base memory map configuration.

*   `fvp-foundation-gicv2legacy-psci.dtb`

    For use with Foundation FVP with legacy VE GIC memory map configuration.

*   `fvp-foundation-gicv3-psci.dtb`

    For use with Foundation FVP with Base memory map configuration and Linux
    GICv3 support.


Copy the chosen FDT blob as `fdt.dtb` to the directory from which the FVP
is launched. Alternatively a symbolic link may be used.

### Obtaining a root file-system

To prepare a Linaro LAMP based Open Embedded file-system, the following
instructions can be used as a guide. The file-system can be provided to Linux
via VirtioBlock or as a RAM-disk. Both methods are described below.

#### Prepare VirtioBlock

To prepare a VirtioBlock file-system, do the following:

1.  Download and unpack the disk image.

    NOTE: The unpacked disk image grows to 3 GiB in size.

        wget http://releases.linaro.org/14.04/openembedded/aarch64/vexpress64-openembedded_lamp-armv8-gcc-4.8_20140417-630.img.gz
        gunzip vexpress64-openembedded_lamp-armv8-gcc-4.8_20140417-630.img.gz

2.  Make sure the Linux kernel has Virtio support enabled using
    `make ARCH=arm64 menuconfig`.

        Device Drivers  ---> Virtio drivers  ---> <*> Platform bus driver for memory mapped virtio devices
        Device Drivers  ---> [*] Block devices  --->  <*> Virtio block driver
        File systems    ---> <*> The Extended 4 (ext4) filesystem

    If some of these configurations are missing, enable them, save the kernel
    configuration, then rebuild the kernel image using the instructions
    provided in the section "Obtaining a Linux kernel".

3.  Change the Kernel command line to include `root=/dev/vda2`. This can either
    be done in the EDK2 boot menu or in the platform file. Editing the platform
    file and rebuilding EDK2 will make the change persist. To do this:

    1.  In EDK2, edit the following file:

            ArmPlatformPkg/ArmVExpressPkg/ArmVExpress-FVP-AArch64.dsc

    2.  Add `root=/dev/vda2` to:

            gArmPlatformTokenSpaceGuid.PcdDefaultBootArgument|"<Other default options>"

    3.  Remove the entry:

            gArmPlatformTokenSpaceGuid.PcdDefaultBootInitrdPath|""

    4.  Rebuild EDK2 (see "Obtaining UEFI" section above).

4.  The file-system image file should be provided to the model environment by
    passing it the correct command line option. In the FVPs the following
    option should be provided in addition to the ones described in the
    "Running the software" section below.

    NOTE: A symbolic link to this file cannot be used with the FVP; the path
    to the real file must be provided.

    On the Base FVPs:

        -C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>"

    On the Foundation FVP:

        --block-device="<path-to>/<file-system-image>"


5.  Ensure that the FVP doesn't output any error messages. If the following
    error message is displayed:

        ERROR: BlockDevice: Failed to open "<path-to>/<file-system-image>"!

    then make sure the path to the file-system image in the model parameter is
    correct and that read permission is correctly set on the file-system image
    file.

#### Prepare RAM-disk

To prepare a RAM-disk root file-system, do the following:

1.  Download the file-system image:

        wget http://releases.linaro.org/14.04/openembedded/aarch64/linaro-image-lamp-genericarmv8-20140417-667.rootfs.tar.gz

2.  Modify the Linaro image:

        # Prepare for use as RAM-disk. Normally use MMC, NFS or VirtioBlock.
        # Be careful, otherwise you could damage your host file-system.
        mkdir tmp; cd tmp
        sudo sh -c "zcat ../linaro-image-lamp-genericarmv8-20140417-667.rootfs.tar.gz | cpio -id"
        sudo ln -s sbin/init .
        sudo sh -c "echo 'devtmpfs /dev devtmpfs mode=0755,nosuid 0 0' >> etc/fstab"
        sudo sh -c "find . | cpio --quiet -H newc -o | gzip -3 -n > ../filesystem.cpio.gz"
        cd ..

3.  Copy the resultant `filesystem.cpio.gz` to the directory where the FVP is
    launched from. Alternatively a symbolic link may be used.


6.  Running the software
------------------------

This version of the ARM Trusted Firmware has been tested on the following ARM
FVPs (64-bit versions only).

*   `Foundation_v8` (Version 2.0, Build 0.8.5206)
*   `FVP_Base_AEMv8A-AEMv8A` (Version 5.6, Build 0.8.5602)
*   `FVP_Base_Cortex-A57x4-A53x4` (Version 5.6, Build 0.8.5602)
*   `FVP_Base_Cortex-A57x1-A53x1` (Version 5.6, Build 0.8.5602)
*   `FVP_Base_Cortex-A57x2-A53x4` (Version 5.6, Build 0.8.5602)

NOTE: The software will not work on Version 1.0 of the Foundation FVP.
The commands below would report an `unhandled argument` error in this case.

Please refer to the FVP documentation for a detailed description of the model
parameter options. A brief description of the important ones that affect the
ARM Trusted Firmware and normal world software behavior is provided below.

The Foundation FVP is a cut down version of the AArch64 Base FVP. It can be
downloaded for free from [ARM's website][ARM FVP website].


### Running on the Foundation FVP with reset to BL1 entrypoint

The following `Foundation_v8` parameters should be used to boot Linux with
4 CPUs using the ARM Trusted Firmware.

NOTE: Using the `--block-device` parameter is not necessary if a Linux RAM-disk
file-system is used (see the "Obtaining a File-system" section above).

NOTE: The `--data="<path to FIP binary>"@0x8000000` parameter is used to load a
Firmware Image Package at the start of NOR FLASH0 (see the "Building the
Trusted Firmware" section above).

    <path-to>/Foundation_v8                   \
    --cores=4                                 \
    --no-secure-memory                        \
    --visualization                           \
    --gicv3                                   \
    --data="<path-to>/<bl1-binary>"@0x0       \
    --data="<path-to>/<FIP-binary>"@0x8000000 \
    --block-device="<path-to>/<file-system-image>"

The default use-case for the Foundation FVP is to enable the GICv3 device in
the model but use the GICv2 FDT, in order for Linux to drive the GIC in GICv2
emulation mode.

The memory mapped addresses `0x0` and `0x8000000` correspond to the start of
trusted ROM and NOR FLASH0 respectively.

### Notes regarding Base FVP configuration options

1. The `-C bp.flashloader0.fname` parameter is used to load a Firmware Image
Package at the start of NOR FLASH0 (see the "Building the Trusted Firmware"
section above).

2. Using `cache_state_modelled=1` makes booting very slow. The software will
still work (and run much faster) without this option but this will hide any
cache maintenance defects in the software.

3. Using the `-C bp.virtioblockdevice.image_path` parameter is not necessary
if a Linux RAM-disk file-system is used (see the "Obtaining a root file-system"
section above).

4. Setting the `-C bp.secure_memory` parameter to `1` is only supported on
Base FVP versions 5.4 and newer. Setting this parameter to `0` is also
supported. The `-C bp.tzc_400.diagnostics=1` parameter is optional. It
instructs the FVP to provide some helpful information if a secure memory
violation occurs.

5. The `--data="<path-to><bl31/bl32/bl33-binary>"@base address of binaries`
parameter is used to load bootloader images in the Base FVP memory (see the
"Building the Trusted Firmware" section above). The base address used to
load the binaries with --data should match the image base addresses in
platform_def.h used while linking the images.
BL3-2 image is only needed if BL3-1 has been built to expect a secure-EL1
payload.


### Running on the AEMv8 Base FVP with reset to BL1 entrypoint

Please read "Notes regarding Base FVP configuration options" section above for
information about some of the options to run the software.

The following `FVP_Base_AEMv8A-AEMv8A` parameters should be used to boot Linux
with 8 CPUs using the ARM Trusted Firmware.

    <path-to>/FVP_Base_AEMv8A-AEMv8A                       \
    -C pctl.startup=0.0.0.0                                \
    -C bp.secure_memory=1                                  \
    -C bp.tzc_400.diagnostics=1                            \
    -C cluster0.NUM_CORES=4                                \
    -C cluster1.NUM_CORES=4                                \
    -C cache_state_modelled=1                              \
    -C bp.pl011_uart0.untimed_fifos=1                      \
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>" \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"      \
    -C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>"

### Running on the Cortex-A57-A53 Base FVP with reset to BL1 entrypoint

Please read "Notes regarding Base FVP configuration options" section above for
information about some of the options to run the software.

The following `FVP_Base_Cortex-A57x4-A53x4` model parameters should be used to
boot Linux with 8 CPUs using the ARM Trusted Firmware.

    <path-to>/FVP_Base_Cortex-A57x4-A53x4                  \
    -C pctl.startup=0.0.0.0                                \
    -C bp.secure_memory=1                                  \
    -C bp.tzc_400.diagnostics=1                            \
    -C cache_state_modelled=1                              \
    -C bp.pl011_uart0.untimed_fifos=1                      \
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>" \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"      \
    -C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>"

### Running on the AEMv8 Base FVP with reset to BL3-1 entrypoint

Please read "Notes regarding Base FVP configuration options" section above for
information about some of the options to run the software.

The following `FVP_Base_AEMv8A-AEMv8A` parameters should be used to boot Linux
with 8 CPUs using the ARM Trusted Firmware.

NOTE: Uses the `-c clusterX.cpuX.RVBAR=@base address of BL3-1` where X is
the cluster number in clusterX and cpu number in cpuX is used to set the reset
vector for each core.

    <path-to>/FVP_Base_AEMv8A-AEMv8A                             \
    -C pctl.startup=0.0.0.0                                      \
    -C bp.secure_memory=1                                        \
    -C bp.tzc_400.diagnostics=1                                  \
    -C cluster0.NUM_CORES=4                                      \
    -C cluster1.NUM_CORES=4                                      \
    -C cache_state_modelled=1                                    \
    -C bp.pl011_uart0.untimed_fifos=1                            \
    -C cluster0.cpu0.RVBAR=0x04006000                            \
    -C cluster0.cpu1.RVBAR=0x04006000                            \
    -C cluster0.cpu2.RVBAR=0x04006000                            \
    -C cluster0.cpu3.RVBAR=0x04006000                            \
    -C cluster1.cpu0.RVBAR=0x04006000                            \
    -C cluster1.cpu1.RVBAR=0x04006000                            \
    -C cluster1.cpu2.RVBAR=0x04006000                            \
    -C cluster1.cpu3.RVBAR=0x04006000                            \
    --data cluster0.cpu0="<path-to>/<bl31-binary>"@0x04006000    \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0x04024000    \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000    \
    -C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>"

### Running on the Cortex-A57-A53 Base FVP with reset to BL3-1 entrypoint

Please read "Notes regarding Base FVP configuration options" section above for
information about some of the options to run the software.

The following `FVP_Base_Cortex-A57x4-A53x4` model parameters should be used to
boot Linux with 8 CPUs using the ARM Trusted Firmware.

NOTE: Uses the `-c clusterX.cpuX.RVBARADDR=@base address of BL3-1` where X is
the cluster number in clusterX and cpu number in cpuX is used to set the reset
vector for each core.

    <path-to>/FVP_Base_Cortex-A57x4-A53x4                        \
    -C pctl.startup=0.0.0.0                                      \
    -C bp.secure_memory=1                                        \
    -C bp.tzc_400.diagnostics=1                                  \
    -C cache_state_modelled=1                                    \
    -C bp.pl011_uart0.untimed_fifos=1                            \
    -C cluster0.cpu0.RVBARADDR=0x04006000                        \
    -C cluster0.cpu1.RVBARADDR=0x04006000                        \
    -C cluster0.cpu2.RVBARADDR=0x04006000                        \
    -C cluster0.cpu3.RVBARADDR=0x04006000                        \
    -C cluster1.cpu0.RVBARADDR=0x04006000                        \
    -C cluster1.cpu1.RVBARADDR=0x04006000                        \
    -C cluster1.cpu2.RVBARADDR=0x04006000                        \
    -C cluster1.cpu3.RVBARADDR=0x04006000                        \
    --data cluster0.cpu0="<path-to>/<bl31-binary>"@0x04006000    \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0x04024000    \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000    \
    -C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>"

### Configuring the GICv2 memory map

The Base FVP models support GICv2 with the default model parameters at the
following addresses. The Foundation FVP also supports these addresses when
configured for GICv3 in GICv2 emulation mode.

    GICv2 Distributor Interface     0x2f000000
    GICv2 CPU Interface             0x2c000000
    GICv2 Virtual CPU Interface     0x2c010000
    GICv2 Hypervisor Interface      0x2c02f000

The AEMv8 Base FVP can be configured to support GICv2 at addresses
corresponding to the legacy (Versatile Express) memory map as follows. These are
the default addresses when using the Foundation FVP in GICv2 mode.

    GICv2 Distributor Interface     0x2c001000
    GICv2 CPU Interface             0x2c002000
    GICv2 Virtual CPU Interface     0x2c004000
    GICv2 Hypervisor Interface      0x2c006000

The choice of memory map is reflected in the build variant field (bits[15:12])
in the `SYS_ID` register (Offset `0x0`) in the Versatile Express System
registers memory map (`0x1c010000`).

*   `SYS_ID.Build[15:12]`

    `0x1` corresponds to the presence of the Base GIC memory map. This is the
    default value on the Base FVPs.

*   `SYS_ID.Build[15:12]`

    `0x0` corresponds to the presence of the Legacy VE GIC memory map. This is
    the default value on the Foundation FVP.

This register can be configured as described in the following sections.

NOTE: If the legacy VE GIC memory map is used, then the corresponding FDT and
BL3-3 images should be used.

#### Configuring AEMv8 Foundation FVP GIC for legacy VE memory map

The following parameters configure the Foundation FVP to use GICv2 with the
legacy VE memory map:

    <path-to>/Foundation_v8                   \
    --cores=4                                 \
    --no-secure-memory                        \
    --visualization                           \
    --no-gicv3                                \
    --data="<path-to>/<bl1-binary>"@0x0       \
    --data="<path-to>/<FIP-binary>"@0x8000000 \
    --block-device="<path-to>/<file-system-image>"

Explicit configuration of the `SYS_ID` register is not required.

#### Configuring AEMv8 Base FVP GIC for legacy VE memory map

The following parameters configure the AEMv8 Base FVP to use GICv2 with the
legacy VE memory map. They must added to the parameters described in the
"Running on the AEMv8 Base FVP" section above:

    -C cluster0.gic.GICD-offset=0x1000                  \
    -C cluster0.gic.GICC-offset=0x2000                  \
    -C cluster0.gic.GICH-offset=0x4000                  \
    -C cluster0.gic.GICH-other-CPU-offset=0x5000        \
    -C cluster0.gic.GICV-offset=0x6000                  \
    -C cluster0.gic.PERIPH-size=0x8000                  \
    -C cluster1.gic.GICD-offset=0x1000                  \
    -C cluster1.gic.GICC-offset=0x2000                  \
    -C cluster1.gic.GICH-offset=0x4000                  \
    -C cluster1.gic.GICH-other-CPU-offset=0x5000        \
    -C cluster1.gic.GICV-offset=0x6000                  \
    -C cluster1.gic.PERIPH-size=0x8000                  \
    -C gic_distributor.GICD-alias=0x2c001000            \
    -C bp.variant=0x0

The `bp.variant` parameter corresponds to the build variant field of the
`SYS_ID` register.  Setting this to `0x0` allows the ARM Trusted Firmware to
detect the legacy VE memory map while configuring the GIC.


- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved._


[Firmware Design]:  ./firmware-design.md

[ARM FVP website]:  http://www.arm.com/fvp
[Linaro Toolchain]: http://releases.linaro.org/13.11/components/toolchain/binaries/
[EDK2]:             http://github.com/tianocore/edk2
[DS-5]:             http://www.arm.com/products/tools/software-tools/ds-5/index.php

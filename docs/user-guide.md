ARM Trusted Firmware User Guide
===============================

Contents :

1.  [Introduction](#1--introduction)
2.  [Host machine requirements](#2--host-machine-requirements)
3.  [Tools](#3--tools)
4.  [Building the Trusted Firmware](#4--building-the-trusted-firmware)
5.  [Obtaining the normal world software](#5--obtaining-the-normal-world-software)
6.  [Preparing the images to run on FVP](#6--preparing-the-images-to-run-on-fvp)
7.  [Running the software on FVP](#7--running-the-software-on-fvp)
8.  [Running the software on Juno](#8--running-the-software-on-juno)


1.  Introduction
----------------
This document describes how to build ARM Trusted Firmware and run it with a
tested set of other software components using defined configurations on the Juno
ARM development platform and ARM Fixed Virtual Platform (FVP) models. It is
possible to use other software components, configurations and platforms but that
is outside the scope of this document.

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

*   `git` package to obtain source code.

*   `build-essential`, `uuid-dev` and `iasl` packages for building UEFI and the
    Firmware Image Package (FIP) tool.

*   `bc` and `ncurses-dev` packages for building Linux.

*   `device-tree-compiler` package for building the Flattened Device Tree (FDT)
    source files (`.dts` files) provided with this software.

*   Baremetal GNU GCC tools. Verified packages can be downloaded from [Linaro]
    [Linaro Toolchain]. The rest of this document assumes that the
    `gcc-linaro-aarch64-none-elf-4.9-2014.07_linux.tar.xz` tools are used.

        wget http://releases.linaro.org/14.07/components/toolchain/binaries/gcc-linaro-aarch64-none-elf-4.9-2014.07_linux.tar.xz
        tar -xf gcc-linaro-aarch64-none-elf-4.9-2014.07_linux.tar.xz

*   (Optional) For debugging, ARM [Development Studio 5 (DS-5)][DS-5] v5.20.


4.  Building the Trusted Firmware
---------------------------------

To build the Trusted Firmware images, follow these steps:

1.  Clone the ARM Trusted Firmware repository from GitHub:

        git clone https://github.com/ARM-software/arm-trusted-firmware.git

2.  Change to the trusted firmware directory:

        cd arm-trusted-firmware

3.  Set the compiler path, specify a Non-trusted Firmware image (BL3-3) and
    a valid platform, and then build:

        CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- \
        BL33=<path-to>/<bl33_image>                               \
        make PLAT=<platform> all fip

    If `PLAT` is not specified, `fvp` is assumed by default. See the "Summary of
    build options" for more information on available build options.

    The BL3-3 image corresponds to the software that is executed after switching
    to the non-secure world. UEFI can be used as the BL3-3 image. Refer to the
    "Obtaining the normal world software" section below.

    The TSP (Test Secure Payload), corresponding to the BL3-2 image, is not
    compiled in by default. Refer to the "Building the Test Secure Payload"
    section below.

    By default this produces a release version of the build. To produce a debug
    version instead, refer to the "Debugging options" section below.

    The build process creates products in a `build` directory tree, building
    the objects and binaries for each boot loader stage in separate
    sub-directories.  The following boot loader binary files are created from
    the corresponding ELF files:

    *   `build/<platform>/<build-type>/bl1.bin`
    *   `build/<platform>/<build-type>/bl2.bin`
    *   `build/<platform>/<build-type>/bl31.bin`

    where `<platform>` is the name of the chosen platform and `<build-type>` is
    either `debug` or `release`. A Firmare Image Package (FIP) will be created
    as part of the build. It contains all boot loader images except for
    `bl1.bin`.

    *   `build/<platform>/<build-type>/fip.bin`

    For more information on FIPs, see the "Firmware Image Package" section in
    the [Firmware Design].

4.  (Optional) Some platforms may require a BL3-0 image to boot. This image can
    be included in the FIP when building the Trusted Firmware by specifying the
    `BL30` build option:

        BL30=<path-to>/<bl30_image>

5.  Output binary files `bl1.bin` and `fip.bin` are both required to boot the
    system. How these files are used is platform specific. Refer to the
    platform documentation on how to use the firmware images.

6.  (Optional) Build products for a specific build variant can be removed using:

        make DEBUG=<D> PLAT=<platform> clean

    ... where `<D>` is `0` or `1`, as specified when building.

    The build tree can be removed completely using:

        make realclean

7.  (Optional) Path to binary for certain BL stages (BL2, BL3-1 and BL3-2) can be
    provided by specifying the BLx=<path-to>/<blx_image> where BLx is the BL stage.
    This will bypass the build of the BL component from source, but will include
    the specified binary in the final FIP image. Please note that BL3-2 will be
    included in the build, only if the `SPD` build option is specified.

    For example, specifying BL2=<path-to>/<bl2_image> in the build option, will
    skip compilation of BL2 source in trusted firmware, but include the BL2
    binary specified in the final FIP image.

### Summary of build options

ARM Trusted Firmware build system supports the following build options. Unless
mentioned otherwise, these options are expected to be specified at the build
command line and are not to be modified in any component makefiles. Note that
the build system doesn't track dependency for build options. Therefore, if any
of the build options are changed from a previous build, a clean build must be
performed.

#### Common build options

*   `BL30`: Path to BL3-0 image in the host file system. This image is optional.
    If a BL3-0 image is present then this option must be passed for the `fip`
    target.

*   `BL33`: Path to BL3-3 image in the host file system. This is mandatory for
    `fip` target in case the BL2 from ARM Trusted Firmware is used.

*   `BL2`: This is an optional build option which specifies the path to BL2
    image for the `fip` target. In this case, the BL2 in the ARM Trusted
    Firmware will not be built.

*   `BL31`:  This is an optional build option which specifies the path to
    BL3-1 image for the `fip` target. In this case, the BL3-1 in the ARM
    Trusted Firmware will not be built.

*   `BL32`:  This is an optional build option which specifies the path to
    BL3-2 image for the `fip` target. In this case, the BL3-2 in the ARM
    Trusted Firmware will not be built.

*   `FIP_NAME`: This is an optional build option which specifies the FIP
    filename for the `fip` target. Default is `fip.bin`.

*   `CROSS_COMPILE`: Prefix to toolchain binaries. Please refer to examples in
    this document for usage.

*   `DEBUG`: Chooses between a debug and release build. It can take either 0
    (release) or 1 (debug) as values. 0 is the default.

*   `LOG_LEVEL`: Chooses the log level, which controls the amount of console log
    output compiled into the build. This should be one of the following:

        0  (LOG_LEVEL_NONE)
        10 (LOG_LEVEL_NOTICE)
        20 (LOG_LEVEL_ERROR)
        30 (LOG_LEVEL_WARNING)
        40 (LOG_LEVEL_INFO)
        50 (LOG_LEVEL_VERBOSE)

    All log output up to and including the log level is compiled into the build.
    The default value is 40 in debug builds and 20 in release builds.

*   `NS_TIMER_SWITCH`: Enable save and restore for non-secure timer register
    contents upon world switch. It can take either 0 (don't save and restore) or
    1 (do save and restore). 0 is the default. An SPD may set this to 1 if it
    wants the timer registers to be saved and restored.

*   `PLAT`: Choose a platform to build ARM Trusted Firmware for. The chosen
    platform name must be the name of one of the directories under the `plat/`
    directory other than `common`.

*   `SPD`: Choose a Secure Payload Dispatcher component to be built into the
    Trusted Firmware. The value should be the path to the directory containing
    the SPD source, relative to `services/spd/`; the directory is expected to
    contain a makefile called `<spd-value>.mk`.

*   `V`: Verbose build. If assigned anything other than 0, the build commands
    are printed. Default is 0.

*   `ARM_GIC_ARCH`: Choice of ARM GIC architecture version used by the ARM GIC
    driver for implementing the platform GIC API. This API is used
    by the interrupt management framework. Default is 2 (that is, version 2.0).

*   `IMF_READ_INTERRUPT_ID`: Boolean flag used by the interrupt management
    framework to enable passing of the interrupt id to its handler. The id is
    read using a platform GIC API. `INTR_ID_UNAVAILABLE` is passed instead if
    this option set to 0. Default is 0.

*   `RESET_TO_BL31`: Enable BL3-1 entrypoint as the CPU reset vector instead
    of the BL1 entrypoint. It can take the value 0 (CPU reset to BL1
    entrypoint) or 1 (CPU reset to BL3-1 entrypoint).
    The default value is 0.

*   `CRASH_REPORTING`: A non-zero value enables a console dump of processor
    register state when an unexpected exception occurs during execution of
    BL3-1. This option defaults to the value of `DEBUG` - i.e. by default
    this is only enabled for a debug build of the firmware.

*   `ASM_ASSERTION`: This flag determines whether the assertion checks within
    assembly source files are enabled or not. This option defaults to the
    value of `DEBUG` - that is, by default this is only enabled for a debug
    build of the firmware.

*   `TSP_INIT_ASYNC`: Choose BL3-2 initialization method as asynchronous or
    synchronous, (see "Initializing a BL3-2 Image" section in [Firmware
    Design]). It can take the value 0 (BL3-2 is initialized using
    synchronous method) or 1 (BL3-2 is initialized using asynchronous method).
    Default is 0.

*   `USE_COHERENT_MEM`: This flag determines whether to include the coherent
    memory region in the BL memory map or not (see "Use of Coherent memory in
    Trusted Firmware" section in [Firmware Design]). It can take the value 1
    (Coherent memory region is included) or 0 (Coherent memory region is
    excluded). Default is 1.

*   `TSPD_ROUTE_IRQ_TO_EL3`: A non zero value enables the routing model
    for non-secure interrupts in which they are routed to EL3 (TSPD). The
    default model (when the value is 0) is to route non-secure interrupts
    to S-EL1 (TSP).

*   `TRUSTED_BOARD_BOOT`: Boolean flag to include support for the Trusted Board
    Boot feature. When set to '1', BL1 and BL2 images include support to load
    and verify the certificates and images in a FIP. The default value is '0'.
    A successful build, when `TRUSTED_BOARD_BOOT=1`, depends upon the correct
    initialization of the `AUTH_MOD` option. Generation and inclusion of
    certificates in the FIP depends upon the value of the `GENERATE_COT` option.

*   `AUTH_MOD`: This option is used when `TRUSTED_BOARD_BOOT=1`. It specifies
    the name of the authentication module that will be used in the Trusted Board
    Boot sequence. The module must be located in `common/auth/<module name>`
    directory. The directory must contain a makefile `<module name>.mk` which
    will be used to build the module. More information can be found in
    [Trusted Board Boot]. The default module name is 'none'.

*   `GENERATE_COT`: Boolean flag used to build and execute the `cert_create`
    tool to create certificates as per the Chain of Trust described in
    [Trusted Board Boot].  The build system then calls the `fip_create` tool to
    include the certificates in the FIP. Default value is '0'.

    Specify `TRUSTED_BOARD_BOOT=1` and `GENERATE_COT=1` to include support for
    the Trusted Board Boot Sequence in the BL1 and BL2 images and the FIP.

    Note that if `TRUSTED_BOARD_BOOT=0` and `GENERATE_COT=1`, the BL1 and BL2
    images will not include support for Trusted Board Boot. The FIP will still
    include the key and content certificates. This FIP can be used to verify the
    Chain of Trust on the host machine through other mechanisms.

    Note that if `TRUSTED_BOARD_BOOT=1` and `GENERATE_COT=0`, the BL1 and BL2
    images will include support for Trusted Board Boot, but the FIP will not
    include the key and content certificates, causing a boot failure.

*   `CREATE_KEYS`: This option is used when `GENERATE_COT=1`. It tells the
    certificate generation tool to create new keys in case no valid keys are
    present or specified. Allowed options are '0' or '1'. Default is '1'.

*   `ROT_KEY`: This option is used when `GENERATE_COT=1`. It specifies the
    file that contains the ROT private key in PEM format.

*   `TRUSTED_WORLD_KEY`: This option is used when `GENERATE_COT=1`. It
    specifies the file that contains the Trusted World private key in PEM
    format.

*   `NON_TRUSTED_WORLD_KEY`: This option is used when `GENERATE_COT=1`. It
    specifies the file that contains the Non-Trusted World private key in PEM
    format.

*   `BL30_KEY`: This option is used when `GENERATE_COT=1`. It specifies the
    file that contains the BL3-0 private key in PEM format.

*   `BL31_KEY`: This option is used when `GENERATE_COT=1`. It specifies the
    file that contains the BL3-1 private key in PEM format.

*   `BL32_KEY`: This option is used when `GENERATE_COT=1`. It specifies the
    file that contains the BL3-2 private key in PEM format.

*   `BL33_KEY`: This option is used when `GENERATE_COT=1`. It specifies the
    file that contains the BL3-3 private key in PEM format.

#### FVP specific build options

*   `FVP_TSP_RAM_LOCATION`: location of the TSP binary. Options:
    -   `tsram` : Trusted SRAM (default option)
    -   `tdram` : Trusted DRAM
    -   `dram`  : Secure region in DRAM (configured by the TrustZone controller)

For a better understanding of FVP options, the FVP memory map is explained in
the [Firmware Design].

#### Juno specific build options

*   `PLAT_TSP_LOCATION`: location of the TSP binary. Options:
    -   `tsram` : Trusted SRAM (default option)
    -   `dram`  : Secure region in DRAM (set by the TrustZone controller)

### Creating a Firmware Image Package

FIPs are automatically created as part of the build instructions described in
the previous section. It is also possible to independently build the FIP
creation tool and FIPs if required. To do this, follow these steps:

Build the tool:

    make -C tools/fip_create

It is recommended to remove the build artifacts before rebuilding:

    make -C tools/fip_create clean

Create a Firmware package that contains existing BL2 and BL3-1 images:

    # fip_create --help to print usage information
    # fip_create <fip_name> <images to add> [--dump to show result]
    ./tools/fip_create/fip_create fip.bin --dump \
       --bl2 build/<platform>/debug/bl2.bin --bl31 build/<platform>/debug/bl31.bin

     Firmware Image Package ToC:
    ---------------------------
    - Trusted Boot Firmware BL2: offset=0x88, size=0x81E8
      file: 'build/<platform>/debug/bl2.bin'
    - EL3 Runtime Firmware BL3-1: offset=0x8270, size=0xC218
      file: 'build/<platform>/debug/bl31.bin'
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
      --bl2 build/<platform>/release/bl2.bin

    Firmware Image Package ToC:
    ---------------------------
    - Trusted Boot Firmware BL2: offset=0x88, size=0x7240
      file: 'build/<platform>/release/bl2.bin'
    - EL3 Runtime Firmware BL3-1: offset=0x72C8, size=0xC218
    ---------------------------
    Updating "fip.bin"


### Debugging options

To compile a debug version and make the build more verbose use

    CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- \
    BL33=<path-to>/<bl33_image>                               \
    make PLAT=<platform> DEBUG=1 V=1 all fip

AArch64 GCC uses DWARF version 4 debugging symbols by default. Some tools (for
example DS-5) might not support this and may need an older version of DWARF
symbols to be emitted by GCC. This can be achieved by using the
`-gdwarf-<version>` flag, with the version being set to 2 or 3. Setting the
version to 2 is recommended for DS-5 versions older than 5.16.

When debugging logic problems it might also be useful to disable all compiler
optimizations by using `-O0`.

NOTE: Using `-O0` could cause output images to be larger and base addresses
might need to be recalculated (see the "Memory layout of BL images" section in
the [Firmware Design]).

Extra debug options can be passed to the build system by setting `CFLAGS`:

    CFLAGS='-O0 -gdwarf-2'                                    \
    CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- \
    BL33=<path-to>/<bl33_image>                               \
    make PLAT=<platform> DEBUG=1 V=1 all fip


### Building the Test Secure Payload

The TSP is coupled with a companion runtime service in the BL3-1 firmware,
called the TSPD. Therefore, if you intend to use the TSP, the BL3-1 image
must be recompiled as well. For more information on SPs and SPDs, see the
"Secure-EL1 Payloads and Dispatchers" section in the [Firmware Design].

First clean the Trusted Firmware build directory to get rid of any previous
BL3-1 binary. Then to build the TSP image and include it into the FIP use:

    CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- \
    BL33=<path-to>/<bl33_image>                               \
    make PLAT=<platform> SPD=tspd all fip

An additional boot loader binary file is created in the `build` directory:

*   `build/<platform>/<build-type>/bl32.bin`

The FIP will now contain the additional BL3-2 image. Here is an example
output from an FVP build in release mode including BL3-2 and using
FVP_AARCH64_EFI.fd as BL3-3 image:

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


### Building the Certificate Generation Tool

The `cert_create` tool can be built separately through the following commands:

    $ cd tools/cert_create
    $ make [DEBUG=1] [V=1]

`DEBUG=1` builds the tool in debug mode. `V=1` makes the build process more
verbose. The following command should be used to obtain help about the tool:

    $ ./cert_create -h

The `cert_create` tool is automatically built with the `fip` target when
`GENERATE_COT=1`.


### Building a FIP image with support for Trusted Board Boot

The Trusted Board Boot feature is described in [Trusted Board Boot]. The
following steps should be followed to build a FIP image with support for this
feature.

1.  Fulfill the dependencies of the `polarssl` authentication module by checking
    out the tag `polarssl-1.3.9` from the [PolarSSL Repository].

    The `common/auth/polarssl/polarssl.mk` contains the list of PolarSSL source
    files the module depends upon. `common/auth/polarssl/polarssl_config.h`
    contains the configuration options required to build the PolarSSL sources.

    Note that the PolarSSL SSL library is licensed under the GNU GPL version 2
    or later license. Using PolarSSL source code will affect the licensing of
    Trusted Firmware binaries that are built using this library.

2.  Ensure that the following command line variables are set while invoking
    `make` to build Trusted Firmware:

    *   `POLARSSL_DIR=<path of the directory containing PolarSSL sources>`
    *   `AUTH_MOD=polarssl`
    *   `TRUSTED_BOARD_BOOT=1`
    *   `GENERATE_COT=1`


### Checking source code style

When making changes to the source for submission to the project, the source
must be in compliance with the Linux style guide, and to assist with this check
the project Makefile contains two targets, which both utilise the
`checkpatch.pl` script that ships with the Linux source tree.

To check the entire source tree, you must first download a copy of
`checkpatch.pl` (or the full Linux source), set the `CHECKPATCH` environment
variable to point to the script and build the target checkcodebase:

    make CHECKPATCH=<path-to-linux>/linux/scripts/checkpatch.pl checkcodebase

To just check the style on the files that differ between your local branch and
the remote master, use:

    make CHECKPATCH=<path-to-linux>/linux/scripts/checkpatch.pl checkpatch

If you wish to check your patch against something other than the remote master,
set the `BASE_COMMIT` variable to your desired branch. By default, `BASE_COMMIT`
is set to `origin/master`.


5.  Obtaining the normal world software
---------------------------------------

### Obtaining EDK2

Potentially any kind of non-trusted firmware may be used with the ARM Trusted
Firmware but the software has only been tested with the EFI Development Kit 2
(EDK2) open source implementation of the UEFI specification.

To build the software to be compatible with the Foundation and Base FVPs, or the
Juno platform, follow these steps:

1.  Clone the [EDK2 source code][EDK2] from GitHub:

        git clone -n https://github.com/tianocore/edk2.git

    Not all required features are available in the EDK2 mainline yet. These can
    be obtained from the ARM-software EDK2 repository instead:

        cd edk2
        git remote add -f --tags arm-software https://github.com/ARM-software/edk2.git
        git checkout --detach v2.1-rc0

2.  Copy build config templates to local workspace

        # in edk2/
        . edksetup.sh

3.  Build the EDK2 host tools

        make -C BaseTools clean
        make -C BaseTools

4.  Build the EDK2 software

    1.  Build for FVP

            GCC49_AARCH64_PREFIX=<absolute-path-to-aarch64-gcc>/bin/aarch64-none-elf- \
            make -f ArmPlatformPkg/Scripts/Makefile EDK2_ARCH=AARCH64 \
            EDK2_DSC=ArmPlatformPkg/ArmVExpressPkg/ArmVExpress-FVP-AArch64.dsc \
            EDK2_TOOLCHAIN=GCC49 EDK2_BUILD=RELEASE \
            EDK2_MACROS="-n 6 -D ARM_FOUNDATION_FVP=1"

        The EDK2 binary for use with the ARM Trusted Firmware can then be found
        here:

             Build/ArmVExpress-FVP-AArch64/RELEASE_GCC49/FV/FVP_AARCH64_EFI.fd

    2.  Build for Juno

            GCC49_AARCH64_PREFIX=<absolute-path-to-aarch64-gcc>/bin/aarch64-none-elf- \
            make -f ArmPlatformPkg/ArmJunoPkg/Makefile EDK2_ARCH=AARCH64 \
            EDK2_TOOLCHAIN=GCC49 EDK2_BUILD=RELEASE

        The EDK2 binary for use with the ARM Trusted Firmware can then be found
        here:

            Build/ArmJuno/RELEASE_GCC49/FV/BL33_AP_UEFI.fd

    The EDK2 binary should be specified as `BL33` in in the `make` command line
    when building the Trusted Firmware. See the "Building the Trusted Firmware"
    section above.

5.  (Optional) To build EDK2 in debug mode, remove `EDK2_BUILD=RELEASE` from the
    command line.

6.  (Optional) To boot Linux using a VirtioBlock file-system, the command line
    passed from EDK2 to the Linux kernel must be modified as described in the
    "Obtaining a root file-system" section below.

7.  (Optional) If legacy GICv2 locations are used, the EDK2 platform description
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

Preparing a Linux kernel for use on the FVPs can be done as follows
(GICv2 support only):

1.  Clone Linux:

        git clone git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git

    Not all required features are available in the kernel mainline yet. These
    can be obtained from the ARM-software Linux repository instead:

        cd linux
        git remote add -f --tags arm-software https://github.com/ARM-software/linux.git
        git checkout --detach 1.3-Juno

2.  Build with the Linaro GCC tools.

        # in linux/
        make mrproper
        make ARCH=arm64 defconfig

        CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- \
        make -j6 ARCH=arm64

The compiled Linux image will now be found at `arch/arm64/boot/Image`.


6.  Preparing the images to run on FVP
--------------------------------------

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

### Preparing the kernel image

Copy the kernel image file `arch/arm64/boot/Image` to the directory from which
the FVP is launched. Alternatively a symbolic link may be used.

### Obtaining a root file-system

To prepare a Linaro LAMP based Open Embedded file-system, the following
instructions can be used as a guide. The file-system can be provided to Linux
via VirtioBlock or as a RAM-disk. Both methods are described below.

#### Prepare VirtioBlock

To prepare a VirtioBlock file-system, do the following:

1.  Download and unpack the disk image.

    NOTE: The unpacked disk image grows to 3 GiB in size.

        wget http://releases.linaro.org/14.12/openembedded/aarch64/vexpress64-openembedded_lamp-armv8-gcc-4.9_20141211-701.img.gz
        gunzip vexpress64-openembedded_lamp-armv8-gcc-4.9_20141211-701.img.gz

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
    "Running the software on FVP" section below.

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

        wget http://releases.linaro.org/14.12/openembedded/aarch64/linaro-image-lamp-genericarmv8-20141212-729.rootfs.tar.gz

2.  Modify the Linaro image:

        # Prepare for use as RAM-disk. Normally use MMC, NFS or VirtioBlock.
        # Be careful, otherwise you could damage your host file-system.
        mkdir tmp; cd tmp
        sudo sh -c "zcat ../linaro-image-lamp-genericarmv8-20141212-729.rootfs.tar.gz | cpio -id"
        sudo ln -s sbin/init .
        sudo sh -c "echo 'devtmpfs /dev devtmpfs mode=0755,nosuid 0 0' >> etc/fstab"
        sudo sh -c "find . | cpio --quiet -H newc -o | gzip -3 -n > ../filesystem.cpio.gz"
        cd ..

3.  Copy the resultant `filesystem.cpio.gz` to the directory where the FVP is
    launched from. Alternatively a symbolic link may be used.


7.  Running the software on FVP
-------------------------------

This version of the ARM Trusted Firmware has been tested on the following ARM
FVPs (64-bit versions only).

*   `Foundation_Platform` (Version 9.1, Build 9.1.33)
*   `FVP_Base_AEMv8A-AEMv8A` (Version 6.2, Build 0.8.6202)
*   `FVP_Base_Cortex-A57x4-A53x4` (Version 6.2, Build 0.8.6202)
*   `FVP_Base_Cortex-A57x1-A53x1` (Version 6.2, Build 0.8.6202)
*   `FVP_Base_Cortex-A57x2-A53x4` (Version 6.2, Build 0.8.6202)

NOTE: The build numbers quoted above are those reported by launching the FVP
with the `--version` parameter.

NOTE: The software will not work on Version 1.0 of the Foundation FVP.
The commands below would report an `unhandled argument` error in this case.

NOTE: The Foundation FVP does not provide a debugger interface.

Please refer to the FVP documentation for a detailed description of the model
parameter options. A brief description of the important ones that affect the
ARM Trusted Firmware and normal world software behavior is provided below.

The Foundation FVP is a cut down version of the AArch64 Base FVP. It can be
downloaded for free from [ARM's website][ARM FVP website].


### Running on the Foundation FVP with reset to BL1 entrypoint

The following `Foundation_Platform` parameters should be used to boot Linux with
4 CPUs using the ARM Trusted Firmware.

NOTE: Using the `--block-device` parameter is not necessary if a Linux RAM-disk
file-system is used (see the "Obtaining a File-system" section above).

NOTE: The `--data="<path to FIP binary>"@0x8000000` parameter is used to load a
Firmware Image Package at the start of NOR FLASH0 (see the "Building the
Trusted Firmware" section above).

    <path-to>/Foundation_Platform             \
    --cores=4                                 \
    --secure-memory                           \
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

Please refer to these notes in the subsequent "Running on the Base FVP"
sections.

1.  The `-C bp.flashloader0.fname` parameter is used to load a Firmware Image
    Package at the start of NOR FLASH0 (see the "Building the Trusted Firmware"
    section above).

2.  Using `cache_state_modelled=1` makes booting very slow. The software will
    still work (and run much faster) without this option but this will hide any
    cache maintenance defects in the software.

3.  Using the `-C bp.virtioblockdevice.image_path` parameter is not necessary
    if a Linux RAM-disk file-system is used (see the "Obtaining a root
    file-system" section above).

4.  Setting the `-C bp.secure_memory` parameter to `1` is only supported on
    Base FVP versions 5.4 and newer. Setting this parameter to `0` is also
    supported. The `-C bp.tzc_400.diagnostics=1` parameter is optional. It
    instructs the FVP to provide some helpful information if a secure memory
    violation occurs.

5.  This and the following notes only apply when the firmware is built with
    the `RESET_TO_BL31` option.

    The `--data="<path-to><bl31|bl32|bl33-binary>"@<base-address-of-binary>`
    parameter is used to load bootloader images into Base FVP memory (see the
    "Building the Trusted Firmware" section above). The base addresses used
    should match the image base addresses in `platform_def.h` used while linking
    the images. The BL3-2 image is only needed if BL3-1 has been built to expect
    a Secure-EL1 Payload.

6.  The `-C cluster<X>.cpu<Y>.RVBAR=@<base-address-of-bl31>` parameter, where
    X and Y are the cluster and CPU numbers respectively, is used to set the
    reset vector for each core.

7.  Changing the default value of `FVP_SHARED_DATA_LOCATION` will also require
    changing the value of
    `--data="<path-to><bl31-binary>"@<base-address-of-bl31>` and
    `-C cluster<X>.cpu<X>.RVBAR=@<base-address-of-bl31>`, to the new value of
    `BL31_BASE` in `platform_def.h`.

8.  Changing the default value of `FVP_TSP_RAM_LOCATION` will also require
    changing the value of
    `--data="<path-to><bl32-binary>"@<base-address-of-bl32>` to the new value of
    `BL32_BASE` in `platform_def.h`.


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
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>" \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"      \
    -C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>"

### Running on the AEMv8 Base FVP with reset to BL3-1 entrypoint

Please read "Notes regarding Base FVP configuration options" section above for
information about some of the options to run the software.

The following `FVP_Base_AEMv8A-AEMv8A` parameters should be used to boot Linux
with 8 CPUs using the ARM Trusted Firmware.

    <path-to>/FVP_Base_AEMv8A-AEMv8A                             \
    -C pctl.startup=0.0.0.0                                      \
    -C bp.secure_memory=1                                        \
    -C bp.tzc_400.diagnostics=1                                  \
    -C cluster0.NUM_CORES=4                                      \
    -C cluster1.NUM_CORES=4                                      \
    -C cache_state_modelled=1                                    \
    -C cluster0.cpu0.RVBAR=0x04023000                            \
    -C cluster0.cpu1.RVBAR=0x04023000                            \
    -C cluster0.cpu2.RVBAR=0x04023000                            \
    -C cluster0.cpu3.RVBAR=0x04023000                            \
    -C cluster1.cpu0.RVBAR=0x04023000                            \
    -C cluster1.cpu1.RVBAR=0x04023000                            \
    -C cluster1.cpu2.RVBAR=0x04023000                            \
    -C cluster1.cpu3.RVBAR=0x04023000                            \
    --data cluster0.cpu0="<path-to>/<bl31-binary>"@0x04023000    \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0x04001000    \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000    \
    -C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>"

### Running on the Cortex-A57-A53 Base FVP with reset to BL3-1 entrypoint

Please read "Notes regarding Base FVP configuration options" section above for
information about some of the options to run the software.

The following `FVP_Base_Cortex-A57x4-A53x4` model parameters should be used to
boot Linux with 8 CPUs using the ARM Trusted Firmware.

    <path-to>/FVP_Base_Cortex-A57x4-A53x4                        \
    -C pctl.startup=0.0.0.0                                      \
    -C bp.secure_memory=1                                        \
    -C bp.tzc_400.diagnostics=1                                  \
    -C cache_state_modelled=1                                    \
    -C cluster0.cpu0.RVBARADDR=0x04023000                        \
    -C cluster0.cpu1.RVBARADDR=0x04023000                        \
    -C cluster0.cpu2.RVBARADDR=0x04023000                        \
    -C cluster0.cpu3.RVBARADDR=0x04023000                        \
    -C cluster1.cpu0.RVBARADDR=0x04023000                        \
    -C cluster1.cpu1.RVBARADDR=0x04023000                        \
    -C cluster1.cpu2.RVBARADDR=0x04023000                        \
    -C cluster1.cpu3.RVBARADDR=0x04023000                        \
    --data cluster0.cpu0="<path-to>/<bl31-binary>"@0x04023000    \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0x04001000    \
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

    <path-to>/Foundation_Platform             \
    --cores=4                                 \
    --secure-memory                           \
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
    -C gicv3.gicv2-only=1                               \
    -C bp.variant=0x0

The `bp.variant` parameter corresponds to the build variant field of the
`SYS_ID` register.  Setting this to `0x0` allows the ARM Trusted Firmware to
detect the legacy VE memory map while configuring the GIC.


8.  Running the software on Juno
--------------------------------

### Preparing Trusted Firmware images

To execute the versions of software components on Juno referred to in this
document, the latest [Juno Board Recovery Image] must be installed. If you
have an earlier version installed or are unsure which version is installed,
follow the recovery image update instructions in the [Juno Software Guide]
on the [ARM Connected Community] website.

The Juno platform requires a BL3-0 image to boot up. This image contains the
runtime firmware that runs on the SCP (System Control Processor). This image is
embedded within the [Juno Board Recovery Image] but can also be
[downloaded directly][Juno SCP Firmware].

Rebuild the Trusted Firmware specifying the BL3-0 image. Refer to the section
"Building the Trusted Firmware". Alternatively, the FIP image can be updated
manually with the BL3-0 image:

    fip_create --dump --bl30 <path-to>/<bl30-binary> <path-to>/<FIP-binary>

### Obtaining the Flattened Device Tree

Juno's device tree blob is built along with the kernel. It is located in:

    <path-to-linux>/arch/arm64/boot/dts/juno.dtb

### Other Juno software information

Please refer to the [Juno Software Guide] to:

*   Deploy a root filesystem
*   Install and run the Juno binaries on the board
*   Obtain any other Juno software information


- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved._


[Firmware Design]:  ./firmware-design.md

[ARM FVP website]:             http://www.arm.com/fvp
[ARM Connected Community]:     http://community.arm.com
[Juno Software Guide]:         http://community.arm.com/docs/DOC-8396
[Juno Board Recovery Image]:   http://community.arm.com/servlet/JiveServlet/download/9427-1-15432/board_recovery_image_0.10.1.zip
[Juno SCP Firmware]:           http://community.arm.com/servlet/JiveServlet/download/9427-1-15422/bl30.bin.zip
[Linaro Toolchain]:            http://releases.linaro.org/14.07/components/toolchain/binaries/
[EDK2]:                        http://github.com/tianocore/edk2
[DS-5]:                        http://www.arm.com/products/tools/software-tools/ds-5/index.php
[Polarssl Repository]:         https://github.com/polarssl/polarssl.git
[Trusted Board Boot]:          trusted-board-boot.md

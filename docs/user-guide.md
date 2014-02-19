ARM Trusted Firmware User Guide
===============================

Contents :

1.  Introduction
2.  Using the Software
3.  Firmware Design
4.  References


1.  Introduction
----------------

The ARM Trusted Firmware implements a subset of the Trusted Board Boot
Requirements (TBBR) Platform Design Document (PDD) [1] for ARM reference
platforms. The TBB sequence starts when the platform is powered on and runs up
to the stage where it hands-off control to firmware running in the normal
world in DRAM. This is the cold boot path.

The ARM Trusted Firmware also implements the Power State Coordination Interface
([PSCI]) PDD [2] as a runtime service. PSCI is the interface from normal world
software to firmware implementing power management use-cases (for example,
secondary CPU boot, hotplug and idle). Normal world software can access ARM
Trusted Firmware runtime services via the ARM SMC (Secure Monitor Call)
instruction. The SMC instruction must be used as mandated by the [SMC Calling
Convention PDD][SMCCC] [3].


2.  Using the Software
----------------------

### Host machine requirements

The minimum recommended machine specification for building the software and
running the FVP (Fixed Virtual Platform) model is a dual-core processor running
at 2GHz with 12GB of RAM.  For best performance, use a machine with a quad-core
processor running at 2.6GHz with 16GB of RAM.

The software has been tested on Ubuntu 12.04.02 (64-bit).  Packages used
for building the software were installed from that distribution unless
otherwise specified.


### Tools

The following tools are required to use the ARM Trusted Firmware:

*   `git` package to obtain source code

*   `ia32-libs` package

*   `build-essential` and `uuid-dev` packages for building UEFI and the Firmware
    Image Package(FIP) tool

*   `bc` and `ncurses-dev` packages for building Linux

*   Baremetal GNU GCC tools. Verified packages can be downloaded from [Linaro]
    [Linaro Toolchain]. The rest of this document assumes that the
    `gcc-linaro-aarch64-none-elf-4.8-2013.09-01_linux.tar.xz` tools are used.

        wget http://releases.linaro.org/13.09/components/toolchain/binaries/gcc-linaro-aarch64-none-elf-4.8-2013.09-01_linux.tar.xz
        tar -xf gcc-linaro-aarch64-none-elf-4.8-2013.09-01_linux.tar.xz

*   The Device Tree Compiler (DTC) included with Linux kernel 3.12-rc4 is used
    to build the Flattened Device Tree (FDT) source files (`.dts` files)
    provided with this software.

*   (Optional) For debugging, ARM [Development Studio 5 (DS-5)][DS-5] v5.16.


### Building the Trusted Firmware

To build the software for the FVPs, follow these steps:

1.  Clone the ARM Trusted Firmware repository from Github:

        git clone https://github.com/ARM-software/arm-trusted-firmware.git

2.  Change to the trusted firmware directory:

        cd arm-trusted-firmware

3.  Set the compiler path, specify a Normal World (BL3-3) image and build:

        CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- BL33=<path-to-bl33> make PLAT=fvp

    By default this produces a release version of the build. To produce a debug
    version instead, refer to the "Debugging options" section below. UEFI can be
    used as the BL3-3 image, refer to the "Obtaining the normal world software"
    section below.

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

    For more information on the `fip.bin` image see the "Firmware Image Package"
    section below

4.  Copy the `bl1.bin` and `fip.bin` binary files to the directory from which
    the FVP will be launched. Symbolic links of the same names may be created
    instead.

5.  (Optional) Build products for a specific build variant can be removed using:

        make DEBUG=<D> PLAT=fvp clean

    ... where `<D>` is `0` or `1`, as specified when building.

    The build tree can be removed completely using:

        make realclean


#### Debugging options

To compile a debug version and make the build more verbose use

    CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- make PLAT=fvp DEBUG=1 V=1

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
    make PLAT=fvp DEBUG=1 V=1


NOTE: The Foundation FVP does not provide a debugger interface.


#### Checking source code style

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


### Obtaining the normal world software

#### Obtaining UEFI

TODO: Update UEFI GitHub hash.

Clone the [EDK2 (EFI Development Kit 2) source code][EDK2] from Github. This
version supports the Base and Foundation FVPs. EDK2 is an open source
implementation of the UEFI specification:

    git clone -n https://github.com/tianocore/edk2.git
    cd edk2
    git checkout 75f630347cace34e2d3abed2a5556ba71cfc50a9


To build the software to be compatible with Foundation and Base FVPs, follow
these steps:

1.  Change into the EDK2 source directory

        cd edk2

2.  Copy build config templates to local workspace

        . edksetup.sh

3.  Rebuild EDK2 host tools

        make -C BaseTools clean
        make -C BaseTools

4.  Build the software

        CROSS_COMPILE=<absolute-path-to-aarch64-gcc>/bin/aarch64-none-elf- \
        make -f ArmPlatformPkg/Scripts/Makefile EDK2_ARCH=AARCH64          \
        EDK2_DSC=ArmPlatformPkg/ArmVExpressPkg/ArmVExpress-FVP-AArch64.dsc \
        EDK2_TOOLCHAIN=ARMGCC EDK2_MACROS="-n 6 -D ARM_FOUNDATION_FVP=1"

    The EDK2 binary for use with the ARM Trusted Firmware can then be found
    here:

        Build/ArmVExpress-FVP-AArch64/DEBUG_ARMGCC/FV/FVP_AARCH64_EFI.fd

This will build EDK2 for the default settings as used by the FVPs.

To boot Linux using a VirtioBlock file-system, the command line passed from
EDK2 to the Linux kernel must be modified as described in the "Obtaining a
root file-system" section below.

If legacy GICv2 locations are used, the EDK2 platform description must be
updated. This is required as EDK2 does not support probing for the GIC
location.  To do this, build the software as described above with the
`ARM_FVP_LEGACY_GICV2_LOCATION` flag:

    -D ARM_FVP_LEGACY_GICV2_LOCATION=1

Then clean the source tree before rebuilding EDK2:

    make -f ArmPlatformPkg/Scripts/Makefile EDK2_ARCH=AARCH64          \
    EDK2_DSC=ArmPlatformPkg/ArmVExpressPkg/ArmVExpress-FVP-AArch64.dsc \
    EDK2_TOOLCHAIN=ARMGCC clean

The EDK2 binary `FVP_AARCH64_EFI.fd` should be specified as `BL33` to the
Trusted Firmware Makefile. See the "Building the Trusted Firmware" section.

#### Obtaining a Linux kernel

The software has been verified using Linux kernel version 3.12-rc4. Patches
have been applied to the kernel in order to enable CPU hotplug.

Preparing a Linux kernel for use on the FVPs with hotplug support can
be done as follows (GICv2 support only):

1.  Clone Linux:

        git clone git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git

    The CPU hotplug features are not yet included in the mainline kernel. To
    use these, add the patches from Mark Rutland's kernel, based on
    Linux 3.12-rc4:

        cd linux
        git remote add -f --tags markr git://linux-arm.org/linux-mr.git
        git checkout -b hotplug arm64-cpu-hotplug-20131023

2.  Build with the Linaro GCC tools.

        # in linux/
        make mrproper
        make ARCH=arm64 defconfig

        # Enable Hotplug
        make ARCH=arm64 menuconfig
        #   Kernel Features ---> [*] Support for hot-pluggable CPUs

        CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- \
        make -j6 ARCH=arm64

3.  Copy the Linux image `arch/arm64/boot/Image` to the working directory from
    where the FVP is launched. Alternatively a symbolic link may be used.

#### Obtaining the Flattened Device Trees

Depending on the FVP configuration and Linux configuration used, different
FDT files are required. FDTs for the Foundation and Base FVPs can be found in
the Trusted Firmware source directory under `fdts/`. The Foundation FVP has a
subset of the Base FVP components. For example, the Foundation FVP lacks CLCD
and MMC support, and has only one CPU cluster.

*   `fvp-base-gicv2-psci.dtb`

    (Default) For use with both AEMv8 and Cortex-A57-A53 Base FVPs with
    Base memory map configuration.

*   `fvp-base-gicv2legacy-psci.dtb`

    For use with both AEMv8 and Cortex-A57-A53 Base FVPs with legacy VE GIC
    memory map configuration.

*   `fvp-base-gicv3-psci.dtb`

    For use with AEMv8 Base FVP with Base memory map configuration and
    Linux GICv3 support.

*   `fvp-foundation-gicv2-psci.dtb`

    (Default) For use with Foundation FVP with Base memory map configuration.

*   `fvp-foundation-gicv2legacy-psci.dtb`

    For use with Foundation FVP with legacy VE GIC memory map configuration.

*   `fvp-foundation-gicv3-psci.dtb`

    For use with Foundation FVP with Base memory map configuration and Linux
    GICv3 support.


Copy the chosen FDT blob as `fdt.dtb` to the directory from which the FVP
is launched. Alternatively a symbolic link may be used.

#### Obtaining a root file-system

To prepare a Linaro LAMP based Open Embedded file-system, the following
instructions can be used as a guide. The file-system can be provided to Linux
via VirtioBlock or as a RAM-disk. Both methods are described below.

##### Prepare VirtioBlock

To prepare a VirtioBlock file-system, do the following:

1.  Download and unpack the disk image.

    NOTE: The unpacked disk image grows to 2 GiB in size.

        wget http://releases.linaro.org/13.09/openembedded/aarch64/vexpress64-openembedded_lamp-armv8_20130927-7.img.gz
        gunzip vexpress64-openembedded_lamp-armv8_20130927-7.img.gz

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

    1.  In EDK, edit the following file:

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
        -C bp.virtioblockdevice.image_path="<path-to>/vexpress64-openembedded_lamp-armv8_20130927-7.img"

    On the Foundation FVP:
        --block-device="<path-to>/vexpress64-openembedded_lamp-armv8_20130927-7.img"


5.  Ensure that the FVP doesn't output any error messages. If the following
    error message is displayed:

        ERROR: BlockDevice: Failed to open "vexpress64-openembedded_lamp-armv8_20130927-7.img"!

    then make sure the path to the file-system image in the model parameter is
    correct and that read permission is correctly set on the file-system image
    file.

##### Prepare RAM-disk

To prepare a RAM-disk root file-system, do the following:

1.  Download the file-system image:

        wget http://releases.linaro.org/13.09/openembedded/aarch64/linaro-image-lamp-genericarmv8-20130912-487.rootfs.tar.gz

2.  Modify the Linaro image:

        # Prepare for use as RAM-disk. Normally use MMC, NFS or VirtioBlock.
        # Be careful, otherwise you could damage your host file-system.
        mkdir tmp; cd tmp
        sudo sh -c "zcat ../linaro-image-lamp-genericarmv8-20130912-487.rootfs.tar.gz | cpio -id"
        sudo ln -s sbin/init .
        sudo ln -s S35mountall.sh etc/rcS.d/S03mountall.sh
        sudo sh -c "echo 'devtmpfs /dev devtmpfs mode=0755,nosuid 0 0' >> etc/fstab"
        sudo sh -c "find . | cpio --quiet -H newc -o | gzip -3 -n > ../filesystem.cpio.gz"
        cd ..

3.  Copy the resultant `filesystem.cpio.gz` to the directory where the FVP is
    launched from. Alternatively a symbolic link may be used.


### Running the software

This version of the ARM Trusted Firmware has been tested on the following ARM
FVPs (64-bit versions only).

*   `Foundation_v8` (Version 2.0, Build 0.8.5206)
*   `FVP_Base_AEMv8A-AEMv8A` (Version 5.2, Build 0.8.5202)
*   `FVP_Base_Cortex-A57x4-A53x4` (Version 5.2, Build 0.8.5202)

NOTE: The software will not work on Version 1.0 of the Foundation FVP.
The commands below would report an `unhandled argument` error in this case.

Please refer to the FVP documentation for a detailed description of the model
parameter options. A brief description of the important ones that affect the
ARM Trusted Firmware and normal world software behavior is provided below.

The Foundation FVP is a cut down version of the AArch64 Base FVP. It can be
downloaded for free from [ARM's website][ARM FVP website].

#### Running on the Foundation FVP

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
    --data="<path to bl1.bin>"@0x0            \
    --data="<path to FIP binary>"@0x8000000   \
    --block-device="<path-to>/vexpress64-openembedded_lamp-armv8_20130927-7.img"

The default use-case for the Foundation FVP is to enable the GICv3 device in
the model but use the GICv2 FDT, in order for Linux to drive the GIC in GICv2
emulation mode.

The memory mapped addresses `0x0` and `0x8000000` correspond to the start of
trusted ROM and NOR FLASH0 respectively.

#### Running on the AEMv8 Base FVP

The following `FVP_Base_AEMv8A-AEMv8A` parameters should be used to boot Linux
with 8 CPUs using the ARM Trusted Firmware.

NOTE: Using `cache_state_modelled=1` makes booting very slow. The software will
still work (and run much faster) without this option but this will hide any
cache maintenance defects in the software.

NOTE: Using the `-C bp.virtioblockdevice.image_path` parameter is not necessary
if a Linux RAM-disk file-system is used (see the "Obtaining a root file-system"
section above).

NOTE: The `-C bp.flashloader0.fname` parameter is used to load a Firmware Image
Package at the start of NOR FLASH0 (see the "Building the Trusted Firmware"
section above).

    <path-to>/FVP_Base_AEMv8A-AEMv8A                    \
    -C pctl.startup=0.0.0.0                             \
    -C bp.secure_memory=0                               \
    -C cluster0.NUM_CORES=4                             \
    -C cluster1.NUM_CORES=4                             \
    -C cache_state_modelled=1                           \
    -C bp.pl011_uart0.untimed_fifos=1                   \
    -C bp.secureflashloader.fname=<path to bl1.bin>     \
    -C bp.flashloader0.fname=<path to FIP binary>       \
    -C bp.virtioblockdevice.image_path="<path-to>/vexpress64-openembedded_lamp-armv8_20130927-7.img"

#### Running on the Cortex-A57-A53 Base FVP

The following `FVP_Base_Cortex-A57x4-A53x4` model parameters should be used to
boot Linux with 8 CPUs using the ARM Trusted Firmware.

NOTE: Using `cache_state_modelled=1` makes booting very slow. The software will
still work (and run much faster) without this option but this will hide any
cache maintenance defects in the software.

NOTE: Using the `-C bp.virtioblockdevice.image_path` parameter is not necessary
if a Linux RAM-disk file-system is used (see the "Obtaining a root file-system"
section above).

NOTE: The `-C bp.flashloader0.fname` parameter is used to load a Firmware Image
Package at the start of NOR FLASH0 (see the "Building the Trusted Firmware"
section above).

    <path-to>/FVP_Base_Cortex-A57x4-A53x4               \
    -C pctl.startup=0.0.0.0                             \
    -C bp.secure_memory=0                               \
    -C cache_state_modelled=1                           \
    -C bp.pl011_uart0.untimed_fifos=1                   \
    -C bp.secureflashloader.fname=<path to bl1.bin>     \
    -C bp.flashloader0.fname=<path to FIP binary>       \
    -C bp.virtioblockdevice.image_path="<path-to>/vexpress64-openembedded_lamp-armv8_20130927-7.img"

### Configuring the GICv2 memory map

The Base FVP models support GICv2 with the default model parameters at the
following addresses. The Foundation FVP also supports these addresses when
configured for GICv3 in GICv2 emulation mode.

    GICv2 Distributor Interface     0x2f000000
    GICv2 CPU Interface             0x2c000000
    GICv2 Virtual CPU Interface     0x2c010000
    GICv2 Hypervisor Interface      0x2c02f000

The Base FVP models can be configured to support GICv2 at addresses
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
UEFI images should be used.

#### Configuring AEMv8 Foundation FVP GIC for legacy VE memory map

The following parameters configure the Foundation FVP to use GICv2 with the
legacy VE memory map:

    <path-to>/Foundation_v8                  \
    --cores=4                                \
    --no-secure-memory                       \
    --visualization                          \
    --no-gicv3                               \
    --data="<path to bl1.bin>"@0x0           \
    --data="<path to FIP binary>"@0x8000000  \
    --block-device="<path-to>/vexpress64-openembedded_lamp-armv8_20130927-7.img"

Explicit configuration of the `SYS_ID` register is not required.


#### Configuring AEMv8 Base FVP GIC for legacy VE memory map

The following parameters configure the AEMv8 Base FVP to use GICv2 with the
legacy VE memory map:

NOTE: Using the `-C bp.virtioblockdevice.image_path` parameter is not necessary
if a Linux RAM-disk file-system is used (see the "Obtaining a root file-system"
section above).

    <path-to>/FVP_Base_AEMv8A-AEMv8A                    \
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
    -C bp.variant=0x0                                   \
    -C bp.virtioblockdevice.image_path="<path-to>/vexpress64-openembedded_lamp-armv8_20130927-7.img"

The `bp.variant` parameter corresponds to the build variant field of the
`SYS_ID` register.  Setting this to `0x0` allows the ARM Trusted Firmware to
detect the legacy VE memory map while configuring the GIC.

#### Configuring Cortex-A57-A53 Base FVP GIC for legacy VE memory map

The following parameters configure the Cortex-A57-A53 Base FVP to use GICv2 with
the legacy VE memory map:

NOTE: Using the `-C bp.virtioblockdevice.image_path` parameter is not necessary
if a Linux RAM-disk file-system is used (see the "Obtaining a root file-system"
section above).

    <path-to>/FVP_Base_Cortex-A57x4-A53x4               \
    -C legacy_gicv2_map=1                               \
    -C bp.virtioblockdevice.image_path="<path-to>/vexpress64-openembedded_lamp-armv8_20130927-7.img"

Explicit configuration of the `SYS_ID` register is not required.


3.  Firmware Design
-------------------

The cold boot path starts when the platform is physically turned on. One of
the CPUs released from reset is chosen as the primary CPU, and the remaining
CPUs are considered secondary CPUs. The primary CPU is chosen through
platform-specific means. The cold boot path is mainly executed by the primary
CPU, other than essential CPU initialization executed by all CPUs. The
secondary CPUs are kept in a safe platform-specific state until the primary
CPU has performed enough initialization to boot them.

The cold boot path in this implementation of the ARM Trusted Firmware is divided
into three stages (in order of execution):

*   Boot Loader stage 1 (BL1)
*   Boot Loader stage 2 (BL2)
*   Boot Loader stage 3 (BL3-1). The '1' distinguishes this from other 3rd level
    boot loader stages.

The ARM Fixed Virtual Platforms (FVPs) provide trusted ROM, trusted SRAM and
trusted DRAM regions. Each boot loader stage uses one or more of these
memories for its code and data.


### BL1

This stage begins execution from the platform's reset vector in trusted ROM at
EL3. BL1 code starts at `0x00000000` (trusted ROM) in the FVP memory map. The
BL1 data section is placed at the start of trusted SRAM, `0x04000000`. The
functionality implemented by this stage is as follows.

#### Determination of boot path

Whenever a CPU is released from reset, BL1 needs to distinguish between a warm
boot and a cold boot. This is done using a platform-specific mechanism. The
ARM FVPs implement a simple power controller at `0x1c100000`. The `PSYS`
register (`0x10`) is used to distinguish between a cold and warm boot. This
information is contained in the `PSYS.WK[25:24]` field. Additionally, a
per-CPU mailbox is maintained in trusted DRAM (`0x00600000`), to which BL1
writes an entrypoint. Each CPU jumps to this entrypoint upon warm boot. During
cold boot, BL1 places the secondary CPUs in a safe platform-specific state while
the primary CPU executes the remaining cold boot path as described in the
following sections.

#### Architectural initialization

BL1 performs minimal architectural initialization as follows.

*   Exception vectors

    BL1 sets up simple exception vectors for both synchronous and asynchronous
    exceptions. The default behavior upon receiving an exception is to set a
    status code. In the case of the FVP this code is written to the Versatile
    Express System LED register in the following format:

        SYS_LED[0]   - Security state (Secure=0/Non-Secure=1)
        SYS_LED[2:1] - Exception Level (EL3=0x3, EL2=0x2, EL1=0x1, EL0=0x0)
        SYS_LED[7:3] - Exception Class (Sync/Async & origin). The values for
                       each exception class are:

        0x0 : Synchronous exception from Current EL with SP_EL0
        0x1 : IRQ exception from Current EL with SP_EL0
        0x2 : FIQ exception from Current EL with SP_EL0
        0x3 : System Error exception from Current EL with SP_EL0
        0x4 : Synchronous exception from Current EL with SP_ELx
        0x5 : IRQ exception from Current EL with SP_ELx
        0x6 : FIQ exception from Current EL with SP_ELx
        0x7 : System Error exception from Current EL with SP_ELx
        0x8 : Synchronous exception from Lower EL using aarch64
        0x9 : IRQ exception from Lower EL using aarch64
        0xa : FIQ exception from Lower EL using aarch64
        0xb : System Error exception from Lower EL using aarch64
        0xc : Synchronous exception from Lower EL using aarch32
        0xd : IRQ exception from Lower EL using aarch32
        0xe : FIQ exception from Lower EL using aarch32
        0xf : System Error exception from Lower EL using aarch32

    A write to the LED register reflects in the System LEDs (S6LED0..7) in the
    CLCD window of the FVP. This behavior is because this boot loader stage
    does not expect to receive any exceptions other than the SMC exception.
    For the latter, BL1 installs a simple stub. The stub expects to receive
    only a single type of SMC (determined by its function ID in the general
    purpose register `X0`). This SMC is raised by BL2 to make BL1 pass control
    to BL3-1 (loaded by BL2) at EL3. Any other SMC leads to an assertion
    failure.

*   MMU setup

    BL1 sets up EL3 memory translation by creating page tables to cover the
    first 4GB of physical address space. This covers all the memories and
    peripherals needed by BL1.

*   Control register setup
    -   `SCTLR_EL3`. Instruction cache is enabled by setting the `SCTLR_EL3.I`
        bit. Alignment and stack alignment checking is enabled by setting the
        `SCTLR_EL3.A` and `SCTLR_EL3.SA` bits. Exception endianness is set to
        little-endian by clearing the `SCTLR_EL3.EE` bit.

    -   `CPUECTLR`. When the FVP includes a model of a specific ARM processor
        implementation (for example A57 or A53), then intra-cluster coherency is
        enabled by setting the `CPUECTLR.SMPEN` bit. The AEMv8 Base FVP is
        inherently coherent so does not implement `CPUECTLR`.

    -   `SCR`. Use of the HVC instruction from EL1 is enabled by setting the
        `SCR.HCE` bit. FIQ exceptions are configured to be taken in EL3 by
        setting the `SCR.FIQ` bit. The register width of the next lower
        exception level is set to AArch64 by setting the `SCR.RW` bit. External
        Aborts and SError Interrupts are configured to be taken in EL3 by
        setting the `SCR.EA` bit.

    -   `CPTR_EL3`. Accesses to the `CPACR_EL1` register from EL1 or EL2, or the
        `CPTR_EL2` register from EL2 are configured to not trap to EL3 by
        clearing the `CPTR_EL3.TCPAC` bit. Access to the trace functionality is
        configured not to trap to EL3 by clearing the `CPTR_EL3.TTA` bit.
        Instructions that access the registers associated with Floating Point
        and Advanced SIMD execution are configured to not trap to EL3 by
        clearing the `CPTR_EL3.TFP` bit.

    -   `CNTFRQ_EL0`. The `CNTFRQ_EL0` register is programmed with the base
        frequency of the system counter, which is retrieved from the first entry
        in the frequency modes table.

    -   Generic Timer. The system level implementation of the generic timer is
        enabled through the memory mapped interface.

#### Platform initialization

BL1 enables issuing of snoop and DVM (Distributed Virtual Memory) requests from
the CCI-400 slave interface corresponding to the cluster that includes the
primary CPU. BL1 also initializes UART0 (PL011 console), which enables access to
the `printf` family of functions.

#### BL2 image load and execution

BL1 execution continues as follows:

1.  BL1 determines the amount of free trusted SRAM memory available by
    calculating the extent of its own data section, which also resides in
    trusted SRAM. BL1 loads a BL2 raw binary image from platform storage, at a
    platform-specific base address. The filename of the BL2 raw binary image
    must be `bl2.bin`. If the BL2 image file is not present or if there is not
    enough free trusted SRAM the following error message is printed:

        "Failed to load boot loader stage 2 (BL2) firmware."

    If the load is successful, BL1 updates the limits of the remaining free
    trusted SRAM. It also populates information about the amount of trusted
    SRAM used by the BL2 image. The exact load location of the image is
    provided as a base address in the platform header. Further description of
    the memory layout can be found later in this document.

2.  BL1 prints the following string from the primary CPU to indicate successful
    execution of the BL1 stage:

        "Booting trusted firmware boot loader stage 1"

3.  BL1 passes control to the BL2 image at Secure EL1, starting from its load
    address.

4.  BL1 also passes information about the amount of trusted SRAM used and
    available for use. This information is populated at a platform-specific
    memory address.


### BL2

BL1 loads and passes control to BL2 at Secure EL1. BL2 is linked against and
loaded at a platform-specific base address (more information can be found later
in this document). The functionality implemented by BL2 is as follows.

#### Architectural initialization

BL2 performs minimal architectural initialization required for subsequent
stages of the ARM Trusted Firmware and normal world software. It sets up
Secure EL1 memory translation by creating page tables to address the first 4GB
of the physical address space in a similar way to BL1. EL1 and EL0 are given
access to Floating Point & Advanced SIMD registers by clearing the `CPACR.FPEN`
bits.

#### Platform initialization

BL2 does not perform any platform initialization that affects subsequent
stages of the ARM Trusted Firmware or normal world software. It copies the
information regarding the trusted SRAM populated by BL1 using a
platform-specific mechanism. It calculates the limits of DRAM (main memory)
to determine whether there is enough space to load the normal world software
images. A platform defined base address is used to specify the load address for
the BL3-1 image. It also defines the extents of memory available for use by the
BL3-2 image.

#### Normal world image load

BL2 loads the normal world firmware image (e.g. UEFI). BL2 relies on BL3-1 to
pass control to the normal world software image it loads. Hence, BL2 populates
a platform-specific area of memory with the entrypoint and Current Program
Status Register (`CPSR`) of the normal world software image. The entrypoint is
the load address of the normal world software image. The `CPSR` is determined as
specified in Section 5.13 of the [PSCI PDD] [PSCI]. This information is passed
to BL3-1.

#### BL3-2 (Secure Payload) image load

BL2 loads the optional BL3-2 image. The image executes in the secure world. BL2
relies on BL3-1 to pass control to the BL3-2 image, if present. Hence, BL2
populates a platform- specific area of memory with the entrypoint and Current
Program Status Register (`CPSR`) of the BL3-2 image. The entrypoint is the load
address of the BL3-2 image. The `CPSR` is initialized with Secure EL1 and Stack
pointer set to SP_EL1 (EL1h) as the mode, exception bits disabled (DAIF bits)
and AArch64 execution state. This information is passed to BL3-1.

##### UEFI firmware load

BL2 loads the BL3-3 (UEFI) image into non-secure memory as defined by the
platform (`0x88000000` for FVPs), and arranges for BL3-1 to pass control to that
location. As mentioned earlier, BL2 populates platform-specific memory with the
entrypoint and `CPSR` of the BL3-3 image.

#### BL3-1 image load and execution

BL2 execution continues as follows:

1.  BL2 loads the BL3-1 image into a platform-specific address in trusted SRAM
    and the BL3-3 image into a platform specific address in non-secure DRAM.
    The images are identified by the files `bl31.bin` and `bl33.bin` in
    platform storage. If there is not enough memory to load the images or the
    images are missing it leads to an assertion failure. If the BL3-1 image
    loads successfully, BL1 updates the amount of trusted SRAM used and
    available for use by BL3-1. This information is populated at a
    platform-specific memory address.

2.  BL2 passes control back to BL1 by raising an SMC, providing BL1 with the
    BL3-1 entrypoint. The exception is handled by the SMC exception handler
    installed by BL1.

3.  BL1 turns off the MMU and flushes the caches. It clears the
    `SCTLR_EL3.M/I/C` bits, flushes the data cache to the point of coherency
    and invalidates the TLBs.

4.  BL1 passes control to BL3-1 at the specified entrypoint at EL3.


### BL3-1

The image for this stage is loaded by BL2 and BL1 passes control to BL3-1 at
EL3. BL3-1 executes solely in trusted SRAM. BL3-1 is linked against and
loaded at a platform-specific base address (more information can be found later
in this document). The functionality implemented by BL3-1 is as follows.

#### Architectural initialization

Currently, BL3-1 performs a similar architectural initialization to BL1 as
far as system register settings are concerned. Since BL1 code resides in ROM,
architectural initialization in BL3-1 allows override of any previous
initialization done by BL1. BL3-1 creates page tables to address the first
4GB of physical address space and initializes the MMU accordingly. It replaces
the exception vectors populated by BL1 with its own. BL3-1 exception vectors
signal error conditions in the same way as BL1 does if an unexpected
exception is raised. They implement more elaborate support for handling SMCs
since this is the only mechanism to access the runtime services implemented by
BL3-1 (PSCI for example). BL3-1 checks each SMC for validity as specified by
the [SMC calling convention PDD][SMCCC] before passing control to the required
SMC handler routine.

#### Platform initialization

BL3-1 performs detailed platform initialization, which enables normal world
software to function correctly. It also retrieves entrypoint information for
the normal world software image loaded by BL2 from the platform defined
memory address populated by BL2.

* GICv2 initialization:

    -   Enable group0 interrupts in the GIC CPU interface.
    -   Configure group0 interrupts to be asserted as FIQs.
    -   Disable the legacy interrupt bypass mechanism.
    -   Configure the priority mask register to allow interrupts of all
        priorities to be signaled to the CPU interface.
    -   Mark SGIs 8-15, the secure physical timer interrupt (#29) and the
        trusted watchdog interrupt (#56) as group0 (secure).
    -   Target the trusted watchdog interrupt to CPU0.
    -   Enable these group0 interrupts in the GIC distributor.
    -   Configure all other interrupts as group1 (non-secure).
    -   Enable signaling of group0 interrupts in the GIC distributor.

*   GICv3 initialization:

    If a GICv3 implementation is available in the platform, BL3-1 initializes
    the GICv3 in GICv2 emulation mode with settings as described for GICv2
    above.

*   Power management initialization:

    BL3-1 implements a state machine to track CPU and cluster state. The state
    can be one of `OFF`, `ON_PENDING`, `SUSPEND` or `ON`. All secondary CPUs are
    initially in the `OFF` state. The cluster that the primary CPU belongs to is
    `ON`; any other cluster is `OFF`. BL3-1 initializes the data structures that
    implement the state machine, including the locks that protect them. BL3-1
    accesses the state of a CPU or cluster immediately after reset and before
    the MMU is enabled in the warm boot path. It is not currently possible to
    use 'exclusive' based spinlocks, therefore BL3-1 uses locks based on
    Lamport's Bakery algorithm instead. BL3-1 allocates these locks in device
    memory. They are accessible irrespective of MMU state.

*   Runtime services initialization:

    The only runtime service implemented by BL3-1 is PSCI. The complete PSCI API
    is not yet implemented. The following functions are currently implemented:

    -   `PSCI_VERSION`
    -   `CPU_OFF`
    -   `CPU_ON`
    -   `CPU_SUSPEND`
    -   `AFFINITY_INFO`

    The `CPU_ON`, `CPU_OFF` and `CPU_SUSPEND` functions implement the warm boot
    path in ARM Trusted Firmware. `CPU_ON` and `CPU_OFF` have undergone testing
    on all the supported FVPs. `CPU_SUSPEND` & `AFFINITY_INFO` have undergone
    testing only on the AEM v8 Base FVP. Support for `AFFINITY_INFO` is still
    experimental. Support for `CPU_SUSPEND` is stable for entry into power down
    states. Standby states are currently not supported. `PSCI_VERSION` is
    present but completely untested in this version of the software.

    Unsupported PSCI functions can be divided into ones that can return
    execution to the caller and ones that cannot. The following functions
    return with a error code as documented in the [Power State Coordination
    Interface PDD] [PSCI].

    -   `MIGRATE` : -1 (NOT_SUPPORTED)
    -   `MIGRATE_INFO_TYPE` : 2 (Trusted OS is either not present or does not
         require migration)
    -   `MIGRATE_INFO_UP_CPU` : 0 (Return value is UNDEFINED)

    The following unsupported functions do not return and signal an assertion
    failure if invoked.

    -   `SYSTEM_OFF`
    -   `SYSTEM_RESET`

    BL3-1 returns the error code `-1` if an SMC is raised for any other runtime
    service. This behavior is mandated by the [SMC calling convention PDD]
    [SMCCC].


### Normal world software execution

BL3-1 uses the entrypoint information provided by BL2 to jump to the normal
world software image (BL3-3) at the highest available Exception Level (EL2 if
available, otherwise EL1).


### Memory layout on FVP platforms

On FVP platforms, we use the Trusted ROM and Trusted SRAM to store the trusted
firmware binaries. BL1 is originally sitting in the Trusted ROM. Its read-write
data are relocated at the base of the Trusted SRAM at runtime. BL1 loads BL2
image near the top of the the trusted SRAM. BL2 loads BL3-1 image between BL1
and BL2. This memory layout is illustrated by the following diagram.

    Trusted SRAM
    +----------+ 0x04040000
    |          |
    |----------|
    |   BL2    |
    |----------|
    |          |
    |----------|
    |   BL31   |
    |----------|
    |          |
    |----------|
    | BL1 (rw) |
    +----------+ 0x04000000

    Trusted ROM
    +----------+ 0x04000000
    | BL1 (ro) |
    +----------+ 0x00000000

Each bootloader stage image layout is described by its own linker script. The
linker scripts export some symbols into the program symbol table. Their values
correspond to particular addresses. The trusted firmware code can refer to these
symbols to figure out the image memory layout.

Linker symbols follow the following naming convention in the trusted firmware.

*   `__<SECTION>_START__`

    Start address of a given section named `<SECTION>`.

*   `__<SECTION>_END__`

    End address of a given section named `<SECTION>`. If there is an alignment
    constraint on the section's end address then `__<SECTION>_END__` corresponds
    to the end address of the section's actual contents, rounded up to the right
    boundary. Refer to the value of `__<SECTION>_UNALIGNED_END__`  to know the
    actual end address of the section's contents.

*   `__<SECTION>_UNALIGNED_END__`

    End address of a given section named `<SECTION>` without any padding or
    rounding up due to some alignment constraint.

*   `__<SECTION>_SIZE__`

    Size (in bytes) of a given section named `<SECTION>`. If there is an
    alignment constraint on the section's end address then `__<SECTION>_SIZE__`
    corresponds to the size of the section's actual contents, rounded up to the
    right boundary. In other words, `__<SECTION>_SIZE__ = __<SECTION>_END__ -
    _<SECTION>_START__`. Refer to the value of `__<SECTION>_UNALIGNED_SIZE__`
    to know the actual size of the section's contents.

*   `__<SECTION>_UNALIGNED_SIZE__`

    Size (in bytes) of a given section named `<SECTION>` without any padding or
    rounding up due to some alignment constraint. In other words,
    `__<SECTION>_UNALIGNED_SIZE__ = __<SECTION>_UNALIGNED_END__ -
    __<SECTION>_START__`.

Some of the linker symbols are mandatory as the trusted firmware code relies on
them to be defined. They are listed in the following subsections. Some of them
must be provided for each bootloader stage and some are specific to a given
bootloader stage.

The linker scripts define some extra, optional symbols. They are not actually
used by any code but they help in understanding the bootloader images' memory
layout as they are easy to spot in the link map files.

#### Common linker symbols

Early setup code needs to know the extents of the BSS section to zero-initialise
it before executing any C code. The following linker symbols are defined for
this purpose:

* `__BSS_START__` This address must be aligned on a 16-byte boundary.
* `__BSS_SIZE__`

Similarly, the coherent memory section must be zero-initialised. Also, the MMU
setup code needs to know the extents of this section to set the right memory
attributes for it. The following linker symbols are defined for this purpose:

* `__COHERENT_RAM_START__` This address must be aligned on a page-size boundary.
* `__COHERENT_RAM_END__` This address must be aligned on a page-size boundary.
* `__COHERENT_RAM_UNALIGNED_SIZE__`

#### BL1's linker symbols

BL1's early setup code needs to know the extents of the .data section to
relocate it from ROM to RAM before executing any C code. The following linker
symbols are defined for this purpose:

* `__DATA_ROM_START__` This address must be aligned on a 16-byte boundary.
* `__DATA_RAM_START__` This address must be aligned on a 16-byte boundary.
* `__DATA_SIZE__`

BL1's platform setup code needs to know the extents of its read-write data
region to figure out its memory layout. The following linker symbols are defined
for this purpose:

* `__BL1_RAM_START__` This is the start address of BL1 RW data.
* `__BL1_RAM_END__` This is the end address of BL1 RW data.

#### BL2's and BL3-1's linker symbols

Both BL2 and BL3-1 need to know the extents of their read-only section to set
the right memory attributes for this memory region in their MMU setup code. The
following linker symbols are defined for this purpose:

* `__RO_START__`
* `__RO_END__`

#### How to choose the right base address for each bootloader stage image

The current implementation of the image loader has some limitations. It is
designed to load images dynamically, at a load address chosen to minimize memory
fragmentation. The chosen image location can be either at the top or the bottom
of free memory. However, until this feature is fully functional, the code also
contains support for loading images at a link-time fixed address.

BL1 is always loaded at address `0x0`. BL2 and BL3-1 are loaded at specified
locations in Trusted SRAM. The lack of dynamic image loader support means these
load addresses must currently be adjusted as the code grows. The individual
images must be linked against their ultimate runtime locations.

BL2 is loaded near the top of the Trusted SRAM. BL3-1 is loaded between BL1
and BL2. All three images are resident concurrently in Trusted RAM during boot
so overlaps are not permitted.

The image end addresses can be determined from the link map files of the
different images. These are the `build/<platform>/<build-type>/bl<x>/bl<x>.map`
files, with `<x>` the stage bootloader.

* `bl1.map` link map file provides `__BL1_RAM_END__` address.
* `bl2.map` link map file provides `__BL2_END__` address.
* `bl31.map` link map file provides `__BL31_END__` address.

To prevent images from overlapping each other, the following constraints must be
enforced:

1.  `__BL1_RAM_END__ <= BL31_BASE`
2.  `__BL31_END__ <= BL2_BASE`
3.  `__BL2_END__ <= (<Top of Trusted SRAM>)`

This is illustrated by the following memory layout diagram:

    +----------+ 0x04040000
    |          |
    |----------| __BL2_END__
    |   BL2    |
    |----------| BL2_BASE
    |          |
    |----------| __BL31_END__
    |   BL31   |
    |----------| BL31_BASE
    |          |
    |----------| __BL1_RAM_END__
    | BL1 (rw) |
    +----------+ 0x04000000

Overlaps are detected during image linking as follows.

Constraint 1 is enforced by BL1's linker script. If it is violated then the
linker will report an error while building BL1 to indicate that it doesn't
fit:

    aarch64-none-elf-ld: BL31 image overlaps BL1 image.

This error means that the BL3-1 base address needs to be incremented. Ensure
that the new memory layout still obeys all constraints.

Constraint 2 is enforced by BL3-1's linker script. If it is violated then the
linker will report an error while building BL3-1 to indicate that it doesn't
fit:

    aarch64-none-elf-ld: BL31 image overlaps BL2 image.

This error can either mean that the BL3-1 base address needs to be decremented
or that BL2 base address needs to be incremented. Ensure that the new memory
layout still obeys all constraints.

Constraint 3 is enforced by BL2's linker script. If it is violated then the
linker will report an error while building BL2 to indicate that it doesn't
fit. For example:

    aarch64-none-elf-ld: address 0x40400c8 of bl2.elf section `.bss' is not
    within region `RAM'

This error means that the BL2 base address needs to be decremented. Ensure that
the new memory layout still obeys all constraints.

Since constraint checks are scattered across linker scripts, it is required to
`make clean` prior to building to ensure that all possible overlapping scenarios
are checked.

The current implementation of the image loader can result in wasted space
because of the simplified data structure used to represent the extents of free
memory. For example, to load BL2 at address `0x0402D000`, the resulting memory
layout should be as follows:

    ------------ 0x04040000
    |          |  <- Free space (1)
    |----------|
    |   BL2    |
    |----------| BL2_BASE (0x0402D000)
    |          |  <- Free space (2)
    |----------|
    |   BL1    |
    ------------ 0x04000000

In the current implementation, we need to specify whether BL2 is loaded at the
top or bottom of the free memory. BL2 is top-loaded so in the example above,
the free space (1) above BL2 is hidden, resulting in the following view of
memory:

    ------------ 0x04040000
    |          |
    |          |
    |   BL2    |
    |----------| BL2_BASE (0x0402D000)
    |          |  <- Free space (2)
    |----------|
    |   BL1    |
    ------------ 0x04000000

BL3-1 is bottom-loaded above BL1. For example, if BL3-1 is bottom-loaded at
`0x0400E000`, the memory layout should look like this:

    ------------ 0x04040000
    |          |
    |          |
    |   BL2    |
    |----------| BL2_BASE (0x0402D000)
    |          |  <- Free space (2)
    |          |
    |----------|
    |          |
    |   BL31   |
    |----------|  BL31_BASE (0x0400E000)
    |          |  <- Free space (3)
    |----------|
    |   BL1    |
    ------------ 0x04000000

But the free space (3) between BL1 and BL3-1 is wasted, resulting in the
following view:

    ------------ 0x04040000
    |          |
    |          |
    |   BL2    |
    |----------| BL2_BASE (0x0402D000)
    |          |  <- Free space (2)
    |          |
    |----------|
    |          |
    |          |
    |   BL31   | BL31_BASE (0x0400E000)
    |          |
    |----------|
    |   BL1    |
    ------------ 0x04000000


### Firmware Image Package (FIP)

Using a Firmware Image Package (FIP) allows for packing bootloader images (and
potentially other payloads) into a single archive that can be loaded by the ARM
Trusted Firmware from non-volatile platform storage. A driver to load images
from a FIP has been added to the storage layer and allows a package to be read
from supported platform storage. A tool to create Firmware Image Packages is
also provided and described below.

#### Firmware Image Package layout

The FIP layout consists of a table of contents (ToC) followed by payload data.
The ToC itself has a header followed by one or more table entries. The ToC is
terminated by an end marker entry. All ToC entries describe some payload data
that has been appended to the end of the binary package. With the information
provided in the ToC entry the corresponding payload data can be retrieved.

    ------------------
    | ToC Header     |
    |----------------|
    | ToC Entry 0    |
    |----------------|
    | ToC Entry 1    |
    |----------------|
    | ToC End Marker |
    |----------------|
    |                |
    |     Data 0     |
    |                |
    |----------------|
    |                |
    |     Data 1     |
    |                |
    ------------------

The ToC header and entry formats are described in the header file
`include/firmware_image_package.h`. This file is used by both the tool and the
ARM Trusted firmware.

The ToC header has the following fields:
    `name`: The name of the ToC. This is currently used to validate the header.
    `serial_number`: A non-zero number provided by the creation tool
    `flags`: Flags associated with this data. None are yet defined.

A ToC entry has the following fields:
    `uuid`: All files are referred to by a pre-defined Universally Unique
        IDentifier [UUID] . The UUIDs are defined in
        `include/firmware_image_package`. The platform translates the requested
        image name into the corresponding UUID when accessing the package.
    `offset_address`: The offset address at which the corresponding payload data
        can be found. The offset is calculated from the ToC base address.
    `size`: The size of the corresponding payload data in bytes.
    `flags`: Flags associated with this entry. Non are yet defined.

#### Creating a Firmware Image Package

The FIP creation tool can be used to pack specified images into a binary package
that can be loaded by the ARM Trusted Firmware from platform storage. The tool
currently only supports packing bootloader images. Additional image definitions
can be added to the tool as required.

The tool can be found in `tools/fip_create`. Instructions on how to build and
use the tool follow.

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


#### Loading from a Firmware Image Package (FIP)

The Firmware Image Package (FIP) driver can load images from a binary package on
non-volatile platform storage. For the FVPs this currently NOR FLASH. For
information on how to load a FIP into FVP NOR FLASH see the "Running the
software" section.

Bootloader images are loaded according to the platform policy as specified in
`plat/<platform>/plat_io_storage.c`. For the FVPs this means the platform will
attempt to load images from a Firmware Image Package located at the start of NOR
FLASH0.

Currently the FVPs policy only allows for loading of known images. The platform
policy can be modified to add additional images.


### Code Structure

Trusted Firmware code is logically divided between the three boot loader
stages mentioned in the previous sections. The code is also divided into the
following categories (present as directories in the source code):

*   **Architecture specific.** This could be AArch32 or AArch64.
*   **Platform specific.** Choice of architecture specific code depends upon
    the platform.
*   **Common code.** This is platform and architecture agnostic code.
*   **Library code.** This code comprises of functionality commonly used by all
    other code.
*   **Stage specific.** Code specific to a boot stage.
*   **Drivers.**

Each boot loader stage uses code from one or more of the above mentioned
categories. Based upon the above, the code layout looks like this:

    Directory    Used by BL1?    Used by BL2?    Used by BL3?
    bl1          Yes             No              No
    bl2          No              Yes             No
    bl31         No              No              Yes
    arch         Yes             Yes             Yes
    plat         Yes             Yes             Yes
    drivers      Yes             No              Yes
    common       Yes             Yes             Yes
    lib          Yes             Yes             Yes

All assembler files have the `.S` extension. The linker source files for each
boot stage have the extension `.ld.S`. These are processed by GCC to create the
linker scripts which have the extension `.ld`.

FDTs provide a description of the hardware platform and are used by the Linux
kernel at boot time. These can be found in the `fdts` directory.


4.  References
--------------

1.  Trusted Board Boot Requirements CLIENT PDD (ARM DEN 0006B-5). Available
    under NDA through your ARM account representative.

2.  [Power State Coordination Interface PDD (ARM DEN 0022B.b)][PSCI].

3.  [SMC Calling Convention PDD (ARM DEN 0028A)][SMCCC].


- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved._


[Change Log]: change-log.md

[ARM FVP website]:  http://www.arm.com/fvp
[Linaro Toolchain]: http://releases.linaro.org/13.09/components/toolchain/binaries/
[EDK2]:             http://github.com/tianocore/edk2
[DS-5]:             http://www.arm.com/products/tools/software-tools/ds-5/index.php
[PSCI]:             http://infocenter.arm.com/help/topic/com.arm.doc.den0022b/index.html "Power State Coordination Interface PDD (ARM DEN 0022B.b)"
[SMCCC]:            http://infocenter.arm.com/help/topic/com.arm.doc.den0028a/index.html "SMC Calling Convention PDD (ARM DEN 0028A)"
[UUID]:             https://tools.ietf.org/rfc/rfc4122.txt "A Universally Unique IDentifier (UUID) URN Namespace"

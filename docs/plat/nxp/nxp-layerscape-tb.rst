TRUSTED_BOARD_BOOT option can be enabled by specifying
TRUSTED_BOARD_BOOT=1 on command line during make

By default the build considers that with TRUSTED_BOARD_BOOT option,
boot is being booted with secure boot i.e either SB_EN=1 or ITS=1.
This would mean that bl2.bin would get signed and header would be
embedded in the "bl2_<boot_src>_sec.pbl"
To explicity disable secure boot, use option SECURE_BOOT=false from
command line

2 options are provided for TRUSTED_BOARD_BOOT:
-------------------------------------------------------------------------
Option 1:
CoT using X 509 certificates
-------------------------------------------------------------------------

This CoT is as provided by ARM.
To use this option user needs to specify mbedtld dir path in
MBEDTLS_DIR.
GENERATE_COT=1 adds the certificates to the FIP image

ROTPK for x.509 certificates is generated and embedded in bl2.bin
and verified as part of CoT by Boot ROM during secure boot.

Typical command line to build this option

 make PLAT=<plat> all fip pbl SPD=opteed BL32=tee.bin BL33=u-boot.bin \
      RCW = <secure bot RCW>	\
      TRUSTED_BOARD_BOOT=1 GENERATE_COT=1 MBEDTLS_DIR=<mbedtls dir path>

-------------------------------------------------------------------------
Option 2:
CoT using traditional CSF headers.
-------------------------------------------------------------------------

This option is automatically selected when TRUSTED_BOARD_BOOT is set
but MBEDTLS_DIR path is not specified.

CSF header is embedded to each of the BL31, BL32 and  BL33 image.

To generate CSF header, path of CST repository needs to be specified
as CST_DIR

Default input files for CSF header generation is added in this repo.
Default input file requires user to generate RSA key pair named
srk.pri and srk.pub and add them in ATF repo. The keys can be generated
using gen_keys tool of CST.

To change the input file , user can use the options
BL33_INPUT_FILE, BL32_INPUT_FILE, BL31_INPUT_FILE

There are 2 paths in secure boot flow :
1. development Mode (sb_en = 1, its = 0)
In this flow , even on ROTPK comparison failure, flow would continue.
However SNVS is transitioned to non-secure state

2. Production mode (ITS =1)
Any failure is fatal failure

TRUSTED_BOARD_BOOT can be enabled in non secure boot flow also. ROTPK
would be ignored in that case and failures won't result in snvs transition.

1. Generate the SRK Key Pair.
From CST
./gen_keys <key_sz>
 Copy srk.pri/srk.pub to TF-A repo

(To change the key/key-names change the requierd input file in
drivers/nxp/csf_hdr_parser/

(For more details of CST refer to NXP QorIQ LSDK documentation)

2. make PLAT=<plat> all fip pbl SPD=opteed BL32=tee.bin BL33=u-boot.bin \
      RCW = <secure bot RCW>	\
      TRUSTED_BOARD_BOOT=1 CST_DIR=<cst dir path>

To use user provided input files :
make PLAT=<plat> all fip pbl SPD=opteed BL32=tee.bin BL33=u-boot.bin \
      RCW = <secure bot RCW>	\
      TRUSTED_BOARD_BOOT=1 CST_DIR=<cst dir path> BL33_INPUT_FILE=<ip file> BL32_INPUT_FILE=<ip_file> \
      BL31_INPUT_FILE = <ip file>

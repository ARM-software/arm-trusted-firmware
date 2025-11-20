# Change Log & Release Notes

This document contains a summary of the new features, changes, fixes and known
issues in each release of Trusted Firmware-A.

## [2.14.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.13.0..refs/tags/v2.14.0) (2025-11-20)

### ⚠ BREAKING CHANGES

- **Services**

  - **SPM**

    - **EL3 SPMC**

      - no longer accepts invalid fragment offsets

        **See:** validate fragment offset ([c55b519](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c55b519ed87997f8136ebce6d39e08d08a6e9a2a))

      - Zeroes or forwards a different set of registers,
          depending on the FF-A version of the source and destination.
          E.g. a call from a v1.1 caller to a v1.2 destination will zero
          out the extended registers, which is different from the old behavior
          of forwarding everything to EL2 SPMC, but only x0-x7 to the EL3 SPMC.

        **See:** fixed x8-x17 register handling for FFA 1.2 ([bb9fc8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb9fc8c0bbf5f68f7461296ba36bf398091cb675))

- **Libraries**

  - **Firmware Handoff**

    - Projects/scripts relying on the in-tree TLC tool will
      need to use the new, externalized location or workflow.

      **See:** remove in-tree TLC implementation ([3ff7523](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3ff7523883adb4ddc9503324282ecde06c5e5f3c))

    - LibTL is now included in TF-A as a submodule.
        Please run `git submodule update --init --recursive` if you encounter
        issues after migrating to the latest version of TF-A.

      **See:** switch to LibTL submodule ([b5d0740](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b5d0740e14f428f2c5341d1222d0769bdde35ea3))

- **Drivers**

  - **Measured Boot**

    - LibEventLog is now included in TF-A as a submodule.
        Please run `git submodule update --init --recursive` if you encounter
        issues after migrating to the latest version of TF-A.

      **See:** move to ext event log lib ([b67e984](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b67e984664a8644d6cfd1812cabaa02cf24f09c9))

- **Tools**

  - **Memory Mapping Tool**

    - The image memory map visualization tool now uses
        commands, rather than arguments, to determine the behaviour of the
        script. See the commit message for further details.

      **See:** change behavioural flags to commands ([8daebef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8daebefe59e51bbf562d0e9726a5183d5d265d26))

### New Features

- **Architecture**

  - **CPU feature like FEAT_XXXX / ID register handling in general**

    - add ID register defines and read helpers ([8515b43](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8515b4395e62c771169909453ce2eaece6c1221e))
    - add support for FEAT_FGWTE3 ([4274b52](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4274b5265576c1c27680970978066d05b7b2b96b))
    - add support for FEAT_IDTE3 ([f396aec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f396aec85ac52e1ee152082e635552b5c63b7583))
    - add support for FEAT_RME_GPC2 ([09a4bcb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/09a4bcb81bc37def40ecb638b121e00e05bcbcbd))
    - do feature detection on secondary cores too ([d335bbb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d335bbb1e20d4a8f0a6a26b97ba2a710015bf727))
    - enable FEAT_AIE support ([cc2523b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc2523bb21cc254642bd70a6812c5c0d69ed4225))
    - enable FEAT_CPA2 for EL3 ([a1032be](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a1032beb656d78d1cffc97fa64c961d098b23b48))
    - enable FEAT_MPAM_PE_BW_CTRL support ([c42aefd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c42aefd3eb1b5888ee6f3d1f8645b62ec850cdcc))
    - enable FEAT_PFAR support ([b3bcfd1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b3bcfd12c8469df79b212647b9eb2743d7dbb070))
    - fix build with ENABLE_FEAT_PAUTH_LR=1 ([3325415](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3325415bd1155854e1d2b4c3b8effd06f319014a))
    - include enabled security state scope ([d508bab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d508bab30dfc96fc837ae82bfa0e52f33d617c79))
    - introduce FEAT_RME_GDI support ([5e827bf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5e827bf0296efeea9c99dd93ff5d346a928452bd))

  - **DynamIQ Shared Unit (DSU)**

    - enable PMU registers access at EL1 ([1f866fc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1f866fc9763370503a6596cafada9e5a4d6531f5))
    - support power control and autonomous powerdown config ([d52ff2b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d52ff2b3a9da29ee498f2c287c172bbdbfcb461b))

- **Platforms**

  - **AMD**

    - **Versal Gen 2**

      - add hooks for mmap and early setup ([4efae6a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4efae6ab0d6a4de5aaadf927bb65da649e022597))
      - add SPMD support for SPMC at S-EL1 ([c7ddb0f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c7ddb0f3cde76d50cc6ad6dfb994ca08c95620d6))
      - add support of MMI_GEM as wakeup source ([4589ce0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4589ce0a58c58785d33f32badcdc117be61e9883))
      - parse reserve memory subnodes ([59eaed0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/59eaed03723208bb4cf1ead13012dff733261922))
      - remove handoff entry from tl ([9029408](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/90294080aaae7e7ed1f70e47c4fdf78b06437d92))
      - validate non-secure entry addr ([27e7222](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/27e7222106021042451660104f58967a86387f6a))

  - **Arm**

    - add pseudo CRB area ([d771d57](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d771d57a45bd4832e043d34a19dd3a20e322213a))
    - handle RMM ep_info during LFA ([4ea0ebc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4ea0ebc29b6a7a17d87f451b8adf158b3a02db7c))
    - implement arm platform GPT logging ([fe3299d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fe3299d1b2db7f2a17cb76203b70a142b78c0819))
    - introduce ARM_SPM_NS_MEM_BASE and move NS buffer ([78a6c8f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/78a6c8fffcf73c0df027d4787376e0f62ce6994a))
    - load config after GPT FIP offset ([6757473](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/67574733960b797dc56e24884e68ea3cc0f6739f))
    - load tos_fw_cfg using xferlist in SPMC_AT_EL3 ([00c353c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/00c353c4678a14be74e2840f8c03184d57560a83))
    - unify Linux kernel as BL33 handling ([2f5fd82](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2f5fd8267a2159338924d6fcaa0b30de98b1aa0c))

    - **Common**

      - add support for kernel DT handoff convention ([291e493](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/291e493d15c9c0b3ddec705f866b716a0190bf5e))

    - **FVP**

      - add a GICv5 device tree ([d358eb2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d358eb214d7d12dae209938f44022da1ca5654a4))
      - add extra DRAM configuration for TZC ([887cdf4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/887cdf484cacc87cc8f51a573db68b758df32f33))
      - add firmware update agent uuid in StandaloneMm ([28d325c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28d325c33d11137b1f69e7a06adbd9df43bcd507))
      - add FVP_HW_CONFIG_ADDR make variable ([93fc69d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/93fc69deb40923a7cd9b4abd6baf5ebda4ddc2f9))
      - add GICv5 support ([e2e90fa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e2e90fa1cd8ebe344b3552fa762d3fd3d63d5721))
      - add pseudo CRB area ([235d975](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/235d9754c037f4e343364ef23f53de9a7d381f64))
      - add SoC name support to FVP ([cd802c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd802c29aeeba30a8f54ef5c130d72d3acb53fb0))
      - add StandaloneMm manifest for rust-spmc ([35721cb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/35721cb6e2f8810d995671ea078ccdff467a0ca3))
      - add stub implementation for plat_lfa_notify_activate() ([31e9fd9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/31e9fd9cc883a48c0849f233a94200c0ca5dd2dc))
      - add the GICv5 config ([9a099b5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9a099b510d6374747012e291aa6b2408dc84b76b))
      - bump the SPMC version ([4d29a8f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d29a8fa9849add903cf8d10c4553edc83f3f96b))
      - change PLAT_SP_IMAGE_NS_BUF_BASE ([b19b693](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b19b693421f37f0066047e8160d256a3f93f16ed))
      - enable kernel dt convention ([8946bb0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8946bb03c6c473497a29eba1340e60ca10cb0c6c))
      - give bootargs on all configs ([73c587e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/73c587ec986865741945b1a4f4ecaabf8f7ce641))
      - implement LFA get components API ([3bb5266](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3bb52661c1f0d92ee4d8bd369b6bed0b98162d08))
      - implement platform API for LFA activation pending check ([357079c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/357079c760dff2b1009f44177283e8c79451b971))
      - implement platform API for LFA cancel operation ([b4cbf50](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b4cbf5088c2f32e01bcd6aabd1eb04bd2d8a95d2))
      - implement platform API for load and auth image ([a7fbccc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a7fbcccd43a42ad6e1a8069f16bc53bbde961096))
      - increase bl2 mmap tables for handoff ([25688b8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/25688b873f2e9173fa61a59ca4be361a1420f29b))
      - increase PLAT_ARM_SPMC_SIZE for rust-spmc ([1cc0294](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1cc0294598f82b8e5bc474b13e2b99b274473665))
      - increase secure partition's table mapping count ([b1f527a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b1f527ab995be99eade39f3b4189de46b29e09c0))
      - increase xtable for pseudo CRB for SPM_MM ([8569456](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/856945605ef7e4564a986efd8b49ef9d60772976))
      - increase xtable for pseudo CRB for SPMC_AT_EL3 ([3d35b10](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3d35b101d8612c3061e7c8db51908d95f0768d49))
      - initialize LFA component activators in platform layer ([b100e91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b100e91dc944c45279e6581ef2da2dfff41650e1))
      - introduce fvp_stmm_bl2_sp_list.dts for StandaloneMm ([98ae901](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/98ae901799d06b9381a5d988c83f608fc701e39f))
      - move PLAT_ARM_SPMC_SP_MANIFEST_SIZE definition ([3c90095](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3c90095d0d32092fd1d819dff9f6f1500364d0d9))
      - organize fvp_stmm_manifest.dts ([8d66892](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8d66892ab883e902d4438cbfc6a128f13e29df93))
      - tos_fw_config with transfer list ([bc3014a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc3014a80c4efe55cf1d8a36558510f6b7c224d1))
      - update evtlog info in the xferlist's DT_SPMC_MANIFEST entry ([10f6ccd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10f6ccdc2893c3b4d39d2641bc47ca1c14e48d28))

    - **Juno**

      - add pseudo CRB area ([66579ca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/66579ca003a602bd8cf8cf6e9457b04ee0bf93a1))
      - boot with TRANSFER_LIST ([fad8844](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fad88444e30621ec711dedf9ff5b4a3675b2b3a9))
      - change PLAT_SP_IMAGE_NS_BUF_BASE ([b53b69c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b53b69ca80cc0581157ced2e815215fb62588d7a))
      - change preprocessor condition for plat_get_mbedtls_heap() ([1776a1e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1776a1ef3414f19922667621ed6296c20c750166))
      - change the FW_NS_HANDOFF_BASE ([eee8963](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eee89638959c6fc04282269ba8b5e91e3b0019c4))
      - increase MAX_XLAT_TABLES for SPMC_AT_EL3 ([c76e828](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c76e8284fa61cb8488d9191e769590856b1df1ce))
      - increase xtable for pseudo CRB ([7d142cb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d142cb5b9ed4d5fd6b87720f4015f59b52e7748))
      - organize juno_stmm_manifest.dts ([8706efc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8706efcb365d3b57497f2a4a5d57fac657c38fbe))
      - support image measured boot ([1c199c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1c199c54fc900e7e6dfc1155efb570b2ad21d938))
      - support StandaloneMm ([e9a457f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e9a457f4abf5e3849c863c7395dc3d973c033926))

    - **Neoverse-RD**

      - change PLAT_SP_IMAGE_NS_BUF_BASE ([c4d39b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c4d39b7214020819df63d23ed4e16399d49b8eb0))

      - **RD-V3**

        - enable numa aware per-cpu for RD-V3-Cfg2 ([8e85be4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8e85be4474baa2a51542d396d430d0441db55296))

    - **TC**

      - add support for configuring DSU settings ([fd4e602](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fd4e60268e53c49b61fbe98e874a443afb9e384d))
      - bump SPMC version to FF-A v1.3 TC platform ([838eb53](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/838eb53d9f1fc19d8428a64514595d8fd43e342e))
      - change PLAT_SP_IMAGE_NS_BUF_BASE ([22e97b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/22e97b78ec851b7f4d6c1c58fe0a504a9fbe2b55))

    - **Automotive RD**

      - **RD-Aspen**

        - add BL31 for RD-Aspen platform ([c2cd362](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c2cd362c423d4a60d8123ea1570e049c611eb124))
        - add DSU support ([d69c3b1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d69c3b1c204b0d9fe6bc9a8d51f48701a206d548))
        - add DSU to the device tree ([b0a8c52](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b0a8c52e362f4619498fe50e138520067854fac9))
        - add support for configurable platform's CPU topology ([c16a3b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c16a3b7c33f6b5e1ec379c129f1715339b561e24))
        - enable tbb on rd-aspen platform ([287e24f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/287e24f510d445023e57a5bff090190e267245c8))
        - introduce Arm RD-Aspen platform ([d1a1abe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d1a1abeca9bcd40d313ead4ae6ad0ee87d5e1f96))
        - scmi gracefully shutdown system ([ba4814b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ba4814b89de536ecc40a07c29bb3f1e31e31ba38))
        - support BL32 (OP-TEE) ([33a10dc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33a10dcade6dcec628523fd8638ef9714ddcb54a))
        - support configurable CPU topology in device tree ([b666f0a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b666f0a1a9b554382ebd8190a365c5750d0d85e1))

  - **Intel**

    - migrate RSU client to SiPSVC V3 ([b85b49e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b85b49e40c0f62d6c68800a8bd9107fd7c8192c5))
    - move common functions to common lib files ([6fcd047](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6fcd047bd32bd6146a6d1714396bb0406df093c1))
    - support IO96B ECC Error Injection via SMC call ([bdcd41d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bdcd41dd1f5de3da0f8e774343ed0032770153de))
    - update CPUECTLR_EL1 to boost ethernet performance ([bb9e34f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb9e34f99ca7138d4380694701d4625fb1ab8aab))

  - **MediaTek**

    - add APIs exposed to the static library ([97881aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/97881aacebc2bff6b1992b035d986ec2708fe7ec))

    - **MT8189**

      - add dfd driver ([aed177c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aed177ce0067b77ca39c85d0ae1d733c874d1541))
      - add EC pin control in SPM ([ed11c2f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ed11c2ff203c633bedee1af982092c458a941349))
      - add hotplug driver ([b8d63a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b8d63a7a48dd095c520cce5cfd6c310073f9fe67))
      - add IOMMU enable control in SiP service ([4c449fc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4c449fcad3c544d6d6847118a569e1be4eda71c2))
      - add LPM v2 support ([d92ee8e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d92ee8e9070975c61f2a73cc8c28c46d5aa6f2e7))
      - add mcdi driver ([5f00709](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f00709ebd56d63950dabd335784f4581262843c))
      - add mt8189 mtcmos platform data ([d8c718c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d8c718c502549b0c411773ae8344acda5d657ba9))
      - add mtcmos config ([7e864c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e864c7f40a4c5a4d9b6fcf584ae5a70ae23abc5))
      - add pmic driver ([868b2d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/868b2d60743bbb44ff247fde7b57e3983f0a71ac))
      - add PWRAP driver ([9c9324c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9c9324cc96ebbf7775b1424d2f466eba89b7f7d3))
      - add reset and poweroff function for PSCI call ([148152f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/148152f3bbb4587258b362165f254cc4a58291f9))
      - add SPM basic features support ([65db67b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/65db67b8ba5e8c7c649f5798c6cdb738447d2b4e))
      - add SPM common driver support ([5f748b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f748b3cbb794e9de9c48d7b07a541f26317d46a))
      - add SPM features support ([083cfad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/083cfadb6a246c89e306f5a83d05cad90e3bab1f))
      - add SPM low power mode ([3289678](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/32896781b63e9258f160f3b9763890a8c6e48505))
      - add SPMI driver ([257aa94](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/257aa94fa6e6db1da27b648092ac7ab03420ce73))
      - add support display driver ([6997076](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/69970765fa9f9ec7a9de2408e50ea21d681df1c2))
      - add support for PTP3 ([3f446df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3f446df45f1af055718d61459ee98019907d09af))
      - add thermal driver support ([3da2d29](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3da2d29cde87f8d571813982fd64d2c84483673a))
      - add UFS driver support ([897c2d0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/897c2d07f9f6c22ea23e06e35ac8f1a52e8ca3c3))
      - add UFS functions used by libbl31.a ([260e18b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/260e18b17c095521b8a989c184aa53415e63fe36))
      - add VCORE DVFS drivers ([5e91cfd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5e91cfd69c49dbfac79b112be8c04aa9ed0046ca))
      - enable DCM to support LP mode ([f018e05](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f018e05d23ec077b1c2e44fa583e5695770e8024))
      - enable PMIC low power setting ([d701cf8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d701cf8152b94548fc75a529549e09a0127de829))
      - fix the typo in the display driver ([0f462e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0f462e65142e1cdb53cb98f34f4130260a5b568d))
      - link prebuilt library ([4dfb819](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4dfb8193787ca0c54b93a16f459c0a3cbfb605d8))

    - **MT8196**

      - add CPU QoS stub implementation ([4488b22](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4488b229e57398bc71ec49507a30437e8294c3b0))
      - add MMinfra support ([c33b98d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c33b98d7b84d58e9177da37614e22ff576496f08))
      - add SLBC SiP handler ([e86fb81](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e86fb819834f52eb35b142456f3e6e4fec185e58))
      - add SMMU SID stub implementation ([5be0e22](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5be0e22591d764156ad5e6bc1c952f986c0595e1))
      - add UFS functions used by the static library ([31a69d9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/31a69d9ae79c57d349bb04444a5bbd6267957c87))
      - enable MTE2 ([e2ad194](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e2ad194d4573799204542a6866f110ce9250d245))
      - extract common SPM code for reuse ([532ac05](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/532ac05729c4e0ae4c44f39ac34869e35b38a6ba))

  - **NXP**

    - **i.MX**

      - **i.MX 9**

        - add full system power control option ([0df6ba3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0df6ba31aa4a7761abb8bfda7151148439ec2b8b))
        - add imx9 common code base ([4ddfb6f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4ddfb6f199ad93824611a9bcd4cb62ae13559760))
        - add necessary ele api call support ([1168465](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/11684655ebed02e7dbbe4efd069f25c369150ee8))

        - **i.MX94**

          - add initial support for imx94 ([4249a4f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4249a4fba8f03af1c3cb651c70e3d21e6d0ecd3e))

        - **i.MX95**

          - add initial support for i.MX95 ([d70b09f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d70b09f8d755f2331d3baa63f0d2ab45831bb51b))
          - add optee support ([7bde9a4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7bde9a4e60162739c3848c62ceb471f4695ab14b))
          - implement a semaphore for GIC quiescing ([f7e7ea1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f7e7ea1fa39f6fe9601c84c19f72029c4d2c257c))
          - support trusty os ([b182f70](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b182f709847ff793e0df35c6f58210451bc1e270))

    - **S32G274A**

      - move fip in a dedicated partition ([88b8aa9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/88b8aa976247bbb35c4e2d4bfc9e775ab6d83da9))

      - **S32G274ARDB**

        - add custom DDR FW UUID entry ([4d9ac8f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d9ac8f0ae47fbc892b605d46d0f1f815e388a45))
        - add DDR clock source support ([c0cbf5a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0cbf5ad6e5d6ef6ee43f6e926e1d9f7a488b451))
        - add mc_rgm_release_periph func ([f2ab124](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f2ab12441fdcc8d98ee7da3f406d0f1045ca1b59))
        - init the generic timer for BL2 ([e790512](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e7905128404a0edcc8eb9f7300daf253eddae607))
        - initialize the IO buffer ([dbf400d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dbf400d09676c37d4e425b8844bf0633f001018f))
        - initialize the uSDHC driver ([d82c211](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d82c211d684271d57fcefeb43c4afc72c06f1bfd))
        - set the system counter rate ([3c60749](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3c60749b0665202f628e9eb8a999ec363baa3bbd))

  - **QEMU**

    - add support for FEAT_GCS ([d9872d7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d9872d758faeab415fe845c689aa02232ed1396d))
    - add support for FEAT_SxPxE ([139ddfb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/139ddfb50397064a65901c852f4924d217f7ece3))
    - add support for FEAT_TCR2 and FEAT_SCTLR2 ([436cc70](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/436cc702799adb6ac2ac5fb15d2c4e67bb8cdbd2))
    - skip paged image info ([803560d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/803560ded6da67c6518ff24d9f4c91a3e8a86811))

    - **SBSA**

      - support s-el2 and s-el1 spmc ([cda0487](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cda0487a85a120f13b29dcde0f8a99792b7ad364))

  - **QTI**

    - add BL32 support ([48897ba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/48897bad6e7d63d065d25e1dcb155946e0089d15))
    - add TF-A BL2 common platform framework ([c48d0ae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c48d0aef377385a3afd3fa41610c90dcc51d2d88))
    - introduce basic XPU driver ([292ffc0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/292ffc06ef98aa89c570f8739a8d99d9a45c8fe2))

    - **Kodiak**

      - add support for RB3Gen2 platform ([ac44b9c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ac44b9c7726e12d798a4cc5310da3710099a19b1))

  - **Renesas**

    - **R-Car**

      - add support for Renesas R-Car S4 / V4H / V4M ([b45b5ba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b45b5bacb95d0e2d4539a7869c1ccf90da041498))
      - deduplicate plat_crash_print_regs ([06f8eb5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/06f8eb57c0521ec933d8d434766b0db11351aa78))
      - deduplicate PWRC SRAM trampoline ([57e22e0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/57e22e078f8e4633c59c0e1d4d6cf227da7efa61))
      - deduplicate PWRC timer ([92196d4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/92196d4fdac7fc495a43162d3b634f0ad96af6f2))
      - deduplicate SCIF console_rcar_register ([9979a20](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9979a20af60abed97bf1c004cfb1607b3dbda05d))
      - deduplicate stack protector ([223d989](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/223d989ed990fedf58c8f605d5e4c924b022cd62))
      - replace static rcar_putc pointer with static register offsets ([b21216f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b21216f716e0ad975ed65e70735f82e05d55b5ca))
      - split common SCIF code ([3c6170b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3c6170b649a23d60ca4dcb9d256cc3c53f90cc70))

  - **Rockchip**

    - **RK3568**

      - support SCMI for clock/reset domain ([4e1ccc6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4e1ccc60ea47b889e704c7d7f0301ad986e40893))

  - **ST**

    - adapt .stm32 file creation for clang ([fa4acc2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fa4acc2a8435927d9da55f838820e21c81a136af))
    - adapt stm32 linker scripts for clang ([43560d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/43560d8e65bead4c8a2e3a2e81447d6c30688925))
    - add RNG minor version ([864466b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/864466beb0460537a648d11024e3db20c8901323))
    - introduce SoC family compilation switch ([701178d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/701178dc69151a96e959a0c44d294e92d63cbdca))
    - put stm32image tool in build directory ([0b4722c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0b4722cf261829596f82f92d62aced48c95ccd83))
    - remove unsupported option for clang ([44bf952](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44bf95231ecd576b37fcb2b3b6e4f34722eb3dee))
    - update stm32 linker scripts ([6778835](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/67788359e5fddd99c8f40ce7bd8c75ed1ce6bac4))
    - update stm32cubeprogrammer API ([ef39709](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef39709c04e74d4957ff3c3e6343210d1873ddc2))
    - use and override default MBedTLS config ([1dd3b65](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1dd3b6565da16dda3d4aa33a41b552816b7539f2))

    - **STM32MP1**

      - add stm32_get_uid_otp ([08252f9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/08252f9d172b87d37a8afadadd8c181dc14ce191))
      - enable MCE driver for STM32MP13 ([c7a457a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c7a457ab9cb09ab0d59d296851f88f57273d8217))
      - prepare DDR secure area encryption for STM32MP13 ([d06b375](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d06b375326e5d5eb90cc913bc4c1db1350f5c275))

    - **STM32MP2**

      - add HASH and RNG compilation ([27b4244](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/27b4244bbdb1c3c5c881b55c17057cd317dfc35a))
      - add RIF dt-binding defines ([7b4b208](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7b4b208ed84a150ba67f13b308c5981a3626b10d))
      - add RIFSC/RISAB protection for USB3DR ([ecad2c9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ecad2c912ba51f8ab774a3027424962d2e504116))
      - add some platform helpers ([2c831e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c831e4b116be6215290f678d35b1946160c099a))
      - add STM32MP_USB_PROGRAMMER support ([6d1366e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d1366e5bf8cd4255f765737972f24b204691617))
      - add support for minimal FIP with only DDR FW ([eb43024](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eb43024c455123726c85e2be029ca0b448f11d81))
      - generate FIP for DDR initialization ([aa63c23](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aa63c231da15d79420fc8d605bdadda5eed9d16f))
      - prepare DDR secure area encryption ([f2b9807](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f2b9807d2c26e2fd453e77809ebc09232ed7fbbd))

      - **STM32MP21**

        - add clock and reset bindings ([dcb00b1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dcb00b10ab70f08b3738d10f4691b1b3e98ab310))
        - add PWR registers file ([2ec3cec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2ec3cec569d2420370d5f9a14b40b435c450b53e))
        - add RCC registers file ([58cf812](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/58cf812a38e5cc5fce2d75ee48216d2e64cebf1e))

      - **STM32MP25**

        - add RCC register to display all IWDG flags ([e957c33](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e957c3374a7fd36664d40e79200150ca64f6fee9))

  - **Texas Instruments**

    - add mmu regions for am62l soc ([8853eba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8853eba6e1abbb7cd0d955adb37a0e5eefc0c73d))
    - add support for boot notification msg ([7d3c700](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d3c700fedb953aa9ea4e166bae69ef01ea7abaf))
    - am62lx init: boot notif and version msg ([987c9b0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/987c9b042c2c3b6c92c4401a4cbeb423be7920d4))
    - build generic timer ([a5cf0ba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a5cf0ba437b91b22a63a60b9b5c6bdb067413b69))
    - de-assert AINACTS at boot ([879fdd0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/879fdd07981cc8b529499cece1d672ab50274379))
    - enable power state validation for K3 SoCs ([7f5ff95](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f5ff95b39cc64ea70206fa58fd3e5aae0c33e7d))

  - **Xilinx**

    - deprecate PM_REQ_SUSPEND EEMI API ([93434bd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/93434bdd785f31900aedcec85aaff5fabed4539f))
    - use common SECURE/NON_SECURE macro ([4fd510e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4fd510e0b69530b0a66b91f9ba35b942fa60e75f))

    - **Versal**

      - add hooks for mmap and early setup ([72b9f52](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/72b9f52d373b2dc79fe29f771e8c37aaee6d9d31))
      - add support to clear PM specific data ([1d4372c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d4372c4e8ffbfceceb3629940e170e06bb5164c))

    - **ZynqMP**

      - mark IPI calls secure/non-secure ([8ce93ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8ce93ec9531c4b7da2d25f2e3957ca13204a3ac1))

- **Bootloader Images**

  - **BL31**

    - add placeholder activator implementation for LFA ([f647f56](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f647f56123002e14eaf3d2c1b127431ee376a141))

- **Services**

  - **FF-A**

    - bump SPMD FF-A version ([dd87b73](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dd87b735561d5d8070978c62984c38f531a6641d))

  - **RME**

    - **RMMD**

      - add per-CPU activation token ([89d979c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/89d979ce9cddd4396a1da9c3ec0dee54bee52c30))
      - add RMM_RESERVE_MEMORY SMC handler ([745c129](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/745c129a89da77cab5af28d633610a24fc466544))
      - add warm reset helpers for primary and secondary CPUs ([22bbb59](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/22bbb59f6cbf9bbf51ab778ce2a01cc419df5423))

    - **RMM**

      - add placeholder activator callbacks for LFA ([d837ccd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d837ccd8fd185196c9aa08d218339357522de89d))

  - **SPM**

    - update MM_SP_MEMORY_ATTRIBUTES_GET_AARCH64 interface ([127bd5b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/127bd5b1fe9a9611a6762b8a6ee64972de448c4e))

    - **EL3 SPMC**

      - allow physical partitions to have multiple UUIDs ([0322d7a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0322d7af50439fc7144676852aa3a70410c36fc3))
      - deliver TPM event log via hob list ([4e5247c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4e5247c1515f3f58269d7767f33113716221879e))
      - enable FFA_MEM_RETRIEVE_MEM_REQ from the hypervisor ([7d34c9b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d34c9bb620344aebeb6d3cbc457ff3700af0ab5))
      - get sp_manifest via xferlist ([aae2370](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aae2370cb387fcf4783d0170896d73a23a66a060))
      - parse and report VM availability messages ([ce299f9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce299f9594ea589f44fb8f3a81a611c263395d4f))
      - update FF-A version checks ([3f1c63d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3f1c63dd9ba821307fafccb95511b11f80971947))
      - update FFA_MEM_PERM_GET interface ([9bfe78c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9bfe78c2ef792c666954c315e647737369636ab1))

    - **SPMD**

      - add FFA_NS_RES_INFO_GET ABI ([08f9ba5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/08f9ba5b8e0d38a28154d9fef37a8749a523a528))
      - get spmc manifest from xferlist ([55fd56d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/55fd56d7abece6c14607f03899fe94ac47b0fcae))
      - support for FFA_ABORT invocation from SWd ([b3dcd50](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b3dcd505ae9e25ae5b5c2e435143915b4097f3e4))

  - **DRTM**

    - remove todo that is implemented ([9232d3f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9232d3f2c38a9bbedf72168d8a0d268d88f25bcb))

  - **Live Firmware Activation**

    - add activation handler interface for component activation ([9e03218](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9e032181ae28cd83b1aca1a802133706a930890b))
    - add helper to check LFA prime completion status ([ce78f3c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce78f3cda801acae7877107da99facb94216fad8))
    - add initial implementation for LFA_ACTIVATE ([07de22d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07de22d29e58edcaabfcfe313771ea1062a75bc6))
    - add initial implementation for LFA_PRIME ([67fa182](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/67fa182fc4c460ebf6d7abe3366f9ecbbe393dc6))
    - add LFA holding pen logic ([ff7daec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff7daec6f4bd151a1787c12db546509db49597df))
    - add platform hook for activation notification ([5084b7f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5084b7f11d6bc4921dd106e3fae6930d252c6792))
    - allow LFA_PRIME from one CPU at a time ([c8e0821](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c8e08212b85169a00b9b914b69d87529e14bdb6b))
    - create LFA SMC handler template ([cf48f49](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cf48f49fd352b3c68635677c1564a449d3745930))
    - enable LFA of RMM ([f024c57](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f024c573042f7ad29c41e68d261c859b91c6dcde))
    - implement LFA_CANCEL SMC ([3f7b286](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3f7b28626636d45bb0e695db9a5d4bbe3af82329))
    - implement LFA_GET_INFO SMC and integrate LFA build ([b9dee50](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b9dee50c5419d351effa60fb995a6dd1724a4031))
    - implement LFA_GET_INVENTORY SMC ([06a6f29](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/06a6f2968df7ac5a1016afe60a22f2f7d03f6c9b))
    - introduce support for call_again for LFA_PRIME ([59b826c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/59b826ce6a254cb24daa00e4bd3b1374b50945db))

  - **Secure Payload Dispatcher**

    - **OP-TEE**

      - check paged image size ([c9e91b2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c9e91b2430188a78bbb3bfe0f457a49425e38bdd))

- **Libraries**

  - add mmio read with timeout ([7a1970f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7a1970f4ad497c9f83595fc321f58aa58549e369))

  - **CPU Support**

    - add pabandon support to Nevis ([6588ce0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6588ce0a03587fc35d3903cdff54922a82742231))
    - add pabandon support to the Alto cpu ([2e764df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e764df05a8a0988f92d303f6bdabaf0ee68ce04))
    - add support for caddo cpu ([656500f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/656500f9a48c7866ac3673faead75c7bdc441ab3))
    - add support for Canyon CPU ([5fc2895](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5fc2895cd486b708a8dd131b77709aa90767e226))
    - add support for Dionysus cpu library ([2cdc34c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2cdc34c5580e4b32bcc768302c2c4601aedca651))
    - add support for Neoverse-N2 prefetcher ([7538438](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/75384389b9539b5d66aadecb10dae1c70e97e383))
    - add support for venom cpu ([d4c50e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d4c50e77bf9d1b28da5e513325b07d3fee78119f))
    - add support for veymont cpu ([51247cc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/51247ccbaef059d0d5a6da1973c1b05873a83f1b))
    - enable Maximum Power Mitigation Mechanism ([037c7a8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/037c7a810f30e4d306cd1dbe7006d83842c72e1f))
    - fix external LLC presence bit in Neoverse N3 ([ff90ce4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff90ce4151bdaa9641a3136e0b83122788487f5d))
    - update cpu_check_csv2 check ([2b43216](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2b43216593f10ca55ba64d4990d1079083448835))
    - implement workaround for DSU-120 erratum 2900952 ([efc945f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/efc945f1a8c19a678737dd07734e57375f0f830c))

  - **EL3 Runtime**

    - add flags argument to handle_sysreg_trap ([88655be](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/88655be93c148f9849d8fd02d7edaf0cb7d04058))

  - **Per-cpu**

    - add documentation for per-cpu framework ([b92586a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b92586abe3ce3b442542a1cce0bfe2923a72a3cf))
    - integrate per-cpu framework into BL31/BL32 ([98859b9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/98859b9911d6faa02907de476b9e7235e4602041))
    - introduce framework accessors/definers ([962958d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/962958d309f6c0b7c1490ab1bec67804c58589f5))
    - introduce linker changes for NUMA aware per-cpu framework ([7256cf0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7256cf0ab7d539b134798b699aff4e2753d3f0cf))
    - migrate amu_ctx to per-cpu framework ([0f90f37](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0f90f374332a408671f4ffe6a5b3092d5edfffab))
    - migrate psci_cpu_pd_nodes to per-cpu framework ([9f407e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9f407e448be8ca93d2e5ee47ccec99ef4ed64946))
    - migrate psci_ns_context to per-cpu framework ([6d2d846](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d2d846f4c2cc65b80284858a24c0dbf2ba738d9))
    - migrate rmm_context to per-cpu framework ([f708e9d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f708e9ddea03684fd127f1e398ab2b25e870b63f))
    - migrate spm_core_context to per-cpu framework ([f5dca2a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f5dca2a95b09e9049ae95282681922c0705aac5f))

  - **PSCI**

    - add API to get number of CPUs currently in ON state ([a7be2a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a7be2a57b6999d2035655d75b9938770e8ecc5a8))
    - check that CPUs handled a pabandon ([461b62b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/461b62b548cf57e97b22081d889ebec1b8c71a82))
    - make pabandon support generic ([04c39e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/04c39e46e0b237c7e1ccfdd5428d7ab675fd6a92))
    - optimise clock init on a pabandon ([fd914fc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fd914fc823573654017be5facb571eee0611a619))

  - **SMCCC**

    - add SoC name support to SMCCC_ARCH_SOC_ID ([cb4ee3e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cb4ee3e4c203cc8d5415bf1e3db529885ec900eb))
    - clear scr_el3.nse in smccc_arch_feature_availability ([847c111](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/847c1115cfc78154c02b9452e775a0b277a09a00))
    - enable support for FEAT_MEC ([a357d15](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a357d1577db20bec6113de34fc4d888b2c68b5a3))
    - introduce TPM Start SMC ([96546b5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/96546b5cc3562a455ca65f0d1d3640805eeb4a59))
    - introduce a new vendor_el3 service for ACS SMC handler ([f69f551](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f69f551269f1d126877889a1cab27cc1692316ea))

  - **C Standard Library**

    - add __2snprintf alias for armclang-specific name mangling ([ab0255a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ab0255a70fa451bd3fa1a68aaf459d020e01a18f))

  - **Locks**

    - add non-blocking spinlock_try() API ([86822f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/86822f24857915bc8bc21ac0ffb86be69d5c966d))

  - **Firmware Handoff**

    - add firmware handoff threat model ([a5e9623](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a5e9623edf1a76bf4aabd8e21c9dde90d0b35c65))
    - support libtl submodule builds ([06f3c70](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/06f3c7058c42a9f1a9f7df75ea2de71a000855e8))

- **Drivers**

  - **Authentication**

    - **mbedTLS**

      - update mbedtls to version 3.6.4 ([24d6ed9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24d6ed9fb4d0cd54e80d310923d85d28a5342938))
      - update mbedtls to version 3.6.5 ([5affb6a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5affb6a7761c298f221f260472f672c3b5bdd394))

  - **FWU**

    - add platform hook for shared NV ctr ([75b5be9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/75b5be98b516feade0dddac99946d40ff6e63ef0))
    - create flag for BL2 separation ([5ce4ee1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5ce4ee1a3ec2c844b24cab4fb10e3b961d2d8964))
    - documentation for BL2 separation ([cb0a4e9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cb0a4e9d55d49e3679bc3e3db0105ba05bb1c5b7))
    - separate bl2 image from rest of the FIP ([d57362b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d57362bd92c2e5c8a1222fd763e24163c1234938))

  - **MMC**

    - add define for no response ([46a1167](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/46a1167010fbccd0b9cdcdd1b8a7714509a5663b))

  - **GUID Partition Tables Support**

    - platform hook to log corrupted GPT ([3537dad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3537dad5ed65c4aad334a4a77c3f03b2e2a7f503))

  - **SCMI**

    - add base protocol agent API ([6445c83](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6445c834836c5a812fb106c25a4a532549b1cd4d))
    - add i.MX9 SCMI vendor CPU protocol ([dbfe9df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dbfe9df89a21daa63e2d4cb3f7eb34e57b8ebbc0))
    - add support for discovering and changing parent clocks ([684952d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/684952d17275d49ddb0fcb1e3d5df020fb5eae0f))
    - support graceful system power set ([eb113bc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eb113bcb5d67420d80aa2b403ee63c7480152bc4))
    - update version to 3.0 ([e8a96bf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8a96bfab9f4a6611167a56cb81731360a1286df))

  - **Arm**

    - **GIC**

      - change gic_cpuif_enable/disable to weak ([34a94fa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/34a94fa558af52bf4a829d6f1572b7058b785374))
      - make IRQ groups optional ([5f65250](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f652507e8037f9901c4f32d833f194aca6d9cfa))

      - **GICv5**

        - add a barebones GICv5 driver ([13b6281](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/13b62814abb822e1841fd9430fa96da91d63b776))
        - add support for building with gicv5 ([8cef63d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8cef63d6c7184fe1eebc354716e4b3910d385f9b))
        - assign interrupt sources to appropriate security states ([82b228b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/82b228ba638cb027cbedfbd4835587b6c465fedc))
        - initialise the IRS ([dfb37a2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dfb37a2df579ccf0e92c97515452b13ce2c9fba8))
        - initialise the IWB ([7179920](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/71799209def08f4afdc49555246268cf1300a2d7))
        - probe components ([4db6bf9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4db6bf9faa5f09747c157abee09a6946cc328d3a))

      - **GICv3**

        - add GIC-720AE model id ([0d65d5a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0d65d5a4d1b3ac0d05288c2822f2482e8b49e20c))

  - **NXP**

    - **FLEXSPI**

      - add 128Mbytes flash info ([7b370c1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7b370c1969531d707f5a1760fc688c335f6fa1c8))

    - **uSDHC**

      - add data buffer ([b61379f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b61379fb692205f5066e5d3b7fbbb3bdc438cae8))
      - add dynamic mapping ([cdf002d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cdf002debe0c0885baf691be928aa2c464fb7141))
      - add timeout to blocking loops ([01d24d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/01d24d69efe61b80c4cb32e0c37ade858f654530))
      - flush and invalidate buffers ([7e2a434](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e2a434765543b10e322cbd096d1719788fa4569))

  - **ST**

    - add Memory Cipher Engine driver ([6d79740](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d79740261e136c5ba8fae65d09ab2d0806a2c4c))
    - add RIFSC driver ([8934c7b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8934c7b0d195137cb11f3f4afe1bbe5bfd48c03d))
    - add RISAF driver ([399cfdd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/399cfdd45cb3ba129dee042721be9326ccb7c5dc))

    - **Clock**

      - add STM32MP21 and STM32MP23 RCC variants ([088238a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/088238ad79868b7ab42c69a53f1a4e09772fc004))
      - rename RCC_USBTCCFGR register into RCC_UCPDCFGR ([d3e47fb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d3e47fb7fcfc4c0082e95d6f908787d025a97c9d))

    - **Crypto**

      - add multi instance and error management in RNG driver ([02b770a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/02b770ae7de0a99795cade9204125d3b1cc638a1))

    - **USB**

      - add USB DWC3 driver ([867cd15](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/867cd155f5bc062a480a7dbcadcf8adc9ca1d66a))

- **Miscellaneous**

  - **DT Bindings**

    - add MCE DT bindings for STM32MP13 ([5f90891](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f908916ec5f0ccc98828c3c5e2ddb1d84e6e4ed))

  - **FDTs**

    - **ST**

      - **STM32MP1**

        - add MCE support for STM32MP13 DK board ([0bea409](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0bea409a76ffe95ef0ae1a92ef792dbd753adb67))

      - **STM32MP2**

        - add firewall nodes in fw-config ([86d91be](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/86d91bed9d68cc067ed8d58a77ce705d8a15fb59))
        - add memory firewall node ([bb3c45d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb3c45db3c70d01e7fffd4bccceb00c778aa98c9))

        - **STM32MP25**

          - add RNG node ([c434b76](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c434b76510118b179ab11189ff938632160e3ea4))
          - enable rng nodes for ST boards ([9598477](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/95984773f4b5657f5f89332613d84b6d00bd6c3d))
	  - add RISAF nodes for STM32MP25 ([8f783a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8f783a5eedd7e51ea7a1469d9d5d312a8825ad66))

- **Documentation**

  - platform hook for whether NV ctr is shared ([95ed23a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/95ed23a1c289de98aeebee702ab593137663f53c))
  - platform hook to log GPT corruption ([2605cde](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2605cde2e64b03a6644d11556c92e47f5a067724))
  - update context management's threat model ([e77cd73](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e77cd73f944acb67f66c3d30a508a4adf35cbb79))
  - add Architectural Feature Support reference ([5548ab9b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5548ab9b3f824918f2823cba1eab5cbb7448d6d3))

- **Build System**

  - add helpers for managing compiler launchers ([5c24052](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5c24052a35d5fdff341f3ea46f49facd0ac5bf11))
  - add shell fragment joining utility (`shell-join`) ([a75ab9a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a75ab9a7e8a3deafa47284784e403eaed6dd7fc3))
  - add shell fragment length utility (`shell-words`) ([8165d82](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8165d82605460484b8b29f7bfbc0f22db16c610b))
  - add shell fragment slicing utility (`shell-slice`) ([9a782d4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9a782d40161414b7566a2dc6c374b5ce2442c597))
  - add shell program reference utility (`shell-program`) ([a72154c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a72154ce368823f2a49a314e11c678d501bde275))
  - add shell quoting utility (`shell-quote`) ([10cb835](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10cb835fcab2cf4a909cf829de6671e51bc1bab7))
  - add shell word extraction utility (`shell-word`) ([5980fa7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5980fa7c27122a690df02ae821407ef084f32110))
  - add shell word map utility (`shell-map`) ([37cd618](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/37cd6184eda771edd40ee4aeda2385fdcb1d2358))
  - add temporary variable binding utility (`with`) ([6eb35c6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6eb35c602d8fd0d841d202a682eddce5926911f9))
  - add utilities for modifying includes ([0fcee05](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0fcee05f13a7fe235b4c07efa96ce4841627d522))
  - allow full LTO builds with clang ([ddc918b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ddc918b1f30926e8a41856ec2cfc22733c4e850d))
  - create define macro to be used by BL ([116d2c0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/116d2c09446c8630d152bd3bbd70032d9d79a0fb))
  - pass cflags to the linker when LTO is enabled ([885ed9e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/885ed9e05eace7f128629380bd0b1a72bb6f6b2c))
  - put crttool in the build directory ([f4595e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f4595e6e4f3659ee6deec9490a243a1b84333f85))
  - put enctool in the build directory ([96fbe88](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/96fbe884492aa68a9500a6f5f9ac0600414260bc))
  - put fiptool in the build directory ([cbd6cec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cbd6cec3c35f22733034fbdd4801efe20d26ba9a))
  - set CRYPTO_SUPPORT macro per BL ([6e2fe62](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e2fe623fbbf299b4584bc202520f60955633077))
  - setting CRYPTO_LIB via CRYPTO_SUPPORT ([cd92fb2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd92fb243c95de0b7faf0ee3afd3a1bd7952c524))
  - update `shell-map` to also pass indices ([1d5ae1e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d5ae1e5e86722709ed6dcf0bba5f17df3348719))
  - use clang as a linker ([6c2e5bf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c2e5bf68955cdcce18fbd32697692e352e3c34c))
  - use LDLIBS instead of LDFLAGS for library search paths ([d833129](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d833129ad19eb9cfeaf3049aee39b0b3b7fb9bf0))

- **Tools**

  - **Memory Mapping Tool**

    - add summary command to memory map script ([a98d466](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a98d4661539eed5098acd28f827960195de64140))

### Resolved Issues

- **Architecture**

  - **CPU feature like FEAT_XXXX / ID register handling in general**

    - add ras files to the build from a common location ([25fe31b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/25fe31b2e7aa141e21b9906259bc4bcfaf77ae52))
    - configure CPTR_EL2.ZEN and CPTR_EL2.TZ to match Linux ([7f471c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f471c593d33a0c88e3db1e8852ec71d0a8d1d1b))
    - do feature detection before feature enablement ([0f57a38](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0f57a3889c423d02c0e0645701f0e1856ead0f20))
    - don't overwrite PAuth keys with an erroneous cache clean ([072e8ae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/072e8aeb2b37970f90f682cbbfc44b62c0a04557))
    - extend FEAT_EBEP handling to delegate PMU control to EL2 ([714a1a9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/714a1a93d10637ed00e087585c6aa625b6c254f8))
    - remove unused FEAT_NV feature functions ([3c0ebab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3c0ebab538e22b565fdccb718f8b9b5a753b6718))
    - update FEAT_PAUTH's feat detect line to tri-state ([e293fca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e293fcadee24a1228ab9dbf6f29a57c29304272c))
    - use of additional breakpoints ([482fbf8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/482fbf81bdfbccb62757737cadf7a4dc182b41c4))

  - **DynamIQ Shared Unit (DSU)**

    - dsu config for all cores in hot reset ([98863b1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/98863b1ed0ebd667c8e352b847224e1128f47027))
    - fix illegal address Coverity finding ([70516ab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/70516ab6f84a51cb153416565f3d4f2e583760db))

- **Platforms**

  - add essential bool type ([d83e1f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d83e1f051fceabad2710a400124ba889ecea1784))
  - declare unused parameters as void ([ee14e1a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ee14e1aec25b271a6a778979f18e5ef841b34019))
  - declare unused parameters as void ([877279d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/877279dea678cfc92b04547350c80e988cb60c4b))
  - fix misra violation 10.1 ([5d09adb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d09adbe0e882d40bf58c53749f60db45e273ce2))
  - typedef operands to match data type ([0523d3d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0523d3dc42584b7f7e413529f315367d90327cfa))
  - replace stray BL2_AT_EL3 with RESET_TO_BL2 ([f856626](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f856626bf6d52ec4275dffe7ed1650047a2dc214))

  - **Allwinner**

    - avoid crash when running without DTB ([69b4a59](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/69b4a591142331d56704d1799700154cceca60a1))
    - fix variable may be used uninitialized error ([ce750f1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce750f168fa4108634e7c4de99face2a2717d3db))

  - **AMD**

    - **Versal Gen 2**

      - align comment about invalid console selection ([bf51768](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf517685dd380b6a1e9b8aebeb584f970caff9b7))
      - align IOU_SCNTR base address macro name with other platforms ([18a77ba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18a77ba73be466df7a53e648aeb8277649cbd4e2))
      - enable graceful system shutdown ([08ae97c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/08ae97c19de536c1cca136aac9bf9d6d1e1d3e57))
      - fix offsets for apu pcil ([02210f6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/02210f637d6233a74834a6cfc4076b583eb8ab53))
      - handle debugfs specific APIs before EEMI handler ([633cf6b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/633cf6b7aae59729187d5a35ff851e04c26e1aff))
      - initialize counter-timer frequency register ([f08dcf5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f08dcf5ebf2fccee954de193633c5f99bf65b912))
      - move plat_core_pos_by_mpidr to asm ([d2244f3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d2244f32f7773d7a7c8031fd4a95fbd3fbad7917))
      - remove extraneous parentheses ([bb2b022](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb2b0227c6b9996d9bc5a91cec5cc0a4a40c3e2c))
      - remove PM_ABORT_SUSPEND API implementation ([7739450](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7739450fba7ff1e1ed2c5f830b1779e15021374d))
      - runtime console in debug mode ([b3555f1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b3555f12c4457fb7933f3a9bfcfc4987be3aa55f))
      - use common function to get system counter frequency ([f2ae203](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f2ae203a230e618acf2123e77d96474b8f51deee))
      - use PM_STATE_CPU_OFF for core power down in SELF_SUSPEND ([9cfc723](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9cfc7235a5e1a21cd0e36ea5d430c24898b8bdda))
      - validate Node ID in PM callback events ([244f9fb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/244f9fb94fb5bfb1bc1bf08ff894c06ad9f567ff))

  - **Arm**

    - derive RMM bank size from payload ([dbda614](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dbda614c934614dac5ec7e43be3a0450eed7a129))
    - don't override the gic redistributor frames ([1d59d68](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d59d686bf8f99bd3d94f6aebc0867449c24c289))
    - increase reserved DRAM1 mem for NS images ([7a171ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7a171adea59c631040d5b0bc203cec1ef37be063))
    - move lfa componet header to common and fix the helper ([716deb8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/716deb86fd28d452b64df3c49be694ac832c8526))
    - re-enable console by default in BL31 ([de46405](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/de4640579c5318ba08ec57531ef87d63f5574557))
    - typecast operands to match data type ([8a13157](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8a13157151a61c5015ccfc990bf660629748b16e))
    - typecast operands to match data type ([3d7caf4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3d7caf476b851b3e9d9ee546626a80597c2b3495))

    - **Common**

      - add missing curly braces ([618e37c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/618e37c3decc9ef6d2b57bc850aa43b0ccf3641f))
      - add missing curly braces ([3ba36ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3ba36ea07ca22c748b5adcf5d9bff00e752681d7))
      - error out if image load size is zero ([854b4dd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/854b4dd92ce0546bb97562c613679d27844bdb60))
      - initialize the variables ([929a290](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/929a290e26c3c0600eef80dad43ead370a5bd9d7))
      - rename exit label ([1db51f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1db51f0ae32bd65dc39e001a7228abe5d64b69e2))
      - typecast operands to match data type ([f3ecd83](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f3ecd836afa13c560d63b49c0f05e913011a20b2))

    - **CSS**

      - don't require the GICC frame to be defined on GICv3 ([0204bc8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0204bc8c9e9af563011beaa645faf595f260e1e8))

    - **FVP**

      - avoid stack usage in check_cpupwrctrl_el1_is_available() ([36ceead](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/36ceead86b06a776fb13ca1f0f4d26979f658662))
      - do not unregister the console on system suspend ([f185a54](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f185a542db1b58924db40de430d94f80e2a21c1c))
      - fix coverity issue unsigned_compare ([4824e25](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4824e250595881be7a59f8080818c59d30c3b10a))
      - give fvp_ras.c better dependencies ([7e87f49](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e87f4940529fe98711f104811c80f18bf079c6d))
      - initialise the event log's size to avoid using gibberish values ([b199ca1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b199ca1a9b1f12505c48d3e3bd35c418fdbb507e))
      - pass an `interrupt-names` property for the timer when using GICv5 ([8c37540](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8c37540524a16dc564b6758c6c62d51d3a9487f3))
      - skip SP discovery through FFA_PARTITION_INFO_GET_REGS ([0fbcef0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0fbcef00857765a9aac3ee47c1d1243b5dbd218b))
      - use global option for setting PLAT_ARM_MAX_BL2_SIZE ([27bff0b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/27bff0b955a73ced26b12fa200c5dbbee30b8e84))
      - workaround when PCIe 2 region is not present in DTB ([2cbea16](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2cbea163eb42f9c6d5754c3a10ede4e37c921a47))

    - **Morello**

      - fix the incorrect order of gpu interrupts in dt ([45a567a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/45a567ac331b55291930dfdfd3b86822f56da493))

    - **Neoverse-RD**

      - add console initialisation to BL31 ([47fca89](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47fca89ddda843c97ac4f93babb44f2af9e111c1))

    - **TC**

      - fix c1_pro power down abandon ([2e1dff2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e1dff2da3aa88133992f6e0ccbf89ef88193693))
      - force specifying TARGET_PLATFORM ([81fece2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/81fece2109470318000e538adcdc717e2aa84560))

    - **Automotive RD**

      - **RD-Aspen**

        - enable CPU feature runtime checking ([3a324c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3a324c26e9f36ecc77ac816ea6b926c050549aab))
        - fix timer bus cells & fix ranges ([6fb6bee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6fb6bee1dfd7fd896c44cc21b02b4ef3aad3bbd0))

  - **Intel**

    - add 5us delay before Linux reconfig to avoid HNOC hang ([afae10f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/afae10f87382779facc96995445dd8d49c15f3c0))
    - add memory alignment at cadence SD/eMMC driver's descriptor ([2fcb37d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2fcb37db8384bfb7a08edcd024f5ca4e245dc2df))
    - add missing cache flush operation for hmac ([1363096](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/13630966de0ac4cfdbec5fa6abe9033bcb2284be))
    - configure SCR1 for 32/16 non-secure SMRs and context banks ([a042bb3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a042bb3df063a501c1be27afb57ef82cf7b4a0c2))
    - configure usb3 system manager reg in TFA ([00c1b8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/00c1b8c721924540f33576c0c90278e9dd28d652))
    - correct condition to process unused entries ([ef80b7a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef80b7a294cdfd85052a644e8d3a34000de2674b))
    - fix eMMC driver issues in boot flow on agilex5 ([38636fe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/38636fea416bf90b651a6623f94d2637282f9444))
    - fix iossm driver timeout in agilex5 ([5b173df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b173df396306f4243ecd6a369bbe9849349b9b8))
    - fix SDMMC driver when sdmclk running at 200MHz ([5482237](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/548223725c256139f9ce6b192b245c67972f8257))
    - fix socfpga_psci for cpu on off function ([8f7575e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8f7575efcb3df7eeb21e92677c3a7be3fe3c36df))
    - fix variable may be used uninitialized error ([cfde117](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cfde117042d8c64263c2819e6e37adac2ebf2587))
    - flush the mailbox response buffer in SiPSVC V3 ([cb3ceb5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cb3ceb53fbeb63d3e4dc7a74fd737a9f35102493))
    - iossm v2 enhancement refactor ([f1b1fae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1b1fae99902e1f30d49d8c7390ed80f48e89e15))
    - match declaration with definition ([c90c0be](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c90c0bedb3ebd960f06e50808ce17b1ac716db75))
    - remove wfi polling when performing cpu on ([53791e8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/53791e82a9c98cea08b0e9239d0c12a380859133))
    - select the DFI interface based on the hand-off data ([6993598](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6993598f0a52f21a945cf16ba7c8a1bf712379c3))
    - set BIT2 of system manager MPFE Interface Select ([130e88a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/130e88aad973e579615ac79fe35441f4cee90697))
    - solve agilex warm reset issue ([2c03c2c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c03c2c05febb27d8d3007c7636ab474ddf41e05))
    - solve s10 warm reset issue ([7e94cc1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e94cc10a9dce76deef258811b07a345c9087a61))
    - support DDR In-line and Out-of-Band ECC handling ([92d2277](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/92d22776fc15e8301e1c3f2689a1f04143e5db30))
    - support generic mailbox command in SiPSVC V3 ([cbb62e0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cbb62e011beb86b59a90b5d4f27d5b1878a6e5a1))
    - support SMC 64bit return args in SiPSVC V3 ([cdab401](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cdab401839b0af9a8cdd265ed22c1145a0d37e0b))
    - undo setting USB 3.1 reset pulse bit in BL2 ([e655389](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e655389fdc9583944d907240830de3734fe9bcd5))
    - unexpected DDR reset type value observed on Agilex5 ([8bdfbaf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8bdfbaf49803e973a55788ef2e03f8847d7eaff1))
    - update FCS AES method for GCM block modes ([1e1dbad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1e1dbad08c7644c91db2a235a1969b2f1c96be4f))
    - update generic mailbox command filter method ([0934946](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0934946ec05aee43e851d5ba59f5aebd77daf401))
    - update initialization to prevent warnings message ([da1e000](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/da1e00085282823ca5e645e815e27ee4bbe29924))
    - update nand driver to enable Linux OS boot ([6f7f8b1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6f7f8b18e9641d34c5b02b5f5e050d6e0fae12fc))
    - update TFA to patch for Linux 6.12 rebase warning message ([6e6efe8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e6efe8c89e86a2c01604b89c146fe822b7e192c))
    - update the AES GCM/GCM_GHASH modes return data size ([8e47685](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8e476852f1ac5eaaed09141409197f6f65f4c1c8))
    - update the RSU version logic read ([d7286ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d7286ade76a9a5f81b4242f6400b75e5f11fb9b1))
    - verify data size in AES GCM and GCM-GHASH modes ([34f092a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/34f092a1a680a2cbd92e696d560cc4b1bb28f0e1))

  - **MediaTek**

    - fix mtcmos build failure ([68514bd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68514bd98c903752e58a3028cf97c044c2c4e814))

    - **MT8188**

      - fix variable may be used uninitialized error ([3599f40](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3599f40a26bf8013009e6db09411d325cec24469))

    - **MT8189**

      - fix mt_spm_pmic coverity ([7c3bcb6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7c3bcb60b530f7238762478c8e24814dd6b8958b))
      - remove unused SPM definitions and files ([9fc5866](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9fc5866a7c7c6996e2215090a4e98307bb5c3797))

    - **MT8195**

      - fix variable may be used uninitialized error ([8914774](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8914774ee700e9b958c55f943a4d0e2cf755fb4b))

    - **MT8196**

      - duplicated ETIMEDOUT definition ([0e039ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0e039eaf53868e38531394871e25d00f14440ce8))

  - **NXP**

    - driver crypto caam ([cacde83](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cacde83bed99c3c18be7bfc4a392b324e5c949f8))
    - tbbr: adds snvs_init ([ce9b87e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce9b87e74ded8f48caf2591b866c25cd371fd62c))

    - **i.MX**

      - add static attribute for platform specific gic struct ([c7294df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c7294df95d6796295bb433f9b1ace603d12f3c4a))
      - check if the cluster id is valid ([cbe42e1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cbe42e115a7f393748fc0d1c9b6c9d2c2a5aeb77))
      - match function parameters to declaration ([82987cb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/82987cbd62b45c6be6934e606f93eddc4be850ab))

      - **i.MX 8M**

        - add RDC entries for all missing UARTs ([a2c6e11](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a2c6e11dbc8e9991e0cf44be713140fec915f470))
        - don't reconfigure default region0 ([9bf1480](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9bf148071aad597e7fe7d1080c00aeb35b67a3dd))

        - **i.MX 8M Plus**

          - assign wdog1 to domain0 only ([8845f8b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8845f8b2886391fcb7662f013fd8161f72f396b8))
          - remove ERR050463 VPUMIX workaround ([b1ed68d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b1ed68d9ad2d9878c03fc2f3c06c97d2b0bc6f5b))

      - **i.MX 8ULP**

        - fix variable may be used uninitialized error ([e551dbd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e551dbd2430d8060a90995b682b3d5c7c18f0524))

    - **Layerscape**

      - **LX2**

        - **LX216**

          - **LX2160**

            - add DDRC missing DIMMs ([25b410b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/25b410bbe7c873673bed072d2b1fceb8d27cf429))
            - set snoop-delayed exclusive handling on A72 cores ([5acf82b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5acf82b2ea7b47b38571d559117c42ea6ea8e0db))
	    - put cert_create_tbbr.mk in the standard location ([7c3e9a0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7c3e9a0ca470eb53b9fab66ba103b3fa5a93ba07))

  - **QEMU**

    - add reason parameter to MEC update ([9c6e060](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9c6e060e512d5a2f4d5e9de1e2dc94df587c77fc))
    - fix variable may be used uninitialized error ([db0d535](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/db0d5350af0354c5b63ba2f06236df4b7c27fa65))

  - **QTI**

    - allow secure r/w to the EUD enable register ([bdd1932](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bdd1932c463f57fb066325b7b8392b97c06b69a1))
    - fix build without coreboot ([e76c1b9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e76c1b902a3dbdbcdd252214436bf94115089096))
    - fix config PLAT_XLAT_TABLES_DYNAMIC ([7f86b63](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f86b6350b843a375f4eda1c777fc9e56cdaef87))
    - uart platform base address for SC7180 ([5a12275](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5a1227592dd2d50884f815a664ea5e384e7a1ea5))

  - **Renesas**

    - **R-Car**

      - **R-Car 3**

        - add missing image_base/size assignment to BL33 image loading path ([e6b05fc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e6b05fcb13252aaa46f64fb18221ddb4cca59c40))
        - clear TCR_EL1 at the BL2 entry point ([fe87637](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fe87637a25168ea25b9ff869882a66c54287de5f))

      - **R-Car 4**

        - assure SCIF and HSCIF clock are always enabled ([13e1e76](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/13e1e761bc84bf42287aab0c8c2e8423ce4a38a5))
        - drop unused plat_pm_scmi ([3c57f96](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3c57f96aa1adb070e3ed25c230482c4e03fae345))

  - **ST**

    - add build directory for stm32image ([0d35f9e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0d35f9e54939b500ea90d27444ec7e3146b57d94))
    - allow several call of stm32cubeprog_uart_load ([ed88979](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ed8897918cc0ece8ee856baa56ef0a8d3a5cb974))
    - mark INCBIN-generated sections as SHF_ALLOC ([454441e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/454441e723c58621ec5506c2fa091208e3450a15))
    - replace down counter by a timeout upon dfu detach ([f79ca8d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f79ca8d8e8200688da749b37d1bc5dbf2ff06fb2))

    - **STM32MP2**

      - correct typo in definition header ([f53f260](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f53f260f7b789d53ce5941581e85f0a74faa4855))

  - **Texas Instruments**

    - remove validate_power_state definition ([80cfd5a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/80cfd5a6c767507b759bf453bf5660c2f8f8de54))
    - specify allowable rcv_addr in mailbox ([64e58ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/64e58ce3ee5a8fe273a4bfb2fd6c3b66bbc5a088))

  - **Xilinx**

    - add API to get powerdown request status ([c0719d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0719d210ca971f09ef5a77a4bb9a5b918313cf3))
    - add macro to extract node class from node ID ([2f8856f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2f8856faf89491d7371abbf91e517e98cabcb98f))
    - fix missing security flag in suspend path ([5cac1d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5cac1d853df2bd15e04c2198b80c2eb2a4182243))
    - incorrect usage of SECURE_FLAG for psci ([3e3cdf2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3e3cdf26f2c211e9e0dce2c7da9357b108d9ae05))
    - limit pm_feature_check deprecation warning to once per boot ([eaceb37](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eaceb3736ce18d8d91ad81f4cc4d3ea690057b50))
    - match function type as its declared ([1d94b27](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d94b27bcefb69542cf935fd948e524e1bcf1973))
    - remove PM_ABORT_SUSPEND API implementation ([ecee071](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ecee07197570b52c0048b3fb00e43997fa08f588))
    - resolve misra rule 10.1 violations ([c314a0b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c314a0b3fe24f4f17745ea90976662e9fe25ea7d))
    - resolve misra rule 12.2 violations ([f78c597](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f78c597041fcb6ee7bf1affc270b3178a9e9dba0))
    - resolve misra rule 16.3 violations ([a335cd9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a335cd91179a7733703482238e32f6db6c212186))
    - resolve misra rule 16.4 violations ([ea3ec86](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea3ec86552d3dcae2ef5b086d52d74164d02813d))
    - resolve misra rule 2.5 violations ([93db9e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/93db9e6161d5de43927d0d117c247608c36211e8))
    - resolve misra rule 4.5 violations ([2993166](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2993166d498ba8207d3935bfb1ae012b15125049))
    - resolve misra rule 4.6 violations ([6df7184](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6df7184e5ae089f54d19d67e276cb09d2e1f99c0))
    - resolve misra rule 8.13 violations ([cd60ab7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd60ab793e22897486dc0797eabcc10681892d4e))
    - resolve violations generated with IPI_CRC_CHECK enabled ([7d0eb0e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d0eb0e1e54494aeadf9af80121e7c82103b441c))
    - typecast operands to match data type ([87904ba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/87904ba8e60ff20b536c2dcf9b8f9cd2447d39d5))
    - validate Node ID in PM callback events ([a6dd46a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a6dd46ae06f12010347364def6f8d26e1b0442d9))

    - **DCC (Debug Communication Channel)**

      - typecast operands to match data type ([abfcd67](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/abfcd67ddfaddb3ac04a6afc15b445267e8608ce))

    - **Versal**

      - add external declaration ([33a264c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33a264cb5ad0b45149b87ac457caf1f9ba497439))
      - match function declaration with its definition ([5b51d4d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b51d4dedd23ff09cea9a03e61c6f4035ef5b20b))
      - match function type as its declared ([4e89096](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4e89096a5592ffed4d877b173ceab75b882dba49))
      - modify IPI4 and IPI5 trigger bit definitions ([c96f838](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c96f838acb4825c71e534af4f327dec61a7bfd79))
      - remove client-side implementation of PM_ABORT_SUSPEND ([86d9b35](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/86d9b35ff3499262af179c0d4bca7764628ceda5))
      - replace ull with ULL to fix misra violation ([f709265](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f7092652061b18b1597997a79c434e05094e8d40))
      - runtime console in debug mode ([0701792](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0701792f8a932f69e939b467a900d2cb640f5faf))
      - switch case has only one confirming clause ([325949f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/325949f19ec921d5a34d32f0b5109f01c2869f85))
      - typecast operands to match data type ([5d8831c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d8831c23b4daf9ef4ac67ba35d06889bd5f4dae))
      - typecast operands to match data type ([c1b0a52](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c1b0a52bcde47408274e7e560039801b2839ac93))
      - update integer const with suffix U ([1cbf6c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1cbf6c4ad57913752b13d605b1ec6d9384ba99de))

    - **Versal NET**

      - add fallback on handoff failure ([05d0cb4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/05d0cb4ff24570d727f8ab887ac4a81c32a87a2c))
      - fix coverity violation prevent buffer overrun ([3ef5820](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3ef5820c43e9feeff00b0e5722959b89b141610f))
      - move plat_core_pos_by_mpidr to asm ([9127041](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9127041a4ce1db729e6024f82229be1ee0520164))
      - remove client-side code of PM_ABORT_SUSPEND ([eeef4ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eeef4ac0d47a7569b19b848349ce941f57d349b9))
      - runtime console in debug mode ([18283e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18283e6d45d78acd49c162666eb6100bc52dedad))

    - **ZynqMP**

      - remove PM_ABORT_SUSPEND API implementation ([c069c8e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c069c8ef7173e43915e69d02cdddc7f11c36cabc))
      - runtime console in debug mode ([2546350](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/254635034a00cf6635177132d93941991f2287a4))
      - workaround issue around cntpct_el0 ([97ec082](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/97ec082a20bfdb81b93dc9b528f4d52ed27611cd))

- **Bootloader Images**

  - **BL1**

    - use per-world context correctly ([a873d26](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a873d26f2b226f00b81babbb452652bf08b2e5ee))

  - **BL2**

    - unify the BL2 EL3 and RME entrypoints ([04cf04c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/04cf04c72d403e0c057505882fac9002d39d4102))

  - **BL31**

    - add missing curly braces ([ec93223](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ec932236c55b3d90cb250d8a4c53d1dee8e57d54))
    - declare function as static ([d9712f9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d9712f9cae10fdeb8696ffcd3ca35d58666ea9dd))
    - remove incorrect asserts ([3e1d33d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3e1d33d6ab738fe624e856ddb083573833b0c411))
    - typecast operands to match data type ([2fa4dee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2fa4dee6e857f67350199da61dac61a5171f5862))

  - **BL32**

    - **TSP**

      - add missing include ([13c9952](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/13c99526aedd4e4aa1343ca89038403eee80a993))
      - keep the tsp D128 unaware, not the dispatcher ([b77c6aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b77c6aaca110e7c9b84df4570125ab280de58210))

- **Services**

  - declare unused parameters as void ([8cee7b2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8cee7b244b4826ab5facca38edb2467aed67b6e3))
  - typecast operands to match data type ([d16ad81](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d16ad813760ef62b5c887b2ce7b810710a4d1fc8))

  - **RME**

    - enable support for FEAT_D128 on Realm world ([a3effe0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a3effe0a35dee28be72d8914d2024804e4421ca6))
    - fix incorrect shift operation in rmmd ([c08285c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c08285cf771717860988e9fc98fa758b49620a83))

    - **TRP**

      - report argument errors before bailing out ([ab5db33](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ab5db3344a20e7ddde51edf8436a2cc6e737feb9))

    - **RMMD**

      - avoid race conditions in CPU finish ([5782406](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/57824063713bf6478382d445a8fa1361352b4b7c))
      - correct activation condition check ([5ba2ad3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5ba2ad35781762661cc3702773ee43a65bd756a0))

    - **RMM**

      - enable SMCCC_ARCH_FEATURE_AVAILABILITY for RMM ([cbba59c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cbba59c41a992c9134e668c3e5890747a1fe2114))

  - **SPM**

    - change the SMMUv3TestEngine being used ([015c76d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/015c76d8b442efcce109d573ab0021c392baf01b))

    - **EL3 SPMC**

      - do not check NS bit for fragments ([153eb4c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/153eb4c8eb914d6e3ecfe11d0d4d94c53bca68c9))
      - fixed x8-x17 register handling for FFA 1.2 ([bb9fc8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb9fc8c0bbf5f68f7461296ba36bf398091cb675))
      - set NS bit by default and clear it as needed ([cb2e574](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cb2e5746e4ed15e7a216fef3b739248ddd063a5f))
      - update event log related include ([982e702](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/982e702e4aff961d848f0cfc64c5069b57defa5a))
      - validate fragment offset ([c55b519](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c55b519ed87997f8136ebce6d39e08d08a6e9a2a))

    - **SPMD**

      - skip spurious id while group0 interrupt handling ([5b10f25](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b10f25a1ff3fcbbe431eebb36722ec1c5d04797))

  - **DRTM**

    - add event name data for separator event entries ([94aa3d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/94aa3d276cbb891ecdcb8b17163cda4063794c00))
    - correct PROTECT_MEM_REGION value as per DRTM specification ([65f6936](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/65f6936ef5ff145f599a5d0f29f2fd67b8a0833d))
    - measure and record the DLME EP offset instead of absolute address ([f3fec61](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f3fec61fbd9d0e718be0f3fd9a42cc965b08b11a))
    - remove plat_system_reset() ([30bbc4f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/30bbc4fa591c68264cf09baaef383711985299a4))
    - set correct max region mask for DMA protection to 0xFFFF ([3b6e594](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3b6e5947d9c1cfadafcc076049b869efb1739840))

  - **TRNG**

    - avoid undefined behaviour in pack_entropy ([f2db4eb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f2db4ebc0c8c4a752f81599a259a09bc8b8392d0))

  - **Live Firmware Activation**

    - check error code from plat_lfa_load_auth_image ([10f81c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10f81c86181bc122c8432c5c4907a1a0ea3ec452))
    - include string.h in lfa_main.c ([c0d32ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0d32ee6ca286828ed6ea351c1646597c8357183))

  - **Secure Payload Dispatcher**

    - **OP-TEE**

      - allow opteed_handle_smc_load to handle 64-bit data_pa ([0c1ca2e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c1ca2ee20268e26b2d184e3025ce99a9e6e6b79))

- **Libraries**

  - align round_up with MISRA 10.1 and 10.8 ([fa8b749](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fa8b74958e57abb7866cd4611fb62afea795aa68))
  - declare unused parameters as void ([ccec2b9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ccec2b98f2dce6c0c3f84eb3298be0f2d050305c))
  - modify function to have single return ([fecf325](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fecf325b8c59772a02563697d33d77b3ba97f282))
  - use 64-bit constants in MDCR_EL2 bit macros ([a9e3195](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a9e3195c286f52f9d84bd782a397e19d85424427))

  - **CPU Support**

    - check minor revision before applying runtime errata ([645917a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/645917ab32a25457824d0c7039e80b1cc228925b))
    - correct alto library ([4348393](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/434839325adfcfc6ff017aed66c766f1cc30a375))
    - inform the compiler that struct cpu_ops is aligned ([759ed94](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/759ed9461296ffa74473ea31a8e5f1f03d8b9af3))
    - organize Cortex-X2 errata entries ([f753b4a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f753b4a99651bfb662137c98f2e0aaa43c577a2c))
    - put back the global declaration for erratum [#3701747](https://review.trustedfirmware.org:29418/TF-A/trusted-firmware-a/issues/3701747) ([28a0b5a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28a0b5a16ac19abe4545b400c2124d5f846bacf6))
    - use correct Makefile indentation for CVE-2018-3639 check ([360460a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/360460a1b0ebf338224e9b6b8abb032177acd6c0))
    - workaround for Cortex-A510 erratum 2008766 ([d64d421](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d64d4215744bb34bba96ab088fb05ab6cffda846))
    - workaround for Cortex-A510 erratum 2169012 ([124ff99](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/124ff99fab733926afab60781c175261813f0834))
    - workaround for Cortex-A510 erratum 2218134 ([4592f4e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4592f4eaea7c40b4a7d59cad381611f07a91f406))
    - workaround for Cortex-A510 erratum 2420992 ([4fb7090](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4fb7090e72617f87ceb118a61bff8f393cdb31c1))
    - workaround for Cortex-A510 erratum 3672349 ([af1fa79](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af1fa79631262d2b1454dec37cb52f64705b016e))
    - workaround for Cortex-A510 erratum 3704847 ([ea88493](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea88493655a82b9f363951cf6b604ff21cd711f0))
    - workaround for Cortex-A710 erratum 1901946 ([4467348](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4467348b63e02fde9a823bd476e50bc281ed63f7))
    - workaround for Cortex-A710 erratum 1916945 ([df067c0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/df067c0a60b0c05554ebac51a37fb7d9ba866130))
    - workaround for Cortex-A710 erratum 1917258 ([d91c417](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d91c417716f260081e6a78f561224117088fc2e0))
    - workaround for Cortex-A710 erratum 1927200 ([cb2702c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cb2702c4b72746b6ef4e2da8d04d3f4b56d85398))
    - workaround for Cortex-A715 erratum 2376701 ([4fca3ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4fca3ee496ff218e2b6efdcc979602dadc96bc27))
    - workaround for Cortex-A715 erratum 2409570 ([d6e941e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d6e941e2325627379cd16f8399378bdcc1aecf81))
    - workaround for Cortex-A715 erratum 3711916 ([5c5b9e3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5c5b9e3e05d2d9dfcf594754e82889fe710beef4))
    - workaround for Cortex-A720 erratum 2729604 ([217a79c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/217a79c4c3cfe714cb1ce4d083bd8f174d1be93c))
    - workaround for Cortex-A720 erratum 3711910 ([87e69a8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/87e69a8ff3820350adf64b910bddffcb5b1d8eca))
    - workaround for Cortex-X2 erratum 1901946 ([ce64ea6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce64ea6ecb2b03eaf23fd4ec1460072ed472a92b))
    - workaround for Cortex-X2 erratum 1916945 ([ff879c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff879c528056dd49b9fa18c544e832d6dc0dd75c))
    - workaround for Cortex-X2 erratum 1917258 ([ccee7fa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ccee7fa879a4252b3e665b07c302a5fc47fd129d))
    - workaround for Cortex-X2 erratum 1927200 ([e236548](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e236548455cc3f8e28cdd2daa3cffc6fae6ea73c))
    - workaround for Cortex-X2 erratum 1934260 ([2c0467a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c0467af6a62263401181cdf3e88cf0016457b02))
    - workaround for Cortex-X2 erratum 2136059 ([a8e4d5a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a8e4d5a5549d47ebc41fbfbc5e22b4e4c66ae918))
    - workaround for Cortex-X2 erratum 2267065 ([41b9697](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/41b969764a866587c1403db0971877aa267fd783))
    - workaround for Cortex-X2 erratum 2291219 ([989c798](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/989c798d5ad7a3965747c54695f5f307bd44f37c))
    - workaround for Cortex-X3 erratum 3213672 ([42920aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/42920aa743901395431f8c0ad0b79d0d79ef4ade))
    - workaround for Cortex-X3 erratum 3692984 ([f828efe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f828efe258e148b2707249a65b2e31ab9718c4a6))
    - workaround for Cortex-X3 erratum 3827463 ([6a464ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a464ee7754d9b24a9a35668bc5019164267dac7))
    - workaround for Cortex-X4 erratum 3133195 ([58148b9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/58148b92bed490cd050df4c6faad080b1f3f2571))
    - workaround for Cortex-X4 erratum 3887999 ([5a45f0f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5a45f0fca7002c2b5d4138d51d9ea5788440c229))
    - workaround for Neoverse-V2 erratum 3701771 ([98ea732](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/98ea7329c52b864473e5280bba3806dd04b30dc7))
    - workaround for Neoverse-V2 erratum 3841324 ([7d94765](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d947650dc41712cfc8830068a7ce06d56c6c205))
    - keep leading zeros in CVE ID prints ([d1ed0c3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d1ed0c3db3b0bb76d49c66a1c1259ae8607094e4))

  - **Debug FS**

    - guard against negative ch offsets ([c56a85d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c56a85d09bd7788cdb77329723c8459e5d834a53))
    - return negative value on error ([b1f5309](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b1f53092326b3fdadb0af444ecdcb3c4e1e8b84a))
    - set debugfs smc start to vendor EL3 ([4db17f4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4db17f4e57632b5f273114e141a00385ac5ee72e))

  - **EL3 Runtime**

    - allow RNDR access at EL3 even when RNG_TRAP is enabled ([45218c6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/45218c6466d6ddc55c4eea198a4245cd591bbeb1))
    - typecast operands to match data type ([f05b489](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f05b4894acfb681017bfda71fb16e6079a4f6a43))
    - remove unused cpu_data related macros ([2c730ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c730eeab4567df5ddcccb377d06e3bd754ef998))

    - **Context Management**

      - correctly restore BL2's context ([780c9f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/780c9f09241fb8c91f898f9a26c742757dd92af7))
      - deprecate use of NS_TIMER_SWITCH ([ccf6796](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ccf67965e1cd17291c8ba5119014fb41ed9168c0))
      - disable SPE/TRBE correctly ([985b6a6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/985b6a6bc84745f1423f851c86037a15fe1ac39c))
      - do not restore spsr and elr twice on external aborts ([f9f4944](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f9f4944d89939bb0e0d4ebf482969c37b602874b))
      - do not save SCR_EL3 on external aborts ([a202529](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a202529e8ed33bacd10069f4f7df081970f0c752))
      - remove unused macro ([c81b9cb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c81b9cb9da5ac487365783863d9c2155b7322c6b))
      - use safe casting in memcpy ([ea5a4e9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea5a4e9855b537a1b67231c5c017dc4e24c94ca2))

    - **SIMD**

      - enforce FP regs context mgmt when SVE regs are enabled ([da81d45](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/da81d45d752604fa0c31bb0327150bbd806ff680))

  - **Per-cpu**

    - remove redundant casts ([f7ccf12](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f7ccf1262ae80edcc0c325c5992f8cb991365666))

  - **PSCI**

    - add missing curly braces ([bac32cc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bac32cc44291b53bc4ccb957694dfbcef962f193))
    - add missing curly braces ([f3d9e22](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f3d9e22ac1929ef5fad0743d24a81aee9f689b63))
    - do not modify higher levels for standby in OSI mode ([b34be5d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b34be5df710fd65d609dc2aafcfbd1f3b9132888))
    - fix coverity issue with out-of-bounds read ([654ab9e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/654ab9e05194145f0cec99ca026ce6a9345c47e7))
    - initialise variable to default zero ([a9eb44d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a9eb44d48c9b99e0bbc4740265ea5e72e9675330))
    - initialise variable to default zero ([e8c3fdd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8c3fddb70ba8a268d1fe58002cc3354c500b597))
    - initialize the variables ([df51e33](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/df51e33b804b5f98754d2925e7f6779f7a8b5466))
    - typecast operands to match data type ([f6166f7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f6166f7f9d773d3d428c6993f21dc7c211932fa3))

  - **Granule Protection Tables**

    - fix fill_l1_cont_desc() function ([9bc1e59](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9bc1e59968f610a81eac4e55045480cc13abf8f8))

  - **SMCCC**

    - cleanup unused declaration ([8397768](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8397768665785a9379475b9bf781ea2caaf22888))
    - don't panic on a feature availability call with FEAT_RNG_TRAP ([f610c8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f610c8c38b523dd8f314dd5c849c92a0be2e7ded))
    - fixed define when ENABLE_FEAT_FPMR is disabled ([57b23ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/57b23eaa6714b96c1319af9174caf9ad0b09e7c4))
    - ignore SCR_EEL2_BIT ([ee75a71](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ee75a71e8de09d32b2c9b77fef0586cadcc842da))
    - modify feat discovery to use aarch64 value ([a0fa44b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a0fa44b48501a3f9ed102d8220afeb67df8e001a))
    - resolve caller world confusion ([4e4a8c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4e4a8c585d880a11e66ad551da3d5203fff327d7))

  - **Translation Tables**

    - add missing curly braces ([98a2af6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/98a2af68489e2f40712beae8860d4fc471d85216))
    - change MT_DEVICE to map to nGnRnE ([00669dc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/00669dcd96b9a51d6da01d68f45348950e6cf58f))
    - typecast expressions to match data type ([b6c1cdf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b6c1cdf57f08da7bcf703535b4957ed94cea9eed))

  - **C Standard Library**

    - add missing curly braces ([60e5aee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/60e5aee11890da7d561e11a6c8f251de5c068bb4))
    - add the memcpy declaration back ([f23d26d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f23d26d7b6794d2469aa898a86f0236e0cb21357))
    - fix coverity overflowed constant ([4d7238b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d7238bb1226310812b248f5719dc3892b1f7f0f))
    - fix coverity overflowed constant ([02dbb14](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/02dbb148837f7059e6806ec06dd097f44801d32f))
    - replace true-false with explicit comparisons ([28ac195](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28ac195b593d59fb74307dbcc6760885859badb7))

  - **Locks**

    - mark spin_trylock as a public function ([d7e9372](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d7e9372f5776c6fe45cfe8a011ba193261cf5a06))

  - **DICE Protection Environment**

    - prevent compiler warnings ([0c3b84c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c3b84c1e2b48a14bfc1946233a61d1425f20910))

  - **Context Management**

    - correct guards for PIE and POE registers ([f77d713](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f77d71325af9285c80b04a7aae76f70398f24137))

- **Drivers**

  - **Console**

    - add missing curly braces ([efea4ae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/efea4aec8779fae84c8d4bfdf8570ba15d065c1d))
    - create unique variable name ([6d6aa1d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d6aa1da8bc20b3eb99d2e05cedfd6f1579f8d2b))
    - ensured proper bitwise shift operation ([1ec2c39](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1ec2c39bc9a414c2b29a9ac0c5efee9aa492b700))
    - match function parameter is decleration ([acad3b0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/acad3b0f2c820481af73a50f507e34775ec8d6de))
    - typecast operands to match data type ([51df71c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/51df71c32681d38eb2f0bc89e63f6c1320e5fe27))
    - typecast operands to match data type ([5bbe4fd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5bbe4fdfb5696e62e4d0b06796036bf36e821b27))

  - **FWU**

    - fwu NV ctr upgraded on trial run ([526d956](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/526d956be980278453efc56c9e0357adaae2fc0a))

  - **SCMI**

    - change error code output for wrong ids ([48ec8d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/48ec8d339e601ce303ddb18805168abf9b4a3ef8))
    - fix coverity issue INTEGER_OVERFLOW ([6ec871d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6ec871d26977d559d1ca65f5b62ada5967bf293c))
    - fix coverity issue INTEGER_OVERFLOW ([a443fbd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a443fbd0d34b4c1fbc8c648e07a9e1fe3d22dab9))

  - **TPM**

    - extend timeout before reading data ([6053ca4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6053ca4cfba9c4edc5548120af311be4f4e3ac6d))

  - **Arm**

    - add missing curly braces ([21b7d86](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21b7d8608af98e3452b0e9b5f7577f73fe6d6748))
    - declare unused parameters as void ([d17b69d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d17b69dbd121f18561af802902d57b90a4e93f2a))
    - typecast operands to match data type ([0cd8e55](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0cd8e55f7bd6f7e231fd2f90fd4364b95c98918e))

    - **GIC**

      - fix coverity issue INTEGER_OVERFLOW ([b6f3b4f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b6f3b4f17ce3e149c705c901ad3d69e55929ca33))

      - **GICv3**

        - add missing curly braces ([fcaa188](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fcaa1882bc9add7013f4c7c0d52b6fa21c0de82e))
        - avoid incrementing global gicr_frames pointer ([6262a3e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6262a3ecf48ecadb012340fe7d8cb2a7bc24ba6f))
        - fix misra violation 12.1 ([4780c6f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4780c6fd868b7f107f7fad4ba399f8edf294545b))
        - match function definition and declaration ([d9df6b4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d9df6b432149236be9c39663387cbc73209f6972))
        - remove plat_gicv3_base.c ([07e18c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07e18c238e8479b7fb4ea6cba777765e9b3fa5a0))
        - typecast operands to match data type ([77189b0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77189b037ed6249872d21a6b3548d06c6d6b5ca1))
        - typecast operands to match data type ([eaa454a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eaa454ac5146f96fca0dc62f12ef67d7bc5e91d7))

    - **MHU**

      - shift by minor revision offset ([9fad664](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9fad664ef2f934890690465db15a782045c274ef))

  - **MediaTek**

    - fix incorrect source assignment ([f6d7b01](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f6d7b010d187acd6ec8980a36028a64a92df5b67))

  - **NXP**

    - **NXP Crypto**

      - restricts generating nxp_mkvb via ns-world ([76a9560](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/76a95609a05674635e867949f84a3fd499c01313))

    - **DDR**

      - add missing macro define for Errata A009663 ([f1f04d7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1f04d786795a02f81abeef7d1513e93098591ed))

    - **uSDHC**

      - correct the usage of BIT and GENMASK ([26fd068](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/26fd06888aed38f2d098b7a1339eda1adca54117))
      - fix clk_rate and bus_width type ([3d16507](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3d165079ddd74f596c46c9f66528702226501a9f))
      - fix the clock rate calculation ([2e90f3e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e90f3e68f9e3aec8666fd6d07ef9b4577e4f4fa))
      - handle response for CMD0 ([92a7b54](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/92a7b540f0052cc6e914873b6751f24c5d75f719))
      - remove unnecessary delay ([6347429](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6347429e9c8db63b1f2850725ed8c5253ecb8da0))
      - wait SDSTB before changing the clock ([583a544](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/583a544c62c334feef700e5da77087603ddaa705))

  - **ST**

    - remove useless field in fixed regul ([adeee68](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/adeee68b8a08a158c7d37b00102b3b1be040ba79))

    - **BSEC**

      - remove useless defines in BSEC3 ([b43afb7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b43afb7fe16c2351ae34a56d1dddb15c60a105b3))
      - rename OTPSR field ([6fede18](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6fede181224fd4f60ca5f835e764ea2f72cf4ab3))

    - **Clock**

      - add ck_bus_risaf4 clock for STM32MP2 ([2a20f3e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2a20f3e62a6aa35f68847bf99592afa030fd6cc1))
      - force ARM_DIVSEL for flexgen63 config at 400MHz ([40d0ceb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40d0cebe1022da24ebbd9ee67b5cdc7049aeaf5e))
      - prevent panic when external oscillator is absent ([6fb522d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6fb522dec0c0eae6f3be80b85714629989883ade))

    - **Crypto**

      - do not set IPRST if BUSY flag is present ([6bc7c5b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6bc7c5b70eba5032ee8b45419c8946ebdc7c0b48))
      - improve RNG health test configuration ([382dff5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/382dff55ba5a3d691f022fb02243aa6809252993))
      - resolve MISRA warning in HASH ([84ebe2a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/84ebe2a532380fbb10a13c880c84368f98af2c8e))

    - **DDR**

      - bad refresh update level toggle sequence ([6851fd9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6851fd9ecccd27c5131408c2e532a9bf624831be))
      - remove TODO in STM32MP2 driver ([fd5e5e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fd5e5e7b71845a67bfd0c684bfb884a6971a23e2))

    - **UART**

      - aarch32: remove unnecessary timeout waiting in putc ([5bebf8f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5bebf8fe5a217a75eea7b81c80afefa2b7cf1ae5))
      - aarch32: wait for UART ISR register TC bit for console flush ([35988c6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/35988c64162732939a6dacb0bd3bebf517c60517))
      - aarch64: unwait for UART ISR register TC bit in putc function ([8ad5ea0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8ad5ea03b5aebbfb275e347857e93f0bd6035eb2))
      - aarch64: wait for UART ISR register TC bit for console flush ([65a96c0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/65a96c048a7b61efc4275a33f734ca529981cbf7))

    - **USB**

      - correct phy_epnum type for error trace ([8defd6f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8defd6fe25d1faf12b36cca0af919f5e3fe1aa4d))
      - init endpoint with fixed value if only one is used ([fce3675](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fce367557fd1ca5e94afaf3309d283270a992a9c))
      - stub dead code ([e5bafa2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e5bafa29de77de3f6c5898e5e95c244d123ab498))

    - **Watchdog**

      - remove num_irq ([9adc427](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9adc42705a63af79a9dae6b65b3032361742e719))

- **Miscellaneous**

  - **AArch64**

    - do not print EL1 registers on EL3 panic ([8f15231](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8f15231912392342e7e11a244dee5b55bf7eb698))

  - **AArch32**

    - make get_cpu_ops_ptr() PCS compliant ([71f7a36](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/71f7a363fe9d5aa6466ffd4b663cf52d9033deaa))

  - **DT Bindings**

    - bad FLEXGEN configuration of pred-division for STM32MP25 ([a8d4cc7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a8d4cc71f077643d512fab9d7ede38ae6dab9da4))

  - **FDTs**

    - remove extra members in PCI interrupt-map ([cd170ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd170ec8744832785f03fbafe5f4dfe5c3a75302))

    - **ST**

      - **STM32MP2**

        - set SDMMC max frequency to 166 MHz on stm32mp25 ([a6665c0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a6665c0891a6c6606cee7e1eba1d8b367d6aa85e))

        - **STM32MP25**

          - new swizzle configuration for STM32MP257F-EV1 board ([f42f2e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f42f2e734287aca9a5b27c6f50c7e80fd88da110))

  - **Security**

    - add clrbhb support ([d6affea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d6affea1608080fa3a10dedc7b6abd3010d5eaf1))
    - fix Cortex-A715 CVE-2022-23960 ([ad0e848](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad0e8487fc640882c55dfced5e8c2d9412160094))
    - fix Cortex-X3 CVE-2022-23960 ([07df6c1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07df6c1cd678facb00e9810453260afec3d32528))
    - fix Neoverse V2 CVE-2022-23960 ([3084363](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3084363cd2333263e91ac79b365424f7c756c47b))
    - fix spectre bhb loop count for Cortex-A720 ([9fd05e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9fd05e648bc4cbe50a5794ae25e9a24b1c8d6eb3))
    - remove CVE_2022_23960 Cortex-A720 ([e22ccf0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e22ccf01a9e03fc08f6bfd07ca0032aaa16b579d))
    - remove CVE_2022_23960 Cortex-X4 ([a055fdd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a055fdddf01635c1ee2549319aeaa1830569c7c1))
    - remove CVE_2022_23960 Neoverse V3 ([a2e22ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a2e22acfdb9d32d115798eddceec9fff53a4867b))

  - **SDEI**

    - fix coverity finding  array index read ([6d11328](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d1132857efe58779da2028bad542cd797e9eb22))
    - resolve coverity error for SDEI GIC ([9dde2cc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9dde2cc1b8838807c1b4a7fa1ec62e883249d50d))

- **Documentation**

  - ensure all submodules are cloned ([efb21dd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/efb21ddae3bf1a12321057c96b9632876632a3a8))
  - fix some broken links ([854d199](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/854d199b56bc159aa137e499b23e1de75925ef17))
  - obey the build directory ([725a80d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/725a80d77cfd899c1d276d79f3a2c9c95d9bc55c))

  - **Changelog**

    - fix platform order and add smcc to deprecated ([07dc387](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07dc387db30ae623724e3ca85fe90ab30d7d960d))

- **Build System**

  - add DTB dependency for certificate generation ([f02135a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f02135a36970670baf3a7b42afdf184616673f01))
  - add include directory dependencies ([19f4e19](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/19f4e19921ca8d3ea94edccd30af222123f3f301))
  - align the cpu-ops flags with all others ([2897374](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28973741944acf2081463fd933e358e108ce2ab1))
  - correctly detect that an option is missing with ld_option ([b45fc16](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b45fc164162d53ee5f658b896951dae75af4ca54))
  - correctly forward C compiler to CMake ([fcada3d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fcada3d5edf9383102562c9df3f0ba95c445576f))
  - don't rely on Event Log build tree ([76d5d32](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/76d5d32fcf7e8859721e0d63a1ecc6b674a4ae0e))
  - fix compiler wrapper detection ([3dc69bc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3dc69bcb88b69f516eb42cea5d6e8acf5b1c6084))
  - fix incorrect expansions in `shell-map` ([46a898f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/46a898f9456a8e498b037623d31fc4b57879a63b))
  - fix incorrect parentheses expansion in `shell-map` ([cfc2d76](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cfc2d7668b18dded0055bed5619710e630ff8a8a))
  - fix lib cflags use in build macros ([d8a23ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d8a23ecd7f3c361667d25a99c502b144913e3be6))
  - fix Makefile syntax in constraints helpers ([d42144a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d42144a2adaf3dc53311540e7c88ef2cc00315c7))
  - force CMake to cross-compile ([3dc41a6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3dc41a619603fcc6582646e4fbfee2b356a2b47b))
  - forward compiler launcher to CMake ([f162e3a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f162e3ab58b5e60102b45006ca7b6cf19e1d3a70))
  - link Event Log library directly ([03032a9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/03032a95ae953145ee9bd3b2361b6f621a32666a))
  - prevent races on the build directory ([774fb37](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/774fb3798e43f03afbc8d7de8e62716cafd6914c))
  - put the -target definitions in toolchain.mk ([aa6edba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aa6edba3954811559a5cc5bb3cd7968d45f3e1d2))
  - remove redundant variables ([500927e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/500927ef6db936850c5102884cdd96a525b214f9))
  - remove SUPPORT_STACK_MEMTAG ([6bf7c6a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6bf7c6addda1dac8c8583cde14b94f7032c31c03))
  - scan symbols until all are resolved ([bba54a3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bba54a3fdf7949b7b5f8e0ed7c8719507d3aef90))
  - set ERRATA_SPECULATIVE_AT after platform.mk ([c2dc512](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c2dc512993686a9667d05e4b9f94297992c96f3c))
  - set linker to lld before evaluating ld_option ([5ecae95](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5ecae95170dc730e187bcc34130b2c26e18fb915))
  - shell-escape toolchain wrapper commands ([78cb57c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/78cb57ccbc0e9aa3b27f0aeac2fcaf8890d29c79))
  - simplify the -target options ([b926017](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b92601738ca0c3b79faab60d3988a50e80bbb4a8))

- **Tools**

  - renesas: rzg: Fix tool build ([fa0df1b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fa0df1bd76b176f7832031c1fa3a0044aacf4e37))

  - **Firmware Image Package Tool**

    - skip Layerscape makefile for S32 build ([ba3668f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ba3668f1865b44635e8c7aa3a38d0d315850cec3))

  - **Secure Partition Tool**

    - handle load-address-relative-offset property ([5f7116a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f7116a8fbe5e7992e4ab09969b53d58e56ddbc4))

  - **Memory Mapping Tool**

    - ensure terminal width is respected ([8ab677b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8ab677b900d34ea5c2234767551437e88b3cb9a3))

- **Dependencies**

  - **libfdt**

    - add suffix 'U' to unsigned integers ([a1321ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a1321ac00aaee56e589b81bc898c75189e859fea))

## [2.13.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.12.0..refs/tags/v2.13.0) (2025-05-14)

The threat model for context management support is not available in the release.

### ⚠ BREAKING CHANGES

- **Platforms**

  - **Arm**

    - rename PLAT_MHU_VERSION flag

      **See:** rename PLAT_MHU_VERSION flag ([a773955](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a7739550946acc0f9fd9f3360738eb046057ce28))

- **Bootloader Images**

  - **BL32**

    - **TSP**

      - The prototype for `tsp_early_platform_setup` has been
        redefined. Platforms must update their implementations to match the new
        function signature.

        **See:** cascade boot arguments to platforms ([32d9e8e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/32d9e8ec6c1f2889ffeb549007a7569754add5f1))

- **Drivers**

  - **Arm**

    - **RSE**

      - remove rse_comms_init

        **See:** remove rse_comms_init ([91c7a95](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/91c7a952bedb34f1b4c691988f9da9af02209441))

### New Features

- **Architecture**

  - **CPU feature like FEAT_XXXX / ID register handling in general**

    - add support for FEAT_PAUTH_LR ([025b1b8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/025b1b816b607c7ac43a77172040c44b7750a622))
    - add support for PMUv3p9 ([ba9e6a3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ba9e6a342de0a8922d75559bc3c539a5174e2004))
    - enable FEAT_BTI to FEAT_STATE_CHECKED ([10ecd58](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10ecd58093a34e95e2dfad65b1180610f29397cc))
    - enable FEAT_PAuth to FEAT_STATE_CHECKED ([8d9f5f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8d9f5f2586d32ca19c11f65f3c6954615f3efdf6))
    - disable FPMR trap ([a57e18e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a57e18e4337b74ce3d133a18f07fa891f0fd5fa9))
    - enable FEAT_MOPS in EL3 when INIT_UNUSED_NS_EL2=1 ([6b8df7b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6b8df7b9e5cc70ef0433a99bf73db6db6002482c))
    - setup per world MDCR_EL3 ([c95aa2e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c95aa2eb0dc7471df5f2c7a3d1cf939046bc170e))
    - add support for FEAT_SPE_FDS ([4fd9814](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4fd9814f4865efdffca17b7bbfc8d4bd244866f3))

- **Platforms**

  - **AMLogic**

    - **GXL**

      - add support for booting from U-Boot SPL/with standard params ([8dca65d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8dca65d96f787b6802b75bc72f830149293cbb6d))

  - **Arm**

    - add a macro for SPMC manifest base address ([eab1ed5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eab1ed54bfb6038a0c1ada79de409a5506e4dae0))
    - add initrd props to dtb at build time ([1c08ff3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1c08ff3277d47979c7897842ba683c23aa8197f7))
    - add support for Transfer List creation ([4c5ccbf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4c5ccbf43c1e34e3f2f71878f732efbe2449e9f6))
    - convert arm platforms to expect a wakeup ([da305ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/da305ec75dedca5e8e939790ab02fe7c0ba999d5))
    - enable Linux boot from fip as BL33 ([eb8cb95](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eb8cb9534b58537e4ead8d1f8112ece45993c86d))
    - migrate heap info to fw handoff ([ada4e59](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ada4e59d166ab9fe7b039ed0f0b272398f71bdb9))
    - port event log to firmware handoff ([b30d904](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b30d9043be420f6a288687cf9f282d4e578f69aa))
    - support AArch32 booting with handoff ([abdb953](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/abdb953b00fff4aace32d9be10005b89751858d0))
    - support boot info handoff and event log ([a852fa1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a852fa1d594f63bdc502a0c73eb77c5ce88aa02e))
    - use provided algs for (swd/p)rotpk ([da57b6e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/da57b6e3cfdb36ad96d136d17ec3d300f699fee3))
    - use the provided hash alg to hash rotpk ([d51981e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d51981e15dd8383d727bd15e718f657f2cd2aaf5))

    - **FVP**

      - add StandaloneMm manifest in fvp ([8416e79](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8416e7917f9c82336e900659d35038942418582f))
      - add stub function to retrieve DLME image auth features ([1733deb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1733deb46c02444febfd25e411d09b1888ba8a7b))
      - add stub platform function to get ACPI table region size ([5d37755](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d377555e862dd07e11022ba0a70f0150e34454b))
      - allocate L0 GPT at the top of SRAM ([7a4a070](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7a4a07078b3d15648c1cbbd9f309b0c11da56165))
      - build hob library ([8740771](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/874077132cf9a0504cccde6651c5714fa97beb07))
      - change size of PCIe memory region 2 ([2e55a3d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e55a3d74d588780e04f1632c1b9d7ad33fb5f4f))
      - define single Root region ([665a8fd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/665a8fdf3aa372862d62c34d23ffd678798a265c))
      - give bootargs on all configs ([a507f4f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a507f4f5f07253986b585e03583d40a818ebc3fb))
      - increase BL1 RW for PSA Crypto ([51bdb70](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/51bdb70fdb3719ad1eabb4e0d1037f4d89a552d6))
      - increase bl2 mmap len for handoff ([24f7830](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24f78301288195f511528fc58b00e48f61933fa8))
      - increase cactus-tertiary size ([dcd8d7f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dcd8d7f13d5318ea3b38e6558c13e7401d57530c))
      - increase GPT PPS to 1TB ([aeec55c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aeec55c8760f384bee46e1999287b0a757227c69))
      - port event log to firmware handoff ([5bf0b80](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5bf0b80784e29e9e77edb26b605d2c1791ecded2))
      - set defaults for build commandline ([bf9a25f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf9a25f075c4c94a3d47bc54c1a8a7b964444ff1))
      - support AArch32 booting with handoff ([2ab298b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2ab298b58bf3096f033ec922acc163c106118f62))

    - **Neoverse-RD**

      - deprecate and remove RD-N1-Edge platform variants ([71ad967](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/71ad967333a684f66eccd0de559ad2159ca9c1bd))
      - deprecate and remove RD-V1 platform variants ([afb3075](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/afb3075592bc8cc2f130f2ed6c5871a391bf0e56))
      - deprecate and remove SGI-575 platform ([a023201](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a0232015d089c0fca1e78b1d5fb9c8be61acca67))
      - use larger stack size when S-EL2 spmc is enabled ([82f4659](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/82f46593de26d758457391d8b5036abcc533c7e3))

      - **RD-V3**

        - add carveout for BL32 image ([6823f5f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6823f5f520e7dfcd0812bdf8d93deb8c770de5e8))
        - add dts files to enable hafnium as BL32 ([4d9b828](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d9b8281f35256f7367a6b76b1259d9edbefe713))
        - define SPMC manifest base address ([12973bc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/12973bcc89eff3a4bb4953061d48e8805b3c6079))
        - enable the support to fetch dynamic config ([37cc7fa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/37cc7fa539d9cfbbe1fefadb2346bc7415422fec))
        - introduce platform handler for Group0 interrupt ([4593b93](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4593b932396dca2ed945ab2c95c6cc6712c3eeeb))

    - **TC**

      - add 'kaslr-seed' node in device tree for TC3 ([2d967e9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d967e92e00fe05f0c1ecaddf414d50078050f9d))
      - add devicetree node for AP/RSE MHU ([06fa4c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/06fa4c4df28165dbf1d08158d86ef9997f099ad6))
      - add dsu pmu node for TC4 ([50ad0cf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/50ad0cfda371f520c3b0452af8955aac30634431))
      - add MCN PMU nodes in dts for TC4 ([624deb0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/624deb0825cf6b7e27165edf182bd075e58ee2ff))
      - add SLC MSC nodes to TC4 DT ([99f6790](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99f6790cb9681d0a2960f8d30d82c20db19f6ab7))
      - allow Android load and Boot From RAM ([932e64a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/932e64a1d79ef5ea7ed1525c45c9f11725ce5079))
      - configure UART for TC4 FPGA ([84ca47a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/84ca47a8ac03821cfd4dbe902c0ae71621e2f12f))
      - define MCN related macros for TC4 ([8f61c20](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8f61c20457c8e683b9c6b2a3f3c4ebcf4b1a5371))
      - enable Arm SPE for TC4 ([cea55c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cea55c836f205a988634c04416082e35ac350ff9))
      - enable DSU PMU el1 access for TC4 ([00397b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/00397b30b8da9489dd8ed5f51aa976bb34165a27))
      - enable MCN non-secure access to pmu counters on TC4 ([d1062c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d1062c472a2976a03c74479eb18327e46f604f6c))
      - enable stack protector ([d1de6b2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d1de6b2b57d9e52c3b08c63ae4ce2d1e6703ce70))
      - enable trng ([2ae197a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2ae197acd6a91a96619090e503521d44bee494b2))
      - fpga: Enable support for loading FIP image to DRAM ([969b759](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/969b7591dcd94b0380116d301c2ba463dd9bfd7c))
      - get entropy with PSA Crypto API ([8f0235f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8f0235fb8f2d46ee6ca6309f8c365ad57e3a1565))
      - increase SCP BL2 size to support optimization 0 ([3755e82](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3755e82c0f2c557d38dc4111f4ccd31a9394ee25))
      - initialize MHU channels with RSE ([0328f34](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0328f3422275a3d292c9025a1a1faaf9d54aa8d9))
      - initialize the RSE communication in earlier phase ([a3f9617](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a3f9617964ee30070855f9ba5e42ccb28dcc6a05))
      - port BL1-BL2 interface to firmware handoff framework ([93c50ae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/93c50ae67fbdefd0fa2457a1b44fc538265b8fe0))
      - port BL2-BL31 interface to firmware handoff framework ([2a36dee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2a36dee8f158290cc30425df0753b6d887f12256))
      - port BL31-BL33 interface to firmware handoff framework ([25a6bcd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/25a6bcd57104507c2f23760c2cc2ace43b5c00dd))
      - print ni-tower discovery tree ([d87a856](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d87a8562306a46049560ba4387789918ecbca4b6))
      - update CPU PMU nodes for tc4 ([1ce2c74](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1ce2c745a8b471dcc9e9f64e2163add566f5ce69))

    - **Automotive RD**

      - **RD-1 AE**

        - add Generic Timer in device tree ([6e1bf7e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e1bf7e97f7389f59c2ea0d28cfeef5399db7881))
        - add support for OP-TEE SPMC ([8b27eb7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8b27eb7d91d97d707af94fff11ea02963ee2c3c0))

  - **Intel**

    - add FDT support for Altera products ([29d1e29](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/29d1e29d7cb137311529603056ce36ff268f861c))
    - add support for query SDM config error and status ([fcf906c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fcf906c90088227b51a526533e71f4c8cd879778))
    - implementation of SiPSVC-V3 protocol framework ([204d5e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/204d5e67b90b9dfaf7d7e064c048b4587a73f965))
    - provide atf build version via smc call ([d1c58d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d1c58d86496164d701d3286786db73182925d0e9))
    - support FCS commands with SiPSVC V3 framework ([597fff5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/597fff5f8739b29fb779075be216d16acc28ab58))

  - **Marvell**

    - add trng driver ([6d5fad8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d5fad8d9ea4011fa49556592c7a4560e7206940))

  - **MediaTek**

    - add gic driver ([d905b3d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d905b3df300c1a822e666c5d8bd903ce16f5d507))
    - add UFS stub implementation ([57c7351](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/57c7351545befba76ae4725ef602eca0491cc5ac))
    - add vcp driver support ([a1763ae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a1763ae97eab53476eef556a068de4bdf36b737a))
    - update mtk_sip_def.h ([ead2602](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ead26026ff9f877b65011de55e3b3874e14aa137))

    - **MT8189**

      - add GIC driver on MT8189 ([1d193f9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d193f91ea89ad525acc9ec5d78084da445d3813))
      - add GPIO support ([cecbb93](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cecbb93c1e623f163621a97a2368c72ac9068bfb))
      - add reset and poweroff function for PSCI call ([4e40a1f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4e40a1fdcf50dcfacd4331163f5a547d325714b3))
      - disable L3C shared SRAM if the bootloader is coreboot ([7794e7c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7794e7c0b19f1fe470e62f59421d5770de1fcc95))
      - enable cirq for Mediatek MT8189 ([83a5a0d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83a5a0d1cc2d7cbb90ed9a11bb2b929a5f3b26e0))
      - initialize platform for MT8189 ([6c60901](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c60901a4569ac2d7c5807158186d6701d3a2cf3))

    - **MT8196**

      - add APU kernel control operations ([5e5c57d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5e5c57d52b1cfaec5182b2d01f804fae9ed54af4))
      - add APU power on/off functions ([3ee4b2d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3ee4b2def6d1ee92dafb1730ff7852d8af532009))
      - add APU power-on init flow ([0781f78](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0781f7804a6922b3bb40f2b50880a9563e8ccd84))
      - add APU RCX DevAPC setting ([f31932b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f31932b43087af86c56f919307a3ba4f7e994138))
      - add APU SMMU hardware semaphore operations ([2d134d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d134d28f585f4bbcbf2e698443ab79b64128af2))
      - add APUMMU setting ([e534d4f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e534d4f633c30742ef7ced69abe077399b8eb5e6))
      - add APUSYS AO DevAPC setting ([31a0b87](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/31a0b877566b0ebac6d11f2f8711cc11c0665e44))
      - add CPC module for power management ([75530ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/75530ee280048a416844265f947d7a11b920d1f1))
      - add DCM driver ([e578702](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e578702f71f99f728ca2406a89c1345132f44b78))
      - add GPIO support ([4cb9f2a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4cb9f2a5bf3a89d99f31505f830431e67a552709))
      - add LPM v2 support ([da8cc41](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/da8cc41bc802555e15e280a7c7bf97bece8f69b3))
      - add mcdi driver ([5cb0bc0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5cb0bc07e3700ae9354e48adad8a990b02d7c80d))
      - add mcusys moudles for power management ([95e974f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/95e974fa15da2e5f5ff2e151d0d97390e483be98))
      - add Mediatek EMI stub implementation for mt8196 ([39f5e27](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/39f5e2782061835650a80832a4a058921bd79568))
      - add Mediatek MMinfra stub implementation ([4794746](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4794746eec6191563a97fec55c06027e42abe9bb))
      - add mtcmos driver ([1f913a6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1f913a6e3a3eb6349db9a2263e15594c4dcd5766))
      - add PMIC driver ([d4e6f98](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d4e6f98d7f8b33ebd7706e8a440c568262500e21))
      - add pwr_ctrl module for CPU power management ([4ba679d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4ba679da8b90ee15dd1234bc773854debb9b2466))
      - add reset and poweroff function for PSCI call ([22d74da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/22d74da7cd0132b3b40c2289a3cbd152e548632c))
      - add SMMU driver for PM ([86dd08d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/86dd08d838a6a49e7915df0f20c90c3a3e003056))
      - add smpu protection for APU secure memory ([7ed4d67](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ed4d67c6351e61dd5500a0887e4d5e85b8ab764))
      - add SPM basic features support ([fb57af7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fb57af70ae8c93ebe806eb6281036a15113dc4f5))
      - add SPM common driver support ([a24b53e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a24b53e0e581cde7b5c43607ab76ded8336dc353))
      - add SPM common version support ([5532feb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5532feb70c699c54a21b02fada8fed59a3992786))
      - add SPM features support ([01ce1d5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/01ce1d5d2fedb4e7db2127286080f6495a5403cf))
      - add SPMI driver ([adf73ae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/adf73ae20a7aa6f8230cb7a19551edb239db8afe))
      - add topology module for power management ([da54c72](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/da54c7243618cbb21c5269cc153f4ac2508ee76f))
      - add vcore dvfs drivers ([f0dce79](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f0dce79600f239d9c2f382ba4573aba5e5126001))
      - disable debug flag in APU driver ([31137e1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/31137e1b1559b6e838e9a4e4b9f74a5473ba8837))
      - enable appropriate errata ([0d11e62](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0d11e62e67deb9a898b2ac5e7fbda58408df371b))
      - enable APU on mt8196 ([f5a6aa0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f5a6aa02a9332c42712c709c6017927b99ca04c2))
      - enable APU spmi operation ([823a57e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/823a57e11cd98ee6043e9637ff3a4cf58feb60ff))
      - enable apusys mailbox mpu protection ([83f836c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83f836c96238c0d0765d94cc1f8ed1c179d1878c))
      - enable apusys security control ([9059a37](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9059a375eeb20c08cdcd5e604b9fd68b47a31e7e))
      - enable cirq for MediaTek MT8196 ([49d8c11](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49d8c11285b28bddf9cdc7830ce039d4fc734f35))
      - enable DP and eDP for mt8196 ([3e43d1d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3e43d1d317fa0f4ba6fdfc95583b8e313333769d))
      - enable IRQ configuration ([16f94b9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/16f94b91f10a5f8d52b6eb16a685881c97f7df89))
      - enable PMIC low power setting ([e8e8768](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8e87683f2b4d8213a84475e5f981961dcc8a16b))
      - enable vcore dvfsrc feature ([a3c218a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a3c218afd6596a46791b7f5d84a8d45d3c6977a0))
      - fix MT8196 gpio driver ([6f891e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6f891e68964888bb9d97f83600d85e606882a8d2))
      - initialize platform for MediaTek MT8196 ([a65fadf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a65fadfbbd5919939bfe367fe3f2d3c22ca4cbf0))
      - link prebuilt library ([e033943](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e0339436614a2b86cae04dabc74efdb31e2b0fb4))
      - refactor LPM header include paths to use lpm_v2 ([6fac00a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6fac00a49099beda9caeb66d8bc35b00f0f52f10))
      - show ERROR log if need ([1ba50c3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1ba50c33fcdf10d2132d6d96a5238c378b57de2f))
      - turn on APU smpu protection ([5de1ace](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5de1ace54a7088828dfaccd275ec2f1938ce7ee4))

  - **NXP**

    - **S32G274A**

      - add console mapping ([a1e07b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a1e07b399b5724e0d8f68a2e6ba6ce3102c2c424))
      - dynamically map GIC regions ([5680f81](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5680f81cecbbbb8a584dcf62bcb766a1cb25345f))
      - dynamically map SIUL2 and fip img ([507ce7e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/507ce7ed6f5c2c34a94f18c6d66db27b163e0f2a))
      - enable MMU for BL2 stage ([eb4d418](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eb4d4185fa67edb6d137a5f7a0eb5e209b4e6299))
      - enable MMU for BL31 stage ([e2ae6ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e2ae6ceccc6fc96debbfbacfb4b288cbf78da0f2))
      - enable SDHC clock ([47b3a82](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47b3a8251f1b7948360714254f8ec22b0e23875e))
      - increase the number of MMU regions ([0089258](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/008925861f3d6b30722895bc9701b85ec3d81215))
      - map each image before its loading ([34fb2b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/34fb2b35b9996d2afe40ad52f3a004bb90bf95f2))
      - split early clock initialization ([61b5ef2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/61b5ef21af8bcb38d3492e15d1d3d1fbecf3cf49))

  - **QEMU**

    - add hob support for qemu platforms ([648d2d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/648d2d8e2dab8cf939350a1788c5204751dcc9be))
    - add plat_rmmd_mecid_key_update() ([9c9a31e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9c9a31eb9ae36cab9db58ddc5d49f82ca3cf976d))
    - hand off TPM event log via TL ([cc58f08](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc58f08fe6a53ab375322dbdec6edbe5707110b8))
    - update for renamed struct memory_bank ([991f536](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/991f5360b68d6cdfbda2950100dbf78ac1c482cc))

    - **SBSA**

      - add support for RME on SBSA machine ([acb0937](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/acb09373baa1f951b68168e044ddf94d299ef44f))
      - adjust DT memory start address when supporting RME ([99bc6cf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99bc6cf518c1d6c0995d53d00670c0ac972ea64d))
      - configure GPT based on system RAM ([d079d65](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d079d65d420d0f9edd4aa64b27305fc6537095da))
      - configure RMM manifest based on system RAM ([fb4edc3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fb4edc35bc19e411ae566d1089198760042324eb))
      - dissociate QEMU NS start address and NS_DRAM0_BASE ([26da60e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/26da60e2a0e47768997b1b2079848beb9b5479c6))
      - increase maximum FIP size ([122dbc2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/122dbc2c17494e96e7fd45ce6e03b68f8722585b))
      - relocate DT after the RMM when RME is enabled ([17af959](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/17af9597e2b5d7cda2aaadad74b352e9805522d2))

  - **QTI**

    - platform support for qcs615 ([f60617d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f60617d3b1f9446d7f1528b3ac16fe6c4db9779d))

  - **Raspberry Pi**

    - **Raspberry Pi 3**

      - add dTPM backed measured boot ([4f9894d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4f9894db3572b6e375c7369bc2619cc690169604))
      - implement eventlog handoff to BL33 ([6dfcf4e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6dfcf4e1df3b4690fdf2629815d2a91294f34493))
      - implement mboot for rpi3 ([c4c9e2b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c4c9e2bc436198ee8f4544e8091c0067c4b4d9be))

  - **Rockchip**

    - increase FDT Buffer for Rockchip Devices ([ab99dce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ab99dce4b7c8473d5bcb8c833bd410ab87b1e801))
    - update uart baudrate for rk3399 ([6d7f1d4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d7f1d4945ad3b85e33fb85eb7374d0fef2e9b64))

    - **RK3576**

      - support rk3576 ([036935a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/036935a8144b9c4b9f95f249ff4384945b846d40))

  - **Socionext**

    - **Synquacer**

      - add support Hob creation ([357f28d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/357f28db6bcca7856a8cdbedfe6ce4668b06b48c))

  - **ST**

    - use dedicated version of DT for SP_MIN ([71ba164](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/71ba1647e09ca7ccecfeca417b22d1f7d023e338))

    - **STM32MP1**

      - rework SVC services ([39b08bc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/39b08bc366ff1bb08a2ab1dfd9c1eab3157fe16d))

    - **STM32MP2**

      - add a runtime service for STGEN configuration ([7f41506](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f41506fa7c10ba8cc393a9e67ae81bed379d07b))
      - add common SMC runtime services ([f55b136](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f55b136abcdd2d6ba3bd5613eee8fff943de0fd1))
      - add FWU support ([c28c0ca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c28c0ca213147fff56555b038de8261fc5b92211))
      - disable PIE by default on STM32MP2 platform ([ac9abe7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ac9abe7e597b1c5712a449b4a2366c859621e435))
      - use USART1 for debug console on ultra-fly boards ([d59dd96](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d59dd96ddb2d58df989de07dc3d3fd86a1130652))

  - **Texas Instruments**

    - add support for TI mailbox driver ([9347ff4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9347ff4561e445d48839eac215cb452571cb8e54))
    - introduce basic support for the AM62L ([21b14fd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21b14fd2aef32cdb8e6304b0e1383157e651a86c))
    - introduce PSCI Driver for AM62L ([1abdc20](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1abdc20b35f7b39862f91dbf1dfb462872380f71))

  - **Xilinx**

    - **Versal NET**

      - add SDEI support ([10510c9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10510c98c7501a76af16f7345225cc4225ee2cda))

    - **ZynqMP**

      - add pin group for lower qspi interface ([fe81d9c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fe81d9c959968599db8b8a5b1f150224f3315a38))

  - **AMD**

    - **Versal Gen 2**

      - add bufferless IPI Support ([af22b19](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af22b19d6b9cc3985ca61c68eb1a364dff7c4874))
      - add dependency macro for PM ([aec66c3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aec66c38c72a71cd1d37926c29ce08a25823bf89))
      - add support for platform management ([414cf08](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/414cf08b76bcf7e8fdb841c3663716d151b133a8))
      - extended SMCCC payload for EEMI ([0cc5e21](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0cc5e2105564bbcf9fae8d39869d51fb6c58ba23))
      - is OCM configured as coherent ([c3ab09d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c3ab09d1c543bda64d543557556e8e03d2b26c32))
      - retrieve DT address from transfer list ([ea45387](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea453871ef70c116d40c950926c3c1a2c4036896))
      - update platform version to versal2 ([4003ac0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4003ac02eb4e5b0551614710f3e99a2e23cfe799))

- **Bootloader Images**

  - adding psa crypto - crypto_mod_finish() ([055c97a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/055c97afb2427092766687253d6ff851ee77455e))

  - **BL32**

    - enable r3 usage for boot args ([3fabca7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3fabca724a724266f41a210d377d79072b36e140))

    - **TSP**

      - cascade boot arguments to platforms ([32d9e8e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/32d9e8ec6c1f2889ffeb549007a7569754add5f1))

- **Services**

  - **FF-A**

    - add FFA_MEM_PERM_GET/SET_SMC64 ([ddf72e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ddf72e6a36bd6ef0958af5cfd638926861fe5c21))
    - support FFA_MSG_SEND_DIRECT_REQ2/RESP2 ([09a580b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/09a580b7961827501f94dd3dafbc27c7c5b69237))
       ```
       The FFA_MSG_SEND_DIRECT_REQ2/RESP2 early implementation in the EL3 SPMC is limited
       to the use of 8 input/output registers per the SMCCC which isn't compliant to the
       FF-A v1.2 specification mandating the use of 18 registers. This non compliance will
       be fixed in the next release with the necessary versioning checks.
       ```

  - **RME**

    - add SMMU and PCIe information to Boot manifest ([90552c6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/90552c612e31ba88c6a7ad47c4081bc89d09c9c5))

    - **TRP**

      - test el3-rmm ide km interface ([8b3a89f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8b3a89faddca3a955324adc4b48c864781e4c802))

    - **RMMD**

      - add FEAT_MEC support ([7e84f3c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e84f3cf908c8eab1565b8e2d9a543e50de2e78e))
      - add RMM_MECID_KEY_UPDATE call ([f801fdc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f801fdc22ef4fce3cc24fd1cbccde5772c15b633))
      - el3-rmm ide key management interface ([2132c70](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2132c707d73753338ce05f262843a91738177f5e))
      - verify FEAT_MEC present before calling plat hoook ([609ada9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/609ada9691ca3793ad7203536fb4c95088bd3026))

    - **RMM**

      - add PCIe IO info to Boot manifest ([bef44f6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bef44f60edaa763031bce6144c0267aefbb1f2bf))

  - **SPM**

    - use xfer list with Hob list in SPM_MM ([9ae5f67](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9ae5f67306b380c626a74782eff9e98b7f7996ee))

    - **EL3 SPMC**

      - ffa error handling in direct msg ([e1168bc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e1168bc37563d1f18d6d2a6dc4ed468eadf673f2))
      - support Hob list to boot S-EL0 SP ([4053a64](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4053a647f6eca71e5a34bc52303a049e3324ca90))
      - use spmd_smc_switch_state after secure interrupt ([a0a7f15](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a0a7f158d2932117d57ec0d74113890a565e0cbc))

    - **SPM MM**

      - move mm_communication header define to general header ([b51436c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b51436c2ca4f1ceb2145dc752584df6ab1b77670))

  - **DRTM**

    - add platform API to retrieve ACPI tables region size ([7792bdb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7792bdbdf91a5854e580adea0f993886c3eec5df))
    - ensure event types aligns with DRTM specification v1.1 ([8d24a30](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8d24a30d6073239a40eb835cb6014172080f2f06))
    - introduce plat API for DLME authentication features ([0f7ebef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0f7ebef73eb72f4627cf657fcb92f60c01001b41))
    - log No-Action Event in Event Log for DRTM measurements ([2ec4488](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2ec44880d0a47a60b851fea09b7bec0cd694e858))
    - retrieve DLME image authentication features ([94127ae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/94127ae299aedf7642e598785c90961ab6e18266))
    - update DLME data header with actual Event Log size ([9753238](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9753238fee94e97aff307a2c98d8e76bcbcfa211))
    - validate launch features in DRTM parameters ([8666bcf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8666bcfa75c2d7b80ccd0a92b252d5308e08216c))

- **Libraries**

  - **CPU Support**

    - add ENABLE_ERRATA_ALL flag ([593ae35](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/593ae35435f855ff3e48facc6a049261c0c37ea7))
    - add support for Alto CPU ([940ecd0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/940ecd072c695f51fc09eed1c13e50bbfa1690da))
    - add sysreg_bit_toggle ([bb80185](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb801857eaf21365402a4748296c05cb3c6e861f))

  - **EL3 Runtime**

    - **RAS**

      - add asynchronous error type corrected ([daeae49](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/daeae495117e8e1496a44392217230ced17b39be))
      - add eabort get helper function ([ec6f49c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ec6f49c26be266e9d2b26818f35765ba9edd4e10))

  - **HOB**

    - copy StandaloneMm Hob creation library in edk2 ([2105831](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2105831848c4aef432f49e229c530cbda3489b10))
    - introduce Hob creation library ([8953568](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8953568a2db48032f2a2d4065e68fd1b20980caf))
    - modify Hob creation code imported from edk2 ([6b68b4a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6b68b4a42f9858102f2c804026ed1ce136e15813))

  - **PSCI**

    - allow cores to wake up from powerdown ([2b5e00d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2b5e00d4eacbac4b315c1c2925882d0b77bc9205))
    - remove cpu context init by index ([ef738d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef738d19d3565e44ed44a6b06c6d9ab036322ac5))

  - **ROMlib**

    - add PSA Crypto ROMLIB support ([cf1b7fe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cf1b7fe657a3215423ab397363cc4fa1a0e6399f))

  - **GPT**

    - statically allocate bitlocks array ([b0f1c84](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b0f1c84035fb25e331b21f08f3f3e8e643c3394d))

  - **SMCCC**

    - add FEAT_TWED to ARCH_FEATURE_AVAILABILITY ([d33ff5e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d33ff5e0eec0f90cf74a442fd83faec4a03ecde5))
    - implement SMCCC_ARCH_FEATURE_AVAILABILITY ([8db1705](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8db170524de1eb83c21ee6344d628961f9b84456))

  - **C Standard Library**

    - import qsort implementation ([277713e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/277713e0ae3b9a20d661cbd9e214112f67bed35b))

  - **PSA**

    - add interface with RSE for retrieving entropy ([1147a47](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1147a470c257403466cdbd55a13823ca1652063e))

  - **Firmware Handoff**

    - add 32-bit variant of ep info ([7ffc1d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ffc1d6cf3c3981d74a3ac830f8a57f953b4ff03))
    - add 32-bit variant of SRAM layout ([8001247](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8001247ce267a583ae6a24a37a77f17427bd5204))
    - add func to check and init a tl ([f1d9459](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1d94593354a948dfc81ca569d4832afb20aaf41))
    - add lib to sp-min sources ([79e7aae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/79e7aae82dd173d1ccc63e5d553222f1d58f12f5))
    - add Mbed-TLS heap info entry tag ([0e932b8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0e932b8563b6e25ae5f233a789cd420e46ae9297))
    - add transfer entry printer ([937c513](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/937c513d5e8996d106e2661ea88786bde513ae74))
    - common API for TPM event log handoff ([4d8b4ca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d8b4ca0f5ab98e2d5d764a1530ae5fe55ba79bc))
    - transfer entry ID for TPM event log ([9821775](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9821775657a415aac4b1d1f9c66da6390a016934))

- **Drivers**

  - **Authentication**

    - add crypto_mod_finish() function ([0331bd2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0331bd22c6082776a3d82b3cd3a1e5771643562b))
    - add update of current_pk_oid in auth ([9c18c35](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9c18c35cbb21632d817ed2650ce80e5d544cac3b))
    - add util file for current pk_oid ([1716805](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/17168053b38878f700d5c4ac11421cb96a50bc1b))
    - extend REGISTER_CRYPTO_LIB calls ([95d49c6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/95d49c624f7a7784db1af6bb4de2007d2f044d61))
    - increase mbedtls heap for PSA RSA ([2ffc28c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2ffc28c833337d72b32580178ace9b466c7ebc24))
    - introducing auth.mk ([142ee34](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/142ee34ea375479282d842e1905a61436e792cd8))
    - mbedtls psa key id mgmt ([8a7505b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8a7505b008c71e61fa4e4d2b973932ec5720d526))

    - **mbedTLS**

      - introduce crypto lib heap info struct ([24da55e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24da55eef7f032a8b4957fd9eb53dc3b87d01997))
      - mbedtls config update for v3.6.2 ([c307efc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c307efce85cab2336ce0786b8ac46130508b8bfa))
      - optimize SHA256 for reduced memory footprint ([b57468b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b57468b3d029cb123b7881cefc68290cdeb31d9d))
      - update mbedtls to version 3.6.3 ([08f8c0a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/08f8c0a503b5c61d32f07edc8dae1a8179a174cd))

  - **I/O**

    - add generic gpio spi bit-bang driver ([3c54570](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3c54570afca013e050db3d01a4c948ae938d908a))

  - **Measured Boot**

    - add fw handoff event log utils ([ca39163](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ca391636ada5a382e35bdc41b930b46852a822a1))
    - make event log lib standalone ([cb03020](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cb03020e20859e61e94a2cf6a3eda86f87e796e2))

  - **TPM**

    - add Infineon SLB9670 GPIO SPI config ([6fa56e9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6fa56e93679631b0d23c88e962b9e40c97971942))
    - add tpm drivers and framework ([36e3d87](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/36e3d877cd6caf51155a74936f15b461cc9b814c))

  - **Arm**

    - **GIC**

      - add support for local chip addressing ([c89438b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c89438bcea49bf9d265e6e17f1ef13e6cfc9f62b))

  - **NXP**

    - **Clock**

      - add a basic get_rate implementation ([bd69113](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bd691136639963b61c028e55d5889997430e7fa7))
      - add base address for PERIPH_DFS ([29f8a95](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/29f8a952cbbc7ee91188464cdf01b243735d41f8))
      - add clock modules for uSDHC ([cf6d73d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cf6d73d4c3b0fc4b299e6249f063b5adb5e3bedc))
      - add clock objects for CGM dividers ([63d536f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/63d536fe183bdf1c6ad99a5fd81cda7e2f32750f))
      - add get_rate for clock muxes ([d1567da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d1567da68d954be8f454ed641cbf7a08ca86f0bd))
      - add get_rate for partition objects ([a74cf75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a74cf75f084e62888f57f7718f614bcd6e5eb50f))
      - add get_rate for s32cc_clk ([46de0b9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/46de0b9c992fd4da90075b39ccff0a849a976301))
      - add get_rate for s32cc_dfs ([2fb2550](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2fb25509b800726342955194a0c6ac24299fb08e))
      - add get_rate for s32cc_dfs_div ([8f23e76](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8f23e76fa5886ef9adbd867a546f291200fc2142))
      - add get_rate for s32cc_fixed_div ([7c298eb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7c298ebcbf1003b98f815b86b2014112e89644d3))
      - add get_rate for s32cc_pll ([fbebafa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fbebafa518d5cbc69d0c64023f002ff6706019f2))
      - add get_rate for s32cc_pll_out_div ([a762c50](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a762c50579cb3bcae9c266e652c7c959e66fa943))
      - dynamic map of the clock modules ([514c738](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/514c738045b5c2de0f8e99b68830322af88dad4d))
      - enable MC_CGM dividers ([2710bda](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2710bdadc4cefd45bb7056c7e4c1e20dd1dfddc4))
      - get MC_CGM divider's parent ([1586904](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/15869048b26f2ca2ebb78913ba1824532745744a))
      - get MC_CGM divider's rate ([ad412c0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad412c0d823ec1acad76f75419f40bdb09b2ad51))
      - get parent for the fixed dividers ([35988a9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/35988a9db0aad69d068715ffe4d2f456aed5b508))
      - get pll rate using get_module_rate ([43b4b29](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/43b4b29fb996ee05d2ca98c7f824d6a003342215))
      - restore pll output dividers rate ([c23dde6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c23dde6c193d26fae9b2a8e18140b90faeba3661))
      - set MC_CGM divider's rate ([f99078a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f99078a6a09c09c93ac8f0aa679e1e85ab9658f3))
      - set the rate for partition objects ([8501b1f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8501b1fc60e3f6d01a38a3055dc0f274ecb039c6))

  - **ST**

    - **ST PMIC**

      - add defines for NVM shadow registers ([c1222e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c1222e7b8e635061690fe430e9213e86aae87920))

- **Miscellaneous**
  - add a generic EXTRACT macro ([f963578](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f963578b24906121b75375517f46c8d4d2a538fd))
  - add EXTRACT_FIELD macro for field extraction ([af1dd6e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af1dd6e1a58957dc04747b9c87f482166022265c))
  - implement strnlen secure and strcpy secure function ([eb08889](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eb088894dc9fb08eb3da82b86ebdabe82ae45940))

  - **AArch64**

    - add DBGPRCR_EL1 register accessors ([bdcef87](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bdcef87cf5e16896688ee69aef216f7929a065c5))

  - **FDTs**

    - **ST**

      - **STM32MP1**

        - **STM32MP15**

          - add Linux Automation GmbH Fairy Tux 2 ([093c738](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/093c738ab642005ca273aa6f21d306c81163012c))
          - add Linux Automation GmbH TAC ([e67497f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e67497f9cc435e37c53b6a6361af3824247597df))
          - add SP_MIN versions of DT files ([20544d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/20544d66ccd53f8524d9d616fdab98dab88df37a))

      - **STM32MP2**

        - add LPDDR4 files ([64f82e5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/64f82e5a06cf4aeef9da40a8d503dc3f77cdb840))
        - add STM32MP257F-DK board support ([6a9e5ff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a9e5ffda76a7625786cfe6d4246dfdcb4a9f8f1))
        - add dual-ranked LPDDR4 config for STM32MP2 ([99adf4d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99adf4d43e09915e190b645227f41fa3ea10dd51))

        - **STM32MP25**

          - enable WDQS for LPDDR4 ([29917d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/29917d3a12c98ebe9776c3191cb699e1687771be))
          - update 2GB DDR configs ([388cb47](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/388cb47cf0275cfc435df305a75781f2cf5a5ca7))
          - add support for STM32MP257D-based ultra-fly-sbc board ([a4e31cf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4e31cf0d96438bca564e219490f55fa2e6abeb4))

- **Documentation**

  - update mboot threat model with dTPM ([b00f6ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b00f6ece560696a89bf2a91289ea9ef51a768c5d))

- **Build System**

  - rk3399: m0: add support for new binutils versions ([6fbec46](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6fbec46a59de301edd9ad63af3a8a007977173cf))

- **Tools**

  - **Secure Partition Tool**

    - add StMM memory region descriptor ([3553087](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/35530877967fd943186cd0afc895f71f0976bf23))
    - add the HOB list creation script ([cc594af](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc594af66e05b5f863b00dfab939f53e558d9c23))
    - include HOB file in the TL pkg ([32ecc0e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/32ecc0ef781d44f462aaeb441458d72b7dd5023d))
    - invoke the HOB list creation code ([2d317e8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d317e80c201573e9a05472ac1c96b0e6fe6e3bf))
    - populate secure partition number in makefile ([9327361](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/93273613b48048a4a884770e292e765fa85e3ad7))
    - specify endianness for HOB bin ([49c6566](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49c656633178d21679afd467e41f43a761d4238c))
    - transfer list to replace SP Pkg ([0fe374e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0fe374ef046f70b6e990024922034d14d418b109))

  - **Transfer List Compiler**

    - add --align argument ([c4c8e26](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c4c8e26a69411902516d394d8ca593db435c612b))
    - formalise random generation of TEs ([157c619](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/157c61978679ca7115e06df92d481b57fa2ae1ef))

### Resolved Issues

- **Architecture**

  - **CPU feature like FEAT_XXXX / ID register handling in general**

    - add support for 128-bit sysregs to EL3 crash handler ([58fadd6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/58fadd62beba8e9fefddae884bfd34f71a183997))
    - add feat_hcx check before enabling FEAT_MOPS ([484befb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/484befbfcbe5289fa237a96c36a2da75ff3ff2ba))
    - avoid using mrrs/msrr for tspd ([f3e2b49](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f3e2b499707c5cde85033c8ed16efe4f16de5231))
    - improve xpaci wrapper ([787977c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/787977c3afe9d951a088a1f12959b00d8343a18d))
    - include FEAT_MOPS declaration in aarch32 header ([8656bda](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8656bdab57b22b5a65bb0480429553c084c1b0a6))
    - replace "bti" mnemonic with hint instructions ([bdac600](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bdac600bc8c61e2facdd0b872cacfb7a24045fe4))
    - add a psb before updating context and remove context saving ([f808873](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f808873372381a401dcd86d7d45a5ee6fd164d50))
    - add a tsb before context switching ([73d98e3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/73d98e37593f4a4044dd28f52127cdc890911c0c))

- **Platforms**

  - **Arm**

    - create build directory before key generation ([db69d11](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/db69d118294f08aae86378c98aa082ac73e15b73))
    - don't race on the build directory ([9855568](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9855568cc5ab8af9a8bee54fcc35112240e16b28))
    - reinit secure and non-secure tls ([2948d1f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2948d1f81904f02034a0d12faf9b8c7f34b05795))
    - resolve build issue with ARM_ROTPK_LOCATION=regs option ([4569a49](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4569a496fd5b41b371c676b9313b796fe8bd27c2))
    - resolve dangling comments around macros ([523c787](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/523c78704fabfd5f35f0e6abe6df2192d33a3c95))
    - resolve misra rule R11.6 violation ([307a533](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/307a533301881869c0f54a5145a81a6a705156b9))
    - update tsp_early_platform_setup prototype ([9018b7b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9018b7b8401ab106228842140b53502ee64f1432))
    - use EL3_PAS in MAP_BL2_TOTAL definition ([875423d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/875423de49646402cd6a7c1850f52982a3b56299))

    - **Common**

      - add missing curly braces ([7e288d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e288d11a5f155de4d6ad2ab9827fd71c0c9f74e))
      - modify function to have single return ([50029b9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/50029b9ac3c60371f8606fb874df9038fb6839d0))
      - remove platform_core_pos_helper() ([96e46f5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/96e46f58d54a1a711bd7474a4ea949785119798e))
      - remove fvp_r ([2cadf21](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2cadf21bc8cc58e9ed8c7ae2af6089ffdb3d0a02))
      - remove unused vfp code ([18b129f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18b129f4c1539444f9b91655d325a013264f7312))
      - add missing curly braces ([0eeda63](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0eeda638a8a6e8d9ff0448ba7405a8a5b86d17e0))
      - ignore the unused function return value ([fc7a720](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fc7a7208e0ccc0722bb29fcbb0cb7a3d74ff0953))

    - **CSS**

      - turn the redistributor off on PSCI CPU_OFF ([50009f6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/50009f61177421118f42d6a000611ba0e613d54b))

    - **FVP**

      - allow PSCI 0.2 in the device tree ([94b500d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/94b500dcbb94b11c5b89657eda10e6f0b5fc87a1))
      - exclude extend memory map TZC regions ([06cec93](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/06cec933def225ef64d81983829c349f47b89319))
      - increase EventLog size for OP-TEE with multiple SPs ([d1a824e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d1a824ea0e8887151d1237f3836e7e0df4d9e5bd))

    - **Juno**

      - resolve BL2 RAM overflow with RSA+ECDSA in GCC(14.2.1) ([dd566a9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dd566a9e565842eb97b6e14da63ba01d13d061ec))

    - **Morello**

      - remove stray white-space in 'morello/platform.mk' ([05533d9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/05533d99922f76af82b765159bb56e6592c09cf0))

    - **Neoverse-RD**

      - initialize CNTFRQ_EL0 for RESET_TO_BL31 ([94a4383](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/94a4383a2b7de555ac68fc4f571d08105cdb3296))
      - initialize timer before use in smmuv3_poll ([64ff172](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/64ff172abe1edf333d41fb99aac1b4c50d85f248))
      - set correct SVE vector lengths ([842ba2f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/842ba2f22196beab45a581517ad43a6cdbf61d9a))

      - **RD-N2**

        - add LCA multichip data for RD-N2-Cfg2 ([289578e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/289578e610b6e4dfb62068a07bc56216e431fb17))
        - correct RD-N2 StMM uuid format ([6fb8d8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6fb8d8cf8427ebf2c63986ea3451ca835eac3aa2))

      - **RD-V3**

        - add console name to checksum calculation on RD-V3 ([222c87e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/222c87e75c747e4fb100a08910c5f870c473449d))
        - add LCA multichip data for RD-V3-Cfg2 ([d0b93a0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d0b93a0dd04c755264355633cc77c8c1ea78ada4))
        - correctly define plat_mboot_measure_key() ([fa0eb3c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fa0eb3cf5bab4cf86671a0ec3c7b0ceab33b7666))
        - correctly handle FP regs context saving ([ac05182](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ac05182df0b7c46c70faf92058063f8498a8624a))
        - fix comment for DRAM1 carveout size ([4e2369c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4e2369c707ba6ebd26810052532afa7d30cc4cc8))
        - handle invalid build combination ([fe488c3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fe488c3796e01187fb6cffdd27a1bee1a33e0931))

    - **TC**

      - define status to fix SPM tests ([8d4d190](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8d4d190915af07750cfc0c05b79c4ebd4bc1df4f))
      - eliminate unneeded MbedTLS dependency ([22220e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/22220e69f996b68c50dcc34cd05152a0fcb0e904))
      - enable certificate on the last secure partition ([2e36131](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e361319ac1907009b42da657f7c55a50a9ccca0))
      - enable Last-level cache (LLC) for tc4 ([7b41aca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7b41acaf72dc208601c315189756fdb9f24f4abf))
      - fix compilation error ([26a520b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/26a520b2befb310b9ee23da108854c9d9c294c11))
      - fix SMMU streamId for tc4 gpu ([bf223c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf223c79377ed5d94f58acd2f354e8524ed6b1c6))
      - map mem_protect flash region ([4bfe49e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4bfe49ec4eff17ded7f7b13fe4ceaa7307a78f92))
      - modify DPU configuration in dts for TC4 FPGA ([bb9b893](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb9b89366f580d6166d0abcd4ae8b3c45943ca1c))
      - modify ethernet configuration for TC4 FPGA ([8dec630](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8dec63032e8d173215fc5a4b6540ecbc189cbf87))
      - modify gpio controller base addr for TC4 FPGA ([5de9d79](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5de9d79bc4b2febe3b55db47039ab2004d8cd4af))
      - modify mmc configuration for TC4 FPGA ([ba1faaf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ba1faaf117158dc1f1272192f3d8131421e96458))
      - replace vencoder with simple panel for kernel > 6.6 ([1d2d96d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d2d96dd5cefdd8ae1872022d4b3b0d564188a29))
      - set console baurate to 38400 for fvp as well ([5428938](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/54289385f128e5ddad8d2f1be0d63a9edf685922))
      - set system-coherency to 0(ACE-LITE) for tc4-gpu ([cada6ca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cada6ca36c0a59a0050abf481ffa82a1838b1fe0))

    - **Automotive RD**

      - **RD-1 AE**

        - fix rd1-ae device tree ([f72eeb2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f72eeb2d227702817d38b44bec8ee988f30f1fb3))
        - rename legacy MPAM build option ([7a5e580](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7a5e580957a426ba4d4db2a07449a43d04628814))

  - **Intel**

    - add FPGA isolation trigger when reconfiguration ([6ce576c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6ce576c63d6333261f48ecee301a11e77cc5f0d0))
    - handle cold reset via physical reset switch ([bf3877e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf3877e072af2b718454e9ee1ee16d769980378e))
    - redesign F2SOC bridge enable and disable flow for Agilex5 ([42e9062](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/42e906205ecbb3e157a802a86ae98ef392a4b6eb))
    - this patch is used to solve DDR and VAB ([458b40d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/458b40df58d60974f2f57017c8f17663e8e0973e))
    - update debug messages to appropriate class ([a550aeb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a550aeb39460d34e5b75421639f54b215b46e616))
    - update ssbl naming conventions ([bf2c213](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf2c213670feb2ecc6acc316be0c0342c5a5159f))
    - update warm reset routine and bootscratch register usage ([646a9a1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/646a9a16150066eaa3146d4e2819d589333b6454))
    - add in support for agilex5 b0 jtag id ([8a0a006](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8a0a006af3362a114ed899f2cfe6c2ef0ec84061))

  - **Marvell**

    - **Armada**

      - don't race on the UART_IMAGE ([3395bd1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3395bd12bcd7595b66902c0fc70ffee0a4681eea))

  - **MediaTek**

    - covert MTK_BL to uppercase for the build ([c710579](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c7105798eda68240093d4c2bb34fcbc09f0ea587))

    - **MT8189**

      - fix mt8189 platform build failure ([49d8678](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49d8678efbae57a2445af4c535f2ebd73dfb6b48))

    - **MT8196**

      - add whole-archive option to prebuilt library ([8f7d9bf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8f7d9bfa0a15d3063f9475da9c5e579ea935ac68))
      - fix wrong register offset of dptx on MT8196 ([b38f8f7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b38f8f7a3e2f4cdd323f195d2a17a6091aa75722))
      - remove CPU_IDLE_SRAM_BASE entry from plat_mmap ([83f37d9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83f37d998123f269e7ce901b190e016d52f9ebe3))
      - remove EC_SUSPEND_PIN initial setting ([1185526](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/11855267b55fb70522ed2ac015a6a0002641412e))
      - remove SPM support for ES chip ([ee2e99c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ee2e99c3e3fc8443bf48b05fe3776e3f2aaa6b23))

  - **NXP**

    - imx_trdc.h header guard ([2e9198d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e9198d0e0cbe9546348dd0a71ff6c3d2061d4eb))

    - **i.MX**

      - **i.MX 8M**

        - fix imx8mq build break ([1b65be5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b65be5943fc4f6a0382d03a4ed7393b383e56a4))

        - **i.MX 8M Plus**

          - apply ERRATA_A53_1530924 erratum ([e6d39da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e6d39da0473b84488dd012091c8deede0adc7d87))

        - **i.MX 8Q**

          - fix imx8mq build break due to hab ([3a36f70](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3a36f70ba02e88ed20083c1278a739c6c870aae0))

      - **i.MX 9**

        - **i.MX93**

          - trdc: restrict BLK_CTRL_S_AONMIX to secure world ([c6bf928](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c6bf92894c6c7096e380fe60c0e57b3db87ea0c7))

    - **S32G274A**

      - reduce the uSDHC clock to 200MHz ([9c640e0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9c640e09a86bdcc4cf77233c71b2ac0d36b7f9d1))

  - **QEMU**

    - fix register convention in BL31 for qemu ([7ad6775](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ad6775bde8fb719ab94ab7d47736fd2dd66cddc))
    - fix RMM manifest checksum calculation ([d08dca4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d08dca4263e5286b7f9b36612dae5767aac74a63))
    - ignore TPM error ([ddb5e2f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ddb5e2fd12a759270dd3566d086bbb91076ae0ae))
    - statically allocate bitlocks array ([a32a77f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a32a77f9c7567141556a823c0b9d4d5488c95722))

    - **SBSA**

      - fix compilation error when accessing DT functions ([33ac6f9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33ac6f99abcbb2d3b8303af5194390cafb4e6853))

  - **QTI**

    - **MSM8916**

      - update tsp_early_platform_setup prototype ([2f02426](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2f024262e986f241fed5dbadbb7f0ab68165bb74))

  - **Raspberry Pi**

    - **Raspberry Pi 3**

      - expose BL1_RW to BL2 map for mboot ([9acaade](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9acaaded3ca9b6966efb72e3a989f8ee753b3a44))
      - use correct name for include guards ([5c0cbb2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5c0cbb2faaa0ddfbe7c36d037918fbda13a661f6))

  - **Renesas**

    - **R-Car**

      - use platform_def ([99fe5c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99fe5c2fd531298457214a61dafcfae15053e900))

  - **Rockchip**

    - pmu: Do not mark already defined functions as weak ([7f25d3c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f25d3cc782337ce2fd445d39cbe295090beb5c9))

    - **PX30**

      - remove unused function ([ce66647](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce666476812099bc5e27dcc1d336e712e0cd78fb))

    - **RK3288**

      - remove unused function ([6e38cc9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e38cc97df86ac1c45aacc9ce301f6df1fc72056))

    - **RK3399**

      - dram: Fix build with gcc 11 ([f86c230](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f86c230ab02ce94d54866c072e8c118095a8783e))
      - fix unquoted .incbin for clang ([ddd70f1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ddd70f199bd0b09a229aa624497b86dbff5b8db5))
      - m0: Makefile: fix outside array bounds warning ([5049f91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5049f910aec254b9656dafe6c4cf7dd3d8b91d5c))
      - mark INCBIN-generated sections as SHF_ALLOC ([279cad8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/279cad8ed38c186aebce3502f9f1ebc52e22b281))

    - **RK3588**

      - pmu: fix assembly symbol redefinition ([f879388](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f879388831795c948c7d5954d5ea6b1c224e6e08))

  - **Socionext**

    - update tsp_early_platform_setup prototype ([c45dbe7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c45dbe7e5d28da67f831159830cdc835095b142b))

  - **ST**

    - **STM32MP2**

      - correct early/crash console init ([23647bd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/23647bd52c805594050c5cd0e387389760778492))

  - **Texas Instruments**

    - fix UNUSED_VALUE in AM62L PSCI Driver ([32302b4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/32302b44abc3a725b4197cec8f1411d8320542d9))

  - **Xilinx**

    - avoid unexpected variable update ([b3d25dc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b3d25dca86ad438423e2554b0c72ea8e429c30fb))
    - dcc console tests failing ([e14ae4b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e14ae4b30175d6c9877fd33497d43a8895f5b3d9))
    - modify function to have single return ([906d589](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/906d58927760c70112fd8e70a7e67f2a4989e9ed))
    - remove unused write_icc_asgi1r_el1() ([1c12cd1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1c12cd10fc4662141777f5b0fc0fbac2d00f50c3))
    - resolve misra rule 10.3 violations ([72eb16b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/72eb16b7f8d679e9da960185904e6e608d5c5a5c))
    - resolve misra rule 10.4 violations ([bdba3c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bdba3c84baa5c595e504ef2902832d04102a223a))
    - resolve misra rule 11.3 violations ([c35fe29](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c35fe2946b6e2b1e4e3fcf845289bcea791eb60a))
    - resolve misra rule 14.4 violation ([a5d5cb3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a5d5cb3cfc83928bee0a00dd987f2731e1b8ec56))
    - resolve misra rule 15.7 violations ([fd44cc7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fd44cc7e9b660a8baae2e8a0d3a2b3efde2e2c7e))
    - resolve misra rule 2.2 violations ([e5adcfc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e5adcfcdd143660bb9c3d56a44a42621066fca59))
    - resolve misra rule 2.3 violations ([09abae0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/09abae060d5c65ec8cb7a42a5ac75fa30eb1aabc))
    - resolve misra rule 2.7 violations ([d87b0ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d87b0ce30189cf692a6a6e21393bde0ac5029780))
    - resolve misra rule 8.3 violations ([3df32f8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3df32f852400be6d4c1f3199c89d9f36169ede71))
    - resolve misra rule 8.4 violations ([4b4080d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4b4080d78637fd0f071e0d6860114eb9ea16dea5))
    - resolve misra rule 8.6 violations ([eec03e9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eec03e947b3855c0a19bb64d39b4627e253a22b5))
    - runtime console to handle dt failure ([0791be8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0791be881388c32c13b5ed9e79dc640aeefaaab4))
    - typecast expression to match data type ([50ab135](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/50ab13577fd533ab99c874772e6630929506229c))
    - typecast expressions to match data type ([83bcef3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83bcef3f509d77582e4534f75ca2b4fd3ea4ffee))
    - typecast operands to match data type ([3a1a2da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3a1a2dae10ef3058e3973715fc4406544ff4aad7))
    - typecast operands to match data type ([7d15b94](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d15b94ba3d7160c79c5c3229015f7e9373490ff))
    - update tsp_early_platform_setup prototype ([470dd8b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/470dd8b49dab7211ca425f16fd94e8c18bbd37bf))

    - **Versal**

      - add unsigned suffix to match data type ([4b23240](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4b2324042a292d6e1bd877daaf91c05dc888a926))
      - handle invalid entry point in cpu hotplug scenario ([435bc14](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/435bc14a94008ab811ebac2735875a99ea6e464c))
      - modify function to have single return ([890781d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/890781d10ce362150359c00b06c8b7e9e1ee34d2))
      - typecast expressions to match data type ([b802b27](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b802b2784c8bcaae56d450ab55ee305880293c85))
      - typecast operands to match data type ([8e4d5c6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8e4d5c6db0e462ea33db2eaee4229583ace6cd6f))
      - typecast operands to match data type ([9b89de5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9b89de5fc4aa22e0cdc8e9f216135041549bbdff))

    - **Versal NET**

      - add missing curly braces ([9334fdf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9334fdf9718fed0f98a990f2f7f43cc40426b6dc))
      - add unsigned suffix to match data type ([baeeadd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/baeeaddff4840028d627f21af64e6397da88a058))
      - enable PSCI reset2 interface ([5f22f57](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f22f573adbbcb30b7e1793fdcc1ba7c88eceff0))
      - handle invalid entry point in cpu hotplug scenario ([e5e417d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e5e417ddecfaedd875adf47de4ad1396dd514ab9))
      - modify function to have single return ([5003a33](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5003a332b8e40ff93333b70a4365c1e13e8b1d10))
      - remove_redundant_lock_defs ([19799fd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/19799fd8fb318682a8169d5e4fc32e41ae50c0b1))
      - typecast expressions to match data type ([3cbe0ae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3cbe0ae5b8b2f79fe87c73d160ef7e33603ae604))
      - typecast operands to match data type ([d51c8e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d51c8e4c654a27e7487b7d15a64cec6f5eeab4e5))
      - typecast operands to match data type ([3dc93e5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3dc93e5139b262ff099c450d82433180b802a484))

    - **ZynqMP**

      - add missing curly braces ([e4a0c44](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e4a0c44f690d0658310914bcb5ae2355808a17b7))
      - align essential type categories ([1877bf2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1877bf2ce18092259cace97cec41ea32a97a8ed7))
      - fix length of clock name ([f535068](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f535068c84c00da3d4af40d6b571d9df39e07264))
      - fix syscnt frequency for QEMU ([55ae162](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/55ae162fd521cbe2ba500570692785159777a0ff))
      - handle invalid entry point in cpu hotplug scenario ([df44616](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/df44616a12096c16dd497646fc06684bdc818760))
      - modify function to have single return ([3f6d479](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3f6d47945a4bd94eae2d6ad17f50b59cda551c10))
      - typecast expression to match data type ([e2cc129](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e2cc129bcc58234dcb4607fe9264c1bdf4b282ea))
      - typecast expressions to match data type ([895e802](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/895e8029aa1f88da164966504be8fc8120c9f7a5))
      - typecast operands to match data type ([6ae9562](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6ae9562473812cce8d97f3a3f2a8ae7aafa75201))
      - typecast operands to match data type ([2863b0c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2863b0c46633d28a920ef14c68ee4a6e4842ab0b))

  - **AMD**

    - update transfer list args for OP-TEE ([573ec22](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/573ec2287eb616d1629b2c32fb28e894d852a78b))

    - **Versal Gen 2**

      - add missing curly braces ([9f51da5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9f51da5e2e6fcd81b22fb1c7f5cd5712bdc6cb3b))
      - align QEMU APU GT frequency with silicon ([f7a380e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f7a380e23ce8c7e4835367ee71af0bdeb505baa7))
      - enable system reset ([058edb8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/058edb8fcd7d32346bda0e43389cd0903074034a))
      - modify function to have single return ([fb2fdcd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fb2fdcd9536874aaff0c14f9c466214e717b0e91))
      - pass tl address to bl32 ([1fb3446](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1fb3446ed230c1011c982eb0bf62214f8abd38b2))
      - rename console build arg to generic ([2333ab4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2333ab4cd214150ac099ba0894bb6d1c3963d945))
      - typecast expressions to match data type ([fbc415d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fbc415d2046ca6e940323a88252fd40a68bebec4))
      - typecast operands to match data type ([07be78d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07be78d500aaabf2401bb4b31eb3b40b677fa110))
      - update DDR address map ([66569a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/66569a76880ceb6c02fbcfe993584c67336df353))
      - update transfer list as optional ([5cb9125](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5cb9125e9654e374dff4f322c3bbaef59511fcda))

- **Bootloader Images**

  - **BL1**

    - prevent null pointer dereference ([2d3b44e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d3b44e3073e8d6ec49dde45ec353d6f41290917))

  - **BL31**

    - add const qualifier ([e358089](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e358089d8382e4239a4b606b0e97cf5475da4976))
    - add missing curly braces ([88edd9c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/88edd9c6a09cc867ea377c7ad968e6a17595acf5))

  - **BL32**

    - **TSP**

      - use %u to display unsigned values ([3b06438](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3b06438dd1e038a7453d3b812ca6ef2da54f6ba8))

- **Services**

  - avoid altering function parameters ([1a0f565](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1a0f565b6240392ccb6982a15098ec288df7ea94))

  - **RME**

    - do not trap access to MPAM system registers in Realm mode ([d048af0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d048af0da1022a844e153093395087c724f12e40))
    - map DEVICE0_BASE as EL3_PAS ([b577248](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b57724806143e36ae17be7e33e5792863c5c026c))
    - remove ENABLE_PIE restriction ([e126ed1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e126ed1ae7d498110b349be5f0bbd40e3bdc32ba))

    - **RMM**

      - add support for BRBCR_EL2 register for feat_brbe ([41ae047](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/41ae04735284f7c97a3993dc6d2d45de3954c2b7))

  - **SPM**

    - **EL3 SPMC**

      - fix FF-A v1.2 version check ([6feaad6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6feaad6c6e36f3bd92dd7181f08b2abc87f54240))
      - move ERROR line inside conditional ([bbf28dc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bbf28dc37d50ea8a5bf40ab3fcc2f529cbb1e024))

    - **SPMD**

      - check pwr mgmt status for SPMC framework response ([8723eaf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8723eaf2fea9d09526fd7e6bc544b9c3103240ac))
      - fix build failure due to redefinition ([a869e2d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a869e2dc4549e86465d485d9ec5d1c18d21c10be))
      - prevent SIMD context loss ([8f60d99](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8f60d99f44f3f75108f7cf707e8d1c8f6ef33be0))

    - **SPM MM**

      - prevent excessive racing ([48426cf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/48426cff7250486b9a00a2e0e69d53cb5ae6e413))

  - **DRTM**

    - add missing DLME data regions for min size requirement ([a65fa57](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a65fa57b129713ddaa3cdff048921368f9dacc2e))
    - adjust Event Log size in DLME ([63d2020](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/63d2020f577aadb9f600a7f1a525d427117fd11d))
    - fix DLME data size check ([28e8f9d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28e8f9d93329b45e8dc9bf6ee1f3d14bde9adda2))
    - sort the address-map in ascending order ([7cf3784](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7cf37848140d3ba29b5967b46acbc5464b0b04b0))

  - **TRNG**

    - allow FEAT_RNG_TRAP in dynamic fashion ([bc30945](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc30945bb660c5be04d82bfa95f5b9caaffba3ef))

  - **ERRATA ABI**

    - add support for handling split workarounds ([bbff267](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bbff267b6f2777483a37781850dd1be01c16b937))

- **Libraries**

  - **CPU Support**

    - add missing add_erratum_entry ([a74b009](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a74b0094163b0efc5525131c5b807aaea2591e9e))
    - avoid SME related loss of context on powerdown ([45c7328](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/45c7328c0b94d043745b4a44c2e14e1a77f5c347))
    - clear CPUPWRCTLR_EL1.CORE_PWRDN_EN_BIT on reset ([c9f352c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c9f352c362a8d114a055bb9206c5b6391ec3b96a))
    - declare reset errata correctly ([5cba510](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5cba510ee31c520c6bd3254a4fd791d411d02152))
    - drop esb from the Neoverse N1 ([e75eea7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e75eea749a59ba67c3eb3dfba41bd8d6770cc708))
    - fix a typo in errata doc ([845213e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/845213ed0ddb08be79f621990db1314f4988d2e5))
    - fix clang compilation issue ([bdaf0d9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bdaf0d9ba71c1fd17f5d35f8fd2a6205136116a3))
    - remove errata setting PF_MODE to conservative ([ac9f4b4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ac9f4b4da6f6ef695a25aa2ed525281a89d40bf9))
    - workaround for accessing ICH_VMCR_EL2 ([7455cd1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7455cd1721ec3b8671d6a2fae879f86ecfe497fb))
    - workaround for Cortex-A710 erratum 3701772 ([463b5b4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/463b5b4a46552887c4fb70536d20f315f889add1))
    - workaround for Cortex-A715 erratum 2804830 ([fcf2ab7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fcf2ab71ac00122bd382973f6e9f08e30b7eca80))
    - workaround for Cortex-A715 erratum 3699560 ([26437af](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/26437afde1e7c16a5a05b8c7e0bb062efe63c3cf))
    - workaround for Cortex-A720 erratum 3699561 ([050c4a3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/050c4a38a335c721c2f6ce38c33f1aa6aa328800))
    - workaround for Cortex-A720-AE erratum 3699562 ([af5ae9a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af5ae9a73f67dc8c9ed493846d031b052b0f22a0))
    - workaround for Cortex-A725 erratum 3699564 ([d732300](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d732300b86238ac7166bc9bebd667a24dc3ed062))
    - workaround for Cortex-X2 erratum 3701772 ([ae6c7c9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae6c7c97d4e0f491854b34628e0fa1038668f8e4))
    - workaround for Cortex-X3 erratum 3701769 ([77feb74](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77feb745e42296c553c33bcaea5be3304168eff3))
    - workaround for Cortex-X4 erratum 2923985 ([cc46166](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc46166144b9a3746807f1d137ff784da3013e26))
    - workaround for Cortex-X4 erratum 2957258 ([09c1edb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/09c1edb84b9d2e91ce425b4300042751b3899434))
    - workaround for Cortex-X4 erratum 3701758 ([38401c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/38401c5388b5c3127745b758b3dc939d27041d7d))
    - workaround for Cortex-X925 erratum 2963999 ([29bda25](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/29bda258d5655d2ac24dda0f73e6b0ec93c3037e))
    - workaround for Cortex-X925 erratum 3701747 ([511148e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/511148ef5077dfb8f6bc7b9655e4ac19e16c4af0))
    - workaround for CVE-2024-5660 for Cortex-A710 ([0d7b503](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0d7b503f8a11237fa129ae7baa2d979d64b9ab68))
    - workaround for CVE-2024-5660 for Cortex-A77 ([aed3e8b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aed3e8b59a034ddcd073f2295cf0ae70a313ccf5))
    - workaround for CVE-2024-5660 for Cortex-A78 ([c818bf1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c818bf1d60c01e1a8953bf0051987dea4db7c4bf))
    - workaround for CVE-2024-5660 for Cortex-A78_AE ([902dc0e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/902dc0e01ffc5f215eabde8e0428ce462db18a73))
    - workaround for CVE-2024-5660 for Cortex-A78C ([46a4cad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/46a4cadb9d63f4265756d82c9e252d7760c43ae8))
    - workaround for CVE-2024-5660 for Cortex-X1 ([26293a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/26293a746319e7073dd3699be0afcd2277ad1999))
    - workaround for CVE-2024-5660 for Cortex-X2 ([5b58142](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b58142c460b9d28740c610d51023a444583a10e))
    - workaround for CVE-2024-5660 for Cortex-X3 ([b0d441b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b0d441bdadae3b38e3a670e03341603785b39f3c))
    - workaround for CVE-2024-5660 for Cortex-X4 ([af65cbb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af65cbb9549765917cf79ab0a819fe58773882ab))
    - workaround for CVE-2024-5660 for Cortex-X925 ([ebc090f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ebc090fbf47a25a1ef84657d03198fc3a29d28e3))
    - workaround for CVE-2024-5660 for Neoverse-N2 ([26e0ff9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/26e0ff9d5e99b2ea0eedfd202401655ee404e52f))
    - workaround for CVE-2024-5660 for Neoverse-V1 ([85709f6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/85709f66194cef32377a32f8e153316648ebb4a9))
    - workaround for CVE-2024-5660 for Neoverse-V2 ([878464f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/878464f02a043207f29282f05109012152df569b))
    - workaround for CVE-2024-5660 for Neoverse-V3 ([ad3da01](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad3da019904302d9ecfc2cf50dfdd6d672427b5d))
    - workaround for Neoverse-N2 erratum 3701773 ([adea6e5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/adea6e52a782eeabd9027e3ca9a9847a13453cfa))
    - workaround for Neoverse-N3 erratum 3699563 ([fded839](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fded839285bdbdb72d492a573274abe22edc9311))
    - workaround for Neoverse-V3 erratum 2970647 ([5f32fd2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f32fd2145020b0007b3f12ae6a95b734ff8e6a2))
    - workaround for Neoverse-V3 erratum 3701767 ([e25fc9d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e25fc9df25092be31e7f7a9cc740e8df661a35c1))
    - workaround for Cortex-A510 erratum 2971420 ([f2bd352](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f2bd35282066f512c26d859aa086cff13955d76b))

  - **EL3 Runtime**

    - add const qualifier ([54c9c68](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/54c9c68a49bc7b3aaa29207f798b1973e3cdb542))
    - add missing curly braces ([858dc35](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/858dc35cfde12ddf6250ed393298fa85d01b77d5))
    - for nested serrors, restore x30 to lower EL address ([0bc3115](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0bc3115f6e7b857344c6ac034d74b0fe81d216ed))
    - replace CTX_ESR_EL3 with CTX_DOUBLE_FAULT_ESR ([c722003](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c72200357aed49fd51dc21e45d4396f5402df811))
    - make sure LTO doesn't garbage collect the handlers ([f8d2a0e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f8d2a0e5caa1b57edf9f7a8e09f9c438a7f3b4be))

    - **Context Management**

      - change back owning security state when a feature is disabled ([13f4a25](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/13f4a25251cc6ce0230e999f39a4668cff25dcd0))
      - don't access a field that doesn't exist ([600717f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/600717fe5909e02a9ec9ee08bcb921a66e653d58))
      - fix context management SYSREG128 write macros ([6595f4c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6595f4cb393090992733eb5e73928424b1e9395a))

    - **RAS**

      - fix status synchronous error type fields ([9c17687](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9c17687aabc9e64a26575d5662ef273185d9d275))
      - fix typo in uncorrectable error type UEO ([e5cd3e8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e5cd3e81d11cb7f9439e5077cf9063e30fc8c2ae))

    - **SIMD**

      - fix base register in fpregs_context_* ([09ada2f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/09ada2f8d63fabd1bb01494c2466f647441107b1))

  - **PSCI**

    - add const qualifier ([7b97084](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7b970841ad1c9925e72c170734ca30016813743d))
    - add missing curly braces ([c7b0a28](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c7b0a28d32ba78a1bec8fe1f9edbcdc215bf7b1a))
    - avoid altering function parameters ([e64cdee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e64cdee4dad610e8f4efb873e133a5fffdea397c))
    - check if a core is the last one in a requested power level ([71d4e03](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/71d4e03444f689210803323b437ee15d537fc6ea))
    - initialise variables ([382ba74](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/382ba743a81c9b61d2ab794a22e37af0ac6128fe))
    - modify variable conflicting with external function ([0839cfc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0839cfc980998d24740e41b44164de39b70079a1))

  - **ROMlib**

    - romlib build without MbedTLS ([e4a070e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e4a070e3d6dd0af0e74184333b9238e70c9a7075))

  - **SMCCC**

    - properly set RAS feature bit ([04b80c1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/04b80c187db3e8d5f0aec6987f1cb2fca2df952d))
    - register PMUv3p5 and PMUv3p7 bits with the FEATURE_AVAILABILITY call ([2bec665](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2bec665f464b1c211eac594b333656b1dfa4b3da))

  - **Translation Tables**

    - zeromem to clear all tables ([aaacde4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aaacde46826df1dd4a119b2a31ff626adbce5653))
    - remove xlat_mpu ([23302d4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/23302d4a53700a7e6b80a897626bee0d67fad028))

  - **C Standard Library**

    - add const qualifier ([b71d082](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b71d0827aeef53d0ad561da212a7deb102f48c45))
    - explicitly check operators precedence ([277d7dd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/277d7dd6c192f350bc80ccbb65c36562e59f8e07))
    - make sure __init functions are garbage collected ([53644fa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/53644fa8221b5c7b726beeda78b253f39abe479b))
    - remove __Nonnull type specifier ([7ce483e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ce483e17cf14ee285a348d0f0081c89793d010b))
    - typecast expressions to match data type ([dd0d433](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dd0d433197d470e1f50ab28f128ff336e021799f))
    - typecast operands to match data type ([26cc285](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/26cc2854609978fcd6b0450e750b3b319b2468fa))

  - **Locks**

    - add missing curly braces ([bd7ad5e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bd7ad5e684aafd12c9c5873e4776c4a14e5e5dc0))

  - **PSA**

    - guard Crypto APIs with CRYPTO_SUPPORT ([8a41106](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8a41106c83961029c2d251f390c091d398c100a6))
    - increase psa-mbedtls heap size for rsa ([52d2934](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52d293456097beef6a3346b244d4e84f78ef2615))

  - **Firmware Handoff**

    - correct 8-bit modulo csum calculation ([5ca0241](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5ca0241c7ac7fc07188281058e052044e8f9ec36))
    - fix message formatting of hex values ([24e1ae2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24e1ae2f0ed3e2c2be680aad6e88313661bf57ee))
    - fix register convention in opteed ([c0688c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0688c55bb8b924f1b660ffca1b33bd65cab9990))
    - remove XFERLIST_TB_FW_CONFIG ([18be2db](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18be2dbe001162b424e52072dee74c9b7613a4f2))

- **Drivers**

  - **Console**

    - add missing curly braces ([9ded5e8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9ded5e8d8be4a5f1f3219742c3790c13156378d8))
    - typecast expressions to match data type ([97eefd9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/97eefd9989aeb2ce2093e873ceab535df9559a59))

  - **Delay Timer**

    - create unique variable name ([472cccb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/472cccb5f94e26ad72b8a52ab2614203230be20f))

  - **MMC**

    - fix the length of the ocr defines ([e02d365](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e02d365ad9c4102cadb2d8886e3aed327a7512c9))
    - fix the length of the response type ([7b4b3f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7b4b3f243ad3ef099de9aa09a526e7ecccc09c04))

  - **GUID Partition Tables Support**

    - fix MBR header load ([2fac89d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2fac89d1267e81e1f4bf0eacf9ea8dbacf80aacb))
    - initialise the mbr_entry variable ([ec48d52](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ec48d52e7890d8e27d1b4419a6a8952d7db777cc))

  - **Arm**

    - add missing curly braces ([03c6bb0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/03c6bb0e3822e05da002c69086357c81f853d6f5))
    - align essential type categories ([bec4a2c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bec4a2c9c3919c4a3012c7a10f4fa75a861f8150))
    - typecast expression to match data type ([0f76d0d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0f76d0d57ec700b835e1715200f4520fde1c11c4))
    - typecast expressions to match data type ([edecc70](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/edecc70331ec5e7583972f66f0e8dc6cf039c686))

    - **GIC**

      - quote the correct flag on error ([df21ca0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/df21ca08e65b2aeb575e54b155fce1e32908eae6))

      - **GICv3**

        - do not assume redistributors are powered down ([57f2d00](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/57f2d009fb198181c53f768233f76087ab918ce1))

    - **SMMU**

      - set root port CR0 GPCEN before ACCESSEN ([8cc9724](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8cc972421faf02cfdd4fea1a21fabe6e9d96e2a3))

  - **Renesas**

    - **R-Car3**

      - disable A/B loader support by default ([1a57115](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1a5711519a8e685d3db43620623f0f616317cfe9))

  - **ST**

    - **ST PMIC**

      - remove deadcode from STPMIC2 driver ([bdbbf48](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bdbbf48f4db6fa64079efcd284955a3bf629b705))

- **Miscellaneous**

  - **AArch32**

    - avoid using r12 to store boot params ([af61b50](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af61b50c1077b6d936c8ed741c1d0b8e43eb2b19))

  - **FDTs**

    - **ST**

      - **STM32MP1**

        - re-enable RTC clock ([33573ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33573ea6842198cfdb5b3fdd320db9e2045855e9))

        - **STM32MP15**

          - update clocks config for prtt1x board family ([5aeb003](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5aeb0031558b4a9ccff9f097ae651d42d7b850a2))

      - **STM32MP2**

        - fix SDMMC slew rate ([575d6dd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/575d6dd7af3644f7c3c9cb34e3fb57d951695023))

  - **Security**

    - add CVE-2024-7881 mitigation to Cortex-X3 ([b0521a1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b0521a164a8d61c95968e728df9af52be1a48553))
    - add CVE-2024-7881 mitigation to Cortex-X4 ([6ce6aca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6ce6acac911f11979a6cd2ee9cc9041f189c6ec7))
    - add CVE-2024-7881 mitigation to Cortex-X925 ([520c220](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/520c2207b96d31adf508edc068bb97cd01d98da4))
    - add CVE-2024-7881 mitigation to Neoverse-V2 ([56bb1d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/56bb1d172ccee56e984559de69e8ebd8683d491b))
    - add CVE-2024-7881 mitigation to Neoverse-V3 ([037a15f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/037a15f5c72e856b8739a2159bd0fe1fe69e1d5b))
    - add support in cpu_ops for CVE-2024-7881 ([4caef42](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4caef42a94b4efe97b09764a7257f701ab7ff3b8))
    - apply SMCCC_ARCH_WORKAROUND_4 to affected cpus ([8ae6b1a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8ae6b1ad6c9c57b09b6d4e7ae3cbdf3aed6455b1))
    - enable WORKAROUND_CVE_2024_7881 build option ([2372179](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2372179484741aa81d5451b20748520677205c71))

  - **SDEI**

    - return SDEI_EINVAL if signaling state is incorrect ([b142ede](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b142ede74ceb5fc0b2908d0e7e8e2b2b27b6b1f6))

  - **TBBR**

    - remove tbbr_cot_bl1_r64.c ([a2328f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a2328f2eac83aa9bc10f89d9141ab9bd66338400))

- **Documentation**

  - fix the indent and the build command for MT8188 ([cef56a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cef56a5c6b42b4518af8c48f48a8790542ce3930))
  - put INIT_UNUSED_NS_EL2 docs back ([4557c0c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4557c0c001ba296328e6292a5a2c973581fbeb47))
  - update the instrumentation procedure ([9b65ffe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9b65ffefd673fb2781b808024d13039151c17668))

- **Build System**

  - do not force PLAT in plat_helpers.mk ([422b181](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/422b181faf2fd740e08cf881880015ea47b0cb67))
  - enable fp during fp save/restore ([5141de1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5141de14432aba7a775c0dd19ecba766d224ec39))
  - handle invalid spd build options ([a0effb9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a0effb91897ff41871cbe2366a54c6e2339246d8))
  - include platform mk earlier ([696ed16](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/696ed16877a121d07a766af5bb4f8d73a8ac01ae))
  - run sp_mk_gen.py with poetry ([dd81623](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dd816235771e19df1fd04ef2c6f1c6d11e429fd5))
  - update clang target for aarch64 ([af8947f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af8947fe17d3a29a47573ab0537d121d3653b53e))

- **Tools**

  - change data type to size_t for doimage ([fbf6555](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fbf6555790ad6de635ebf9e5581c840496166306))

  - **NXP Tools**

    - fix create_pbl buildroot build ([634c7d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/634c7d8172b7965cf4f5d7d6470a25956ac06ab4))
    - fix2 create_pbl buildroot build ([bfe7f80](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bfe7f8018947a960c287f6d8f5ac1efece7261b4))

  - **Certificate Creation Tool**

    - add default keysize to Brainpool ECDSA ([0da16fe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0da16fe32f41387f4ad32e96a939c67a3dc8e611))
    - load openSSL configuration before PKCS11 operations ([785c2c3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/785c2c3e8671aa69269bccd3ee968bebd3777a9f))

  - **Firmware Encryption Tool**

    - put build_msg under LOG_LEVEL flag ([7640df6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7640df6f1ef392d664d16aa4d49c67a24421adce))

  - **Renesas Tools**

    - **R-Car Layout Tool**

      - fix tool build ([72f4b70](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/72f4b70e8e8e656d88bda77605f73076474f5a55))

  - **Transfer List Compiler**

    - add void entries to align data ([03c2660](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/03c2660f7556957262c41032064716414572833c))
    - pass the flags from client interface ([537a25e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/537a25ef7ff6fc95221c70c9543404049143353d))
    - relax entry addition from YAML files ([f0e15dd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f0e15ddca303322764e17524396cb2da2c1c5ccc))

- **Dependencies**

  - remove deprecated husky commands ([b47dddd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b47dddd061e92054c3b2096fc8aa9688bfef68d6))

## [2.12.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.11.0..refs/tags/v2.12.0) (2024-11-19)

The threat model for context management and the asymmetric CPU extension support
feature is not available in the release.

### ⚠ BREAKING CHANGES

- **Bootloader Images**

  - remove unused plat_try_next_boot_source

    **See:** remove unused plat_try_next_boot_source ([2c303e3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c303e393befcd063df60806e5208ff09958d573))

### Resolved Issues

- **Architecture**

  - **Branch Record Buffer Extension (FEAT_BRBE)**

    - allow RME builds with BRBE ([9890eab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9890eab5743629c10a3d7432cdb89b65e11c83b8))

  - **Memory Tagging Extension2**

    - improve ENABLE_FEAT_MTE deprecation warning ([ba65e2d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ba65e2d1574954cead8b474e692eef608deff4b3))
    - remove deprecated CTX_INCLUDE_MTE_REGS/FEAT_MTE ([6f2b881](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6f2b8810f6d48bde930d4384df4b6894effcd14f))

- **Platforms**

  - **Allwinner**

    - dtb: check for correct error condition ([7300a4d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7300a4d1676f0c929f6a41810f9bc43d4e5334eb))
    - enable dtb modifications for CPU idle states to the rich OS ([188a988](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/188a9888e7b541299133a75b7632fdda2584833d))
    - remove unneeded header inclusion ([8bb8f02](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8bb8f02d44d1620de6c410f9091c2dd53814479e))

  - **Arm**

    - **FPGA**

      - avoid stripping kernel trampoline ([8292f24](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8292f240e5d3fc1391cb463d068a69803b72a9e7))

    - **FVP**

      - add DRAM memory regions that linux kernel can share ([18ec9bd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18ec9bdc2d51f0b58d24e4a6520b2922e74e7dd8))
      - add optee specific mem-size attribute ([75265a1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/75265a16c978c75c9737e03101fb4616b0aedf7e))
      - add secure uart interrupt in device region ([fc3a01a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fc3a01aac3a8c4ba2d491e77681567a2727935e3))
      - enable FEAT_MTE2 ([d081c61](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d081c6116e455732b579304268027b9cd98e50ff))
      - fix the FF-A optee manifest by adding the boot info node ([bf36351](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf36351acaa5ecef6243513d68afb083d7aba07e))
      - update the memory size allocated to optee at EL1 ([4739372](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47393722783c4cc636244388dccd9987ecf97fa9))

    - **Neoverse-RD**

      - **RD-V3**

        - remove NEED_* from RD-V3 makefile ([a3eef39](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a3eef39f45d8e82bb306045eaf4a1f3ad37592c7))

    - **TC**

      - add SCP_BL2 to RSE measured boot ([7984154](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/79841546a2782c400751bdc5a4d5f8c0263b3812))
      - add stubs for soc_css_init functions ([f5ae5dc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f5ae5dcd89497d4c5e5187137a8392d4216a5aaa))
      - correct CPU PMU binding ([7aca660](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7aca660c4e77477d81623df00fc7ffab2700dcb9))
      - correct NS timer frame ID for TC ([034cc80](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/034cc8087b249f87bfd42b99ac8553756274ee5a))
      - don't enable TZC on TC3 ([8ce29a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8ce29a74a44523ce3e56da09a7b64f415c08a20f))
      - enable MTE2 unconditionally ([be8eaa5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/be8eaa5e62d2a916c6521e1d9c17ec4698bbbb27))
      - fix the MHUv3 interrupt name in DT ([1bf3325](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1bf33251a8fe774674205df9ea0f49d55233820c))
      - retain NS timer frame ID for TC2 as 0 ([1ba0880](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1ba08807a58d977e2cbf0fec5ec49f29652ff997))

    - **Corstone-1000**

      - fix Makefile error reporting ([09bf366](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/09bf366bef9bcbf10267ec036b8de7b5b35fd58e))
      - clean cache and disable interrupt before system reset ([335c4f8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/335c4f8b301ffe0fd323a25e9995c3e0b1b8aa1d))
      - include platform header file ([783e5ab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/783e5abe94a10c9aa5c7c750ec1590f0529702fa))
      - pass spsr value explicitly ([32690ba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/32690bacb9564263f4ed23e27a1f22ba0a22bc9e))
      - remove unused NS_SHARED_RAM region ([83c11c0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83c11c0bd119ffe8f2673aa09e17e1432b226415))
      - update memory layout comments ([d7417ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d7417adc218c1386b30658e83ea8d4f3b7b72697))

  - **Aspeed**

    - **AST2700**

      - fix mpll calculate statement ([aa09622](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aa09622233a891cb04c65a5db816e0dc76110e21))

  - **HiSilicon**

    - **Poplar**

      - shutdown wdt0 before powering off ([88bc65d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/88bc65d745c0c29f4d2d9a75abe3ea45a235a719))
      - use sysctrl module to reset ([c961e68](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c961e68e7990eb802d6638bc881afa3b7068e60d))

  - **Intel**

    - add cache invalidation during BL31 initialization ([3c640c1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3c640c124ec02f3f0e6bbc5b6d364a0b851ba1ad))
    - add in JTAG ID for Linux FCS ([ea906b9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea906b9bb97fa6011ad974838266d5f82efc134d))
    - add in missing ECC register ([4683946](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4683946015365e1a6e8a7fd8c8c2c72cc6043b02))
    - add in watchdog for QSPI driver ([6704cba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6704cba25d6386469832fe82e8ec6e0fed79b0ce))
    - bridge ack timing issue causing fpga config hung ([9a402d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9a402d2f0f7e4c62c26903af1482d2f67cfa48c5))
    - correct macro naming ([815245e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/815245e4deafc375dd62aa26821059a07e7ad2b5))
    - f2sdram bridge quick write thru failed ([64cf9de](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/64cf9deb770ea7eccd5f92a013b67b492978aea0))
    - fix bridge enable and disable function ([90f5283](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/90f5283ec052f622285ef35210d4bc452e4b905a))
    - fix CCU for cache maintenance ([f06fdb1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f06fdb1469e8855e0b711ba86fde98b44f1d7736))
    - flush L1/L2/L3/Sys cache before HPS cold reset ([7ac7dad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ac7dadb551ee602299aef91043dc4adbd234a3e))
    - implement soc and lwsoc bridge control for burst speed ([a8d81d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a8d81d61e120f2e5958f996cd59ab5219a8a3cce))
    - refactor SDMMC driver for Altera products ([beba204](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/beba20403e23ab128711c2c8c9d480a3a40b804c))
    - remove redundant BIT_32 macro ([7985ade](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7985aded701cc715bff2dd247680b9d0d2ffb42c))
    - software workaround for bridge timeout ([e08039d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e08039d0e2b3ed69bf2b10592006be8008dcb398))
    - update Agilex5 BL2 init flow and other misc changes ([b3d2850](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b3d28508427225f41d55fa3b10fe4f1f1dfbd238))
    - update Agilex5 warm reset subroutines ([c1253b2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c1253b2445d6b57851118fb9cb4ee1eac9e122be))
    - update all the platforms hand-off data offset value ([1838a39](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1838a39a44a058c6fc14e045fabe433c93e609c4))
    - update CCU configuration for Agilex5 platform ([09330a4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/09330a49376306031cf92e26bbd6955ebfe87597))
    - update mailbox SDM printout message ([569a03c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/569a03c7114f4a5c005a8cf4fa1dcae2b54bec56))
    - update memcpy to memcpy_s ([e264b55](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e264b5573952c72805a14e69e438168c00163e9a))
    - update outdated code for Linux direct boot ([21a01da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21a01dac879daaded762f2feccccbdf6c07cf451))
    - update preloaded_bl33_base for legacy product ([f29765f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f29765fd337cc0a405b1ffee945bc6a5db2d7e8b))
    - update sip smc config addr for agilex5 ([7c72dfa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7c72dfac962ce1e1f95be4c974b691d667a8eae4))
    - update the size with addition 0x8000 0000 base ([9978a3f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9978a3fd8b97f024a28be798494b608f43ef5e79))

  - **Marvell**

    - **Armada**

      - **A3K**

        - reset GIC before resetting via CM3 secure coprocessor ([5993af4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5993af454fca84d1401d12eabc3c714b6b5dd953))

  - **MediaTek**

    - **MT8188**

      - remove BL32 region protection if SPD sets to none ([207c447](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/207c4470492ea5b9554051b9abaf6cc9c1a78f35))

  - **NXP**

    - **i.MX**

      - disable DRAM retention by default on i.MX8MQ ([108146c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/108146ce73573ca761fb2072efef0e0c4e4d50bb))

      - **i.MX 8M**

        - 8mq: enable imx_hab_handler ([af79981](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af799814e2639a03b3453744f06a73e77cb66e86))
        - ensure domain permissions for the console ([f7434fa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f7434fa13507b8879922bcf0c55947e9b9606404))

    - **S32G274A**

      - avoid overwriting const fields ([bf01296](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf012960d4f1490897b6a243eb89c70d6e03161f))
      - workaround for ERR051700 erratum ([b47d085](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b47d085a3bc918d51dae48fa7bb13678f3ae14ba))

  - **QEMU**

    - allocate space for GPT bitlock ([e9bcbd7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e9bcbd7b2ee43b3abc89f8e505b9fd5689f91aae))
    - exclude GPT reserve from BL32_MEM_SIZE ([7604288](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7604288577bab9a1ff02fd69e07a803b808bbfae))
    - fix build error with spmd ([1b1b40a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b1b40a941b62a845e57ca8d2bf754396b1b5dcb))
    - fix EL3-SPMC data store alignment ([eee52da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eee52dac2c3e6b7c9ac51624c6200d2201e65bc2))
    - fix L0 GPT page table mapping ([147b1a6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/147b1a6f068bc3db73d0f945137054af83c486f5))
    - remove validate_ns_entrypoint ([e5362e2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e5362e29d556df2e4238e798513f670ca3f85aad))
    - update rmmd_attest_get_platform_token() ([9248ee0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9248ee0cc413a209f93ee330a04890f873fec1ee))

  - **Raspberry Pi**

    - **Raspberry Pi 3**

      - manually populate CNTFRQ reg ([11dff59](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/11dff5994671bf3ec4f26b7ea930bd4749658aa2))
      - use correct define for GPIO reg_clr ([9876baf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9876baf180d307fe36ec846c03c05dd8a1b08d53))

  - **Rockchip**

    - add parenthesis for BITS_SHIFT macro ([901e94e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/901e94ed1a0d5e381d857e062c8b8289cfa80a48))
    - fix "unexpected token" error with clang ([52cdebb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52cdebbcc5d1fffea7af837178a712c8d02bcdde))
    - xlat: fix compatibility between v1 and v2 ([d43a2e8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d43a2e8bf4b4434cf30296cc56fdaf15321e5e8b))

  - **ST**

    - set no-pie option when building ST elf file ([6d26d75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d26d75c374bc9c7aa03d8c745b9f5f9082b18c2))
    - support device tree DDR sizes higher than 16Gbits for aarch64 ([cd9c92c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd9c92cd16b1beb6199ae7a7c01effb0d49ab448))

    - **STM32MP1**

      - remove unnecessary assert on GPIO_BANK_A value ([5c45768](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5c457689b283437cbf1ba87c48bae9e03a579aa8))
      - skip OP-TEE header check if image base is NULL ([b452e7a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b452e7a8246533a4923d54cc916bdf805f9543da))

    - **STM32MP2**

      - enable timer earlier in BL31 ([16a659d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/16a659d73a70ce16662c0e2df4097f3496d65f63))
      - remove mapping of BL2 DT area ([60d0758](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/60d0758411064ac67df22ade6dba460d31d00c81))
      - set PLAT_MAX_PWR_LVL to one ([747d85e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/747d85ee77d8d8b2e04a4988f98cb2fc426103a3))
      - use TOOL_ADD_IMG_PAYLOAD for BL31 DT ([f15f1c6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f15f1c6270d50e06eafb4202dd32326d516960f3))

  - **Xilinx**

    - avoid altering function parameters ([b21e287](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b21e2874f81633892e914f7d53b5bf0fe3b41a18))
    - dcc to support runtime console scope ([238eb54](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/238eb542bb746a776de82236dd25b7ae5876b743))
    - declare unused parameters as void ([d3bb350](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d3bb350c40d202bec31dde04911f1c50d3e71634))
    - explicitly check operators precedence ([8e9a5a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8e9a5a5150c631dec09b9fea610ca3846e0dce9c))
    - fix comment about MEM_BASE/SIZE ([1e2a5e2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1e2a5e2851072803a78a8e998dee1ff4ad5b7f9b))
    - fix logic to read ipi response ([03fa6f4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/03fa6f42502a3b6b318a9a73a228a6c751329a8f))
    - fix OVERRUN coverity violation ([e27b949](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e27b9491f39c4657727d3b1641680a7e5c09a3b4))
    - handle power down event if SGI not registered ([c3ffa4c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c3ffa4c5bae5c2be313faa015bfffdb7b46c4122))
    - map PMC_GPIO device node to interrupt for wakeup source ([692d32b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/692d32b5733b4520093ac059578b2e6c2429b80d))
    - modify conditions to have boolean type ([e223037](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e223037525ef7b2e3794733ba417cbb848907dda))
    - optimize logic to read IPI response ([02943d0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/02943d0d8d05e8a647a72eb11ac9159c6a257aa3))
    - register for idle callback ([a3b0a34](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a3b0a3422c3f2b2718a7f8b337d019f470101d4d))
    - rename variable to avoid conflict ([aba5bf9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aba5bf901d775ffbf77a5034eb91f3667758a4c1))
    - warn if reserved memory pre-exists in DT ([729477f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/729477fd86fc7c471fe44f81ed58e94d1656571f))

    - **Versal**

      - add const qualifier ([0f9f557](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0f9f5575cc2c5de913e4222c149146c149378728))
      - add external declaration ([16c611f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/16c611f8a6f6a6669265fda95115a0ade56078e7))
      - declare unused parameters as void ([ab9aab3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ab9aab38d13a0905804ab5a8480dd31828d5b3ab))
      - evaluate condition for boolean ([b39c82e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b39c82e9201255f6a396ff9a80cb2c2ec038b588))
      - explicitly check operators precedence ([0ed8b4b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ed8b4bffc31e52facf27445503ea668e7ba3dc2))
      - kernel QEMU boot is failing on versal platform ([8e5252f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8e5252f3c08d25575fbbcbb8cb4ed3a4b0c9d506))
      - modify conditions to have boolean type ([1247566](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/12475663b53f6e5ffe18343470d653cc092aca48))
      - remove check for bl32 load address ([4c9ae8a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4c9ae8ae1f266f7558c5bcc98491a4fbb69967f5))
      - variable conflicting with external linkage ([e452826](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e452826ad3aa595f720be2c2500ada2f27d3eaea))

    - **Versal NET**

      - evaluate condition for boolean ([37c46d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/37c46d85d14021fa89186d3221621658410e8720))
      - declare unused parameters as void ([06f63f4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/06f63f4b566c86209fbd13142d6c5453a6fd9c8e))
      - explicitly check operators precedence ([a4ddd24](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4ddd24f97953b6c8ad6b9dfddc240067807c502))
      - ignore the unused function return value ([aa6df8e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aa6df8ec32a48d8e57205b6bb93d4bc283d353f2))
      - modify conditions to have boolean type ([83c3c36](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83c3c36b1b2869ade53f36cfd9052e6b6a17797b))
      - remove check for bl32 load address ([c38ced2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c38ced2d279a40298cab6a4c99b046146c3a1917))
      - variable conflicting with external linkage ([4d2b4e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d2b4e4dd7ed22a41c0569f9b2b2fd5c419a8261))

    - **ZynqMP**

      - add const qualifier ([bb145c9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb145c9d9b543d9440b3b4fc48b8210df4b35ce9))
      - add external declaration ([6c08d1d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c08d1df0ccb14fb66ba081bbe57ea17b8b3bb1c))
      - declare unused parameters as void ([1c43e36](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1c43e36ac18aeaa6816a0474655d699909d616b1))
      - evaluate condition for boolean ([aaf6e76](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aaf6e7627e11b1b8616d798975e40d71d1e03c8c))
      - explicitly check operators precedence ([5b54231](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b542313f8af2373549e71266307b8fbbb8788cd))
      - handle secure SGI at EL1 for OP-TEE ([f5b2fa9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f5b2fa90e0c0324f31e72429e7a7382f49a25912))
      - ignore the unused function return value ([355ccf8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/355ccf895e5106d0f7a9b5932f73759277d1ab2a))
      - modify conditions to have boolean type ([a42e6e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a42e6e44b89fb1be1d3e97e5adc4f7288bb7e69b))
      - variable conflicting with external linkage ([eda23fa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eda23fa5aa065216d9cf86176fbb916b4841c874))

  - **AMD**

    - **Versal Gen 2**

      - add const qualifier ([a0745f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a0745f21aa0c5c869a3788e8f2c590bace11ef0b))
      - add external declaration ([17a8f41](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/17a8f41e458e662c878fc8549d7a04a49e88abac))
      - add ufs specific features support ([b9c20e5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b9c20e5d144347ca28e17df080b7ee9bf0dd9377))
      - correct the UFS clock rates ([b048601](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b048601eeeeb34fb1e7642d1ed7f18f9a51d6ae9))
      - declare unused parameters as void ([851df3c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/851df3c8915d5832d9ac1d58dc3420847cacb0a0))
      - explicitly check operators precedence ([15a9e38](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/15a9e381cdfc607e516f86adc118d036ce78aa86))
      - ospi data integrity cases are failing ([a147362](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a14736268bd5156f657286b535af5d27959dec99))
      - update check for TRANSFER_LIST macro ([7d09198](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d09198f58cefd10a9ca19305782785632ffa72a))
      - variable conflicting with external linkage ([ca39fd4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ca39fd46c1ce0203df7f797fa6bd8a4fc5336c38))

  - **Nuvoton**

    - fix MMU mapping settings ([0a1df64](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0a1df6411734d1793e06e508f27bcf95f01c703f))

- **Services**

  - **RME**

    - **RMMD**

      - continue boot if rmmd_setup fails ([fdd8a24](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fdd8a24b9892fa0e67580dc25f7e7ca0b54c870e))
      - fail gracefully if RME is not enabled ([eacbef4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eacbef4c643a5ee69828a7004abf0097b3d3f728))
      - handle RMMD manifest loading failure ([0c70781](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c707813e9e734d9a62d5cdc592e68e245f4f557))
      - ignore SMC FID when RMM image is not present ([adcd74c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/adcd74ca05fe4d7c3c047c0108cb9f136b67be49))
      - remove the assert check for RMM_BASE ([8cb9c63](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8cb9c635775b2f1c413c28ea8610dc81b6e8928f))

  - **SPM**

    - **EL3 SPMC**

      - use write_el1_ctx_timer() macro to set cntkctl_el1 value ([19082c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/19082c20d98456d147816d8ebf01f4e6721c7b12))

    - **SPMD**

      - remove spmd_handle_spmc_message ([6c378c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c378c2feffd8826542322e8d2cc53fd7f0d8252))

    - **SPM MM**

      - carve out NS buffer TZC400 region ([1922875](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/192287523350dfdc06b794ae2fbc1827ff69ab72))

  - **DRTM**

    - do cache maintenance before launching DLME ([23378ae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/23378ae0bdcdaee5764af9ebf5faed7cdb8b2737))
    - return proper values for DRTM get and set error SMCs ([5e1fa57](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5e1fa57459aa27a28bb21be5496fb471350b6046))

- **Libraries**

  - **CPU Support**

    - modify the fix for Cortex-A75 erratum 764081 ([7f152ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f152ea6856c7780424ec3e92b181d805a314f43))
    - workaround for Cortex-A720 erratum 2792132 ([b1bde25](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b1bde25ed9b302a2203a928457c91693ed7f91a7))
    - workaround for Cortex-A720 erratum 2844092 ([1214090](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/12140908a52230081f85069f0f0a400ddabf44ef))
    - workaround for Cortex-X4 erratum 2816013 ([1e4480b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1e4480bb54b0f567688cfbea2119aa703fcbb7b8))
    - workaround for Cortex-X4 erratum 2897503 ([609d08a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/609d08a86db2ddf09f98105b999d57b8e2eecc8b))
    - workaround for Cortex-X4 erratum 3076789 ([db7eb68](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/db7eb68817dad1a429a2f6518926791c47091b1c))
    - workaround for Cortex-A520(2938996) and Cortex-X4(2726228) ([4a97ff5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4a97ff5111204a18b4f72d1e1cd3d8285f16289d))

  - **EL3 Runtime**

    - correct CASSERT for cpu data size ([483dc2e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/483dc2e43e550cf5d4541a7b164b49edbaa467e6))

  - **PSCI**

    - fix parent parsing in psci_is_last_cpu_to_idle_at_pwrlvl ([01959a1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/01959a1656a08dacd1d036d0441165d52bf7563e))

  - **ROMlib**

    - prevent race condition on the build directory ([25cde5f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/25cde5f810422867bf03b2c0e8354dcee2493e8a))
    - wrap indirectly included functions ([d95d56b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d95d56bd2bfc87951f35d2badde9db336c0a6489))

  - **GPT**

    - fix GPT library fill_l1_tbl() function ([d024cce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d024cce376f01652b91ebdef286dceffc9ffb063))
    - fix RME GPT library bug ([6350aea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6350aea2f186c593ef46737f573de5e4833a9433))

  - **Translation Tables**

    - correct attribute retrieval in a RME enabled system ([e3c0869](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e3c0869f6fbd8008b556738384e3f3a22cf981c3))

  - **Authentication**

    - check the presence of the policy check function ([491832f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/491832fedf979b6b0c00c5c5411780047f106804))
    - correct RSE_CRYPTO_EXPORT_PUBLIC_KEY_SID ([759994a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/759994aa3b1ad1e54ef3a998d0685108fec6d27c))
    - remove the bl2 static c file ([ac106f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ac106f208fad311e691b69e116632239c635a81f))

    - **mbedTLS**

      - fix error return code for calc_hash ([885bd91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/885bd91f27fd31d46f33861b94a814fa4537ab5f))
      - sign verification issue with invalid Key/Signature ([7731465](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7731465252bd82ce97620a327f3b5d8905f8bdb1))
      - add extra hash config to validate ROTPK ([014975c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/014975cea46261d84a934644be2ad53bbdc0dc79))

    - **mbedTLS-PSA**

      - fix P-384 PSA key signature verification ([12a8e95](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/12a8e95303c051dc5671441a6419741db3b0964e))

  - **GUID Partition Tables Support**

    - fix unaligned access in load_mbr_header() ([21a77e0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21a77e08921a13ac4adc523a136d829333a854f1))

  - **Arm**

    - **GIC**

      - **GICv3**

        - fix GITS_CTLR.Quiescent bit definition ([2da29d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2da29d2d07cdd8c52a1c1d6f26d7d45ac11ef2be))
        - incorrect impdef power down sequence ([b1925dc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b1925dcfd97a5d77a796bee8164519b4e8254d8c))
        - wait rwp when gicr_ctrl.enablelpis from 1 to 0 ([66668c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/66668c77cb140c3af1a801b8f56b0c0ec65c4c21))

    - **MHU**

      - fix compilation error with ENABLE_ASSERTIONS=0 option ([e2e8a39](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e2e8a397f88eaedb9d3f16b6b4560eec51aee7e0))

    - **RSE**

      - include lib-psa to resolve build ([654ae70](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/654ae705c35baa1fbd13a0cd8558a64c8454347c))

  - **NXP**

    - **SFP**

      - shift gpio register offsets by 2 ([d30312a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d30312a2dcdbe7aa651f8770d9b00e6ae83baacc))

    - **Clock**

      - broken UART clock initalization ([f8490b8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f8490b85b49c92799a792587658eca4cf36fd4f6))
      - function parameter should not be modified ([8ee0fc3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8ee0fc31992538823177e764e4522293ea829957))

  - **ST**

    - **Clock**

      - adapt order of CSS on LSE and HSE ([eca5103](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eca510346d9ae7d14eea53ec01554bbde6cb2e69))
      - display proper PLL number for STM32MP13 ([039b7d4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/039b7d4673e5b39056a6c0c40204aad2b0258581))
      - do not reconfigure LSE ([f4a2bb9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f4a2bb986b43fcb1c0c8c45b5d9a93798f655453))

    - **DDR**

      - fix coverity issue in ddrphyinit ([5dd1d54](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5dd1d5447750e1be9377ae8d1c4fce2608a53a63))
      - move skipddc_dat definition ([13cc1a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/13cc1a506428398cc8cc142015dca10d24840f96))

    - **GPIO**

      - configure each GPIO mux as secure for STM32MP2 ([179a130](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/179a130aea4876c7fc89606c65b55f143724eb38))

- **Miscellaneous**

  - **DT Bindings**

    - update STM32MP2 clock and reset bindings ([8522909](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/85229098ab70dfb65905f9ad7229db6478335a00))

  - **FDTs**

    - reserved memory: detect existing region ([4248806](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/42488064e10383247d0c321fe1e7fc13eec0752c))

  - **SDEI**

    - fix a crash when attempting to bind more events than are available ([4096bd6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4096bd66c7af0a5661c7926460f2a2ca4162388d))

- **Documentation**

  - fix CPU type for mt8195 ([65ada75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/65ada7571781317f16240ee3694bd684fd3bdaf5))
  - fix the example command for doc build ([9db2b05](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9db2b059eb76eaf51af8e434904caf277b998c99))
  - point poetry readthedocs virtual env ([5383a88](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5383a88b93abead45ab3479536d1b1516d9be3f8))
  - refactor poetry dependency group ([4a29299](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4a29299f2e1640dc9f3136682b914c39930562eb))
  - replace "ARM-TF" with "TF-A" in diagrams ([c4067a9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c4067a9df6e9c478a824bd5b0ac44b84d48c9b40))

- **Build System**

  - correct feature assignment for ARM v8.8 compliance ([94ff1d9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/94ff1d98c95db491137177c2160ef1afe944ff5f))
  - ensure `$(ROT_KEY)` depends on correct directory rules ([7a95759](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7a95759f935202c1f25df10eb32c67bbd69db3c8))
  - fix incorrectly-escaped armlink preprocessor definitions ([df52e26](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/df52e2600deef3fff250d337d06f55863d1dfd76))
  - pass the PLAT option during FIP tool compilation ([40469bf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40469bf977a615400424cdcd78c350b3310ebd2f))
  - string split into two lines causing error ([4f32179](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4f321794ffaacad74258082272163a61f3db8477))


- **Tools**
  - **fiptool**

    - update the fiptool and certtool to fix POSIX build ([ccbfd01](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ccbfd01d95b9b35acb3e2ca5f25379ce8fa0ed1c))

- **Dependencies**

  - **checkpatch**

    - detect issues in commit message ([1a72174](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1a721748605bc753089bc34c6010aa236c9d0ab7))
### New Features

- **Architecture**

  - **Fine-grained Traps 2 (FEAT_FGT2).**

    - add support for FEAT_FGT2 ([33e6aaa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33e6aaacf1e8f327b33fe2db1f5e964b0adb41c7))

  - **CPU feature / ID register handling in general**

    - add ENABLE_FEAT_LS64_ACCDATA ([19d52a8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/19d52a83b755cdf6d9b7defc7eb821eb62e80310))
    - add new feature state for asymmetric features ([43d1d95](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/43d1d951ddb3b725d372884f314babb6594fcd47))
    - upgrade PMU to v8 (FEATURE_DETECTION) ([515d2d4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/515d2d46a318fa3c4c172491c6408c032e6a6b15))

  - **Debug Extension (FEAT_Debugv8p9)**

    - add support for FEAT_Debugv8p9 ([83271d5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83271d5a5aae06c23c59a32c30a0fe83fb82e79f))

  - **Statistical profiling Extension (FEAT_SPE)**

    - introduce spe_disable() function ([651fe50](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/651fe5073c790647305363a4de05cf050e0851de))

  - **Trace Buffer Extension (FEAT_TRBE)**

    - introduce trbe_disable() function ([b36e975](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b36e975ea374589270fc4010aa247e1e56432bda))

  - **Extension to SCTLR_ELx (FEAT_SCTLR2)**

    - enable FEAT_SCTLR2 for Realm world ([b17fecd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b17fecd6cf23f50346d70ec84f5708c95a2db5f8))
    - add support for FEAT_SCTLR2 ([4ec4e54](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4ec4e545c66cb888bfbedcea4030a234421457d7))

  - **128-bit Translation Tables (FEAT_D128)**

    - add support for FEAT_D128 ([3065513](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/306551362c15c3be7d118b549c7c99290716d5d6))

  - **Translation Hardening Extension (FEAT_THE)**

    - add support for FEAT_THE ([6d0433f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d0433f04045f52856ecb837efc873a5504d9fa2))

- **Platforms**

  - **Allwinner**

    - adjust H616 L2 cache size in DTB ([ee5b26f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ee5b26fd0058d5e696cdf83bf389351eab296bf7))
    - h616: add I2C PMIC support ([0444589](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/044458981f986b03445185b646bebbea1d90f11f))
    - h616: add support for AXP313 PMIC ([0385136](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/03851367dbd46f73708fa35da2b501489e44afa4))
    - h616: add support for AXP717 PMIC ([646d06b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/646d06b2378b39b8dfa713b74f936a2b02782e96))

  - **Arm**

    - **Common**

      - add support for loading CONFIG from BL2 ([973e0b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/973e0b7f2cc9ac64132b2179295c424a88b690ea))
      - add fw handoff support for RESET_TO_BL31 ([1a0ebff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1a0ebff784c11f0b11f203b56eeb3180f994c0b9))
      - correct the RESET_TO_BL31 x1 handoff arg ([5da68cc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5da68cc477adf0f686eeb9b6c8c53c1104805f24))
      - load dt before updating entry point ([c1c406a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c1c406a4de90b859a2e534304e33331ecd3dcef8))
      - move HW_CONFIG relocation into BL31 ([fe94a21](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fe94a21a6815fc8623074e7184d87583f2f58940))
      - remove critical handoff code from assert ([cca1b72](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cca1b72b3bf25dab03d3527c9fbe0f5d368382cc))
      - makefile invoke CoT dt2c ([0e0fab0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0e0fab0ca2190d75dd12b655e043ed8b6053221f))
      - generate tbbr c file CoT dt2c ([479c833](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/479c833afcfce3afebefdc8eecefea71c09f0bf1))
      - add COT_DESC_IN_DTB option for Dualroot ([731ac5e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/731ac5ea043efb333ea74c8443c10989acce5d94))

    - **FPGA**

      - enable new CPU features ([1920a32](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1920a32b7fd32c22f4cef6d948c1d0be4efce0e5))

    - **FVP**

      - change UART0-1 to NS device region ([cd656a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd656a5612e6f6942fd8fb768b5dd948efbc37ac))
      - add Cactus partition manifest for EL3 SPMC ([5134623](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/51346236c3f07fd86bf14f4743517ab1d15bd56c))
      - add cpu power control ([d38c64d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d38c64d2466006104142ae23a673a9cf2b4170e2))
      - add Dualroot CoT in DTB support ([0af86f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0af86f08ce5c39e3d53ccd9daa77084acef09fa7))
      - add flash areas for secure partition ([9fb7676](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9fb767630dbb3a54eff17b9b9b83078a7b3e77b7))
      - add SPM manifest for OP-TEE at S-EL1 without S-EL2/Hafnium ([41d73bf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/41d73bffe1cac198ef1f21149ac64f784f5ae8db))
      - allow SIMD context to be put in TZC DRAM ([b4c23ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b4c23adf58dce011ce5119cfc79f4312cea855f7))
      - fdts: add stdout-path to the Foundation FVPs ([2faccab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2faccaba80318b48e7ae738a909a38a989ed3c5e))
      - replace managed-exit with ns-interrupts-action ([887cec9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/887cec9caedb87f824f8f35adbf058e1e83b250e))
      - scale SP_MIN max size based on SRAM size ([3b5eca9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3b5eca9e7a96f7a6f3c764fb981a3b2bfe67e514))
      - update FF-A version to v1.1 supported by optee ([4f37e1e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4f37e1e8b233a2968dd32708eef0a4a44d093b7a))
      - remove duplicate jumptable entry ([180a3a9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/180a3a9ed3e0ee80f4ed4d02d671a7b0fb28db6d))

    - **Neoverse-RD**

      - add a routine to update NT_FW_CONFIG in BL31 ([c6b27c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c6b27c4916d41db9a8f6be089970fa5f79634f7c))
      - add CSS definitions for third gen platforms ([6d52713](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d5271346d38ac9899bc2f8c9fe96b32bcef05c8))
      - add DRAM layout for third gen platforms ([10eb4c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10eb4c4bee31786800a8d61ef54d68d22db97221))
      - add firmware definitions for third gen platforms ([e517ccf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e517ccf52cf9f2578d980b5340900fafe3e9a6e6))
      - add MHUv3 channels on third gen multichip platforms ([47348b1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47348b1c53c1000f7b36593aa1641240d0509947))
      - add MHUv3 doorbell channels on third gen platforms ([46d474f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/46d474fc9fc99b1d8c9e8b66514cc380ec10aa9a))
      - add multichip pas entries ([c72e9dc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c72e9dcdd872f3922eb093afbfded0dd78533cc7))
      - add pas definitions for third gen platforms ([896e9aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/896e9aa98b5cf25a4b5e9d11a58265fdb43dca1e))
      - add RoS definitions for third gen platforms ([fad5a20](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fad5a209a03ae7a893b8e93197ed6e795fe370a6))
      - add scope for RD-Fremont variants ([84973bb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/84973bb3cafeb21f7c706335570fbef41ab62179))
      - add SRAM layout for third gen platforms ([5a37d68](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5a37d68c78b0c1fcd527e2d6fbc40ecf84dc0f15))
      - allow RESET_TO_BL31 for third gen platforms ([4abcfd8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4abcfd8b2ce2fd8aad9f4de652a11a0b6a28e8dd))
      - enable RESET_TO_BL31 for RD-V3 ([527fc46](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/527fc46541b85371b01dc55e5ebc1ba92c1b6b47))

      - **RD-V3**

        - add DRAM pas entries in pas table for multichip ([6a9cf0e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a9cf0e5aaf6bc97b433e79c74cf4ba435c877b2))
        - add implementation for GPT setup ([0876c74](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0876c74285377857d34701f9279cc15b60f6ac50))
        - add support for measured boot at BL1 and BL2 ([6182950](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/61829505d2d40a1b5a3065fda53df7f6b833cdb3))
        - add support for RD-Fremont ([c0513e0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0513e0f8500d8552646f57b2a2e68113c48ad2e))
        - add support for RD-Fremont-Cfg1 ([6a0cb48](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a0cb487fd61e0c583465338bb502833803b8a5a))
        - add support for RD-Fremont-Cfg2 ([eedb2d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eedb2d820a26300314ac81773fe597938e67698e))
        - enable AMU if present on the platform ([faf98b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/faf98b3fe24926bd556b175ce07c97a63b058b45))
        - enable MPAM if present on the platform ([e951985](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e9519857d36517624f954b85b7f24f677fdc6765))
        - enable MTE2 if present on the platform ([f801377](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f80137720cbe08c2de1b130b1a4ba44af037fa1d))
        - enable SVE for SWD and NS ([7e2736b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e2736b0c1fbe5a41cd815da0b625a90f0142a57))
        - fetch attestation key and token from RSE ([0e323ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0e323ec5c4e824c113394f87d1c77103471e8123))
        - helper to initialize rse-comms with AP-RSE MHUv3 ([2a35fcd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2a35fcdd9faa056e182a43ea6e53dc529bfc4186))
        - initialize GPT on GPC SMMU block ([ba35fac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ba35fac174ae4a9d52625e709863b6c565608538))
        - initialize the rse comms driver ([f546113](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f54611376113d7c0cfdfd0eb89752040deb99aff))
        - integrate DTS files for RD-Fremont variants ([1b96641](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b966414c1a2a38a931eb4499bc209c37c4f39db))
        - update Root registers page offset for SMMUv3 ([859355f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/859355f27598da4f9ac76c0d12d1f8db4499e131))
        - set CTX_INCLUDE_SVE_REGS build flag for RD-V3 variants ([1551834](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/155183432afffa8dad4260b0dc4eeef60a8385cd))

    - **TC**

      - add default SLC policy for the gpu ([bebefe0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bebefe0f33411245325c9a25db4eb9d7cbec69fc))
      - add device tree binding for SPE ([77080f6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77080f6aaf7e1cde46a4d48a9e8eb673119dd3ff))
      - add device tree binding for TC4 ([3cedc47](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3cedc47b1d4cf46622b4b5413fab01d3224dc872))
      - add DSU PMU node for tc3 ([d3ae677](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d3ae67771d14e7ffa06793661833654681934d39))
      - add dts entries for MCN PMU nodes ([1401a42](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1401a42c950751170c5cf14106d1872160d7ecea))
      - add MHUv3 addresses between RSS and AP ([5ab7a2f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5ab7a2f2eac2b9e398d83ca2a16738f38a18baf6))
      - add MHUv3 doorbell support on TC3 ([4f65c0b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4f65c0beaad1a73e45919eb0b450a86c4f58de27))
      - add MHUv3 DT binding for TC3 ([6c069e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c069e7168445d5fa1e1a49dbfc269faa65bfa62))
      - add MHUv3 register addresses for TC4 ([36ffe3e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/36ffe3e1be3fe91e2b709b769eb4f17545f6ce04))
      - add new TC4 RoS definitions ([e9e83e9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e9e83e96bb0f7d83dd7e8eae3a3a82f391922bd9))
      - add NI-Tower PMU node for TC3 ([169eb7d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/169eb7daf248e75d40cd72a434aedc70a3d9ebdb))
      - add PPI partitions in DT binding ([ebc991b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ebc991b3a11a01142d8e4d71263c5a9a5f40db1b))
      - add system generic timer register definition for TC4 ([d6b6a8b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d6b6a8b7cc9fa872f752640a52b9a752fa50e3a8))
      - add uart node in spmc manifest ([880dcd0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/880dcd0d791288dab34f9e6668f9491796ef687a))
      - allow TARGET_VERSION=4 ([e8e1b60](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8e1b60820dcba1f2be151d296a8e81de9bed8ba))
      - bind DPU SMMU on TC4 ([e365479](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e365479d0d89999f815ea71b1511ff7952b479e2))
      - bind GPU SMMU on TC4 ([11ec5de](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/11ec5de6957206c9b1ec84b78cccf4e876688a84))
      - bind SCMI over MHUv3 for TC3 ([f2596ff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f2596ff1a8c0c3daddcd406a18224fce9af0f1fc))
      - bind SMMU-600 with the DPU on TC3 FPGA ([4c6960c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4c6960ca4040e5628874f48576170b6f8f3904a9))
      - bind SMMU-700 with DPU on TC3 ([0458d3a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0458d3acae25aa98f28bc0e0aa578fdce7ae92fa))
      - change GIC DT property 'interrupt-cells' to 4 ([1300bbc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1300bbce15308868fefda1be9ee7b4fccedde951))
      - configure MCN rdalloc and wralloc mode ([bb04d02](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb04d0232e8eeb593028aa730618be35d32a4f22))
      - enable el1 access to DSU PMU registers ([de8b9ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/de8b9cedccd652c357aff5311f8d7cb9d663514b))
      - enable Last-level cache (LLC) ([e1b76cb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e1b76cb06a70b5c3d9b46a71c26e7e889dcee91b))
      - enable MCN non-secure access to pmu counters on TC3 ([adc91a3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/adc91a3440af73e2799023117764c6e1b1fd26fb))
      - enable SME and SME2 options for TC4 ([9face21](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9face2123a5925619d54070d0a9e4e628084eff3))
      - enable trbe errata flags for Cortex-A520 and X4 ([74dc801](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/74dc801d4b284e0b3829ab8ec741e0f2c311a7c2))
      - make SPE feature asymmetric ([7754b77](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7754b770cff6fb956e0384150c1f84a1a6abc620))
      - make TCR2 feature asymmetric ([3e8a82a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3e8a82a030735c14eab0d15fa6f65d7c3f90042d))
      - move flash device to own node ([62269d4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/62269d47439e34c161f2c4990f9fdc536d82943a))
      - provide target_locality info of AP FW components ([3201faf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3201faf3563930d90a0eb2fa6fad92f65b01101e))
      - remove static memory used for fwu ([25a2fe3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/25a2fe3b74689614f73138d130ab0cae14269b51))
      - setup ni-tower non-secure access for TC3 ([89c58a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/89c58a5087f12f0e965ce8fdf946038d5799d07d))
      - specify MHU version based on platform ([04085d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/04085d6eb47b67833d0a5444c92c9856b38459f6))
      - support full-HD resolution for the FVP model ([dd5bf9c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dd5bf9c5e26ea47988cde76f916495031ecc85c9))
      - update DT for Drage GPU ([b3a4f8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b3a4f8cfcfad1df90273d0e131c2016068c57f61))

    - **Corstone-1000**

      - add multicore support for fvp ([16f4862](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/16f48623d8d398ec588a958accb037c6debb7f7b))

    - **Automotive RD**

      - **RD-1 AE**

        - add device tree files ([bb7c7e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb7c7e713074e6254955e9e64386493a7ad810f1))
        - enabling Trusted Board Boot(TBB) for RD-1 AE ([2638496](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2638496965edd80e43af71a5952e7005d1fd3e8c))
        - introduce Arm RD-1 AE platform ([f661c74](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f661c74b528f3aee6f30a28a82e8c76ab26f35f7))
        - introduce BL31 for RD-1 AE platform ([daf934c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/daf934ca918057b13fecfe949315e097ca358329))

  - **Aspeed**

    - **AST2700**

      - set up CPU clock frequency by SCU ([e3d1bbd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e3d1bbdb08f643ad54e79c678d9f8cadaf63d4ce))

  - **Intel**

    - add build option for boot source ([ef8b05f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef8b05f559a698cdeca43b3ad287d720f0c22a8a))
    - add in SHA384 authentication ([cab83c3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cab83c34871aa3d20bab81d3fca34c3d746c3db4))
    - add QSPI get devinfo mailbox cmd ([8fb1b48](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8fb1b484ac74f945eb483453b3f7e776c13b7b90))
    - clock manager PLL configuration for Agilex5 platform ([e60bedd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e60bedd5e134e2ad996a0d21a8170caec12c2dd2))
    - direct boot from TF-A to Linux for Agilex ([b5c3a3f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b5c3a3fc94b43f273332518024d4955e2c54a995))
    - enable VAB support for Intel products ([3eb5640](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3eb5640a7d9277eee80b5b31bb30230a374e0fb0))
    - pinmux and power manager config for Agilex5 platform ([94a546a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/94a546acc4d6e659f64266d93d9e74b0a2b86f4f))
    - update Agilex5 DDR and IOSSM driver ([ce21a1a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce21a1a909f2ec98f83c25dd2ed3b7fedd46c46b))
    - update BL2 platform specific functions ([fa1e92c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fa1e92c6360280447a63422b3844df5abf186577))
    - update hand-off data to include agilex5 params ([6875d82](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6875d823ede6f3668e3c176e97083dea97ab236d))

  - **MediaTek**

    - change log level from INFO to VERBOSE ([5f2f384](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f2f384890c44756c6b6d946ae675d72bdadc904))
    - configure DEV_IRQ as G1S interrupt ([240a1ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/240a1ecd1818e3098d641bd3304acda8b1744809))
    - move plat_helpers.h to the common folder ([b741293](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b741293f34e394dc544250b3bad39a148e206f6d))

    - **MT8186**

      - add common and MT8186 TRNG driver ([8c1740e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8c1740e2f260e662ed13fc04e1702c20b66d459f))

    - **MT8188**

      - add MT8188 TRNG driver ([b88d1f5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b88d1f527baa5e2666df465acb85e09a2f8c9f8b))
      - update SVP region ID and permission ([fc77c69](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fc77c69a17c6228c29113c695efc6aac1a8f6b18))
      - update SVP region ID protection flow ([e66c4ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e66c4ea8ae2c586e648e85370c1f04c0b67bbfcb))
      - update the memory usage for SCP core0 and core1 ([83112aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83112aa24f408fda256c536b0880df46726db593))

    - **MT8192**

      - update memory protect region ([7587cfd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7587cfdd96029247145d992ac042bf3af0c2f20d))

    - **MT8195**

      - update memory protect region ([4224783](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4224783f8403031fc12c340efdc87e3cda30fb22))

  - **NXP**

    - **i.MX**

      - add helper to take params from BL2 ([7eae1db](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7eae1db027149e361c84395a14115324d430aa52))

      - **i.MX 8M**

        - **i.MX 8M Nano**

          - optionally take params from BL2 ([c37a877](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c37a877e563fd3953e3ea0dc29570cbd5e13aa36))

        - **i.MX 8M Mini**

          - optionally take params from BL2 ([11d32b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/11d32b33ea3331adf31fac7fe499176a739178b1))

        - **i.MX 8M Plus**

          - optionally take params from BL2 ([3d9fea9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3d9fea941a3be346ea5382c69b06d05ca470903a))

      - **i.MX 9**

        - **i.MX93**

          - optionally take params from BL2 ([02d1813](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/02d1813e8701752ec6bb23ad0c1e68be2f4b38e4))

    - **S32G274A**

      - add ncore support ([5071f7c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5071f7c7ee0c1ef1498d71f6ac65e71014044498))
      - enable BL2 early clocks ([66af542](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/66af5425a6c28af7f426a82af4ec7ea4049aa6f2))
      - enable workaround for ERR051700 ([cc6e9b0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc6e9b01900b0f4101e012889b19ff225ff55001))
      - use s32cc clock driver ([f1e4ac5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1e4ac56b53029e67b2cb626b637a4bfe4904866))

  - **QEMU**

    - **SBSA**

      - handle the information of CPU topology ([c891b4d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c891b4d83578db25d24d2a8e3e7e419e65773ac8))

  - **Raspberry Pi**

    - **Raspberry Pi 5**

      - add PCI SMCCC support ([682607f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/682607fbd775e37fb5631508434dab9e60220c9a))

  - **Renesas**

    - **R-Car**

      - **R-Car 3**

        - populate kaslr-seed in next stage DT ([b9e34d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b9e34d14c954a9af21deb70acc4579b4494824fb))

  - **Rockchip**

    - add RK3566/RK3568 Socs support ([9fd9f1d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9fd9f1d024872b440e3906eded28037330b6f422))

    - **RK3588**

      - enable crypto function ([b833bbe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b833bbe6f088e3ee78037515d6c7c5ebb6d9a0cc))
      - support rk3588 ([e3ec6ff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e3ec6ff4b24c7daa4dfa82709c23a22829947160))
      - support SCMI for clock/reset domain ([04150fe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/04150fee44cc0dec5bbe4cce42e2b626695d6f52))

  - **ST**

    - add FWU with boot from NAND ([795a559](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/795a559bc59887543afa76f05397382befd14fb8))
    - add stm32mp_is_wakeup_from_standby() ([87cd847](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/87cd847ce5640039068993868d6f853e9035c01a))
    - manage backup partitions for NAND devices ([ae81d48](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae81d48d8366bf2d7e890741bb92262b3d3a1aaa))
    - manage BL31 FCONF load_info struct ([aa7f6cd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aa7f6cd8b363fb97efd232991eb9ccedc2316a9d))

    - **STM32MP1**

      - always boot at 650MHz ([f655922](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f65592278869951330325085cf373c3306ccab57))
      - handle DDR power supplies ([47e6231](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47e62314b6baee0e5647c903b0feeba47f804df0))

      - **STM32MP15**

        - remove OP-TEE shared mem ([8dd2a64](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8dd2a64a12b3ee47507aab4fb0294d366a5a5159))

    - **STM32MP2**

      - add BL2 boot first steps ([db77f8b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/db77f8bf227b1ffc6b282408aeccc4737cb1fc78))
      - add BL31 device tree support ([27dd11d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/27dd11dbf5a7dc3d9894e6bae9630b4e5aa36d59))
      - add defines for the PWR peripheral ([6add715](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6add715405bd92e5f5ad59da79c3a23031162544))
      - add fixed regulators support ([c3a7534](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c3a7534167b22d6a14fb0ee224bbb7b49478a479))
      - add fw-config compilation ([5af9369](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5af9369c6ce0beff681ce1548bb5d614c3a6a85e))
      - add helper to get DDRDBG base address ([2fd7b23](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2fd7b230ee8605d109167e1a6f76d87c7fb132f7))
      - add minimal support for BL31 ([03020b6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/03020b6688b459da84bdb2a3fb58c99916bfd7f7))
      - add RETRAM map/unmap capability ([52f530d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52f530d3ab9d27db653670511b238d54e212cf0f))
      - add RISAB registers description ([631c5f8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/631c5f86d5438e92e1d64e7dfdab58e92ad3e24f))
      - boot BL33 at EL1 or EL2 ([c900760](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c900760d47d9fa9833610f5b831712cec1ba2ef2))
      - disable unsupported features ([128df96](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/128df96579f4837ed9571a1843a5b842de52ed3c))
      - display CPU info ([381b2a6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/381b2a6b02ef5b0245f200b8c2d42a4a58cf88be))
      - enable DDR driver ([213a08e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/213a08eb422a69bc7c95579fadf076f5af152f49))
      - enable DDR sub-system clock ([5e0be8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5e0be8c0241e5075b34bd5b14df2df9f048715d3))
      - get chip ID ([154e6e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/154e6e62fe851b95cd17087a8cdd53bfbb39613b))
      - handle DDR power supplies ([e2d6e5e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e2d6e5e21adcf9e41a335c31d5c337c65ad0a133))
      - improve BL31 size management ([64e5a6d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/64e5a6df4638af5a5c308c9ebd4aee5a839f7e3e))
      - initialize gic and delay timer in bl31_plat_arch_setup ([77847f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77847f037df3e28ac221396f118e9fd4189b1894))
      - introduce DDR type compilation flags ([d07e946](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d07e9467d375bd414fefc86dead4a833572a166a))
      - load FW binaries to DDR ([9a0cad3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9a0cad3917e6bb76694e02fd2e099ccb564a6431))
      - load fw-config file ([a846a23](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a846a23596d97b90f203dc39aeef00c0ccd88b9d))
      - manage DDR FW via FIP ([ae84525](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae84525f44ddfe8abd66644475899fdc19893481))
      - print board info ([cdaced3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cdaced366844b80024a8871adcbc94fbe31f6f1b))

  - **Texas Instruments**

    - implement DM_MANAGED suspend ([9b7550f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9b7550f1f0caaa20acb6140211ac298e74894f22))

  - **Xilinx**

    - add feature check function for TF-A specific APIs ([9a0f5d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9a0f5d128ac70da64bc33731c4e4b29007692cc3))
    - add none console ([6d41398](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d41398382430134308a513c027b77ec70b03ae4))
    - remove PM_IOCTL and PM_QUERY_DATA APIs ([924f8ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/924f8ce2e966d2ffdb2c0f29c72cb3a68d293b45))
    - update SiP SVC version number ([c26aa08](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c26aa08bee58e81710ee9d884247fdf9b23c0022))
    - update TF-A to passthrough all PLM commands ([4661c8f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4661c8f508d3ecdb7a258c71a26f489ea1bffc21))

    - **Versal**

      - add DTB console to platform.mk ([d629db2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d629db247648acdb703d841b4d3d303506af6ff0))
      - add support for QEMU COSIM platform ([db827f9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/db827f99a0132389ab18836b9419406b45ccd11c))
      - dedicate console for boot and runtime ([d533f58](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d533f58d556e729a5705b9f1aaeac467291dc686))
      - deprecate build time arg VERSAL_PLATFORM ([09ac1ca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/09ac1ca27c6497cd1e04e108d4d927500d737991))

    - **Versal NET**

      - add DTB console to platform.mk ([d61ba95](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d61ba95eecf61b660cc5161a7e4fd68948775e39))
      - dedicate console for boot and runtime ([28ad0e0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28ad0e0209ac38711d69384da9f706f43e4cc681))
      - set lower cluster bus qos value ([c6f6202](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c6f62027afb2e888b0c5f1eccc42c23bab0885ef))

    - **ZynqMP**

      - add DTB console to platform.mk ([09a02ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/09a02ce0bd37585a85f5b3e7f8dd6d7dc82e5f14))
      - dedicate console for boot and runtime ([4557ab6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4557ab69fe371137d44f8a0ee6bb2129886ab6cd))
      - enable ENABLE_LTO flag ([19d8756](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/19d875677e368e96ca0e96ec59e0c60a092114b4))
      - move zynqmp platform to xlat tables v2 ([fdda980](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fdda980af4b8c8d59374785681a153afda8f71e2))

  - **AMD**

    - populate handoff from TL ([1fbe81f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1fbe81febd4fc69813188ceefb4cbe95a3410ed9))

    - **Versal Gen 2**

      - add dtb & runtime console ([1196474](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/11964742d6557c314b6106a8630a3317666c708f))
      - add dummy implementation for SCMI PD ([095a20a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/095a20a70ce55a08752214fc9eb46bffe4a44a21))
      - add support for AMD Versal Gen 2 platform ([c97857d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c97857dba2588ce44dd1d9907797f9f4e952fea7))
      - implement USB_SET_STATE dummy IOCTL ([282bce1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/282bce19bbdb3a95a5365a0385aecfbfa4293ae6))
      - support dynamic XLAT tables ([9aa71f4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9aa71f48bcf98c047e920a8c671b8f5c58b57b74))

- **Bootloader Images**

  - add plat handler for image loading ([a03dafe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a03dafe5164fd3ec81915c49f4e50f0f927726ea))

  - **BL32**

    - setup GPT in BL31 in RESET_TO_BL31 boot flow ([1547e5e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1547e5e66675ec11bf6dc5958d2d5cff1948cd1f))

- **Services**

  - **RME**

    - **RMMD**

      - el3 token sign during attestation ([6a88ec8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a88ec8b300ca88ba7b6ba8d9626b66a7ee87116))

  - **SPM**

    - **EL3 SPMC**

      - support simd context management upon world switch ([59bdcc5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/59bdcc58c3948cd24428c0aef7c478128b2a0bde))

    - **SPM MM**

      - switch to simd_ctx_save/restore APIs ([e6e3486](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e6e348689a4b25089145abb798fc2b2aabf6f90b))

  - **Secure Payload Dispatcher**

    - **ProvenCore**

      - switch to simd_ctx_save/restore apis ([a9b64ed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a9b64ed969edffe020e2096b5006b27373218ff6))

    - **Trusty**

      - switch to simd_ctx_save/restore apis ([7461025](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/74610259856a1df5ca7b9516e74478bb16490a95))

- **Libraries**

  - **CPU Support**

    - add support for arcadia cpu ([8fa5460](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8fa54607088314aa8e3db1da5649276f2544c75a))
    - add support for cortex-a720ae ([8118078](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8118078b71583e01a486da01f1bf369b4fde3c59))
    - add sysreg_bitfield_insert_from_gpr macro ([ad8b514](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad8b51418e3c9e19ddc957424ab19386711ba7ee))

  - **EL3 Runtime**

    - **Context Management**
      - context switch MDCR_EL3 register ([123002f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/123002f9171384d976d95935b7f566740d69cc68))
      - introduce EL3/root context ([40e5f7a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40e5f7a58f906beef74587a06f7fc35efe20537d))
      - add Root-Context documentation([0f3cd51](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0f3cd5150c8f530bb96b84b0ae8129f749835ba3))
      - enhance the cpu_context memory report ([781e1a4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/781e1a44e0cdbd1fd8bbd978a60dcc947eecf29e))
      - move mpam registers into el2 context ([7d930c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d930c7e599de10bf2418cc93a176122211e7bbb))
      - convert el1-ctx assembly offset entries to c structure ([42e35d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/42e35d2f8c0ec3b931a0da90cb0111369aecea1f))
      - add explicit context entries for ERRATA_SPECULATIVE_AT ([59b7c0a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/59b7c0a03fa8adfc9272f959bd8b4228ddd2607a))
      - remove el1 context when SPMD_SPM_AT_SEL2=1 ([a0674ab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a0674ab08192e2175afe919f929c9985adc32174))
      - support for asymmetric feature among cores ([2f41c9a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2f41c9a7be46b148d557d3d933547c6e9ad1fd40))
      - asymmetric feature support for trbe ([721249b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/721249b0c0cce9fbe60175af6ee895e2bb7a6d10))
      - handle asymmetry for FEAT_TCR2 ([f4303d0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f4303d05ead1026ce5f97f83558f15159e7d6476))
      - handle asymmetry for SPE feature ([188f8c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/188f8c4b6040a35adce6f6c15670f2af436df0c3))
      - test integrity of el1_ctx registers ([7623e08](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7623e085cb5396054b72f1ea3f02e8c7a34568b5))
      - keep actlr_el2 value in the init context ([0aa3284](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0aa3284a45ccf4405cda0bb76f6b16a33e87f222))

    - **SIMD**

      - add data struct for simd ctxt management ([841533d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/841533dd5345dfd7ab78effe1544dc72b6ec840d))
      - add routines to save, restore sve state ([6d5319a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d5319afecf62f931fe03c12f2dbc398e959c7f0))
      - add rules to rationalize simd ctxt mgmt ([3524d07](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3524d0742e6dd4e8ed9e7a11d8268a9ea2f42c6a))
      - add sve state to simd ctxt struct ([4242262](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/42422622f924b0cf636864e045e38110e97ac126))
      - introduce simd context helper APIs ([308ebfa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/308ebfa18859c89c8b630c1c130e7002095e875f))

  - **GPT**

    - change the default max GPT block size to 512MB ([01faa99](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/01faa994ceb2635a175f1d299d3b2cd7afd036c0))
    - add support for large GPT mappings ([ec0088b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ec0088bbab9335c5273e57a84b81adf2201a51db))
    - configure memory size protected by bitlock ([d766084](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d766084fc48ed83890c63a7ef773b8fff9e4ea86))

  - **C Standard Library**

    - avoid CWE-190 for GENMASK macros ([1f0b6e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1f0b6e756a6d1894f7ec8423fac18671b55c51af))
    - fix MISRA 12.2 violations for BIT32 and BIT64 macros ([0605b7e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0605b7e8af4980d4e26afc6720dcbf2644633c53))

  - **PSA**

    - introduce generic library for CCA attestation ([98d36e5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/98d36e5b02f859866da6782a8ad73b0d26d781e8))

  - **Firmware Handoff**

    - fix register convention r1/x1 value on transfer list ([7475815](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7475815f4b3697f6c61868e4ae6680baee8b93e2))
    - make tl generation flexible ([2329e22](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2329e22b8bec6fdbb1b5531f3d29569519782a63))

- **Drivers**

  - **Generic Clock**

    - add set_parent callback ([a2c6016](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a2c6016f927e4b9a23499005c63f3e46f48ff8a2))
    - add set_rate callback ([19f9e2e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/19f9e2e657918d023c9836f8330a967e97a45d7e))

  - **NXP**

    - add clock skeleton for s32cc ([3a580e9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3a580e9e472a5506da82227e809e0bd472dea1b1))
    - add Linflex flush callback ([95ac568](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/95ac568b6137ee8d3a53d3ec911a7116c90e8d5d))

    - **Clock**

      - add A53 clock objects ([44e2130](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44e2130ab9948530cd5eb3fbd1d6d8ead6336845))
      - add ARM PLL enablement ([b5101c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b5101c452e3fefdf4fe13d944372e5ad5d2ea5c4))
      - add ARM PLL ODIV enablement ([84e8208](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/84e82085a1d59624ab7dc14256a152d6d7dd15f2))
      - add CGM0 instance ([9dbca85](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9dbca85ddf0c9a7c64e4207b74c25a09fd923aba))
      - add clock objects for ARM DFS ([44ae54a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44ae54af5cadb499cb72cc0edd71711d7a2d019e))
      - add clock objects for ARM PLL ([a8be748](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a8be748a2821355734f603342b2d2cf7105f6a30))
      - add dependencies for the XBAR clock ([5692f88](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5692f881f5064f612719a4f6e7aa3a4abb827439))
      - add DFS module enablement ([4cd04c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4cd04c50eb4de7dfd65f8811331f0ed3f9f4037c))
      - add FXOSC clock enablement ([8ab3435](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8ab34357497b454b2f5e505d06ce9437da7772e4))
      - add get_parent callback ([96e069c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/96e069cb8ec72b6ac3cac0e7708749cb3fe13abb))
      - add MC_CGM clock objects ([3fa91a9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3fa91a94501ed13587132f6e2aec66a6c054c61e))
      - add MC_ME utilities ([b8c68ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b8c68ad799523229ed7c0a9d025b22f74ffe9eed))
      - add minimal set of S32CC clock ids ([086ee20](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/086ee20fe7ccb9dcbf6e9ee1ce529ae98e6cf977))
      - add objects needed for DDR clock ([4a2ca71](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4a2ca718571b3b46cd091cac50c83e9f76c5927b))
      - add oscillator clock objects ([7c36209](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7c36209b29da152cc5e98b6a141fe85d78fca84b))
      - add partition reset utilities ([11a7c54](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/11a7c54072f651512948446e432421ba7ee57469))
      - add partitions objects ([af3020e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af3020e2ae86b71a87d936bb5e7181393874d708))
      - add PERIPH PLL enablement ([8653352](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8653352ad72e0f95dfd44f2ef9d1b2406dd8dca5))
      - add set_parent callback ([12e7a2c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/12e7a2cd2f8f535dfd63834ce78e3fc248ff39f2))
      - enable the A53 clock ([7004f67](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7004f6782e0c9c7c5875b294af049cd022695cbb))
      - enable the DDR clock ([8a4f840](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8a4f840b1e13b0187b373e014ea314c3dabb122d))
      - enable the XBAR clock ([b8ad880](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b8ad8800b2b13d40a6ea1e997e6feb573744665b))
      - enable UART clock ([e4462da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e4462dae81d0674eaf07ad8fa61b25b28a209d0b))
      - implement set_rate for oscillators ([d937351](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d9373519873b11cf7d9cad57742272c80d8967e7))
      - refactor clock enablement ([5300040](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5300040bfd0acf0e839a9828a1a5341afc936e36))
      - set parent for ARM PLL and MC_CGM muxes ([83af450](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83af45042debcaf76f2f898984f1b74dedc477e1))
      - set rate for clock fixed divider ([65739db](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/65739db28bf0c0d5d4daa8735a2935681f835634))
      - set rate for clock muxes ([64e0c22](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/64e0c2260fa385bdf91d7e3471e10ab251c96644))
      - set rate for PLL divider objects ([de950ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/de950ef04f2bf71924d7ac65e86cfc0cfd97aae3))
      - set rate for PLL objects ([7ad4e23](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ad4e2312f58606ee74ac7c655a655bd85148582))
      - setup the DDR PLL ([18c2b13](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18c2b137f84fed5929ee5f21cbec9260670814a2))

  - **ST**

    - **Clock**

      - add function to restore generic timer rate ([bfe8a12](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bfe8a12eea3d51c07570cce65ea7a290db0ab9ce))
      - add STM32MP2 clock driver ([615f31f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/615f31fe40e5ebf9ecef81eb01abbe52984e093a))
      - don't gate/ungate an oscillator if it is not wired ([f2aebab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f2aebab8591ef9370159fc9ddf976599bdef6349))
      - update with new bindings ([ae1e503](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae1e503763c8bc52eba1a38e320539d61ebe2043))
      - use early traces ([1a25db1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1a25db196d8fb4da379ecea43d0d004470806ee6))

    - **DDR**

      - add STM32MP2 driver ([79629b1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/79629b1a79bd1ee254077d4e76fea05ba73b9bab))

    - **GPIO**

      - add set GPIO config API ([bfa5f61](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bfa5f61b579f9eaeead1278efc5997ddd4b5543a))

    - **ST PMIC**

      - add STPMIC2 driver ([817f42f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/817f42f07ede5ef55dab857cde4e9601e349ad75))

    - **Regulator**

      - add enable ramp-delay ([6897ae8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6897ae8d0f4bba1b147f572306782b1aa6b18666))
      - support regulator_set_voltage for fixed regulator ([156ed97](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/156ed9724f95643dd749b5ed00a7a4b92bab1c71))

    - **Reset**

      - add stm32mp2_reset driver ([f829d7d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f829d7df7e261fb8f68e21dbceab8c77ce65aedd))
      - add system reset management ([d91d10a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d91d10ab39b29339f1c98d95745ba98476fd7e46))

- **Miscellaneous**

  - **DT Bindings**

    - add missing SPIx bus clocks ([c6d50c9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c6d50c9f933a0e11c419848d30ff018d404c9a42))
    - describe ST GPIO banks and config ([deb9c86](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/deb9c864eac86b4c7a57ec5bf90d301f7f741bd0))
    - introduce Dualroot CoT DTB ([703df3a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/703df3a3ef4aafe30a3522b80ec305a9833f732d))
    - new RCC DT bindings ([52b253b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52b253bfa2b1788d30339f75cfe39bce387496f3))

  - **FDT Wrappers**

    - add function to read uint64 with default value ([bc8dfca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc8dfca64d07185304a5acfe87a039c8a6649a4c))

  - **FDTs**

    - add DDR4 files for STM32MP2 ([178aef6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/178aef6989395f956b0e149b2b33cdfc0ac2e854))

    - **STM32MP1**

      - move RNG1 to CSI to improve random generation ([d594239](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d594239d4ebf2d44521bc30ec4b59b23f08c5a36))
      - new RCC DT bindings for STM32MP1 ([4391e5e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4391e5edea930810e68d087ddeb02d06886d891d))
      - remove PLL1 settings ([66d7c8b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/66d7c8bf8ef12f3424fc6da214f9fc65d4cf82b5))
      - remove RTC clock configuration ([703a581](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/703a581e2522bffe21b421c98994dc02aed2934c))

    - **STM32MP2**

      - add BL31 info in fw-config ([a370c85](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a370c856f1f7655384f8e06f7fd84ded63838c02))
      - add clock tree for STM32MP257F-EV1 ([293a4f3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/293a4f3defe95eddaccd671783e4ff855f1d6f8b))
      - add fw-config file ([513b5cc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/513b5cc83add907f2faa8587e1d24195294c03a5))
      - add fw-config files for STM32MP257F-EV1 ([83f571e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83f571edb49e35855fa1ab277b3788354d6e707b))
      - add I2C7 pin muxing ([0a08208](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0a0820885d341cc26620c37f6c10ca478955d11f))
      - add io_policies ([53e8982](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/53e89824aa2b4107a583150d1b14b855f25cd63c))
      - add memory node ([e34839b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e34839b9a275ec9d8487875fc8ef1949a1c41665))
      - add SD-card and eMMC support on STM32MP257F-EV1 ([1dafb40](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1dafb409ba94b3b5c8caba08f691c099e5a7433d))
      - add sdmmc nodes in SoC DT file ([3879761](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3879761fc206d8b3c04f0fb48d811efc267c025f))
      - add sdmmc pins definition ([6a85f67](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a85f6710fb03474d3724667e806ab7deff84814))
      - add UART and I2C nodes for STM32MP2 ([c7cfe27](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c7cfe27a2412cceef6e1e217798d2f3fc43abded))
      - describe stpmic2 power supplies ([e974670](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e97467068a2defaea92ec6acaf76b9f416de02a1))
      - remove pins-are-numbered ([a1a50ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a1a50ef1e2f7c5aac89c65b8a7bc67b1f502f21d))
      - update STM32MP257F-EV1 DT ([f0d6dcb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f0d6dcb2bf5e3d382c908a28d1dc670b4914d366))

      - **STM32MP25**

        - add DDR power supplies ([7323c7f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7323c7f9a30391f14dca7ae0627e1a3ce32b3515))
        - add DDRCTRL and DDRPHY settings in DDR node ([56ac99a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/56ac99a04cac9f29e75153c6bf84e37d2f746f0b))

- **Documentation**

  - add DPE to RSE design doc ([e4582e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e4582e424799c6072e03d1c6244109eb069ac4bd))
  - add RMM option in build-options.rst ([1b7f51e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b7f51ea1662810dea4112a543f2309fe44fdca6))
  - add RSE provided mboot backends to the threat model ([3849d27](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3849d272e3b1317ad660df37f1501cb11827e600))
  - add STM32MP2 docs links ([21b6260](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21b6260ec8d83fc9dbbfca22ef3addcf2018da9f))
  - update mboot threat model ([07c2d18](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07c2d18f4ef6cd1ce61326e0e85d93abe8f2f4ed))

- **Build System**

  - add ability to define platform specific defaults ([1b2fb6a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b2fb6adb53de652d3fe69984731a62da122e0da))
  - add ctags recipes for indexing assembly files ([54b773e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/54b773e18336b2b01b52686799192808b5aa2751))

- **Tools**

  - **Transfer List Compiler**

    - add command gen-header ([9b05c37](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9b05c3739c44418f47c2b50980fe24651a1eed1f))
    - add host tool for static TL generation ([6ac31f3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6ac31f3e76021fed1951d8b62105e6708123f8e3))
    - add support for tox ([38487c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/38487c7fd3f337298ceb60657a6bca5f11816b56))
    - add creating transfer lists from yaml files ([3112099](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/311209934e78b1d7005ae48c95b0d45c08c1c728))
    - add option to input attr as string of flag names ([4dcbba9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4dcbba98cee2260e4c4f680f6a7fda5a98fdc7d5))
    - add option to input text instead of tag id number ([792e8e8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/792e8e896f81fff3e0d75dca5f633903fa18f55e))

  - **Chain of Trust device tree to C source file**

    - standalone CoT dt2c tool ([4274d6f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4274d6f885f9df1845d5a6a0b4145cd2f289f4bb))
    - fix various breakages ([73f7b7d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/73f7b7ddbe9c86520c47a9ceb9dc95f224aa0bc6))
    - use processed Device Tree source file as input ([e19977d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e19977d664027bb16324b1b5e1aaa0ca097e637b))
    - update documentation for cot-dt2c ([b95f398](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b95f398ebd58785f29b96d94d14aec1301f42355))


## [2.11.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.10.0..refs/tags/v2.11.0) (2024-05-17)

### ⚠ BREAKING CHANGES

- **Architecture**

  - **Memory Tagging Extension2**

    - Any platform or downstream code trying to use
      SCR_EL3.ATA bit(26) will see failures as this is now moved to be
      used only with FEAT_MTE2 with
      commit@ef0d0e5478a3f19cbe70a378b9b184036db38fe2

      **See:** remove mte, mte_perm ([c282384](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c282384dbb45b6185b4aba14efebbad110d18e49))

- **Services**

  - **SPM**

    - **SPMD**

      - Given the optimizations made in TF-A SPMD to simplify NS EL1 context
        management, platform integrators must use SPMC binaries built by
        picking commits after 2fc6dcfa97e05159f95859fcf68db3031586f8c7 from
        hafnium repository.

        **See:** skip NS EL1 context save & restore operations ([2d960a1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d960a11601be6e7f24c38d84b2a4fdbb52efb9b))

- **Drivers**

  - **Arm**

    - **RSE**

      - remove PLAT_RSS_NOT_SUPPORTED build option

        **See:** remove PLAT_RSS_NOT_SUPPORTED build option ([878354a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/878354a845cbc51c198b879d3d92ed472e21889c))

  - **FWU**

    - add a config flag for including image info in the FWU metadata ([11d05a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/11d05a77295885f27530cf07029ebc2b36f49918))
    - add a function to obtain an alternate FWU bank to boot ([26aab79](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/26aab79560a2281c4207b01102495459c2bddefc))
    - add some sanity checks for the FWU metadata ([d2566cf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d2566cfb896672ea07c31c37e7acd9ef77abc4fb))
    - document the config flag for including image info in the FWU metadata ([7ae1619](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ae16196cc73a580f298734bb98f2ccb210e3ba9))
    - migrate FWU metadata structure to version 2 ([a89d58b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a89d58bb204c00db260225859bce0b55aa5e2385))

### New Features

- **Architecture**

  - **CPU feature / ID register handling in general**

    - add cortex-a35 l2 extended control register ([a727d59](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a727d59d9c1ef5ecf2f221ce289506da2011dda1))
    - add feature detection for FEAT_CSV2_3 ([30019d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/30019d8698b219d4a642dc59e7178006f59654ff))
    - added few helper functions ([30f05b4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/30f05b4f5db605ddc1a3ca0ae0cbd13ed0e728b6))

  - **DynamIQ Shared Unit (DSU)**

    - save/restore DSU PMU register ([f99a69c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f99a69c386ce5448edfc47eaf146d1a20ac8216e))

  - **Memory Tagging Extension2**

    - add mte2 feat ([8e39788](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8e3978899a481484d8c60bf276be503aebd43afb))

- **Platforms**

  - update SZ_* macros ([6d511a8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d511a8c31f0d792695566ae75c8f7b08b3b7236))

  - **Arm**

    - add COT_DESC_IN_DTB option for CCA CoT ([b76a43c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b76a43c9382e85969cac896cd4d5d6774d0d1553))
    - add trusty_sp_fw_config build option ([0686a01](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0686a01b0cacb9aab840a5c334409b5739a95a97))
    - move GPT setup to common BL source ([341df6a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/341df6af6eb911ffd175e129f61fc59efcf9fcea))
    - retrieve GPT related data from platform ([86e4859](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/86e4859a05614b40ff3cf38f8bd4efc856c546fe))
    - support FW handoff b/w BL1 & BL2 ([9c11ed7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9c11ed7e3e5536ad1fcb9190560e0368da9c5ab5))
    - support FW handoff b/w BL2 & BL31 ([a5566f6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a5566f65fd1be689ca5c63baa1f5b61b40960c8d))
    - add platform API that gets cluster ID ([e6ae019](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e6ae019a84c4d2ad2d2825b32fbcbe304752e3ae))

    - **CSS**

      - initialise generic timer early in the boot ([3447ba1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3447ba1f0405a8590ec31e4b79737efe151c3d5b))

    - **FVP**

      - add CCA CoT in DTB support ([4c79b86](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4c79b86ed6a36b572cf9e96f0269eb5dd0b46d5f))
      - add stdout-path ([8c30a0c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8c30a0c7fe0162de0618b26fb34cc91ea582e5f7))
      - add support for virto-net, virtio-9p and virtio-rng ([51b8b9c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/51b8b9c3c46cec87ebb7b484727c80ff29d73057))
      - added calls to unprotect/protect memory ([6873088](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6873088c2cd6983025b6777d4c3bde912eade571))
      - delegate FFH RAS handling to SP ([d07d4d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d07d4d63374b0d155b9281f9fcaf6b44f18117c8))
      - remove left-over RSS usage ([a1726fa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a1726fa7ffecdcc8f8f4d09bd0bdc97ef3b72f11))

    - **Neoverse-RD**

      - add scope for RD-V1 ([86a4949](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/86a4949fd012a9912c8bf909d14e20657bba2240))
      - add scope for RD-V1-MC ([6fb16da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6fb16dac6e6672040ec80f85f2f337f52cf3f3d3))
      - add scope for SGI-575 ([18b5070](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18b50707f7732a8b3deb46d8d011566199711c0b))
      - disable SPMD_SPM_AT_SEL2 for A75/V1/N1 platforms ([b9c3273](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b9c32730e5b7efe5170ed3c0dda7ab9db397c478))
      - disable SPMD_SPM_AT_SEL2 for N2/V2 platforms ([301c017](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/301c01748ea717d0f2cf3ba1f0a2fe389b6fb155))
      - enable AMU if supported by the platform ([fed9368](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fed9368529e5bc2c9111ac5a743688166661fd8f))
      - remove unused SGI_PLAT build-option ([2d32517](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d32517ce64886f154c6d509f80d0fcde05dc498))

      - **SGI-575**

        - remove SGI-575 from deprecated list ([f104eec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f104eecdea209af87de43c62811a0a9456f2838c))

      - **RD-E1-Edge**

        - remove support for RD-E1-Edge ([c69253c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c69253cc3ad3063380c8f905125fe85f6d942d09))

      - **RD-N1-Edge**

        - remove RD-N1-Edge from deprecated list ([78b7939](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/78b793956f3a86a3dd62394c858ae9ee41379b8b))

      - **RD-N2**

        - enable NEOVERSE_Nx_EXTERNAL_LLC flag ([ab2b363](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ab2b3632171dd5488952ba3f68693e490857e9dc))
        - add dts for secure partition ([49df726](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49df7261be44d5199a930c95667edb6b878355d1))
        - enable AMU if present on the platform ([2cfedfa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2cfedfad9c2c59316adf17d4f0ee561b50a041b6))
        - enable MTE2 if present on the platform ([3a5b375](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3a5b3753033561cb5d7cd7aace634cc66eab0fa7))
        - update power message value to 0 ([08f6398](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/08f6398b2b9566812cd110498e3135dfc2e3e494))

    - **TC**

      - add arm_ffa node in dts ([4fc4e9c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4fc4e9c969930d83f1144441199301d3b4b34a5a))
      - add DPE backend to the measured boot framework ([e7f1181](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e7f1181f8a7729acb07ebac86944e36932bcd09e))
      - add DPE context handle node to device tree ([1f47a71](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1f47a7133f7fe7fb038aca97fc93533964b2b429))
      - add dummy TRNG support to be able to boot pVMs ([7be391d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7be391d1ce5683c717fcf2be584f3d294ebc2bf3))
      - add firmware update secure partition ([d062872](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d0628728a627ee11c97839640d404221a74c3a65))
      - add memory node in the device tree ([5ee4deb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5ee4deb8e69175f57fa51519ef37e3674aa6b9a0))
      - add PMU entry ([553b06b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/553b06b5d4f7ec8e49796e0ffdf081bf5cf30d53))
      - add RSS SDS region right after SCMI payload ([6f503e0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6f503e0eea23a2663ed5cbfe9b925e1e0d65c236))
      - add save/restore DSU PMU register support ([b87d7ab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b87d7ab13f4b03f872c3c4a3dd7c755baf3a38d3))
      - add SCMI power domain and IOMMU toggles ([a658b46](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a658b46dc74ceaa51d119bd7bd9eccdefb0cc455))
      - add spmc manifest with trusty sp ([ba197f5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ba197f5f708fe8e033971c6f4d5b25f6783aaa45))
      - add TC3 platform definitions ([62320dc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/62320dc4fd2c13d9f4b227fe73cad2a79bdba42c))
      - allow booting from DRAM ([18f754a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18f754a275083ea66823b1c9f39e234cf430140e))
      - choose the DPU address and irq based on the target ([8e94163](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8e94163ec041f2d7df41c2dfd8625c06655ba08e))
      - enable gpu/dpu scmi power domain and also gpu perf domain ([127eabe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/127eabeddfc4fb596a1b499fe68ee6f7e5b5b6d5))
      - factor in FVP/FPGA differences ([1b8ed09](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b8ed0993fc5c04f76d949df7e2851e67040bbf9))
      - get the parent component provided DPE context_handle ([467bdf2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/467bdf26b64a38cfbfb3bf8ab915eb97eb6b3037))
      - group components into certificates ([6df8d76](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6df8d7647dad5c347d363554d25e590d24eb05e5))
      - interrupt numbers for `smmu_700` ([2c406dd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c406ddaf700e0f1c80535e309a2245b9e0bee92))
      - introduce an FPGA subvariant and TC3 CPUs ([a02bb36](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a02bb36caa521259ae57a904dedb7fd4e6a51340))
      - pass the DTB address to BL33 in R0 ([638e4a9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/638e4a92d80346b4d46ef2cc5fbb7941d1b7fd31))
      - provide a mock mbedtls-random generation function ([a877818](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a8778185d2fd2b80cee8af7879ecb92be1aa3898))
      - share DPE context handle with child component ([03d388d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/03d388d8e3eb5c6cce65afba060a16fae83d4d12))

  - **Intel**

    - add in QSPI ECC for Linux ([4d122e5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d122e5f199ad1531650ae11de5121057cfc0855))
    - enable query of fip offset on RSU ([6cbe2c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6cbe2c5d19c4af0ba6bbba049962bf55454da8bb))
    - enable SDMMC frontdoor load for ATF->Linux ([32a87d4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/32a87d440087e0a71765a61ec341af7cfcfbda97))
    - increase bl2 size limit ([2d46b2e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d46b2e46189120b6779cd27ec6bd6ec9901f72c))
    - restructure watchdog ([47ca43b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47ca43bcb4565a992bf527f68e1ff60fc036fd12))
    - support QSPI ECC Linux for Agilex ([d6ae69c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d6ae69c8c69016d05d64752538aad53f319b88a2))
    - support QSPI ECC Linux for N5X ([6cf16b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6cf16b36821b9f2a60ed9abbaa593ef62b8b9f2b))
    - support QSPI ECC Linux for Stratix10 ([8be16e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8be16e44cf0143e8651090d80bd14194aa78b1f2))
    - support query of fip offset using RSU ([62be2a1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/62be2a1ae3efcba0bb8b7ec8ef73b2a0f5a437e3))
    - support SDM mailbox safe inject seu error for Linux ([fffcb25](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fffcb25c3c2171624c582d92173154f570708a9a))
    - support wipe DDR after calibration ([68bb3e8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68bb3e836e93b271f9f1c05787025dd3f04dd788))

  - **MediaTek**

    - remove bl32 flag for mtk_bl ([9c41cc1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9c41cc182dd7acf541565ab3df7a4261fb7eaf1b))

    - **MT8188**

      - add secure iommu support ([5fb5ff5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5fb5ff5694c1bcf0ddfc972600b69d7494ca6645))
      - remove apusys kernel handler usage constraints ([0c77651](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c77651fb47c7ffd4b1b37a74aea77373179ab5d))

  - **NXP**

    - **i.MX**

      - **i.MX 8M**

        - add 3600 MTps DDR PLL rate ([f1bb459](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1bb459c3192eb6b3fc6b9b77658d82227eae2d5))
        - add defines for csu_sa access security ([81de503](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/81de50372c9192098118fc8bddaf086a620add87))
        - add imx csu_sa enum type defines for imx8m ([2ac4909](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2ac4909a5ec0a50a75cab9bb587fb1b8e592794d))
        - make bl33 start configurable via PRELOADED_BL33_BASE ([9260a8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9260a8c818aadbf513b2744cad978c18d0f65a8e))
        - obtain boot image set for imx8mn/mp ([6d2c502](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d2c502afb845e7af94c610ab5a375b868c885ba))

        - **i.MX 8M Mini**

          - restrict peripheral access to secure world ([1156c76](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1156c76361c170c83c6b9a9dd7c22aa401a4ce2e))
          - set and lock almost all peripherals as non-secure ([f4b11e5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f4b11e59b81af3e485e6992b10b50b362902eee1))

        - **i.MX 8M Plus**

          - restrict peripheral access to secure world ([0324081](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0324081af0105af536992c8ced2caa5a1928010f))
          - set and lock almost all peripherals as non-secure ([cba7daa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cba7daa10576684670e06d05ff02888a5b4f16bf))

        - **i.MX 8Q**

          - detect console base address during runtime ([52ee817](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52ee8173041c46aafcfa43f004029dddbfa9f9b5))

      - **i.MX 8ULP**

        - add a flag check for the ddr status ([4fafccb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4fafccb9a8f7b35406b08743f6d9c9b519b01c61))
        - add APD power down mode(PD) support in system suspend ([478af8d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/478af8d3c34576793a820733ddba6449c2cf2fac))
        - add i.MX8ULP basic support ([fcd41e8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fcd41e8692ce8e8fc98d069bc131820cbf83c55c))
        - add memory region policy ([5fd0642](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5fd06421f8bf9f5b67e73828281534f14f302630))
        - add OPTEE support ([e7b82a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e7b82a7d2fa1fc3f32724e6836b8f6078d20c103))
        - add some delay before cmc1 access ([c514d3c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c514d3cfa7640313c4d78674df9d7cbe9227420b))
        - add system power off support ([891c547](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/891c547e9658c1827559d8da5e3b87de5a2e9f6a))
        - add the basic support for idle & system suspned ([daa4478](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/daa4478a3cb2f86501c37e5a301cd4d6a6e60ee6))
        - add the initial XRDC support ([ac5d69b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ac5d69b628736f66f72e99532656105fdc07a3fe))
        - add trusty support ([e853041](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e853041920b15b77839027ab802d0cd9a08c7c35))
        - adjust the dram mapped region ([8d50c91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8d50c91b476474cc403c30eb6de6af28cb246e5a))
        - adjust the voltage when sys dvfs enabled ([416c443](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/416c4433f0047a86165e450e60f93020c561151b))
        - allocated caam did for the non secure world ([7c5eedc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7c5eedca4c7f176448e6b92eb5c22ee2ea45e70a))
        - allow RTD to reset APD through MU ([ea1f7a2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea1f7a2e109181f19f5bdeb71533e7dfda753df7))
        - ddrc switch auto low power and software interface ([ee25e6a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ee25e6a51bf20c92471e737ccba98af4a74d1383))
        - enable 512KB cache after resume on imx8ulp ([bcca70b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bcca70b9688c5effa0731f39e2b209071f54be2c))
        - enable the DDR frequency scaling support ([caee273](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/caee2733ba4e7a09ea656b0be85f150a275cc57c))
        - give HIFI4 DSP access to more resources ([351976b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/351976bb063cca7866e214a6bda9302f9ab018b3))
        - not power off LPAV PD when LPAV owner is RTD ([ab787db](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ab787dba7726bdf58c15626e5cc9a3525aade8a3))
        - protect TEE region for secure access only ([ff5e179](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff5e1793b95ed4297deae72cdb665178e6e72e44))
        - update the upower config for power optimization ([36af80c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/36af80c2b420cb32ff57273eda0d7d0e93b49153))
        - update XRDC for ELE to access DDR with CA35 DID ([d159c00](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d159c00532afe50686dd92215de9b420d60502f6))

    - **S32G274A**

      - add S32G274ARDB2 board support ([8b81a39](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8b81a39e28a087e1123271a42c04a7ce3b496a58))
      - enable BL31 stage ([e73c3c3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e73c3c3a6cbc1e81de4c9d73a5d713e6b37ae3b2))

  - **QEMU**

    - allow ARM_ARCH_MAJOR/MINOR override ([e769f83](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e769f830d3116f49ed82769d9d731c4dca8f6188))
    - enable FEAT_ECV when present ([1b694c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b694c77c497cb8272c97417ef1fa4f5f9c869c1))
    - enable transfer list to BL31/32 ([305825b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/305825b490a77e5b0ee816ea29c53bc6444a1d63))
    - load and run RMM image ([8ffe0b2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8ffe0b2edea6b00c9fe7d9ecaeca43c734d3764d))
    - setup Granule Protection Table ([6cd113f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6cd113fe06fdaa67a8457391eb6bcffd295f87fd))
    - setup memory map for RME ([cd75693](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd75693f5ed303c1366fdff9b392d766848b6b67))
    - support TRP for RME ([ebe82a3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ebe82a392f06aa0adddf9cc5caa7af8f561b2fb4))
    - update mapping types for RME ([a5ab1ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a5ab1ef7febb2dc931cd8f7fcd76caac04d628cd))
    - update to manifest v0.3 ([762a1c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/762a1c44b985b71495a90bc3484b576d28c8511a))
    - use mock attestation functions for RME ([c69e95e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c69e95eed0491b481971b48f5df855402ed5392a))

    - **SBSA**

      - handle CPU information ([42925c1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/42925c15bee09162c6dfc8c2204843ffac6201c1))
      - handle memory information ([8b7dd83](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8b7dd8397dd017b61ecda8447e8956a1d9d6d5d3))
      - mpidr needs to be present ([4fc54c9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4fc54c99d08926c2d42173902c8aaf3862722c84))

  - **Raspberry Pi**

    - add Raspberry Pi 5 support ([f834b64](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f834b64f889c1c4e03e590d44a6a52e3ac79cf42))

  - **Renesas**

    - **R-Car**

      - **R-Car 3**

        - add cache operations to boot process ([7e06b06](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e06b06753b12d567b6f48b6e60d6d0a56cf72e5))
        - change CAM setting to improve bus latency of R-Car Gen3 ([e366f8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e366f8cf3349189daafb7ac2ab74d98931757a60))
        - change MMU configurations ([5e8c2d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5e8c2d8e23ca0760bca7e5b692ee95dd2871ec89))
        - enable the stack protection ([cfa466a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cfa466ab733ff021771b94b4a98d22bfdd246139))
        - update IPL and Secure Monitor Rev.4.0.0 ([516a98e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/516a98ef277626aa1858d9a4018d13ab2aeb39e7))

  - **ST**

    - add a function to clear the FWU trial state counter ([6e99fee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e99fee43efa256bdac3b38864206c94bd9ae3c8))
    - add logic to boot the platform from an alternate bank ([6166051](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6166051426638087b5433eff1739d26478313dff))
    - do not directly call BSEC functions in common code ([3007c72](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3007c72844c72e0911721e499dbab37b3eca1cdc))
    - get the state of the active bank directly ([588b01b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/588b01b5e4726cd4a6d235e9f566a546ef17f631))
    - use stm32_get_otp_value_from_idx() in BL31 ([189db94](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/189db9486ddd949f279faa970bfc1dd9cc0e3623))

    - **STM32MP1**

      - only fuse monotonic counter on closed devices ([d6bb94f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d6bb94f3a14ddbcf44c667134ed302eff054954c))

    - **STM32MP2**

      - add BSEC and OTP support ([197ac78](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/197ac780d73c3421c4643e0bc02d112ceffd248f))
      - add ddr-fw parameter for fiptool ([e494afc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e494afc05f8562455e09b4f131f2699990a744f8))
      - add plat_my_core_pos ([d1c85da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d1c85da8ef23a99387823272b03399a07e3a00da))
      - add STM32MP_USB_PROGRAMMER compilation ([2e905c0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e905c0682b4e6d2cfdbd42e41f6097b16967ff5))
      - put back core 1 in wfi after debugger's halt ([2331a34](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2331a34f783b29a9a1fe86f5142d0a359cacb259))
      - use early traces ([47ea303](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47ea303389f6d0ac81617366973ece9d93dc49c9))

  - **Xilinx**

    - add handler for power down req sgi irq ([ade92a6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ade92a64e4d2fbb5f246e6ad891465d10e0d9b26))
    - add new state to identify cpu power down ([5949701](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5949701600c7f3c3a6589d0efd743615156c34b6))
    - add wrapper to handle cpu power down req ([3dd118c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3dd118cf9d60e1eab97af505eb63a2cdc044d747))
    - power down all cores on receiving cpu pwrdwn req ([c3280df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c3280df1bb95ed09b5d5f91f8977bbe99c6a923b))
    - request cpu power down from reset ([88ee081](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/88ee0816a7429689890659f69b895ac84e48f141))
    - send SGI to mailbox driver ([9a7f892](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9a7f892e29ea81c67f6f6b1342a367234e125b63))

    - **Versal**

      - enable errata management feature ([d766f99](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d766f994d2bd00c538f66e95686fc47b45ccbdb9))
      - extend platform address space sizes ([663f024](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/663f024f207bddb7b80167e661c094d77955e292))

      - **Versal NET**

        - add bufferless IPI Support ([511e4a4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/511e4a48ccd5e74af338041be238f5df12fffe3e))

    - **ZynqMP**

      - remove unused pm_get_proc_by_node() ([b03ba48](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b03ba4801d39da1d5acc7a58d9c7736e57efc099))

- **Bootloader Images**

  - **BL32**

    - create an sp_min_setup function ([a1255c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a1255c758593f9f6fb85b70165fad21de7491e1e))

- **Services**

  - **FF-A**

    - update FF-A version to v1.2 ([e830e4c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e830e4cdee3d2238314326ef8c259b35d1c4f167))

  - **RME**

    - build TF-A with ENABLE_RME for Armv9.2 ([7d5fc98](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d5fc98f5483efb942f7cbe4c04bf546a9a8598c))
    - pass console info via RMM-EL3 ifc ([3290447](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/32904472cc55a4bc9d8181a389ce3419033e0101))

  - **SPM**

    - **EL3 SPMC**

      - add support for FFA_CONSOLE_LOG ([638a6f8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/638a6f8e04c543649369374492524f2952f8d6b6))
      - add support for FFA_MEM_PERM_GET and SET ABIs ([1f6b2b2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1f6b2b26535d5254d998239f232d997972d0475b))
      - add support to handle power mgmt calls for s-el0 sp ([5917379](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/59173793f47e27a66c871a0e8237e0f0d462080d))
      - add support to map S-EL0 SP device regions ([727ab1c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/727ab1c4ab1e5ce1559fa6efec510114ce51fdf8))
      - add support to map S-EL0 SP memory regions ([83c3da7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83c3da7711a246e04f4d0a64593fc0ab46f08bad))
      - add support to setup S-EL0 context ([48db2b0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/48db2b0120d1726208ff38a0edf6962f55a988bf))
      - synchronize access to the s-el0 sp context ([5ed8e25](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5ed8e255096bd34d12bc6621e48cf9139bf414b2))

    - **SPMD**

      - add FFA_MSG_SEND_DIR_REQ2 ([cc6047b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc6047b3de52e412988f321723f67077a409e27d))
      - add FFA_MSG_SEND_DIR_RESP2 ([0651b7b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0651b7beb7e08a01c6e28be61026b053d53308fa))
      - initialize SCR_EL3.EEL2 bit at RESET ([8815cda](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8815cdaf57806901cfd388b8ee8c7979a8a2fe15))
      - pass SMCCCv1.3 SVE hint to lower EL ([c925867](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c925867ec1be039abb72a7d65bff1b6a85b3d67a))

  - **DRTM**

    - add ACPI table region size to the DLME header ([5dde96b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5dde96b02490829d023b37931737c2ba2a6ed431))
    - add additional return codes ([89f5c75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/89f5c753af8e5b8091543e8b1cae4d37e345ed7f))
    - for TPM features fw hash algorithm should be 16-bits ([c86cfa3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c86cfa35975542d25d2192b81908074195aafe96))
    - update DRTM version to 1.0 ([9c36b90](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9c36b900f904642f41e201024df584c0eaef9fc5))
    - update references to DRTM beta0 ([b94d590](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b94d59099f0addb32389952dc6ecf35136a23859))
    - update return code if secondary PE is not off ([bc9064a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc9064ae5c983aaca56102c2c0d3513ed022fd46))

  - **ChromeOS**

    - add ChromeOS widevine SMC handler ([b22e689](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b22e6898e1493eb00d0f0de6d48655d744264cb6))

- **Libraries**

  - **CPU Support**

    - add support for Poseidon V CPU ([b77f55d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b77f55d6c7e51025d6c7ada1b4aa9506a046cf0f))
    - support to update External LLC presence in Neoverse N3 ([6fbc98b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6fbc98b15d92d881c4fbb74fd1344f0ef3f128ad))
    - support to update External LLC presence in Neoverse V2 ([6aa5d1b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6aa5d1b3ab7b29c85ffe05942f2991da869e7fed))

  - **EL3 Runtime**

    - introduce UNDEF injection to lower EL ([3c789bf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3c789bfccca548ebcbdafbc7ecb07461d9368bea))

  - **FCONF**

    - support signing-key in root cert node ([04ac0b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/04ac0b3c2711a4cb2f35983e91ff0ee842b52bbd))

  - **OP-TEE**

    - enable transfer list in opteed ([0e8def9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0e8def996e73673d3e2c3d755a84e2b759ab3052))

  - **PSCI**

    - add psci_do_manage_extensions API ([160e843](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/160e8434baa48cc19d69913b00d2a643c788caec))

  - **GPT**

    - validate CRC of GPT partition entries ([7a9e9f6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7a9e9f6e96a93617abd33ef48734b65ad792ec13))

  - **SMCCC**

    - add vendor specific el3 id ([be5b1e2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/be5b1e22346c6d8ce4b0c56604c99f7a9d3676cc))
    - add vendor-specific el3 service ([de6b79d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/de6b79d8b5e15262b328051095e15ad4c67518eb))
    - add version FID for PMF ([42cbefc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/42cbefc72721a9cbf68a70d81cbcb141a2d085f1))

  - **C Standard Library**

    - add printf support for space padding ([0926d2d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0926d2df7a5606c2b7c341d51f04a396084c39f2))

  - **Locks**

    - add bitlock ([222f885](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/222f885df38c3abd34ee239a721654155609631b))

  - **DICE Protection Environment (Experimental)**

    - add cert_id argument to dpe_derive_context() ([6a415bd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a415bd1e71ac944c0ac67507b01f251e63361c3))
    - add client API for DICE Protection Environment ([b03fe8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b03fe8c025f1c8025e70e7289339ecbc6cf83aae))
    - add DPE driver to measured boot ([0ae9c63](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ae9c631eaa32a30df3ff10cb4f0abafccb6c409))
    - add QCBOR library as a dependency of DPE ([c19977b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c19977be0c3654e12accd51d4aef7059411106a6))
    - add typedefs from the Open DICE repo ([584052c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/584052c7f80b406666b9597447eeccef4d6deca4))

  - **Context Management**

    - report context memory usage ([bfef8b9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bfef8b908e3a3cc29656c1d30a6b53490c79539b))
    - add documentation for context management library ([4efd219](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4efd2193621ab7b933f4edfa28888379f3e03cbd))

  - **Firmware Handoff**

    - add additional TE tags ([a312bfb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a312bfb34487774a0e3244266ee45f63af86e2e8))
    - add support for RESET_TO_BL2 ([f019c80](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f019c8013e9c5efeb85eec7792fe901543a5832c))
    - add TE's for BL1 handoff interface ([0646c9b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0646c9b293a2d8cdfd4626d15395385b5c1c2a6c))
    - add TL source files to BL1 ([469b1d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/469b1d8412a748819f8c1bf51f695f2cb9f20489))
    - enhance transfer list library ([40fd755](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40fd755bad9411d1e9e55984107186dde4137635))

- **Drivers**

  - **Authentication**

    - add explicit entries for key OIDs ([2b53106](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2b53106a0e91e0865bf855935de04b24ef1cfa02))

    - **mbedTLS**

      - update config for 3.6.0 ([55aed7d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/55aed7d798f3d48d6aa08d58eb46c4cda318bcfb))

  - **Console**

    - introduce EARLY_CONSOLE ([ae770fe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae770fedf459d5643125d29f48659e3e936ebd2d))

  - **FWU**

    - modify the check for getting the FWU bank's state ([56724d0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/56724d09c2c55ee2b8486b7c706f5fb9d980df88))
    - update the URL links for the FWU specification ([e106a78](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e106a78ef00df4c70a1594a89520af07b939cd92))

  - **SCMI**

    - add scmi sensor support ([e63819f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e63819f2bc307e7a42d43151242009f91ceeb06b))

  - **Arm**

    - **SMMU**

      - fix to perform INV_ALL before enabling GPC ([70d849c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/70d849c14de99e7320cc381b441af8bfe2a38375))
      - separate out smmuv3_security_init from smmuv3_init ([a23710b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a23710b4b943a15a418a5d41236b2b57bd071de6))

    - **MHU**

      - add MHUv3 doorbell driver ([bc17476](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc174764f0daa82128bf60163653fc20db9a7e87))
      - add MHUv3 wrapper APIs for RSS comm driver ([4b4f850](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4b4f8505e7c58ba80a00c47a11f5feaf6d6f44f2))
      - use compile flag to choose mhu version ([996b3af](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/996b3af84cc6aeca90bc0dd3559abffd8bdc0ed7))

    - **RSE**

      - add defines for 'type' range and use them in psa_call() ([002b106](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/002b10604ba0b90ac6e85d445ce2184cab52e39b))
      - adjust parameter packing to match TF-M changes ([5abcc83](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5abcc83991770a2fdbcb57dfc01000c6354da915))

  - **NXP**

    - add Linflex driver ([306946b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/306946b01490cfe0675300412cf738840bd099ef))

  - **ST**

    - **BSEC**

      - add driver for the new IP version BSEC3 ([ae6542f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae6542f6c7ac9224843448424d3a539733bd651b))
      - use early traces ([cf237f8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cf237f8d55255da1aad4f8dccb3110bab6060eba))

    - **Clock**

      - add function to control MCU subsystem ([77b4ca0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77b4ca0b2fd2c35e3bcb516078e1d9e3573172b3))

    - **SDMMC2**

      - set FIFO size to 1024 on STM32MP25 ([d5b4d5d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d5b4d5d2e62e57acdcb2dbbcd4fe208bde92dc4c))

- **Miscellaneous**

  - **AArch64**

    - add functions for TLBI RPALOS ([8754cc5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8754cc5d1c1b33d645b321f465bcfe61bc3915d6))

  - **DT Bindings**

    - introduce CCA CoT, rename TBBR ([c4b35ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c4b35cebffb0d034aa7bdba7cfdb65ba93939e35))

  - **FDTs**

    - **STM32MP2**

      - add board ID OTP in STM32MP257F-EV1 ([88528f5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/88528f55771fdc0a94b2ddd7f49f495a83044a24))
      - add OTP nodes in STM32MP251 SoC DT file ([c238a46](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c238a46a76660cbfa9ed40da4b1d0e5d477c3dd7))

  - **Security**

    - add support for SLS mitigation ([538516f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/538516f5d3db6e2c30dfa9f0b82859389f529e78))

- **Documentation**

  - update maintainer list for neoverse_rd ([2d7902d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d7902d9bf0bafceee9f571225862c476de0cdce))

- **Build System**

  - check that .text section starts at page boundary ([3d6edc3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3d6edc325c52082ab63ffd003c55a4ed875a52c5))
  - redirect stdin to nul during toolchain detection ([b9014f8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b9014f858d1fd963a466228ec15572b0892a8490))

- **Tools**

  - **Memory Mapping Tool**

    - add RELA section display ([a6462e0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a6462e05cf1cd55da44002cdede04053a928cf0a))

### Resolved Issues

- **Architecture**

  - **Memory Tagging Extension2**

    - remove CTX_INCLUDE_MTE_REGS usage ([30788a8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/30788a8455779b70aebd38d53afc8aa19d776c6c))
    - use ATA bit with FEAT_MTE2 ([ef0d0e5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef0d0e5478a3f19cbe70a378b9b184036db38fe2))

  - **Performance Monitors Extension (FEAT_PMUv3)**

    - fix breakage on ARMv7 CPUs with SP_min as BL32 ([e6f8fc7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e6f8fc7437f6b9483ea0463315809d7ff6d5c0ec))

  - **Statistical profiling Extension (FEAT_SPE)**

    - invoke spe_disable during power domain off/suspend ([777f1f6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/777f1f6897b57fe98c70d17c0d318aab3b86e119))

- **Platforms**

  - **Arm**

    - move console flush/switch in common function ([6bdc856](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6bdc856bc9135db420196683501b4f201b30ae3a))
    - only expose `arm_bl2_dyn_cfg_init` to BL2 ([3b48ca1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3b48ca17f350d8b0999e89e8d9215993701e16a0))

    - **FVP**

      - added ranges for linux ([b7491c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b7491c77d7ad2991b8c7c01f0311ebb3b0eca397))
      - don't check MPIDRs with the power controller in BL1 ([6d8546f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d8546f9fc49a03a817b15b20a9d62fadda74b9c))
      - permit enabling SME for SPD=spmd ([0b0fd0b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0b0fd0b47616b706e2f07c6da548cdc913fecd17))

    - **FPGA**

      - halve number of PEs per core ([70b9204](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/70b9204e6f98f1ec4f0529e8c1c88e8ece490d22))

    - **Neoverse-RD**

      - **SGI**

        - align to misra rule for braces ([cacee06](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cacee0605684a75bbe8783c74fddba97b9abcffa))
        - apply workarounds for N2 CPU erratum ([7934b68](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7934b68af6b446783823a114f25c3be06244c0e4))
        - increase BL31 carveout size ([0737bd3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0737bd33faba5c9e6a0e98969e015430e2782332))
        - reduce cper buffer carveout size ([f10d3e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f10d3e4953741eb3be1f9e4c09e7420554a0f050))
        - update spi_id max for sgi multichip platforms ([89d8577](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/89d857780c50bddf94db26f158c008b4cc846edf))

      - **RD-N1-Edge**

        - update RD-N1-Edge's changelog title ([d239ede](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d239edea5644657ac72458cc13e3ce6bb5754ff8))

      - **RD-N2**

        - populate TOS_CONFIG only when SPMC_AT_EL3 is enabled ([10dcffe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10dcffedb36a658cf8a3389fbdeb499d4e7e4446))

    - **TC**

      - correct interrupts ([d2e44e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d2e44e7d71863e3b302b5e72c8262bb0f3964fe6))
      - do not enable MPMM and Aux AMU counters always ([fc42f84](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fc42f84560d33c53b248e14913bbd6a69a8d310a))
      - do not use r0 for HW_CONFIG ([a5a966b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a5a966b12d9fe51a337db3204e7463ad95ba99c6))
      - enable FEAT_MTE2 ([154eb0a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/154eb0a22fa0a88d1f46e3674e3979626a83e063))
      - guard PSA crypto headers under TF-M test-suite define ([d2ce6aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d2ce6aa066ce1539908726de0d94a59c16634c4a))
      - increase BL2 maximum size limit ([19258a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/19258a5839cae9a81fb7256fbea34ff118220161))
      - increase stack size when TRUSTED_BOARD_BOOT=0 ([44ddee6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44ddee6f0a993ed5b3409e6626c0c70b7ed7d7a2))
      - missing device regions in spmc manifest ([5e47112](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5e4711208db622ff6150e69c87962b506742a544))
      - remove timer interrupt from G1S ([9bf31a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9bf31a59d187f6537066f05677972d9767e96c82))

  - **Intel**

    - add HPS remapper to remap base address for SDM ([b727664](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b727664e0dcf62be39552521c451ecde02091917))
    - bl31 overwrite OCRAM configuration ([cfbac59](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cfbac59590056e6b639aed56a1da480cd46f6f3e))
    - fix hardcoded mpu frequency ticks ([150d2be](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/150d2be0d2d440011c91c9bf8013a1ab602b464c))
    - read QSPI bank buffer data in bytes ([2f17ac0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2f17ac01adf28edb90a5ec8f446be1be76971b5c))
    - revert back to use L4 clock ([d0e400b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d0e400b3c626be647b9a20bc4f4869e20cc15dde))
    - revert sys counter to 400MHz ([460692a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/460692afb5b934720b69c410e3b02c540a3b1ddf))
    - temporarily workaround for Zephyr SMP ([68820f6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68820f642191cef67df38516ef1c2ed1411c579f))
    - update DDR range checking for Agilex5 ([f4aaa9f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f4aaa9fd6e6b4edd03976680b94e1c24aa582a68))
    - update fcs crypto init code to check for mode ([b0f4478](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b0f447897d3e2ddd72b291cb450165f4d220663e))
    - update fcs functions to check ddr range ([e8a3454](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8a3454cb74a9b55c0cb678d47a8553ece660439))
    - update from INFO to VERBOSE when print debug message ([56c8d02](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/56c8d022b00ba212f3e21dcfab20c14f3a44eec4))
    - update HPS bridges for Agilex5 SoC FPGA ([2973054](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2973054d9b4ba4fbcad7e04303ce8e0838b2f2b3))
    - update individual return result for hps and fpga bridges ([82752c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/82752c412362607549068d1c10cf7688f309d249))
    - update nand driver to match GHRD design ([a773f41](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a773f4121b3064fba24631e980c6226f23378e06))
    - update stream id to non-secure for SDM ([8fbd307](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8fbd3073cacfc7a23efdfda4eecfaf6607515306))
    - update system counter back to 400MHz ([a72f86a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a72f86ac4208e2aae5da83229cdd9ac97f651e36))

  - **NXP**

    - **i.MX**

      - **i.MX 8M**

        - align 3200 MTps rate with U-Boot ([060fe63](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/060fe63337097c6cadea76ef5d2d383f0d90ef01))
        - fix CSU_SA_REG to work with all sa registers ([c13016b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c13016bac6a6960acbbfb3e0176e1894a7e9fa3a))
        - handle 3734 in addition to 3733 and 3732 MTps rates ([cb60a87](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cb60a876efc156c87afcd5ec53b9cf356f30211d))

        - **i.MX 8M Plus**

          - uncondtionally enable only the USB power domain ([ae6ce19](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae6ce196df5b932f38c543cd8c6d8d86ee600009))

      - **i.MX 8ULP**

        - add sw workaround for csi/hotplug test hang ([e1d5c3c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e1d5c3c8f435424394367e2ff19240b1b8a3073c))
        - fix suspend/resume issue when DBD owner is s400 only ([68f132b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68f132b88bb24277ee34d5c3c94d16c26d7d4545))
        - increase the mmap region num ([047d7d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/047d7d1ba2fc84d8377156f7f45d2d69c3cb5f84))

  - **QEMU**

    - disable FEAT_SB ([59bdb42](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/59bdb426d300a6350334523a8dbc3fa6ae9f3bfc))
    - increase max FIP size ([f465ac2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f465ac221001f82bed907be356917675645d92eb))

  - **Raspberry Pi**

    - consider MT when calculating core index from MPIDR ([6744d07](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6744d07d9475adb49352fa57aa72fce17a95d757))

  - **Renesas**

    - **R-Car**

      - fix implicit rule invocations in tools ([e068a7c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e068a7ca860f35a171f608d55fb8a2a00ebd7561))

      - **R-Car 3**

        - change RAM protection configurations ([e9afde1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e9afde1a2e311df0197a8e9102ef535382aef228))
        - fix load address range check ([4f7e0fa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4f7e0fa38fdb6a25b07afafff492985bcc4e63a0))

  - **Rockchip**

    - add support for building with LTO enabled ([e5e9ccd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e5e9ccdb0c070d3066e7d778e5e2b563acd7ba98))
    - fix documentation in how build bl31 in AARCH64 ([6611e81](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6611e81e14ed4aa16844e3865fd8a9f6fa99a074))

    - **RK3328**

      - apply ERRATA_A53_1530924 erratum ([dd2c888](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dd2c888606dcdd638354c6345e08d4415d9d09fd))

  - **ST**

    - **STM32MP2**

      - add missing include ([cb0d6b5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cb0d6b5b5f7530335eac3c387bbb82d86608b0ea))
      - correct early/crash console init ([4da462d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4da462dcdc2e435c8b732f3ceff4c94ca28b4c43))

  - **Texas Instruments**

    - do not stop non-secure timer on world switch ([d2e1f6a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d2e1f6a8811e52505556f7b91156499d82488751))

    - **K3**

      - increment while reading trail bytes ([0bdaf5c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0bdaf5c804f852fe21f6172e436524157c9f6919))

  - **Xilinx**

    - add console_flush() before shutdown ([7ec53af](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ec53afaade308b35f546480990dbc9304e06e7d))
    - add FIT image check in DT console ([e2d9dfe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e2d9dfe2bffe4fde28f2714058c8c882ea90102a))
    - add FIT image check in prepare_dtb ([046e130](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/046e1304721e8bbf3d304dac22aa290bcbb0d10c))
    - check proc variable before use ([652c1ab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/652c1ab1526877d3505218f87ea96e6a9b2ccc11))
    - deprecate SiP service count query ([6a80c20](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a80c20eff74054c28273b42f3fe8e1a8fc5add4))
    - fix sending sgi to linux ([427e46d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/427e46ddea1e528d4c57b1d8215482055bd79c3e))
    - follow MISRA-C standards for condition check ([655e62a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/655e62aa5bede7ace8f8c6df571707aca9d6e14f))
    - rename macros to align with ARM ([7995319](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/79953190bc856ac3f47281029a80e5129bb4437d))
    - update correct return types ([8eb6a1d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8eb6a1da1229b8f0bff33293cbb86ce20d09259d))

    - **Versal**

      - initialize cntfrq_el0 register ([f000744](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f000744e0f501c89fb2240b47e91c261e3082249))

      - **Versal NET**

        - setup counter frequency ([07625d9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07625d9dd42d81c0e15f101fc0b6efa1c784b6f4))
        - use arm common GIC handlers ([b225926](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b2259261815961042d2a994401929bc76a0d3ee9))

    - **ZynqMP**

      - resolve null pointer dereferencing ([20fa9fc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/20fa9fc82334c67834eb22e20a3f4a07bcbe069d))

  - **Nuvoton**

    - gfx frame buffer memory corruption during secondary boot ([ae2b4a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae2b4a5494f9b4985fc2434e543ab0921e3b5a34))
    - prevent changing clock frequency ([fe8cc55](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fe8cc55a0cb5e47a0c0e28b147ee3e8dfdae07b2))

- **Bootloader Images**

  - **BL1**

    - add missing `__RW_{START,END}__` symbols ([d701b48](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d701b48eef4bb4b4b13ce5ef4091a37047e49a0b))
    - add missing spinlock dependency ([e40b563](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e40b563e87fd4ff58474a289909a1827c8d2bca7))

  - **BL2**

    - make BL2 SRAM footprint flexible ([e0e03a8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e0e03a8d8b7eac45606812d1f2a9685b51e44515))

- **Services**

  - **FF-A**

    - add NS memory node to fvp_spmc_optee_sp manifest ([92bba3e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/92bba3e711a21f2d31842bee64a1bd87e4b65414))

  - **RME**

    - **RMMD**

      - avoid TRP when external RMM is defined ([57bc3c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/57bc3c40560285e6029742b7360f8a0d0ac2346c))
      - fix bug, raised by coverity, when zeroing manifest struct ([83a4e8e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83a4e8e0c69c64219e4d9de6c7f51fb10e3adc5a))

  - **SPM**

    - add device-regions used in tf-a-tests ([45716e3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/45716e377ecb30c17aa3b375ce1e232d15492b9c))
    - not defining load-address in SP config ([04e7f80](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/04e7f80823e8a083138dd25963a5509bacd93257))
    - reduce verbosity on passing tf-a-tests ([29872eb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/29872eb330201334fcb8e418b7dc7ae8ff0dc192))
    - silence warning in sp_mk_generator ([6a3225e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a3225e2277df18e5c3aceb6173579cccefece51))

    - **EL3 SPMC**

      - add datastore linker script markers ([ba33528](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ba33528a00bb83f5562918131cb37574fc287193))
      - fix dangling pointer in FFA_CONSOLE_LOG ([83129bc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83129bcd8e75f1ffbfc9a3bae3d60749b1d22fe3))

    - **SPMD**

      - register group0 handler only if supported ([fca5f0e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fca5f0ebe5c2b5cf1c9d5096db6001a60ff7e089))
      - skip NS EL1 context save & restore operations ([2d960a1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d960a11601be6e7f24c38d84b2a4fdbb52efb9b))

- **Libraries**

  - **CPU Support**

    - workaround for Cortex-A520 erratum 2630792 ([f03bfc3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f03bfc304599540d859c4a07ac85d1bd9ae2c4f0))
    - workaround for Cortex-A520 erratum 2858100 ([34db353](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/34db3531ba085f111274b3b8e18476c4a392c245))
    - workaround for Cortex-A710 erratum 2778471 ([c9508d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c9508d6a1062ec3de4baaa3bd79ceed13eb972ad))
    - workaround for Cortex-A715 erratum 2331818 ([53b3cd2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/53b3cd2532dbdb794ddfedcc8a3985d2404eb6f7))
    - workaround for Cortex-A715 erratum 2344187 ([33c665a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33c665ae955fe5f5ae255f56ef6cdf073a9f601f))
    - workaround for Cortex-A715 erratum 2413290 ([15a0461](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/15a04615bb6834d93ab0077b89726dc17e3ba8b0))
    - workaround for Cortex-A715 erratum 2420947 ([1f73247](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1f732471320cee7b4f355ecff7dcfab7018e48ae))
    - workaround for Cortex-A715 erratum 2429384 ([262dc9f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/262dc9f76086970dab3dc43815890bed0ea29c79))
    - workaround for Cortex-A715 erratum 2561034 ([6a6b282](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a6b282378340dc61cf088ff5a06770cf68f44d8))
    - workaround for Cortex-A715 erratum 2728106 ([10134e3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10134e3556ca61e670017e681eb637889b1bd4f8))
    - workaround for Cortex-A720 erratum 2926083 ([152f4cf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/152f4cfa16bc3d2786f598390450af38f4b2d0be))
    - workaround for Cortex-A720 erratum 2940794 ([7385213](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7385213e602465d27530015a9b28ebc36a77b1c1))
    - workaround for Cortex-A78C erratum 2683027 ([68cac6a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68cac6a0f273dbe4f44563b467c996fafef07016))
    - workaround for Cortex-A78C erratum 2743232 ([81d4094](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/81d4094d637871ff34ddd7c2e2b3e842915f30f5))
    - workaround for Cortex-X2 erratum 2778471 ([b01a93d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b01a93d7789a794ef0635e0a7b0e7e53cc8519e5))
    - workaround for Cortex-X3 erratum 2266875 ([a65c5ba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a65c5ba351178e6119299fa935a3576453cf900b))
    - workaround for Cortex-X3 erratum 2302506 ([3f9df2c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3f9df2c6ad053172c5dab74cd12d82a5b2c93c34))
    - workaround for Cortex-X3 erratum 2372204 ([7f69a40](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f69a40697c3cc64e3fc553f6b50c72b97238dc9))
    - workaround for Cortex X3 erratum 2641945 ([c1aa3fa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c1aa3fa5555250dfbcae99fb6944ad24c4ee6a0b))
    - workaround for Cortex X3 erratum 2743088 ([f43e9f5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f43e9f57dc37a806bcd5e25a46b9f9bb1f365a64))
    - workaround for Cortex-X3 erratum 2779509 ([355ce0a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/355ce0a43abc1559b072b9cd9905f5194a6f0b86))
    - workaround for Cortex-X4 erratum 2701112 ([cc41b56](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc41b56f41af14b00ce9f5c802e2f883786cef38))
    - workaround for Cortex-X4 erratum 2740089 ([c833ca6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c833ca66a6fecbc54e038164e466be677559ec4e))
    - workaround for Cortex-X4 erratum 2763018 ([4731211](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47312115dea140dd7ba26cf0512856a41f3e3067))
    - workaround for Neoverse V1 erratum 2348377 ([71ed917](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/71ed91733140c82a392161c81869fcadb445c01a))
    - workaround for Neoverse V2 erratum 2618597 ([c0f8ce5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0f8ce5379a77e61e89d91e225784801e5bbd3e0))
    - workaround for Neoverse V2 erratum 2662553 ([912c409](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/912c4090fff207b445dde4bff72cc9b6e057e8b7))
    - workaround for Neoverse V2 erratum 3099206 ([8815cda](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8815cdaf57806901cfd388b8ee8c7979a8a2fe15))
    - add Cortex-A520 definitions ([ae19093](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae19093f2aa6dd95cc7819accb0d05c0ebe4eeb3))
    - workaround for Cortex-A715 erratum 2413290 re-factored with ENABLE_SPE_FOR_NS=1 ([bd2f7d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bd2f7d325826f75acd729d4ee2719fd6130a7c5e))
    - fix a defect in Cortex-A715 erratum 2561034 ([57ab6d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/57ab6d897656f71d229268d80e41b26e62179400))
    - add erratum 2701951 to Cortex-X3's list ([106c428](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/106c4283a564e4f37976ebc7dd8bc7d35f6592e4))
    - update status of Cortex-X3 erratum 2615812 ([f589a2a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f589a2a5f1b032ff3a09a419e49db0b97ccd8595))
    - fix incorrect AMU trap settings for N2 CPU ([54b86d4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/54b86d47eb05f09330df57519b7d04b9968890e5))
    - correct variant name for default Poseidon CPU ([61a2968](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/61a29682c66d0437806f81fb8ab0e3ff321dfe04))
    - check for SCU before accessing DSU ([5b5562b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b5562b2e5855f949f1fc0579d7aff15e6b274ef))

  - **EL3 Runtime**

    - **Context Management**

      - add more feature registers to EL1 context mgmt ([d6c76e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d6c76e6c65429326e7572e10f521dd9108a3a1e3))
      - add more system registers to EL1 context mgmt ([ed9bb82](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ed9bb824e4a3815e60acaa69ed66796279f4afbf))
      - hide `cm_init_context_by_index` from BL1 ([a6b3643](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a6b3643c2a1a95146e93c8b6f07c2e491a1230d6))
      - remove ENABLE_FEAT_MTE usage ([a796d5a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a796d5aa11b25622841cd2283630ff9348eed699))
      - save guarded control stack registers ([6aae3ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6aae3acfd0d48e49e2367e6cd883dda7dca974c8))
      - update gic el2 sysregs save/restore mechanism ([937d6fd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/937d6fdb70cd24602fd2638a5dbd5c46d32559c1))
      - couple el2 registers with dependent feature flags ([d6af234](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d6af23443179f6d2239c7f5f190f0d8828bd68cf))
      - move EL1 save/restore routines into C ([59f8882](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/59f8882b44845ab865e354eeda8ce653f5d5fcf3))

  - **FCONF**

    - boot fails using ARM_ARCH_MINOR=8 ([0c86a84](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c86a846d9149ee5af7e1ee4bb185c532ed9d0f8))

  - **OP-TEE**

    - set interrupt handler before kernel boot ([0ec69a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ec69a5bfbfcdf4566db8e96adaf29ad847d3d58))

  - **PSCI**

    - fix parent_idx in psci_validate_state_coordination ([412d92f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/412d92fdfd28d2f850a48e5f0aee95faa894a556))
    - mask the Last in Level nibble in StateId ([0a9c244](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0a9c244b05ef2d2d4b946ba81bb9b9584b479b48))

  - **GPT**

    - declare gpt_tlbi_by_pa_ll() ([832e4ed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/832e4ed520d5ed7e64249fe98c1ffb4550db5eca))
    - unify logging messages ([b99926e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b99926ef7b287738c4b4a87ee7ab4eaed1e4038f))
    - use DC CIGDPAPA when MTE2 is implemented ([62d6465](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/62d64652134ca1d3ea68da65ea9e4ae136f6c44e))

  - **C Standard Library**

    - add memcpy_s source file to libc_asm mk ([99db13b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99db13bfaa5b11345730937c2e0e56cb670c01a5))
    - memset inclusion to libc makefiles ([84eb3ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/84eb3ef6c9f596e968b4f9b83a3a01deda2a8a9d))

  - **PSA**

    - fix static check failure ([bc0ff02](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc0ff02cbb046388eff1a95efd0043757d6ac317))

  - **Context Management**

    - align the memory address of EL2 context registers ([8c56a78](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8c56a78894ddc69167bc093fe19f173feced720c))

  - **Firmware Handoff**

    - correct representation of tag_id ([d594ace](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d594ace68d4fa62cf2f1d5d13503b737b85924e5))

  - **Exception Handling Framework (EHF)**

    - restrict secure world FIQ routing model to SPM_MM ([7671008](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7671008fcfc826dbc3166ff1bdbb9cd7fbc7f68b))

  - **SMCCC**

    - correctly find pmf version ([62865b4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/62865b4ee455806e37a9c5bd52255b8c09cf1a1a))

- **Drivers**

  - **Measured Boot**

    - add missing image identifier string ([a8a09e3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a8a09e3141354b159e7699d7c9c325bdd817b1f5))

  - **SCMI**

    - induce a delay in monitoring SCMI channel status ([af1ac2d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af1ac2d7db47717bc69afd69b56f398aa34b2fb6))

  - **Arm**

    - **GIC**

      - **GICv3**

        - **GIC-600**

          - workaround for Part 1 of GIC600 erratum 2384374 ([24a4a0a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24a4a0a5ec25e179f2e567a6e13a9b5c87db1b81))

      - **GICv2**

        - fix SGIR_NSATT bitshift ([eef240c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eef240cfdedcc59f09dd5cd942448c5dcecc75d6))

    - **MHU**

      - use MHUv2 if PLAT_MHU_VERSION undefined ([c34dd06](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c34dd06a843d71cdba2fa1c3c9067f6f130a0c73))
      - provide only the usable size of memory ([5cd1084](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5cd10848be4f6ac19daa66803c3d512e3eea4266))

    - **RSE**

      - fix bound check during protocol selection ([f754bd4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f754bd466749a9338561f991bfb85140dd034e03))

  - **Renesas**

    - **R-Car3**

      - add integer overflow check ([ef38fb1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef38fb1f5a5f2bdb897158e4244a1eddd2396eeb))
      - add integer overflow check ([93b8952](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/93b8952eefa14141c142070a71fc017736c8910c))
      - check "rcar_image_number" variable before use ([b469880](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b469880e3b6b26849c3d43d3fe88a755a25249bc))
      - check for length underflow ([9778b27](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9778b270e29bac3e16f57f9557098c45858c05de))
      - check loaded NS image area ([ae4860b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae4860b0f5c283aeca4def1449f0293ef22ff508))

  - **USB**

    - add missing include ([f84f21f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f84f21fa8d17662dcdc6b0b8b0caca4a45cd9ccd))

- **Miscellaneous**

    - **TBBR**

      - move rotpk definitions out of arm_def.h ([0f0fd49](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0f0fd499dedd799e19279f0aa1f4f686085a944a))

    - code coverage optimization fix ([152ad11](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/152ad112d73402523302f3cb252aee0efc145736))
    - fix MISRA defects ([c42d0d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c42d0d8754ae8818a7e7a63e873ca7699a7f102b))
    - static checks on spmc dts ([c35299d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c35299d6b4e8b2757e47dc4c5a3b2e0836f89a7d))

- **Documentation**

  - revise the description of REGISTER_CRYPTO_LIB ([5710229](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5710229f9e837f28e4bafee6b51e828f901bf3f1))
  - typo in the romlib design ([3b57ae2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3b57ae23e0891e44d5b648575b80cbad4fc10405))

- **Build System**

  - add forgotten BL_LDFLAGS to lto command line ([49ba1df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49ba1df52204e721f06a6da76ef0f8692ce1b2f8))
  - don't generate build-id ([304ad94](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/304ad94b34c2117823169a199558e7484139caa1))
  - don't rely on that gcc-ar is in the same directory as gcc ([7ef0b83](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ef0b8377fa7fb3697dda5adfa44dafd7e14150f))
  - enforce single partition for LTO build ([31f80ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/31f80efeefaee2c59db50a46cabe2b5fdf20e4ae))
  - march handling with arch-features ([7275ac2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7275ac2af86277e2442ef4b0fee6c35cbe830056))
  - move comment for VERSION_PATCH ([c25d1cc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c25d1ccf1e205b2781ecd0de91e91d35e57b79bc))
  - mute sp_mk_generator from build log ([fbd32ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fbd32ac081c421929728f454427b7839235d2075))
  - properly manage versions in .versionrc.js ([7f74030](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f74030b89136a1673e2a949564403709bc48f5d))
  - wrap toolchain paths in double quotes ([4731c00](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4731c00bb60915c0d4b29c082a752e9925a244b4))

- **Tools**

  - **Certificate Creation Tool**

    - add guardrails around brainpool usage ([c0c280d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0c280dfda7322dcaebb5c6341c0880bdf524e13))
    - use a salt length equal to digest length for RSA-PSS ([e639ad2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e639ad23c8c7a1b320af9ebd519420ae7d431531))

  - **Memory Mapping Tool**

    - fix footprint free space calculation ([9e72d01](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9e72d01ed29c350dfc0567c59bc482901211634b))
    - fix memory map dump when SEPARATE_CODE_AND_RODATA=0 ([6dc8ee6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6dc8ee61ffeee8ea5aafdbef3121fa4e82b57932))

  - **Marvell Tools**

    - include mbedtls/version.h before use ([8eb4efe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8eb4efe70bd5b03917e2063ab8ff5646de88922a))

## [2.10.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.9.0..refs/tags/v2.10.0) (2023-11-21)

### ⚠ BREAKING CHANGES

- **Architecture**

  - **Performance Monitors Extension (FEAT_PMUv3)**

    - This patch explicitly breaks the EL2 entry path. It is
      currently unsupported.

      **See:** convert FEAT_MTPMU to C and move to persistent register init ([83a4dae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83a4dae1af916b938659b39b7d0884359c638185))

- **Libraries**

  - **EL3 Runtime**

    - **Context Management**

      - Initialisation code for handoff from EL3 to NS-EL1
        disabled by default. Platforms which do that need to enable this macro
        going forward

        **See:** introduce INIT_UNUSED_NS_EL2 macro ([183329a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/183329a5847df2bc6164ac8e9dbe7de4ca92836d))

- **Drivers**

  - **Authentication**

    - remove CryptoCell-712/713 support

      **See:** remove CryptoCell-712/713 support ([b65dfe4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b65dfe40aef550ee9ef7e869749013cb7f3c4cce))

### New Features

- **Architecture**

  - **CPU feature / ID register handling in general**

    - add AArch32 PAN detection support ([d156c52](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d156c5220adb35971aafa0b0de922992e4b8aa66))
    - add memory retention bit define for CLUSTERPWRDN ([278beb8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/278beb894aeda23278a01c3c6aff1f40b8ce0a34))
    - deny AArch64-only features when building for AArch32 ([733d112](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/733d112f05ecb29f7d8fce12c66a9721031970df))
    - initialize HFG*_EL2 registers ([4a530b4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4a530b4c6556c87deb22c027dfaf2c5d6c9997a3))

  - **Memory Tagging Extension**

    - adds feature detection for MTE_PERM ([4d0b663](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d0b66323b242323ff738431c523aeb6d18dd3d5))

  - **Performance Monitors Extension (FEAT_PMUv3)**

    - introduce pmuv3 lib/extensions folder ([c73686a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c73686a11cea8f9d22d7df3c5480f8824cfeec09))

- **Platforms**

  - **Allwinner**

    - use reset through scpi for warm/soft reset ([0cf5f08](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0cf5f08a205e4877c9daef5d90e1086643590226))

  - **Arm**

    - add IO policy to use backup gpt header ([3e6d245](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3e6d245772ccb4b43f1ba6cd9d1bb8abe86a516c))
    - ecdsa p384/p256 full key support ([b8ae689](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b8ae68908de5560436c565ac22d59c0cbfc9a7df))
    - enable FHI PPI interrupt to report CPU errors ([f1e4a28](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1e4a28d3f9e4c5e7905f44d41c13de63d735864))
    - reuse SPM_MM specific defines for SPMC_AT_EL3 ([5df1dcc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5df1dccd0be06cc45e82a57dc01be5b6b5d1a21b))
    - save BL32 image base and size in entry point info ([821b01f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/821b01fa7521c0d6a0f16d02929fac3c44d14f86))
    - add memory map entry for CPER memory region ([4dc91ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4dc91ac9069271325ffd3552a6a146256f5d0da3))
    - firmware first error handling support for base RAMs ([5b77a0e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b77a0e6759733d8a7de86e4492bd9b8628282d5))
    - update common platform RAS implementation ([7f15131](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f15131df42a42fef86cc594a56b6e7998dd2ba4))

    - **FVP**

      - add mbedtls_asn1_get_len symbol in ROMlib ([0605060](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/06050601d2a0ff06f92ca30ab988cbaf4e9929a1))
      - add public key-OID information in RSS metadata structure ([bfbb1cb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bfbb1cbaac3e74da37d906c9ce1d39993dce8b66))
      - add spmd logical partition ([5cf311f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5cf311f3a41fc114289265305a6254a8fb412c0e))
      - allow configurable FVP Trusted SRAM size ([41e56f4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/41e56f422df47b8bc1a7699ff258999f900a6290))
      - capture timestamps in bl stages ([ed8f06d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ed8f06ddda52bc0333f79e9ff798419e67771ae5))
      - implement platform function to measure and publish Public Key ([db55d23](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/db55d23d34b687cf6ce79c0723fedf10ef7227be))
      - increase BL1 RW area for PSA_CRYPTO implementation ([ce18938](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce189383dc816cf1a48c1a94329c00f44d8acdc3))
      - mock support for CCA NV ctr ([02552d4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/02552d45e526766e000f3e3ae91ef381d402dab1))
      - new SiP call to set an interrupt pending ([2032401](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/20324013b17706751ecdd68f57c0ab95c522ca7e))
      - spmd logical partition smc handler ([a1a9a95](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a1a9a950713468a734ef3d8da210baf97f7c1071))

    - **Juno**

      - add mbedtls_asn1_get_len symbol in ROMlib ([ec8ba97](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ec8ba97e4ffde486670cb5a22ec4aac01409d92e))

    - **Morello**

      - add cpuidle support ([4f7330d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4f7330dc78ee620b8564a4bbc1ca2f2ae4cd1d9e))
      - add support for I2S audio ([6bcbe43](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6bcbe437909d3779111e19774f911c625e98f1b3))
      - add TF-A version string to NT_FW_CONFIG ([f4e64d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f4e64d1f5e8277013c35dbd8e056b8071942f759))
      - fdts: add CoreSight DeviceTree bindings ([3e6cfa7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3e6cfa7bd05521935c7753401dad823d044bfa23))
      - set NT_FW_CONFIG properties for MCC, PCC and SCP version ([10fd85d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10fd85d8f4a8f338942616ed403a1e02a388a16f))

    - **RD**

      - **RD-N2**

        - enable base element RAM RAS support on RD-N2 platform ([0288632](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/02886326659db3e4f46c0abd10be91a2de82cc90))
        - add defines needed for spmc-el3 ([b4bed4b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b4bed4b769e907c8431b07f698da24660dfe0059))
        - add plat hook for memory transaction ([f99dcba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f99dcbace7015169ac5d230b8007686d144962fb))
        - enable Neoverse N2 CPU error handling support ([e802748](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e80274880bf694fd0b0e869a6ceb67e95e547544))
        - introduce accessor function to obtain datastore ([f458934](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f45893426546703d9e21970889e6333ca30c0dd7))
        - introduce platform handler for Group0 interrupt ([c47d049](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c47d0491ed078cfa8ca400e182fd4a44acd8041a))

    - **SGI**

      - remove RAS setup call from common code ([0f5e8eb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0f5e8eb4536e27f5fd99b1367b18710927b014b9))
      - firmware first error handling for Neoverse N2 CPU ([31d1e4f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/31d1e4ff8dd70dc0094ff44df0c1844d27430e77))
      - increase sp memmap size ([7c33bca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7c33bcab5973fb73b8278c674677663f5109948e))

    - **TC**

      - define memory ranges for tc platform ([9be6b16](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9be6b168fb482835a13ad39e7567721f74d513f9))
      - implement platform function to measure and publish Public Key ([eee9fb0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eee9fb02f7b2c29befa27a0f2f0b6cb966f6d7c5))
      - deprecate Arm TC1 FVP platform ([6a2b11c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a2b11c29da50eed969834f6c6ee97cdb90cb51e))

  - **Aspeed**

    - **AST2700**

      - add Aspeed AST2700 platform support ([85f199b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/85f199b774476706b21f793503b36d861cab0a14))

  - **Intel**

    - add intel_rsu_update() to sip_svc_v2 ([e3c3a48](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e3c3a48c85dd1478e311e2e773a22fecfda69ec5))
    - ccu driver for Agilex5 SoC FPGA ([02df499](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/02df49900006ed44b4a0c239299dd45ca8509c17))
    - clock manager support for Agilex5 SoC FPGA ([1b1a3eb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b1a3eb1edff99b49bb40ad4172073d04a230938))
    - cold/warm reset and smp support for Agilex5 SoC FPGA ([79626f4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/79626f460f115cc32b0dbeb48e72828d2dbf662a))
    - ddr driver for Agilex5 SoC FPGA ([29461e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/29461e4c880235532385c01f202e638fb5ba11de))
    - mailbox and SMC support for Agilex5 SoC FPGA ([8e59b9f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8e59b9f42374aaa641409b6469c8fe9245a33107))
    - memory controller support for Agilex5 SoC FPGA ([18adb4e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18adb4efa42946252b489d02f06cccb61ad0c867))
    - mmc support for Agilex5 SoC FPGA ([4a577da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4a577da6612ef6584695311e687ca00c57d68d53))
    - pinmux, peripheral and Handoff support for Agilex5 SoC FPGA ([fcbb5cf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fcbb5cf7eadb8b048149941b08f09d04a860fee0))
    - platform enablement for Agilex5 SoC FPGA ([7931d33](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7931d3322dc137447981d261e900f5a62d2181ee))
    - power manager for Agilex5 SoC FPGA ([a8bf898](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a8bf898f02185ed838d8039949800843146ab245))
    - reset manager support for Agilex5 SoC FPGA ([9b8d813](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9b8d813cc96173ce8ab7634dea17fb7f89b21626))
    - restructure sys mgr for Agilex ([6197dc9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6197dc98feba98c3e123256424d2d33d5de997b8))
    - restructure sys mgr for S10/N5X ([b653f3c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b653f3caf0f5e624604564c8c89ac8f4b450ba20))
    - sdmmc/nand/combo-phy/qspi driver for Agilex5 SoC FPGA ([ddaf02d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ddaf02d17142187d9f17acd4900aafa598666317))
    - setup SEU ERR read interface for FP8 ([91239f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/91239f2c05c5df041e4a570a9d29c0ccbc34269a))
    - system manager support for Agilex5 SoC FPGA ([7618403](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7618403110dad81c84822332225a7a687dc7f684))
    - uart support for Agilex5 SoC FPGA ([34971f8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/34971f816a777df5afb6672990b9eceda60e84b7))
    - vab support for Agilex5 SoC FPGA ([4754925](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4754925057b27d5992d4c913276602666d303b01))

  - **MediaTek**

    - add APU bootup control smc call ([94a9e62](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/94a9e6243e3978b42017639dad93481267bcf6e4))
    - add APU watchdog timeout control ([baa0d45](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/baa0d45ced6b058681ade9213e30ab0e91f4f4fb))

    - **MT8188**

      - add apusys ao devapc setting ([777e3b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/777e3b71bb0a37f98b4105af657d97c2afc2d0bc))
      - add backup/restore function when power on/off ([233d604](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/233d604f500b5693b0caa6bcfdf0e2f766fd4cbd))
      - add devapc setting of apusys rcx ([5986ae5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5986ae57aa4468b392d0f5fcb8b5bc04388fa3e2))
      - add DSB before udelay ([b254b98](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b254b9815ee25c90264a2305940bc575910f55e4))
      - add emi mpu protection for APU secure memory ([176846a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/176846a50b73267ff787432f74a1d9607b57ed20))
      - add EMI MPU support for SCP and DSP ([013006f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/013006f1f889f5869502147af464e38619459463))
      - add support for SMC from OP-TEE ([34d9d61](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/34d9d619f1c58549736b63aa5c5cddd7f171762e))
      - enable apusys domain remap ([b5900c9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b5900c92a1579371ea6f40199c70673beb08b1ac))
      - enable apusys mailbox mpu protect ([ad7673a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad7673adef9bc5eaeef333ecaca8e85e82abe342))
      - increase TZRAM_SIZE from 192KB to 256KB ([aa1cb27](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aa1cb279b62d82e3d6e7b6ec17b9eb71d598497e))
      - modify APU DAPC permission ([d06edab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d06edabfd14e0d196139fb1c780017f34366ae0d))
      - update return value in mtk_emi_mpu_sip_handler ([d07eee2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d07eee245b3fcc6b276969df34dc63ded1d4c8a2))

    - **MT8195**

      - increase TZRAM ([4f79b67](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4f79b67250641f67327c3e351d2f8339e8fd2d26))

  - **NXP**

    - **i.MX**

      - add dummy 'plat_mboot_measure_key' function ([b9bceef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b9bceef8eebf5c0f7f213921cca885a3f3c64ec1))

      - **i.MX 8M**

        - add more dram pll setting ([8947404](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/89474044a59d74cc088eb09292e99a3ca623fe33))
        - detect console base address during runtime ([df730d9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/df730d94cb5850683371dd695e242a0c3817f070))
        - enable snvs privileged registers access ([8d150c9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8d150c9524b1459b61c9d881100e20da827c1bd0))
        - move the gpc reg & macro to a separate header file ([2a6ffa9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2a6ffa99afb6091110231381d1263407e9d88c3f))

        - **i.MX 8M Nano**

          - add workaround for errata ERR050362 ([8562564](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/85625646692597ba8a1829efaadf56163450efaf))

      - **i.MX 9**

        - **i.MX93**

          - add cpuidle and basic suspend support ([422d30c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/422d30c626beef689967b56d26a68f029e7b7cf9))
          - add OPTEE support ([27a0be7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/27a0be77a064cbc87aaefecbf45fe0a2b133b188))
          - add reset & poweroff support ([cf7ef4c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cf7ef4c762ddb573ffb6f1f434c04fdc52f6c2cf))
          - add the basic support ([2368d7b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2368d7b157c169b84bc46d3d8a57d080507e81bd))
          - add the trdc driver ([2935291](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2935291009c2933714a027b7b5cd1c8e41f70aff))
          - allow SoC masters access to system TCM ([3d3b769](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3d3b769a7c112bff9468dbb21e36ce44125a72c0))
          - protect OPTEE memory to secure access only ([f560f84](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f560f843bdc0e33ef47918a6c10676fa6aff95ac))
          - update the ocram trdc config for did10 ([eb76a24](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eb76a2416a9bd5239db7b55d846bd2a16eec417a))

  - **QEMU**

    - add sdei support for QEMU ([cef76a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cef76a7c5df7056cb73667e4e0b83d022e1b50fa))
    - add "cortex-a710" cpu support ([4734a62](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4734a62d2c22f5b6a1e2b0369248d42fb9eddd1b))
    - add "neoverse-n2" cpu support ([408f9cb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/408f9cb485796a73c5b87da70644665a13c685e4))
    - add "neoverse-v1" cpu support ([6d8d7d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d8d7d2380d5120b3235c6f00eddcab126c3d648))
    - add "neoverse-v1" cpu support ([214de62](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/214de62c92b2fc4b7edda9d9d637b7a4c0ba1fa5))
    - add A55 cpu support for virt ([409c20c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/409c20c84dcfa61de68754152f331a7277609fb2))
    - add dummy plat_mboot_measure_key() BL1 function ([8e2fd6a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8e2fd6a84b17fde92cef48ecaccdc3b666ef0588))
    - add dummy plat_mboot_measure_key() function ([f0f11ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f0f11acd86650da04a41298acbf4ae38b7e25894))
    - implement firmware handoff on qemu ([322af23](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/322af23445fe7a86eaad335b8a0f2ed523f5c1df))

    - **SBSA**

      - handle platform version ([c681d02](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c681d02c6ce2652307a4fcef16bd5626135dfad9))
      - handle GIC base ([1e67b1b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1e67b1b17a1692dd653d31016ccd8fa18b5f8f67))
      - handle GIC ITS address ([4171e98](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4171e981d13e6aa764c2520a2b513beafe449818))

  - **QTI**

    - **MSM8916**

      - add port for MDM9607 ([78aac78](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/78aac78ad246ac8a04e1946bb9cd41b5734ba909))
      - add port for MSM8909 ([cf0a75f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cf0a75f04df8e90c7958304e6e0499a7d2e2519c))
      - add port for MSM8939 ([c28e96c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c28e96cd52f8fbdbbfd0bbc8bacef353ac65bfd6))
      - add SP_MIN port for AArch32 ([45b2bd0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/45b2bd0acbf4678eb59d36eb0db7746f5286a868))
      - add Test Secure Payload (TSP) port ([6b8f9e1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6b8f9e16a7849852abaf190f96130462f70eae17))
      - allow selecting which UART to use ([aad23f1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aad23f1a2c109fb853e498c17fa1e97fbdb6522c))
      - clear CACHE_LOCK for MMU-500 r2p0+ ([d9b0442](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d9b04423cfbf18cb510fb8e65ad02e7a1f4fe873))
      - initialize CCI-400 for multiple clusters ([1240dc7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1240dc7ef11e850bdf7a4e66de3d858e26555842))
      - power on L2 caches for secondary clusters ([c822d26](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c822d26506a589d4fa017246eeb83627f2efb554))

  - **ST**

    - add RCC registers list ([4cfbb84](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4cfbb84aeb361d8e4d72f0b0652d02918168b55e))
    - allow AARCH64 compilation for common code ([dad7181](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dad718169815f7cec09144b770fc66c6d9c58d17))
    - introduce new platform STM32MP2 ([35527fb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/35527fb41829102083b488a5150c0c707c5ede15))
    - support gcc as linker ([7762531](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7762531216a599d98dcf88aef8f8e980e0db90ed))
    - update STM32MP DT files ([4c8e8ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4c8e8ea772905c1420720a900dd3e7d94eefbc7e))

    - **STM32MP1**

      - add FWU with boot from NOR-SPI ([dfbadfd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dfbadfd96b6f3d383e8f1c3c8b0c91ca2110ea2e))

      - **STM32MP15**

        - disable OP-TEE shared memory ([fb1d3bd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fb1d3bd9330ce70f735a344dd4223faffb261118))

    - **STM32MP2**

      - add console configuration ([87a940e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/87a940e027dd11d0ec03ec605f205374b18361ba))
      - generate stm32 file ([e5839ed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e5839ed79e34b8aa8c7c94da8c79e8ee8a7467df))

  - **Texas Instruments**

    - add TI-SCI query firmware capabilities command support ([7ab7828](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ab782801f8c78ae6a8293d25cad687c86a4ac4e))
    - query firmware for suspend capability ([ce1008f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce1008fef1ace613bc36886fd1627164edfef245))
    - remove extra core counts in cluster 2 and 3 ([e986845](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e9868458e6de2ffb3c08e2fafa444a812b895337))

  - **Xilinx**

    - add support to get chipid ([0563601](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0563601f03f0404bbc57464d3458c07614f920ca))
    - clean macro names ([bfd0626](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bfd0626554374dd94a0105a5633df0afeae731b1))
    - fix IPI calculation for Versal/NET ([69a5bee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/69a5bee4c3633fd963d97f90f3a98e95a640d2da))
    - move IPI related macros to plat_ipi.h ([b2258ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b2258ce30cf720d71b1022c9cbee135c879027c5))
    - remove crash console unused macros ([473ada6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/473ada6be65af7fdad85845336f42ed481eea11b))
    - setup local/remote id in header ([068b0bc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/068b0bc6e39f1fc18f9450619942c711f860a7e2))
    - switch boot console to runtime ([9c1c8f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9c1c8f010143e179dee76381f3796f3801e6d220))
    - sync macro names ([04a4833](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/04a483359fef61353d95619e84ec6b495b27adfb))
    - used console also as crash console ([3e6b96e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3e6b96e869238f21c8887b835c3bfed487dbe653))

    - **Versal**

      - add support for SMCC ARCH SOC ID ([079c6e2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/079c6e2403fd07db2b41f7c6e7e8c568467a2c6b))
      - add tsp support ([7ff4d4f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ff4d4fbe58273541da86fa72786d4bd4604be9a))
      - ddr address reservation in dtb at runtime ([56d1857](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/56d1857efc21cff5e75aa65bba21e333a8552d04))
      - enable assertion ([0375188](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0375188a3e114edf62a732e80ea0f08dde3bf0b0))
      - retrieval of console information from dtb ([7c36fbc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7c36fbcc13793899390a01a9b4a623ff2fbf7ee1))

      - **Versal NET**

        - add cluster check in handoff parameters ([01c8c6a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/01c8c6a5542fbf09fa91bbdbc95b735bbc9f02d7))
        - add support for SMCC ARCH SOC ID ([1873e7f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1873e7f7d879c3d0aba54c3785df534b9a7037b7))
        - add the IPI CRC checksum macro support ([ba56b01](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ba56b012c8ba8e5c4e6f77ab8a921e494d040a44))
        - add tsp support ([639b367](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/639b3676cc30dcf3e3e4d478906e7f7f37a7f1e4))
        - ddr address reservation in dtb at runtime ([46a08aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/46a08aab4c56ad9e3f57b127a02fead1e6b8cf38))
        - enable assertion ([80cb4b1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/80cb4b14049c01df9a57cad9d1b94b10f904462f))
        - get the handoff params using IPI ([a36ac40](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a36ac40c4e93e56380374301f558f508ad2cbf96))
        - remove empty crash console setup ([6a14246](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a14246ad469664b56f1fdb111433515ffcccaf6))
        - retrieval of console information from dtb ([a467e81](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a467e813a362fae69484e70ecb26fd8b14489d38))

    - **ZynqMP**

      - enable assertion ([2243ba3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2243ba3c38ae5bab894709a4e98f188815398ef1))
      - remove pm_ioctl_set_sgmii_mode api ([7414aaa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7414aaa1a1e31df66866f0e1c97ba7c9add2427f))
      - retrieval of console information from dtb ([3923462](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3923462239c9e54088bd5b01fd5df469b2758582))

  - **Nuvoton**

    - added support for npcm845x chip ([edcece1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/edcece15c76423832fc1ffdb255528bf4c719516))

- **Bootloader Images**

  - **BL2**

    - add gpt support ([6ed98c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6ed98c45db01023d52a47eb4ede0ffb44de85f00))

  - **BL31**

    - reuse SPM_MM specific defines for SPMC_AT_EL3 ([f5e1bed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f5e1bed2669cce46a1d7c6b8d3f8f884b4d589b3))

  - **BL32**

    - print entry point before exiting SP_MIN ([94e1be2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/94e1be2b2918d8e70ac33cc8551e913d75e86398))

- **Services**

  - **RME**

    - save PAuth context when RME is enabled ([13cc1aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/13cc1aa70a666bc8f768569e5481b3daf499b7d1))

    - **RMMD**

      - enable SME for RMM ([f92eb7e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f92eb7e261bdaea54c10ad34451a7667a6eb4084))
      - pass SMCCCv1.3 SVE hint bit to RMM ([6788963](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/678896301b807cb1130ca27fa53acc66d57b855e))

    - **RMM**

      - update RMI VERSION command as per EAC5 ([ade6000](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ade6000ff0b3aa41d581d5738ce42f5ea4d3b77d))

  - **SPM**

    - separate StMM SP specifics to add support for a S-EL0 SP ([549bc04](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/549bc04f148f3b42ea0808b9ab0794a48d67007d))

    - **EL3 SPMC**

      - add a flag to enable support to load SEL0 SP ([801cd3c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/801cd3c84a7bb8a66c5a40de25e611ec6448239c))

    - **SPMD**

      - add partition info get regs ([0b850e9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0b850e9e7c89667f9a12d49492a60baf44750dd9))
      - add spmd logical partitions ([890b508](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/890b5088203e990d683a9c837e976be62c6501aa))
      - el3 direct message API ([66bdfd6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/66bdfd6e4e6d8e086a30397be6055dbb04846895))
      - get logical partitions info ([95f7f6d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/95f7f6d86a6aadc9d235684fd1aa57ddc4c56ea9))

  - **ERRATA ABI**

    - add support for Cortex-X3 ([9c16521](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9c16521606b1269ef13a69ec450b8d14ef92bde9))

- **Libraries**

  - **CPU Support**

    - add a concise way to implement AArch64 errata ([3f4c1e1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3f4c1e1e7b976e6950cbcc4ddf8c32e989d837ac))
    - add a way to automatically report errata ([4f748cc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4f748cc44cb12160dfca86d94a1075f38f7c99e4))
    - add errata framework helpers ([445f7b5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/445f7b5191992c760e1089f566b94473a0432a1e))
    - add more errata framework helpers ([94a75ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/94a75ad456a8bda75ca1e4343f00be249a201a69))
    - add support for Gelas CPU ([02586e0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/02586e0e28e590fbc5e8461cfdc03db08485c14f))
    - add support for hermes cpu ([a00e907](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a00e907696dd7dcae9ec221ea4ee49d4179a8e2a))
    - add support for Nevis CPU ([5497958](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/549795895cae55b11c1a7ce522aa6740de863fb4))
    - add support for Travis CPU ([a0594ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a0594add2e2661a1b1e1f392bf015687004197bb))
    - conform DSU errata to errata framework PCS ([ee6d04d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ee6d04d449d7a23840bab00f3d3ffd88c6c7bca6))
    - make revision procedure call optional ([4d22b0e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d22b0e5ba01b423f9f5200e4702750102635145))
    - wrappers to propagate AArch32 errata info ([34c51f3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/34c51f327d47653637cf3604b4cd20819e795f25))

  - **EL3 Runtime**

    - modify vector entry paths ([d04c04a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d04c04a4e8d968f9f82de810a3c763474e3faeb7))

    - **RAS**

      - reuse SPM_MM specific defines for SPMC_AT_EL3 ([6e92a82](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e92a82c81d2b0e49df730f68c8312beec1d3b48))
      - use FEAT_IESB for error synchronization ([6597fcf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6597fcf169fa548d40f1e63391d12d207c491266))

  - **Translation Tables**

    - detect 4KB and 16KB page support when FEAT_LPA2 is present ([bff074d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bff074dd941d4fb51d6abade5db4b636f977d6f7))

  - **C Standard Library**

    - add %X to printf/snprintf ([483edc2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/483edc207a533a5eaf07fa1e2c47f29f1dc64e4a))
    - implement memcpy_s in lib ([f328bff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f328bff667c12099e82de6e94f3775a124ee78c7))

  - **PSA**

    - interface with RSS for retrieving ROTPK ([50316e2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/50316e226fbbe30b5eb4121225958a9b63e58bb1))

  - **Firmware Handoff**

    - introduce firmware handoff library ([3ba2c15](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3ba2c15147cc0c86342a443cd0cbfab3d2931c06))
    - port BL31-BL33 interface to fw handoff framework ([94c90ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/94c90ac8168f4e257b67e138a53a2dbc612e4194))

- **Drivers**

  - **Authentication**

    - add CCA NV ctr to CCA CoT ([e3b1cc0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e3b1cc0c51c7b0bae6abd81e15e4c2a00442c5db))
    - add explicit entries for key OIDs ([0cffcdd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0cffcdd617986f0750b384620f5b960059d91fc9))
    - create a zero-OID for Subject Public Key ([9505d03](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9505d03e368d8e620c4defeb53dad846d5bc7e62))
    - ecdsa p384 key support ([557f7d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/557f7d806a62a460404f8d1bec84c9400585930b))
    - measure and publicise the Public Key ([9eaa5a0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9eaa5a09ed5805ec6423bc751b4254fba19090c1))

    - **mbedTLS**

      - update to 3.4.1 ([e686cdb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e686cdb450bbf01d42850457f83e45208a2655f8))
      - add deprecation notice ([267c106](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/267c106f02e6996071985adbe695406a4978e97f))

    - **mbedTLS-PSA**

      - initialise mbedtls psa crypto ([4eaaaa1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4eaaaa19299040cfee0585d7daa744dee716d398))
      - introduce PSA_CRYPTO build option ([5782b89](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5782b890d29646924d8bd3f46acdc73a6e02feb2))
      - mbedTLS PSA Crypto with ECDSA ([255ce97](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/255ce97d609a93ab5528a653735abc46c2627e8f))
      - register an ad-hoc PSA crypto driver ([38f8936](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/38f893692ad9b8edb5413f4b2b9cd15a9b485685))
      - use PSA crypto API during hash calculation ([484b586](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/484b58696d627c68869d86e2c401a9088392659e))
      - use PSA crypto API during signature verification ([eaa62e8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eaa62e825e31fb22a6245d9a5ab9cf5c9f8c0e46))
      - use PSA crypto API for hash verification ([2ed061c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2ed061c43525b8a9cd82b38d31277a8df594edd5))

  - **Measured Boot**

    - introduce platform function to measure and publish Public Key ([2971bad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2971bad8d48c6f0ddb7436efd16375bd72ade6bd))

  - **GUID Partition Tables Support**

    - add interface to init gpt ([f08460d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f08460dc085283f25fd6b5df792f263ccdf22421))
    - add support to use backup GPT header ([ad2dd65](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad2dd65871b4411c735271f98a4fa5102abb2a00))

  - **Arm**

    - **Ethos-N**

      - update npu error handling ([4796d2d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4796d2d9bb4a1c0ccaffa4f6b49dbb0f0304d1d1))

    - **RSS**

      - set the signer-ID in the RSS metadata ([60861a0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/60861a04e06d98ba6a9ae984cc5565f064fac9d1))

  - **ST**

    - **Clock**

      - allow aarch64 compilation of STGEN functions ([b1718c6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b1718c6382cff096c46dd216b5c99586eb303d29))
      - stub fdt_get_rcc_secure_state ([19c3808](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/19c38081d3cbb4062d8894e6c3ec3c4e1d01a767))

    - **UART**

      - add AARCH64 stm32_console driver ([c6d070c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c6d070cdba2c9a37b2253354f4cc3ba7e127e35d))

- **Miscellaneous**

  - **AArch64**

    - add stack debug information to assembly routines ([f832885](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f8328853031ab6dfc57059ff181138babc7779a0))

  - **DT Bindings**

    - add the STM32MP2 clock and reset bindings ([3ccb708](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3ccb708ecede0858c3c8633942dd9ceec1511fa5))

  - **FDTs**

    - **Morello**

      - add thermal framework ([0b22160](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0b221603e909cd493feeaab96d9c6f5458c628a8))

    - **STM32MP2**

      - add stm32mp257f-ev1 board ([9aa5371](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9aa5371f2fde18ed9ef466f3ee08e599bcdca2dd))
      - introduce stm32mp25 pinctrl files ([2c62cc4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c62cc4a879b3ca5414227a2ddcd965814f3d112))
      - introduce stm32mp25 SoCs family ([0dc283d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0dc283d29e4d962553046ea7ba30e90ea64f6d3d))

  - **TBBR**

    - add image id for backup GPT ([1051606](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1051606c3df3b5a0ebd4e4dad1e5e4a57e2f4d69))
    - update PK_DER_LEN for ECDSA P-384 keys ([c1ec23d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c1ec23dd60954582a9b5dd49e85b092e9ece0680))

- **Documentation**

  - introduce STM32MP2 doc ([ee5076f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ee5076f9716591333f1f5aa73b02c130c57917db))
  - save BL32 image base and size in entry point info ([31dcf23](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/31dcf2345172de50b098d7a080c65ee6faa87df8))
  - add a threat model for TF-A with Arm CCA ([4463541](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/446354122cea54255630d250064f5f889045acb0))
  - cover threats inherent to receiving data over UART ([348446a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/348446ad2a836f7fa0ab05cdf6142342a1c4a4b3))
  - add a section for experimental build options ([4885600](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/48856003bfaf8c8d0ce7b29e2e1262f7f1dfbb5d))

- **Build System**

  - include plat header in fdt build ([e03dcc8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e03dcc8f5ee2c2c48732745c5c364951eb36ceec))
  - manage patch version in Makefile ([055ebec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/055ebeca1b642ae69885a95e3c102f95d567a11e))
  - march option selection ([7794d6c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7794d6c8f8c44acc14fbdc5ada5965310056be1e))
  - pass CCA NV ctr option to cert_create ([0f19b7a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0f19b7aada428e0ca69d27ab016928b8fbc64a79))
  - .gitignore to include memory tools ([82257de](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/82257de06df2f744b12907079d5224bd56704de1))
  - allow gcc linker on Aarch32 platforms ([cfe6767](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cfe6767f7dd483f1bd76b2ba88a75809e013c5bd))
  - bump certifi to version 2023.7.22 ([6cbf432](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6cbf43204f3ca7cc6db621652da182743748af3f))
  - convert tabs and ifdef comparisons ([72f027c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/72f027c335a9e20e479e0d684132401546685616))
  - convert tabs to spaces ([1ca73b4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1ca73b4f4a0f6929a6649b4eb12e4ce45644a892))
  - disable ENABLE_FEAT_MPAM for Aarch32 ([a07b459](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a07b4590dd06c9e27ec6d403003bcf55afa9dc27))
  - include Cortex-A78AE cpu file for FVP ([b996db1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b996db168dcdac89245bb2cb60212e3e1b3ad061))
  - pass parameters through response files ([430be43](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/430be4396bbf779c9d2cac0ed8fefd07c7b8fde2))
  - remove duplicated include order ([c189adb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c189adbd5559a31078749fd3ddd483337ad609f6))
  - remove handling of mandatory options ([1ca902a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1ca902a537d622b9f7f53f872586120ae75e2603))

- **Tools**

  - **Firmware Image Package Tool**

    - add ability to build statically ([4d4fec2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d4fec281861066ab2249bc3db7c2decdd176f34))

  - **Secure Partition Tool**

    - generate `ARM_BL2_SP_LIST_DTS` file from `sp_layout.json` ([20629b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/20629b3153bccdda32116ed5c4861e61fa1fba95))

  - **Certificate Creation Tool**

    - add new option for CCA NV ctr ([60753a6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/60753a63290e255d6c4d34d0145ac00e8d69c9cf))
    - add pkcs11 engine support ([616b3ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/616b3ce27d9a8a83a189a16ff6a05698bc6df3c8))
    - ecdsa p384 key support ([c512c89](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c512c89cde91f10e1b283522ac956fa4da85a797))

  - **Memory Mapping Tool**

    - add tabular memory use data ([d9d5eb1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d9d5eb138ded8d4abeaf0cd1341ddf451aa299b8))
    - add topological memory view ([cc60aba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc60aba227e74a171c924146a0b745450af72f3d))

### Resolved Issues

- **Architecture**

  - **CPU feature / ID register handling in general**

    - move nested virtualization support to optionals ([8b2048c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8b2048c1c019d799d1806926724c2fbbc399c4c1))

  - **Memory Partitioning and Monitoring (MPAM) Extension (FEAT_MPAM)**

    - refine MPAM initialization and enablement process ([edebefb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/edebefbcbc01f4ab67a7838e0191736fd9ee0192))

  - **Performance Monitors Extension (FEAT_PMUv3)**

    - make MDCR_EL3.MTPME=1 out of reset ([33815eb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33815eb7194e662169676b2ce88ee4785aac9ccd))

- **Platforms**

  - register PLAT_SP_PRI only if not already registered ([bf01999](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf01999aba3949e810b7c66d3a164c4e3a964bf8))

  - **Arm**

    - add Event Log area behind Trustzone Controller ([d836df7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d836df71ea50e0863f7858f71b06653058e64140))
    - correct the SPMC_AT_EL3 condition ([a0ef1c0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a0ef1c0ef030e8fee8ad8f8a5f4a0fa911403a7c))
    - fix GIC macros for GICv4.1 support ([f1df8f1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1df8f10c6906519c54483f1f7a67f5cc507ec31))
    - add RAS_FFH_SUPPORT check for RAS EHF priority ([1c01284](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1c012840cab6529edbbc1bc7e3bcba11477a6955))
    - do not program DSU CLUSTERPWRDN register ([3209b35](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3209b35d2a372e71b96f3efbd7631d32518dc9b7))

    - **FPGA**

      - enable CPU features required for ARMv9.2 cores ([b321c24](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b321c243423b9341bc04e839a795ff31247eacd5))

    - **FVP**

      - adjust BL2 maximum size as per total SRAM size ([965aace](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/965aacea919525baa03308a5a08205e506be0bf4))
      - adjust BL31 maximum size as per total SRAM size ([24e224b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24e224b41cc6fda4b507861cf8e409d8e4a3f7cd))
      - conditionally increase XLAT and MMAP table entries ([03cf4e9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/03cf4e9aad2774ce221ccfe6f345ffcc8aabee4a))
      - extract core id from mpidr for pwrc operations ([70bc744](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/70bc74441b9901ee91ebb32be1def1e645374488))
      - increase maximum MMAP and XLAT entries count ([12fe591](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/12fe591b3e05255c167c5a9e21eaac2a9946f55c))
      - increase the maximum size of Event Log ([f1dfaa4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1dfaa42cf1a93523501ce694260d88acee7c0c0))
      - resolve broken workaround reference ([bcb3ea9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bcb3ea92f8626e48340bd65c7c3007953e0ee8f4))
      - update pwr_domain_suspend ([f51d277](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f51d277de3e5f84eafafb32596ca0b154d11c4d5))
      - update system suspend in OS-initiated mode ([e0ef05b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e0ef05bb2c260e0441186dd8647dea531bb1daf3))

    - **Morello**

      - configure platform specific secure SPIs ([80f8769](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/80f8769b26efcbce842d0ed62950603dfd83ef9b))

    - **N1SDP**

      - configure platform specific secure SPIs ([7b0c95a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7b0c95abc8e399a4a676647f4cffffa7ed21b3e6))
      - fix spi_ids range for n1sdp multichip boot ([31f60a9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/31f60a968347497562b0129134928d7ac4767710))

    - **SGI**

      - update PLAT_SP_PRI macro definition ([6f689a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6f689a51a577f740b341744e62c667733a79df94))

    - **TC**

      - Correct return type ([b0542b5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b0542b58ca77b922cf879dfb7d38356b32399c56))
      - rename macro to match PSA spec ([1fc20d7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1fc20d7f523e5c4bafb23584b1309ca432307ea4))

    - **Corstone-1000**

      - add cpu_helpers.S to platform.mk ([cb27274](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cb27274c9964deab3b613a48c1f293c122126ee5))
      - modify boot device dependencies ([3ff5fc2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3ff5fc2b35638afea2fad3cd0c76dcadc1adb8c2))
      - removing the signature area ([5856a91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5856a91a641a4cd7403143bb90b098855a77ac16))

  - **Aspeed**

    - **AST2700**

      - add device mapping for coherent memory ([cef2e92](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cef2e92568045da4e1d26a9ebfb38b0176b4ec33))

  - **Broadcom**

    - fix misspelled header inclusion guard ([a9779c1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a9779c11daa251abb9c523b4e01e6ef26c7d46fc))

  - **Cadence**

    - update console flush uart driver ([e27bebb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e27bebb0fe84bf58eed1fb61a65da9280309f24e))

  - **Intel**

    - fix ncore ccu snoop dvm enable bug ([106aa54](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/106aa54d922c8d0980c527530cbb417141fe3f83))
    - resolved coverity checking ([1af7bf7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1af7bf71c042add4f473c056f850a8a4792b6bbd))
    - update boot scratch cold register to use cold 8 ([655af4f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/655af4f49278476ebac6bb865e325eca865684f2))
    - update checking for memcpy and memset ([c418064](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c418064eb5ae2f223457e4a25a91f379e8cf5223))

  - **MediaTek**

    - support saving/restoring GICR registers ([f73466e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f73466e9a2fe35fc31a7a58a2e24308a9db341d7))

  - **NVIDIA**

    - **Tegra**

      - return correct error code for plat_core_pos_by_mpidr ([6bd79b1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6bd79b13f8a8566d047ff25da9110a887b4e36e7))

  - **NXP**

    - **i.MX**

      - **i.MX 8M**

        - make IMX_BOOT_UART_BASE autodetection option more obvious ([101f070](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/101f07022a0337b074c03e60078b94789bc766f6))
        - map BL32 memory only if SPD_opteed or SPD_trusty is enabled ([4827613](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4827613c9a8db6238e9411b508ef20bda3113146))

  - **QEMU**

    - fix 32-bit builds with stack protector ([e57ca89](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e57ca899efe414bd685e89e335a21d15a25b04f8))

    - **SBSA**

      - align FIP base to BL1 size ([408cde8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/408cde8a59080ac2caa11c4d99474b2ef09f90df))

  - **QTI**

    - **SC7280**

      - update pwr_domain_suspend ([a43be0f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a43be0f61003df1d8cf01bd706d5af305428c022))
      - update system suspend in OS-initiated mode ([0a9270a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0a9270abe82b396bf6fa15c7eb39c3499452686a))

  - **Renesas**

    - **R-Car**

      - add mandatory fields in 'reserved-memory' node ([f945498](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f945498faab3bd44f0f957931809de2f59517814))

      - **R-Car 3**

        - fix CPG register code comment ([69c371b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/69c371bc16533eb97a1d9bc408f9f17da87ba641))
        - update Draak and Eagle board IDs ([281edfe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/281edfee02bc72d81aa4972d60216647f932f3df))

  - **ST**

    - allow crypto lib compilation in aarch64 ([76e4fab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/76e4fab000825c4361b4b9843c6e0c2f4f6eb1fd))
    - enable RTC clock before accessing nv counter ([77ce6a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77ce6a561eae769419559632afa4d807a4fc33b6))
    - flush UART at the end of uart_read() ([a9cb7d0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a9cb7d002df4f09dce779b5b56640c2fdd77ba3b))
    - properly check LOADADDR ([9f72f5e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9f72f5eac81c23fe39415b2346b112f64fba8610))
    - reduce MMC block_buffer ([a2500ab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a2500ab7aba27ed5d613718f5f15371bbe895ca6))
    - setting default KEY_SIZE ([6f3ca8a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6f3ca8ada60addc601f685fa51619d2101d7406a))
    - update comment on encryption key ([5c506c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5c506c73751cc3f51df88826b89b5f729d8955c5))
    - update dt_get_ddr_size() type ([2a4abe0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2a4abe0b37f8d1987019c3de30e3301d8f8958d7))

    - **STM32MP1**

      - add void entry in plat_def_toc_entries ([8214ecd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8214ecdab22a72877dfff539eee31cfb92f36423))
      - properly check PSCI functions return ([241f874](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/241f8745459ae413ca22fcc0f1081da8de48796f))
      - use the BSEC nodes compatible for stm32mp13 ([2171bd9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2171bd9511258e7aebaa3ce2f9498093d3a3c63e))

  - **Texas Instruments**

    - align static device region addresses to reduce MMU table count ([53a868f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/53a868f676d9ad6ec37d69155241883b8e7bf0bf))
    - fix TISCI API changes during refactor ([d7a7135](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d7a7135d32a8c7da004c0c19b75bd4e2813f9759))
    - release lock in all TI-SCI xfer return paths ([e92375e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e92375e07cf54c2fbac6616e58116c98507ac177))
    - remove check for zero value in BL31 boot args ([44edd3b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44edd3bd7cfe1d5fb1599ab5eee9b81efea984e0))

  - **Xilinx**

    - add headers to resolve compile time issue ([744d60a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/744d60aab4e0173e21564fde092884c10267a6cc))
    - dcache flush for dtb region ([93ed138](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/93ed138006dc09e5b09222cabae8952dd5363ad2))
    - don't reserve 1 more byte ([c3b69bf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c3b69bf17bc0231b0dae613dc9e1e01e41f32236))
    - dynamic mmap region for dtb ([7ca7fb1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ca7fb1bf0873824531a6eee2da1214b61496b02))
    - remove clock_setrate and clock_getrate api ([e5955d7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e5955d7c63291a736efe75fb93effbc3fefb19fb))
    - remove console error message ([f9820f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f9820f21b8317fb3a08598452b252f7a6a2a4ad7))
    - update dtb when dtb address and tf-a ddr flow is used ([fdf8f92](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fdf8f929df078943c24154e25d9d7661139826b3))

    - **DCC (Debug Communication Channel)**

      - add dcc console unregister function ([0936abe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0936abe9b235dd996e9466288415bb994acbbe8f))
      - enable DCC also for crash console ([c6d9186](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c6d9186f60a08b4a44b1ecf38071eacdc9553ef6))

    - **Versal**

      - add missing irq mapping for wakeup src ([06b9c4c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/06b9c4c87df0b2a052e4f3330b86cc572c7bf885))
      - fix BLXX memory limits for user defined values ([f123b91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f123b91fddfcc882577590bbf4a54e1497ef9a64))
      - make pmc ipi channel as secure ([96eaafa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/96eaafa3f855ea9e0b6ce13a44f37fa9f1026207))
      - type cast addresses to fix integer overflow ([bfe82cf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bfe82cff6f6ab8e557e7ad7db8eae573f1fb02f3))
      - use correct macro name for ocm base address ([56afab7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/56afab73a852fd3e10e607d2d86dedc3bae3ff2d))

      - **Versal NET**

        - add redundant call to avoid glitches ([cebb7cc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cebb7cc110e02281060ec854a28a3bee382d8efa))
        - change flag to increase security ([e8efb65](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8efb65afb996c9832384c96b36aee3092b56a4b))
        - correct device node indexes ([66b5620](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/66b5620c873ef656f779a4c2d844b187ba474d9d))
        - don't clear pending interrupts ([fb73ea6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fb73ea6cc3f9f4f51195b416a0f803a72d81eff6))
        - fix BLXX memory limits for user defined values ([a80da38](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a80da3899a5eea6bc022c37101ac0b7d970846f7))
        - make pmc ipi channel as secure ([2c65b79](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c65b79e256ea5ead117efeaa5d39c3e53c83bdc))
        - use correct macro name for uart baudrate ([e2ef1df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e2ef1dfcdbef7e448e9dd96852ffb8489c187d34))

    - **ZynqMP**

      - do not export apu_ipi ([237c5a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/237c5a74a295d6306529be024aaa3d6af4b32898))
      - fix BLXX memory limits for user defined values ([8ce2fbf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8ce2fbffe37ddcab5071601f1b311ee82a56b7cc))
      - fix prepare_dtb() memory description ([3efee73](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3efee73d528578162b8eb046dce540f0c5f0041a))
      - fix sdei arm_validate_ns_entrypoint() ([3b3c70a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3b3c70a418522176f3a55d8e266e3968f7d4f832))
      - handling of type el3 interrrupts ([e8d61f7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8d61f7d91901f577030f6a45a71cf389b96d9dc))
      - make zynqmp_devices structure smaller ([7e3e799](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e3e79995a3c02871211dd0e983fb6e886a9c518))
      - remove unused headers ([6288636](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/62886363a16f0dcef3b6acdff0a96880cf9940ce))
      - resolve runtime error in TSP ([81ad3b1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/81ad3b14b95e019eaa8d89d444680c14ede4d8ab))
      - type cast addresses to fix overflow issue ([9129163](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/91291633a1c99736803f39edb21cad95a3517ee8))
      - validate clock_id to avoid OOB variable access ([abc79c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/abc79c275be764d76bd983837ffc487664182dac))

  - **Nuvoton**

    - fix typo in platform.mk ([c7efb78](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c7efb78f8edc8fa66bbe2f9bad390d29f6a43fb0))

- **Bootloader Images**

  - **BL2**

    - bl2 start address for RESET_TO_BL2+ENABLE_PIE ([d478ac1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d478ac16c9002114da1c4708a0efb083c494ce2f))

  - **BL31**

    - resolve runtime console garbage in next stage ([889e3d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/889e3d1c68e37dc9f75ae432703fa8ffc7259546))

  - **BL32**

    - always include arm_arch_svc in SP_MIN ([cd0786c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd0786c73e536a1d2507d77ce49e2ae2b8ee71a1))
    - avoid clearing argument registers in RESET_TO_SP_MIN case ([56055e8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/56055e87b0a756d4756a22ed26b855fbe7afe93c))

    - **TSP**

      - fix destination ID in direct request ([ed23d27](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ed23d274fae0b2787421a1b2558d7c1e9ebb07ab))
      - flush uart console ([ae074b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae074b369a25747acf98a23389e9d67b39738c71))

- **Services**

  - **RME**

    - **RMMD**

      - enable sme using sme_enable_per_world ([c0e16d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0e16d30ab70c51737f7a01a6b365d27c1a94f3b))

  - **SPM**

    - **EL3 SPM**

      - fix LSP direct message response ([c040621](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c040621dba5f4c097441e67c9fd99b9df174ba4e))
      - improve direct messaging validation ([48fe24c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/48fe24c50cd4990a76f88e89b77e71b9a90aec6c))

    - **EL3 SPMC**

      - avoid descriptor size calc overflow ([27c0242](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/27c02425089548786a18d355b15acccd51880676))
      - correctly account for emad_offset ([0c2583c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c2583c6fbfd03e70915554d4093e5f9148f3792))
      - fix incorrect CASSERT ([1dd79f9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1dd79f9e2316e5a7a78b0ad5a34ec50288338e6f))
      - only call spmc_shm_check_obj() on complete objects ([d781959](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d781959f81923bc3a59e77abd44df2fcc61f044e))
      - prevent total_page_count overflow ([2d4da8e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d4da8e265660ce7580219b51d5e79fd99ce1458))
      - remove experimental flag ([630a06c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/630a06c4c64f3a6804dd633081190241b1e78484))
      - use uint64_t for 64-bit type ([43318e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/43318e4a4dcc79935150de75fe5dccbb615f4719))
      - use version-dependent minimum descriptor length ([52d8d50](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52d8d506e715dbbeba0938cecd30ac6624d1dcfc))
      - validate descriptor headers ([56c052d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/56c052d31126c93b3c6782ea8e0c3348b5299b75))
      - validate memory address alignment ([327b5b8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/327b5b8b74faedefc45e861c797197cf6fbd6def))
      - validate shmem descriptor alignment ([dd94372](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dd94372d77ff107726a7be53318b5694f3309ddb))

    - **SPMD**

      - coverity scan issues ([b04343f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b04343f3c912c8abc1a37b0ebe461ab574959ecd))
      - fix FFA_VERSION forwarding ([76d53ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/76d53ee1aafca7ba908c7439670509107377b309))
      - perform G0 interrupt acknowledge and deactivation ([6c91fc4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c91fc44580415aaca4cbd774d4373475f33deb2))
      - relax use of EHF with SPMC at S-EL2 ([bb6d0a1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb6d0a174f76240728cd911130703e712520ce16))

  - **ERRATA ABI**

    - added Neoverse N2 to Errata ABI list ([7e030b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e030b376329a0466ffe7676be215770bb46d10f))
    - fix the rev-var for Cortex-A710 ([5c8fcc0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5c8fcc0ca7f5e6dc3aea947800e146fe0ffe9b84))
    - update the Cortex-A76 errata ABI struct ([92d5b50](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/92d5b501d4ba7e00e2ddfd546dc90b786966a352))
    - update the Cortex-A78C errata ABI struct ([7f2caec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f2caecdbc64d1fbd34942285e1194e85c5e8614))
    - update the neoverse-N1 errata ABI struct ([56747a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/56747a5caa50eedeb627795f1c37e0a14953c2bf))
    - update the Neoverse-N2 errata ABI struct ([80af87e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/80af87e476ec3dd1ad26d7a906da82268a29e2b5))

- **Libraries**

  - **CPU Support**

    - assert invalid cpu_ops obtained ([3f721c6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3f721c6edd20cef11c241a3ef84d94c06f5bebb4))
    - check for SME presence in Gelas ([0bbd432](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0bbd4329bf73b0da1ed69578c385dd36358e261e))
    - fix minor issue seen with a9 cpu ([af70470](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af704705c135f85b8b1eeda938e3dcdba3f6e561))
    - fix the rev-var for Cortex-A710 ([2bf7939](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2bf7939a7b313352deb6c6b77ee1316eff142a7c))
    - fix the rev-var of Cortex-X2 ([8ae66d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8ae66d624e2f7cae9577ff8f99e0a45e21fb353d))
    - fix the rev-var of Neoverse-V1 ([ab2b56d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ab2b56df266f73aa53ca348d7945b119e1ef71c7))
    - flush L2 cache for Cortex-A7/12/15/17 ([c5c160c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c5c160cdddd1c365a447c1fcd148fabb9014cce0))
    - integer suffix macro definition ([1a56ed4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1a56ed4b357e9023637c74c39c6885c558a737d2))
    - reduce generic_errata_report()'s size ([f43e09a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f43e09a12e4f4f32185d3e2accceb65895d1f16b))
    - revert erroneous use of override_vector_table macro in Cortex-A73 ([9a0c812](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9a0c81257ff116b2ca33f5b6737e0a000fb7e551))
    - update the fix for Cortex-A78AE erratum 1941500 ([67a2ad1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/67a2ad171d1fb604d4cba8fa7f92ccb66d1ef3f9))
    - update the rev-var for Cortex-A78AE ([c814619](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c814619a364aea3bd55b5ea238541864c0de7dab))
    - workaround for Cortex-A510 erratum 2080326 ([6e86475](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e86475d55fa2981bc342a0eb78b86be233d7718))
    - workaround for Cortex-A710 erratum 2742423 ([d7bc2cb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d7bc2cb4303088873a715bcaa2ac3e0096b9d7f2))
    - workaround for Cortex-X2 erratum 2742423 ([fe06e11](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fe06e118ab0837ff173f6b7e576dcc34b2d26bb1))
    - workaround for Cortex-X3 erratum 2070301 ([2454316](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2454316c2ae4411d0071d88c3db3c95598f12498))
    - workaround for Cortex-X3 erratum 2742421 ([5b0e443](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b0e4438d0e604e80ffff17d02e37cae0f4b2a8f))
    - workaround for Neoverse N2 erratum 2009478 ([74bfe31](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/74bfe31fd2c992d8e1e13bf396a9d5c136967ca5))
    - workaround for Neoverse N2 erratum 2340933 ([68085ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68085ad4827ac7daa39767d479d0565daa32cb47))
    - workaround for Neoverse N2 erratum 2346952 ([6cb8be1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6cb8be17a53f4e11880ba13b78fca15895281cfe))
    - workaround for Neoverse N2 erratum 2743014 ([eb44035](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eb44035cdec5d47d7eb3c904c8e5d8443b9dfcba))
    - workaround for Neoverse N2 erratum 2779511 ([12d2806](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/12d28067c9e76a78b148ed6fb94faf96de5e8502))
    - workaround for Neoverse V2 erratum 2331132 ([8852fb5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8852fb5b7d94229475446c81cfa58851bc2204ff))
    - workaround for Neoverse V2 erratum 2719105 ([b011402](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b01140256b5c0620cbde8e98c0df0e95343a3c71))
    - workaround for Neoverse V2 erratum 2743011 ([58dd153](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/58dd153cc88e832a6b019f1d4c2e6d64986ea69d))
    - workaround for Neoverse V2 erratum 2779510 ([ff34264](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff342643bcfaf20d61148b90a068694fa1c44dca))
    - workaround for Neoverse V2 erratum 2801372 ([40c81ed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40c81ed5335191fbe32466e56aa4fb6db1da466c))

  - **EL3 Runtime**

    - leverage generic interrupt controller helpers ([07f867b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07f867b12251235b8582bec38e9cf39a95703e77))
    - restrict lower el EA handlers in FFH mode ([6d22b08](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d22b089ffb1793d581fde4de76245397ad7d4ee))

    - **Context Management**

      - make ICC_SRE_EL2 fixup generic to all worlds ([5e8cc72](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5e8cc7278659820bcd64c243cbd89c131462314c))
      - set MDCR_EL3.{NSPBE, STE} explicitly ([99506fa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99506face112410ae37cf617b6efa809b4eee0ee))

    - **RAS**

      - remove RAS_FFH_SUPPORT and introduce FFH_SUPPORT ([f87e54f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f87e54f73cfee5042df526af6185ac6d9653a8f5))
      - restrict ENABLE_FEAT_RAS to have only two states ([970a4a8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/970a4a8d8c0d6894fe2fd483d06b6392639e8760))

  - **PSCI**

    - add optional pwr_domain_validate_suspend to plat_psci_ops_t ([d348861](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d34886140c74c0afc48ab20e63523505fcfb4b7d))

  - **SMCCC**

    - ensure that mpidr passed through SMC is valid ([e60c184](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e60c18471fc7488cc0bf1dc7eae3b43be77045a4))
    - pass SMCCCv1.3 SVE hint to internal flags ([b2d8517](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b2d851785f6c03cae4feb015fe69091582e18f5e))

  - **Translation Tables**

    - fix defects on the xlat library reported by coverity scan ([2974ad8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2974ad87b8561706176e113e2ec4457c919cb99a))
    - set MAX_PHYS_ADDR to total mapped physical region ([1a38aaf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1a38aafbff93e478aa6f9e19af1ed76024062a73))

- **Drivers**

  - **Authentication**

    - allow hashes of different lengths ([22a5354](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/22a53545aa37c06a1ffd0f3c15e870b256a41cb7))
    - don't overwrite pk with converted pk when rotpk is hash ([1046b41](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1046b41808b23b4079f04cad370646e05207ded5))

  - **Measured Boot**

    - don't strip last non-0 char ([b85bcb8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b85bcb8ec92126c238572ed7d242115125e411e1))

  - **MMC**

    - initialises response buffer with zeros ([b1a2c51](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b1a2c51a0820fce803431e6ee5bd078bb1a65b0d))

  - **MTD**

    - **NAND**

      - reset the SLC NAND ([f4d765a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f4d765a12815e3f4bd9c4dff5fd88661b3615114))

      - **SPI NAND**

        - add Quad Enable management ([da7a33c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/da7a33cf2f27545d9d290ff0c2ee1ec333b061bb))

  - **SCMI**

    - add parameter for plat_scmi_clock_rates_array ([ca9d6ed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ca9d6edc892165c38f1b2710b537c10d4a57062d))

  - **UFS**

    - performs unsigned shift for doorbell ([e47d8a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e47d8a58b0d5745c943c36fad2ec8a98af709bea))
    - set data segment length ([9d6786c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9d6786cacee7c0eff33d1cec42c09c7002dd83d2))

  - **Arm**

    - **GIC**

      - **GICv3**

        - map generic interrupt type to GICv3 group ([632e5ff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/632e5ffeb8f50a98090065b63d9d071b72acd23c))
        - move invocation of gicv3_get_multichip_base function ([36704d0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/36704d09c6b26045fe2d18530a020ed23d74593d))

        - **GIC-600**

          - fix gic600 maximum SPI ID ([69ed7dc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/69ed7dc2e964c66eb8ff926a63a47b701ae1f3c6))

  - **Renesas**

    - **R-Car3**

      - update DDR setting ([138ddcb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/138ddcbf4d330d13a11576d973513014055f98c1))

  - **ST**

    - **Clock**

      - disabling CKPER clock is not functional on stm32mp13 ([1bbcb58](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1bbcb58a69c4ee2ee13e9d5de4499438ca08b149))

    - **Crypto**

      - do not read RNG data if it's not ready ([53092a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/53092a7780fa3d1b926aae8666f1c5a19cb039f1))
      - use GENMASK_32 to define PKA registers masks ([379d77b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/379d77b3705b0f3a88332663bba956289cad5797))

    - **DDR**

      - express memory size with size_t type ([b4e1e8f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b4e1e8fbf0dde5679d6b3717b8579f7a3343fdf8))

    - **UART**

      - allow 64 bit compilation ([6fef0f6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6fef0f67e47b3b42fc9b5dbc55bdef00a970765d))
      - correctly check UART enabled in flush fonction ([a527380](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a5273808aa1a4514f7849ca91b7859e15bf82bff))
      - skip console flush if UART is disabled ([b156d7b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b156d7b1cca1542f0c1c6f5d4354c43e048dc4a0))

- **Miscellaneous**

  - **AArch32**

    - disable workaround discovery on aarch32 for now ([d1f2748](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d1f2748ed25748237e894c68c5a163326a8c33b9))

  - **FDTs**

    - **STM32MP1**

      - move /omit-if-no-ref/ to overlay files ([f351f91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f351f9110f29a33923780c40d0896832fdb0ac81))

      - **STM32MP13**

        - correct the BSEC nodes compatible ([85c2ea8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/85c2ea8fd325797a44e814b575611aafae9e7613))
        - cosmetic fixes in PLL nodes ([8b82663](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8b826636a39e0f20cc2c0557288b1eeab46fb923))

  - **SDEI**

    - ensure that interrupt ID is valid ([a7eff34](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a7eff3477dcf3624c74f5217419b1a27b7ebd2aa))

  - **TBBR**

    - guard defines under MBEDTLS_CONFIG_FILE ([81c2e15](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/81c2e1566dc4484c23d293961744489a9a6ea3f0))
    - unrecognised 'tos-fw-key-cert' option ([f1cb5bd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1cb5bd19034407f2de7cad23f2cc52ca924e561))

- **Documentation**

  - match boot-order size to implementation ([fd1479d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fd1479d9194d3f3ec98d235e077c9d6e24276fa2))
  - add missing line in the fiptool command for stm32mp1 ([d526d00](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d526d00a13f86bbd2c073c065b6e9aff339e1b41))
  - fix build errors for latexpdf ([443d6ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/443d6ea69992986f56246bcee44e537ab8dec069))
  - remove out-dated information about CI review comments ([74306b2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/74306b2ac8971693d148b34d02c556d94b3e4926))
  - replace deprecated urls under tfa/docs ([5fdf198](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5fdf198c117a4b6dbcf5242f5136f7224ceff6ff))
  - update maintainers list ([9766f41](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9766f41d3c4cae4cd515c2f9266bb7adb4725349))
  - updated certain Neoverse N2 erratum status in docs ([d6d34b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d6d34b39132425dfa8c75352711c463d2989a216))
  - use rsvg-convert as the conversion backend ([c365476](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c3654760033c08e7ffa9337e05c48336032eacb9))

- **Tools**

  - **Firmware Image Package Tool**

    - move juno plat_fiptool.mk ([570a230](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/570a23099c32cafcb63ecb6cc0516d76ea099daf))

  - **Certificate Creation Tool**

    - fix key loading logic ([bb3b0c0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb3b0c0b09ff1d969ddd49b99642740ce2a07064))
    - key: Avoid having a temporary value for pkey in key_load ([ea6f845](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea6f8452f6eb561a0fa96a712da93fcdba40cd9c))

  - **Memory Mapping Tool**

    - reintroduce support for GNU map files ([d0e3053](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d0e3053c4f5b9d2bc70daf4db3c71f99c6da216d))

## [2.9.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.8.0..refs/tags/v2.9.0) (2023-05-16)

### ⚠ BREAKING CHANGES

- **Libraries**

  - **EL3 Runtime**

    - **RAS**

      - The previous RAS_EXTENSION is now deprecated. The equivalent functionality can be achieved by the following 2 options:
         - ENABLE_FEAT_RAS
         - RAS_FFH_SUPPORT

        **See:** replace RAS_EXTENSION with FEAT_RAS ([9202d51](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9202d51990c192e8bc041e6f53d5ce63ee908665))

- **Drivers**

  - **Authentication**

    - unify REGISTER_CRYPTO_LIB

      **See:** unify REGISTER_CRYPTO_LIB ([dee99f1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dee99f10b1dcea09091f4a1d53185153802dfb64))

  - **Arm**

    - **Ethos-N**

      - The Linux Kernel NPU driver can no longer directly configure and boot the NPU in a TZMP1 build. The API version has therefore been given a major version bump with this change.

        **See:** add protected NPU firmware setup ([6dcf3e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6dcf3e774457cf00b91abda715adfbefce822877))

      - Building the FIP when TZMP1 support is enabled in the NPU driver now requires a parameter to specify the NPU firmware file.

        **See:** load NPU firmware at BL2 ([33bcaed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33bcaed1211ab27968433b546979687bc1182630))

- **Build System**

  - BL2_AT_EL3 renamed to RESET_TO_BL2 across the repository.

    **See:** distinguish BL2 as TF-A entry point and BL2 running at EL3 ([42d4d3b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/42d4d3baacb3b11c68163ec85de1bf2e34e0c882))

  - check boolean flags are not empty

    **See:** check boolean flags are not empty ([1369fb8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1369fb82c8e809c1a59a0d99184dbfd2d0b81afa))

  - All input and output linker section names have been prefixed with the period character, e.g. `cpu_ops` -> `.cpu_ops`.

    **See:** always prefix section names with `.` ([da04341](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/da04341ed52d214139fe2d16667ef5b58c38e502))

  - The `EXTRA_LINKERFILE` build system variable has been replaced with the `<IMAGE>_LINKER_SCRIPT_SOURCES` variable. See the commit message for more information.

    **See:** permit multiple linker scripts ([a6ff006](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a6ff0067ab57d848d3fb28a3eb2b47e6cf2a6092))

  - The `LINKERFILE`, `BL_LINKERFILE` and `<IMAGE_LINKERFILE>` build system variables have been renamed. See the commit message for more information.

    **See:** clarify linker script generation ([8227493](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/82274936374bf630bf5256370e93a531fdda6372))

### Resolved Issues

- **Architecture**

  - **CPU feature / ID register handling in general**

    - context-switch: move FGT availability check to callers ([de8c489](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/de8c489247458c00f7b48301fb5c5273c7a628fc))
    - make stub enable functions "static inline" ([d7f3ed3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d7f3ed3655b85223583d8c2d9e719f364266ef26))
    - resolve build errors due to compiler optimization ([e8f0dd5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8f0dd58da231b81ba0ce6f27aaf1e31b4d4c429))

  - **Memory Partitioning and Monitoring (MPAM) Extension (FEAT_MPAM)**

    - feat_detect: support major/minor ([1f8be7f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1f8be7fc66fb59b197dde3b4ea83314b1728c6b8))
    - remove unwanted param for "endfunc" macro ([0e0bd25](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0e0bd250ef08ba70b34db9eb0cab0f6ef4d08edf))
    - run-time checks for mpam save/restore routines ([ed80440](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ed804406bf2ee04bde1c17683cec6f679ea1e160))

  - **Pointer Authentication Extension**

    - make pauth_helpers linking generic ([90ce8b8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/90ce8b8718d079b9e906d06bdd6a72da6cc5b636))

  - **Performance Monitors Extension (FEAT_PMUv3)**

    - switch FVP PMUv3 SPIs to PPI ([d7c455d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d7c455d8cca85de4a520da33db6523c9c8a7ee38))
    - unconditionally save PMCR_EL0 ([1d6d680](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d6d6802dd547c8b378a9a47572ee72e68cceb3b))

  - **Scalable Matrix Extension (FEAT_SME, FEAT_SME2)**

    - disable SME for SPD=spmd ([2fd2fce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2fd2fcedff0595a7050328fa60dc4850d6b424bf))

  - **Statistical profiling Extension (FEAT_SPE)**

    - drop SPE EL2 context switch code ([16e3ddb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/16e3ddba1f049106387dfe21989243d2fc4cf061))

- **Platforms**

  - **Allwinner**

    - check RSB availability in DT on H6 ([658b315](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/658b3154d5b06a467b65cb79d31da751ffc6f5a4))

  - **Arm**

    - arm_rotpk_header undefined reference ([95302e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/95302e4b234589e0487996a5c0f1e111c21ffedc))

    - **A5DS**

      - add default value for ARM_DISABLE_TRUSTED_WDOG ([115ab63](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/115ab63872ab36f8202f4c4aab093c4e9182d4e7))

    - **CSS**

      - fix invalid redistributor poweroff ([60719e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/60719e4e0965aead49d927f12bf2a37bd2629012))

    - **FPGA**

      - include missing header file ([b7253a1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b7253a14cdc633a606472ec4e5aa4123158e2013))

    - **FVP**

      - correct ehf priority for SPM_MM ([fb2fd55](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fb2fd558d8102ad79e5970714e0afec31a6138d7))
      - incorrect UUID name in FVP tb_fw_config ([7f2bf23](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f2bf23dec6e6467704d7d71ec44bee030912987))
      - unconditionally include lib/psa headers ([72db458](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/72db45852d84db6ade0da2a232a44df3e5228b6d))
      - work around BL31 progbits exceeded ([138221c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/138221c2457b9d04101b84084c07d576b0eb5a51))
      - work around DRTM_SUPPORT BL31 progbits exceeded ([7762e5d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7762e5d0ed5c28b0a77dc25cc566cf54a69af7e6))

    - **Morello**

      - add platform-specific power domain functions ([02a5bcb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/02a5bcb0bc3c8596894b6d0ec8c979b330db387a))

    - **N1SDP**

      - add platform-specific power domain functions ([5bdafc4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5bdafc4099b446609965f9132e6c52a7bdeb9ac8))

    - **RD**

      - **RD-N1 Edge**

        - change variable type to fix gcc sign conversion error ([3a3e0e5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3a3e0e5371e99b3764fd8e8d98a447911f3bb915))

    - **TC**

      - increase TC_TZC_DRAM1_SIZE ([7e3f6a8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e3f6a87d74efec780c0832c0535dd64ef830cfa))
      - change the FIP offset to 8 KiB boundary ([d07b8aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d07b8aac39abc3026233e316686f4643d076f8d6))
      - change the properties of optee reserved memory ([2fff46c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2fff46c80fe4aa27cd55ad4bfbe43c3823095259))
      - enable dynamic feature detection of FEAT_SVE for NormalWorld ([67265f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/67265f2f6d7604147080033a1c99150e9a020f28))
      - enable the execution of both platform tests ([657b90e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/657b90ea1aa2831a7feed31f07fc8e92213e6465))
      - only suspend booting after running plat tests ([9b26655](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9b266556d308c0af6f932fedd1c41fbda05204aa))
      - unify TC ROM start addresses ([f9e11c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f9e11c724bb7c919dc9bd5dd8fca1e04140374d2))
      - update the name of mbedtls config header ([d5fc899](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d5fc8992c7b63675b6fc4b2c00a1e1acfdaaeee2))

  - **Broadcom**

    - add braces around bodies of conditionals ([9f58bfb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9f58bfbbe90d2891c289cd27ab7d2ede8b5572d4))

  - **Intel**

    - add mailbox error return status for FCS_DECRYPTION ([76ed322](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/76ed32236aa396cb0e15eb049bea03710ca1992d))
    - agilex bitstream pre-authenticate ([4b3d323](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4b3d323acdd21d8853e38e135bf990b3767ca354))
    - fix Agilex and N5X clock manager to main PLL C0 ([5f06bff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f06bffa831638fd95d2160209000ef36d2a22ce))
    - fix fcs_client crashed when increased param size ([c42402c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c42402cdf8a3dfc6f6e62a92b2898066e8cc46f6))
    - fix pinmux handoff bug on Agilex ([e6c0389](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e6c038909193b83bc293de9b1eb65440e75f8c91))
    - fix print out ERROR when encounter SEU_Err ([1a0bf6e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1a0bf6e1d8fe899359535c0a0a68c2be5e5acaf4))
    - fix sp_timer0 is not disabled in firewall on Agilex ([8de7167](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8de7167eb661ff730a79bd2c6db15c22fdc62c8a))
    - fix the pointer of block memory to fill in and bytes being set ([afe9fcc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/afe9fcc3d262ca279a747c8ab6fa8bacf79c76fb))
    - flash dcache before mmio read ([731622f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/731622fe757ab2bcc0492ad27bafecf24206ddac))
    - mailbox store QSPI ref clk in scratch reg ([7f9e9e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f9e9e4b40152c0cb52bcc53ac3d32fd1c978416))
    - missing NCORE CCU snoop filter fix in BL2 ([b34a48c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b34a48c1ce0dd7e44eac4ceb0537b337857b057f))
    - remove checking on TEMP and VOLT checking for HWMON ([68ac5fe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68ac5fe14c0220673d7ee88a99b3d02be1fef530))
    - update boot scratch to indicate to Uboot is PSCI ON ([7f7a16a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f7a16a6c0a49af593fa080eb66f72a20bb07299))

  - **NVIDIA**

    - **Tegra**

      - append major revision to the chip_id value ([33c4766](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33c476601cf48a4b02259b8cb43819acd824804f))
      - remove dependency on CPU registers to get boot parameters ([0b9f05f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0b9f05fcaea069bff6894d99ec5babc4be29ca67))

      - **Tegra 210**

        - support legacy SMC_ID 0xC2FEFE00 ([40a4e2d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40a4e2d84c38ffae899eaa2c33c1e280312919cf))

  - **NXP**

    - **i.MX**

      - **i.MX 8M**

        - add ddr4 dvfs sw workaround for ERR050712 ([e00fe11](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e00fe11df3fee04c7f3137817294d464466dab22))
        - backup mr12/14 value from lpddr4 chip ([a2655f4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a2655f48697416b8350ba5b3f7f44f1f0be79d4e))
        - correct the rank info get fro mstr ([5277c09](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5277c09606450daaffa43f3cf15fcc427d7ba612))
        - fix coverity out of bound access issue ([0331b1c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0331b1c6111d198195298a2885dbd93cac1ad26a))
        - fix the current fsp init ([25c4323](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/25c43233e866326326f9f82bfae03357c396a99f))
        - fix the dfiphymaster setting after dvfs ([ad0cbbf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad0cbbf513dfabe51a401c06be504e57d6b143ca))
        - fix the dram retention random hang on some imx8mq Rev2.0 ([4bf5019](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4bf5019228cf89e0cbc2cd03627f755d51e3e198))
        - fix the rank to rank space issue ([3330084](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3330084979e4c1a39a92f0642000664c79a00dda))

        - **i.MX 8Q**

          - fix compilation with gcc >= 12.x ([e75a3b6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e75a3b6e89c4bce11d1885426f22262def9bd664))

    - **Layerscape**

      - fix errata a008850 ([c45791b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c45791b2f20909c9a9d2bae84dafc17f55892fc8))
      - fix nv_storage assert checking ([5d599b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d599b71ea6e0020f4f9d0e7af303726483217bc))
      - unlock write access SMMU_CBn_ACTLR ([0ca1d8f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ca1d8fba3bee32242b123ae28ad5c83a657aa0d))

      - **LX2**

        - init global data before using it ([50aa0ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/50aa0ea7acd21e7e9920a91a14db14a9f8c63700))

      - **LS1046A**

        - 4 keys secureboot failure resolved ([c0c157a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0c157a680fcb100afed3e1ea9d342deea72ea05))

  - **QEMU**

    - enable dynamic feature detection of FEAT_SVE for NormalWorld ([fc259b6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fc259b6c3a551efbc810c8e08e82b7b5378f57ba))

    - **SBSA**

      - enable FGT ([c598692](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c598692d0c6a79dd10c34d5a4a740c90261cfc65))
      - enable SVE and SME ([9bff7ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9bff7ce37545162d417953ac36c6878216815b94))

  - **QTI**

    - **MSM8916**

      - add timeout for crash console TX flush ([7e002c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e002c8a13172c44f55ab49062861479b6622884))
      - drop unneeded initialization of CNTACR ([d833af3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d833af3ab50cd2cfecb8868c3d5340df1572f042))
      - flush dcache after writing msm8916_entry_point ([01ba69c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/01ba69cd9b833047653186858a6929e6c9379989))
      - print \r before \n on UART console ([3fb7e40](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3fb7e40a21b1570a8ce1cd1708134fa7a05d94fb))

  - **Raspberry Pi**

    - **Raspberry Pi 3**

      - initialize SD card host controller ([bd96d53](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bd96d533dc28c4c938aa54905787688823cbccac))

  - **Renesas**

    - align incompatible function pointers ([90c4b3b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/90c4b3b62d5303c22fdc5f65f0db784de0f4ac95))

  - **Rockchip**

    - use semicolon instead of comma ([8557d49](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8557d491b6dbd6cbf27cc2ae6425f6cb29ca2c35))

  - **ST**

    - add U suffix for unsigned numbers ([9c1aa12](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9c1aa1253c9c77487b73d46a89941e81e80864eb))
    - explicitly check operators precedence ([56048fe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/56048fe215997ab6788ebd251e8cde094392dfc7))
    - include utils.h to solve compilation error ([377846b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/377846b65e8eb946a6560f1200ca4ca0e1eb8b99))
    - make metadata_block_spec static ([d1d8a9b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d1d8a9bad0be53792e219625b0d327cc4855378f))
    - rework secure-status check in fdt_get_status() ([0ebaf22](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ebaf222899c1c33fe8bd0e69bd2c287ebe1154b))
    - use Boolean type for tests ([45d2d49](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/45d2d495e77c9c8f3e80774e48a80e4882c8ac0d))
    - use indices when counting GPIOs in DT ([e7d7544](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e7d75448b9e46dee22fe23b37c28a522b9ec3a6c))

    - **STM32MP1**

      - add const for strings in stm32mp_get_soc_name() ([d7f5bed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d7f5bed90eaacee0a223bcf23438dfb76dee08e6))
      - add missing platform.h include ([6e55f9e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e55f9e2cde0426c39ccda87b00047f85d30f97d))
      - always define PKA algos flags ([e0e2d64](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e0e2d64f47654e4d86d0e400977eab0e4a01523e))
      - remove boolean check on PLAT_TBBR_IMG_DEF ([231a0ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/231a0adb6abc35c125d4177749af37042575eca2))
      - rework DWL buffer cache invalidation ([127ed00](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/127ed0008e03abb98b5447cb80c5634dfa554e7d))

  - **Texas Instruments**

    - do not take system power reference in bl31_platform_setup() ([9977948](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9977948112d732935362a3fe8518e3b2e4b7f6b7))
    - fix typo in boot authentication message name ([81f525e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/81f525ecc75a3d8b344a27881098fcaab65f2d8f))

  - **Xilinx**

    - fix misra defects ([964e559](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/964e55928c8f966633cc57e41987aa00890f5da7))
    - handle CRC failure in IPI ([5e92be5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5e92be5121e8ecd81a0f89eaae0d1a7ac8f4bfd7))
    - handle CRC failure in IPI callback ([6173d91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6173d914d673249ec47c080909c31a1654545913))
    - initialize values to device enum members ([5c62d59](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5c62d599274b5d9facd4996b50c1a1e153b247a4))
    - remove asserts around arg0/arg1 ([8be2044](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8be20446706c6b2fe911804385f308817495d2d4))
    - remove unnecessary condition ([c984123](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c984123669a7ba7b8d1dc168db8e130ee52bbb1e))
    - remove unused mailbox macros ([15f49cb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/15f49cb49d7daf2cd771c80d3dd80ff15874b40b))
    - resolve integer handling issue ([4e46db4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4e46db40fc86ddc0556c42ba01198d13002fcf14))
    - use lib/smccc.h macros instead of trusty spd ([0ee07d7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ee07d796cece8074eb296415c88872504dee682))

    - **Versal**

      - check smc_fid 23:16 bits ([4a50363](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4a50363aaeaa16edafcff17486006049b30e1e2f))
      - fix incorrect regbase for PMC IPI ([c4185d5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c4185d5103080621393edb770a56aa274f9af1a7))
      - initialize the variable with value 0 in pm code ([cd73d62](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd73d62b0e0920ca4e6c4fea7ab65bcbd63e07de))
      - print proper atf handoff source ([0fe002c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0fe002c9be899f005316ea196ad4c6b08815d482))
      - replace FPD_MAINCCI* macros ([245d30e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/245d30efe617af68c674b411d63c680dca1c21dd))
      - sync location based on IPI_ID macros ([92a43bd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/92a43bdf366502c6919bbd2c8e4f687c51d9738c))

      - **Versal NET**

        - fix irq for IPI0 ([95bbfbc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/95bbfbc6e0789cba871e2518dba76ff9bf712331))
        - clear power down bit during wakeup ([5f0f7e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f0f7e47e05f98587d424c2162d1ce20af4f588d))
        - clear power down interrupt status before enable ([2d056db](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d056db4e4981e0f8a58de0d1e44e46058b308f4))
        - correct aff level for cpu off ([6ada9dc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6ada9dc325aaa29e2f4c87575093401197856639))
        - disable wakeup interrupt during client wakeup ([e663f09](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e663f09b3cc2a3c933191c110557c6ffe5db6d6c))
        - enable wake interrupt during client suspend ([39fffe5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/39fffe552fb04028de750e6080d9a8ba46e89b8c))
        - fix setting power down state ([1f79bdf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1f79bdfd9ae105135a0192017d6f9368045228e9))
        - populate gic v3 rdist data statically ([355dc3d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/355dc3d4deacf73a3d354682bcda454e6d13ed66))
        - resolve misra 10.6 warnings ([8c23775](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8c23775e88bfc4ffa2b0eaf815d4f79992d344e6))
        - resolve misra rule 20.7 warnings ([21d1966](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21d1966a23b57425a400730270c8694e37b1a85c))
        - use spin_lock instead of bakery_lock ([0b3a2cf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0b3a2cf0226878ad7098cc6cd1a97ade74fd9c38))

    - **ZynqMP**

      - add bitmask for get_op_char API ([ad4b667](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad4b667d3ba7ece4cf28106aef6f91259b5b06ee))
      - check return status of pm_get_api_version ([c92ad36](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c92ad369ca3a548ecbf30add110b1561fe416c10))
      - check smc_fid 23:16 bits ([09b342a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/09b342a9d3aa030bde6d52e39203b9b8c8e6b106))
      - conditional reservation of memory in DTB ([c52a142](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c52a142b7ceb397b4d66cc90f2bc717acc7263cd))
      - enable A53 workaround(errata 1530924) ([d8133d7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d8133d7785969b417cbace293db6393c55844fac))
      - fix bl31_zynqmp_setup.c coding style ([26ef5c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/26ef5c29c62def3a21591dd216180d86063acdb4))
      - fix DT reserved allocated size ([2c03915](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c03915322ede112030fcfb8097d4697b92fcc2f))
      - fix xck24 silicon ID ([f156590](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f156590767d5f80e942fa3f88a9b6a94c13ceb55))
      - initialize uint32 with value 0U in pm code ([e65584a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e65584a017fadf002d5bdd1e95527c48610a6963))
      - move EM SMC range to SIP range ([acbae39](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/acbae3998bd829ae4b31ea9da59055e3624991a5))
      - panic w/o handoff structure in !JTAG ([fbe4dbe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fbe4dbeec906038795f72d8f9284a812bd6a852d))
      - remove redundant api_version check ([d0b58c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d0b58c8a9bff3cabfdb59e052ab7eaecfe64b305))
      - remove unused PLAT_NUM_POWER_DOMAINS ([72c3124](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/72c3124f584609275424bf52a20fd707d4f1af6a))
      - separate EM from PM SMCs ([a911396](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a9113966c35af281e9c8972b1209646963ff55d0))
      - update MAX_XLAT_TABLES for DDR memory range ([12446ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/12446ce89e351959aebb610eb2e35cdc7eb84d26))
      - update the conflicting EEMI API IDs ([bcc1348](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bcc1348b6bb2fcd987c8f047fa9f526f32768258))
      - with DEBUG=1 move bl31 to DDR range ([2537f07](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2537f0725ee7d8f46bef3e5b49134419b5c3367b))

- **Bootloader Images**

  - **BL31**

    - avoid clearing of argument registers in RESET_TO_BL31 case ([3e14df6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3e14df6f63303adb134d525b373ec7f08c1b1dc6))

  - **BL32**

    - **TSP**

      - loop / crash if mmap of region fails ([8c353e0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8c353e0058e95cfa20c9a760ebd0908a9a9aa1c1))
      - use verbose for power logs ([3354915](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3354915fff2ad5f97551c22a44a90f4ff7b7cc9b))

- **Services**

  - **RME**

    - update sample platform attestation token ([19c1dce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/19c1dcef88cb837abe175b89739e75e27539a561))

    - **TRP**

      - preserve RMI SMC X4 when not used as return ([b96253d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b96253db08383c3edfb417c505c8da6f7b1dbe75))

    - **RMMD**

      - add missing padding to RMM Boot Manifest and initialize it ([dc0ca64](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dc0ca64e4b6c86090eee025293e7ae7f1fe1cf12))

  - **SPM**

    - **EL3 SPMC**

      - fix coverity scan warnings ([1543d17](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1543d17b9876add1cb89c1f5ffe0e6a129f5809e))
      - improve bound check for descriptor ([def7590](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/def7590b3e34ff69b297c239cb8948d0bdc9c691))
      - report execution state in partition info get ([62cd8f3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/62cd8f3147ed7fb146168c59cab3ba0e006210ad))

    - **SPMD**

      - fix build error with spmd ([fd51b21](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fd51b21573ec2e0d815caecb89cc323aac0fca6d))

- **Libraries**

  - **CPU Support**

    - do not put RAS check before using esb ([9ec2ca2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9ec2ca2d453176179f923d7e0fbaac05341ebdc6))
    - use hint instruction for "tsb csync" ([7a181b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7a181b7d046a710db5238fb37047816636d2bb8a))
    - workaround for Cortex-A510 erratum 2684597 ([aea4ccf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aea4ccf8d9f3eabbc931f0e82df65ffca28c25e5))
    - workaround for Cortex-A710 erratum 2282622 ([89d85ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/89d85ad0aad4fef7f56a9e18968b49e2b843ca9d))
    - workaround for Cortex-A710 erratum 2768515 ([b87b02c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b87b02cf1d93f2be2113192cd5f1927e33121a80))
    - workaround for Cortex-A78 erratum 2742426 ([a63332c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a63332c517ac5699644d3e2fbf159d3e35c32549))
    - workaround for Cortex-A78 erratum 2772019 ([b10afcc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b10afcce5ff1202e1cd922dbd3c1e5980b478429))
    - workaround for Cortex-A78 erratum 2779479 ([7d1700c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d1700c4d475358539c9a84cb325183c86a06f33))
    - workaround for Cortex-A78C erratum 1827430 ([672eb21](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/672eb21e26a41657b8146372d4283e794b430c5f))
    - workaround for Cortex-A78C erratum 1827440 ([b01a59e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b01a59eb2a0456ca3ae6b8d020068ba846f813d4))
    - workaround for Cortex-A78C erratum 2772121 ([00230e3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/00230e37e3c21fed4a46eeb69dea9d808f8402b4))
    - workaround for Cortex-A78C erratum 2779484 ([66bf3ba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/66bf3ba482e46137e19f368f1386436a33eaba74))
    - workaround for Cortex-X2 erratum 2282622 ([f9c6301](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f9c6301d743405bd91b9a1fe433ce14fa60a830f))
    - workaround for Cortex-X2 erratum 2768515 ([1cfde82](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1cfde82227558a8cc1792c068bc7a7cdf8feab43))
    - workaround for Cortex-X3 erratum 2615812 ([c7e698c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c7e698cfdedbe2b1c8212dd71477f289f7644953))
    - workaround for Neoverse N2 erratum 2743089 ([1ee7c82](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1ee7c8232c153203d104f148a33e6f641d503f96))
    - workaround for Neoverse V1 errata 2743233 ([f1c3eae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1c3eae9e091a63b42eebae8b03d4d470c9c3f75))
    - workaround for Neoverse V1 errata 2779461 ([2757da0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2757da06149238041308060e5cb51f0870a02a15))
    - workaround for Neoverse V1 erratum 2743093 ([31747f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/31747f057b13b5934b607b7021139e58a55f7766))
    - workaround platforms non-arm interconnect ([ab062f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ab062f0510d42b2019667e3f4df82a1f57121412))

  - **EL3 Runtime**

    - allow SErrors when executing in EL3 ([1cbe42a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1cbe42a510812a4a4415a26ba46821cad1c04b68))
    - do not save scr_el3 during EL3 entry ([e61713b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e61713b00715fc988a970687f9bf53418b81b0ca))
    - restore SPSR/ELR/SCR after esb ([ff1d2ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff1d2ef387f085fdada4a122284b3b044fdde09c))

    - **RAS**

      - do not put RAS check before esb macro ([7d5036b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d5036b8ec911d83ede6eb73f1693b6f160d90ed))

  - **FCONF**

    - fix FCONF_ARM_IO_UUID_NUMBER value ([e208f32](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e208f3244b311a23b3e7fa1c03b3e98a6228714a))
    - make struct fconf_populator static ([40e740d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40e740dc14e807455d8db99dc758af355aa7fa8f))

  - **OP-TEE**

    - address late comments and fix bad rc ([8d7c80f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8d7c80fa4c5ab17e25d6d82ff0b1e67795e903fb))
    - return UUID for image loading service ([85ab882](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/85ab88238183be1e27835e14e3588fb73e0f6aa7))

  - **PSCI**

    - do not panic on illegal MPIDR ([8a6d0d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8a6d0d262ae03db0a0bedd047a2df6f95e8823f6))
    - potential array overflow with cpu on ([6632741](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/66327414fb1e3248d443f4eb2835f437625fb92c))
    - remove unreachable switch/case blocks ([ad27f4b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad27f4b5d918bbd1feb9a2deed3cb0e2ae39616e))
    - tighten psci_power_down_wfi behaviour ([695a48b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/695a48b5b4366d1005f8b9a0fc83726914668fb5))

  - **GPT**

    - fix compilation error for gpt_rme.c ([a0d5147](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a0d5147b8282374e107461421bb229272fde924b))

  - **SMCCC**

    - check smc_fid [23:17] bits ([f8a3579](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f8a35797b919d8ea041480bd5eb2a334e7056e0b))

  - **C Standard Library**

    - properly define SCHAR_MIN ([06c01b0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/06c01b085fb28fcfe26d747da2ba33415dbd52b9))
    - remove __putchar alias ([28dc825](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28dc82580e50961f9b76933b20d576a6afc5035c))

  - **Context Management**

    - enable SCXTNUM access ([01cf14d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/01cf14dd41cae9c68cb5e76a815747a0d2a19a4a))

- **Drivers**

  - **Authentication**

    - avoid out-of-bounds read in auth_nvctr() ([abb8f93](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/abb8f936fd0ad085b1966bdc2cddf040ba3865e3))
    - forbid junk after extensions ([fd37982](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fd37982a19a4a2911912ce321b9468993a0919ad))
    - only accept v3 X.509 certificates ([e9e4a2a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e9e4a2a6fd33d8fc21b00cfc9816a3dd3fef47fe))
    - properly validate X.509 extensions ([f5c5185](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f5c51855d36e399e6e22cc1eb94f6b58e51b3b6d))
    - reject invalid padding in digests ([f47547b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f47547b35462571636a76b737602e827ae43bc24))
    - reject junk after certificates ([ca34dbc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ca34dbc0cdb1c4e1ab62aa4dd195cf9389b9edb7))
    - reject padding after BIT STRING in signatures ([a8c8c5e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a8c8c5ef2a8f5a27772eb708f2201429dd8d32b2))
    - require at least one extension to be present ([72460f5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/72460f50e2437a85ce5229c430931aab8f4a0d5b))
    - require bit strings to have no unused bits ([8816dbb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8816dbb3819e626d14e1bb9702f6446cb80e26f0))
    - use NULL instead of 0 for pointer check ([654b65b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/654b65b36d60a9c08e1d0cd88b35cd7bc2c813af))

    - **mbedTLS**

      - fix mbedtls coverity issues ([a9edc32](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a9edc32c8964ffe047909b4847edd710b5879f35))

  - **Console**

    - correct scopes for console symbols ([03bd481](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/03bd48102b575a9c86eed73866a5f9cd4d03e2d5))
    - fix crash on spin_unlock with cache disabled ([5fb6946](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5fb6946ad70f5c6e82502a704633bba1dd82e507))

  - **I/O**

    - compare function pointers with NULL ([06d223c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/06d223cb4f54543299b96d40a682e33f9147e192))

  - **MMC**

    - align part config type ([53cbc94](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/53cbc949670877d1b661782ab452f6fac2302ce3))
    - do not modify r_data in mmc_send_cmd() ([bf78a65](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf78a6504254be9bf2cee38828a72f84773d4aa7))
    - explicitly check operators precedence ([14cda51](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/14cda5168de45bbbcce1a5152140111d4fc8fd21))
    - remove redundant reset_to_idle call ([bc0a738](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc0a73866f3e4f7138892b228eb592be118b40d2))

  - **GUID Partition Tables Support**

    - add missing curly braces ([1290662](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1290662034578b4e52443c79f34dfd7c284c0435))
    - add U suffix for unsigned numbers ([d1c6c49](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d1c6c495541b6e387179f987acbef274a12c7535))

  - **SCMI**

    - change function prototype to fix gcc error ([f0f2c90](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f0f2c90365d933ee0a160b4bf5723fc303d9ab73))
    - fix compilation error in scmi base ([7c38934](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7c3893423d6ba5088f92f4ebdb626285759a1bcd))

  - **UFS**

    - device present (DP) field is set to '1' ([83103d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83103d1264fe3cd7d54f3a89121d6889b4d33980))
    - flush the entire PRDT ([83ef869](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83ef8698f9d1477c892cad15b4e48574ed634903))
    - only allow using one slot ([56db7b8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/56db7b8b08d5bb350a02e1f794dc6eb02827917f))
    - poll UCRDY for all commands ([6e57b2f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e57b2f00e36e63da765e3aa1650b03772999726))
    - set the PRDT length field properly ([20fdbcf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/20fdbcf502bd457a4b74ffa9a610d573594f1f6c))

  - **Arm**

    - **Ethos-N**

      - add workaround for erratum 2838783 ([5a89947](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5a89947ab3ef8541b7adb6058af9ef141073043d))

    - **GIC**

      - wrap cache enabled assert under plat_can_cmo ([78fbb0e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/78fbb0ec8372a638b2b2a0276776892141ff43f8))

      - **GICv3**

        - fixed bug in the initialization of GICv3 SGIs/(E)PPIs interrupt priorities ([5d68e89](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d68e8913ea983b21eb4a1163e6215ff8f8e96e4))
        - restore scr_el3 after changing it ([1d0d5e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d0d5e40206c693e24b0a4de7dbcfc4b79f3138e))
        - workaround for NVIDIA erratum T241-FABRIC-4 ([a02a45d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a02a45dfef4b02fa363a5f843ba6a0aac52d181f))

    - **RSS**

      - do not consider MHU_ERR_ALREADY_INIT as error ([55a7aa9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/55a7aa9252acfc9712a914e74bcddefc3a8d6390))
      - fix msg deserialization bugs in comms ([dda0528](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dda052851a78fad150b6565ea4bb75644bd37dce))
      - remove null-terminator from RSS metadata ([85a14bc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/85a14bc0a9598668c4678f9eda2ba497acba5ced))

  - **NXP**

    - fix fspi coverity issue ([5199b3b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5199b3b93c6ada8dd830f625f77987d3474a6f98))
    - fix sd secure boot failure ([236ca56](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/236ca5667e8ac82aa53d4e933a78e6ca1ebf456e))
    - fix tzc380 memory regions config ([07d8e34](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07d8e34fdd5a81b6fe5f805560be44c1063cea79))
    - use semicolon instead of comma ([50b8ea1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/50b8ea115f117e17646d73fe7606bee14bd02630))

    - **NXP Crypto**

      - fix coverity issue ([e492299](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e49229911f4e08e317453883886a113f3332b776))
      - fix secure boot assert inclusion ([334badb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/334badb50f3ad55762785a6ba0266c2eb4d93e8e))

    - **DDR**

      - add checking return value ([e83812f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e83812f11a2d725931de88308c5b520d88bcca86))
      - apply Max CDD values for warm boot ([00bb8c3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/00bb8c37e0fe57ae2126857ce2d2700106a76884))
      - fix coverity issue ([2d541cb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d541cbcbe90217df107e1ac0c4adb76d647b283))
      - fix underrun coverity issue ([87612ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/87612eaefff34548b72fed0d8c93dcf73f9b8c81))
      - use CDDWW for write to read delay ([fa01056](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fa0105693c85eacf6eda22eca63f220d304f7768))

  - **ST**

    - **Clock**

      - avoid arithmetics on pointers ([4198fa1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4198fa1db7297d8385bb6624d4bd475870e5bf12))
      - give the size for parent_mp13 and dividers_mp13 tables ([ee21709](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ee21709e98a9e0f60a46d79caf5b702a0b7941cc))
      - remove useless switch ([69a2e32](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/69a2e320b6798ce3cf5cb27bf70e3384cfac3ebb))
      - use Boolean type for tests ([c3ae7da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c3ae7da02dd8b358239dde47c3325e333af81056))

    - **Crypto**

      - move flag control into source code ([6a187a0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a187a002ee72ef865222870b2ecf99cf4d4efb8))
      - remove platdata functions ([6b3ca0a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6b3ca0a81723290e2d9b33c406c0e65c1870baa8))
      - set get_plain_pk_from_asn1() static ([70a422b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/70a422ba83df3f572af1d2931e950feb78592ca3))

    - **GPIO**

      - define shift as uint32_t ([5d942ff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d942ff1964131bf33f445f66175fe8211c77e23))

    - **SDMMC2**

      - check transfer size before filling register ([029f81e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/029f81e04c0232843f3e546fa080778a1008a9c5))

    - **ST PMIC**

      - define pmic_regs table size ([3cebeec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3cebeec2ae452d33ec0cea322f4ab18137e41631))
      - enclose macro parameter in parentheses ([be7195d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/be7195d06cb7731fe0d906c6eabe6cb6f39f29b1))

    - **Regulator**

      - enclose macro parameters in parentheses ([91af163](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/91af163cbbfab936e70568998e8b9dcb10203b8e))
      - explicitly check operators precedence ([68083e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68083e7ad5ded7adbeca147546bbda6c14cab049))
      - rework for_each_*rdev macros ([6a3ffb5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a3ffb53910f136d14ddad5042da01a03e5087c4))
      - use Boolean type for tests ([9a00daf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9a00daf9dd0a25da45a43142ca27126e6e26a622))

    - **USB**

      - replace redundant checks with asserts ([02af589](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/02af589cfa8d8aefaffeef3390e3fb8fdf51978f))

- **Style**

  - correct some typos ([1b491ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b491eead580d7849a45a38f2c6a935a5d8d1160))

- **Miscellaneous**

  - **AArch64**

    - allow build with ARM_ARCH_MINOR=4 ([78f56ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/78f56ee71c9ffe7a6ee36268f0fe1f7ca7d01738))

  - **FDT Wrappers**

    - use correct prototypes ([e0c56fd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e0c56fd71fbd7e8ef307777db8940fb2cf3c9957))

  - **FDTs**

    - **STM32MP1**

      - **STM32MP15**

        - use /omit-if-no-ref/ for spi and i2c ([d480df2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d480df2116fc0d629d52f654bc218ee36251cb33))
        - use interrupts-extended for i2c2 ([600c8f7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/600c8f7d953d466e0ec5fd04bd6ef2e44c9c9125))

  - **PIE**

    - pass `-fpie` to the preprocessor as well ([966660e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/966660ecd0c8a3d6e4d18a5352bb431e71a9a793))

  - **UUID**

    - add missing `#include` directives ([12562af](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/12562af369e897c67aa45bfeb97cd7bb5d500cf6))

  - add missing click dependency ([ff12683](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff12683e87e44ead813600fac5415e05e7f95700))
  - add parenthesis for tests in MIN, MAX and CLAMP macros ([8406db1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8406db14fbba19c25d000eaeab538a0474795da1))
  - increase BL32 limit ([c2a7612](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c2a76122c88e9ba5de493e1aa765ad170614a31d))
  - remove old-style declarations ([f4b8470](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f4b8470feee4437fb3984baeee8c61ed91f63f51))
  - remove useless "return" at void functions ([af4d8c6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af4d8c6d505c001ee78ea9dd9d8dd76ba039af9b))
  - unify fallthrough annotations ([e138400](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e138400d1c19a561eaf9f23b0cadc07226684561))

- **Documentation**

  - add a build.tools.python entry ([4052d95](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4052d9583c850feeb8add29734bda0ef0343c238))
  - add few missed links for Security Advisories ([43f3a9c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/43f3a9c4d67da76a00f9050e7cfe1333da51ff92))
  - add plantuml as a dependency ([65982a9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/65982a94ef113d5d652d8e1a521b219be75fca42))
  - add readthedocs configuration file ([8a84776](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8a84776340bf4215d235b7b6dc09cf94aed8c6b3))
  - deprecate plat_convert_pk() in v2.9 ([e0f58c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e0f58c7fb685560933e3583cb1dfab8fb2963692))
  - make required compiler version == rather than >= ([415195c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/415195c03e6e1b3a5335ee242ab4116d2d1ac0b1))
  - python version must be string ([3aa919e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3aa919eb278e7e0b23742ea043d79e1b1f1d75c6))
  - specify python version to 3.10 ([a7773c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a7773c590d0319bdf3b4ddc67c7b22180020224b))

- **Build System**

  - add a default value for INVERTED_MEMMAP ([4d32f91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d32f9138d61719bbaab57fdd853877a7e06b1cd))
  - allow lower address access with gcc-12 ([dea23e2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dea23e245fb890c6c06eff7d1aed8fffa981fc05))
  - allow warnings when using lld ([ebac692](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ebac6922d1f6fc16c5d3953dfb512553001dcdd3))
  - partially fix qemu aarch32 build ([c68736d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c68736dab5631af3d9a1d33cb911e90e67e8ee34))

- **Tools**

  - **NXP Tools**

    - fix coverity issue ([4fa0f09](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4fa0f097399c7d396bc14a6692476ada6981c458))

  - **Secure Partition Tool**

    - add dependency to SP image ([4daeaf3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4daeaf341a347a60fd481fb4a1530f18f8e4c058))

  - **Certificate Creation Tool**

    - change WARN to VERBOSE ([76a85cf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/76a85cfa0ab5e7093ad18601b7e73a1e425d8025))

- **Dependencies**

  - add missing aeabi_memset.S ([bdedee5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bdedee5a0f156d05eb62c704e702bfd1c506dc5d))

### New Features

- **Architecture**

  - **Extended Translation Control Register (FEAT_TCR2).**

    - add FEAT_TCR2 to the changelog ([a366640](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a366640cf22d7d0e610564f81e189f6037ff9473))
    - support FEAT_TCR2 ([d333160](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d3331603664ca7d4ab1510df09e722e6ffb1df29))

  - **CPU feature / ID register handling in general**

    - enable FEAT_SME for FEAT_STATE_CHECKED ([45007ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/45007acd46981b9f289f03b283eb53e7ba37bb67))
    - enable FEAT_SVE for FEAT_STATE_CHECKED ([2b0bc4e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2b0bc4e028a75d75c6d6942ddd404ef331db29be))
    - extend check_feature() to deal with min/max ([a4cccb4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4cccb4f6cbbb35d12bd5f8779f3c6d8d762619c))

  - **Guarded Control Stack (FEAT_GCS)**

    - support guarded control stack ([688ab57](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/688ab57b9349adb19277d88f2469ceeadb8ba083))

  - **Support for the `HCRX_EL2` register (FEAT_HCX)**

    - initialize HCRX_EL2 to its default value ([ddb615b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ddb615b419074727ac0a1430cf0f88bd018ac8df))

  - **Scalable Matrix Extension (FEAT_SME, FEAT_SME2)**

    - enable SME2 functionality for NS world ([03d3c0d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/03d3c0d729e24713d657209bedf74d255550babb))

- **Platforms**

  - **Allwinner**

    - add extra CPU control registers ([b15e2cd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b15e2cda14b3ffddebd8b40cc5c31c1c0e9cbf0d))
    - add function to detect H616 die variant ([fbde260](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fbde260b11171f0f67afbc631e22fe26366ff448))
    - add support for Allwinner T507 SoC ([018c1d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/018c1d878fbfd696ebeda52b5188e4658b87bf75))

  - **Arm**

    - add ARM_ROTPK_LOCATION variant full key ([5f89928](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f899286eac994b8337959ad924a43c1a4a543c9))
    - carveout DRAM1 area for Event Log ([6b2e961](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6b2e961fb1428c3fe213c524164a00fcaee495c4))

    - **FVP**

      - add Event Log maximum size property in DT ([1cf3e2f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1cf3e2f0a8eb0d6324ce3db68dd5c78bdb690a8a))
      - copy the Event Log to TZC secured DRAM area ([191aa5d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/191aa5d3fc793c5c4cd8960d1ef7b95010cc9d87))
      - define ns memory in the SPMC manifest ([7f28179](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f28179a46b40ede461326dd329eb832c0d72b0d))
      - emulate trapped RNDR ([1ae7552](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1ae75529bc2e5a213c3e458898c219c34aa99f65))
      - enable errata management interface ([d3bed15](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d3bed15851a1b35b2608f7275f1294c8d4f7aee7))
      - enable FEAT_FGT by default ([15107da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/15107daad6b83b4ee1edfebf420b6779a054318e))
      - enable FEAT_HCX by default ([2e12418](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e1241888ee82a5a9b3b30acd83a1f4ea6732f1b))
      - enable support for PSCI OS-initiated mode ([e75cc24](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e75cc247c744d21e52f834a442bf1c26d0ab6161))
      - increase BL1_RW and BL2 size ([dbb9c1f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dbb9c1f5b69134ca43c944d84b413331a64fba15))
      - introduce PLATFORM_TEST_EA_FFH config ([fe38cc6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fe38cc68975b23084b4ba512254926941c865a07))
      - introduce PLATFORM_TEST_RAS_FFH config ([5602ce1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5602ce1d8db3256a7766776cb908b1f716c2d463))
      - update device tree with load addresses of TOS_FW config ([1779762](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/177976286e347acd905d8082f31c201b9900d28e))

    - **Juno**

      - support ARM_IO_IN_DTB option for Juno ([2fad320](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2fad320f5623edcdd23297ab57c4b9b0b0ef872c))

    - **Morello**

      - add GPU DT node ([cd94c3d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd94c3d6ad5e738c2583486b7a973bd8e516089b))
      - add support for HW_CONFIG ([be79071](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/be79071ef73b4b08cca310ec7e7d915faea8f036))
      - implement methods to retrieve soc-id information ([cc266bc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc266bcd8c0a1d839151b69436fdf2c1ad07b0a1))

    - **RD**

      - **RD-N2**

        - add platform id value for rdn2 variant 3 ([028c619](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/028c6190d9f3d892a84b5b9cbfdbbab808a73acb))

    - **TC**

      - enable MPAM functionality of L3 DSU cache ([b45ec8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b45ec8cea483a38e358146b99205504ff7f98001))
      - add delegated attest and measurement tests ([25dd217](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/25dd2172ae564c74b7e8b42aa96d5ee9a865ec75))
      - allow secure watchdog timer to trigger periodically ([28b2d86](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28b2d86cd28ffc54c6272defcd6f123a925012f1))
      - use smmu 700 ([ed80eab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ed80eab6a686ce1042300cfbdb90e13366aa08d4))

  - **Intel**

    - extending to support SMMU in FCS ([4687021](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4687021d2eedea880ad8596b32e85da72f8cba02))
    - fix bridge disable and reset ([9ce8251](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9ce82519c65f0dd93d2673ebb967d02f52b19a04))
    - implement timer init divider via CPU frequency for N5X ([02a9d70](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/02a9d70c4deaa2102386611ac6b305838003148d))
    - setup FPGA interface for Agilex ([3905f57](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3905f57134853f47f6e859b8b6322a7dbbfc49f7))

  - **MediaTek**

    - add APU init flow ([5243091](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5243091633b8fe8057cec176ac31adb72fdf3506))
    - add new features of LPM ([917abdd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/917abdd99012d01ef4fa804ecec1503bef68ed9b))
    - add SiP service for OP-TEE ([621eaab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/621eaab5cc3c9d98783700b7515b1da118b3d21c))
    - add SMC handler for EMI MPU ([c842cc0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c842cc0e5d1432a681cbddce62a852ff282169ae))
    - add SPM's SSPM notifier ([c234ad1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c234ad17d7d7278e1afa0f416982bb0f60a04dcf))

    - **MT8188**

      - add apu power on/off control ([8e38b92](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8e38b928490516d308bdceebc4ad032852bf2716))
      - add MT8188 SPM debug logs ([f85b34b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f85b34b112eec006c14afab0eadbd45d1b0d0e7e))
      - add MT8188 SPM support ([45d5075](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/45d507599e213f8f3a26502c3ca8de6b1cfdc611))
      - add SPM feature support ([f299efb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f299efbea685aa8075ec4d6d0f70d189cce3ee07))
      - add the register definitions accessed by SPM ([1a64689](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1a64689df5e7bf78aa8724c1d75f414ea62750eb))
      - enable SPM and LPM ([380f64b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/380f64b2e39c60cb9a1f751b25cbce11c5e03e20))
      - keep infra and peri on when system suspend ([e56a939](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e56a939cabb5ae0fe967c19ddacf97304c563f37))
      - update INFRA IOMMU enable flow ([98415e1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/98415e1a80ca025a000241cf3fc175272890c0e8))

    - **MT8195**

      - add support for SMC from OP-TEE ([ccc61e1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ccc61e10029b8ddfcb5cb65201862a18ebbc953d))

  - **NVIDIA**

    - **Tegra**

      - implement 'pwr_domain_off_early' handler ([96d07af](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/96d07af402a5d191b7d1200a75c1b206f21cc395))

  - **NXP**

    - **i.MX**

      - **i.MX 8M**

        - add more dram pll setting ([4234b90](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4234b902ae37ca05640888e31405ec97c8cde316))
        - fix the ddr4 dvfs random hang on imx8m ([093888c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/093888caaf54cbfe38d4b68406d98fbcf5c7d81f))
        - update the ddr4 dvfs flow to include ddr3l support ([0e39488](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0e39488ff3f2edac04d7f5acb58d9a22baa3a69e))
        - use non-fast wakeup stop mode for system suspend ([ef4e5f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef4e5f0f105f184f02ad4d1cc17cecec9b45502a))

        - **i.MX 8Q**

          - add anamix pll override setting for DSM mode ([387a1df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/387a1df18e0b5bf1d305c72df284b1b89f3c1cd3))
          - add BL31 PIE support ([8cfa94b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8cfa94b7a7fc398cc0ea803891f6277065bb7575))
          - add the dram retention support for imx8mq ([dd108c3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dd108c3c1fe3f958a38ae255e57b41e5453d077f))
          - add version for B2 ([99475c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99475c5dcc14123dda51bda32d21753f0b4c357d))
          - add workaround code for ERR11171 on imx8mq ([88a2646](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/88a264657fad2f71369fec4b53478e8a595d10e9))
          - always set up console ([36be108](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/36be10861e851e7e4df06bb08aab60d8e878d2b2))
          - correct the slot ack setting for STOP mode ([724ac3e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/724ac3e2c23441d11f642f2ae91c8a8834ea179f))
          - enable dram dvfs support on imx8mq ([8962bdd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8962bdd603508f649fd7a332e580c0e456ccc0ad))
          - make IMX_BOOT_UART_BASE configurable via build parameter ([202737e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/202737efda85b2ea61934123b8ffa492f5dc3679))
          - remove empty bl31_plat_runtime_setup ([7698dba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7698dbab96072881e0912322db5036529bf8553c))

      - **i.MX 8**

        - add support for debug uart on lpuart1 ([8406447](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8406447f13c65fe93aab7ed641b7e8fe3eb47a0b))

    - **Layerscape**

      - **LX2**

        - enable OCRAM ECC ([e8faff3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8faff3da962ce112e32d8f1fdb8155e078eae75))
        - support more variants ([c07f5e9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c07f5e9e50959a3667e5a96ac808d1d16bb72698))

  - **QEMU**

    - add "neoverse-n1" cpu support ([226f4c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/226f4c8e35c4441e80ad523b9105eab4ca630396))
    - add A76/N1 cpu support for virt ([6b66693](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6b66693685f828a51c7f78bfa402d6b192169a6d))
    - combine TF-A artefacts into ROM file ([63bb905](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/63bb90569792893a4e7401004c23cde488fda0cc))
    - increase max cpus per cluster to 16 ([73a7aca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/73a7aca2a53d4dbb62909c5741830eee9eac5ee8))
    - increase size of bl2 ([db2bf3a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/db2bf3ac193f66f365b962b911e7bb2ffbde0a25))
    - make coherent memory section optional ([af994ae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af994ae8a089ead6082ca82036d30074f554ed52))
    - support el3 spmc ([302f053](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/302f05354f5aab340c315e0d04915367c65c6b27))
    - support pointer authentication ([cffc956](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cffc956edf3a14508ed5740c1ed093326ca67e72))
    - support s-el2 spmc ([36802e2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/36802e2c792f79ab630b53298dfd4f1e5a95d173))
    - update abi between spmd and spmc ([25ae7ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/25ae7ad1878244f78206cc7c91f7bdbd267331a1))

  - **QTI**

    - **SC7280**

      - add support for PSCI_OS_INIT_MODE ([e528bbe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e528bbec74af359714203c7f8d356074733ea9cd))

    - **MSM8916**

      - expose more timer frames ([1781bf1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1781bf1c40594e3a3f36404da793d5c7a6bca533))

  - **ST**

    - mandate dtc version 1.4.7 ([38ac8bb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/38ac8bbbe450343e8545a44f370ff9da57cbed26))

    - **STM32MP1**

      - add mbedtls-3.3 support config ([c9498c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c9498c8f56387ad23530dcc6e57940d2b118d907))

  - **Texas Instruments**

    - add PSCI system_off support ([0bdef26](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0bdef264c2bd356e2a89fc5ac7c438694618d272))
    - add sub and patch version number support ([852378f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/852378fd60d8cc536799639774f1e4ffe124131d))
    - disable L2 dataless UniqueClean evictions ([10d5cf1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10d5cf1b26f03d61a90cdcff5163965fa48e291c))
    - do not handle EAs in EL3 ([2fcd408](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2fcd408bb3a6756767a43c073c597cef06e7f2d5))
    - set L2 cache data ram latency on A72 cores to 4 cycles ([aee2f33](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aee2f33a675891f660fc0d06e739ce85f3472075))
    - set L2 cache ECC and and parity on A72 cores ([81858a3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/81858a353f8e45f5cc57ce855188043b1745ea08))
    - set snoop-delayed exclusive handling on A72 cores ([5668db7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5668db72b724dc256d9b300f6938a08625624a48))
    - synchronize access to secure proxy threads ([312eec3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/312eec3ecde9837f61fc0d7b46b4197ec2257ee7))

  - **Xilinx**

    - add device node indexes ([407eb6f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/407eb6fda06d7be034dc7f1c537183f64126f074))
    - sync copyright format ([2774965](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/27749653c7dbea1bd5b34a39085bc7cb12d46501))

    - **Versal**

      - replace irq array with switch case ([0ec6c31](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ec6c31320c6d86e89dce8775af2bbdfa7a302fa))
      - switch to xlat_v2 ([0e9f54e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0e9f54e5bb7f4b44bca9c63cce37913070fea23a))

      - **Versal NET**

        - add jtag dcc support ([30e8bc3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/30e8bc365c1007da97f93c71e5fa16b6be56b679))
        - add support for set wakeup source ([c38d90f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c38d90f7964ddf186f4cbaad6da91dd0a44627e3))
        - add support for uart1 console ([2f1b4c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2f1b4c55502262dba0ccd147f87cdb38cf4131f2))

    - **ZynqMP**

      - add hooks for custom runtime setup ([88a8938](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/88a8938e62989b7319b20c46c046aa8845852ce9))
      - add hooks for mmap and early setup ([7013400](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/70134000842cbc7c052031dd453bdec8f4cb73f1))
      - add SMCCC_ARCH_SOC_ID support ([8f9ba3f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8f9ba3f344545740fc44e90fb8322c7728ae94ec))
      - add support for custom sip service ([496d708](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/496d708154d893fb9f412390acd433337faccecc))
      - build pm code as library ([3af2ee9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3af2ee906842378ee91f07aa4ea5565cd1a0f8c2))
      - bump up version of query_data API ([aaf5ce7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aaf5ce77fb22f54a8ca7bc8d3be6172dacbfc0c1))
      - make stack size configurable ([5753665](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/57536653e62765f9529d045b118ad881369bc73a))

- **Services**

  - **RME**

    - read DRAM information from FVP DTB ([8268590](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/826859049859a5bd88e142695e10a559d85721c1))
    - set DRAM information in Boot Manifest platform data ([a97bfa5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a97bfa5ff18b2682e3b9c528cbd5fb16ceec3393))

    - **RMM**

      - add support for the 2nd DRAM bank ([346cfe2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/346cfe2b46a83bc9e6656f43ec55a196503b154a))

  - **SPM**

    - **EL3 SPMC**

      - make platform logical partition optional ([555677f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/555677fe81c5e1888254ac36acb0a02b3850dc46))

    - **SPMD**

      - add support for FFA_EL3_INTR_HANDLE_32 ABI ([6671b3d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6671b3d8224a8c4c3fea7cbe66b56945c432393f))
      - copy tos_fw_config in secure region ([0cea2ae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0cea2ae07db089e60322677021da4743a084f9ca))
      - fail safe if SPM fails to initialize ([0d33649](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0d33649e3e2a21def73327522b9861b4619fc5c2))
      - introduce FFA_PARTITION_INFO_GET_REGS ([eaaf517](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eaaf517cd1bd8c9d5e3e6d2d202a69a0cbcb45bf))
      - introduce platform handler for Group0 interrupt ([f0b64e5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f0b64e507e9105813d9a5d16f70101cf0d8ca5a4))
      - map SPMC manifest region as EL3_PAS ([8c829a9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8c829a9240109dd7a66a3c26f734f23477b12551))
      - register handler for group0 interrupt from NWd ([a1e0e87](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a1e0e871f10201a9dbdc1dadfd27904888246adc))

  - **ERRATA_ABI**

    - errata management firmware interface ([ffea384](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ffea3844c00daf8dee466840a4932cac04b3eb57))

- **Libraries**

  - **CPU Support**

    - add support for blackhawk cpu ([6578343](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6578343bb2aab6ec5ae309097047a83445aa12da))
    - add support for chaberton cpu ([516a52f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/516a52f6f5cda6acb311ffd6e8fb77f2e09c1357))

  - **EL3 Runtime**

    - handle traps for IMPDEF registers accesses ([0ed3be6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ed3be6fc2c8d275862959d1ee6a0354cc01ad5d))
    - introduce system register trap handler ([ccd81f1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ccd81f1e097c3eafe38523110c8eebabbe662508))

  - **FCONF**

    - rename 'ns-load-address' to 'secondary-load-address' ([05e5503](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/05e550302103a527b9f8d3869942c203c7b2dd65))

  - **OP-TEE**

    - add device tree for coreboot table ([f4bbf43](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f4bbf435554e87de31c0a70039aa03b19962aaea))
    - add loading OP-TEE image via an SMC ([05c69cf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/05c69cf75edf53478e23fce157fea72372b49597))

  - **PSCI**

    - add support for OS-initiated mode ([606b743](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/606b7430077c15695a5b3bcfbad4975f00c9bf95))
    - add support for PSCI_SET_SUSPEND_MODE ([b88a441](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b88a4416b5e5f2bda2240c632ba79e15a9a75c45))
    - introduce 'pwr_domain_off_early' hook ([6cf4ae9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6cf4ae979a5f8be23927b97ecfe789dabcb53dbd))
    - update PSCI_FEATURES ([9a70e69](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9a70e69e059863d7aec11883e6345b54058264e0))

  - **C Standard Library**

    - add %c to printf/snprintf ([44d9706](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44d9706e5428d8e3588d04565c7cd738ffc1e472))
    - add support for fallthrough statement ([023f1be](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/023f1bed1dde23564e3b66a99c4a45b09e38992b))

  - **PSA**

    - add read_measurement API ([6d0525a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d0525aafe17e7affb0f71e86a5121989c150c42))
    - interface with RSS for NV counters ([8374508](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8374508b00909cdffbe6233cf8fddcb49924faed))

- **Drivers**

  - **Authentication**

    - compare platform and certificate ROTPK for authentication ([f1e693a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1e693a77548950cfffcb1d5a4b67cf349e0aed9))

    - **mbedTLS**

      - add support for mbedtls-3.3 ([51e0615](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/51e061591bbf13af2486c3bb5f37ed609578d145))

  - **UFS**

    - adds timeout and error handling ([2c5bce3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c5bce3833848dac4fbb2ae19be418145e68c8a1))

  - **Arm**

    - **Ethos-N**

      - add check for NPU in SiP setup ([a2cdbb1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a2cdbb1df088cde410aea1d5989dfc500aaf7939))
      - add event and aux control support ([7820777](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7820777fa3c8ca454ab40d5d8a8ba0e311bbb6f9))
      - add multiple asset allocators ([8a921e3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8a921e354575cd16aaa6f2f5a2aeaaaea35ab886))
      - add NPU firmware validation ([313b776](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/313b776f851ed184abb265df2b6269fe78f48ecd))
      - add NPU sleeping SMC call ([2a2e3e8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2a2e3e87706b56fd1b8e787d3a552cfc12725934))
      - add NPU support in fiptool ([c91b08c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c91b08c8a44aafac4f72c64aa8d4777b8c73647e))
      - add protected NPU firmware setup ([6dcf3e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6dcf3e774457cf00b91abda715adfbefce822877))
      - add protected NPU TZMP1 regions ([d77c11e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d77c11e896e04be93caa4a56e50646af6806843f))
      - add reserved memory address support ([a19a024](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a19a0241a6f1573e11d4d747dabb756d15ac4801))
      - add reset type to reset SMC calls ([fa37d30](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fa37d30856fef6742bd82e4e0a3252a4d0b9e091))
      - add separate RO and RW NSAIDs ([986c4e9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/986c4e991ace5cb40bed35145184e66863c47152))
      - add SMC call to get FW properties ([e9812dd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e9812ddca6e72c0501ef1e84753f335dcafb74cd))
      - add stream extends and attr support ([e64abe7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e64abe7bdaeed99093ae5b4aab8956a04ff4075a))
      - add support for NPU to cert_create ([f309607](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f309607229e049a6ff9cbc858efa4dd0c0b921b8))
      - add support to set up NSAID ([70a296e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/70a296ee8641802dc60754aec5b18d8347820a5c))
      - load NPU firmware at BL2 ([33bcaed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33bcaed1211ab27968433b546979687bc1182630))

    - **GIC**

      - **GICv3**

        - enlarge the range for intr_num of structure interrupt_prop_t ([d5eee8f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d5eee8f3fbf53fce84c979e68433a27c93e3e96b))

    - **RSS**

      - add TC platform UUIDs for RSS images ([6ef63af](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6ef63af65f55e9402e4cdc534928faceb9c6e003))

    - **SBSA**

      - helper api for refreshing watchdog timer ([e8166d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8166d3e5937b8db43921b5049672b16af7f58e0))

- **Miscellaneous**

  - **AArch64**

    - make ID system register reads non-volatile ([c2fb8ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c2fb8ef66ccc8222c70ab802cdaf29f1592cbbb6))

  - **FDTs**

    - **STM32MP1**

      - use /omit-if-no-ref/ for pins nodes ([0aae96c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0aae96cfb9ef826d207f2d18d4a9f21fa1a5dee7))

      - **STM32MP15**

        - add support for prtt1x board family ([3812ceb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3812ceba8fcd682faeed6e71190a848771fd2022))

  - **PIE/POR**

    - support permission indirection and overlay ([062b6c6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/062b6c6bf23f9656332b0aa3fed59c15f34f9361))

- **Documentation**

  - allow verbose build ([f771a34](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f771a3446356d92c6c27df5c4f3bb07a2561b36b))

- **Build System**

  - add support for new binutils versions ([1f49db5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1f49db5f25cdd4e43825c9bcc0575070b80f628c))
  - allow additional CFLAGS for library build ([5a65fcd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5a65fcd5f9c67baa681f664e4596760ca1f2606a))

  - **Git Hooks**

    - add pre-commit hook ([cf9346c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cf9346cb83804feb083b56a668eb0a462983e038))

  - add support for poetry ([793f72c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/793f72c06ca1c2782f800c9f20980ca6b7870072))

- **Tools**

  - **Firmware Image Package Tool**

    - handle FIP in a disk partition ([06e69f7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/06e69f7c94637c693ea5eb26038096c196d10f07))

- **Dependencies**

  - **Compiler runtime libraries**

    - update source files ([658ce7a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/658ce7ad8eceb40741cd40f1639a6d923f922fad))

## [2.8.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.7.0..refs/tags/v2.8.0) (2022-11-15)

### ⚠ BREAKING CHANGES

- **Drivers**

  - **Arm**

    - **Ethos-N**

      - add support for SMMU streams

        **See:** add support for SMMU streams ([b139f1c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b139f1cf975f9968eb8bd1182a173b976ecf06f9))

### New Features

- **Architecture**

  - pass SMCCCv1.3 SVE hint bit to dispatchers ([0fe7b9f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0fe7b9f2bcdf754c483399c841e5f0ec71e53ef3))

  - **Branch Record Buffer Extension (FEAT_BRBE)**

    - add brbe under feature detection mechanism ([1298f2f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1298f2f13d6d97dfcac120a2ee68d5eea3797068))

  - **Confidential Compute Architecture (CCA)**

    - introduce new "cca" chain of trust ([56b741d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/56b741d3e41cd6b2f6863a372a9489c819e2b0e9))

  - **Pointer Authentication Extension**

    - add/modify helpers to support QARMA3 ([9ff5f75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9ff5f754aea00d0e86ba5191839fc0faef949fe0))

  - **Trapping support for RNDR/RNDRRS (FEAT_RNG_TRAP)**

    - add EL3 support for FEAT_RNG_TRAP ([ff86e0b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff86e0b4e6c34d28b8642dd8eb9cbdd517bad195))

  - **Scalable Matrix Extension (FEAT_SME)**

    - fall back to SVE if SME is not there ([26a3351](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/26a3351edab1501d7e19ae96540c34b2700ac32f))

  - **Scalable Vector Extension (FEAT_SVE)**

    - support full SVE vector length ([bebcf27](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bebcf27f1c75f48cc129e8608cba113d0db32ef8))

  - **Trace Buffer Extension (FEAT_TRBE)**

    - add trbe under feature detection mechanism ([47c681b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47c681b7d7f03e77f6cdd7b5d116ae64671ab8ca))

- **Platforms**

  - **Arm**

    - add support for cca CoT ([f242379](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f24237921e3fa61e64fa1ec845e14e2748d04a2b))
    - forbid running RME-enlightened BL31 from DRAM ([1164a59](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1164a59cb16a9bbc672fa6d07895bc6fa0361bcb))
    - provide some swd rotpk files ([98662a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/98662a73c903b06f53c9f9da6a9404187fc10352))
    - retrieve the right ROTPK for cca ([50b4497](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/50b449776df11cac06347e8ef1af5dae701a0e3a))

    - **CSS**

      - add interrupt handler for reboot request ([f1fe144](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1fe1440db197d514b5484e780cfb90f504c62b9))
      - add per-cpu power down support for warm reset ([158ed58](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/158ed580bdf5736abfa9f16f61be1ca1609e0e41))

    - **FVP**

      - add example manifest for TSP ([3cf080e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3cf080ed61e90668f0c44ca7f577e51c081e5c7c))
      - add crypto support in BL31 ([c9bd1ba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c9bd1bacffd9697ec4ebac77e45588cf6c261a3b))
      - add plat API to set and get the DRTM error ([586f60c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/586f60cc571f0f3b6d20eb5033717e9b0cc66af4))
      - add plat API to validate that passed region is non-secure ([d5f225d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d5f225d95d3dc7473340ffebfcb9068b54f91a17))
      - add platform hooks for DRTM DMA protection ([d72c486](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d72c486b52dc654e4216d41dcc1b0f87bdbdf3e9))
      - build delegated attestation in BL31 ([0271edd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0271eddb0c00b01033bf651f0eeaf659c0c2dd39))
      - dts: drop 32-bit .dts files ([b920330](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b92033075aa27031091e184b54f4dc278ecb27bc))
      - fdts: update rtsm_ve DT files from the Linux kernel ([2716bd3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2716bd33e318821c373b3d4dce88110a340a740d))
      - increase BL31's stack size for DRTM support ([44df105](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44df105ff867aeb2aa5d20faa3e8389866099956))
      - increase MAX_XLAT_TABLES entries for DRTM support ([8a8dace](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8a8dace5a5cd3a51d67df3cea86628f29cc96013))
      - support building RSS comms driver ([29e6fc5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/29e6fc5cc7d0c8bc4ba615fd97df4cb65d3c7ba3))

    - **RD**

      - **RD-N2**

        - add a new 'isolated-cpu-list' property ([afa4157](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/afa41571b856509c25c66c331737b895144b681b))
        - add SPI ID ranges for RD-N2 multichip platform ([9f0835e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9f0835e9156f13b56336a47a4b51e90719a852ff))
        - enable extended SPI support ([108488f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/108488f9ac026f036c0de2b824b339a30f9a0cbb))

    - **SGI**

      - increase memory reserved for bl31 image ([a62cc91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a62cc91aeedbdcfb3396983ed165eb35b8d4c3fa))
      - read isolated cpu mpid list from sds ([4243ef4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4243ef41d480fd8e870f74defe263156a6c02c8d))
      - add page table translation entry for secure uart ([2a7e080](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2a7e080cc50be5739afcfb3b7db59e4d610a7d53))
      - bump bl1 rw size ([94df8da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/94df8da3ab520330b2e7d276603f33e284c27b3f))
      - configure SRAM and BL31 size for sgi platform ([8fd820f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8fd820ffb918ad8fdc1f2c72cc64dad5eaff77aa))
      - deviate from arm css common uart related definitions ([173674a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/173674ae428aa23e8f2a38d5542d0ea52eed7e80))
      - enable css implementation of warm reset ([18884c0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18884c002e6c298f27d6e4792eab2c9f4d89bddb))
      - remove override for `ARM_BL31_IN_DRAM` build-option ([a371327](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a371327ba9fc2e1c5988ac1436b29c42aab8dfd8))
      - route TF-A logs via secure uart ([0601083](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0601083f0ce0045bd957c1343d2196be0887973b))

    - **TC**

      - add MHU addresses for AP-RSS comms on TC2 ([6299c3a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6299c3a0f7c8220b0bf15723ec8995b72bf97677))
      - add RSS-AP message size macro ([445130b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/445130b127f411bdf4958fa10f292a930c9ae57d))
      - add RTC PL031 device tree node ([a816de5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a816de564f927ebb72ab7692b8b3f46073179310))
      - enable RSS backend based measured boot ([6cb5d32](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6cb5d3268fa41d15480c4e070a51577b333767fe))
      - increase maximum BL1/BL2/BL31 sizes ([e6c1316](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e6c131655fa168ffd1ae738a74ba25e5f850036c))
      - introduce TC2 platform ([eebd2c3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eebd2c3f61c90942fb186fa43fbb4c4a543d8b55))
      - move start address for BL1 to 0x1000 ([9335c28](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9335c28a019ee2d9ab7a0f9276b91415f3c9f1bc))

  - **HiSilicon**

    - **HiKey960**

      - add a FF-A logical partition ([25a357f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/25a357f1932cf2b0d125dd98b82eeacad14005ea))
      - add memory sharing hooks for SPMC_AT_EL3 ([5f905a2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f905a249839e9e20ebf44c22d95caaf3a2e5611))
      - add plat-defines for SPMC_AT_EL3 ([feebd4c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/feebd4c7a86b6f0fcc1eb5008ba5f7d44e75beaf))
      - add SP manifest for SPMC_AT_EL3 ([6971642](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6971642d23d0c5e33e507eb78b7c569045e2f85d))
      - define a datastore for SPMC_AT_EL3 ([e618c62](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e618c621b3ece7a0262ff9245027132982e6207c))
      - increase secure workspace to 64MB ([e0eea33](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e0eea337b32e37bbef9bad1310b96b9c0d86f7b9))
      - read serial number from UFS ([c371b83](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c371b83f0c5b503c21bd1b6092bc0230032329ce))
      - upgrade to xlat_tables_v2 ([6cfc807](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6cfc8078d032d278e09523e236ab5b36f69f2ec0))

  - **MediaTek**

    - add more flexibility of mtk_pm.c ([6ca2046](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6ca2046ef15dcf19fbda5f12cbfe1004d340c969))
    - add more options for build helper ([5b95e43](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b95e439c745dcf94899238b82826d8f1d32acbe))
    - add smcc call for MSDC ([4dbe24c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4dbe24cf7d2b04c552f394062f42c30fee7e26a6))
    - extend SiP vendor subscription events ([99d30b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99d30b72c02502731ecf116acfda44ee3c2c9e5e))
    - implement generic platform port ([394b920](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/394b92084d53e2bf8960731be7a79c999871f127))
    - introduce mtk init framework ([52035de](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52035dee1ae7b0f2f0d5f16c734ca7a5cea127b7))
    - move dp drivers to common folder ([d150b62](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d150b6296e6960f2548b265b8b23e6cdb502d3b7))
    - move lpm drivers back to common ([cd7890d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd7890d79e9d508e82f3078f02e8277f8c8df181))
    - move mtk_cirq.c drivers to cirq folder ([cc76896](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc76896d9e416b15548b2d6bf068e5d3f9b4064a))
    - support coreboot BL31 loading ([ef988ae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef988aed9e09a4108b87decb14dee5f2d23230a4))

    - **MT8186**

      - add EMI MPU support for SCP and DSP ([3d4b6f9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3d4b6f932444c7b0f70f8654b92193b294527056))

    - **MT8188**

      - add armv8.2 support ([45711e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/45711e4e1614fbed75ea645777cc2bb11d4be96f))
      - add audio support ([c70f567](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c70f567ad75c30a990cb60c71b6c0b02538366fd))
      - add cpu_pm driver ([4fe7e6a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4fe7e6a8d9f09c40d087167432cb07621c175b3f))
      - add DCM driver ([bc9410e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc9410e2376e0b6355ea6440aa90ad968fc5f3b3))
      - add DFD control in SiP service ([7079a94](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7079a942bd9705fd9e0cd220324f7dfd9c53dcad))
      - add display port control in SiP service ([a4e5023](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4e502319d136d8854ef2ed4aaa6d5368541e551))
      - add EMI MPU basic drivers ([8454f0d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8454f0d65eeb85b72f454376faa0f7a15226e240))
      - add IOMMU enable control in SiP service ([be45724](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/be457248c6b0a7f3c61bd95af58372938d13decd))
      - add LPM driver support ([f604e4e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f604e4ef6e306c6d87e17e77e50a68aad0510110))
      - add MCUSYS support ([4cc1ff7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4cc1ff7ef2c3544ef1aabeb2973a2d8f7800776b))
      - add pinctrl support ([ec4cfb9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ec4cfb91fc197a024d1edb9fae5e9ce100e5b200))
      - add pmic and pwrap support ([e9310c3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e9310c34b018944a6c29a8f408f0a34b43a0df6d))
      - add reset and poweroff functions ([a72b9e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a72b9e7754a27e6ebccf79f0cc4fb7cc5a0a8a5e))
      - add RTC support ([af5d8e0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af5d8e07955ddef9000c64de94deb2703e6ffcf0))
      - add support for PTP3 ([44a1051](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44a10511c9e5a66b3a33abba44856a7a5dc5e655))
      - apply ERRATA for CA-78 ([abb995a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/abb995abbe45874a397351cbb134ae32d4cc545b))
      - enable MTK_PUBEVENT_ENABLE ([0b1186a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0b1186a3e6fd6daffaef3f6cf59650bb9121191c))
      - initialize GIC ([cfb0516](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cfb0516f3cc36e3d0ec9b0bdabf1eb6ea2b275c1))
      - initialize platform for MediaTek MT8188 ([de310e1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/de310e1e5f0b76b9de2b93759344540e0109c8eb))
      - initialize systimer ([215869c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/215869c693c136192505a004ec368f503f146505))

  - **NXP**

    - **i.MX**

      - **i.MX 8M**

        - add dram retention flow for imx8m family ([c71793c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c71793c6476fa2828f866b8d7b272289f0d9a15c))
        - add support for high assurance boot ([720e7b6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/720e7b66f2353ef7ed32a8f85f8396fbc0766ffc))
        - add the anamix pll override setting ([66d399e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/66d399e454b160ce358346cfa9142a24d8493a41))
        - add the ddr frequency change support for imx8m family ([9c336f6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9c336f6118a94970f4045641a971fd1e24dba462))
        - add the PU power domain support on imx8mm/mn ([44dea54](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44dea5444b087acd758b1c8370999be635e17e43))
        - keep pu domains in default state during boot stage ([9d3249d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9d3249de8078e33b90193d8f91f4914acc36c6ec))
        - make psci common code pie compatible ([5d2d332](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d2d3328db88846accd179c96d71bab79a150937))

        - **i.MX 8M Nano**

          - add BL31 PIE support ([62d37a4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/62d37a4362456694bdae6d8921c2c7572a0d99a4))
          - add hab and map required memory blocks ([b5f06d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b5f06d3dfad8c27bdf528b083ef919ce4022c52d))
          - enable dram retention suuport on imx8mn ([2003fa9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2003fa94dc9b9eda575ebfd686308c6f87c366f0))

        - **i.MX 8M Mini**

          - add BL31 PIE support ([a8e6a2c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a8e6a2c83ce511dad88eb68f98a3191fa93564d4))
          - add hab and map required memory blocks ([5941f37](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5941f37288a5ceac495cbdbd3e3d02f1a3c55e0a))
          - enable dram retention suuport on imx8mm ([b7abf48](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b7abf485ee15c3e5b16522bb91dd6b0c24bfbfc0))

        - **i.MX 8M Plus**

          - add BL31 PIE support ([7a443fe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7a443fefa4eaef65332a38c8189573b5b4b4a1e3))
          - add hab and map required memory blocks ([62a93aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/62a93aa7afcd022f06d322c36979f0aa02713beb))

        - **i.MX 8Q**

          - add 100us delay after USB OTG SRC bit 0 clear ([66345b8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/66345b8b13dc32bcd9f6af3c04f60532e7d82858))

    - **Layerscape**

      - **LS1043A**

        - **LS1043ARDB**

          - update ddr configure for ls1043ardb-pd ([18af644](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18af644279b36e841068db0e1c857dedf1456b38))

  - **QEMU**

    - increase size of bl31 ([0e6977e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0e6977eee178a6436e4a7e1503ea854989316ff4))

  - **QTI**

    - fix to support cpu errata ([6cc743c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6cc743cf0fa9b216f2af8ff87c716dcc0bb6f6a0))
    - updated soc version for sc7180 and sc7280 ([39fdd3d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/39fdd3d85d1165cd1b876288532000c5c6eb1ecb))

  - **Socionext**

    - **Synquacer**

      - add BL2 support ([48ab390](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/48ab390444e1dabb669430ace9b8e5a80348eed0))
      - add FWU Multi Bank Update support ([a193825](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a19382521c583b3dde89df14678b011960097f6c))
      - add TBBR support ([19aaeea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/19aaeea00bc4fba94af7aca508af878136930f4a))

  - **ST**

    - add trace for early console ([00606df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/00606df01201fcad509ea9ddff89d5f176bee793))
    - enable MMC_FLAG_SD_CMD6 for SD-cards ([53d5b8f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/53d5b8ff50d322f764b1f5a8c882b9ee1ba952c9))
    - properly manage early console ([5223d88](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5223d88032dcecb880d620e63bfa70799dc6cc1a))
    - search pinctrl node by compatible ([b14d3e2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b14d3e22b4964ce589d107e7fd68601bf070f44c))

    - **STM32MP1**

      - add a check on TRUSTED_BOARD_BOOT with secure chip ([54007c3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/54007c37d560dd170efa52a79feb206aefb90ed4))
      - add a stm32mp crypto library ([ad3e46a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad3e46a35cb208e16adfe3d753214739583dca10))
      - add define for external scratch buffer for nand devices ([9ee2510](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9ee2510b62ef9428d767523ddb9c5a39b7a2b954))
      - add early console in SP_min ([14a0704](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/14a070408d9231dc1c487dfe36058b93faf5915c))
      - add plat_report_*_abort functions ([0423868](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0423868373026a667f0c004e4d365fa12fd734ef))
      - add RNG initialization in BL2 for STM32MP13 ([2742374](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2742374414c5891ac37fd4d42ba62c3cff1474c6))
      - add the decryption support ([cd79116](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd791164a9ad2f42d25d24012715bbe763b41e1c))
      - add the platform specific build for tools ([461d631](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/461d631acae9daec77c9668216280cbf66240249))
      - add the TRUSTED_BOARD_BOOT support ([beb625f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/beb625f90bfd1858b9d413cae67457e57c79a118))
      - allow to override MTD base offset ([e0bbc19](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e0bbc190d500e53ee0566af85639d3cdbbe7177d))
      - configure the serial boot load address ([4b2f23e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4b2f23e55f27b6baccf3e858234e69685d51fcf4))
      - extend STM32MP_EMMC_BOOT support to FIP format ([95e4908](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/95e4908e17fbb44aed1f8612fefdd6d21fef8f49))
      - manage second NAND OTP on STM32MP13 ([d3434dc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d3434dca0b3acb902fe3a6cf39065ba917f69b1c))
      - manage STM32MP13 rev.Y ([a3f97f6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a3f97f66c36e987a6617f1f39c3b9e64b763212c))
      - optionally use paged OP-TEE ([c4dbcb8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c4dbcb885201c89a44df203661af007945782993))
      - remove unused function from boot API ([f30034a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f30034a298a8d7260464cbcf2d2306bff533d6dd))
      - retrieve FIP partition by type UUID ([1dab28f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1dab28f99dfa03dc11538056a90f00f37bfb1085))
      - save boot auth status and partition info ([ab2b325](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ab2b325c1ab895e626d4e11a9f26b9e7c968f8d8))
      - update ROM code API for header v2 management ([89c0774](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/89c07747d0396b92c83af8736ff49ef8c09bc176))

      - **STM32MP13**

        - change BL33 memory mapping ([10f6dc7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10f6dc789350ed5915a474b2d411890261b741ae))

      - **STM32MP15**

        - manage OP-TEE shared memory ([722ca35](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/722ca35ecc1c5de8682ca8df315a6369d0c21946))

  - **Texas Instruments**

    - **K3**

      - add support for J784S4 SoCs ([4a566b2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4a566b26ae6135d4c13deab9d3f1c40c1cb8960a))

  - **Xilinx**

    - **Versal**

      - add infrastructure to handle multiple interrupts ([e497421](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e497421d7f1e13d15313d1ca71a8e91f370cce1e))
      - get the handoff params using IPI ([205c7ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/205c7ad4cd73e5c091b03f23a3a3be74da5c8aea))
      - resolve the misra 10.1 warnings ([b86e1aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b86e1aade1c0953bd60ae0b35f1c3571ee8bae3f))
      - update macro name to generic and move to common place ([f99306d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f99306d49ba074279c5402a0a34e6bc9797d77de))

      - **Versal NET**

        - add support for QEMU COSIM platform ([6a079ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a079efd909b459448f561618df24fa94038dbad))
        - add documentation for Versal NET SoC ([4efdc48](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4efdc488961502033262613b6f20abcee68bbf84))
        - add SMP support for Versal NET ([8529c76](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8529c7694f8d614e76dcc80b394ec8a6751df44c))
        - add support for IPI ([0bf622d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0bf622de68cd353a8406f76647b6afd8791d675d))
        - add support for platform management ([0654ab7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0654ab7f75449307c79789e12be7aab2338edcc3))
        - add support for Xilinx Versal NET platform ([1d333e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d333e69091f0c71854a224e8cfec08695b7d1f3))

    - **ZynqMP**

      - optimization on pinctrl_functions ([314f9f7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/314f9f7957fbab12dc8d073cf054b99520372e0e))
      - add support for ProvenCore ([358aa6b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/358aa6b21118ae4eedf816f663aa950b58f7fd4e))
      - add support for xck24 silicon ([86869f9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/86869f99d0c144ed18fb947866554a4a56b67741))
      - protect eFuses from non-secure access ([d0b7286](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d0b7286e48f0a34e7e9a8db3948caf1809193430))
      - resolve the misra 10.1 warnings ([bfd7c88](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bfd7c881905702082e3c2a56d5228ccf5fe98f11))

- **Bootloader Images**

  - add interface to query TF-A semantic ver ([dddf428](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dddf4283b043ad0a81d27bd5bb2f0c647c511e11))

  - **BL32**

    - **TSP**

      - add FF-A support to the TSP ([4a8bfdb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4a8bfdb90956ecec02ba5e189fe5452817a65179))
      - add ffa_helpers to enable more FF-A functionality ([e9b1f30](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e9b1f300a974a7e82190b95899c3128b73088488))
      - enable test cases for EL3 SPMC ([15ca1ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/15ca1ee342a4dcd8a73a4ae158d245cd4266c832))
      - increase stack size for tsp ([5b7bd2a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b7bd2af0b2972dfffeaa674947c0082d6b5126b))

- **Services**

  - add a SPD for ProvenCore ([b0980e5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b0980e584398fc5adc908cd68f1a6deefa943d29))

  - **RME**

    - **RMMD**

      - add support for RMM Boot interface ([8c980a4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8c980a4a468aeabb9e49875fec395c625a0c2b2b))
      - add support to create a boot manifest ([1d0ca40](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d0ca40e9084903d21e570bb312646626aaf574b))

  - **SPM**

    - add tpm event log node to spmc manifest ([054f0fe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/054f0fe1361ba0cb339fb0902470988a82a24cf7))

    - **SPMD**

      - avoid spoofing in FF-A direct request ([5519f07](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5519f07cd46a4139615a3e8f5e57d1834b23a6f8))

  - **DRTM**

    - add a few DRTM DMA protection APIs ([2b13a98](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2b13a985994213f766ada197427f96e064f1b59b))
    - add DRTM parameters structure version check ([c503ded](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c503ded2c5d9ceec9fba4cc0901805307a14af3d))
    - add Event Log driver support for DRTM ([4081426](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40814266d53b7154daf5d212de481b397db43823))
    - add PCR entries for DRTM ([ff1e42e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff1e42e20aa247ba11cf81742abff07ece376ba8))
    - add platform functions for DRTM ([2a1cdee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2a1cdee4f5e6fe0b90399e442075880acad1869e))
    - add remediation driver support in DRTM ([1436e37](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1436e37dcb894a539a22da48a34ef01566ae728b))
    - add standard DRTM service ([e62748e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e62748e3f1f16934f0ef2d5742f3ca0b125eaea2))
    - check drtm arguments during dynamic launch ([40e1fad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40e1fad69b9f28ab5e57cea33261bf629b05519c))
    - ensure that no SDEI event registered during dynamic launch ([b1392f4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b1392f429cdd368ea2b8e183a1ac0fb31deaf694))
    - ensure that passed region lies within Non-Secure region of DRAM ([764aa95](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/764aa951b2ca451694c74791964a712d423d8206))
    - flush dcache before DLME launch ([67471e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/67471e75b3cf48c361e71894a666bce4395bbb35))
    - introduce drtm dynamic launch function ([bd6cc0b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bd6cc0b2388c52f2b232427be61ff52c042d724a))
    - invalidate icache before DLME launch ([2c26597](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c265975a76977c6373636f5f28e114d1b73e10e))
    - prepare DLME data for DLME launch ([d42119c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d42119cc294fbca2afc263fe5e44538a0ca5e7b8))
    - prepare EL state during dynamic launch ([d1747e1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d1747e1b8e617ad024456791ce0ab8950bb282ca))
    - retrieve DRTM features ([e9467af](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e9467afb2d483ccec8f816902624d848e8f21d86))
    - take DRTM components measurements before DLME launch ([2090e55](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2090e55283c4bf85c7a61735ca0e872745c55896))
    - update drtm setup function ([d54792b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d54792bd93f76b943bf0559c8373b898e0e3b93c))

- **Libraries**

  - **CPU Support**

    - add library support for Hunter ELP ([8c87bec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8c87becbc64f2e233ac905aa006d5e15a63a9a8b))
    - add a64fx cpu to tf-a ([74ec90e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/74ec90e69bbd0e932a61f5461eedc4abd1b99d44))
    - make cache ops conditional ([04c7303](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/04c7303b9c3d2215eebc3d59431519990abe03d0))
    - remove plat_can_cmo check for aarch32 ([92f8be8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/92f8be8fd1e77be67e9c9711afa8705204758304))
    - update doc and check for plat_can_cmo ([a2e0123](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a2e0123484e62df8ed9f2943dbd158471bf31221))

  - **OP-TEE**

    - check paged_image_info ([c0a11cd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0a11cd8698394e1d3d3d7c9cedb19846ba59223))

  - **PSCI**

    - add a helper function to ensure that non-boot PEs are offline ([ce14a12](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce14a12f8b8f02b7221f37c7c4b46f909c1a4346))

  - **C Standard Library**

    - introduce __maybe_unused ([351f9cd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/351f9cd8897fd3ea52db2421721a152494b16328))

  - **PSA**

    - add delegated attestation partition API ([4b09ffe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4b09ffef49663ebc8c8f5c3da19636208fe2fa06))
    - remove initial attestation partition API ([420deb5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/420deb5a0dbbd35962e5449f82434c703e7a1179))

- **Drivers**

  - **Authentication**

    - allow to verify PublicKey with platform format PK ([40f9f64](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40f9f644e8af34e745dbaec73d7128c0a4902e54))
    - enable MBEDTLS_CHECK_RETURN_WARNING ([a4e485d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4e485d7bf1c428d64e90e9821e4b1a109d10626))

    - **Crypto**

      - update crypto module for DRTM support ([e43caf3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e43caf3890817e91b3d35b5ae1149a208f1a4016))

    - **mbedTLS**

      - update mbedTLS driver for DRTM support ([8b65390](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8b653909b7e2371c6dcddbeac112b9671c886f34))

  - **I/O**

    - **MTD**

      - add platform function to allow using external buffer ([f29c070](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f29c0702d2e7a67327b67766f91793d8ae6d0f73))

  - **MMC**

    - get boot partition size ([f462c12](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f462c1249ac41f43423011bb12ace38cbeb0af4c))
    - manage SD Switch Function for high speed mode ([e5b267b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e5b267bba14c55e7906d120c52d4e8e8bbb68df6))

  - **MTD**

    - add platform function to allow using external buffer ([f29c070](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f29c0702d2e7a67327b67766f91793d8ae6d0f73))

  - **GUID Partition Tables Support**

    - allow to find partition by type UUID ([564f5d4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/564f5d477663bc007916a11c48bdd8b9be4ad369))

  - **SCMI**

    - send powerdown request to online secondary cpus ([14a2892](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/14a289230918b23b0985e215d38614dc7480bd02))
    - set warm reboot entry point ([5cf9cc1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5cf9cc130a90fd8c4503c57ec4af235b469fd473))

  - **Arm**

    - **Ethos-N**

      - add support for SMMU streams ([b139f1c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b139f1cf975f9968eb8bd1182a173b976ecf06f9))

    - **GIC**

      - add APIs to raise NS and S-EL1 SGIs ([dcb31ff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dcb31ff79096fc88b45df8068e5de83b93f833ed))

      - **GICv3**

        - validate multichip data for GIC-700 ([a78b3b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a78b3b382b07675a89a66ddffe926ed225eeb245))

    - **RSS**

      - add new comms protocols ([3125901](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/31259019235aebf7aa533d5c893940f597fb1a8b))

  - **ST**

    - **Crypto**

      - add AES decrypt/auth by SAES IP ([4bb4e83](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4bb4e836498b0131feefbba3f857a0bf3b89e543))
      - add ECDSA signature check with PKA ([b0fbc02](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b0fbc02aea76d31e749444da63b084e6b2bd089b))
      - add STM32 RNG driver ([af8dee2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af8dee20d5fee29f34ccd9b9556e0c23655ff549))
      - remove BL32 HASH driver usage ([6b5fc19](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6b5fc19227ff8935b1352c0e4c0d716ebee60aa2))
      - update HASH for new hardware version used in STM32MP13 ([68039f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68039f2d14626adce09512871d6cde20ff45e1d9))

    - **SDMMC2**

      - define FIFO size ([b46f74d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b46f74d4e68ee08b6e912cd7f855a16cc5e79a6a))
      - make reset property optional ([8324b16](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8324b16cd5e0b1ae2f85264a74f879e8fb1bca2a))
      - manage CMD6 ([3deebd4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3deebd4ccf39904d7fe777f53e9dbaa86691d653))

    - **UART**

      - add initialization with the device tree ([d99998f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d99998f76ed2e8676be25e31e9479a90c16c7098))
      - manage STM32MP_RECONFIGURE_CONSOLE ([ea69dcd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea69dcdc737d8b48fec769042922914e988153ef))

- **Miscellaneous**

  - **Debug**

    - add AARCH32 CP15 fault registers ([bb22891](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb2289142cbf0f3546c1034e0500b5dc32aef740))
    - add helpers for aborts on AARCH32 ([6dc5979](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6dc5979a6cb2121e4c16e7bd62e24030e0f42755))

  - **FDTs**

    - **STM32MP1**

      - add CoT and fuse references for authentication ([928fa66](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/928fa66272a0985c900c996912b54904c64d0520))
      - change pin-controller to pinctrl ([44fea93](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44fea93bf729f631f6ae47e06ac7b6012a795791))

      - **STM32MP13**

        - use STM32MP_DDR_S_SIZE in fw-config ([936f29f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/936f29f6b51b3c7f37fd34e30a7f1f7c3944b361))

      - **STM32MP15**

        - add Avenger96 board with STM32MP157A DHCOR SoM ([51e2230](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/51e223058fe70b311542178f1865514745fa7874))
        - add support for STM32MP157C based DHCOM SoM on PDK2 board ([eef485a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eef485abb13b6df9a94137edd82904aab0ecf02d))

  - **SDEI**

    - add a function to return total number of events registered ([e6381f9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e6381f9cf8c0c62c32d5a4765aaf166f50786914))

  - **TBBR**

    - increase PK_DER_LEN size ([1ef303f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1ef303f9f79020330bbd8e48ac652e8f2121a41b))

- **Tools**

  - **Firmware Image Package Tool**

    - add cca, core_swd, plat cert in FIP ([147f52f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/147f52f3e81f7ccf1dae90bc5687ec137feeb46c))

  - **Certificate Creation Tool**

    - define the cca chain of trust ([0a6bf81](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0a6bf811d7f873a180ef4b9f96f5596b26d270c6))
    - update for ECDSA brainpoolP256r/t1 support ([e78ba69](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e78ba69e3525c968118eb91f443b1e9db9eee5f5))

- **Dependencies**

  - **Compiler runtime libraries**

    - update compiler-rt source files ([8a6a956](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8a6a9560b5dcccfb68064c0c8c9b4b47981c6ac7))

  - **libfdt**

    - add function to set MAC addresses ([1aa7e30](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1aa7e302a84bbf46a97bcfbb54b6b6d57de76cee))
    - upgrade libfdt source files ([94b2f94](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/94b2f94bd63258c300b53ad421488c3c4455712b))

  - **zlib**

    - update zlib source files ([a194255](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a194255d75ed9e2ef56bd6e14349a3e7d86af934))

### Resolved Issues

- **Architecture**

  - **Performance Monitors Extension (FEAT_PMUv3)**

    - add sensible default for MDCR_EL2 ([7f85619](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f8561985778cbe5cdc7d57984c818119e87adaf))

  - **Scalable Matrix Extension (FEAT_SME)**

    - add missing ISBs ([46e92f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/46e92f2862326cbe57acecb2d0f3c2ffbcc176d2))

- **Platforms**

  - **Arm**

    - **FVP**

      - fdts: Fix idle-states entry method ([0e3d880](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0e3d88070f69c6aa7cc51a2847cbba3535992397))
      - fdts: fix memtimer subframe addressing ([3fd12bb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3fd12bb8c622917d8491082b1472c39efb89c0cf))
      - fdts: unify and fix PSCI nodes ([6b2721c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6b2721c01691743a65475e82944e2f8868bf0159))

    - **FVP Versatile Express**

      - fdts: Fix vexpress,config-bus subnode names ([60da130](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/60da130a8c5ac29bc35870180c35ca04db506e0f))

    - **Morello**

      - dts: add model names ([30df890](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/30df8904d0f6973bbce1ecb51f14c1e4725ddf0b))
      - dts: fix DP SMMU IRQ ordering ([fba729b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fba729b0ca22be379792ce677296cda075036753))
      - dts: fix DT node naming ([41c310b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/41c310b4f691c1eefcd0234619bc751966389297))
      - dts: fix GICv3 compatible string ([982f258](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/982f2585bb27b58c017af70d852a433f36711db1))
      - dts: fix SCMI shmem/mboxes grouping ([8aeb1fc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8aeb1fcf832d4e06157a1bed1d18ba244c1fe9ee))
      - dts: fix SMMU IRQ ordering ([5016ee4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5016ee44a740127f7865dc26ed0efbbff1481c7e))
      - dts: fix stdout-path target ([67a8a5c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/67a8a5c92e7c65108b3cdf6f4f9dd2de7e22f3cd))
      - dts: remove #a-c and #s-c from memory node ([f33e113](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f33e113c7a7dffd8ed219f25191907fd64bcf19f))
      - dts: use documented DPU compatible string ([3169572](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3169572ed1bf0de17bb813583cab7ea295a8ec8d))
      - move BL31 to run from DRAM space ([05330a4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/05330a49cd91c346a8b9dc3aff35d0032db4d413))

    - **N1SDP**

      - add numa node id for pcie controllers ([2974d2f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2974d2f2d03e842ed5e01e2e04dd3de6c1d07277))
      - mapping Run-time UART to IOFPGA UART0 ([4a81e91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4a81e91f2752a817364e1fccedb08bb453ad5a56))
      - replace non-inclusive terms from dts file ([e6ffafb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e6ffafbeeae8c78abac37475f19899f0c98523ca))

    - **TC**

      - resolve the static-checks errors ([066450a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/066450abf326f1a68a21cdddf29f62eff95041a9))
      - tc2 bl1 start address shifted by one page ([8597a8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8597a8cbc23f0f03a15d013dd44a4ed59c991872))

  - **Intel**

    - fix asynchronous read response by copying data to input buffer ([dd7adcf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dd7adcf3a89a75973a88118eeb867d1c212c4ad0))
    - fix Mac verify update and finalize for return response data ([fbf7aef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fbf7aef408a9f67fabc712bbfd52438290364879))

  - **MediaTek**

    - remove unused cold_boot.[c|h] ([8cd3b69](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8cd3b693d6d5d3db2433a96c5f2905d92a387cc4))
    - switch console to runtime state before leaving BL31 ([fcf4dd9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fcf4dd9f794b28bbfff3ee7d66bac8d5e260f46a))
    - use uppercase for definition ([810d568](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/810d568141050db7d500c5f5ad91efaff93d2036))
    - wrap cold_boot.h with MTK_SIP_KERNEL_BOOT_ENABLE ([24476b2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24476b2e6128dae2ca2ac46344e18f6f02eae7bf))

    - **MT8186**

      - fix SCP permission ([8a998b5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8a998b5aca3ca895a7722e7496a7fd18cd838f94))
      - fix EMI_MPU domain setting for DSP ([28a8b73](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28a8b738feaade74f23af0e889005e687fde38b5))
      - fix the DRAM voltage after the system resumes ([600f168](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/600f168172a9281a0061f84e4da5318e08762aa1))
      - move SSPM base register definition to platform_def.h ([2a2b51d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2a2b51d8f76e2acdabb431e928beb90e0a30c87c))

    - **MT8188**

      - add mmap entry for CPU idle SRAM ([32071c0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/32071c0263899e0e7a4b7f2c754e6363547f33b1))
      - refine c-state power domain for extensibility ([e35f4cb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e35f4cbf80ba671c42644c1ac7f8f6541042c6e5))
      - refine gic init flow after system resume ([210ebbb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/210ebbb0a6a0520cb3a5930c4fefa94baee33462))

  - **NXP**

    - **i.MX**

      - **i.MX 8M**

        - correct serial output for HAB JR0 ([6e24d79](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e24d795094e7fac1edc13336ce0bfd39d98e66f))
        - fix dram retention fsp_table access ([6c8f523](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c8f523138cd94bc0608708e821a09b02c8c2f5a))
        - move caam init after serial init ([901d74b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/901d74b2d46cbd8b1d27477fa16388520fdabab1))
        - update poweroff related SNVS_LPCR bits only ([ad6eb19](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad6eb1951b986f30635025bbdf29e257b6b1e362))

        - **i.MX 8Q**

          - correct architected counter frequency ([21189b8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21189b8e21062b71c9056ac1cf60d25bb018007c))

  - **QEMU**

    - enable SVE and SME ([337ff4f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/337ff4f1dd6604738d79fd3fa275ae74d74256b2))

  - **QTI**

    - adding secure rm flag ([b5959ab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b5959ab029fb0a8a271967b0bd7ef438d59061bd))

  - **Raspberry Pi**

    - **Raspberry Pi 3**

      - tighten platform pwr_domain_pwr_down_wfi behaviour ([028c4e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/028c4e42d8f632d40081b88f66d0d05c7d7c9b23))

  - **Renesas**

    - **R-Car**

      - **R-Car 3**

        - fix RPC-IF device node name ([08ae247](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/08ae2471b1417f1d8083a79771338aa2a00b6711))

  - **Rockchip**

    - align fdt buffer on 8 bytes ([621acbd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/621acbd055d712ab8bf79054911155598fdb74d0))

    - **RK3399**

      - explicitly define the sys_sleep_flag_sram type ([7a5e90a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7a5e90a89d91d6662d3e468893e07c91b3a165ee))

  - **Socionext**

    - **Synquacer**

      - increase size of BL33 ([a12a66d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a12a66d0d6d4732d41a27b1ecbc8874731c78101))

  - **ST**

    - add max size for FIP in eMMC boot part ([e7cb4a8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e7cb4a86b884d2922984d3cd4651fb905650cfd6))
    - add missing string.h include ([0d33d38](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0d33d38334cae909a66c74187a36b5833afb8093))

    - **STM32MP1**

      - enable crash console in FIQ handler ([484e846](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/484e846a03a1af5f88e2e28835b6349cc5977935))
      - fdts: stm32mp1: align DDR regulators with new driver ([9eed71b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9eed71b7221c5fc7ed887f1087e42c9f1a62f581))
      - update the FIP load address for serial boot ([32f2ca0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/32f2ca04bfd2d93329f2f17d9c9d134f339710f9))

      - **STM32MP13**

        - correct USART addresses ([de1ab9f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/de1ab9fe052deba06a0904b10a6e0312ca49658e))

  - **Xilinx**

    - include missing header ([28ba140](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28ba1400216d7c7195929d1bd53f059a440a89a2))
    - miscellaneous fixes for xilinx platforms ([bfc514f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bfc514f10393fb7f4641ad5e75049f3acc246dd2))
    - remove unnecessary header include ([0ee2dc1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ee2dc118c34ceacc921fee196a4ba9102bdfbea))
    - update define for ZynqMP specific functions ([24b5b53](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24b5b53a5922de40e53f0a7ecf65d3d0acc30a0d))

    - **Versal**

      - add SGI register call version check ([5897e13](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5897e135445e2bf3345297fbe9971a113506d714))
      - enable a72 erratum 859971 and 1319367 ([769446a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/769446a6899d840df8aa5746ec32bf7530fc9826))
      - fix code indentation issues ([72583f9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/72583f92e6cc1d691b709e05c3ae280dce016fef))
      - fix macro coding style issues ([80806aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/80806aa1234606bb55af40ae0667cdf4d44423be))
      - fix Misra-C violations in bl31_setup and pm_svc_main ([68ffcd1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68ffcd1bb22f2c2eac6c3329a1974b3e8ec6f515))
      - remove clock related macros ([47f8145](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47f8145324181b86b6f460fb0c92144ef43e4e14))
      - resolve misra 10.1 warnings ([19f92c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/19f92c4cfe014c5495f3073917119385b0014eda))
      - resolve misra 15.6 warnings ([1117a16](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1117a16e0379986ea68581c02fb2fee40937452b))
      - resolve misra 8.13 warnings ([3d2ebe7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3d2ebe756a50c27a00a03ae7f0109ed04681ac96))
      - resolve the misra 4.6 warnings ([f7c48d9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f7c48d9e30e9444f1fdb808ae5d06ed675e335fa))
      - resolve the misra 4.6 warnings ([912b7a6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/912b7a6fe46619e5df55dbd0b95d306f7bb2695c))
      - route GIC IPI interrupts during setup ([04cc91b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/04cc91b43c1d10fcba563e18f06336987e6e3a24))
      - use only one space for indentation ([dee5885](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dee588591328b96d9b9ef908869c8b42bd2632f2))

    - **Versal NET**

      - Enable a78 errata workarounds ([bcc6e4a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bcc6e4a02a88056b9c45ff28f405e09444433528))
      - add default values for silicon ([faa22d4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/faa22d48d9929d57975b84ab76cb595afdcf57f4))
      - use api_id directly without FUNCID_MASK ([b0eb6d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b0eb6d124b1764264778d17b1519bfe62b7b9337))

    - **ZynqMP**

      - fix coverity scan warnings ([1ac6af1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1ac6af1199e2d14492a9d75aaba69bc775e55bd8))
      - ensure memory write finish with dsb() ([ac6c135](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ac6c135c83fe4efa4d6e9b9c06e899b57ce5647a))
      - fix for incorrect afi write mask value ([4264bd3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4264bd33e718023c62a2776e3ca40db88fce8b08))
      - move bl31 with DEBUG=1 back to OCM ([389594d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/389594dfa7e60a720d60f0d55296f91ba1610de5))
      - move debug bl31 based address back to OCM ([0ba3d7a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ba3d7a4ca04486f45d062fab54238d9a554a682))
      - remove additional 0x in %p print ([05a6107](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/05a6107ff18b03f4ca33496268398133abf04aaa))
      - resolve misra 4.6 warnings ([cdb6211](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cdb62114cfcdaeb85e64bcde459342a0a95f58e3))
      - resolve misra 8.13 warnings ([8695ffc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8695ffcfcb3801ea287fae7652ba1c350636831f))
      - resolve MISRA-C:2012 R.10.1 warnings ([c889088](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c889088386432af69e3ca853825c4219884c1cc1))
      - resolve the misra 4.6 warnings ([15dc3e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/15dc3e4f8d9730ce58cc599fb9970d486c8b9202))
      - resolve the misra 4.6 warnings ([ffa9103](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ffa910312c371080f4d0d50eb1354ad05b7be7a8))
      - resolve the misra 8.6 warnings ([7b1a6a0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7b1a6a08ccc7522687f66e6e989bbc597d08ab06))

- **Bootloader Images**

  - **BL31**

    - allow use of EHF with S-EL2 SPMC ([7c2fe62](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7c2fe62f1347bb94d82e9fdd3bc5eaebedaf0bc7))
    - harden check in delegate_async_ea ([d435238](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d435238dc364f0c9f0e41661365f83d83899829d))
    - pass the EA bit to 'delegate_sync_ea' ([df56e9d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/df56e9d199939c571b3fd8f539d213fc36e14494))

- **Services**

  - **RME**

    - refactor RME fid macros ([fb00dc4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fb00dc4a7b208cf416d082bb4367b54286bc8e3b))
    - relax RME compiler requirements ([7670ddb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7670ddb1fb5d4fa5e2e234375f7a4c0763f1c57a))
    - update FVP platform token ([364b4cd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/364b4cddbab859a56e63813aab4e983433187191))
    - use RMM shared buffer for attest SMCs ([dc65ae4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dc65ae46439f4d1be06e3a016fe76319d7a62954))
    - xlat table setup fails for bl2 ([e516ba6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e516ba6de5e248e93156b5261cedbff811226e0e))

    - **RMMD**

      - return X4 output value ([8e51ccc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8e51cccaefc1e0e79ac2f0667ffec1cc46cf7665))

  - **SPM**

    - **EL3 SPMC**

      - check descriptor size for overflow ([eed15e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eed15e4310a7bcd90bf6d66b00037e05186329bb))
      - compute full FF-A V1.1 desc size ([be075c3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/be075c3edf634a2df1065597266c3e41d284287b))
      - deadlock when relinquishing memory ([ac568b2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ac568b2bccb9da71f2bd7f1c7204189d1ff678d9))
      - error handling in allocation ([cee8bb3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cee8bb3b38ea266a5008719548965352ec695cae))
      - fix detection of overlapping memory regions ([0dc3518](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0dc35186669ddaedb3a932e103c3976bc3bf75d6))
      - fix incomplete reclaim validation ([c4adbe6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c4adbe6e67617bb2d4f0ffb1c1daa3395f7ac227))
      - fix location of fragment length check ([21ed9ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21ed9ea32325fc556fa7e907e4995888bd3a3b45))
      - fix relinquish validation check ([b4c3621](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b4c3621e0dc8e7ec6d3229253e0326f12c8fe5a9))

- **Libraries**

  - **CPU Support**

    - fix cpu version check for Neoverse N2, V1 ([03ebf40](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/03ebf409c711e9f2006cedded7dc415dfe566975))
    - workaround for Cortex-A510 erratum 2666669 ([afb5d06](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/afb5d069a6fa049f18e90fa50e714b8a4acc55f4))
    - workaround for Cortex-A710 2216384 ([b781fcf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b781fcf139c3a609f1adffb8097a23eadbed53a9))
    - workaround for Cortex-A710 erratum 2291219 ([888eafa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/888eafa00b99aa06b4ff688407336811a7ff439a))
    - workaround for Cortex-A76 erratum 2743102 ([4927309](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49273098a5ccd87a2084a85f9e47d74fa3ecfc90))
    - workaround for Cortex-A77 erratum 2743100 ([4fdeaff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4fdeaffe860a998e8503b847ecceec60dcddcdc5))
    - workaround for Cortex-A78C erratum 2376749 ([5d3c1f5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d3c1f58905d3b7350e02c4687dceaf0971700b3))
    - workaround for Cortex-X3 erratum 2313909 ([7954412](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/79544126943a90d31d81177655be11f75330ffed))
    - workaround for Neoverse N1 erratum 2743102 ([8ce4050](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8ce40503ad00fe0dd35de6e51551da2b4f08a848))
    - workaround for Neoverse-N2 erratum 2326639 ([43438ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/43438ad1ad6651964e9ae75d35f40aed8d86d088))
    - workaround for Neoverse-N2 erratum 2388450 ([884d515](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/884d515625aa09b22245c32db2fcc9222c7f34fd))
    - workaround for Cortex A78C erratum 2242638 ([6979f47](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6979f47fecfd34ac1405117c23f2e36ecb552a20))
    - workaround for Cortex-A510 erratum 2347730 ([11d448c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/11d448c93463180d03b46e9ba204124ff7ad5116))
    - workaround for Cortex-A510 erratum 2371937 ([a67c1b1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a67c1b1b2b521c888790c68e4201ecce0836a0e9))
    - workaround for Cortex-A710 erratum 2147715 ([3280e5e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3280e5e655ad64b6e299e18624d9c586e6b37cb1))
    - workaround for Cortex-A710 erratum 2371105 ([3220f05](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3220f05ef900addccb6e444d6746e4ed28c9804f))
    - workaround for Cortex-A77 erratum 2356587 ([7bf1a7a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7bf1a7aaaa41034587e43d5805b42da83090b85b))
    - workaround for Cortex-A78C 2132064 ([8008bab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8008babd58f60c91a88ad79df3d32f63596b433a))
    - workaround for Cortex-A78C erratum 2395411 ([4b6f002](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4b6f0026ea2622b3f46cdef5b468853ddd281b39))
    - workaround for Cortex-X2 erratum 2371105 ([bc0f84d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc0f84de40d4f1efddfb50071fff09d32f0ea9b2))
    - workaround for Neoverse-N2 erratum 2376738 ([e6602d4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e6602d4b153b81b49b39c22e70f052f9018687b7))
    - workaround for Neoverse-V1 erratum 1618635 ([14a6fed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/14a6fed5ac14035f578a75a9758f9df7ba4d7496))
    - workaround for Neoverse-V1 erratum 2294912 ([39eb5dd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/39eb5ddbbf98bdb6c012a9d852f489f2f8e15c05))
    - workaround for Neoverse-V1 erratum 2372203 ([57b73d5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/57b73d553305d89da7098f9b53b0a2356ca7ff8b))

  - **EL3 Runtime**

    - **RAS**

      - restrict RAS support for NS world ([46cc41d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/46cc41d5592a16f702f7f0c0c41f8948a3e11cda))
      - trap "RAS error record" accesses only for NS ([00e8f79](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/00e8f79c15d36f65f6c7f127177105e02177cbc0))

  - **FCONF**

    - fix type error displaying disable_auth ([381f465](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/381f465ca92f7c9759e85c1bfb4c95ceda26581e))

  - **PSCI**

    - fix MISRA failure - Memory - illegal accesses ([0551aac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0551aac5637a638d4b9d8865a2c20ec5153de3bf))

  - **GPT**

    - correct the GPC enable sequence ([14cddd7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/14cddd7a58799c8a9d349a4adc0136c1ab5d0b6c))

  - **C Standard Library**

    - pri*ptr macros for aarch64 ([d307229](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d307229d754ae4d833ed50be50420aaf070065bf))

  - **PSA**

    - fix Null pointer dereference error ([c32ab75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c32ab75c41adfe28a60f1ff159012a7d78e72fdc))
    - update measured boot handle ([4d879e1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d879e1e5a40cefae5b5e13086a16741bf3f6d67))
    - add missing semicolon ([d219ead](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d219ead1db5ca02ec7c7905ac01d7b268c5026ae))
    - align with original API in tf-m-extras ([471c989](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/471c9895a630560561717067113e4c4d7127bb9f))
    - extend measured boot logging ([901b0a3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/901b0a3015a652d9eb66c063b0984fade9adf08f))

  - **Context Management**

    - remove explicit ICC_SRE_EL2 register read ([2b28727](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2b28727e6dafdaa08a517b5a97bda5de26cc8919))

  - **Semihosting**

    - fix seek call failure check ([7c49438](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7c4943887477754024f0f736461d9543d502efcc))

- **Drivers**

  - **Authentication**

    - correct sign-compare warning ([ed38366](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ed38366f1dfeb0b0789fd69b400728598ae3c64e))

  - **Measured Boot**

    - add SP entries to event_log_metadata ([e637a5e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e637a5e19da72599229fd2c70e793c123aaf14ca))
    - clear the entire digest array of Startup Locality event ([70b1c02](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/70b1c025003452602f68feb13402c705e44145aa))
    - fix verbosity level of RSS digests traces ([2abd317](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2abd317d27a26bbfa3da7fe3fe709da3fa0f09af))

  - **MMC**

    - remove broken, unsecure, unused eMMC RPMB handling ([86b015e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/86b015eb1be57439c2a01cb35d800c7f1b5c8467))
    - resolve the build error ([ccf8392](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ccf8392ccb105638fe710901d3c7ed6594d9450e))

  - **SCMI**

    - base: fix protocol list querying ([cad90b5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cad90b569db7c547470cca922bd93207adcadfad))
    - base: fix protocol list response size ([d323f0c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d323f0cf000f1d999bf78d89c0037af76b6bf8d8))

  - **UFS**

    - add retries to ufs_read_capacity ([28645eb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28645ebd706fe6ac9f34db9f7be5657fe4cffc1a))
    - fix slot base address computation ([7d9648d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d9648dd6cf3b1dcd90b6917d9d0b545b1c4c975))
    - init utrlba/utrlbau with desc_base ([9d6d1a9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9d6d1a94c99c3a0e89792c5cc118a1d8c8a9dbb7))
    - point utrlbau to header instead of upiu ([9d3f6c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9d3f6c4b6068b3a4747f5d1dc650607876eff583))
    - removes dp and run-stop polling loops ([660c208](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/660c208d9bd2770f295005fc26a9b6f788567f41))
    - retry commands on unit attention ([3d30955](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3d309556c75bcdb59fd4e4178fa2b79aa472dc90))

  - **Arm**

    - **GIC**

      - **GICv3**

        - fix overflow caused by left shift ([6aea762](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6aea7624a01cc39c19d4237c4b108659270a61c5))
        - update the affinity mask to 8 bit ([e689048](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e689048e20af70983e0d384301c408fc725cb5eb))

        - **GIC-600**

          - implement workaround to forward highest priority interrupt ([e1b15b0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e1b15b09a530f2a0b0edc4384e977452d6b389eb))

    - **RSS**

      - clear the message buffer ([e3a6fb8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e3a6fb84f523e68d2f1398348d1ae2635f3e57bc))
      - determine the size of sw_type in RSS mboot metadata ([2c8f2a9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c8f2a9ad45023354516d419dc9fda2a4f02812b))
      - fix build issues with comms protocol ([ab545ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ab545efddcdbf5d08ad3b1e8f4ea15a0faf168a7))
      - reduce input validation for measured boot ([13a129e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/13a129e8dcea358033f3c83b2d81b25129e02d43))
      - remove dependency on attestation header ([6aa7154](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6aa71542f35047ea0b537e3a6016de6c579c9d6b))
      - rename AP-RSS message size macro ([70247dd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/70247ddbbd0a55a1ddf1d02f2a35b5cad3949dd1))

  - **NXP**

    - **DDR**

      - fix firmware buffer re-mapping issue ([742c23a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/742c23aab79a21803472c5b4314b43057f1d3e84))

  - **ST**

    - **Clock**

      - correct MISRA C2012 15.6 ([56f895e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/56f895ede3a2a4a97c0e4f8270050aff20a167bc))
      - correctly check ready bit ([3b06a53](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3b06a53044e754979cb0608fd93a137a5879a6a0))

- **Miscellaneous**

  - **AArch64**

    - make AArch64 FGT feature detection more robust ([c687776](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c6877763cd3a286983df160c8207368174c1b820))

  - **Debug**

    - backtrace stack unwind misses lr adjustment ([a149eb4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a149eb4d87453f58418ad32c570090739a3e0dd6))
    - decouple "get_el_str()" from backtrace ([0ae4a3a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ae4a3a3f0cd841b83f2944dde9837ea67f08813))

  - **FDTs**

    - **STM32MP1**

      - **STM32MP13**

        - align sdmmc pins with kernel ([c7ac7d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c7ac7d65a7d1ee1b656bf1260ede6b8e2226bbac))
        - cleanup DT files ([4c07deb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4c07deb53e0e7daafc93bc67fdcbb3de7b73d730))
        - correct PLL nodes name ([93ed4f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/93ed4f0801f5b3571abdd7e039d09d508c987063))
        - remove secure status ([8ef8e0e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8ef8e0e30e301e6b2595d571f004ae86b1a1ce06))
        - update SDMMC max frequency ([c9a4cb5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c9a4cb552cdd168fcab2c0383b8fbe30dc99092f))

  - **Security**

    - optimisations for CVE-2022-23960 ([e74d658](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e74d658181e5e69b6b5e16b40adc1ffef4c1efb9))

- **Documentation**

  - document missing RMM-EL3 runtime services ([e50fedb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e50fedbc869341d044d4cb3479a0ab3d4edaf225))
  - add LTS maintainers ([ab0d4d9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ab0d4d9d44fe54535a0ae647092a3cfff368f126))
  - update maintainers list ([f23ce63](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f23ce639050481cda939b9e4738ed01d46481ee3))

  - **Changelog**

    - fix the broken link to commitlintrc.js ([c1284a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c1284a7f93309c88fd781d2b4720f742e147284e))

- **Build System**

  - disable default PIE when linking ([7b59241](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7b5924184566bcdcc01966905ffdcabcd6ea4b32))
  - discard sections also with SEPARATE_NOBITS_REGION ([64207f8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/64207f858f5cbf44aa6528be19a863acc4444568))
  - ensure that the correct rule is called for tools ([598b166](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/598b166bbc2f09fc219d44ecff0c870854bfa093))
  - fix arch32 build issue for clang ([94eb127](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/94eb127719881f39c7f235c887fb2c0b82341696))
  - make TF-A use provided OpenSSL binary ([e95abc4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e95abc4c01822ef43e9e874d63d6596dc0b57279))

- **Tools**

  - **Secure Partition Tool**

    - fix concurrency issue for SP packages ([0aaa382](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0aaa382fe2395c82c9491b199b6b82819afd368f))
    - operators "is/is not" in sp_mk_gen.py ([1a28f29](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1a28f290b8224eb1d78a2476faaedc5154f82208))
    - 'sp_mk_generator.py' reference to undef var ([0be2475](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0be2475f6990a37d2d54b7ed06bac9cb46f4660d))

- **Dependencies**

  - add missing aeabi_memcpy.S ([93cec69](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/93cec697deb654303379cae8f25a31dc8b90cd31))

## [2.7.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.6..refs/tags/v2.7.0) (2022-05-20)

### New Features

- **Architecture**

  - **Statistical profiling Extension (FEAT_SPE)**

    - add support for FEAT_SPEv1p2 ([f20eb89](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f20eb893a072bb9b404eedb886e8c65fe76ffb45))

  - **Branch Record Buffer Extension (FEAT_BRBE)**

    - add BRBE support for NS world ([744ad97](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/744ad97445ce7aa65adaef376d0b5bafc12a90d3))

  - **Extended Cache Index (FEAT_CCIDX)**

    - update the do_dcsw_op function to support FEAT_CCIDX ([d0ec1cc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d0ec1cc437c59e64ecba44710dbce82a04ff892d))

- **Platforms**

  - add SZ_* macros ([1af59c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1af59c457010e6e3e6536752736eb02115bca543))

  - **Allwinner**

    - add SMCCC SOCID support ([436cd75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/436cd754f2b0f9c0ce3094961bd1e179eeff2fc1))
    - allow to skip PMIC regulator setup ([67412e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/67412e4d7ae3defaac78ef5e351c63e06cfd907a))
    - apx803: add aldo1 regulator ([a29f6e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a29f6e76cbf76d509c00f84f068b59864d210dfd))
    - choose PSCI states to avoid translation ([159c36f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/159c36fd2fc5afbe979e5028b9e845ed4b7a40f1))
    - provide CPU idle states to the rich OS ([e2b1877](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e2b18771fc2a0528dda18dbdaac08dd8530df25a))
    - simplify CPU_SUSPEND power state encoding ([52466ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52466ec38ef312da62ad062720a03a183329f831))

  - **Arm**

    - **FVP**

      - measure critical data ([cf21064](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cf21064ec8a1889f64de48e30e38285227d27745))
      - update HW_CONFIG DT loading mechanism ([39f0b86](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/39f0b86a76534d0b7c71dd0c8b34f1a74480386b))
      - enable RSS backend based measured boot ([c44e50b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c44e50b72567205650c6455f3a258f36af0c84dd))

    - **Morello**

      - add changes to enable TBBR boot ([4af5397](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4af53977533bee7b5763d3efad1448545c2ebef7))
      - add DTS for Morello SoC platform ([572c8ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/572c8ce255397f7cff9640676e510817a8e4c6a3))
      - add support for nt_fw_config ([6ad6465](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6ad6465e5ce452688cac079f16d26f64e9f4ce3c))
      - add TARGET_PLATFORM flag ([8840711](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8840711f33131969ec6b62ca3da079cf0573ac8b))
      - configure DMC-Bing mode ([9b8c431](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9b8c431e2b2d656da7f8c4158e3d32e104446fec))
      - expose scmi protocols in fdts ([87639aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/87639aab0b6a30d4f49d069c0ea06900b11072a6))
      - split platform_info sds struct ([4a7a9da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4a7a9dafbc953089957a0cc1a7183731a5b003e1))
      - zero out the DDR memory space ([2d39b39](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d39b39704c1e4f2a189543ac4ff05ae58e5f5c8))

    - **N1SDP**

      - add support for nt_fw_config ([cf85030](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cf85030efe73439e06295f8185b0a6bebf7b5eae))
      - enable trusted board boot on n1sdp ([fe2b37f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fe2b37f6858168a56c3d393bc72f560468d02165))

    - **RD**

      - **RD-N2**

        - add board support for rdn2cfg2 variant ([efeb438](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/efeb43808d2e3ed23e1d51d5e86460db92971e96))
        - add support for rdedmunds variant ([ef515f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef515f0d3466a8beded4fd662718abbd97391b13))

    - **SGI**

      - add page table translation entry for secure uart ([33d10ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33d10ac8bf134519f303fd7ce5fb5d583be2f515))
      - deviate from arm css common uart related definitions ([f2cccca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f2ccccaa81ec14a80fedb48c37226e5d852ada7a))
      - enable fpregs context save and restore ([18fa43f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18fa43f753b79cfc3cc5426a3ef50b04efbf6206))
      - route TF-A logs via secure uart ([987e2b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/987e2b7c20eb4ab4215ff5289b715300f5cec054))

    - **TC**

      - add reserved memory region for Gralloc ([ad60a42](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad60a42cd79713984065dca8540c091c49755f32))
      - enable CI-700 PMU for profiling ([fbfc598](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fbfc59840f9cd0ea53921c7f6fb9f4850a3b42ee))
      - enable GPU ([82117bb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/82117bb48180175c25936b0ff9e33563e25e18f4))
      - enable SMMU for DPU ([4a6ebee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4a6ebeeca37ece34a58982c8b6ebdc8cfd70814b))
      - enable tracing ([59da207](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/59da207e2f2f028c9051c89bc5a05e95d996c18c))

    - **Corstone-1000**

      - identify bank to load fip ([cf89fd5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cf89fd57ed3286d7842eef41cd72a3977eb6d317))
      - implement platform specific psci reset ([a599c80](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a599c80d063975cbeedbc86cfb619fca8545c487))
      - made changes to accommodate 3MB for optee ([854d1c1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/854d1c103a9b73bbde7ef1b89b06b29e3cc053bb))

  - **Intel**

    - add macro to switch between different UART PORT ([447e699](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/447e699f70f1a1d1b85a8136b445eba689166c5d))
    - add RSU 'Max Retry' SiP SMC services ([4c26957](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4c26957be253a7ab3acb316f42bf3ee10c409ed2))
    - add SiP service for DCMF status ([984e236](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/984e236e0dee46708534a23c637271a931ceb67e))
    - add SMC for enquiring firmware version ([c34b2a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c34b2a7a1a38dba88b6b668a81bd07c757525830))
    - add SMC support for Get USERCODE ([93a5b97](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/93a5b97ec9e97207769db18ae34886e6b8bf2ea4))
    - add SMC support for HWMON voltage and temp sensor ([52cf9c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52cf9c2cd4882534d02e8996e4ff1143ee59290e))
    - add SMC support for ROM Patch SHA384 mailbox ([77902fc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77902fca8fe7449473b09198e1fe197f7b4765d7))
    - add SMC/PSCI services for DCMF version support ([44eb782](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44eb782e15c9af532f2455b37bd53ca93830f6e2))
    - add SMPLSEL and DRVSEL setup for Stratix 10 MMC ([bb0fcc7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb0fcc7e011ec4319a79734ba44353015860e39f))
    - add support for F2S and S2F bridge SMC with mask to enable, disable and reset bridge ([11f4f03](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/11f4f03043ef05762f4d6337804c39dc8f9af54f))
    - allow to access all register addresses if DEBUG=1 ([7e954df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e954dfc2ba83262f7596dd0f17de75163e49e5e))
    - create source file for firewall configuration ([afa0b1a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/afa0b1a82a404c616da2da8f52cdcd587938955f))
    - enable firewall for OCRAM in BL31 ([ae19fef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae19fef33707700a91b0b672aa784e084a6ca500))
    - enable SMC SoC FPGA bridges enable/disable ([b7f3044](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b7f3044e8725d9af997999547630892cf9e2f0ad))
    - extend attestation service to Agilex family ([581182c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/581182c1916df03860744d8e32941c72b2cc3fda))
    - implement timer init divider via cpu frequency. ([#1](https://review.trustedfirmware.org:29418/TF-A/trusted-firmware-a/issues/1)) ([f65bdf3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f65bdf3a54eed8f7651761c25bf6cc7437f4474b))
    - initial commit for attestation service ([d174083](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d17408316db10db611e23716e8a5b9b9f53ad509))
    - single certificate feature enablement ([7facace](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7facacec6328e505b243a4974d045d45fe068afd))
    - support AES Crypt Service ([6726390](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6726390eb02e9659cfaf2d3598be9bf12fbc5901))
    - support crypto service key operation ([342a061](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/342a0618c7ff89327ac5b34dc0713509ffae609b))
    - support crypto service session ([6dc00c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6dc00c24ab0100a2aae0f416c72470f8ed17e149))
    - support ECDH request ([4944686](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49446866a515c2db855d456f39df3d586b2084b7))
    - support ECDSA Get Public Key ([d2fee94](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d2fee94afa6ba7e76508e6bead7eb2936c5eafb8))
    - support ECDSA HASH Signing ([6925410](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/692541051b8cb0f435ae46c5d7351231ee292319))
    - support ECDSA HASH Verification ([7e25eb8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e25eb87016ba8355cf0a3a5f71fb8b8785de044))
    - support ECDSA SHA-2 Data Signature Verification ([5830506](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/583050607e43cef8b544a5700386a019e54c422f))
    - support ECDSA SHA-2 Data Signing ([07912da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07912da1b7663451493fb5e40e4c33deeb18a639))
    - support extended random number generation ([24f9dc8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24f9dc8a43fea350416ca9312a78ab4e786da8ad))
    - support HMAC SHA-2 MAC verify request ([c05ea29](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c05ea2969070be90a7dbb2d0344c66d89401edf6))
    - support session based SDOS encrypt and decrypt ([537ff05](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/537ff052579862a4865d36d06940feaa796d16da))
    - support SHA-2 hash digest generation on a blob ([7e8249a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e8249a2dbacfa751990c47644f0403311c6e260))
    - support SiP SVC version ([f0c40b8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f0c40b897f8a25bc50c53239dcf750dd395ebabf))
    - support version 2 SiP SVC SMC function ID for mailbox commands ([c436707](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c436707bc6eed31ab61408ef40db6063d05f0912))
    - support version 2 SiP SVC SMC function ID for non-mailbox commands ([ad47f14](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad47f1422f3f9aa4a622e08b71fc8f5caab98a98))
    - update to support maximum response data size ([b703fac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b703facaaae1e3fe5afa4742b436bb07e065b5e9))

  - **Marvell**

    - **Armada**

      - **A3K**

        - add north and south bridge reset registers ([a4d35ff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4d35ff381c625d61bcc22f9f9a1a45d8663b19d))

  - **MediaTek**

    - introduce mtk makefile ([500d40d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/500d40d877617653d347fb6308144973d4297ab9))

    - **MT8195**

      - apply erratas of CA78 for MT8195 ([c21a736](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c21a736d6f3fa9fb0647bff404b0174ebf1acd91))
      - add EMI MPU surppot for SCP and DSP ([690cb12](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/690cb1265ea84851bd6405a0a6a57d2f1c9f03a3))
      - dump EMI MPU configurations ([20ef588](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/20ef588e86ad8f3cf13382c164463046db261feb))
      - improve SPM wakeup log ([ab45305](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ab45305062f50f81e5c3f800ef4c6cef5097cb04))

    - **MT8186**

      - add DFD control in SiP service ([e46e9df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e46e9df0d0e05f2aaee613fc4f697fcc8d79c0b3))
      - add SPM suspend driver ([7ac6a76](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ac6a76c47d429778723aa804b64c48220a10f11))
      - add Vcore DVFS driver ([635e6b1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/635e6b108e773daf37c00f46e6fbb1cae4e78f96))
      - disable 26MHz clock while suspending ([9457cec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9457cec8c02f78ba56fd9298dd795766c89281a2))
      - initialize platform for MediaTek MT8186 ([27132f1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/27132f13ca871dc3cf1aa6938995284cf5016e00))
      - add power-off function for PSCI ([a68346a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a68346a772859ee6971ec14c6473d2a853e9c66f))
      - add CPU hotplug ([1da57e5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1da57e54b2270b3b49710afa6fd947b01d61b261))
      - add DCM driver ([95ea87f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/95ea87ffc2445c77f070e6a2f78ffa424810faed))
      - add EMI MPU basic driver ([1b17e34](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b17e34c5d7740a357b2027d88aef7760b346616))
      - add MCDI drivers ([06cb65e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/06cb65ef079941d0525dca75dd0e110e9330906d))
      - add pinctrl support ([af5a0c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af5a0c40aff21c4b8771365f19dcb01d6086b30d))
      - add pwrap and pmic driver ([5bc88ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5bc88ec61c75ed42b41d84817aa4d6ee68a2efc8))
      - add reboot function for PSCI ([24dd5a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24dd5a7b71544c503446e58cb23c0cfd09245a3c))
      - add RTC drivers ([6e5d76b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e5d76bac8786120d037953f5a6fd67aaff035c1))
      - add SiP service ([5aab27d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5aab27dc4294110a6c0b69bf5ec5343e7df883a7))
      - add sys_cirq support ([109b91e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/109b91e38c8d4f73941c8574759560a1f1636d05))
      - apply erratas for MT8186 ([572f8ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/572f8adbb062c36835fbb82944dd2ed772134bfd))
      - initialize delay_timer ([d73e15e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d73e15e66a33398c8fc51c83f975a3f35494faf5))
      - initialize GIC ([206f125](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/206f125cc177bc110eb87d40ffc7fa18b28c01ce))
      - initialize systimer ([a6a0af5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a6a0af57c3369dfc6fc2f25877d812a24e9be311))

  - **NXP**

    - add SoC erratum a008850 ([3d14a30](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3d14a30b88762e901e134acc89c6ac4fa9e3f321))
    - add ifc nor and nand as io devices ([b759727](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b759727f5936a687314168dd8912d30897a8c6be))
    - add RCPM2 registers definition ([d374060](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d374060abe9b63296f63f1e3c811aeeddb7a093c))
    - add CORTEX A53 helper functions ([3ccc8ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3ccc8ac3e5da48819a2fc90ec48a175515de38cb))

    - **i.MX**

      - **i.MX 8M**

        - add a simple csu driver for imx8m family ([71c40d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/71c40d3bb7c90a6c36d5c49d0830ca95aba65a2f))
        - add imx csu/rdc enum type defines for imx8m ([0c6dfc4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c6dfc47847608b6ade0c00716e93afc6725362c))
        - enable conditional build for SDEI ([d2a339d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d2a339dfa1665edf87a30a4318af954e764c205c))
        - enable the coram_s tz by default on imx8mn/mp ([d5ede92](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d5ede92d78c829d8a3adad0759219b79e0dc0707))
        - enable the csu init on imx8m ([0a76495](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0a76495bc2cb0c5291027020a3cd2d3adf31c8ed))
        - do not release JR0 to NS if HAB is using it ([77850c9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77850c96f23bcdc76ecb0ecd27a982c00fde5d9d))
        - switch to xlat_tables_v2 ([4f8d5b0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4f8d5b018efc42d1ffa76fca8efb0d16a57f5edd))

        - **i.MX 8M Mini**

          - enable optee fdt overlay support ([9d0eed1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9d0eed111cb1294605b6d82291fef16a51d35e46))
          - enable Trusty OS on imx8mm ([ff3acfe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff3acfe3cc1658917376152913a9d1b5b9b8de34))
          - add support for measured boot ([cb2c4f9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cb2c4f93c18b948fbfde9d50ab7d30362be0e00a))

        - **i.MX 8M Plus**

          - add trusty for imx8mp ([8b9c21b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8b9c21b480dd5c3265be1105a9462b3f5657a6b1))
          - enable BL32 fdt overlay support on imx8mp ([aeff146](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aeff14640a91f6d33bfdbc0dc7b0e920f6d14b91))

        - **i.MX 8M Nano**

          - enable optee fdt overlay support ([2612891](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/26128912884b26fab67bce9d87ba0e1c85a0be1e))
          - enable Trusty OS for imx8mn ([99349c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99349c8ecba910dabbaa72b9be91f3ed762036f5))

        - **i.MX 8M Q**

          - enable optee fdt overlay support ([023750c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/023750c6a898e77c185839f5e56f8e23538f718a))
          - enable trusty for imx8mq ([a18e393](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a18e393339e1d481f4fdf0d621fe4f39ce93a4fe))

    - **Layerscape**

      - add CHASSIS 3 support for tbbr ([9550ce9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9550ce9ddd7729a961f51ed61ea4b2030e284dcb))
      - add new soc errata a009660 support ([785ee93](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/785ee93cc3bd9b43d88fee5acefbd131bf6f2756))
      - add new soc errata a010539 support ([85bd092](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/85bd0929433875e0b84fdc2046d9ec2cf0164903))
      - add soc helper macro definition for chassis 3 ([602cf53](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/602cf53b6f507cea88f4af5c07bed9325bc7a9b8))
      - define more chassis 3 hardware address ([0d396d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0d396d6455a659c4e679f02fae1f9043713474b0))
      - print DDR errata information ([3412716](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3412716b30260958b30d1fa2e1c6d8cce195cd7d))

      - **LS1043A**

        - add ls1043a soc support ([3b0de91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3b0de9182501fae9de372efd1faaf35a7bf74f68))

        - **LS1043ARDB**

          - add ls1043ardb board support ([e4bd65f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e4bd65fed8a12d06181c1343cf786ac91badb6b0)

      - **LX2**

        - enable DDR erratas for lx2 platforms ([cd960f5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd960f5009ee062bba9c479505caee6bbe644649))

      - **LS1046A**

        - add new SoC platform ls1046a ([cc70859](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc708597fa72094c5a01df60e6538e4a7429c2a0))

        - **LS1046ARDB**

          - add ls1046ardb board support ([bb52f75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb52f7560b62043ed08a753f399dc80e8c1582d3))

        - **LS1046AFRWY**

          - add ls1046afrwy board support ([b51dc56](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b51dc56ab9ea79e4709f0d0ce965525d0d3da918))

        - **LS1046AQDS**

          - add board ls1046aqds support ([16662dc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/16662dc40dd2578d3000528ece090ed39ed18b9c))

      - **LS1088A**

        - add new SoC platform ls1088a ([9df5ba0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9df5ba05b4fe4cd44157363a897b73553ba6e2f1))

        - **LS1088ARDB**

          - add ls1088ardb board support ([2771dd0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2771dd0293b6cda6811e8bed95f2354a3ee0124e))

        - **LS1088AQDS**

          - add ls1088aqds board support ([0b0e676](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0b0e67669814139c6818e61e03d0d0e3314fdc99))

  - **QEMU**

    - add SPMD support with SPMC at S-EL1 ([f58237c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f58237ccd9fd2350730d60ab7de59b5c376bfb35))
    - add support for measured boot ([5e69026](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5e690269d579d9461be3c5f5e3f59d4c666863a0))

  - **QTI**

    - **MSM8916**

      - allow booting secondary CPU cores ([a758c0b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a758c0b65c6730fb07846899d6436ba257484d34))
      - initial platform port ([dddba19](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dddba19a6a3cb7a1039beaffc3169c4eb3291afd))
      - setup hardware for non-secure world ([af64473](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af6447315c8534331513ca6b6556af661e0ba88b))

  - **Renesas**

    - **R-Car**

      - **R-Car 3**

        - modify sequence for update value for WUPMSKCA57/53 ([d9912cf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d9912cf3d1022fc6d38a6059290040985de56e63))
        - modify type for Internal function argument ([ffb725b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ffb725be98ffd010c851629a6da75bf57f770c7f))
        - update IPL and Secure Monitor Rev.3.0.3 ([14d9727](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/14d9727e334300b3f5f57e76a9f6e21431e6c6b5))

  - **ST**

    - add a function to configure console ([53612f7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/53612f72938f37244a5f10ae7c57abe7358c221f))
    - add STM32CubeProgrammer support on UART ([fb3e798](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fb3e7985c9b657c535c02b722ecc413f643e671e))
    - add STM32MP_UART_PROGRAMMER target ([9083fa1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9083fa11ead67272b94329e8f84257de6658620d))
    - add early console in BL2 ([c768b2b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c768b2b22f4fb16cf8be8b4815a1984b29918c20))
    - disable authentication based on part_number ([49abdfd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49abdfd8cececb91a4bc7e7b29a30c09dce461c7))
    - get pin_count from the gpio-ranges property ([d0f2cf3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d0f2cf3b148df75d5cbbd42dfa18012043e5d1f4))
    - map 2MB for ROM code ([1697ad8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1697ad8cc81307972d31cec3b27d58f589eeeb3f))
    - protect UART during platform init ([acf28c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/acf28c267b3679a0770b2010f2ec3fb3c2d19975))
    - update stm32image tool for header v2 ([2d8886a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d8886aceed613b9be25f20900914cacc8bb0fb9))
    - update the security based on new compatible ([812daf9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/812daf916c9c977a4f6d7d745d22b90c8492fc71))
    - use newly introduced clock framework ([33667d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33667d299bd5398ca549f542345e0f321b483d17))

    - **ST32MP1**

      - adaptations for STM32MP13 image header ([a530874](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a5308745ee3ab3b77ca942052e60968bcc01340d))
      - add "Boot mode" management for STM32MP13 ([296ac80](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/296ac8012b77ea84079b38cc60ee786a5f91857f))
      - add a second fixed regulator ([225ce48](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/225ce4822ccf2e7c7c1fca6cf3918d4399158613))
      - add GUID values for updatable images ([8d6b476](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8d6b4764f3e54431c3d01342d39d1efa70c3dbf9))
      - add GUID's for identifying firmware images to be booted ([41bd8b9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/41bd8b9e2ad3b755505684601f07d4f7f8ec04c4))
      - add helper to enable high speed mode in low voltage ([dea02f4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dea02f4eaed855c2f05d8a1d7eefca313e98e5b4))
      - add logic to pass the boot index to the Update Agent ([ba02add](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ba02add9ea8fb9a8b0a533c1065a77c7dda4f2a6))
      - add logic to select the images to be booted ([8dd7553](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8dd755314fdfa077465bd6cd5e248be392d90378))
      - add NVMEM layout compatibility definition ([dfbdbd0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dfbdbd0625990267c6742268118ea748e77c6123))
      - add part numbers for STM32MP13 ([30eea11](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/30eea116cdd66b3fa1e1208e185eb7285a83d898))
      - add regulator framework compilation ([bba9fde](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bba9fdee589fb9a7aca5963f53b7ce67c30520b3))
      - add sdmmc compatible in platform define ([3331d36](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3331d3637c295993a78f22afe7463cf1c334d329))
      - add sign-compare warning ([c10f3a4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c10f3a4559ebf7a654a9719fec619e81e6ee1d69))
      - add stm32_get_boot_interface function ([a6bfa75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a6bfa75cf25241a486ab371ae105ea7ebf2d34d8))
      - add support for building the FWU feature ([ad216c1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad216c106682f1d2565b2a08e11a601b418dc8a4))
      - add support for reading the metadata partition ([0ca180f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ca180f6416160a523ff442f1ad0b768a9a3a948))
      - add timeout in IO compensation ([de02e9b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/de02e9b0ec29548b8ce5ef6ee9adcd9c5edb0518))
      - allow configuration of DDR AXI ports number ([88f4fb8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/88f4fb8fa759b1761954067346ee674b454bdfde))
      - call pmic_voltages_init() in platform init ([ffd1b88](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ffd1b889225a8aec124df9e330f41dc638fd7180))
      - chip rev. Z is 0x1001 on STM32MP13 ([ef0b8a6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef0b8a6c1b1a0eab3626041f3168f82bdb410836))
      - enable BL2_IN_XIP_MEM to remove relocation sections ([d958d10](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d958d10eb360024e15f3c921dc3863a0cee98830))
      - enable format-signedness warning ([cff26c1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cff26c19169dd94857e8180cc46b7aa4ccac574a))
      - get CPU info from SYSCFG on STM32MP13 ([6512c3a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6512c3a62a4a7baaf32597284b242bc7172b7e26))
      - introduce new flag for STM32MP13 ([bdec516](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bdec516ee862bfadc25a4d0c02a3b8d859c1fa25))
      - manage HSLV on STM32MP13 ([fca10a8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fca10a8f1b47231ef92634a0adf1a26cbfc97c2a))
      - manage monotonic counter ([f5a3688](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f5a3688b8608df0f269a0b6df18632ebb9e26a01))
      - new way to access platform OTP ([ae3ce8b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae3ce8b28eac73e9a41fdb28424d9f0f4b5f200e))
      - preserve the PLL4 settings for USB boot ([bf1af15](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf1af154db2c89028a8a551c18885add35d38966))
      - register fixed regulator ([967a8e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/967a8e63c33822680e3a4631430dcd9a4a64becd))
      - remove unsupported features on STM32MP13 ([111a384](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/111a384c90afc629e644e7a8284abbd4311cc6b3))
      - retry 3 times FWU trial boot ([f87de90](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f87de907c87e5b2091592c131c4d3d2f737bef01))
      - select platform compilation either by flag or DT ([99a5d8d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99a5d8d01d38474b056766651bd746a4fe93ab20))
      - skip TOS_FW_CONFIG if not in FIP ([b706608](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b7066086424c2f6fd04880db852306d6f487962e))
      - stm32mp_is_single_core() for STM32MP13 ([7b48a9f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7b48a9f3286b8f174acf8821fec48fd2e4771514))
      - update BACKUP_BOOT_MODE for STM32MP13 ([4b031ab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4b031ab4c50d0b9f7127daa7f4eec634f39de970))
      - update boot API for header v2.0 ([5f52eb1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f52eb15970e57d2777d114948fc1110e3dd3f6c))
      - update CFG0 OTP for STM32MP13 ([1c37d0c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1c37d0c1d378769249c797de5b13d73cf6f17a53))
      - update console management for SP_min ([aafff04](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aafff0435448c8409935132be41758e0031f0822))
      - update IO compensation on STM32MP13 ([8e07ab5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8e07ab5f705b213af28831f7c3e9878154e07df0))
      - update IP addresses for STM32MP13 ([52ac998](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52ac9983d67522b6b821391941c8b0d01fd68941))
      - update memory mapping for STM32MP13 ([48ede66](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/48ede6615168118c674288f2e4f8ee1b11d2fa02))
      - updates for STM32MP13 device tree compilation ([d38eaf9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d38eaf99d327bc1400f51c87b6d8a2f92cd828c6))
      - usb descriptor update for STM32MP13 ([d59b9d5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d59b9d53b9cfb2443575c62c6716eb5508374a7b))
      - use clk_enable/disable functions ([c7a66e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c7a66e720ae1a1a5ef98eaf9ff327cd352549010))
      - use only one filter for TZC400 on STM32MP13 ([b7d0058](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b7d0058a3a9153a3863cf76a6763ea751b3ab48d))
      - warn when debug enabled on secure chip ([ac4b8b0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ac4b8b06eb23134d2a9002834541d33f8d43661b))

  - **Texas Instruments**

    - add enter sleep method ([cf5868b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cf5868b8cd7239dee69bdf6ba3ab87bd06bf15f5))
    - add gic save and restore calls ([b40a467](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b40a467783e5911f97d6e92ebdeb34ca2f005552))
    - add PSCI handlers for system suspend ([2393c27](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2393c27680a1ec636e413051e87e986df5a866fe))
    - allow build config of low power mode support ([a9f46fa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a9f46fad82b807a9f0a967245e3ac10ee8dd0ef1))
    - increase SEC_SRAM_SIZE to 128k ([38164e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/38164e64bd853a8329475e9168c5fcb94ecc528b))

  - **Xilinx**

    - **Versal**

      - add SPP/EMU platform support for versal ([be73459](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/be73459a945d8fa781fcc864943ccd0a8d92421c))
      - add common interfaces to handle EEMI commands ([1397967](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1397967490c9f0ebff0d20a566260d1475fe065e))
      - add SMCCC call TF_A_PM_REGISTER_SGI ([fcf6f46](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fcf6f469318d693a024d42ae2d0f4afb26c1e85d))
      - add support to reset SGI ([bf70449](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf70449ba2d1ffd20b01741c491dc0f565009b3d))
      - add UART1 as console ([2c79149](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c791499c26b40c31ce7f68c3bf0dca777fc62de))
      - enhance PM_IOCTL EEMI API to support additional arg ([d34a5db](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d34a5db8a76abdfc8fa68f43b24b971699086a06))
      - get version for ATF related EEMI APIs ([da6e654](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/da6e654bc8b03ee784d0e96a71c4e591e63930f2))
      - remove the time stamp configuration ([18e2a79](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18e2a79f8a5eaa72a2a7e641c2481beb9f827dce))

    - **ZynqMP**

      - disable the -mbranch-protection flag ([67abd47](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/67abd4762bd563be94e734bb0fe4087e88d5d446))
      - fix section `coherent_ram' will not fit in region `RAM' ([9b4ed0a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9b4ed0af02a8ff1fd9a81af5213fde16d3eb8d92))
      - add feature check support ([223a628](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/223a6284b8a0a3ead884a7f0cf333a464d32e319))
      - add support to get info of xilfpga ([cc077c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc077c22273075db328bd30fa12c28abf9eef052))
      - add uart1 as console ([ea66e4a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea66e4af0baf5d5b905e72f824a672f16a6e0f98))
      - increase the max xlat tables when debug build is enabled ([4c4b961](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4c4b9615b1d9512a4a89aa08e722547cc491a07b))
      - pass ioctl calls to firmware ([76ff8c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/76ff8c459e9e6d105e614d68648bd6680806f93e))
      - pm_api_clock_get_num_clocks cleanup ([e682d38](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e682d38b56854e1586b25d929dbc83543b4c66e4))

- **Bootloader Images**

  - add XLAT tables symbols in linker script ([bb5b942](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb5b942e6f133198daedcca0b74ec598af260a54))

  - **BL2**

    - add support to separate no-loadable sections ([96a8ed1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/96a8ed14b74cca33a8caf567d0f0a2d3b2483a3b))

  - **BL31**

    - aarch64: RESET_TO_BL31_WITH_PARAMS ([25844ff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/25844ff728e4a0e5430ba2032457aba7b780a701))

- **Services**

  - **RME**

    - add dummy platform token to RMMD ([0f9159b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0f9159b7ebb7e784a8ed998869ff21095fa105b1))
    - add dummy realm attestation key to RMMD ([a043510](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a0435105f229a65c7861b5997793f905cf90b823))

  - **SPM**

    - update ff-a boot protocol documentation ([573ac37](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/573ac37373d3e8b2c31b3aaeed759e4656e060ec))

    - **EL3 SPMC**

      - allow BL32 specific defines to be used by SPMC_AT_EL3 ([2d65ea1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d65ea1930d4ce26cc176a8c60e9401d0b4f862a))
      - add plat hook for memory transactions ([a8be4cd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a8be4cd057bce5f0b4ac6af396c0c870474d1ef4))
      - add EL3 SPMC #defines ([44639ab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44639ab73e43e0b79da834dff8c85266d68e5066))
      - introduce accessor function to obtain datastore ([6a0788b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a0788bc0e704283e52c80990aa2bb6e047a0cc2))
      - add FF-A secure partition manager core ([5096aeb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5096aeb2ba646548a7a6ab59e975b996e6c9026a))
      - add FFA_FEATURES handler ([55a2963](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/55a296387b9720855df429a08c886f47a4a45057))
      - add FFA_PARTITION_INFO_GET handler ([f74e277](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f74e27723bb54ad1318fa462fbcff70af555b2e6))
      - add FFA_RUN handler ([aad20c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aad20c85cb6f4bc91318d3c6488cf72a20fdbe96))
      - add FFA_RX_RELEASE handler ([f0c25a0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f0c25a082fc8b891d4d21518028118561caa4735))
      - add function to determine the return path from the SPMC ([20fae0a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/20fae0a7ce7fd407cd3efb7745017ee6ab605159))
      - add helper function to obtain endpoint mailbox ([f16b6ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f16b6ee3deac93706efe465f399c9542e12d5eeb))
      - add helper function to obtain hyp structure ([a7c0050](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a7c00505f85684326a223535a319c170d14826f6))
      - add helper to obtain a partitions FF-A version ([c2b1434](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c2b1434369292081f907c548e496f59e197eb2f1))
      - add partition mailbox structs ([e1df600](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e1df6008d9b4a00da25ec08fbdcbd3a5967fdb54))
      - add support for direct req/resp ([9741327](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9741327df577c3f43db42b26bda607429e62af0b))
      - add support for FF-A power mgmt. messages in the EL3 SPMC ([59bd2ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/59bd2ad83c13ed3c84bb9b841032c95927358890))
      - add support for FFA_MSG_WAIT ([c4db76f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c4db76f066f236fe490ebc7a50833a04e08f5151))
      - add support for FFA_SPM_ID_GET ([46872e0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/46872e01f5efb555fef8367595b59e5d2f75cec0))
      - add support for forwarding a secure interrupt to the SP ([729d779](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/729d7793f830781ff8ed44d144c3346c6e4251a3))
      - add support for handling FFA_ERROR ABI ([d663fe7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d663fe7a3002ff028c190eb732278b878e78b7b7))
      - add support for v1.1 FF-A boot protocol ([2e21921](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e21921502b1317031cf2a2f69c5d47ac88a505d))
      - add support for v1.1 FF-A memory data structures ([7e804f9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e804f9695c48681c91e9e6fc6175eb6997df867))
      - enable building of the SPMC at EL3 ([1d63ae4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d63ae4d0d8374a732113565be90d58861506e39))
      - enable checking of execution ctx count ([5b0219d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b0219ddd5da42413f4c2be9302224b5b71295ff))
      - enable handling FF-A RX/TX Mapping ABIs ([1a75224](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1a752245ecae6487844c57667e24b704e6df8079))
      - enable handling FFA_VERSION ABI ([0c7707f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c7707fdf21fc2a8658f5a4bdfd2f8883d02ada5))
      - enable handling of the NS bit ([0560b53](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0560b53e71ab6daefa8e75665a718605478746a4))
      - enable parsing of messaging methods from manifest ([3de378f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3de378ff8c9430c964cbe9b0c58fa5afc4d237ce))
      - enable parsing of UUID from SP Manifest ([857f579](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/857f5790da3770a9ca52416274eec4e545c9be53))
      - enable the SPMC to pass the linear core ID in a register ([f014300](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f0143004e548582760aacd6f15f5499b18081a69))
      - prevent read only xlat tables with the EL3 SPMC ([70d986d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/70d986ddbbf56a20c7550c079dd4dc9462332594))
      - support FFA_ID_GET ABI ([d5fe923](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d5fe92350cb018ae7083ed26a6a16508ccd82a86))
      - allow forwarding of FFA_FRAG_RX/TX calls ([642db98](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/642db9840712044b9c496e04a7acd60580e54117))
      - enable handling of FF-A SMCs with the SPMC at EL3 ([bb01a67](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb01a67306f47271adde051e541c760028c1a0f1))
      - update SPMC init flow to use EL3 implementation ([6da7607](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6da76075bf4b953d621aa15c379e62a5f785de3f))
      - add logical partition framework ([7affa25](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7affa25cad400101c016082be2d102be0f4fce80))
      - add FF-A memory management code ([e0b1a6d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e0b1a6d59e57c0dbe87f5b8f8166f1123664f058))
      - prevent duplicated sharing of memory regions ([fef85e1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fef85e1e53fcf44e8d9ed50c89d8a764bf1b7738))
      - support multiple endpoints in memory transactions ([f0244e5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f0244e5dd1b8cbab75ef00c1b9b56eed5b3cad4b))

    - **SPMD**

      - forward FFA_VERSION from SPMD to SPMC ([9944f55](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9944f55761c4d5cc1feefaf5e33bf7fb83d8f5f3))
      - enable SPMD to forward FFA_VERSION to EL3 SPMC ([9576fa9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9576fa93a2effc23a533b80dce41d7104a8d200b))
      - add FFA_MSG_SEND2 forwarding in SPMD ([c2eba07](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c2eba07c47f8d831629104eeffcec11ed7d3b0a5))
      - add FFA_RX_ACQUIRE forwarding in SPMD ([d555233](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d555233fe5a04dfd99fd6ac30bacc5284285c131))

    - **SPM MM**

      - add support to save and restore fp regs ([15dd6f1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/15dd6f19da8ee4b20ba525e0a742d0df9e46e071))

- **Libraries**

  - **CPU Support**

    - add library support for Poseidon CPU ([1471475](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1471475516cbf1b4a411d5ef853bd92d0edd542e))
    - add support for Cortex-X1 ([6e8eca7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e8eca78e5db966e10e2fa2737e9be4d5af51fa9))
    - add L1PCTL macro definiton for CPUACTLR_EL1 ([8bbb1d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8bbb1d80a58dbdf96fcabbdebbfbd21d2d5344a4))

  - **EL3 Runtime**

    - add arch-features detection mechanism ([6a0da73](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a0da73647546aea1d10b4b2347bac9d532bcb43))
    - replace ARM_ARCH_AT_LEAST macro with FEAT flags ([0ce220a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ce220afb24f0511332b251952019d7011ccc282))

  - **FCONF**

    - add a helper to get image index ([9e3f409](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9e3f409398af447b1d03001dd981007a9bb1617e))
    - add NS load address in configuration DTB nodes ([ed4bf52](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ed4bf52c33b6860d58a2ffc946bd293ec76bbdaa))

  - **Standard C Library**

    - add support for length specifiers ([701e94b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/701e94b08f382691b0deabd4df882abd87e17ab5))

  - **PSA**

    - add initial attestation API ([0848565](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/084856513d6730a50a3d65ac9c3bdae465117c40))
    - add measured boot API ([758c647](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/758c64715b691be92de623f81032494e38a43cc8))
    - mock PSA APIs ([0ce2072](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ce2072d9b9f419bb19595454395a33a5857ca2f))

- **Drivers**

  - **Generic Clock**

    - add a minimal clock framework ([847c6bc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/847c6bc8e6d55b1c0f31a52407aa61515cd6c612))

  - **FWU**

    - add a function to pass metadata structure to platforms ([9adce87](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9adce87efc8acc947b8b49d700c9773a7f071e02))
    - add basic definitions for GUID handling ([19d63df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/19d63df1af72b312109b827cca793625ba6fcd16))
    - add platform hook for getting the boot index ([40c175e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40c175e75bc442674a5dc793c601b09681158ab9))
    - pass a const metadata structure to platform routines ([6aaf257](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6aaf257de4a4070ebc233f35a09bce4c39ea038c))
    - simplify the assert to check for fwu init ([40b085b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40b085bddf60cf8c533b379ccb41e6668c5080dd))

  - **Measured Boot**

    - add RSS backend ([0442ebd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0442ebd2e9bcf5fa4344d8fa8ef4b69a3b249e33))

  - **GUID Partition Tables Support**

    - add a function to identify a partition by GUID ([3cb1065](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3cb1065581f6d9a8507af8dbca3779d139aa0ca7))
    - cleanup partition and gpt headers ([2029f93](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2029f930097b0c3b1b1faa660032d16ed01a5c86))
    - copy the partition GUID into the partition structure ([7585ec4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7585ec4d36ebb7e286cfec959b2de084eded8201))
    - make provision to store partition GUID value ([938e8a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/938e8a500a25a949cfd25f0cb79f6c1359c9b40c))
    - verify crc while loading gpt header ([a283d19](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a283d19f82ddb635d9d9fa061e7fd956167ebe60))

  - **Arm**

    - **GIC**

      - allow overriding GICD_PIDR2_GICV2 address ([a7521bd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a7521bd5d887bfd69d99a55a81416e38ba9ebc97))

      - **GIC-600AE**

        - disable SMID for unavailable blocks ([3f0094c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3f0094c15d433cd3de413a4633a4ac2b8e1d1f2e))
        - enable all GICD, PPI, ITS SMs ([6a1c17c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a1c17c770139c00395783e7568220d61264c247))
        - introduce support for RAS error handling ([308dce4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/308dce40679f63db504cd3d746a0c37a2a05f473))

    - **SMMU**

      - add SMMU abort transaction function ([6c5c532](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c5c5320511ab8202fb9eccce9e66b4e4e0d9a33))
      - configure SMMU Root interface ([52a314a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52a314af254966a604e192fcc3326737354f217a))

    - **MHU**

      - add MHU driver ([af26d7d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af26d7d6f01068809f17cc2d49a9b3d573c640a9))

    - **RSS**

      - add RSS communication driver ([ce0c40e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce0c40edc93aa403cdd2eb6c630ad23e28b01c3e))

    - **TZC**

      - **TZC-380**

        - add sub-region register definition ([fdafe2b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fdafe2b5ead66a1b5175db77bcc7cedafa14a059))

  - **Marvell**

    - **Armada**

      - **A3K**

        - **A3720**

          - preserve x1/x2 regs in console_a3700_core_init() ([7c85a75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7c85a7572960efbaabe20c9db037bcec66be3e98))

  - **MediaTek**

    - **APU**

      - add mt8195 APU clock and pll SiP call ([296b590](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/296b590206aa6db51e5c82b1a97a4f9707b49c4d))
      - add mt8195 APU iommap regions ([339e492](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/339e4924a7a3fd11bc176e0bf3e01d76133d364c))
      - add mt8195 APU mcu boot and stop SiP call ([88906b4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/88906b443734399be5c07a5bd690b63d3d82cefa))

  - **NXP**

    - **DCFG**

      - add Chassis 3 support ([df02aee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/df02aeeec640d2358301e903d9c8c473d455be9e))
      - add gic address align register definition ([3a8c9d7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3a8c9d78d4c65544d789bd64bd005ac10b5b352d))
      - add some macro definition ([1b29fe5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b29fe534b8732193850fced2da1dc449450bd3b))

    - **NXP Crypto**

      - add chassis 3 support ([d60364d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d60364d48e31b33b57049d848b7462eb0e0de612))

    - **DDR**

      - add rawcard 1F support ([f2de48c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f2de48cb143c20ccd7a9c141df3d34cae74049de))
      - add workaround for errata A050958 ([291adf5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/291adf521a54a365e54964bff4dae53d51c65936))

    - **GIC**

      - add some macros definition for gicv3 ([9755fd2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9755fd2ec2894323136715848910b13053cfe0ce))

    - **CSU**

      - add bypass bit mask definition ([ec5fc50](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ec5fc501f15922967bf5d8260072ba1f9aec9640))

    - **IFC NAND**

      - add IFC NAND flash driver ([28279cf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28279cf2c141caf5e4e7156f874cde6f5a0d271b))

    - **IFC NOR**

      - add IFC nor flash driver ([e2fdc77](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e2fdc77ba4eee91f0d1490e34f0fff552fc55dc9))

    - **TZC-380**

      - add tzc380 platform driver support ([de9e57f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/de9e57ff1f3769e770eac44b94127eb7239a63f2))

  - **ST**

    - introduce fixed regulator driver ([5d6a264](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d6a2646f7759a5a2b3daed0d8aef4588c552ba4))

    - **Clock**

      - add clock driver for STM32MP13 ([9be88e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9be88e75c198b08c508d8e470964720a781294b3))
      - assign clocks to the correct BL ([7418cf3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7418cf397916c97cb4ecf159b1f497a84299b695))
      - check HSE configuration in serial boot ([31e9750](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/31e9750bc17bd472d4f2a3db297461efc301be51))
      - define secure and non-secure gate clocks ([aaa09b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aaa09b713c6f539fb5b2ee7e2dfd75f2d46875f5))
      - do not refcount on non-secure clocks in bl32 ([3d69149](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3d69149a7e9e9a899d57f48bee26f98614f88935))
      - manage disabled oscillator ([bcccdac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bcccdacc7e7b7b985df942b3fae26cb9038a2574))

    - **DDR**

      - add read valid training support ([5def13e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5def13eb01ebac5656031bdc388a215d012fdaf8))

    - **GPIO**

      - allow to set a gpio in output mode ([53584e1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/53584e1d5b2b843ea3bb9e01e3f01ea7c364ee6a))
      - do not apply secure config in BL2 ([fc0aa10](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fc0aa10a2cd3cab887a8baa602891d1f45db2537))
      - add a function to reset a pin ([737ad29](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/737ad29bf992a7a79d538d1e0b47c7f38d9a4b9d))

    - **SDMMC2**

      - allow compatible to be defined in platform code ([6481a8f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6481a8f1e045ac80f0325b8bfe7089ba23deaf7b))
      - manage cards power cycle ([258bef9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/258bef913aa76ead1b10c257d1695d9c0ef1c79d))

    - **ST PMIC**

      - add pmic_voltages_init() function ([5278ec3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5278ec3faf2010fd6aea1d8cd4294dd229c5c21d))
      - register the PMIC to regulator framework ([85fb175](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/85fb175b5ef854bc4607db98a4cfb5f35d822cee))

    - **STPMIC1**

      - add new services ([ea552bf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea552bf5a57b573a6b09e396e3466b3c4af727f0))
      - add USB OTG regulators ([13fbfe0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/13fbfe046e71393961d2c70a4f748a15f9c15f77))

    - **Regulator**

      - add support for regulator-always-on ([9b4ca70](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9b4ca70d97d9a2556752b511ff9fe52012faff02))
      - add a regulator framework ([d5b4a2c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d5b4a2c4e7fd0bcb9f08584b242e69a2e591fb71))

    - **UART**

      - manage oversampling by 8 ([1f60d1b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1f60d1bd33d434b0c82a74e276699ee5a2f63833))
      - add uart driver for STM32MP1 ([165ad55](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/165ad5561ef598ea6261ba082610eeff3f208df7))

- **Miscellaneous**

  - **Debug**

    - update print_memory_map.py ([d16bfe0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d16bfe0feffe6a20399fb91d86fd8f7282b941dd))

  - **DT Bindings**

    - add bindings for STM32MP13 ([1b8898e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b8898eb32c3872a34fc59f4216736f23af0c6ea))
    - add TZC400 bindings for STM32MP13 ([24d3da7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24d3da76d221390bb47d501c2ed77a1a7d2b42e7))

  - **FDT Wrappers**

    - add function to find or add a sudnode ([dea8ee0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dea8ee0d3f13f8d1638745b76e86bd7617bf92e7))

  - **FDTs**

    - add the ability to supply idle state information ([2b2b565](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2b2b565717cc0299e75e8806004d1a3548e9fbf7))

    - **STM32MP1**

      - add DDR support for STM32MP13 ([e6fddbc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e6fddbc995947d4e5a5dc6607c76cd46fdd840e2))
      - add DT files for STM32MP13 ([3b99ab6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3b99ab6e370a01caec14bc5422a86001eaf291b8))
      - add nvmem_layout node and OTP definitions ([ff8767c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff8767cbfc2bb851a2f6cc32fbe3693ddbfb7d12))
      - add st-io_policies node for STM32MP13 ([2bea351](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2bea35122d102492f18c427535ce6c9b7016e356))
      - add support for STM32MP13 DK board ([2b7f7b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2b7f7b751f4b0f7a8a0f4a35407af22cc269e529))
      - update NVMEM nodes ([375b79b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/375b79bb4a773fe6a5dd971272c72bf12155050e))

- **Documentation**

  - context management refactor proposal ([3274226](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/327422633bef112a10579d4daeca0f596cd02911))

  - **Threat Model**

    - Threat Model for TF-A v8-R64 Support ([dc66922](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dc669220d5666c2c808bc11ba81c86a9b071271a))

- **Tools**

  - **Secure Partition Tool**

    - add python SpSetupActions framework ([b1e6a41](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b1e6a41572240839e62099aa00298174b18c696a))
    - delete c version of the sptool ([f4ec476](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f4ec47613fef8db8037195147dc2ac6fb6f154ff))
    - python version of the sptool ([2e82874](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e82874cc9b7922e000dd4d7718e3153e347b1d7)
    - use python version of sptool ([822c727](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/822c72791f791d26e233df0c15a655c3dbd8b117))

### Resolved Issues

- **Architecture**

  - **Activity Monitors Extension (FEAT_AMU)**

    - add default value for ENABLE_FEAT_FGT and ENABLE_FEAT_ECV flags ([820371b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/820371b13028a6f620a62cf73a951883d051666b))
    - fault handling on EL2 context switch ([f74cb0b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f74cb0be8ac80eb3072555cb04eb09375d4cb31f))
    - limit virtual offset register access to NS world ([a4c3945](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4c394561af31ae0417ed9ff3b3152adb7cd5355))

  - **Scalable Vector Extension (FEAT_SVE)**

    - disable ENABLE_SVE_FOR_NS for AARCH32 ([24ab2c0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24ab2c0af74be174acf755a36b3ebba867184e60))

- **Platforms**

  - **Allwinner**

    - improve DTB patching error handling ([79808f1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/79808f10c32d441572666551b1545846079af15b))

  - **Arm**

    - fix fvp and juno build with USE_ROMLIB option ([861250c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/861250c3b26d64f859f5f37686e453d5074fa976))
    - increase ARM_BL_REGIONS count ([dcb1959](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dcb1959161935aa58d2bb852f3cef0b96458a4e1))
    - remove reclamation of functions starting with "init" ([6c87abd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c87abdda400354ebf4f5351086c32a4620475c9))
    - use PLAT instead of TARGET_PLATFORM ([c5f3de8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c5f3de8dabc9b955b6051a6c6116d40b10a84f5d))
    - fix SP count limit without dual root CoT ([9ce15fe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9ce15fe8910580efca46b9f102e117402ce769db))

    - **FVP**

      - FCONF Trace Not Shown ([0c55c10](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c55c10305df6217fd978d58ce203dbad3edd4d5))
      - disable reclaiming init code by default ([fdb9166](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fdb9166b9494402eb2da7e0b004c121b322725e0))
      - extend memory map to include all DRAM memory regions ([e803542](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e80354212f591c8813dec27353e8241e03155b4c))
      - fix NULL pointer dereference issue ([a42b426](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a42b426b8548e3304e995f1a49d2470d71072949))
      - op-tee sp manifest doesn't map gicd ([69cde5c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/69cde5cd9563f0c665862f1e405ae8e8d2818c6e))

    - **Morello**

      - change the AP runtime UART address ([07302a2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07302a23ec1af856b3d4de0439161a8c23414f84))
      - fix SoC reference clock frequency ([e8b7a80](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8b7a80436c2bc81c61fc4703d6580f2fe9226a9))
      - include errata workaround for 1868343 ([f94c84b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f94c84baa2a2bad75397b0ec6a0922fe8a475847))

    - **SGI**

      - disable SVE for NS to support SPM_MM builds ([78d7e81](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/78d7e819798ace643b6e22025dc76aedb199bbd5))

    - **TC**

      - remove the bootargs node ([68fe3ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68fe3cec25bc9ea4e1bafdb1d9f5315e245d650b))

    - **Corstone-1000**

      - change base address of FIP in the flash ([1559450](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1559450132c5e712f4d6896e53e4f1cb521fa465))

  - **Broadcom**

    - allow build to specify mbedTLS absolute path ([903d574](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/903d5742953d9d4b224e71d8b1e62635e83f44a9))
    - fix the build failure with mbedTLS config ([95b5c01](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/95b5c0126b802b894ea0177d973978e06b6a254d))

  - **Intel**

    - add flash dcache after return response for INTEL_SIP_SMC_MBOX_SEND_CMD ([ac097fd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ac097fdf07ad63b567ca751dc518f8445a0baef6))
    - allow non-secure access to FPGA Crypto Services (FCS) ([4837a64](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4837a640934630f8034ceec1bb84cc40673d8a6b))
    - always set doorbell to SDM after sending command ([e93551b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e93551bb3bd8ac43779fa70c7363ee2568da45ca))
    - assert if bl_mem_params is NULL pointer ([35fe7f4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/35fe7f400a7f1d65ff2fee5531d20f6c2f3e6f39))
    - bit-wise configuration flag handling ([276a436](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/276a43663e8e315fa1bf0aa4824051d88705858b))
    - change SMC return arguments for INTEL_SIP_SMC_MBOX_SEND_CMD ([108514f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/108514ff7160a86efb791449a4635ffe0f9fdf2c))
    - configuration status based on start request ([e40910e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e40910e2dc3fa59bcce83ec1cf9a33b3e85012c4))
    - define macros to handle buffer entries ([7db1895](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7db1895f0be2f8c6710bf51d8441d5e53e3ef0fe))
    - enable HPS QSPI access by default ([000267b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/000267be22d3c0077c0fd0a8377ceeed5aada4c3))
    - extend SDM command to return the SDM firmware version ([c026dfe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c026dfe38cfae379457a6ef53130bd5ebc9d7808))
    - extending to support large file size for AES encryption and decryption ([dcb144f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dcb144f1fbcef73ddcc448d5ed6134aa279069b6))
    - extending to support large file size for SHA-2 ECDSA data signing and signature verifying ([1d97dd7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d97dd74cd128edd7ad45b725603444333c7b262))
    - extending to support large file size for SHA2/HMAC get digest and verifying ([70a7e6a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/70a7e6af958f3541476a8de6baac8e376fcc67f9))
    - fix bit masking issue in intel_secure_reg_update ([c9c0709](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c9c070994caedf123212aad23b6942122c5dd793))
    - fix configuration status based on start request ([673afd6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/673afd6f8e7266900b00a7cbeb275fe1a3d69cce))
    - fix ddr address range checker ([12d71ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/12d71ac6627bb6822a0314e737794a8503df79dd))
    - fix ECC Double Bit Error handling ([c703d75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c703d752cce4fd101599378e72db66ccf53644fa))
    - fix fpga config write return mechanism ([ef51b09](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef51b097bfa906bf1cee8ee641a1b7bcc8c5f3c0))
    - flush dcache before sending certificate to mailbox ([49d44ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49d44ec5f357b1bcf8eae9e91fbd72aef09e00dd))
    - get config status OK status ([07915a4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07915a4fd5848fbac69dcbf28f00353eed10a942))
    - introduce a generic response error code ([651841f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/651841f20110ce6fac650e3ac47b0a9cce18e6f3))
    - make FPGA memory configurations platform specific ([f571183](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f571183b066b1a91b7fb178c3aad9d6360d1918c))
    - modify how configuration type is handled ([ec4f28e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ec4f28ecec8887a685d6119c096ad346da1ea53e))
    - null pointer handling for resp_len ([a250c04](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a250c04b0cc807f626df92a7091ff13b3a3aa9ed))
    - refactor NOC header ([bc1a573](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc1a573d5519f121cb872fce1d88fe2e0db07b2c))
    - reject non 4-byte align request size for FPGA Crypto Service (FCS) ([52ed157](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52ed157fd66812debb13a792c21f763de01aef70))
    - remove redundant NOC header declarations ([58690cd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/58690cd629b4ccdefe5313f805219598074a3501))
    - remove unused printout ([0d19eda](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0d19eda0dd2ffae27d0551b1f0a06a2b8f96c853))
    - update certificate mask for FPGA Attestation ([fe5637f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fe5637f27aebfdab42915c2ced2c34d8685ee2bb))
    - update encryption and decryption command logic ([02d3ef3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/02d3ef333d4a0a07a3e40defb12a8cde3a7cba03))
    - use macro as return value ([e0fc2d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e0fc2d1907b1c8a062c44a435be77a12ffeed84b))

  - **Marvell**

    - **Armada**

      - **A3K**

        - change fatal error to warning when CM3 reset is not implemented ([30cdbe7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/30cdbe7043832f7bd96b40294ac062a8fc9c540f))
        - fix comment about BootROM address range ([5a60efa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5a60efa12a57cde98240f861e45609cb9b94d58d))

  - **Mediatek**

    - **MT8186**

      - remove unused files in drivers/mcdi ([bc714ba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc714bafe7ae8ca29075ba9bf3985c0e15ae0f64))
      - extend MMU region size ([0fe7ae9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0fe7ae9c64aa6f6d5b06a80de9c88081057d5dbe))

  - **NVIDIA**

    - **Tegra**

      - **Tegra 194**

        - remove incorrect erxctlr assert ([e272c61](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e272c61ce8185deb397dcf168ec72bdaa5926a33))

  - **NXP**

    - fix total dram size checking ([0259a3e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0259a3e8282ed17c1d378a27f820f44b3bebab07))
    - increase soc name maximum length ([3ccd7e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3ccd7e45a2c3ff9fa7794f0284c9d0298e7cb982))

    - **i.MX**

      - **i.MX 8M**

        - check the validation of domain id ([eb7fb93](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eb7fb938c3ce34ccfb143ae8ba695df899098436))

        - **i.MX 8M Plus**

          - change the BL31 physical load address ([32d5042](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/32d5042204e8b41caa4c0c1ed5b48bad9f1cb1b5))

    - **Layerscape**

      - fix build issue of mmap_add_ddr_region_dynamically ([e2818d0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e2818d0afc20a60d302f85f4c915e4ae4cc3cb9c))
      - fix coverity issue ([5161cfd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5161cfde9bfaa3a715d160fcd4870f276adad332))
      - update WA for Errata A-050426 ([72feaad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/72feaad980cdc472868bc95914202bf57ed51b2d))

      - **LX2**

        - drop erratum A-009810 ([e36b0e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e36b0e4910aea56f90a6ab9b8cf3dc4008220031))

  - **Renesas**

    - **R-Car**

      - **R-Car 3**

        - change stack size of BL31 ([d544dfc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d544dfcc4959d203b06dbfb85fb0ad895178b379))
        - fix SYSTEM_OFF processing for R-Car D3 ([1b49ba0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b49ba0fde5eb9e47fe50152c192579101feb718))
        - fix to bit operation for WUPMSKCA57/53 ([82bb6c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/82bb6c2e88314a5b3f2326c95095c3b20a389947))

  - **Socionext**

    - **Synquacer**

      - initialise CNTFRQ in Non Secure CNTBaseN ([4d4911d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d4911d77d4d59c7dd18d7fc3724ddb1fa3582b7))

  - **ST**

    - add missing header include ([b1391b2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b1391b294ca7803f46bc47048b4a02a15dda9a16))
    - don't try to read boot partition on SD cards ([9492b39](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9492b391a35c66e1e7630e95347259191b28314d))
    - fix NULL pointer dereference issues ([2deff90](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2deff904a953c6a87331ab6830ab80e3889d9e23))
    - manage UART clock and reset only in BL2 ([9e52d45](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9e52d45fdf619561e0a7a833b77aaacc947a4dfd))
    - remove extra chars from dtc version ([03d2077](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/03d20776efc20a04a5191a4f39965079a4d60b3c))

    - **ST32MP1**

      - add missing debug.h ([356ed96](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/356ed961184847dcd088cfcda44b71eeb0ef2377))
      - correct dtc version check ([429f10e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/429f10e3367748abd33b4f6f9ee362c0ba74dd95))
      - correct include order ([ff7675e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff7675ebf94999618dbde14bb59741cefb2b2edd))
      - correct types in messages ([43bbdca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/43bbdca04f5a20bb4e648e18fc63061b6a6e4ecf))
      - deconfigure UART RX pins ([d7176f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d7176f0319cd399aae9a906e5d78e67b32e183f5))
      - do not reopen debug features ([21cfa45](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21cfa4531a76a7c3cad00e874400b97e2f68723c))
      - fix enum prints ([ceab2fc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ceab2fc3442dbda1c4beaff3c4fe708a04c02303))
      - include assert.h to fix build failure ([570c71b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/570c71b20a195ade510f5d584c69325d2634c50b))
      - remove interrupt_provider warning for dtc ([ca88c76](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ca88c761d34854ed3e0b16b9c5f39b0790d320ab))
      - restrict DEVICE2 mapping in BL2 ([db3e0ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/db3e0ece7157181a3529d14172368003eb63dc30))
      - rework switch/case for MISRA ([f7130e8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f7130e81cf9c3682232bb9319b1798184b44920f))
      - set reset pulse duration to 31ms ([9a73a56](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9a73a56c353d32742e03b828647562bdbe2ddbb2))

  - **Xilinx**

    - fix coding style violations ([bb1768c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb1768c67ea06ac466e2cdc7e5338c3d23dac79d))
    - fix mismatching function prototype ([81333ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/81333eac716b25a9fd112cc4f5990e069f3bdb40))

    - **Versal**

      - resolve misra R10.1 in pm services ([775bf1b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/775bf1bbd32c2df47f4ff597eb8a452d2983e590))
      - resolve misra R10.3 ([b2bb3ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b2bb3efb8f590f31b1205c51d56be1dd6f473fbb))
      - resolve misra R10.3 in pm services ([5d1c211](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d1c211e225d40d2926bf34483c90f907a6c5dc3))
      - resolve misra R10.6 ([93d4625](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/93d462562727f4f428e6f975a972226dafbfd305))
      - resolve misra R10.6 in pm services ([fa98d7f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fa98d7f2f8752e37f740b43f533547288552a393))
      - resolve misra R14.4 ([a62c40d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a62c40d42703d5f60a8d80938d2cff721ee131bd))
      - resolve misra R15.6 ([b9fa2d9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b9fa2d9fc154feffe78e677ace54b0e34f011439))
      - resolve misra R15.6 in pm services ([4156719](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4156719550ceddf5b1b4a47464fb32f7506e0dca))
      - resolve misra R15.7 ([bc2637e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc2637e3799dbc9642447ddb719e0262347b1309))
      - resolve misra R16.3 in pm services ([27ae531](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/27ae5310883b0db7d4e2dd4fbc1fd58e675f75b5))
      - resolve misra R17.7 ([526a1fd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/526a1fd1472874561988777f8ecd8b87734a0671))
      - resolve misra R20.7 in pm services ([5dada62](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5dada6227b949ef702bfab7986bc083689afdaf7))
      - resolve misra R7.2 ([0623dce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0623dcea0f6e7a5c9d65413445df8a96a2b40d42))
      - fix coverity scan warnings ([0b15187](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0b15187225a9134e3acbc7693646b21d43617b3b))
      - fix the incorrect log message ([ea04b3f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea04b3fe183b6661f656b4cc38cb93a73d9bc202))

    - **ZynqMP**

      - define and enable ARM_XLAT_TABLES_LIB_V1 ([c884c9a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c884c9a55b167383ff3d96d2d0a30ac6842bcc86))
      - query node status to power up APU ([b35b556](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b35b556718b60b78cb5d96b0c137e2fe82eb0086))
      - resolve misra 7.2 warnings ([5bcbd2d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5bcbd2de127292f3ad076217e08468388c6844b0))
      - resolve misra 8.3 warnings ([944e7ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/944e7ea94f2594e2b128c671cf7415265302596b))
      - resolve misra R10.3 ([2b57da6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2b57da6c91ebe14588e63e5a24f31ef32711eca2))
      - resolve misra R14.4 warnings ([dd1fe71](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dd1fe7178b578916b1e133b7c65c183e1f994371))
      - resolve misra R15.6 warnings ([eb0d2b1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eb0d2b17722c01a22bf3ec1123f7bed2bf891b09))
      - resolve misra R15.7 warnings ([16de22d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/16de22d037644359ef2a04058134f9c326b36633))
      - resolve misra R16.3 warnings ([e7e5d30](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e7e5d30308ccfb931f7b6d0afa6c5c23971e95c0))
      - resolve misra R8.4 warnings ([610eeac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/610eeac89438d603435bde694eb4ddab07f46e45))
      - update the log message to verbose ([1277af9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1277af9bacca36b46d7aa341187bb3abef84332f))
      - use common interface for eemi apis ([a469c1e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a469c1e1f4c1cd69f98ce45d6e0709de091b8cb3))

- **Bootloader Images**

  - **BL1**

    - invalidate SP in data cache during secure SMC ([f1cbbd6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1cbbd6332bb85672dc72cbcc4ac7023323c6936))

  - **BL2**

    - correct messages with image_id ([e4c77db](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e4c77db9c80d87009611a3079454877e6ce45a04))
    - define RAM_NOLOAD for XIP ([cc562e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc562e74101d800b0b0ee3422fb7f4f8321ae2b7))

- **Services**

  - **RME**

    - enable/disable SVE/FPU for Realms ([a4cc85c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4cc85c129d031d9c887cf59b1baeaef18a43010))
    - align RMI and GTSI FIDs with SMCCC ([b9fd2d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b9fd2d3ce3d4e543a2e04dc237cd4e7ff7765c7a))
    - preserve x4-x7 as per SMCCCv1.1 ([1157830](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/11578303fd04a8da36fddb5e6de44f026bf4d24c))

    - **TRP**

      - Distinguish between cold and warm boot ([00e8113](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/00e8113145aa12d89db72068bdd3157f08575d14))

  - **SPM**

    - **EL3 SPMC**

      - fix incorrect FF-A version usage ([25eb2d4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/25eb2d41a6d2ede1e945bbc67ae3f740b92a40bb))
      - fix FF-A memory transaction validation ([3954bc3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3954bc3c03439dbdc7029cf2418c79a037918ce4))

- **Libraries**

  - **CPU Support**

    - workaround for  Cortex-A710 2282622 ([ef934cd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef934cd17c30dcc39cd9022a1c4e9523ec8ba617))
    - workaround for  Cortex-A710 erratum 2267065 ([cfe1a8f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cfe1a8f7123f0dc8376b2075cc6e8e32b13739b2))
    - workaround for Cortex A78 AE erratum 2376748 ([92e8708](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/92e870843e9bd654fd1041d66f284c19ca9c0d4f))
    - workaround for Cortex A78 AE erratum 2395408 ([3f4d81d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3f4d81dfd26649fbcbbbe993a9f0236f5bb07c8a))
    - workaround for Cortex X2 erratum 2002765 ([34ee76d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/34ee76dbdfeee85f123cb903ea95dbee5e9a44a5))
    - workaround for Cortex X2 erratum 2058056 ([e16045d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e16045de50e8b430e6601ba0e1e47097d8310f3d))
    - workaround for Cortex X2 erratum 2083908 ([1db6cd6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1db6cd60279e2d082876692a65cf9c532f506a69))
    - workaround for Cortex-A510 erratum 1922240 ([8343563](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83435637bfafbf1ce642a5fabb52e8d7b2819e36))
    - workaround for Cortex-A510 erratum 2041909 ([e72bbe4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e72bbe47ba7f2a0087654fd99ae24b5b7b444943))
    - workaround for Cortex-A510 erratum 2042739 ([d48088a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d48088acbe400133037ae74acf1b722b059119bb))
    - workaround for Cortex-A510 erratum 2172148 ([c0959d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0959d2c460cbf7c14e7ba2a57d69ecddae80fd8))
    - workaround for Cortex-A510 erratum 2218950 ([cc79018](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc79018b71e45acb524fc5d429d394497ad53646))
    - workaround for Cortex-A510 erratum 2250311 ([7f304b0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f304b02a802b7293d7a8b4f4030c5ff00158404))
    - workaround for Cortex-A510 erratum 2288014 ([d5e2512](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d5e2512c6b86409686f5d1282922ebdf72459fc2))
    - workaround for Cortex-A710 erratum 2008768 ([af220eb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af220ebbe467aa580e6b9ba554676f78ffec930f))
    - workaround for Cortex-A710 erratum 2136059 ([8a855bd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8a855bd24329e081cf13a257c7d2dc3ab4e5dcca))
    - workaround for Cortex-A78 erratum 2376745 ([5d796b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d796b3a25150faff68013880f5a9350cbc53889))
    - workaround for Cortex-A78 erratum 2395406 ([3b577ed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3b577ed53d104cfb324390b7519da5e7744d1001))
    - workaround for Cortex-X2 errata 2017096 ([e7ca443](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e7ca4433fa591233e7e2912b689ab56e531f9775))
    - workaround for Cortex-X2 errata 2081180 ([c060b53](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c060b5337a43cd42f55b99d83096bb44b51b5335))
    - workaround for Cortex-X2 erratum 2147715 ([63446c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/63446c27d11453faacfddecffa44d3880615d412))
    - workaround for Cortex-X2 erratum 2216384 ([4dff759](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4dff7594f94f1e788aef709cc5b3d079693b6242))
    - workaround for DSU-110 erratum 2313941 ([7e3273e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e3273e8e4dca44e7cb88a827b94e662fa8f83e9))
    - workaround for Rainier erratum 1868343 ([a72144f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a72144fb7a30c2782a583a3b0064e741d1fe2c9f))
    - workarounds for cortex-x1 errata ([7b76c20](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7b76c20d8eb4271b381371ce0d510fbe6ad825bf))
    - use CPU_NO_EXTRA3_FUNC for all variants ([b2ed998](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b2ed99894d326993961680fb8e786c267a712400))

  - **EL3 Runtime**

    - set unset pstate bits to default ([7d33ffe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d33ffe4c116506ed63e820d5b6edad81680cd11))

    - **Context Management**

      - add barrier before el3 ns exit ([0482503](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/04825031b2384a08504821f39e98e23bb6f93f11))
      - remove registers accessible only from secure state from EL2 context ([7f41bcc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f41bcc76d8857b4678c90796ebd85794ff3ee5f))
      - refactor the cm_setup_context function ([2bbad1d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2bbad1d126248435e26f9d0d9f5920d8806148d7))
      - remove initialization of EL2 registers when EL2 is used ([fd5da7a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fd5da7a84731e9687f56c263ff3aa8ebed75075a))
      - add cm_prepare_el3_exit_ns function ([8b95e84](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8b95e8487006ff77a7d84fba5bd20ba7e68d8330))
      - refactor initialization of EL1 context registers ([b515f54](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b515f5414b00a8b7ca9b21363886ea976bd19914))

  - **FCONF**

    - correct image_id type in messages ([cec2fb2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cec2fb2b1a8359bf1f349a5b8c8a91a1845f4ca1))

  - **PSCI**

    - correct parent_node type in messages ([b9338ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b9338eee7fbcac7f4b55f27b064572e847810422))

  - **GPT**

    - rework delegating/undelegating sequence ([6a00e9b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a00e9b0c8c37fc446f83ef63e95a75353e31e8b))

  - **Translation Tables**

    - fix bug on VERBOSE trace ([956d76f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/956d76f69d0c96829784c5a6d16aa79e4e0ecab1))

  - **Standard C Library**

    - correct some messages ([a211fde](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a211fde940d4dbd8e95e4f352af2a066a4f89f30))
    - fix snprintf corner cases ([c1f5a09](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c1f5a0925ddf84981d9e176d146bfddb48eb45d1))
    - limit snprintf radix value ([b30dd40](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b30dd4030dcef950eac05393013ee019c3cb3205))
    - snprintf: include stdint.h ([410c925](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/410c925ab31693dc74d654ff9167c8eed3ec5a62))

  - **Locks**

    - add __unused for clang ([5a030ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5a030ce4aed271344087bca723903e10fef59ac9))

- **Drivers**

  - **FWU**

    - rename is_fwu_initialized ([aae7c96](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aae7c96de63914c954f0fc64cd795844832483fc))

  - **I/O**

    - **MTD**

      - correct types in messages ([6e86b46](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e86b462490429fee6db877338a649b0e199b0ec))

  - **Measured Boot**

    - add RMM entry to event_log_metadata ([f4e3e1e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f4e3e1e85f64d8930e89c1396bc9785512f656bd))

  - **MTD**

    - correct types in messages ([6e86b46](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e86b462490429fee6db877338a649b0e199b0ec))

  - **SCMI**

    - add missing \n in ERROR message ([0dc9f52](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0dc9f52a2a9f0b9686c65dd60c84e0bcca552144))
    - make msg_header variable volatile ([99477f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99477f051ef857a1e0600cb98858fc74c007e1ff))
    - use same type for message_id ([2355ebf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2355ebff6f6312086868f44b8ad7f821f6385208))

  - **UFS**

    - delete call to inv_dcache_range for utrd ([c5ee858](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c5ee8588bf9a36075723e5aacceefa93fd2de8c9))
    - disables controller if enabled ([b3f03b2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b3f03b20135fc5fcd5e6ec7e5ca49f1e59b5602e))
    - don't zero out buf before ufs read ([2ef6b8d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2ef6b8d378e7f7c1b1eb7abe176989c3f996f2dc))
    - don't zero out the write buffer ([cd3ea90](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd3ea90b200534b8c9d81619731c9ce198478a3c))
    - fix cache maintenance issues ([38a5ecb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/38a5ecb756e217a80ed951747797ab150449ee9b))
    - move nutrs assignment to ufs_init ([0956319](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0956319b580726029ddc4e00cde6c5a348b99052))
    - read and write attribute based on spec ([a475518](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a475518337e15935469543b1cce353e5b337ef52))

  - **Arm**

    - **GIC**

      - **GICv3**

        - fix iroute value wrong issue ([65bc2d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/65bc2d224b836c230888796c4eda455997dccd8b))

    - **TZC**

      - **TZC-400**

        - correct message with filter ([bdc88d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bdc88d2154448957f452cb472ff95ccec5808ca1))

  - **Marvell**

    - **COMPHY**

      - change reg_set() / reg_set16() to update semantics ([95c26d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/95c26d6489bd8b2fc8b8e14bc2da5d2918055acc))

      - **Armada 3700**

        - drop MODE_REFDIV constant ([9fdecc7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9fdecc72f0fce17ca2cd8e4c3b26c01262166d10))
        - fix comment about COMPHY status register ([4bcfd8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4bcfd8c02e3e3aa27b55dedeed11fb16bac991a9))
        - fix comments about selector register values ([71183ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/71183ef6654c2a485458307a84ce7c473524689a))
        - fix Generation Setting registers names ([e5a2aac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e5a2aac5bbc6dedb20edcc8e7850be2813cb668b))
        - fix PIN_PU_IVREF register name ([c9f138e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c9f138ebfef90d5b7b5651f06efd81bcbc55366b))
        - fix reference clock selection value names ([6ba97f8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6ba97f83dbb314b076588b97415a4078924e1903))
        - fix SerDes frequency register value name ([bdcf44f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bdcf44f1af496e06b693b781fe16bbc2a05fa365))
        - use reg_set() according to update semantics ([4d01bfe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d01bfe66522b13f0d9042206e986551c94fc01e))

    - **Armada**

      - **A3K**

        - **A3720**

          - configure UART after TX FIFO reset ([15546db](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/15546dbf40e5ea81a982a1e6d1e5ba729b06ae51))
          - do external reset during initialization ([0ee80f3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ee80f35a28d651d243a6d56678800f9697d14c0))

  - **NXP**

    - ddr: corrects mapping of HNFs nodes ([e3a2349](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e3a234971abb2402cbf376eca6fcb657a7709fae))

    - **QSPI**

      - fix include path for QSPI driver ([ae95b17](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae95b1782b7a3ab9bbe46ae9ab31f48fb6ebe137))

    - **NXP Crypto**

      - refine code to avoid hang issue for some of toolchain ([fa7fdfa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fa7fdfabf07d91439b0869ffd8e805f0166294bf))

    - **DDR**

      - fix coverity issue ([f713e59](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f713e5954e0906443cd20ae97e229ddbb9ab7005))

  - **ST**

    - **Clock**

      - check _clk_stm32_get_parent return ([b8eab51](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b8eab512bf9d253f96b0333ee0f1bffa1afc3170))
      - correct stm32_clk_parse_fdt_by_name ([7417cda](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7417cda6aeaf6abf48dfbe22dc965b626f61c613))
      - correct types in error messages ([44fb470](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44fb470b7f298645ac31ada4491553824d77d934))
      - initialize pllcfg table ([175758b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/175758b2777eb6df3c4aefd79448e97e76a15272))
      - print enums as unsigned ([9fa9a0c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9fa9a0c55cc830e609415d2cedd2d34fcbec1008))

    - **DDR**

      - add missing debug.h ([15ca2c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/15ca2c5e14abe415e70d08fb595973dd3e3b0af9))
      - correct DDR warnings ([a078134](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a078134e2305ca5695731bc275a5ca892cc38880))

     - **FMC**

      - fix type in message ([afcdc9d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/afcdc9d8d71e2b60071d3d34704f0e598e67a514))

    - **SDMMC2**

      - check regulator enable/disable return ([d50e7a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d50e7a71cb5f8ecfbe2eb69c163d532bab82cbf0))
      - correct cmd_idx type in messages ([bc1c98a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc1c98a8c79b6f72395123ea8ed857a488746d4b))

    - **ST PMIC**

      - add static const to pmic_ops ([57e6018](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/57e6018305a97f4e3627d16d8b1886419f274b4a))
      - correct verbose message ([47065ff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47065ffe44c701b231322ec7160c8624d50a9deb))

    - **SPI**

      - always check SR_TCF flags in stm32_qspi_wait_cmd() ([55de583](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/55de58323e458b38b455439a8846cb663deb5508))
      - remove SR_BUSY bit check before sending command ([5993b91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5993b9157fd049d06194083032771ffcf73da086))

    - **UART**

      - correctly fill BRR register ([af7775a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af7775ab535138ff49643f749110dca143d4122c))

  - **USB**

    - correct type in message ([bd9cd63](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bd9cd63ba096cb16161efa4df40f957421660df1))

- **Miscellaneous**

  - **AArch64**

    - fix encodings for MPAMVPM* registers ([e926558](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e92655849d0a9e5893eb2d7e5f42cf8b931d4db6))

  - **FDTs**

    - **STM32MP1**

      - correct memory mapping for STM32MP13 ([99605fb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99605fb1166794db1dedf1b7280cb184945c229c))
      - remove mmc1 alias if not needed ([a0e9724](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a0e972438b99012da422411c8e504a19bdad44a2))

  - **PIE**

    - align fixup_gdt_reloc() for aarch64 ([5ecde2a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5ecde2a271ac0f3762c16f5a277a70e55e172f0b))
    - do not skip __RW_END__ address during relocation ([4f1a658](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4f1a658f899a169e702b1c7146b59f7c04b0338b))

  - **Security**

    - apply SMCCC_ARCH_WORKAROUND_3 to A73/A75/A72/A57 ([9b2510b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9b2510b69de26cc7f571731b415f6dec82669b6c))
    - loop workaround for CVE-2022-23960 for Cortex-A76 ([a10a5cb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a10a5cb609045de216c01111ec3fcf09a092da0b))
    - report CVE 2022 23960 missing for aarch32 A57 and A72 ([2e5d7a4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e5d7a4b6b26d9d8b6c8e580c33d877e591b1fb3))
    - update Cortex-A15 CPU lib files for CVE-2022-23960 ([187a617](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/187a61761ef5d59bed0c94cca725bd6f116f64d0))
    - workaround for CVE-2022-23960 ([c2a1521](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c2a15217c3053117f4d39233002cb1830fa96670))
    - workaround for CVE-2022-23960 ([1fe4a9d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1fe4a9d181ead0dcb2bc494e90552d3e7f0aaf4c))
    - workaround for CVE-2022-23960 for A76AE, A78AE, A78C ([5f802c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f802c8832f3c5824ca6de17593205ebbf8bf585))
    - workaround for CVE-2022-23960 for Cortex-A57, Cortex-A72 ([be9121f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/be9121fd311ff48c94f3d90fe7efcf84586119e4))
    - workaround for CVE-2022-23960 for Cortex-X1 ([e81e999](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e81e999b9da33ab5d2d3e5185b1ad7c46046329c))

- **Tools**

  - **NXP Tools**

    - fix create_pbl print log ([31af441](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/31af441a0445d4a5e88ddcc371c51b3701c25839))
    - fix tool location path for byte_swape ([a89412a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a89412a649020367a3ed0f87658ee131cd3dcd18))

  - **Firmware Image Package Tool**

    - avoid packing the zero size images in the FIP ([ab556c9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ab556c9c646f1b5f1b500449a5813a4eecdc0302))
    - respect OPENSSL_DIR ([0a956f8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0a956f81805b46b1530f30dd79d16950dc491a7b)

  - **Secure Partition Tool**

    - add leading zeroes in UUID conversion ([b06344a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b06344a3f2c5a0fede3646627f37d1fce3d3d585))
    - update Optee FF-A manifest ([ca0fdbd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ca0fdbd8e0d625ece0f87ca16eacabf13db70921))

  - **Certificate Creation Tool**

    - let distclean Makefile target remove the cert_create tool ([e15591a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e15591aaf47ab45941f0d7a03abf3e4a830ac1d9))

- **Dependencies**

  - **commitlint**

    - change scope-case to lower-case ([804e52e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/804e52e9a770de72913f27b5bc9e7dd965e114c5))

## [2.6.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.5..refs/tags/v2.6) (2021-11-22)

### ⚠ BREAKING CHANGES

- **Architecture**

  - **Activity Monitors Extension (FEAT_AMU)**

    - The public AMU API has been reduced to enablement only
      to facilitate refactoring work. These APIs were not previously used.

      **See:** privatize unused AMU APIs ([b4b726e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b4b726ea868359cf683c07337b69fe91a2a6929a))

    - The `PLAT_AMU_GROUP1_COUNTERS_MASK` platform definition
      has been removed. Platforms should specify per-core AMU counter masks
      via FCONF or a platform-specific mechanism going forward.

      **See:** remove `PLAT_AMU_GROUP1_COUNTERS_MASK` ([6c8dda1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c8dda19e5f484f8544365fd71d965f0afc39244))

- **Libraries**

  - **FCONF**

    - FCONF is no longer added to BL1 and BL2 automatically
      when the FCONF Makefile (`fconf.mk`) is included. When including this
      Makefile, consider whether you need to add `${FCONF_SOURCES}` and
      `${FCONF_DYN_SOURCES}` to `BL1_SOURCES` and `BL2_SOURCES`.

      **See:** clean up source collection ([e04da4c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e04da4c8e132f43218f18ad3b41479ca54bb9263))

- **Drivers**

  - **Arm**

    - **Ethos-N**

      - multi-device support

        **See:** multi-device support ([1c65989](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1c65989e70c9734defc666e824628620b2060b92))

### New Features

- **Architecture**

  - **Activity Monitors Extension (FEAT_AMU)**

    - enable per-core AMU auxiliary counters ([742ca23](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/742ca2307f4e9f82cb2c21518819425e5bcc0f90))

  - **Support for the `HCRX_EL2` register (FEAT_HCX)**

    - add build option to enable FEAT_HCX ([cb4ec47](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cb4ec47b5c73e04472984acf821e6be41b98064f))

  - **Scalable Matrix Extension (FEAT_SME)**

    - enable SME functionality ([dc78e62](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dc78e62d80e64bf4fe5d5bf4844a7bd1696b7c92))

  - **Scalable Vector Extension (FEAT_SVE)**

    - enable SVE for the secure world ([0c5e7d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c5e7d1ce376cabcebebc43dbf238fe4482ab2dc))

  - **System Register Trace Extensions (FEAT_ETMv4, FEAT_ETE and FEAT_ETEv1.1)**

    - enable trace system registers access from lower NS ELs ([d4582d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d4582d30885673987240cf01fd4f5d2e6780e84c))
    - initialize trap settings of trace system registers access ([2031d61](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2031d6166a58623ae59034bc2353fcd2fabe9c30))

  - **Trace Buffer Extension (FEAT_TRBE)**

    - enable access to trace buffer control registers from lower NS EL ([813524e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/813524ea9d2e4138246b8f77a772299e52fb33bc))
    - initialize trap settings of trace buffer control registers access ([40ff907](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40ff90747098ed9d2a09894d1a886c10ca76cee6))

  - **Self-hosted Trace Extension (FEAT_TRF)**

    - enable trace filter control register access from lower NS EL ([8fcd3d9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8fcd3d9600bb2cb6809c6fc68f945ce3ad89633d))
    - initialize trap settings of trace filter control registers access ([5de20ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5de20ece38f782c8459f546a08c6a97b9e0f5bc5))

  - **RME**

    - add context management changes for FEAT_RME ([c5ea4f8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c5ea4f8a6679131010636eb524d2a15b709d0196))
    - add ENABLE_RME build option and support for RMM image ([5b18de0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b18de09e80f87963df9a2e451c47e2321b8643a))
    - add GPT Library ([1839012](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1839012d5b5d431f7ec307230eae9890a5fe7477))
    - add Realm security state definition ([4693ff7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4693ff7225faadc5ad1bcd1c2fb3fbbb8fe1aed0))
    - add register definitions and helper functions for FEAT_RME ([81c272b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/81c272b3b71af38bc5cfb10bbe5722e328a1578e))
    - add RMM dispatcher (RMMD) ([77c2775](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77c2775323a5ff8b77230f05c0cc57f830e9f153))
    - add Test Realm Payload (TRP) ([50a3056](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/50a3056a3cd33d395e8712e1d1e67a8840bf3db1))
    - add xlat table library changes for FEAT_RME ([3621823](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/362182386bafbda9e6671be921fa30cc20610d30))
    - disable Watchdog for Arm platforms if FEAT_RME enabled ([07e96d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07e96d1d2958b6f121476fd391ac67bf8c2c4735))
    - run BL2 in root world when FEAT_RME is enabled ([6c09af9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c09af9f8b36cdfa1dc4d5052f7e4792f63fa88a))

- **Platforms**

  - **Allwinner**

    - add R329 support ([13bacd3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/13bacd3bc3e6b76009adf9183e5396b6457eb12c))

  - **Arm**

    - add FWU support in Arm platforms ([2f1177b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2f1177b2b9ebec3b2fe92607cd771bda1dc9cbfc))
    - add GPT initialization code for Arm platforms ([deb4b3a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/deb4b3a63e3a52f2e9823865a1932f6289ccb7ac))
    - add GPT parser support ([ef1daa4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef1daa420f7b2920b2ee35379de2aefed6ab2605))
    - enable PIE when RESET_TO_SP_MIN=1 ([7285fd5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7285fd5f9aa6d9cc0e0f1dc9c71785b46a88d999))

    - **FPGA**

      - add ITS autodetection ([d7e39c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d7e39c43f2f58aabb085ed7b8f461f9ece6002d0))
      - add kernel trampoline ([de9fdb9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/de9fdb9b5925ae08137d4212a85e9a1d319509c9))
      - determine GICR base by probing ([93b785f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/93b785f5ae66a6418581c304c83a346e8baa5aa3))
      - query PL011 to learn system frequency ([d850169](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d850169c9c233c4bc413d8319196557b54683688))
      - support GICv4 images ([c69f815](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c69f815b09ab85d3ace8fd2979ffafb1184ec76c))
      - write UART baud base clock frequency into DTB ([422b44f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/422b44fb56db7ca8b1a2f9f706733d7d4c2fdeb1))

    - **FVP**

      - enable external SP images in BL2 config ([33993a3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33993a3737737a03ee5a9d386d0a027bdc947c9c))
      - add memory map for FVP platform for FEAT_RME ([c872072](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c8720729726faffc39ec64f3a02440a48c8c305a))
      - add RMM image support for FVP platform ([9d870b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9d870b79c16ef09b0c4a9db18e071c2fa235d1ad))
      - enable trace extension features by default ([cd3f0ae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd3f0ae6f855b2998bc09e5c3a458528c92acb90))
      - pass Event Log addr and size from BL1 to BL2 ([0500f44](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0500f4479eb1d0d5ab9e83dac42b633a5ff677dd))

    - **FVP-R**

      - support for TB-R has been added
      - configure system registers to boot rich OS ([28bbbf3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28bbbf3bf583e0c85004727e694455dfcabd50a4))

    - **RD**

      - **RD-N2**

        - add support for variant 1 of rd-n2 platform ([fe5d5bb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fe5d5bbfe6bd0f386f92bdc419a7e04d885d5b43))
        - add tzc master source ids for soc dma ([3139270](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3139270693ab0fc6d66fed4fe11e183829b47e2e))

    - **SGI**

      - add CPU specific handler for Neoverse N2 ([d932a58](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d932a5831e26620d61d171d0fd8bc2f14938e6f1))
      - add CPU specific handler for Neoverse V1 ([cbee43e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cbee43ebd69377bce1c4fa8d40c6fd67f2be2ee4))
      - increase max BL2 size ([7186a29](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7186a29bbfe3044d5e8001ddfe1d9238578e0944))
      - enable AMU for RD-V1-MC ([e8b119e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8b119e03ad9de5fc440e5929287c94c22fc3946))
      - enable use of PSCI extended state ID format ([7bd64c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7bd64c70e91f73a236b84fb51d5045e308479b5a))
      - introduce platform variant build option ([cfe1506](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cfe1506ee8303d9e0714b3a5b2cd165f76ad5d11))

    - **TC**

      - enable MPMM ([c19a82b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c19a82bef08df58350f1b6668e0604ff8a5bd46d))
      - Enable SVE for both secure and non-secure world ([10198ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10198eab3aa7b0eeba10d9667197816b052ba3e4))
      - populate HW_CONFIG in BL31 ([34a87d7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/34a87d74d9fbbe8037431ea5101110a9f1cf30e1))
      - introduce TC1 platform ([6ec0c65](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6ec0c65b09745fd0f4cee44ee3aa99870303f448))
      - add DRAM2 to TZC non-secure region ([76b4a6b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/76b4a6bb208c22b1c5971964a209ff7d54982348))

      - add bootargs node ([4a840f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4a840f27cd7a05d8e3687aa325adcd019c0d22ee))
      - add cpu capacity to provide scheduling information ([309f593](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/309f5938e610c73cb51b3ba175fed971f49d0888))
      - add Ivy partition ([a19bd32](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a19bd32ed14c33571f3715198d47bac9d0f2808e))
      - add support for trusted services ([ca93248](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ca9324819ee308f9b3a4bb004f02a512c8f301f6))
      - update Matterhorn ELP DVFS clock index ([a2f6294](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a2f6294c98935895d4592ef7e30058ca6e995f4b))
      - update mhuv2 dts node to align with upstream driver ([63067ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/63067ce87e4afa193b2c7f6a4917d1e54b61b000))

    - **Diphda**

      - adding the diphda platform ([bf3ce99](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf3ce9937182e5d8d91e058baabb8213acedacdb))
      - disabling non volatile counters in diphda ([7f70cd2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f70cd29235cc5e96ff6b5f509c7e4260bec5610))
      - enabling stack protector for diphda ([c7e4f1c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c7e4f1cfb84136a7521f26e403a6635ffdce4a2b))

  - **Marvell**

    - introduce t9130_cex7_eval ([d01139f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d01139f3b59a1bc6542e74f52ff3fb26eea23c69))

    - **Armada**

      - **A8K**

        - allow overriding default paths ([0b702af](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0b702afc3aabc349a513a5b00397b58a62fea634))

  - **MediaTek**

    - enable software reset for CIRQ ([b3b162f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b3b162f3b48e087f6656513862a6f9e1fa0757b1))

    - **MT8192**

      - add DFD control in SiP service ([5183e63](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5183e637a0496ad8dfbd8c892bc874ac6a1531bf))

    - **MT8195**

      - add DFD control in SiP service ([3b994a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3b994a75306cc487144dd8e2e15433799e62e6f2))
      - add display port control in SiP service ([7eb4223](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7eb42237575eb3f241c9b22efc5fe91368470aa6))
      - remove adsp event from wakeup source ([c260b32](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c260b3246b6be27c7463d36ce7f76368c94a8540))
      - add DCM driver ([49d3bd8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49d3bd8c4c80ecd19ecfd74812ff1eaa01478cdd))
      - add EMI MPU basic drivers ([75edd34](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/75edd34ade8efaa8a76c5fd59103454023632989))
      - add SPM suspend driver ([859e346](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/859e346b89461f31df17b76ef25ce9e8d2a7279d))
      - add support for PTP3 ([0481896](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/048189637ead887787bd5bc47b1dfab98f321705))
      - add vcore-dvfs support ([d562130](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d562130ea9637b885135a5efe41cb98f2365754f))
      - support MCUSYS off when system suspend ([d336e09](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d336e093dd9ec917ce69484eae8914d98efa328d))

  - **NXP**

    - add build macro for BOOT_MODE validation checking ([cd1280e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd1280ea2e5c8be6f28485a2d5054d06e54e74c1))
    - add CCI and EPU address definition ([6cad59c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6cad59c429b4382ad62aee3a67fa1b3fd4ad38b7))
    - add EESR register definition ([8bfb168](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8bfb16813aff9b3dcbeaa2f77027d44b97f04b6d))
    - add SecMon register definition for ch_3_2 ([66f7884](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/66f7884b5229b1d2977d73d105af1c34cb55f95d))
    - define common macro for ARM registers ([35efe7a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/35efe7a4cea4b3c55b661aac49ef1a85ca8feaa9))
    - define default PSCI features if not defined ([a204785](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a2047853224083328ef67cacbc17a2001ba14701))
    - define default SD buffer ([4225ce8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4225ce8b87635287ecf5cd3baaf31ea703a2640b))

    - **i.MX**

      - **i.MX 8M**

        - add sdei support for i.MX8MN ([ce2be32](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce2be321e8a5865871810b36c580181ea95a1a64))
        - add sdei support for i.MX8MP ([6b63125](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6b63125c415491417e1c389e4015be5ebdee2841))
        - add SiP call for secondary boot ([9ce232f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9ce232fe985a0bb308af459ede8a22629255d4e7))
        - add system_reset2 implementation ([60a0dde](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/60a0dde91bd03f4011c1d52d4d3aea8166e939a0))

        - **i.MX 8M Mini**

          - enlarge BL33 (U-boot) size in FIP ([d53c9db](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d53c9dbf9ff9c435552b62f47fb95bfe86d025e3))

        - **i.MX 8M Plus**

          - add imx8mp_private.h to the build ([91566d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/91566d663b26434813fa674412bb695be1965557))
          - add in BL2 with FIP ([75fbf55](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/75fbf5546b7beca93e4782bc35906f9536392e04))
          - add initial definition to facilitate FIP layout ([f696843](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f696843eab5cf0547b6c6307eaccea25678654c4))
          - enable Trusted Boot ([a16ecd2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a16ecd2cff36b3a8a76d223f4e272e165c941b31))

    - **Layerscape**

      - add ls1028a soc and board support ([52a1e9f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52a1e9ff37251987b71b743951038cd8d1fa0ba4))

      - **LX2**

        - add SUPPORTED_BOOT_MODE definition ([28b3221](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28b3221aebdd48577e2288a75cd2f7547da514e9))

        - **LS1028A**

          - add ls1028a soc support ([9d250f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9d250f03d7a38cac86655495879b2151b877db0d))

          - **LS1028ARDB**

            - add ls1028ardb board support ([34e2112](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/34e2112d1a3a8e4ea33a24bdc6505518266333a9))

  - **QTI**

    - **SC7280**

      - add support for pmk7325 ([b8a0511](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b8a05116ed2a87a9689c4f9be6218a4bce88034a))
      - support for qti sc7280 plat ([46ee50e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/46ee50e0b34e19d383a28bc3b3dadbfb4c07b270))

  - **Renesas**

    - **R-Car**

      - change process for Suspend To RAM ([731aa26](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/731aa26f38d76645b6d50077c28dffb9b02dd08a))

      - **R-Car 3**

        - add a DRAM size setting for M3N ([f95d551](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f95d551217a287bd909aa3c82f4ade4986ad7244))
        - add new board revision for Salvator-XS/H3ULCB ([4379a3e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4379a3e9744cf3b0844446335aca40357a889b9a))
        - add optional support for gzip-compressed BL33 ([ddf2ca0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ddf2ca03979ea9fad305b1bc59beb6e27f0e1c02))
        - add process of SSCG setting for R-Car D3 ([14f0a08](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/14f0a0817297905c03ddf2c4c6040482ef71d744))
        - add process to back up X6 and X7 register's value ([7d58aed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d58aed3b05fa8c677a7c823c1ca5017a462a3d3))
        - add SYSCEXTMASK bit set/clear in scu_power_up ([63a7a34](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/63a7a34706eedba4d13ce6fc661a634801cf8909))
        - apply ERRATA_A53_1530924 and ERRATA_A57_1319537 ([2892fed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2892fedaf27d8bbc68780a4a2c506c768e81b9f1))
        - change the memory map for OP-TEE ([a4d821a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4d821a5a625d941f95ec39fb51ac4fc07c46c5c))
        - emit RPC status to DT fragment if RPC unlocked ([12c75c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/12c75c8886a0ee69d7e279a48cbeb8d1602826b3))
        - keep RWDT enabled ([8991086](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/899108601a0c3b08ead5e686d92ea0794700ff35))
        - modify LifeC register setting for R-Car D3 ([5460f82](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5460f82806752e419fdd6862e8ca9c5fefbee3f2))
        - modify operation register from SYSCISR to SYSCISCR ([d10f876](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d10f87674ecee54cffe1ab554cc05733fd16c7f0))
        - modify SWDT counter setting for R-Car D3 ([053c134](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/053c134683cf74fbf4efad311815b806821f1436))
        - remove access to RMSTPCRn registers in R-Car D3 ([71f2239](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/71f2239f53cd3137ad6abdaf0334dc53f2f21cb1))
        - update DDR setting for R-Car D3 ([042d710](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/042d710d1d917357c5142b340c79978264d3afb1))
        - update IPL and Secure Monitor Rev.3.0.0 ([c5f5bb1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c5f5bb17abfcf6c0eeb3e6c3d70499de0bd6abc0))
        - use PRR cut to determine DRAM size on M3 ([42ffd27](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/42ffd279dd1a686b19e2f1b69d2e35413d5efeba))

  - **ST**

    - add a new DDR firewall management ([4584e01](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4584e01dc643665038004f6c8a4f8bd64e14dacb))
    - add a USB DFU stack ([efbd65f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/efbd65fa7b5cf70f20d6b18152741ccdf8a65bb6))
    - add helper to save boot interface ([7e87ba2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e87ba2598a07facdeb73237dcb350a261ac17b6))
    - add STM32CubeProgrammer support on USB ([afad521](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/afad5214a79259f56bc2003b00859abfe8a18d4d))
    - add STM32MP_EMMC_BOOT option ([214c8a8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/214c8a8d08b2b3c24f12cbc69f497f44851ca524))
    - create new helper for DT access ([ea97bbf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea97bbf6a001b270fd0a25b4b0d0c382e277f3f8))
    - implement platform functions for SMCCC_ARCH_SOC_ID ([3d20178](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3d201787e8246022b1f193283c12e7cb4bfc83ff))
    - improve FIP image loading from MMC ([18b415b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18b415be9d631b3e0c3a3caacc5f02edb9413f6b))
    - manage io_policies with FCONF ([d5a84ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d5a84eeaac2c8ce14d3f2662dc9523b4abf41516))
    - use FCONF to configure platform ([29332bc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/29332bcd680ce7e5f864813d9a900360f5e35d41))
    - use FIP to load images ([1d204ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d204ee4ab12893fceb12097bd4f0a074be253b2))

    - **ST32MP1**

      - add STM32MP_USB_PROGRAMMER target ([fa92fef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fa92fef0a024cdb537fe56c84a0156cc48c1ac2d))
      - add USB DFU support for STM32MP1 ([942f6be](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/942f6be211d4816ad2568d30d807b8fd53d7f981))

  - **Xilinx**

    - **Versal**

      - add support for SLS mitigation ([302b4df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/302b4dfb8fb0041959b8593a098ccae6c61e3238))

    - **ZynqMP**

      - add support for runtime feature config ([578f468](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/578f468ac058bbb60b08f78e2aa2c20cdc601620))
      - sync IOCTL IDs ([38c0b25](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/38c0b2521a0ea0951f4e1ee678ccdbce5fc07a98))
      - add SDEI support ([4143268](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4143268a5ca8f91f1014e0d83edf766946ffff76))
      - add support for XCK26 silicon ([7a30e08](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7a30e08b70e7fbb745554d500182bb6e258c5ab8))
      - extend DT description by TF-A ([0a8143d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0a8143dd636d4234dd2e79d32cb49dc80675c68f))

- **Bootloader Images**

  - import BL_NOBITS_{BASE,END} when defined ([9aedca0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9aedca021d917c7435aa2a0405972aa9d44493a2))

- **Services**

  - **FF-A**

    - adding notifications SMC IDs ([fc3f480](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fc3f480023e3a52460add25f18dd550dde44d9ff))
    - change manifest messaging method ([bb320db](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb320dbc4751f7ea0c37ffba07d14628e58081d0))
    - feature retrieval through FFA_FEATURES call ([96b71eb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/96b71eb9597efbf4857216cac1caeefc9e8bbf3e))
    - update FF-A version to v1.1 ([e1c732d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e1c732d46fa91231b39209621ead1e5a5fb2c497))
    - add Ivy partition to tb fw config ([1bc02c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1bc02c2e0f63b6a7863e10cf6189292d42e693db))
    - add support for FFA_SPM_ID_GET ([70c121a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/70c121a258e43dc2462ed528b44d92594ffb27b3))
    - route secure interrupts to SPMC ([8cb99c3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8cb99c3fc3539bb9926e73a1c33fd72f424fc453))

- **Libraries**

  - **CPU Support**

    - add support for Hayes CPU ([7bd8dfb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7bd8dfb85a8bf5c22d6a39f4538b89cc748090d1))
    - add support for Hunter CPU ([fb9e5f7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fb9e5f7bb76e9764b3ecd7973668c851015fa1b4))
    - add support for Demeter CPU ([f4616ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f4616efafbc1004f1330f515b898e7617e338875))
    - workaround for Cortex A78 AE erratum 1941500 ([47d6f5f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47d6f5ff16d1f2ad009d630a381054b10fa0a06f))
    - workaround for Cortex A78 AE erratum 1951502 ([8913047](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8913047a52e646877812617a2d98cff99494487b))

  - **MPMM**

    - add support for MPMM ([6812078](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68120783d6d6f99c605e9f746ee0e91e2908feb1))

  - **OP-TEE**

    - introduce optee_header_is_valid() ([b84a850](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b84a850864c05fef587fcbb301f955428966de64))

  - **PSCI**

    - require validate_power_state to expose CPU_SUSPEND ([a1d5ac6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a1d5ac6a5aa5d9d18a481de20d272f64a71391f7))

  - **SMCCC**

    - add bit definition for SMCCC_ARCH_SOC_ID ([96b0596](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/96b0596ea25e1f03b862a5bfaa92add6c3e51a33))

- **Drivers**

  - **FWU**

    - add FWU metadata header and build options ([5357f83](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5357f83d4ee89fb831d7e4f6149ae2f652e1b9af))
    - add FWU driver ([0ec3ac6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ec3ac60d86b75d132e7a63fc09ea47e67f90bbd))
    - avoid booting with an alternate boot source ([4b48f7b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4b48f7b56577a78cdc9a2b47280cb62cbae0f7c3))
    - avoid NV counter upgrade in trial run state ([c0bfc88](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0bfc88f8e8e03974834cbcacbbfbd5f202a2857))
    - initialize FWU driver in BL2 ([396b339](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/396b339dc20b97ddd75146e03467a255e28f31b9))
    - introduce FWU platform-specific functions declarations ([efb2ced](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/efb2ced256dacbab71ca11cbc87f70f413ca6729))

  - **I/O**

    - **MTD**

      - offset management for FIP usage ([9a9ea82](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9a9ea82948fd2f1459b6351cb0641f3f77b4e6de))

  - **Measured Boot**

    - add documentation to build and run PoC ([a125c55](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a125c556230501ee0f5ec9f8b0b721625d484a41))
    - move init and teardown functions to platform layer ([47bf3ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47bf3ac31ec84d4b221fdef760c04b5f4416cba4))
    - image hash measurement and recording in BL1 ([48ba034](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/48ba0345f7b42880ec4442d7e90e3e1af95feadd))
    - update tb_fw_config with event log properties ([e742bcd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e742bcdae0d28dc14a2aa0b4ca30f50420bb5ebe))

  - **MMC**

    - boot partition read support ([5014b52](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5014b52dec0c2527ca85c0fbe9c9281a24cc7b10))

  - **MTD**

    - **NAND**

      - count bad blocks before a given offset ([bc3eebb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc3eebb25d5ee340e56047d0e46b81d5af85ff17))

  - **SCMI**

    - add power domain protocol ([7e4833c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e4833cdde8235d228f1f1c40f52b989ad5aa98a))

  - **Arm**

    - **Ethos-N**

      - multi-device support ([1c65989](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1c65989e70c9734defc666e824628620b2060b92))

    - **GIC**

      - **GICv3**

        - detect GICv4 feature at runtime ([858f40e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/858f40e379684fefc8b52c7b9e60576bc3794a69))
        - introduce GIC component identification ([73a643e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/73a643eed9d88910a09ca666bc7ab7f5e532324e))
        - multichip: detect GIC-700 at runtime ([feb7081](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/feb7081863f454b9e465efc074ca669f7a4c783d))

        - **GIC-600AE**

          - introduce support for Fault Management Unit ([2c248ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c248ade2e958eed33127b4ea767fbb7499f31a7))

    - **TZC**

      - **TZC-400**

        - update filters by region ([ce7ef9d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce7ef9d146ce5ca6b9be5ef049377b3817d53d10))

  - **MediaTek**

    - **APU**

      - add mt8192 APU device apc driver ([f46e1f1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f46e1f18539d6d992c82ae605c2cd2a1d0757fa4))
      - add mt8192 APU iommap regions ([2671f31](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2671f3187249d641c55929c812d6691aeeff502a))
      - add mt8192 APU SiP call support ([ca4c0c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ca4c0c2e78eb19d442de4608d9096a755b540a37))
      - setup mt8192 APU_S_S_4 and APU_S_S_5 permission ([77b6801](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77b6801966d203e09ca118fad42543e934d73e6f))

    - **EMI MPU**

      - add MPU support for DSP ([6c4973b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c4973b0a9a75aa83233b696c97d573426eebd98))

  - **NXP**

    - **DCFG**

      - define RSTCR_RESET_REQ ([6c5d140](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c5d140ed99cfec47b239acc242c0f3db1e3bf7c))

    - **FLEXSPI**

      - add MT35XU02G flash info ([a4f5015](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4f5015a0080134251e9272719f5dad1ce2aa842))

  - **Renesas**

    - **R-Car3**

      - add extra offset if booting B-side ([993d809](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/993d809cc115ce23dd2df1df19dc8bb548cc19cd))
      - add function to judge a DDR rank ([726050b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/726050b8e2d2ee2234e103e2df55f9c7f262c851))

  - **ST**

    - manage boot part in io_mmc ([f3d2750](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f3d2750aa2293c0279bc447a85771827ca8b74c1))

    - **USB**

      - add device driver for STM32MP1 ([9a138eb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9a138eb5f29f6747e181a1b3b4199ad57721a3e0))

  - **USB**

    - add a USB device stack ([859bfd8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/859bfd8d42341c6dea2b193db79dc4828e074ad7))

- **Miscellaneous**

  - **Debug**

    - add new macro ERROR_NL() to print just a newline ([fd1360a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fd1360a339e84ccd49f8a2d8a42e4c131a681b3c))

  - **CRC32**

    - **Hardware CRC32**

      - add support for HW computed CRC ([a1cedad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a1cedadf73863ff103fecd64fa188334e1541337))

    - **Software CRC32**

      - add software CRC32 support ([f216937](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f21693704a7bac275e12b44ae30fd210bc317175))

  - **DT Bindings**

    - add STM32MP1 TZC400 bindings ([43de546](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/43de546b909947ab44f104aaee02b98fba70f44c))

  - **FDT Wrappers**

    - add CPU enumeration utility function ([2d9ea36](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d9ea360350303e37a8dd39f3599ac88aaef0ff9))

  - **FDTs**

    - add for_each_compatible_node macro ([ff76614](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff766148b52bfecf09728a83fc3becc7941d943c))
    - introduce wrapper function to read DT UUIDs ([d13dbb6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d13dbb6f1d5e28737a3319af035a6cb991bc6f8f))
    - add firewall regions into STM32MP1 DT ([86b43c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/86b43c58a4105c8cef13d860dd73fa9bd560526a))
    - add IO policies for STM32MP1 ([21e002f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21e002fb777fad9d02a94dc961f077fb444517fa))
    - add STM32MP1 fw-config DT files ([d9e0586](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d9e0586b619b331eb2db75911ca82f927e20bd1c))

    - **STM32MP1**

      - align DT with latest kernel ([e8a953a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8a953a9b85806f7324c8c7245435d5b9226c279))
      - delete nodes for non-used boot devices ([4357db5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4357db5b17ce6ba7357dd99276f34ab497ce60ef))

  - **NXP**

    - **OCRAM**

      - add driver for OCRAM initialization ([10b1e13](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10b1e13bd200849ff134dd8d2fde341a8526f563))

    - **PSCI**

      - define CPUECTLR_TIMER_2TICKS ([3a2cc2e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3a2cc2e262890cffee1fc46835e85be6055189e8))

- **Dependencies**

  - **libfdt**

    - also allow changing base address ([4d585fe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d585fe52feb231d5e73ec50a505122d5e9bf450))

### Resolved Issues

- **Architecture**

- **Platforms**

  - print newline before fatal abort error message ([a5fea81](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a5fea8105887d0dd15edf94aebd591b1b6b5ef05))

  - **Allwinner**

    - delay after enabling CPU power ([86a7429](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/86a7429e477786dad6fab002538aef825f4ca35a))

  - **Arm**

    - correct UUID strings in FVP DT ([748bdd1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/748bdd19aa27c15438d829bdba42fe4062a265a1))
    - fix a VERBOSE trace ([5869ebd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5869ebd0e87f1de987e51994103440fa8c77b26f))
    - remove unused memory node ([be42c4b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/be42c4b4bf3c44f2970b7a1658c46b8d5863cad1))

    - **FPGA**

      - allow build after MAKE_* changes ([9d38a3e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9d38a3e698331e3c8192cc3e0cc8584e6ed987d9))
      - avoid re-linking from executable ELF file ([a67ac76](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a67ac7648cd814ed8f8d4ece1b265c6d48c6dc81))
      - Change PL011 UART IRQ ([195381a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/195381a91313bc0bce2cfa087f3c55136a9e8496))
      - limit BL31 memory usage ([d457230](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d4572303ed45faceffed859955b0e71724fddfd2))
      - reserve BL31 memory ([13e16fe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/13e16fee86451e2f871c2aac757b32299fe5ead6))
      - streamline generated axf file ([9177e4f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9177e4fd9356b0f249be8b6fe14f222e10f1e6cd))
      - enable AMU extension ([d810e30](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d810e30dd6b47e0725dccbcb42ca0a0c5215ee34))
      - increase initrd size ([c3ce73b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c3ce73be0bfe31fa28805fe92b3e727232ffd37a))

    - **FVP**

      - fix fvp_cpu_standby() function ([3202ce8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3202ce8bbb4af8580736d2a1634ad45c3f89d931))
      - spmc optee manifest remove SMC allowlist ([183725b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/183725b39d75e362a32b3c5d0be110c255c56bdd))
      - allow changing the kernel DTB load address ([672d669](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/672d669d6c72f92c6b81464d1d421e392bc1aa3e))
      - bump BL2 stack size ([d22f1d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d22f1d358731f0f55f2f392fa587f0fa8d315aa5))
      - provide boot files via semihosting ([749d0fa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/749d0fa80d1c7ca30b4092a381a06deeeaf1747f))
      - OP-TEE SP manifest per latest SPMC changes ([b7bc51a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b7bc51a7a747bf40d219b2041e5b3ce56737a71b))
      - mock support for CCA NV ctr ([7423e5e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7423e5e893179d37061a67f8eafda24e649a79ea))

    - **FVP-R**

      - fix compilation error in release mode ([7d96e79](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d96e79a1a2efdf85f1ed46cdd5c577b58054f53))

    - **Morello**

      - initialise CNTFRQ in Non Secure CNTBaseN ([7f2d23d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f2d23d9d790df90021de6c5165ef10fe5cc5590))

    - **TC**

      - enable AMU extension ([b5863ca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b5863cab9adb3fed0c1e4dfb92cf906794e7bdb4))
      - change UUID to string format ([1c19536](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1c1953653c20b4a8c61a7deb3fc493d496d8c478))
      - remove "arm,psci" from psci node ([814646b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/814646b4cb792ab14df04e28360fefd168399b3c))
      - remove ffa and optee device tree node ([f1b44a9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1b44a9050fbc12e8c260107bfff2930476df062))
      - set cactus-tertiary vcpu count to 1 ([05f667f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/05f667f0c670ba9682050714561309f00210c282))

    - **SGI**

      - avoid redefinition of 'efi_guid' structure ([f34322c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f34322c1cea1e355aeb4133df6aa601d719be5a3))

  - **Marvell**

    - Check the required libraries before building doimage ([dd47809](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dd47809e9ea75188060bf8b294efa8578d255c63))

    - **Armada**

      - select correct pcie reference clock source ([371648e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/371648e1c76b5230bf8e153629064c02086365c9))
      - fix MSS loader for A8K family ([dceac43](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dceac436f620e60cd0149194377871b225216079))

      - **A3K**

        - disable HANDLE_EA_EL3_FIRST by default ([3017e93](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3017e932768c7357a1a41493c58323419e9a1ec9))
        - enable workaround for erratum 1530924 ([975563d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/975563dbfc012b6e8a7765dd8e48220e1bc53dec))
        - Fix building uart-images.tgz.bin archive ([d3f8db0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d3f8db07b618e79c05805a1598e5e834e42fea98))
        - Fix check for external dependences ([2baf503](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2baf50385ba2b460afef4a7919b13b3a350fd03a))
        - fix printing info messages on output ([9f6d154](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9f6d15408340af07ed3c2500202b147189eaa7ef))
        - update information about PCIe abort hack ([068fe91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/068fe919613197bf221c00fb84a1d94c66a7a8ca))
        - Remove encryption password ([076374c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/076374c9b97d47b10ba5c6034817866c08d66ed4))

      - **A8K**

        - Add missing build dependency for BLE target ([04738e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/04738e69917f8e8790bf4cf83ceb05f85e1f45bb))
        - Correctly set include directories for individual targets ([559ab2d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/559ab2df4a35cd82b2a67a0bebeb3028544a6766))
        - Require that MV_DDR_PATH is correctly set ([528dafc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/528dafc367c4f49d4904c4335422502dacf469bf))
        - fix number of CPU power switches. ([5cf6faf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5cf6fafe223da89c60e2323c242ea188b17e98c3))

  - **MediaTek**

    - **MT8183**

      - fix out-of-bound access ([420c26b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/420c26b33a29c8328a1806ccb2f5a5885041fdfc))

    - **MT8195**

      - use correct print format for uint64_t ([964ee4e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/964ee4e6be70ef638d6c875a761ab5ca359d84fe))
      - fix error setting for SPM ([1f81ccc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1f81cccedd40cb397813b0fa826ea1d793b02089))
      - extend MMU region size ([9ff8b8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9ff8b8ca9393e31e790eb2c8e7ea5c5f41f45198))
      - fix coverity fail ([85e4d14](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/85e4d14df157b5641421ea2b844c146ddc230152))

  - **NXP**

    - **i.MX**

      - do not keep mmc_device_info in stack ([99d37c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99d37c8cb8196a7296311fb4f97f80f086021c74))

      - **i.MX 8M**

        - **i.MX 8M Mini**

          - fix FTBFS on SPD=opteed ([10bfc77](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10bfc77e7b3afce17185114ac66361a0914f7784))

    - **Layerscape**

      - **LX2**

        - **LS1028A**

          - define endianness of scfg and gpio ([2475f63](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2475f63bdec6c24c13f7d6ec7f70275b1bde5c15))
          - fix compile error when enable fuse provision ([a0da9c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a0da9c4bd296ec1a47683a1ee05f5d1ed71828c7))

  - **QEMU**

    - (NS_DRAM0_BASE + NS_DRAM0_SIZE) ADDR overflow 32bit ([325716c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/325716c97b7835b8d249f12c1461556bab8c53a0))
    - reboot/shutdown with low to high gpio ([bd2ad12](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bd2ad12ef10f558a5b15f5768b66e7b2606c6498))

  - **QTI**

    - **SC1780**

      - qti smc addition ([cc35a37](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc35a3771d28a96906f8d0f393ff664924a2d4dc))

  - **Raspberry Pi**

    - **Raspberry Pi 4**

      - drop /memreserve/ region ([5d2793a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d2793a61aded9602af86e90a571f64ff07f93b3))

  - **Renesas**

    - **R-Car**

      - change process that copy code to system ram ([49593cc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49593cc1ce0d0471aeef7ca24a5415da2dd55bea))
      - fix cache maintenance process of reading cert header ([c77ab18](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c77ab18ec7c8e0f3d953177b835e004a9b53515f))
      - fix to load image when option BL2_DCACHE_ENABLE is enabled ([d2ece8d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d2ece8dba2f31091b1fa6c302d4255495bb15705))

      - **R-Car 3**

        - fix disabling MFIS write protection for R-Car D3 ([a8c0c3e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a8c0c3e9d0df2215ed3b9ef66f4596787d957566))
        - fix eMMC boot support for R-Car D3 ([77ab366](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77ab3661e55c39694c7ee81de2d1615775711b64))
        - fix source file to make about GICv2 ([fb3406b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fb3406b6b573cb0b35138ca3c89c5641d3d7b790))
        - fix version judgment for R-Car D3 ([c3d192b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c3d192b8e52823dcbc32e21e47c30693d38bb49f))
        - generate two memory nodes for larger than 2 GiB channel 0 ([21924f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21924f2466b9b5e1243c142932e6f498da5633e9))

  - **Rockchip**

    - **RK3399**

      - correct LPDDR4 resume sequence ([2c4b0c0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c4b0c05c6546e24eb7209ffb3bb465d4feed164))
      - fix dram section placement ([f943b7c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f943b7c8e292e3aad2fcbdd0a37505f62b3b4c87))

  - **Socionext**

    - **Synquacer**

      - update scmi power domain off handling ([f7f5d2c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f7f5d2c4cd209c2d21244da4fa442050eb4531ab))

  - **ST**

    - add STM32IMAGE_SRC ([f223505](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f22350583c2e26ea291eae3dc54db867fdf0d9af))
    - add UART reset in crash console init ([b38e2ed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b38e2ed29ef791dad0cb61fed81b74d612f58b01))
    - apply security at the end of BL2 ([99080bd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99080bd1273331007f0b2d6f64fed51ac6861bcd))
    - correct BSEC error code management ([72c7884](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/72c7884092684af4cc3c49e08f913b3ffed783ba))
    - correct IO compensation disabling ([c2d18ca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c2d18ca80f4bd32f58ba07f53d9bb2586df18fc0))
    - correct signedness comparison issue ([5657dec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5657decc7ffa1376c0a97b6d14ea1428877f5af4))
    - improve DDR get size function ([91ffc1d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/91ffc1deffa2c1c64efe4dfaf27b78f2621a8b0b))
    - only check header major when booting ([8ce8918](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8ce89187459ec77dd9ffdffba3a2b77838d51b6d))
    - panic if boot interface is wrong ([71693a6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/71693a66341e7d9d683ef32981243cb4c4439351))
    - remove double space ([306dcd6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/306dcd6b0d1981b75e103c560a4034bdaa6862d5))

    - **ST32MP1**

      - add bl prefix for internal linker script ([7684ddd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7684dddcfb14c45bad33b091410a0bf14a3a9830))

  - **Xilinx**

    - **Versal**

      - correct IPI buffer offset ([e1e5b13](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e1e5b1339b9f73f7f1893d8a6d4dfe4b19ba0ad1))
      - use sync method for blocking calls ([fa58171](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fa58171534976f94b93a44184afd050d8225e404))

    - **ZynqMP**

      - use sync method for blocking calls ([c063c5a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c063c5a4f92d5787536e595ca4906b458b0f26cb))

- **Services**

  - drop warning on unimplemented calls ([67fad51](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/67fad514ee974dcf0252fa0e9219eb3c580eb714))

  - **RME**

    - fixes a shift by 64 bits bug in the RME GPT library ([322b344](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/322b344e30cb87b9293060d5946b3c17fe3b9133))

  - **SPM**

    - do not compile if SVE/SME is enabled ([4333f95](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4333f95bedb5f2b53dcb62e0e9c563794ec33c07))
    - error macro to use correct print format ([0c23e6f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c23e6f44d41593b6e7f97594c12b5791bd75189))
    - revert workaround hafnium as hypervisor ([3221fce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3221fce842c0b5aea984bb8dbc1393082bd88a58))
    - fixing coverity issue for SPM Core. ([f7fb0bf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f7fb0bf77f3434bfb67411cad65e704fdef27f76))

- **Libraries**

  - **LIBC**

    - use long for 64-bit types on aarch64 ([4ce3e99](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4ce3e99a336b74611349595ea7fd5ed0277c3eeb))

  - **CPU Support**

    - correct Demeter CPU name ([4cb576a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4cb576a0c5bd2e7669606996a9f79602596df07c))
    - workaround for Cortex A78 erratum 2242635 ([1ea9190](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1ea9190c6a4d2299c6dc19adc0bbe93d4f051eff))
    - workaround for Cortex-A710 erratum 2058056 ([744bdbf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/744bdbf732ffd2abf84b2431624051e93bc29f7b))
    - workaround for Neoverse V1 erratum 2216392 ([4c8fe6b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4c8fe6b17fa994a630b2a30f8666df103f2e370d))
    - workaround for Neoverse-N2 erratum 2138953 ([ef8f0c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef8f0c52ddf83e815a029319971682d7a26b6a6f))
    - workaround for Neoverse-N2 erratum 2138958 ([c948185](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c948185c973c13df36c62c4bcb50e22b14d6e06a))
    - workaround for Neoverse-N2 erratum 2242400 ([603806d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/603806d1376c4b18211fb1d4cc338153de026c32))
    - workaround for Neoverse-N2 erratum 2242415 ([5819e23](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5819e23bc47c860872141caf42bddddb1b8679a5))
    - workaround for Neoverse-N2 erratum 2280757 ([0d2d999](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0d2d99924e1be548e75c46cfd536f7503cf863e0))
    - rename Matterhorn, Matterhorn ELP, and Klein CPUs ([c6ac4df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c6ac4df622befb5bb42ac136745094e1498c91d8))

  - **EL3 Runtime**

    - correct CASSERT for pauth ([b4f8d44](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b4f8d44597faf641177134ee08db7c3fcef5aa14))
    - fix SVE and AMU extension enablement flags ([68ac5ed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68ac5ed0493b24e6a0a178171a47db75a31cc423))
    - random typos in tf-a code base ([2e61d68](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e61d6871cc310e9404fe5cfa10b9828f1c869a7))
    - Remove save/restore of EL2 timer registers ([a7cf274](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a7cf2743f3eb487912302aafc748c81bbd1fc603))

  - **OP-TEE**

    - correct signedness comparison ([21d2be8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21d2be83a2eabb328071e857e538ced3c8351874))

  - **GPT**

    - add necessary barriers and remove cache clean ([77612b9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77612b90acaffc82cea712f4a431c727bbb968ec))
    - use correct print format for uint64_t ([2461bd3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2461bd3a89f7f2cdf4a7302536746733970cfe53))

  - **Translation Tables**

    - remove always true check in assert ([74d720a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/74d720a026735263d2f290fd05370dad0d4c7219))

- **Drivers**

  - **Authentication**

    - avoid NV counter upgrade without certificate validation ([a2a5a94](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a2a5a9456969266dc68d5845f31e05be0c3ff2e3))

    - **CryptoCell-713**

      - fix a build failure with CC-713 library ([e5fbee5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e5fbee5085c682ac3438e6f66c8bdaffb6076fa2))

  - **MTD**

    - fix MISRA issues and logic improvement ([5130ad1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5130ad14d52a0196422fed8a7d08e25659890b15))
    - macronix quad enable bit issue ([c332740](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c3327408eb4b5852c0ed9d8933c35aaa6de34c21))

    - **NAND**

      - **SPI NAND**

        - check correct manufacturer id ([4490b79](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4490b7963303fbe59b07a66c8498a803eb5c239c))
        - check that parameters have been set ([bc453ab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc453ab1b2fd4267d34f2b9587f73b8940ee1538))

  - **SCMI**

    - entry: add weak functions ([b3c8fd5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b3c8fd5d778144340d289ad4825123106aac4a96))
    - smt: fix build for aarch64 ([0e223c6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0e223c6a9e5a2d92cae00fdd16a02a3f8971b114))
    - mention "SCMI" in driver initialisation message ([e0baae7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e0baae7316bfdf3e49e5e158f79eb80cd51fc700))
    - relax requirement for exact protocol version ([125868c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/125868c94150f52ff85cdb59aee623ab1f9f259d))

  - **UFS**

    - add reset before DME_LINKSTARTUP ([905635d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/905635d5e74e3c7b7b2412a673009c8aaabb73e1))

  - **Arm**

    - **GIC**

      - **GICv3**

        - add dsb in both disable and enable function of gicv3_cpuif ([5a5e0aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5a5e0aac398989536dc4be790820af89da3d093a))

      - **GIC-600AE**

	- fix timeout calculation ([7f322f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f322f228e76caa5480f827af0aa6751f00fc1c4))

    - **TZC**

      - **TZC-400**

        - never disable filter 0 ([ef378d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef378d3ec1ef9d7c28baef32ed409688e962542b))

  - **Marvell**

    - **COMPHY**

      - fix name of 3.125G SerDes mode ([a669983](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a669983c78828e3f4a4f14b9e5a6ee79dcfde20f))

      - **Armada 3700**

        - configure phy selector also for PCIe ([0f3a122](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0f3a1221093256999af5f2a80e9b3d7231b9f5fb))
        - fix address overflow ([c074f70](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c074f70ce5d85e1735b589b323fac99d7eb988b5))
        - handle failures in power functions ([49b664e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49b664e75f43fda08dddef4f0510d346bdd25565))

      - **CP110**

        - fix error code in pcie power on ([c0a909c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0a909cdcce2d9a2ceefe672ad2fc1cae7e39ec4))

    - **Armada**

      - **A3K**

        - **A3720**

          - fix configuring UART clock ([b9185c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b9185c75f7ec2b600ebe0d49281e216a2456b764))
          - fix UART clock rate value and divisor calculation ([66a7752](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/66a7752834382595d26214783ae4698fd1f00bd6))
          - fix UART parent clock rate determination ([5a91c43](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5a91c439cbeb1f64b8b9830de91efad5113d3c89))

  - **MediaTek**

    - **PMIC Wrapper**

      - update idle flow ([9ed4e6f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9ed4e6fb669b8fcafc4e8acfa6a36db305d27ac8))

    - **MT8192**

      - **SPM**

        - add missing bit define for debug purpose ([310c3a2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/310c3a26e17d99aafc73b3504d0b6dfbdb97fd4c))

  - **NXP**

    - **FLEXSPI**

      - fix warm boot wait time for MT35XU512A ([1ff7e46](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1ff7e46b092b74891bc2dc7263e4dfae947b2223))

    - **SCFG**

      - fix endianness checking ([fb90cfd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fb90cfd4eee504f1d16aa143728af427dc6e0ed8))

    - **SFP**

      - fix compile warning ([3239a17](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3239a17561c124df7095391c0d64e86910660cdc))

  - **Renesas**

    - **R-Car3**

      - console: fix a return value of console_rcar_init ([bb273e3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb273e3be1c4f1cddeac9ceaac95fb56e41e6b98))
      - ddr: update DDR setting for H3, M3, M3N ([ec767c1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ec767c1b99675fbb50ef1b2fdb2d38e881e4789d))
      - emmc: remove CPG_CPGWPR redefinition ([36d5645](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/36d5645aec947ab00b925b21141e59e58e1efd8c))
      - fix CPG registers redefinition ([0dae56b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0dae56bb2f0aa1f89ec98ebe3931fb19751a5c72))
      - i2c_dvfs: fix I2C operation ([b757d3a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b757d3a1d901bee9b7ad430702575adba04889ba))

  - **ST**

    - **Clock**

      - use correct return value ([8f97c4f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8f97c4fab1769b3f7f37a2a7a01ade36e5c94eaa))
      - correctly manage RTC clock source ([1550909](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/15509093f0ba9a10f97c6f92bc3bb9fcf79a48ce))
      - fix MCU/AXI parent clock ([b8fe48b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b8fe48b6f2b07fce49363cb3c0f8dac9e286439b))
      - fix MPU clock rate ([602ae2f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/602ae2f23c2bc9d79a9ab2b7c5dde1932fffc984))
      - fix RTC clock rating ([cbd2e8a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cbd2e8a6afdd05c4b404d7998134a3f60cc15518))
      - keep RTC clock always on ([5b111c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b111c74795ea5e9c8a12d0e6b18d77e431311ed))
      - keep RTCAPB clock always on ([373f06b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/373f06be4ee1114369b96763481b58885623aea4))
      - set other clocks as always on ([bf39318](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf39318d93c270ff72bda4b46e4771aba7aea313))

    - **I/O**

      - **STM32 Image**

        - invalidate cache on local buf ([a5bcf82](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a5bcf82402ff415326b4dba42aae95c499821e94))
        - uninitialized variable warning ([c1d732d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c1d732d0db2463998036c678619007da79a25b3f))

    - **ST PMIC**

      - initialize i2c_state ([4282284](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/42822844bfed2e9ffaeae850cc60f5c3d4d9d654))
      - missing error check ([a4bcfe9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4bcfe94e73db89ce2ebbb23c8e33e51eea5026a))

    - **STPMIC1**

      - fix power switches activation ([0161991](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0161991184e5feacacc679bdb9c92681b85235eb))
      - update error cases return ([ed6a852](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ed6a85234653c5ee2520389b769ff47e321df8a4))

    - **UART**

      - **STM32 Console**

        - do not skip init for crash console ([49c7f0c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49c7f0cef4cc864185828750f1f61f3f33f284f7))

  - **USB**

    - add a optional ops get_other_speed_config_desc ([216c122](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/216c1223c2c65bd1c119a28b9406f70a9ee7b063))
    - fix Null pointer dereferences in usb_core_set_config ([0cb9870](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0cb9870ddfa1b2fec50debe6d6333cbcb3df1e7e))
    - remove deadcode when USBD_EP_NB = 1 ([7ca4928](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ca49284be083b03ae11aa348b40358876ee5d4b))
    - remove unnecessary cast ([025f5ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/025f5ef201a39ba7285f368139e690bbd7a44653))

- **Miscellaneous**

  - use correct printf format for uint64_t ([4ef449c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4ef449c15a4055d92632cb7e72267f525a7e2fca))

  - **DT Bindings**

    - fix static checks ([0861fcd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0861fcdd3e3f2625e133de3dae9c548de7c1ee48))

  - **FDTs**

    - avoid output on missing DT property ([49e789e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49e789e353efaf97f84eca016c6a1b8a2b3e3d98))
    - fix OOB write in uuid parsing function ([d0d6424](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d0d642450f1f3a0f43e0e156ef57a0c460dd48cf))

    - **Morello**

      - fix scmi clock specifier to cluster mappings ([387a906](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/387a9065a271ecde0e47dc5a9f9d037637502beb))

    - **STM32MP1**

      - correct copyright dates ([8d26029](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8d26029168fe70a86de524ed68c56e8666823714))
      - set ETH clock on PLL4P on ST boards ([3e881a8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3e881a8834a955f1e552300bdbf1dafd02ea8f1c))
      - update PLL nodes for ED1/EV1 boards ([cdbbb9f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cdbbb9f7ecd4687fa52e1c655b631377c24862b9))
      - use 'kHz' as kilohertz abbreviation ([4955d08](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4955d08de7aa664387d2e5f690e78b85ac23a402))

  - **PIE**

    - invalidate data cache in the entire image range if PIE is enabled ([596d20d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/596d20d9e4d50c02b5a0cce8cad2a1c205cd687a))

  - **Security**

    - Set MDCR_EL3.MCCD bit ([12f6c06](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/12f6c0649732a35a7ed45ba350a963f09a5710ca))

  - **SDEI**

    - fix assert while kdump issue ([d39db26](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d39db2695ba626b9c0ee38652fe160b4e84b15d9))
    - print event number in hex format ([6b94356](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6b94356b577744d425476a029c47bd35eb13c148))
    - set SPSR for SDEI based on TakeException ([37596fc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/37596fcb43e34ed4bcf1bd3e86d8dec1011edab8))

- **Documentation**

  - fix TF-A v2.6 release date in the release information page ([c90fa47](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c90fa47202b762fe8f54e9c0561e94d37907b6ad))
  - fix `FF-A` substitution ([a61940c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a61940ca739eb89be7c1bb2408a9178c2da5cb70))
  - fix typos in v2.5 release documentation ([481c7b6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/481c7b6b9107a3f71ee750f89cacdd8f9c729838))
  - remove "experimental" tag for stable features ([700e768](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/700e7685dd4682a929645a79de39f503c9140b2d))

  - **Contribution Guidelines**

    - fix formatting for code snippet ([d0bbe81](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d0bbe8150eb35fe2bac1567751bf84a8f073dd39))

- **Build System**

  - use space in WARNINGS list ([34b508b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/34b508be9f021831423a8a14f56dff547e24c743))

  - **Git Hooks**

    - downgrade `package-lock.json` version ([7434b65](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7434b65208175bdf3f44e0e62aaaeabc9c494ee3))

- **Tools**

  - **STM32 Image**

    - improve the tool ([8d0036d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8d0036d3d8c8ac1524539ea90382acafb1e524c0))

  - **SPTOOL**

    - SP UUID little to big endian in TF-A build ([dcdbcdd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dcdbcddebdee8d4d2c6c8316f615b428758b22ac))

  - **DOIMAGE**

    - Fix doimage syntax breaking secure mode build ([6d55ef1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d55ef1a24dc92a3b737aaa02141f550caaace06))

- **Dependencies**

  - **checkpatch**

    - do not check merge commits ([77a0a7f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77a0a7f1d96b188849d1d8d8884b3c93857d3f69))

## [2.5.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.4..refs/tags/v2.5) (2021-05-17)

### New Features

- Architecture support

  - Added support for speculation barrier(`FEAT_SB`) for non-Armv8.5 platforms
    starting from Armv8.0
  - Added support for Activity Monitors Extension version 1.1(`FEAT_AMUv1p1`)
  - Added helper functions for Random number generator(`FEAT_RNG`) registers
  - Added support for Armv8.6 Multi-threaded PMU extensions (`FEAT_MTPMU`)
  - Added support for MTE Asymmetric Fault Handling extensions(`FEAT_MTE3`)
  - Added support for Privileged Access Never extensions(`FEAT_PANx`)

- Bootloader images

  - Added PIE support for AArch32 builds
  - Enable Trusted Random Number Generator service for BL32(sp_min)

- Build System

  - Added build option for Arm Feature Modifiers

- Drivers

  - Added support for interrupts in TZC-400 driver
  - Broadcom
    - Added support for I2C, MDIO and USB drivers
  - Marvell
    - Added support for secure read/write of dfc register-set
    - Added support for thermal sensor driver
    - Implement a3700_core_getc API in console driver
    - Added rx training on 10G port
  - Marvell Mochi
    - Added support for cn913x in PCIe mode
  - Marvell Armada A8K
    - Added support for TRNG-IP-76 driver and accessing RNG register
  - Mediatek MT8192
    - Added support for following drivers
      - MPU configuration for SCP/PCIe
      - SPM suspend
      - Vcore DVFS
      - LPM
      - PTP3
      - UART save and restore
      - Power-off
      - PMIC
      - CPU hotplug and MCDI support
      - SPMC
      - MPU
  - Mediatek MT8195
    - Added support for following drivers
      - GPIO, NCDI, SPMC drivers
      - Power-off
      - CPU hotplug, reboot and MCDI
      - Delay timer and sys timer
      - GIC
  - NXP
    - Added support for
      - non-volatile storage API
      - chain of trust and trusted board boot using two modes: MBEDTLS and CSF
      - fip-handler necessary for DDR initialization
      - SMMU and console drivers
      - crypto hardware accelerator driver
      - following drivers: SD, EMMC, QSPI, FLEXSPI, GPIO, GIC, CSU, PMU, DDR
      - NXP Security Monitor and SFP driver
      - interconnect config APIs using ARM CCN-CCI driver
      - TZC APIs to configure DDR region
      - generic timer driver
      - Device configuration driver
  - IMX
    - Added support for image loading and io-storage driver for TBBR fip booting
  - Renesas
    - Added support for PFC and EMMC driver
    - RZ Family:
      - G2N, G2E and G2H SoCs
        - Added support for watchdog, QoS, PFC and DRAM initialization
    - RZG Family:
      - G2M
        - Added support for QoS and DRAM initialization
  - Xilinx
    - Added JTAG DCC support for Versal and ZynqMP SoC family.

- Libraries

  - C standard library
    - Added support to print `%` in `snprintf()` and `printf()` APIs
    - Added support for strtoull, strtoll, strtoul, strtol APIs from FreeBSD
      project
  - CPU support
    - Added support for
      - Cortex_A78C CPU
      - Makalu ELP CPU
      - Makalu CPU
      - Matterhorn ELP CPU
      - Neoverse-N2 CPU
  - CPU Errata
    - Arm Cortex-A76: Added workaround for erratum 1946160
    - Arm Cortex-A77: Added workaround for erratum 1946167
    - Arm Cortex-A78: Added workaround for erratum 1941498 and 1951500
    - Arm Neoverse-N1: Added workaround for erratum 1946160
  - Flattened device tree(libfdt)
    - Added support for wrapper function to read UUIDs in string format from dtb

- Platforms

  - Added support for MediaTek MT8195
  - Added support for Arm RD-N2 board
  - Allwinner
    - Added support for H616 SoC
  - Arm
    - Added support for GPT parser
    - Protect GICR frames for fused/unused cores
  - Arm Morello
    - Added VirtIO network device to Morello FVP fdts
  - Arm RD-N2
    - Added support for variant 1 of RD-N2 platform
    - Enable AMU support
  - Arm RD-V1
    - Enable AMU support
  - Arm SGI
    - Added support for platform variant build option
  - Arm TC0
    - Added Matterhorn ELP CPU support
    - Added support for opteed
  - Arm Juno
    - Added support to use hw_config in BL31
    - Use TRNG entropy source for SMCCC TRNG interface
    - Condition Juno entropy source with CRC instructions
  - Marvell Mochi
    - Added support for detection of secure mode
  - Marvell ARMADA
    - Added support for new compile option A3720_DB_PM_WAKEUP_SRC
    - Added support doing system reset via CM3 secure coprocessor
    - Made several makefile enhancements required to build WTMI_MULTI_IMG and
      TIMDDRTOOL
    - Added support for building DOIMAGETOOL tool
    - Added new target mrvl_bootimage
  - Mediatek MT8192
    - Added support for rtc power off sequence
  - Mediatek MT8195
    - Added support for SiP service
  - STM32MP1
    - Added support for
      - Seeed ODYSSEY SoM and board
      - SDMMC2 and I2C2 pins in pinctrl
      - I2C2 peripheral in DTS
      - PIE for BL32
      - TZC-400 interrupt managament
      - Linux Automation MC-1 board
  - Renesas RZG
    - Added support for identifying EK874 RZ/G2E board
    - Added support for identifying HopeRun HiHope RZ/G2H and RZ/G2H boards
  - Rockchip
    - Added support for stack protector
  - QEMU
    - Added support for `max` CPU
    - Added Cortex-A72 support to `virt` platform
    - Enabled trigger reboot from secure pl061
  - QEMU SBSA
    - Added support for sbsa-ref Embedded Controller
  - NXP
    - Added support for warm reset to retain ddr content
    - Added support for image loader necessary for loading fip image
    - lx2160a SoC Family
      - Added support for
        - new platform lx2160a-aqds
        - new platform lx2160a-rdb
        - new platform lx2162a-aqds
        - errata handling
  - IMX imx8mm
    - Added support for trusted board boot
  - TI K3
    - Added support for lite device board
    - Enabled Cortex-A72 erratum 1319367
    - Enabled Cortex-A53 erratum 1530924
  - Xilinx ZynqMP
    - Added support for PS and system reset on WDT restart
    - Added support for error management
    - Enable support for log messages necessary for debug
    - Added support for PM API SMC call for efuse and register access

- Processes

  - Introduced process for platform deprecation
  - Added documentation for TF-A threat model
  - Provided a copy of the MIT license to comply with the license requirements
    of the arm-gic.h source file (originating from the Linux kernel project and
    re-distributed in TF-A).

- Services

  - Added support for TRNG firmware interface service
  - Arm
    - Added SiP service to configure Ethos-N NPU
  - SPMC
    - Added documentation for SPM(Hafnium) SMMUv3 driver
  - SPMD
    - Added support for
      - FFA_INTERRUPT forwading ABI
      - FFA_SECONDARY_EP_REGISTER ABI
      - FF-A v1.0 boot time power management, SPMC secondary core boot and early
        run-time power management

- Tools

  - FIPTool
    - Added mechanism to allow platform specific image UUID
  - git hooks
    - Added support for conventional commits through commitlint hook, commitizen
      hook and husky configuration files.
  - NXP tool
    - Added support for a tool that creates pbl file from BL2
  - Renesas RZ/G2
    - Added tool support for creating bootparam and cert_header images
  - CertCreate
    - Added support for platform-defined certificates, keys, and extensions
      using the platform's makefile
  - shared tools
    - Added EFI_GUID representation to uuid helper data structure

### Changed

- Common components

  - Print newline after hex address in aarch64 el3_panic function
  - Use proper `#address-cells` and `#size-cells` for reserved-memory in dtbs

- Drivers

  - Move SCMI driver from ST platform directory and make it common to all
    platforms
  - Arm GICv3
    - Shift eSPI register offset in GICD_OFFSET_64()
    - Use mpidr to probe GICR for current CPU
  - Arm TZC-400
    - Adjust filter tag if it set to FILTER_BIT_ALL
  - Cadence
    - Enhance UART driver APIs to put characters to fifo
  - Mediatek MT8192
    - Move timer driver to common folder
    - Enhanced sys_cirq driver to add more IC services
  - Renesas
    - Move ddr and delay driver to common directory
  - Renesas rcar
    - Treat log as device memory in console driver
  - Renesas RZ Family:
    - G2N and G2H SoCs
      - Select MMC_CH1 for eMMC channel
  - Marvell
    - Added support for checking if TRNG unit is present
  - Marvell A3K
    - Set TXDCLK_2X_SEL bit during PCIe initialization
    - Set mask parameter for every reg_set call
  - Marvell Mochi
    - Added missing stream IDs configurations
  - MbedTLS
    - Migrated to Mbed TLS v2.26.0
  - IMX imx8mp
    - Change the bl31 physical load address
  - QEMU SBSA
    - Enable secure variable storage
  - SCMI
    - Update power domain protocol version to 2.0
  - STM32
    - Remove dead code from nand FMC driver

- Libraries

  - C Standard Library
    - Use macros to reduce duplicated code between snprintf and printf
  - CPU support
    - Sanity check pointers before use in AArch32 builds
    - Arm Cortex-A78
      - Remove rainier cpu workaround for errata 1542319
    - Arm Makalu ELP
      - Added "\_arm" suffix to Makalu ELP CPU lib

- Miscellaneous

  - Editorconfig
    - set max line length to 100

- Platforms

  - Allwinner
    - Added reserved-memory node to DT
    - Express memmap more dynamically
    - Move SEPARATE_NOBITS_REGION to platforms
    - Limit FDT checks to reduce code size
    - Use CPUIDLE hardware when available
    - Allow conditional compilation of SCPI and native PSCI ops
    - Always use a 3MHz RSB bus clock
    - Enable workaround for Cortex-A53 erratum 1530924
    - Fixed non-default PRELOADED_BL33_BASE
    - Leave CPU power alone during BL31 setup
    - Added several psci hooks enhancements to improve system shutdown/reset
      sequence
    - Return the PMIC to I2C mode after use
    - Separate code to power off self and other CPUs
    - Split native and SCPI-based PSCI implementations
  - Allwinner H6
    - Added R_PRCM security setup for H6 board
    - Added SPC security setup for H6 board
    - Use RSB for the PMIC connection on H6
  - Arm
    - Store UUID as a string, rather than ints
    - Replace FIP base and size macro with a generic name
    - Move compile time switch from source to dt file
    - Don't provide NT_FW_CONFIG when booting hafnium
    - Do not setup 'disabled' regulator
    - Increase SP max size
    - Remove false dependency of ARM_LINUX_KERNEL_AS_BL33 on RESET_TO_BL31 and
      allow it to be enabled independently
  - Arm FVP
    - Do not map GIC region in BL1 and BL2
  - Arm Juno
    - Refactor juno_getentropy() to return 64 bits on each call
  - Arm Morello
    - Remove "virtio-rng" from Morello FVP
    - Enable virtIO P9 device for Morello fvp
  - Arm RDV1
    - Allow all PSCI callbacks on RD-V1
    - Rename rddaniel to rdv1
  - Arm RDV1MC
    - Rename rddanielxlr to rdv1mc
    - Initialize TZC-400 controllers
  - Arm TC0
    - Updated GICR base address
    - Use scmi_dvfs clock index 1 for cores 4-7 through fdt
    - Added reserved-memory node for OP-TEE fdts
    - Enabled Theodul DSU in TC platform
    - OP-TEE as S-EL1 SP with SPMC at S-EL2
    - Update Matterhorm ELP DVFS clock index
  - Arm SGI
    - Allow access to TZC controller on all chips
    - Define memory regions for multi-chip platforms
    - Allow access to nor2 flash and system registers from S-EL0
    - Define default list of memory regions for DMC-620 TZC
    - Improve macros defining cper buffer memory region
    - Refactor DMC-620 error handling SMC function id
    - Refactor SDEI specific macros
    - Added platform id value for RDN2 platform
    - Refactored header file inclusions and inclusion of memory mapping
  - Arm RDN2
    - Allow usage of secure partitions on RDN2 platform
    - Update GIC redistributor and TZC base address
  - Arm SGM775
    - Deprecate Arm sgm775 FVP platform
  - Marvell
    - Increase TX FIFO EMPTY timeout from 2ms to 3ms
    - Update delay code to be compatible with 1200 MHz CPU
  - Marvell ARMADA
    - Postpone MSS CPU startup to BL31 stage
    - Allow builds without MSS support
    - Use MSS SRAM in secure mode
    - Added missing FORCE, .PHONY and clean targets
    - Cleanup MSS SRAM if used for copy
    - Move definition of mrvl_flash target to common marvell_common.mk file
    - Show informative build messages and blank lines
  - Marvell ARMADA A3K
    - Added a new target mrvl_uart which builds UART image
    - Added checks that WTP, MV_DDR_PATH and CRYPTOPP_PATH are correctly defined
    - Allow use of the system Crypto++ library
    - Build \$(WTMI_ENC_IMG) in \$(BUILD_PLAT) directory
    - Build intermediate files in \$(BUILD_PLAT) directory
    - Build UART image files directly in \$(BUILD_UART) subdirectory
    - Correctly set DDR_TOPOLOGY and CLOCKSPRESET for WTMI
    - Do not use 'echo -e' in Makefile
    - Improve 4GB DRAM usage from 3.375 GB to 3.75 GB
    - Remove unused variable WTMI_SYSINIT_IMG from Makefile
    - Simplify check if WTP variable is defined
    - Split building \$(WTMI_MULTI_IMG) and \$(TIMDDRTOOL)
  - Marvell ARMADA A8K
    - Allow CP1/CP2 mapping at BLE stage
  - Mediatek MT8183
    - Added timer V20 compensation
  - Nvidia Tegra
    - Rename SMC API
  - TI K3
    - Make plat_get_syscnt_freq2 helper check CNT_FID0 register
    - Fill non-message data fields in sec_proxy with 0x0
    - Update ti_sci_msg_req_reboot ABI to include domain
    - Enable USE_COHERENT_MEM only for the generic board
    - Explicitly map SEC_SRAM_BASE to 0x0
    - Use BL31_SIZE instead of computing
    - Define the correct number of max table entries and increase SRAM size to
      account for additional table
  - Raspberry Pi4
    - Switch to gicv2.mk and GICV2_SOURCES
  - Renesas
    - Move headers and assembly files to common folder
  - Renesas rzg
    - Added device tree memory node enhancements
  - Rockchip
    - Switch to using common gicv3.mk
  - STM32MP1
    - Set BL sizes regardless of flags
  - QEMU
    - Include gicv2.mk for compiling GICv2 source files
    - Change DEVICE2 definition for MMU
    - Added helper to calculate the position shift from MPIDR
  - QEMU SBSA
    - Include libraries for Cortex-A72
    - Increase SHARED_RAM_SIZE
    - Addes support in spm_mm for upto 512 cores
    - Added support for topology handling
  - QTI
    - Mandate SMC implementation
  - Xilinx
    - Rename the IPI CRC checksum macro
    - Use fno-jump-tables flag in CPPFLAGS
  - Xilinx versal
    - Added the IPI CRC checksum macro support
    - Mark IPI calls secure/non-secure
    - Enable sgi to communicate with linux using IPI
    - Remove Cortex-A53 compilation
  - Xilinx ZynqMP
    - Configure counter frequency during initialization
    - Filter errors related to clock gate permissions
    - Implement pinctrl request/release EEMI API
    - Reimplement pinctrl get/set config parameter EEMI API calls
    - Reimplement pinctrl set/get function EEMI API
    - Update error codes to match Linux and PMU Firmware
    - Update PM version and support PM version check
    - Update return type in query functions
    - Added missing ids for 43/46/47dr devices
    - Checked for DLL status before doing reset
    - Disable ITAPDLYENA bit for zero ITAP delay
    - Include GICv2 makefile
    - Remove the custom crash implementation

- Services

  - SPMD
    - Lock the g_spmd_pm structure
    - Declare third cactus instance as UP SP
    - Provide number of vCPUs and VM size for first SP
    - Remove `chosen` node from SPMC manifests
    - Move OP-TEE SP manifest DTS to FVP platform
    - Update OP-TEE SP manifest with device-regions node
    - Remove device-memory node from SPMC manifests
  - SPM_MM
    - Use sp_boot_info to set SP context
  - SDEI
    - Updata the affinity of shared event

- Tools

  - FIPtool
    - Do not print duplicate verbose lines about building fiptool
  - CertCreate
    - Updated tool for platform defined certs, keys & extensions
    - Create only requested certificates
    - Avoid duplicates in extension stack

### Resolved Issues

- Several fixes for typos and mis-spellings in documentation

- Build system

  - Fixed \$\{FIP_NAME} to be rebuilt only when needed in Makefile
  - Do not mark file targets as .PHONY target in Makefile

- Drivers

  - Authorization
    - Avoid NV counter upgrade without certificate validation
  - Arm GICv3
    - Fixed logical issue for num_eints
    - Limit SPI ID to avoid misjudgement in GICD_OFFSET()
    - Fixed potential GICD context override with ESPI enabled
  - Marvell A3700
    - Fixed configuring polarity invert bits
  - Arm TZC-400
    - Correct FAIL_CONTROL Privileged bit
    - Fixed logical error in FILTER_BIT definitions
  - Renesas rcar
    - Fixed several coding style violations reported by checkpatch

- Libraries

  - Arch helpers
    - Fixed assertions in processing dynamic relocations for AArch64 builds
  - C standard library
    - Fixed MISRA issues in memset() ABI
  - RAS
    - Fixed bug of binary search in RAS interrupt handler

- Platforms

  - Arm
    - Fixed missing copyrights in Arm-gic.h file
    - Fixed the order of header files in several dts files
    - Fixed error message printing in board makefile
    - Fixed bug of overriding the last node in image load helper API
    - Fixed stdout-path in fdts files of TC0 and N1SDP platforms
    - Turn ON/OFF redistributor in sync with GIC CPU interface ON/OFF for css
      platforms
  - Arm FVP
    - Fixed Generic Timer interrupt types in platform dts files
  - Arm Juno
    - Fixed parallel build issue for romlib config
  - Arm SGI
    - Fixed bug in SDEI receive event of RAS handler
  - Intel Agilex
    - Fixed PLAT_MAX_PWR_LVL value
  - Marvell
    - Fixed SPD handling in dram port
  - Marvell ARMADA
    - Fixed TRNG return SMC handling
    - Fixed the logic used for LD selector mask
    - Fixed MSS firmware loader for A8K family
  - ST
    - Fixed few violations reported by coverity static checks
  - STM32MP1
    - Fixed SELFREF_TO_X32 mask in ddr driver
    - Do not keep mmc_device_info in stack
    - Correct plat_crash_console_flush()
  - QEMU SBSA
    - Fixed memory type of secure NOR flash
  - QTI
    - Fixed NUM_APID and REG_APID_MAP() argument in SPMI driver
  - Intel
    - Do not keep mmc_device_info in stack
  - Hisilicon
    - Do not keep mmc_device_info in stack

- Services

  - EL3 runtime
    - Fixed the EL2 context save/restore routine by removing EL2 generic timer
      system registers
    - Added fix for exception handler in BL31 by synchronizing pending EA using
      DSB barrier
  - SPMD
    - Fixed error codes to use int32_t type
  - TSPD
    - Added bug fix in tspd interrupt handling when TSP_NS_INTR_ASYNC_PREEMPT is
      enabled
  - TRNG
    - Fixed compilation errors with -O0 compile option
  - DebugFS
    - Checked channel index before calling clone function
  - PSCI
    - Fixed limit of 256 CPUs caused by cast to unsigned char
  - TSP
    - Fixed compilation erros when built with GCC 11.0.0 toolchain

- Tools

  - FIPtool
    - Do not call `make clean` for `all` target
  - CertCreate
    - Fixed bug to avoid cleaning when building the binary
    - Used preallocated parts of the HASH struct to avoid leaking HASH struct
      fields
    - Free arguments copied with strdup
    - Free keys after use
    - Free X509_EXTENSION structures on stack to avoid leaking them
    - Optimized the code to avoid unnecessary attempts to create non-requested
      certificates

## [2.4.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.3..refs/tags/v2.4) (2020-11-17)

### New Features

- Architecture support
  - Armv8.6-A
    - Added support for Armv8.6 Enhanced Counter Virtualization (ECV)
    - Added support for Armv8.6 Fine Grained Traps (FGT)
    - Added support for Armv8.6 WFE trap delays
- Bootloader images
  - Added support for Measured Boot
- Build System
  - Added build option `COT_DESC_IN_DTB` to create Chain of Trust at runtime
  - Added build option `OPENSSL_DIR` to direct tools to OpenSSL libraries
  - Added build option `RAS_TRAP_LOWER_EL_ERR_ACCESS` to enable trapping RAS
    register accesses from EL1/EL2 to EL3
  - Extended build option `BRANCH_PROTECTION` to support branch target
    identification
- Common components
  - Added support for exporting CPU nodes to the device tree
  - Added support for single and dual-root Chains of Trust in secure partitions
- Drivers
  - Added Broadcom RNG driver
  - Added Marvell `mg_conf_cm3` driver
  - Added System Control and Management Interface (SCMI) driver
  - Added STMicroelectronics ETZPC driver
  - Arm GICv3
    - Added support for detecting topology at runtime
  - Dual Root
    - Added support for platform certificates
  - Marvell Cache LLC
    - Added support for mapping the entire LLC into SRAM
  - Marvell CCU
    - Added workaround for erratum 3033912
  - Marvell CP110 COMPHY
    - Added support for SATA COMPHY polarity inversion
    - Added support for USB COMPHY polarity inversion
    - Added workaround for erratum IPCE_COMPHY-1353
  - STM32MP1 Clocks
    - Added `RTC` as a gateable clock
    - Added support for shifted clock selector bit masks
    - Added support for using additional clocks as parents
- Libraries
  - C standard library
    - Added support for hexadecimal and pointer format specifiers in `snprint()`
    - Added assembly alternatives for various library functions
  - CPU support
    - Arm Cortex-A53
      - Added workaround for erratum 1530924
    - Arm Cortex-A55
      - Added workaround for erratum 1530923
    - Arm Cortex-A57
      - Added workaround for erratum 1319537
    - Arm Cortex-A76
      - Added workaround for erratum 1165522
      - Added workaround for erratum 1791580
      - Added workaround for erratum 1868343
    - Arm Cortex-A72
      - Added workaround for erratum 1319367
    - Arm Cortex-A77
      - Added workaround for erratum 1508412
      - Added workaround for erratum 1800714
      - Added workaround for erratum 1925769
    - Arm Neoverse-N1
      - Added workaround for erratum 1868343
  - EL3 Runtime
    - Added support for saving/restoring registers related to nested
      virtualization in EL2 context switches if the architecture supports it
  - FCONF
    - Added support for Measured Boot
    - Added support for populating Chain of Trust properties
    - Added support for loading the `fw_config` image
  - Measured Boot
    - Added support for event logging
- Platforms
  - Added support for Arm Morello
  - Added support for Arm TC0
  - Added support for iEi PUZZLE-M801
  - Added support for Marvell OCTEON TX2 T9130
  - Added support for MediaTek MT8192
  - Added support for NXP i.MX 8M Nano
  - Added support for NXP i.MX 8M Plus
  - Added support for QTI CHIP SC7180
  - Added support for STM32MP151F
  - Added support for STM32MP153F
  - Added support for STM32MP157F
  - Added support for STM32MP151D
  - Added support for STM32MP153D
  - Added support for STM32MP157D
  - Arm
    - Added support for platform-owned SPs
    - Added support for resetting to BL31
  - Arm FPGA
    - Added support for Klein
    - Added support for Matterhorn
    - Added support for additional CPU clusters
  - Arm FVP
    - Added support for performing SDEI platform setup at runtime
    - Added support for SMCCC's `SMCCC_ARCH_SOC_ID` command
    - Added an `id` field under the NV-counter node in the device tree to
      differentiate between trusted and non-trusted NV-counters
    - Added support for extracting the clock frequency from the timer node in
      the device tree
  - Arm Juno
    - Added support for SMCCC's `SMCCC_ARCH_SOC_ID` command
  - Arm N1SDP
    - Added support for cross-chip PCI-e
  - Marvell
    - Added support for AVS reduction
  - Marvell ARMADA
    - Added support for twin-die combined memory device
  - Marvell ARMADA A8K
    - Added support for DDR with 32-bit bus width (both ECC and non-ECC)
  - Marvell AP806
    - Added workaround for erratum FE-4265711
  - Marvell AP807
    - Added workaround for erratum 3033912
  - Nvidia Tegra
    - Added debug printouts indicating SC7 entry sequence completion
    - Added support for SDEI
    - Added support for stack protection
    - Added support for GICv3
    - Added support for SMCCC's `SMCCC_ARCH_SOC_ID` command
  - Nvidia Tegra194
    - Added support for RAS exception handling
    - Added support for SPM
  - NXP i.MX
    - Added support for SDEI
  - QEMU SBSA
    - Added support for the Secure Partition Manager
  - QTI
    - Added RNG driver
    - Added SPMI PMIC arbitrator driver
    - Added support for SMCCC's `SMCCC_ARCH_SOC_ID` command
  - STM32MP1
    - Added support for exposing peripheral interfaces to the non-secure world
      at runtime
    - Added support for SCMI clock and reset services
    - Added support for STM32MP15x CPU revision Z
    - Added support for SMCCC services in `SP_MIN`
- Services
  - Secure Payload Dispatcher
    - Added a provision to allow clients to retrieve the service UUID
  - SPMC
    - Added secondary core endpoint information to the SPMC context structure
  - SPMD
    - Added support for booting OP-TEE as a guest S-EL1 Secure Partition on top
      of Hafnium in S-EL2
    - Added a provision for handling SPMC messages to register secondary core
      entry points
    - Added support for power management operations
- Tools
  - CertCreate
    - Added support for secure partitions
  - CertTool
    - Added support for the `fw_config` image
  - FIPTool
    - Added support for the `fw_config` image

### Changed

- Architecture support
- Bootloader images
- Build System
  - The top-level Makefile now supports building FipTool on Windows
  - The default value of `KEY_SIZE` has been changed to to 2048 when RSA is in
    use
  - The previously-deprecated macro `__ASSEMBLY__` has now been removed
- Common components
  - Certain functions that flush the console will no longer return error
    information
- Drivers
  - Arm GIC
    - Usage of `drivers/arm/gic/common/gic_common.c` has now been deprecated in
      favour of `drivers/arm/gic/vX/gicvX.mk`
    - Added support for detecting the presence of a GIC600-AE
    - Added support for detecting the presence of a GIC-Clayton
  - Marvell MCI
    - Now performs link tuning for all MCI interfaces to improve performance
  - Marvell MoChi
    - PIDI masters are no longer forced into a non-secure access level when
      `LLC_SRAM` is enabled
    - The SD/MMC controllers are now accessible from guest virtual machines
  - Mbed TLS
    - Migrated to Mbed TLS v2.24.0
  - STM32 FMC2 NAND
    - Adjusted FMC node bindings to include an EBI controller node
  - STM32 Reset
    - Added an optional timeout argument to assertion functions
  - STM32MP1 Clocks
    - Enabled several additional system clocks during initialization
- Libraries
  - C Standard Library
    - Improved `memset` performance by avoiding single-byte writes
    - Added optimized assembly variants of `memset`
  - CPU support
    - Renamed Cortex-Hercules to Cortex-A78
    - Renamed Cortex-Hercules AE to Cortex-A78 AE
    - Renamed Neoverse Zeus to Neoverse V1
  - Coreboot
    - Updated ‘coreboot_get_memory_type’ API to take an extra argument as a
      ’memory size’ that used to return a valid memory type.
  - libfdt
    - Updated to latest upstream version
- Platforms
  - Allwinner
    - Disabled non-secure access to PRCM power control registers
  - Arm
    - `BL32_BASE` is now platform-dependent when `SPD_spmd` is enabled
    - Added support for loading the Chain of Trust from the device tree
    - The firmware update check is now executed only once
    - NV-counter base addresses are now loaded from the device tree when
      `COT_DESC_IN_DTB` is enabled
    - Now loads and populates `fw_config` and `tb_fw_config`
    - FCONF population now occurs after caches have been enabled in order to
      reduce boot times
  - Arm Corstone-700
    - Platform support has been split into both an FVP and an FPGA variant
  - Arm FPGA
    - DTB and BL33 load addresses have been given sensible default values
    - Now reads generic timer counter frequency, GICD and GICR base addresses,
      and UART address from DT
    - Now treats the primary PL011 UART as an SBSA Generic UART
  - Arm FVP
    - Secure interrupt descriptions, UART parameters, clock frequencies and
      GICv3 parameters are now queried through FCONF
    - UART parameters are now queried through the device tree
    - Added an owner field to Cactus secure partitions
    - Increased the maximum size of BL2 when the Chain of Trust is loaded from
      the device tree
    - Reduces the maximum size of BL31
    - The `FVP_USE_SP804_TIMER` and `FVP_VE_USE_SP804_TIMER` build options have
      been removed in favour of a common `USE_SP804_TIMER` option
    - Added a third Cactus partition to manifests
    - Device tree nodes now store UUIDs in big-endian
  - Arm Juno
    - Increased the maximum size of BL2 when optimizations have not been applied
    - Reduced the maximum size of BL31 and BL32
  - Marvell AP807
    - Enabled snoop filters
  - Marvell ARMADA A3K
    - UART recovery images are now suffixed with `.bin`
  - Marvell ARMADA A8K
    - Option `BL31_CACHE_DISABLE` is now disabled (`0`) by default
  - Nvidia Tegra
    - Added VPR resize supported check when processing video memory resize
      requests
    - Added SMMU verification to prevent potential issues caused by undetected
      corruption of the SMMU configuration during boot
    - The GIC CPU interface is now properly disabled after CPU off
    - The GICv2 sources list and the `BL31_SIZE` definition have been made
      platform-specific
    - The SPE driver will no longer flush the console when writing individual
      characters
  - Nvidia Tegra194
    - TZDRAM setup has been moved to platform-specific early boot handlers
    - Increased verbosity of debug prints for RAS SErrors
    - Support for powering down CPUs during CPU suspend has been removed
    - Now verifies firewall settings before using resources
  - TI K3
    - The UART number has been made configurable through `K3_USART`
  - Rockchip RK3368
    - The maximum number of memory map regions has been increased to 20
  - Socionext Uniphier
    - The maximum size of BL33 has been increased to support larger bootloaders
  - STM32
    - Removed platform-specific DT functions in favour of using existing generic
      alternatives
  - STM32MP1
    - Increased verbosity of exception reports in debug builds
    - Device trees have been updated to align with the Linux kernel
    - Now uses the ETZPC driver to configure secure-aware interfaces for
      assignment to the non-secure world
    - Finished good variants have been added to the board identifier
      enumerations
    - Non-secure access to clocks and reset domains now depends on their state
      of registration
    - NEON is now disabled in `SP_MIN`
    - The last page of `SYSRAM` is now used as SCMI shared memory
    - Checks to verify platform compatibility have been added to verify that an
      image is compatible with the chip ID of the running platform
  - QEMU SBSA
    - Removed support for Arm's Cortex-A53
- Services
  - Renamed SPCI to FF-A
  - SPMD
    - No longer forwards requests to the non-secure world when retrieving
      partition information
    - SPMC manifest size is now retrieved directly from SPMD instead of the
      device tree
    - The FF-A version handler now returns SPMD's version when the origin of the
      call is secure, and SPMC's version when the origin of the call is
      non-secure
  - SPMC
    - Updated the manifest to declare CPU nodes in descending order as per the
      SPM (Hafnium) multicore requirement
    - Updated the device tree to mark 2GB as device memory for the first
      partition excluding trusted DRAM region (which is reserved for SPMC)
    - Increased the number of EC contexts to the maximum number of PEs as per
      the FF-A specification
- Tools
  - FIPTool
    - Now returns `0` on `help` and `help <command>`
  - Marvell DoImage
    - Updated Mbed TLS support to v2.8
  - SPTool
    - Now appends CertTool arguments

### Resolved Issues

- Bootloader images
  - Fixed compilation errors for dual-root Chains of Trust caused by symbol
    collision
  - BL31
    - Fixed compilation errors on platforms with fewer than 4 cores caused by
      initialization code exceeding the end of the stacks
    - Fixed compilation errors when building a position-independent image
- Build System
  - Fixed invalid empty version strings
  - Fixed compilation errors on Windows caused by a non-portable architecture
    revision comparison
- Drivers
  - Arm GIC
    - Fixed spurious interrupts caused by a missing barrier
  - STM32 Flexible Memory Controller 2 (FMC2) NAND driver
    - Fixed runtime instability caused by incorrect error detection logic
  - STM32MP1 Clock driver
    - Fixed incorrectly-formatted log messages
    - Fixed runtime instability caused by improper clock gating procedures
  - STMicroelectronics Raw NAND driver
    - Fixed runtime instability caused by incorrect unit conversion when waiting
      for NAND readiness
- Libraries
  - AMU
    - Fixed timeout errors caused by excess error logging
  - EL3 Runtime
    - Fixed runtime instability caused by improper register save/restore routine
      in EL2
  - FCONF
    - Fixed failure to initialize GICv3 caused by overly-strict device tree
      requirements
  - Measured Boot
    - Fixed driver errors caused by a missing default value for the `HASH_ALG`
      build option
  - SPE
    - Fixed feature detection check that prevented CPUs supporting SVE from
      detecting support for SPE in the non-secure world
  - Translation Tables
    - Fixed various MISRA-C 2012 static analysis violations
- Platforms
  - Allwinner A64
    - Fixed USB issues on certain battery-powered device caused by improperly
      activated USB power rail
  - Arm
    - Fixed compilation errors caused by increase in BL2 size
    - Fixed compilation errors caused by missing Makefile dependencies to
      generated files when building the FIP
    - Fixed MISRA-C 2012 static analysis violations caused by unused structures
      in include directives intended to be feature-gated
  - Arm FPGA
    - Fixed initialization issues caused by incorrect MPIDR topology mapping
      logic
  - Arm RD-N1-edge
    - Fixed compilation errors caused by mismatched parentheses in Makefile
  - Arm SGI
    - Fixed crashes due to the flash memory used for cold reboot attack
      protection not being mapped
  - Intel Agilex
    - Fixed initialization issues caused by several compounding bugs
  - Marvell
    - Fixed compilation warnings caused by multiple Makefile inclusions
  - Marvell ARMADA A3K
    - Fixed boot issue in debug builds caused by checks on the BL33 load address
      that are not appropriate for this platform
  - Nvidia Tegra
    - Fixed incorrect delay timer reads
    - Fixed spurious interrupts in the non-secure world during cold boot caused
      by the arbitration bit in the memory controller not being cleared
    - Fixed faulty video memory resize sequence
  - Nvidia Tegra194
    - Fixed incorrect alignment of TZDRAM base address
  - NXP iMX8M
    - Fixed CPU hot-plug issues caused by race condition
  - STM32MP1
    - Fixed compilation errors in highly-parallel builds caused by incorrect
      Makefile dependencies
  - STM32MP157C-ED1
    - Fixed initialization issues caused by missing device tree hash node
  - Raspberry Pi 3
    - Fixed compilation errors caused by incorrect dependency ordering in
      Makefile
  - Rockchip
    - Fixed initialization issues caused by non-critical errors when parsing FDT
      being treated as critical
  - Rockchip RK3368
    - Fixed runtime instability caused by incorrect CPUID shift value
  - QEMU
    - Fixed compilation errors caused by incorrect dependency ordering in
      Makefile
  - QEMU SBSA
    - Fixed initialization issues caused by FDT exceeding reserved memory size
  - QTI
    - Fixed compilation errors caused by inclusion of a non-existent file
- Services
  - FF-A (previously SPCI)
    - Fixed SPMD aborts caused by incorrect behaviour when the manifest is
      page-aligned
- Tools
  - Fixed compilation issues when compiling tools from within their respective
    directories
  - FIPTool
    - Fixed command line parsing issues on Windows when using arguments whose
      names also happen to be a subset of another's
  - Marvell DoImage
    - Fixed PKCS signature verification errors at boot on some platforms caused
      by generation of misaligned images

### Known Issues

- Platforms
  - NVIDIA Tegra
    - Signed comparison compiler warnings occurring in libfdt are currently
      being worked around by disabling the warning for the platform until the
      underlying issue is resolved in libfdt

## [2.3.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.2..refs/tags/v2.3) (2020-04-20)

### New Features

- Arm Architecture
  - Add support for Armv8.4-SecEL2 extension through the SPCI defined SPMD/SPMC
    components.
  - Build option to support EL2 context save and restore in the secure world
    (CTX_INCLUDE_EL2_REGS).
  - Add support for SMCCC v1.2 (introducing the new SMCCC_ARCH_SOC_ID SMC). Note
    that the support is compliant, but the SVE registers save/restore will be
    done as part of future S-EL2/SPM development.
- BL-specific
  - Enhanced BL2 bootloader flow to load secure partitions based on firmware
    configuration data (fconf).
  - Changes necessary to support SEPARATE_NOBITS_REGION feature
  - TSP and BL2_AT_EL3: Add Position Independent Execution `PIE` support
- Build System
  - Add support for documentation build as a target in Makefile
  - Add `COT` build option to select the Chain of Trust to use when the Trusted
    Boot feature is enabled (default: `tbbr`).
  - Added creation and injection of secure partition packages into the FIP.
  - Build option to support SPMC component loading and run at S-EL1 or S-EL2
    (SPMD_SPM_AT_SEL2).
  - Enable MTE support
  - Enable Link Time Optimization in GCC
  - Enable -Wredundant-decls warning check
  - Makefile: Add support to optionally encrypt BL31 and BL32
  - Add support to pass the nt_fw_config DTB to OP-TEE.
  - Introduce per-BL `CPPFLAGS`, `ASFLAGS`, and `LDFLAGS`
  - build_macros: Add CREATE_SEQ function to generate sequence of numbers
- CPU Support
  - cortex-a57: Enable higher performance non-cacheable load forwarding
  - Hercules: Workaround for Errata 1688305
  - Klein: Support added for Klein CPU
  - Matterhorn: Support added for Matterhorn CPU
- Drivers
  - auth: Add `calc_hash` function for hash calculation. Used for authentication
    of images when measured boot is enabled.
  - cryptocell: Add authenticated decryption framework, and support for
    CryptoCell-713 and CryptoCell-712 RSA 3K
  - gic600: Add support for multichip configuration and Clayton
  - gicv3: Introduce makefile, Add extended PPI and SPI range, Add support for
    probing multiple GIC Redistributor frames
  - gicv4: Add GICv4 extension for GIC driver
  - io: Add an IO abstraction layer to load encrypted firmwares
  - mhu: Derive doorbell base address
  - mtd: Add SPI-NOR, SPI-NAND, SPI-MEM, and raw NAND framework
  - scmi: Allow use of multiple SCMI channels
  - scu: Add a driver for snoop control unit
- Libraries
  - coreboot: Add memory range parsing and use generic base address
  - compiler_rt: Import popcountdi2.c and popcountsi2.c files, aeabi_ldivmode.S
    file and dependencies
  - debugFS: Add DebugFS functionality
  - el3_runtime: Add support for enabling S-EL2
  - fconf: Add Firmware Configuration Framework (fconf) (experimental).
  - libc: Add memrchr function
  - locks: bakery: Use is_dcache_enabled() helper and add a DMB to the
    'read_cache_op' macro
  - psci: Add support to enable different personality of the same soc.
  - xlat_tables_v2: Add support to pass shareability attribute for normal memory
    region, use get_current_el_maybe_constant() in is_dcache_enabled(),
    read-only xlat tables for BL31 memory, and add enable_mmu()
- New Platforms Support
  - arm/arm_fpga: New platform support added for FPGA
  - arm/rddaniel: New platform support added for rd-daniel platform
  - brcm/stingray: New platform support added for Broadcom stingray platform
  - nvidia/tegra194: New platform support for Nvidia Tegra194 platform
- Platforms
  - allwinner: Implement PSCI system suspend using SCPI, add a msgbox driver for
    use with SCPI, and reserve and map space for the SCP firmware
  - allwinner: axp: Add AXP805 support
  - allwinner: power: Add DLDO4 power rail
  - amlogic: axg: Add a build flag when using ATOS as BL32 and support for the
    A113D (AXG) platform
  - arm/a5ds: Add ethernet node and L2 cache node in devicetree
  - arm/common: Add support for the new `dualroot` chain of trust
  - arm/common: Add support for SEPARATE_NOBITS_REGION
  - arm/common: Re-enable PIE when RESET_TO_BL31=1
  - arm/common: Allow boards to specify second DRAM Base address and to define
    PLAT_ARM_TZC_FILTERS
  - arm/corstone700: Add support for mhuv2 and stack protector
  - arm/fvp: Add support for fconf in BL31 and SP_MIN. Populate power domain
    descriptor dynamically by leveraging fconf APIs.
  - arm/fvp: Add Cactus/Ivy Secure Partition information and use two instances
    of Cactus at S-EL1
  - arm/fvp: Add support to run BL32 in TDRAM and BL31 in secure DRAM
  - arm/fvp: Add support for GICv4 extension and BL2 hash calculation in BL1
  - arm/n1sdp: Setup multichip gic routing table, update platform macros for
    dual-chip setup, introduce platform information SDS region, add support to
    update presence of External LLC, and enable the NEOVERSE_N1_EXTERNAL_LLC
    flag
  - arm/rdn1edge: Add support for dual-chip configuration and use CREATE_SEQ
    helper macro to compare chip count
  - arm/sgm: Always use SCMI for SGM platforms
  - arm/sgm775: Add support for dynamic config using fconf
  - arm/sgi: Add multi-chip mode parameter in HW_CONFIG dts, macros for remote
    chip device region, chip_id and multi_chip_mode to platform variant info,
    and introduce number of chips macro
  - brcm: Add BL2 and BL31 support common across Broadcom platforms
  - brcm: Add iproc SPI Nor flash support, spi driver, emmc driver, and support
    to retrieve plat_toc_flags
  - hisilicon: hikey960: Enable system power off callback
  - intel: Enable bridge access, SiP SMC secure register access, and uboot
    entrypoint support
  - intel: Implement platform specific system reset 2
  - intel: Introduce mailbox response length handling
  - imx: console: Use CONSOLE_T_BASE for UART base address and generic console_t
    data structure
  - imx8mm: Provide uart base as build option and add the support for opteed spd
    on imx8mq/imx8mm
  - imx8qx: Provide debug uart num as build
  - imx8qm: Apply clk/pinmux configuration for DEBUG_CONSOLE and provide debug
    uart num as build param
  - marvell: a8k: Implement platform specific power off and add support for
    loading MG CM3 images
  - mediatek: mt8183: Add Vmodem/Vcore DVS init level
  - qemu: Support optional encryption of BL31 and BL32 images and
    ARM_LINUX_KERNEL_AS_BL33 to pass FDT address
  - qemu: Define ARMV7_SUPPORTS_VFP
  - qemu: Implement PSCI_CPU_OFF and qemu_system_off via semihosting
  - renesas: rcar_gen3: Add new board revision for M3ULCB
  - rockchip: Enable workaround for erratum 855873, claim a macro to enable hdcp
    feature for DP, enable power domains of rk3399 before reset, add support for
    UART3 as serial output, and initialize reset and poweroff GPIOs with known
    invalid value
  - rpi: Implement PSCI CPU_OFF, use MMIO accessor, autodetect Mini-UART vs.
    PL011 configuration, and allow using PL011 UART for RPi3/RPi4
  - rpi3: Include GPIO driver in all BL stages and use same "clock-less" setup
    scheme as RPi4
  - rpi3/4: Add support for offlining CPUs
  - st: stm32mp1: platform.mk: Support generating multiple images in one build,
    migrate to implicit rules, derive map file name from target name, generate
    linker script with fixed name, and use PHONY for the appropriate targets
  - st: stm32mp1: Add support for SPI-NOR, raw NAND, and SPI-NAND boot device,
    QSPI, FMC2 driver
  - st: stm32mp1: Use stm32mp_get_ddr_ns_size() function, set XN attribute for
    some areas in BL2, dynamically map DDR later and non-cacheable during its
    test, add a function to get non-secure DDR size, add DT helper for reg by
    name, and add compilation flags for boot devices
  - socionext: uniphier: Turn on ENABLE_PIE
  - ti: k3: Add PIE support
  - xilinx: versal: Add set wakeup source, client wakeup, query data, request
    wakeup, PM_INIT_FINALIZE, PM_GET_TRUSTZONE_VERSION, PM IOCTL, support for
    suspend related, and Get_ChipID APIs
  - xilinx: versal: Implement power down/restart related EEMI, SMC handler for
    EEMI, PLL related PM, clock related PM, pin control related PM, reset
    related PM, device related PM , APIs
  - xilinx: versal: Enable ipi mailbox service
  - xilinx: versal: Add get_api_version support and support to send PM API to
    PMC using IPI
  - xilinx: zynqmp: Add checksum support for IPI data, GET_CALLBACK_DATA
    function, support to query max divisor, CLK_SET_RATE_PARENT in gem clock
    node, support for custom type flags, LPD WDT clock to the pm_clock
    structure, idcodes for new RFSoC silicons ZU48DR and ZU49DR, and id for new
    RFSoC device ZU39DR
- Security
  - Use Speculation Barrier instruction for v8.5+ cores
  - Add support for optional firmware encryption feature (experimental).
  - Introduce a new `dualroot` chain of trust.
  - aarch64: Prevent speculative execution past ERET
  - aarch32: Stop speculative execution past exception returns.
- SPCI
  - Introduced the Secure Partition Manager Dispatcher (SPMD) component as a new
    standard service.
- Tools
  - cert_create: Introduce CoT build option and TBBR CoT makefile, and define
    the dualroot CoT
  - encrypt_fw: Add firmware authenticated encryption tool
  - memory: Add show_memory script that prints a representation of the memory
    layout for the latest build

### Changed

- Arm Architecture
  - PIE: Make call to GDT relocation fixup generalized
- BL-Specific
  - Increase maximum size of BL2 image
  - BL31: Discard .dynsym .dynstr .hash sections to make ENABLE_PIE work
  - BL31: Split into two separate memory regions
  - Unify BL linker scripts and reduce code duplication.
- Build System
  - Changes to drive cert_create for dualroot CoT
  - Enable -Wlogical-op always
  - Enable -Wshadow always
  - Refactor the warning flags
  - PIE: Pass PIE options only to BL31
  - Reduce space lost to object alignment
  - Set lld as the default linker for Clang builds
  - Remove -Wunused-const-variable and -Wpadded warning
  - Remove -Wmissing-declarations warning from WARNING1 level
- Drivers
  - authentication: Necessary fix in drivers to upgrade to mbedtls-2.18.0
  - console: Integrate UART base address in generic console_t
  - gicv3: Change API for GICR_IPRIORITYR accessors and separate GICD and GICR
    accessor functions
  - io: Change seek offset to signed long long and panic in case of io setup
    failure
  - smmu: SMMUv3: Changed retry loop to delay timer
  - tbbr: Reduce size of hash and ECDSA key buffers when possible
- Library Code
  - libc: Consolidate the size_t, unified, and NULL definitions, and unify
    intmax_t and uintmax_t on AArch32/64
  - ROMLIB: Optimize memory layout when ROMLIB is used
  - xlat_tables_v2: Use ARRAY_SIZE in REGISTER_XLAT_CONTEXT_FULL_SPEC, merge
    REGISTER_XLAT_CONTEXT\_{FULL_SPEC,RO_BASE_TABLE}, and simplify end address
    checks in mmap_add_region_check()
- Platforms
  - allwinner: Adjust SRAM A2 base to include the ARISC vectors, clean up MMU
    setup, reenable USE_COHERENT_MEM, remove unused include path, move the
    NOBITS region to SRAM A1, convert AXP803 regulator setup code into a driver,
    enable clock before resetting I2C/RSB
  - allwinner: h6: power: Switch to using the AXP driver
  - allwinner: a64: power: Use fdt_for_each_subnode, remove obsolete register
    check, remove duplicate DT check, and make sunxi_turn_off_soc static
  - allwinner: Build PMIC bus drivers only in BL31, clean up PMIC-related error
    handling, and synchronize PMIC enumerations
  - arm/a5ds: Change boot address to point to DDR address
  - arm/common: Check for out-of-bound accesses in the platform io policies
  - arm/corstone700: Updating the kernel arguments to support initramfs, use
    fdts DDR memory and XIP rootfs, and set UART clocks to 32MHz
  - arm/fvp: Modify multithreaded dts file of DynamIQ FVPs, slightly bump the
    stack size for bl1 and bl2, remove re-definition of topology related build
    options, stop reclaiming init code with Clang builds, and map only the
    needed DRAM region statically in BL31/SP_MIN
  - arm/juno: Maximize space allocated to SCP_BL2
  - arm/sgi: Bump bl1 RW limit, mark remote chip shared ram as non-cacheable,
    move GIC related constants to board files, include AFF3 affinity in core
    position calculation, move bl31_platform_setup to board file, and move
    topology information to board folder
  - common: Refactor load_auth_image_internal().
  - hisilicon: Remove uefi-tools in hikey and hikey960 documentation
  - intel: Modify non secure access function, BL31 address mapping, mailbox's
    get_config_status, and stratix10 BL31 parameter handling
  - intel: Remove un-needed checks for qspi driver r/w and s10 unused source
    code
  - intel: Change all global sip function to static
  - intel: Refactor common platform code
  - intel: Create SiP service header file
  - marvell: armada: scp_bl2: Allow loading up to 8 images
  - marvell: comphy-a3700: Support SGMII COMPHY power off and fix USB3 powering
    on when on lane 2
  - marvell: Consolidate console register calls
  - mediatek: mt8183: Protect 4GB~8GB dram memory, refine GIC driver for low
    power scenarios, and switch PLL/CLKSQ/ck_off/axi_26m control to SPM
  - qemu: Update flash address map to keep FIP in secure FLASH0
  - renesas: rcar_gen3: Update IPL and Secure Monitor Rev.2.0.6, update DDR
    setting for H3, M3, M3N, change fixed destination address of BL31 and BL32,
    add missing #{address,size}-cells into generated DT, pass DT to OpTee OS,
    and move DDR drivers out of staging
  - rockchip: Make miniloader ddr_parameter handling optional, cleanup securing
    of ddr regions, move secure init to separate file, use base+size for secure
    ddr regions, bring TZRAM_SIZE values in lined, and prevent macro expansion
    in paths
  - rpi: Move plat_helpers.S to common
  - rpi3: gpio: Simplify GPIO setup
  - rpi4: Skip UART initialisation
  - st: stm32m1: Use generic console_t data structure, remove second QSPI flash
    instance, update for FMC2 pin muxing, and reduce MAX_XLAT_TABLES to 4
  - socionext: uniphier: Make on-chip SRAM and I/O register regions configurable
  - socionext: uniphier: Make PSCI related, counter control, UART, pinmon, NAND
    controller, and eMMC controller base addresses configurable
  - socionext: uniphier: Change block_addressing flag and the return value type
    of .is_usb_boot() to bool
  - socionext: uniphier: Run BL33 at EL2, call uniphier_scp_is_running() only
    when on-chip STM is supported, define PLAT_XLAT_TABLES_DYNAMIC only for BL2,
    support read-only xlat tables, use enable_mmu() in common function, shrink
    UNIPHIER_ROM_REGION_SIZE, prepare uniphier_soc_info() for next SoC, extend
    boot device detection for future SoCs, make all BL images completely
    position-independent, make uniphier_mmap_setup() work with PIE, pass SCP
    base address as a function parameter, set buffer offset and length for
    io_block dynamically, and use more mmap_add_dynamic_region() for loading
    images
  - spd/trusty: Disable error messages seen during boot, allow gic base to be
    specified with GICD_BASE, and allow getting trusty memsize from
    BL32_MEM_SIZE instead of TSP_SEC_MEM_SIZE
  - ti: k3: common: Enable ARM cluster power down and rename device IDs to be
    more consistent
  - ti: k3: drivers: ti_sci: Put sequence number in coherent memory and remove
    indirect structure of const data
  - xilinx: Move ipi mailbox svc to xilinx common
  - xilinx: zynqmp: Use GIC framework for warm restart
  - xilinx: zynqmp: pm: Move custom clock flags to typeflags, remove
    CLK_TOPSW_LSBUS from invalid clock list and rename FPD WDT clock ID
  - xilinx: versal: Increase OCM memory size for DEBUG builds and adjust cpu
    clock, Move versal_def.h and versal_private to include directory
- Tools
  - sptool: Updated sptool to accommodate building secure partition packages.

### Resolved Issues

- Arm Architecture
  - Fix crash dump for lower EL
- BL-Specific
  - Bug fix: Protect TSP prints with lock
  - Fix boot failures on some builds linked with ld.lld.
- Build System
  - Fix clang build if CC is not in the path.
  - Fix 'BL stage' comment for build macros
- Code Quality
  - coverity: Fix various MISRA violations including null pointer violations, C
    issues in BL1/BL2/BL31 and FDT helper functions, using boolean essential,
    type, and removing unnecessary header file and comparisons to LONG_MAX in
    debugfs devfip
  - Based on coding guidelines, replace all `unsigned long` depending on if
    fixed based on AArch32 or AArch64.
  - Unify type of "cpu_idx" and Platform specific defines across PSCI module.
- Drivers
  - auth: Necessary fix in drivers to upgrade to mbedtls-2.18.0
  - delay_timer: Fix non-standard frequency issue in udelay
  - gicv3: Fix compiler dependent behavior
  - gic600: Fix include ordering according to the coding style and power up
    sequence
- Library Code
  - el3_runtime: Fix stack pointer maintenance on EA handling path, fixup
    'cm_setup_context' prototype, and adds TPIDR_EL2 register to the context
    save restore routines
  - libc: Fix SIZE_MAX on AArch32
  - locks: T589: Fix insufficient ordering guarantees in bakery lock
  - pmf: Fix 'tautological-constant-compare' error, Make the runtime
    instrumentation work on AArch32, and Simplify PMF helper macro definitions
    across header files
  - xlat_tables_v2: Fix assembler warning of PLAT_RO_XLAT_TABLES
- Platforms
  - allwinner: Fix H6 GPIO and CCU memory map addresses and incorrect ARISC code
    patch offset check
  - arm/a5ds: Correct system freq and Cache Writeback Granule, and cleanup
    enable-method in devicetree
  - arm/fvp: Fix incorrect GIC mapping, BL31 load address and image size for
    RESET_TO_BL31=1, topology description of cpus for DynamIQ based FVP, and
    multithreaded FVP power domain tree
  - arm/fvp: spm-mm: Correcting instructions to build SPM for FVP
  - arm/common: Fix ROTPK hash generation for ECDSA encryption, BL2 bug in
    dynamic configuration initialisation, and current RECLAIM_INIT_CODE behavior
  - arm/rde1edge: Fix incorrect topology tree description
  - arm/sgi: Fix the incorrect check for SCMI channel ID
  - common: Flush dcache when storing timestamp
  - intel: Fix UEFI decompression issue, memory calibration, SMC SIP service,
    mailbox config return status, mailbox driver logic, FPGA manager on
    reconfiguration, and mailbox send_cmd issue
  - imx: Fix shift-overflow errors, the rdc memory region slot's offset,
    multiple definition of ipc_handle, missing inclusion of cdefs.h, and correct
    the SGIs that used for secure interrupt
  - mediatek: mt8183: Fix AARCH64 init fail on CPU0
  - rockchip: Fix definition of struct param_ddr_usage
  - rpi4: Fix documentation of armstub config entry
  - st: Correct io possible NULL pointer dereference and device_size type, nand
    xor_ecc.val assigned value, static analysis tool issues, and fix incorrect
    return value and correctly check pwr-regulators node
  - xilinx: zynqmp: Correct syscnt freq for QEMU and fix clock models and IDs of
    GEM-related clocks

### Known Issues

- Build System
  - dtb: DTB creation not supported when building on a Windows host.

    This step in the build process is skipped when running on a Windows host. A
    known issue from the 1.6 release.

  - Intermittent assertion firing `ASSERT: services/spd/tspd/tspd_main.c:105`
- Coverity
  - Intermittent Race condition in Coverity Jenkins Build Job
- Platforms
  - arm/juno: System suspend from Linux does not function as documented in the
    user guide

    Following the instructions provided in the user guide document does not
    result in the platform entering system suspend state as expected. A message
    relating to the hdlcd driver failing to suspend will be emitted on the Linux
    terminal.

  - mediatek/mt6795: This platform does not build in this release

## [2.2.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.1..refs/tags/v2.2) (2019-10-22)

### New Features

- Architecture
  - Enable Pointer Authentication (PAuth) support for Secure World

    - Adds support for ARMv8.3-PAuth in BL1 SMC calls and BL2U image for
      firmware updates.

  - Enable Memory Tagging Extension (MTE) support in both secure and non-secure
    worlds

    - Adds support for the new Memory Tagging Extension arriving in ARMv8.5. MTE
      support is now enabled by default on systems that support it at EL0.
    - To enable it at ELx for both the non-secure and the secure world, the
      compiler flag `CTX_INCLUDE_MTE_REGS` includes register saving and
      restoring when necessary in order to prevent information leakage between
      the worlds.

  - Add support for Branch Target Identification (BTI)
- Build System
  - Modify FVP makefile for CPUs that support both AArch64/32
  - AArch32: Allow compiling with soft-float toolchain
  - Makefile: Add default warning flags
  - Add Makefile check for PAuth and AArch64
  - Add compile-time errors for HW_ASSISTED_COHERENCY flag
  - Apply compile-time check for AArch64-only CPUs
  - build_macros: Add mechanism to prevent bin generation.
  - Add support for default stack-protector flag
  - spd: opteed: Enable NS_TIMER_SWITCH
  - plat/arm: Skip BL2U if RESET_TO_SP_MIN flag is set
  - Add new build option to let each platform select which implementation of
    spinlocks it wants to use
- CPU Support
  - DSU: Workaround for erratum 798953 and 936184
  - Neoverse N1: Force cacheable atomic to near atomic
  - Neoverse N1: Workaround for erratum 1073348, 1130799, 1165347, 1207823,
    1220197, 1257314, 1262606, 1262888, 1275112, 1315703, 1542419
  - Neoverse Zeus: Apply the MSR SSBS instruction
  - cortex-Hercules/HerculesAE: Support added for Cortex-Hercules and
    Cortex-HerculesAE CPUs
  - cortex-Hercules/HerculesAE: Enable AMU for Cortex-Hercules and
    Cortex-HerculesAE
  - cortex-a76AE: Support added for Cortex-A76AE CPU
  - cortex-a76: Workaround for erratum 1257314, 1262606, 1262888, 1275112,
    1286807
  - cortex-a65/a65AE: Support added for Cortex-A65 and Cortex-A65AE CPUs
  - cortex-a65: Enable AMU for Cortex-A65
  - cortex-a55: Workaround for erratum 1221012
  - cortex-a35: Workaround for erratum 855472
  - cortex-a9: Workaround for erratum 794073
- Drivers
  - console: Allow the console to register multiple times

  - delay: Timeout detection support

  - gicv3: Enabled multi-socket GIC redistributor frame discovery and migrated
    ARM platforms to the new API

    - Adds `gicv3_rdistif_probe` function that delegates the responsibility of
      discovering the corresponding redistributor base frame to each CPU itself.

  - sbsa: Add SBSA watchdog driver

  - st/stm32_hash: Add HASH driver

  - ti/uart: Add an AArch32 variant
- Library at ROM (romlib)
  - Introduce BTI support in Library at ROM (romlib)
- New Platforms Support
  - amlogic: g12a: New platform support added for the S905X2 (G12A) platform
  - amlogic: meson/gxl: New platform support added for Amlogic Meson S905x (GXL)
  - arm/a5ds: New platform support added for A5 DesignStart
  - arm/corstone: New platform support added for Corstone-700
  - intel: New platform support added for Agilex
  - mediatek: New platform support added for MediaTek mt8183
  - qemu/qemu_sbsa: New platform support added for QEMU SBSA platform
  - renesas/rcar_gen3: plat: New platform support added for D3
  - rockchip: New platform support added for px30
  - rockchip: New platform support added for rk3288
  - rpi: New platform support added for Raspberry Pi 4
- Platforms
  - arm/common: Introduce wrapper functions to setup secure watchdog
  - arm/fvp: Add Delay Timer driver to BL1 and BL31 and option for defining
    platform DRAM2 base
  - arm/fvp: Add Linux DTS files for 32 bit threaded FVPs
  - arm/n1sdp: Add code for DDR ECC enablement and BL33 copy to DDR, Initialise
    CNTFRQ in Non Secure CNTBaseN
  - arm/juno: Use shared mbedtls heap between BL1 and BL2 and add basic support
    for dynamic config
  - imx: Basic support for PicoPi iMX7D, rdc module init, caam module init,
    aipstz init, IMX_SIP_GET_SOC_INFO, IMX_SIP_BUILDINFO added
  - intel: Add ncore ccu driver
  - mediatek/mt81\*: Use new bl31_params_parse() helper
  - nvidia: tegra: Add support for multi console interface
  - qemu/qemu_sbsa: Adding memory mapping for both FLASH0/FLASH1
  - qemu: Added gicv3 support, new console interface in AArch32, and
    sub-platforms
  - renesas/rcar_gen3: plat: Add R-Car V3M support, new board revision for
    H3ULCB, DBSC4 setting before self-refresh mode
  - socionext/uniphier: Support console based on multi-console
  - st: stm32mp1: Add OP-TEE, Avenger96, watchdog, LpDDR3, authentication
    support and general SYSCFG management
  - ti/k3: common: Add support for J721E, Use coherent memory for shared data,
    Trap all asynchronous bus errors to EL3
  - xilinx/zynqmp: Add support for multi console interface, Initialize IPI table
    from zynqmp_config_setup()
- PSCI
  - Adding new optional PSCI hook `pwr_domain_on_finish_late`
    - This PSCI hook `pwr_domain_on_finish_late` is similar to
      `pwr_domain_on_finish` but is guaranteed to be invoked when the respective
      core and cluster are participating in coherency.
- Security
  - Speculative Store Bypass Safe (SSBS): Further enhance protection against
    Spectre variant 4 by disabling speculative loads/stores (SPSR.SSBS bit) by
    default.
  - UBSAN support and handlers
    - Adds support for the Undefined Behaviour sanitizer. There are two types of
      support offered - minimalistic trapping support which essentially
      immediately crashes on undefined behaviour and full support with full
      debug messages.
- Tools
  - cert_create: Add support for bigger RSA key sizes (3KB and 4KB), previously
    the maximum size was 2KB.
  - fiptool: Add support to build fiptool on Windows.

### Changed

- Architecture
  - Refactor ARMv8.3 Pointer Authentication support code
  - backtrace: Strip PAC field when PAUTH is enabled
  - Prettify crash reporting output on AArch64.
  - Rework smc_unknown return code path in smc_handler
    - Leverage the existing `el3_exit()` return routine for smc_unknown return
      path rather than a custom set of instructions.
- BL-Specific
  - Invalidate dcache build option for BL2 entry at EL3
  - Add missing support for BL2_AT_EL3 in XIP memory
- Boot Flow
  - Add helper to parse BL31 parameters (both versions)
  - Factor out cross-BL API into export headers suitable for 3rd party code
  - Introduce lightweight BL platform parameter library
- Drivers
  - auth: Memory optimization for Chain of Trust (CoT) description
  - bsec: Move bsec_mode_is_closed_device() service to platform
  - cryptocell: Move Cryptocell specific API into driver
  - gicv3: Prevent pending G1S interrupt from becoming G0 interrupt
  - mbedtls: Remove weak heap implementation
  - mmc: Increase delay between ACMD41 retries
  - mmc: stm32_sdmmc2: Correctly manage block size
  - mmc: stm32_sdmmc2: Manage max-frequency property from DT
  - synopsys/emmc: Do not change FIFO TH as this breaks some platforms
  - synopsys: Update synopsys drivers to not rely on undefined overflow
    behaviour
  - ufs: Extend the delay after reset to wait for some slower chips
- Platforms
  - amlogic/meson/gxl: Remove BL2 dependency from BL31
  - arm/common: Shorten the Firmware Update (FWU) process
  - arm/fvp: Remove GIC initialisation from secondary core cold boot
  - arm/sgm: Temporarily disable shared Mbed TLS heap for SGM
  - hisilicon: Update hisilicon drivers to not rely on undefined overflow
    behaviour
  - imx: imx8: Replace PLAT_IMX8\* with PLAT_imx8\*, remove duplicated linker
    symbols and deprecated code include, keep only IRQ 32 unmasked, enable all
    power domain by default
  - marvell: Prevent SError accessing PCIe link, Switch to xlat_tables_v2, do
    not rely on argument passed via smc, make sure that comphy init will use
    correct address
  - mediatek: mt8173: Refactor RTC and PMIC drivers
  - mediatek: mt8173: Apply MULTI_CONSOLE framework
  - nvidia: Tegra: memctrl_v2: fix "overflow before widen" coverity issue
  - qemu: Simplify the image size calculation, Move and generalise FDT PSCI
    fixup, move gicv2 codes to separate file
  - renesas/rcar_gen3: Convert to multi-console API, update QoS setting, Update
    IPL and Secure Monitor Rev2.0.4, Change to restore timer counter value at
    resume, Update DDR setting rev.0.35, qos: change subslot cycle, Change
    periodic write DQ training option.
  - rockchip: Allow SOCs with undefined wfe check bits, Streamline and complete
    UARTn_BASE macros, drop rockchip-specific imported linker symbols for bl31,
    Disable binary generation for all SOCs, Allow console device to be set by
    DTB, Use new bl31_params_parse functions
  - rpi/rpi3: Move shared rpi3 files into common directory
  - socionext/uniphier: Set CONSOLE_FLAG_TRANSLATE_CRLF and clean up console
    driver
  - socionext/uniphier: Replace DIV_ROUND_UP() with div_round_up() from
    utils_def.h
  - st/stm32mp: Split stm32mp_io_setup function, move
    stm32_get_gpio_bank_clock() to private file, correctly handle Clock
    Spreading Generator, move oscillator functions to generic file, realign
    device tree files with internal devs, enable RTCAPB clock for dual-core
    chips, use a common function to check spinlock is available, move
    check_header() to common code
  - ti/k3: Enable SEPARATE_CODE_AND_RODATA by default, Remove shared RAM space,
    Drop \_ADDRESS from K3_USART_BASE to match other defines, Remove MSMC port
    definitions, Allow USE_COHERENT_MEM for K3, Set L2 latency on A72 cores
- PSCI
  - PSCI: Lookup list of parent nodes to lock only once
- Secure Partition Manager (SPM): SPCI Prototype
  - Fix service UUID lookup
  - Adjust size of virtual address space per partition
  - Refactor xlat context creation
  - Move shim layer to TTBR1_EL1
  - Ignore empty regions in resource description
- Security
  - Refactor SPSR initialisation code
  - SMMUv3: Abort DMA transactions
    - For security DMA should be blocked at the SMMU by default unless
      explicitly enabled for a device. SMMU is disabled after reset with all
      streams bypassing the SMMU, and abortion of all incoming transactions
      implements a default deny policy on reset.
    - Moves `bl1_platform_setup()` function from arm_bl1_setup.c to FVP
      platforms' fvp_bl1_setup.c and fvp_ve_bl1_setup.c files.
- Tools
  - cert_create: Remove RSA PKCS#1 v1.5 support

### Resolved Issues

- Architecture
  - Fix the CAS spinlock implementation by adding a missing DSB in
    `spin_unlock()`
  - AArch64: Fix SCTLR bit definitions
    - Removes incorrect `SCTLR_V_BIT` definition and adds definitions for
      ARMv8.3-Pauth `EnIB`, `EnDA` and `EnDB` bits.
  - Fix restoration of PAuth context
    - Replace call to `pauth_context_save()` with `pauth_context_restore()` in
      case of unknown SMC call.
- BL-Specific Issues
  - Fix BL31 crash reporting on AArch64 only platforms
- Build System
  - Remove several warnings reported with W=2 and W=1
- Code Quality Issues
  - SCTLR and ACTLR are 32-bit for AArch32 and 64-bit for AArch64
  - Unify type of "cpu_idx" across PSCI module.
  - Assert if power level value greater then PSCI_INVALID_PWR_LVL
  - Unsigned long should not be used as per coding guidelines
  - Reduce the number of memory leaks in cert_create
  - Fix type of cot_desc_ptr
  - Use explicit-width data types in AAPCS parameter structs
  - Add python configuration for editorconfig
  - BL1: Fix type consistency
  - Enable -Wshift-overflow=2 to check for undefined shift behavior
  - Updated upstream platforms to not rely on undefined overflow behaviour
- Coverity Quality Issues
  - Remove GGC ignore -Warray-bounds
  - Fix Coverity #261967, Infinite loop
  - Fix Coverity #343017, Missing unlock
  - Fix Coverity #343008, Side affect in assertion
  - Fix Coverity #342970, Uninitialized scalar variable
- CPU Support
  - cortex-a12: Fix MIDR mask
- Drivers
  - console: Remove Arm console unregister on suspend
  - gicv3: Fix support for full SPI range
  - scmi: Fix wrong payload length
- Library Code
  - libc: Fix sparse warning for \_\_assert()
  - libc: Fix memchr implementation
- Platforms
  - rpi: rpi3: Fix compilation error when stack protector is enabled
  - socionext/uniphier: Fix compilation fail for SPM support build config
  - st/stm32mp1: Fix TZC400 configuration against non-secure DDR
  - ti/k3: common: Fix RO data area size calculation
- Security
  - AArch32: Disable Secure Cycle Counter
    - Changes the implementation for disabling Secure Cycle Counter. For ARMv8.5
      the counter gets disabled by setting `SDCR.SCCD` bit on CPU cold/warm
      boot. For the earlier architectures PMCR register is saved/restored on
      secure world entry/exit from/to Non-secure state, and cycle counting gets
      disabled by setting PMCR.DP bit.
  - AArch64: Disable Secure Cycle Counter
    - For ARMv8.5 the counter gets disabled by setting `MDCR_El3.SCCD` bit on
      CPU cold/warm boot. For the earlier architectures PMCR_EL0 register is
      saved/restored on secure world entry/exit from/to Non-secure state, and
      cycle counting gets disabled by setting PMCR_EL0.DP bit.

### Deprecations

- Common Code
  - Remove MULTI_CONSOLE_API flag and references to it
  - Remove deprecated `plat_crash_console_*`
  - Remove deprecated interfaces `get_afflvl_shift`, `mpidr_mask_lower_afflvls`,
    `eret`
  - AARCH32/AARCH64 macros are now deprecated in favor of `__aarch64__`
  - `__ASSEMBLY__` macro is now deprecated in favor of `__ASSEMBLER__`
- Drivers
  - console: Removed legacy console API
  - console: Remove deprecated finish_console_register
  - tzc: Remove deprecated types `tzc_action_t` and `tzc_region_attributes_t`
- Secure Partition Manager (SPM):
  - Prototype SPCI-based SPM (services/std_svc/spm) will be replaced with
    alternative methods of secure partitioning support.

### Known Issues

- Build System Issues
  - dtb: DTB creation not supported when building on a Windows host.

    This step in the build process is skipped when running on a Windows host. A
    known issue from the 1.6 release.
- Platform Issues
  - arm/juno: System suspend from Linux does not function as documented in the
    user guide

    Following the instructions provided in the user guide document does not
    result in the platform entering system suspend state as expected. A message
    relating to the hdlcd driver failing to suspend will be emitted on the Linux
    terminal.

  - mediatek/mt6795: This platform does not build in this release

## [2.1.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.0..refs/tags/v2.1) (2019-03-29)

### New Features

- Architecture

  - Support for ARMv8.3 pointer authentication in the normal and secure worlds

    The use of pointer authentication in the normal world is enabled whenever
    architectural support is available, without the need for additional build
    flags.

    Use of pointer authentication in the secure world remains an experimental
    configuration at this time. Using both the `ENABLE_PAUTH` and
    `CTX_INCLUDE_PAUTH_REGS` build flags, pointer authentication can be enabled
    in EL3 and S-EL1/0.

    See the {ref}`Firmware Design` document for additional details on the use of
    pointer authentication.

  - Enable Data Independent Timing (DIT) in EL3, where supported

- Build System

  - Support for BL-specific build flags

  - Support setting compiler target architecture based on `ARM_ARCH_MINOR` build
    option.

  - New `RECLAIM_INIT_CODE` build flag:

    A significant amount of the code used for the initialization of BL31 is not
    needed again after boot time. In order to reduce the runtime memory
    footprint, the memory used for this code can be reclaimed after
    initialization.

    Certain boot-time functions were marked with the `__init` attribute to
    enable this reclamation.

- CPU Support

  - cortex-a76: Workaround for erratum 1073348
  - cortex-a76: Workaround for erratum 1220197
  - cortex-a76: Workaround for erratum 1130799
  - cortex-a75: Workaround for erratum 790748
  - cortex-a75: Workaround for erratum 764081
  - cortex-a73: Workaround for erratum 852427
  - cortex-a73: Workaround for erratum 855423
  - cortex-a57: Workaround for erratum 817169
  - cortex-a57: Workaround for erratum 814670
  - cortex-a55: Workaround for erratum 903758
  - cortex-a55: Workaround for erratum 846532
  - cortex-a55: Workaround for erratum 798797
  - cortex-a55: Workaround for erratum 778703
  - cortex-a55: Workaround for erratum 768277
  - cortex-a53: Workaround for erratum 819472
  - cortex-a53: Workaround for erratum 824069
  - cortex-a53: Workaround for erratum 827319
  - cortex-a17: Workaround for erratum 852423
  - cortex-a17: Workaround for erratum 852421
  - cortex-a15: Workaround for erratum 816470
  - cortex-a15: Workaround for erratum 827671

- Documentation

  - Exception Handling Framework documentation
  - Library at ROM (romlib) documentation
  - RAS framework documentation
  - Coding Guidelines document

- Drivers

  - ccn: Add API for setting and reading node registers

    - Adds `ccn_read_node_reg` function
    - Adds `ccn_write_node_reg` function

  - partition: Support MBR partition entries

  - scmi: Add `plat_css_get_scmi_info` function

    Adds a new API `plat_css_get_scmi_info` which lets the platform register a
    platform-specific instance of `scmi_channel_plat_info_t` and remove the
    default values

  - tzc380: Add TZC-380 TrustZone Controller driver

  - tzc-dmc620: Add driver to manage the TrustZone Controller within the DMC-620
    Dynamic Memory Controller

- Library at ROM (romlib)

  - Add platform-specific jump table list

  - Allow patching of romlib functions

    This change allows patching of functions in the romlib. This can be done by
    adding "patch" at the end of the jump table entry for the function that
    needs to be patched in the file jmptbl.i.

- Library Code

  - Support non-LPAE-enabled MMU tables in AArch32
  - mmio: Add `mmio_clrsetbits_16` function
    - 16-bit variant of `mmio_clrsetbits`
  - object_pool: Add Object Pool Allocator
    - Manages object allocation using a fixed-size static array
    - Adds `pool_alloc` and `pool_alloc_n` functions
    - Does not provide any functions to free allocated objects (by design)
  - libc: Added `strlcpy` function
  - libc: Import `strrchr` function from FreeBSD
  - xlat_tables: Add support for ARMv8.4-TTST
  - xlat_tables: Support mapping regions without an explicitly specified VA

- Math

  - Added softudiv macro to support software division

- Memory Partitioning And Monitoring (MPAM)

  - Enabled MPAM EL2 traps (`MPAMHCR_EL2` and `MPAM_EL2`)

- Platforms

  - amlogic: Add support for Meson S905 (GXBB)

  - arm/fvp_ve: Add support for FVP Versatile Express platform

  - arm/n1sdp: Add support for Neoverse N1 System Development platform

  - arm/rde1edge: Add support for Neoverse E1 platform

  - arm/rdn1edge: Add support for Neoverse N1 platform

  - arm: Add support for booting directly to Linux without an intermediate
    loader (AArch32)

  - arm/juno: Enable new CPU errata workarounds for A53 and A57

  - arm/juno: Add romlib support

    Building a combined BL1 and ROMLIB binary file with the correct page
    alignment is now supported on the Juno platform. When `USE_ROMLIB` is set
    for Juno, it generates the combined file `bl1_romlib.bin` which needs to be
    used instead of bl1.bin.

  - intel/stratix: Add support for Intel Stratix 10 SoC FPGA platform

  - marvell: Add support for Armada-37xx SoC platform

  - nxp: Add support for i.MX8M and i.MX7 Warp7 platforms

  - renesas: Add support for R-Car Gen3 platform

  - xilinx: Add support for Versal ACAP platforms

- Position-Independent Executable (PIE)

  PIE support has initially been added to BL31. The `ENABLE_PIE` build flag is
  used to enable or disable this functionality as required.

- Secure Partition Manager

  - New SPM implementation based on SPCI Alpha 1 draft specification

    A new version of SPM has been implemented, based on the SPCI (Secure
    Partition Client Interface) and SPRT (Secure Partition Runtime) draft
    specifications.

    The new implementation is a prototype that is expected to undergo intensive
    rework as the specifications change. It has basic support for multiple
    Secure Partitions and Resource Descriptions.

    The older version of SPM, based on MM (ARM Management Mode Interface
    Specification), is still present in the codebase. A new build flag, `SPM_MM`
    has been added to allow selection of the desired implementation. This flag
    defaults to 1, selecting the MM-based implementation.

- Security

  - Spectre Variant-1 mitigations (`CVE-2017-5753`)

  - Use Speculation Store Bypass Safe (SSBS) functionality where available

    Provides mitigation against `CVE-2018-19440` (Not saving x0 to x3 registers
    can leak information from one Normal World SMC client to another)

### Changed

- Build System

  - Warning levels are now selectable with `W=<1,2,3>`
  - Removed unneeded include paths in PLAT_INCLUDES
  - "Warnings as errors" (Werror) can be disabled using `E=0`
  - Support totally quiet output with `-s` flag
  - Support passing options to checkpatch using `CHECKPATCH_OPTS=<opts>`
  - Invoke host compiler with `HOSTCC / HOSTCCFLAGS` instead of `CC / CFLAGS`
  - Make device tree pre-processing similar to U-boot/Linux by:
    - Creating separate `CPPFLAGS` for DT preprocessing so that compiler options
      specific to it can be accommodated.
    - Replacing `CPP` with `PP` for DT pre-processing

- CPU Support

  - Errata report function definition is now mandatory for CPU support files

    CPU operation files must now define a `<name>_errata_report` function to
    print errata status. This is no longer a weak reference.

- Documentation

  - Migrated some content from GitHub wiki to `docs/` directory
  - Security advisories now have CVE links
  - Updated copyright guidelines

- Drivers

  - console: The `MULTI_CONSOLE_API` framework has been rewritten in C

  - console: Ported multi-console driver to AArch32

  - gic: Remove 'lowest priority' constants

    Removed `GIC_LOWEST_SEC_PRIORITY` and `GIC_LOWEST_NS_PRIORITY`. Platforms
    should define these if required, or instead determine the correct priority
    values at runtime.

  - delay_timer: Check that the Generic Timer extension is present

  - mmc: Increase command reply timeout to 10 milliseconds

  - mmc: Poll eMMC device status to ensure `EXT_CSD` command completion

  - mmc: Correctly check return code from `mmc_fill_device_info`

- External Libraries

  - libfdt: Upgraded from 1.4.2 to 1.4.6-9

  >

  - mbed TLS: Upgraded from 2.12 to 2.16

  >

  This change incorporates fixes for security issues that should be reviewed to
  determine if they are relevant for software implementations using Trusted
  Firmware-A. See the [mbed TLS releases] page for details on changes from the
  2.12 to the 2.16 release.

- Library Code

  - compiler-rt: Updated `lshrdi3.c` and `int_lib.h` with changes from LLVM
    master branch (r345645)
  - cpu: Updated macro that checks need for `CVE-2017-5715` mitigation
  - libc: Made setjmp and longjmp C standard compliant
  - libc: Allowed overriding the default libc (use `OVERRIDE_LIBC`)
  - libc: Moved setjmp and longjmp to the `libc/` directory

- Platforms

  - Removed Mbed TLS dependency from plat_bl_common.c

  - arm: Removed unused `ARM_MAP_BL_ROMLIB` macro

  - arm: Removed `ARM_BOARD_OPTIMISE_MEM` feature and build flag

  - arm: Moved several components into `drivers/` directory

    This affects the SDS, SCP, SCPI, MHU and SCMI components

  - arm/juno: Increased maximum BL2 image size to `0xF000`

    This change was required to accommodate a larger `libfdt` library

- SCMI

  - Optimized bakery locks when hardware-assisted coherency is enabled using the
    `HW_ASSISTED_COHERENCY` build flag

- SDEI

  - Added support for unconditionally resuming secure world execution after {{
    SDEI }} event processing completes

    {{ SDEI }} interrupts, although targeting EL3, occur on behalf of the
    non-secure world, and may have higher priority than secure world interrupts.
    Therefore they might preempt secure execution and yield execution to the
    non-secure {{ SDEI }} handler. Upon completion of {{ SDEI }} event handling,
    resume secure execution if it was preempted.

- Translation Tables (XLAT)

  - Dynamically detect need for `Common not Private (TTBRn_ELx.CnP)` bit

    Properly handle the case where `ARMv8.2-TTCNP` is implemented in a CPU that
    does not implement all mandatory v8.2 features (and so must claim to
    implement a lower architecture version).

### Resolved Issues

- Architecture
  - Incorrect check for SSBS feature detection
  - Unintentional register clobber in AArch32 reset_handler function
- Build System
  - Dependency issue during DTB image build
  - Incorrect variable expansion in Arm platform makefiles
  - Building on Windows with verbose mode (`V=1`) enabled is broken
  - AArch32 compilation flags is missing `$(march32-directive)`
- BL-Specific Issues
  - bl2: `uintptr_t is not defined` error when `BL2_IN_XIP_MEM` is defined
  - bl2: Missing prototype warning in `bl2_arch_setup`
  - bl31: Omission of Global Offset Table (GOT) section
- Code Quality Issues
  - Multiple MISRA compliance issues
  - Potential NULL pointer dereference (Coverity-detected)
- Drivers
  - mmc: Local declaration of `scr` variable causes a cache issue when
    invalidating after the read DMA transfer completes
  - mmc: `ACMD41` does not send voltage information during initialization,
    resulting in the command being treated as a query. This prevents the command
    from initializing the controller.
  - mmc: When checking device state using `mmc_device_state()` there are no
    retries attempted in the event of an error
  - ccn: Incorrect Region ID calculation for RN-I nodes
  - console: `Fix MULTI_CONSOLE_API` when used as a crash console
  - partition: Improper NULL checking in gpt.c
  - partition: Compilation failure in `VERBOSE` mode (`V=1`)
- Library Code
  - common: Incorrect check for Address Authentication support

  - xlat: Fix XLAT_V1 / XLAT_V2 incompatibility

    The file `arm_xlat_tables.h` has been renamed to `xlat_tables_compat.h` and
    has been moved to a common folder. This header can be used to guarantee
    compatibility, as it includes the correct header based on
    `XLAT_TABLES_LIB_V2`.

  - xlat: armclang unused-function warning on `xlat_clean_dcache_range`

  - xlat: Invalid `mm_cursor` checks in `mmap_add` and `mmap_add_ctx`

  - sdei: Missing `context.h` header
- Platforms
  - common: Missing prototype warning for `plat_log_get_prefix`

  - arm: Insufficient maximum BL33 image size

  - arm: Potential memory corruption during BL2-BL31 transition

    On Arm platforms, the BL2 memory can be overlaid by BL31/BL32. The memory
    descriptors describing the list of executable images are created in BL2 R/W
    memory, which could be possibly corrupted later on by BL31/BL32 due to
    overlay. This patch creates a reserved location in SRAM for these
    descriptors and are copied over by BL2 before handing over to next BL image.

  - juno: Invalid behaviour when `CSS_USE_SCMI_SDS_DRIVER` is not set

    In `juno_pm.c` the `css_scmi_override_pm_ops` function was used regardless
    of whether the build flag was set. The original behaviour has been restored
    in the case where the build flag is not set.
- Tools
  - fiptool: Incorrect UUID parsing of blob parameters
  - doimage: Incorrect object rules in Makefile

### Deprecations

- Common Code
  - `plat_crash_console_init` function
  - `plat_crash_console_putc` function
  - `plat_crash_console_flush` function
  - `finish_console_register` macro
- AArch64-specific Code
  - helpers: `get_afflvl_shift`
  - helpers: `mpidr_mask_lower_afflvls`
  - helpers: `eret`
- Secure Partition Manager (SPM)
  - Boot-info structure

### Known Issues

- Build System Issues
  - dtb: DTB creation not supported when building on a Windows host.

    This step in the build process is skipped when running on a Windows host. A
    known issue from the 1.6 release.
- Platform Issues
  - arm/juno: System suspend from Linux does not function as documented in the
    user guide

    Following the instructions provided in the user guide document does not
    result in the platform entering system suspend state as expected. A message
    relating to the hdlcd driver failing to suspend will be emitted on the Linux
    terminal.

  - arm/juno: The firmware update use-cases do not work with motherboard
    firmware version \< v1.5.0 (the reset reason is not preserved). The Linaro
    18.04 release has MB v1.4.9. The MB v1.5.0 is available in Linaro 18.10
    release.

  - mediatek/mt6795: This platform does not build in this release

## [2.0.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v1.6..refs/tags/v2.0) (2018-10-02)

### New Features

- Removal of a number of deprecated APIs

  - A new Platform Compatibility Policy document has been created which
    references a wiki page that maintains a listing of deprecated interfaces and
    the release after which they will be removed.
  - All deprecated interfaces except the MULTI_CONSOLE_API have been removed
    from the code base.
  - Various Arm and partner platforms have been updated to remove the use of
    removed APIs in this release.
  - This release is otherwise unchanged from 1.6 release

### Issues resolved since last release

- No issues known at 1.6 release resolved in 2.0 release

### Known Issues

- DTB creation not supported when building on a Windows host. This step in the
  build process is skipped when running on a Windows host. Known issue from 1.6
  version.
- As a result of removal of deprecated interfaces the Nvidia Tegra, Marvell
  Armada 8K and MediaTek MT6795 platforms do not build in this release. Also
  MediaTek MT8173, NXP QorIQ LS1043A, NXP i.MX8QX, NXP i.MX8QMa, Rockchip
  RK3328, Rockchip RK3368 and Rockchip RK3399 platforms have not been confirmed
  to be working after the removal of the deprecated interfaces although they do
  build.

## [1.6.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v1.5..refs/tags/v1.6) (2018-09-21)

### New Features

- Addressing Speculation Security Vulnerabilities

  - Implement static workaround for CVE-2018-3639 for AArch32 and AArch64
  - Add support for dynamic mitigation for CVE-2018-3639
  - Implement dynamic mitigation for CVE-2018-3639 on Cortex-A76
  - Ensure {{ SDEI }} handler executes with CVE-2018-3639 mitigation enabled

- Introduce RAS handling on AArch64

  - Some RAS extensions are mandatory for Armv8.2 CPUs, with others mandatory
    for Armv8.4 CPUs however, all extensions are also optional extensions to the
    base Armv8.0 architecture.
  - The Armv8 RAS Extensions introduced Standard Error Records which are a set
    of standard registers to configure RAS node policy and allow RAS Nodes to
    record and expose error information for error handling agents.
  - Capabilities are provided to support RAS Node enumeration and iteration
    along with individual interrupt registrations and fault injections support.
  - Introduce handlers for Uncontainable errors, Double Faults and EL3 External
    Aborts

- Enable Memory Partitioning And Monitoring (MPAM) for lower EL's

  - Memory Partitioning And Monitoring is an Armv8.4 feature that enables
    various memory system components and resources to define partitions.
    Software running at various ELs can then assign themselves to the desired
    partition to control their performance aspects.
  - When ENABLE_MPAM_FOR_LOWER_ELS is set to 1, EL3 allows lower ELs to access
    their own MPAM registers without trapping to EL3. This patch however,
    doesn't make use of partitioning in EL3; platform initialisation code should
    configure and use partitions in EL3 if required.

- Introduce ROM Lib Feature

  - Support combining several libraries into a self-called "romlib" image, that
    may be shared across images to reduce memory footprint. The romlib image is
    stored in ROM but is accessed through a jump-table that may be stored in
    read-write memory, allowing for the library code to be patched.

- Introduce Backtrace Feature

  - This function displays the backtrace, the current EL and security state to
    allow a post-processing tool to choose the right binary to interpret the
    dump.
  - Print backtrace in assert() and panic() to the console.

- Code hygiene changes and alignment with MISRA C-2012 guideline with fixes
  addressing issues complying to the following rules:

  - MISRA rules 4.9, 5.1, 5.3, 5.7, 8.2-8.5, 8.8, 8.13, 9.3, 10.1, 10.3-10.4,
    10.8, 11.3, 11.6, 12.1, 14.4, 15.7, 16.1-16.7, 17.7-17.8, 20.7, 20.10,
    20.12, 21.1, 21.15, 22.7
  - Clean up the usage of void pointers to access symbols
  - Increase usage of static qualifier to locally used functions and data
  - Migrated to use of u_register_t for register read/write to better match
    AArch32 and AArch64 type sizes
  - Use int-ll64 for both AArch32 and AArch64 to assist in consistent format
    strings between architectures
  - Clean up TF-A libc by removing non arm copyrighted implementations and
    replacing them with modified FreeBSD and SCC implementations

- Various changes to support Clang linker and assembler

  - The clang assembler/preprocessor is used when Clang is selected. However,
    the clang linker is not used because it is unable to link TF-A objects due
    to immaturity of clang linker functionality at this time.

- Refactor support APIs into Libraries

  - Evolve libfdt, mbed TLS library and standard C library sources as proper
    libraries that TF-A may be linked against.

- CPU Enhancements

  - Add CPU support for Cortex-Ares and Cortex-A76
  - Add AMU support for Cortex-Ares
  - Add initial CPU support for Cortex-Deimos
  - Add initial CPU support for Cortex-Helios
  - Implement dynamic mitigation for CVE-2018-3639 on Cortex-A76
  - Implement Cortex-Ares erratum 1043202 workaround
  - Implement DSU erratum 936184 workaround
  - Check presence of fix for errata 843419 in Cortex-A53
  - Check presence of fix for errata 835769 in Cortex-A53

- Translation Tables Enhancements

  - The xlat v2 library has been refactored in order to be reused by different
    TF components at different EL's including the addition of EL2. Some
    refactoring to make the code more generic and less specific to TF, in order
    to reuse the library outside of this project.

- SPM Enhancements

  - General cleanups and refactoring to pave the way to multiple partitions
    support

- SDEI Enhancements

  - Allow platforms to define explicit events
  - Determine client EL from NS context's SCR_EL3
  - Make dispatches synchronous
  - Introduce jump primitives for BL31
  - Mask events after CPU wakeup in {{ SDEI }} dispatcher to conform to the
    specification

- Misc TF-A Core Common Code Enhancements

  - Add support for eXecute In Place (XIP) memory in BL2
  - Add support for the SMC Calling Convention 2.0
  - Introduce External Abort handling on AArch64 External Abort routed to EL3
    was reported as an unhandled exception and caused a panic. This change
    enables Trusted Firmware-A to handle External Aborts routed to EL3.
  - Save value of ACTLR_EL1 implementation-defined register in the CPU context
    structure rather than forcing it to 0.
  - Introduce ARM_LINUX_KERNEL_AS_BL33 build option, which allows BL31 to
    directly jump to a Linux kernel. This makes for a quicker and simpler boot
    flow, which might be useful in some test environments.
  - Add dynamic configurations for BL31, BL32 and BL33 enabling support for
    Chain of Trust (COT).
  - Make TF UUID RFC 4122 compliant

- New Platform Support

  - Arm SGI-575
  - Arm SGM-775
  - Allwinner sun50i_64
  - Allwinner sun50i_h6
  - NXP QorIQ LS1043A
  - NXP i.MX8QX
  - NXP i.MX8QM
  - NXP i.MX7Solo WaRP7
  - TI K3
  - Socionext Synquacer SC2A11
  - Marvell Armada 8K
  - STMicroelectronics STM32MP1

- Misc Generic Platform Common Code Enhancements

  - Add MMC framework that supports both eMMC and SD card devices

- Misc Arm Platform Common Code Enhancements

  - Demonstrate PSCI MEM_PROTECT from el3_runtime
  - Provide RAS support
  - Migrate AArch64 port to the multi console driver. The old API is deprecated
    and will eventually be removed.
  - Move BL31 below BL2 to enable BL2 overlay resulting in changes in the layout
    of BL images in memory to enable more efficient use of available space.
  - Add cpp build processing for dtb that allows processing device tree with
    external includes.
  - Extend FIP io driver to support multiple FIP devices
  - Add support for SCMI AP core configuration protocol v1.0
  - Use SCMI AP core protocol to set the warm boot entrypoint
  - Add support to Mbed TLS drivers for shared heap among different BL images to
    help optimise memory usage
  - Enable non-secure access to UART1 through a build option to support a serial
    debug port for debugger connection

- Enhancements for Arm Juno Platform

  - Add support for TrustZone Media Protection 1 (TZMP1)

- Enhancements for Arm FVP Platform

  - Dynamic_config: remove the FVP dtb files
  - Set DYNAMIC_WORKAROUND_CVE_2018_3639=1 on FVP by default
  - Set the ability to dynamically disable Trusted Boot Board authentication to
    be off by default with DYN_DISABLE_AUTH
  - Add librom enhancement support in FVP
  - Support shared Mbed TLS heap between BL1 and BL2 that allow a reduction in
    BL2 size for FVP

- Enhancements for Arm SGI/SGM Platform

  - Enable ARM_PLAT_MT flag for SGI-575
  - Add dts files to enable support for dynamic config
  - Add RAS support
  - Support shared Mbed TLS heap for SGI and SGM between BL1 and BL2

- Enhancements for Non Arm Platforms

  - Raspberry Pi Platform
  - Hikey Platforms
  - Xilinx Platforms
  - QEMU Platform
  - Rockchip rk3399 Platform
  - TI Platforms
  - Socionext Platforms
  - Allwinner Platforms
  - NXP Platforms
  - NVIDIA Tegra Platform
  - Marvell Platforms
  - STMicroelectronics STM32MP1 Platform

### Issues resolved since last release

- No issues known at 1.5 release resolved in 1.6 release

### Known Issues

- DTB creation not supported when building on a Windows host. This step in the
  build process is skipped when running on a Windows host. Known issue from 1.5
  version.

## [1.5.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v1.4..refs/tags/v1.5) (2018-03-20)

### New features

- Added new firmware support to enable RAS (Reliability, Availability, and
  Serviceability) functionality.

  - Secure Partition Manager (SPM): A Secure Partition is a software execution
    environment instantiated in S-EL0 that can be used to implement simple
    management and security services. The SPM is the firmware component that is
    responsible for managing a Secure Partition.

  - SDEI dispatcher: Support for interrupt-based {{ SDEI }} events and all
    interfaces as defined by the {{ SDEI }} specification v1.0, see
    [SDEI Specification]

  - Exception Handling Framework (EHF): Framework that allows dispatching of EL3
    interrupts to their registered handlers which are registered based on their
    priorities. Facilitates firmware-first error handling policy where
    asynchronous exceptions may be routed to EL3.

    Integrated the TSPD with EHF.

- Updated PSCI support:

  - Implemented PSCI v1.1 optional features `MEM_PROTECT` and `SYSTEM_RESET2`.
    The supported PSCI version was updated to v1.1.

  - Improved PSCI STAT timestamp collection, including moving accounting for
    retention states to be inside the locks and fixing handling of wrap-around
    when calculating residency in AArch32 execution state.

  - Added optional handler for early suspend that executes when suspending to a
    power-down state and with data caches enabled.

    This may provide a performance improvement on platforms where it is safe to
    perform some or all of the platform actions from `pwr_domain_suspend` with
    the data caches enabled.

- Enabled build option, BL2_AT_EL3, for BL2 to allow execution at EL3 without
  any dependency on TF BL1.

  This allows platforms which already have a non-TF Boot ROM to directly load
  and execute BL2 and subsequent BL stages without need for BL1. This was not
  previously possible because BL2 executes at S-EL1 and cannot jump straight to
  EL3.

- Implemented support for SMCCC v1.1, including `SMCCC_VERSION` and
  `SMCCC_ARCH_FEATURES`.

  Additionally, added support for `SMCCC_VERSION` in PSCI features to enable
  discovery of the SMCCC version via PSCI feature call.

- Added Dynamic Configuration framework which enables each of the boot loader
  stages to be dynamically configured at runtime if required by the platform.
  The boot loader stage may optionally specify a firmware configuration file
  and/or hardware configuration file that can then be shared with the next boot
  loader stage.

  Introduced a new BL handover interface that essentially allows passing of 4
  arguments between the different BL stages.

  Updated cert_create and fip_tool to support the dynamic configuration files.
  The COT also updated to support these new files.

- Code hygiene changes and alignment with MISRA guideline:

  - Fix use of undefined macros.
  - Achieved compliance with Mandatory MISRA coding rules.
  - Achieved compliance for following Required MISRA rules for the default build
    configurations on FVP and Juno platforms : 7.3, 8.3, 8.4, 8.5 and 8.8.

- Added support for Armv8.2-A architectural features:

  - Updated translation table set-up to set the CnP (Common not Private) bit for
    secure page tables so that multiple PEs in the same Inner Shareable domain
    can use the same translation table entries for a given stage of translation
    in a particular translation regime.
  - Extended the supported values of ID_AA64MMFR0_EL1.PARange to include the
    52-bit Physical Address range.
  - Added support for the Scalable Vector Extension to allow Normal world
    software to access SVE functionality but disable access to SVE, SIMD and
    floating point functionality from the Secure world in order to prevent
    corruption of the Z-registers.

- Added support for Armv8.4-A architectural feature Activity Monitor Unit (AMU)

  extensions.

  In addition to the v8.4 architectural extension, AMU support on Cortex-A75 was
  implemented.

- Enhanced OP-TEE support to enable use of pageable OP-TEE image. The Arm
  standard platforms are updated to load up to 3 images for OP-TEE; header,
  pager image and paged image.

  The chain of trust is extended to support the additional images.

- Enhancements to the translation table library:

  - Introduced APIs to get and set the memory attributes of a region.
  - Added support to manage both privilege levels in translation regimes that
    describe translations for 2 Exception levels, specifically the EL1&0
    translation regime, and extended the memory map region attributes to include
    specifying Non-privileged access.
  - Added support to specify the granularity of the mappings of each region, for
    instance a 2MB region can be specified to be mapped with 4KB page tables
    instead of a 2MB block.
  - Disabled the higher VA range to avoid unpredictable behaviour if there is an
    attempt to access addresses in the higher VA range.
  - Added helpers for Device and Normal memory MAIR encodings that align with
    the Arm Architecture Reference Manual for Armv8-A (Arm DDI0487B.b).
  - Code hygiene including fixing type length and signedness of constants,
    refactoring of function to enable the MMU, removing all instances where the
    virtual address space is hardcoded and added comments that document
    alignment needed between memory attributes and attributes specified in
    TCR_ELx.

- Updated GIC support:

  - Introduce new APIs for GICv2 and GICv3 that provide the capability to
    specify interrupt properties rather than list of interrupt numbers alone.
    The Arm platforms and other upstream platforms are migrated to use interrupt
    properties.

  - Added helpers to save / restore the GICv3 context, specifically the
    Distributor and Redistributor contexts and architectural parts of the ITS
    power management. The Distributor and Redistributor helpers also support the
    implementation-defined part of GIC-500 and GIC-600.

    Updated the Arm FVP platform to save / restore the GICv3 context on system
    suspend / resume as an example of how to use the helpers.

    Introduced a new TZC secured DDR carve-out for use by Arm platforms for
    storing EL3 runtime data such as the GICv3 register context.

- Added support for Armv7-A architecture via build option ARM_ARCH_MAJOR=7. This
  includes following features:

  - Updates GICv2 driver to manage GICv1 with security extensions.
  - Software implementation for 32bit division.
  - Enabled use of generic timer for platforms that do not set
    ARM_CORTEX_Ax=yes.
  - Support for Armv7-A Virtualization extensions \[DDI0406C_C\].
  - Support for both Armv7-A platforms that only have 32-bit addressing and
    Armv7-A platforms that support large page addressing.
  - Included support for following Armv7 CPUs: Cortex-A12, Cortex-A17,
    Cortex-A7, Cortex-A5, Cortex-A9, Cortex-A15.
  - Added support in QEMU for Armv7-A/Cortex-A15.

- Enhancements to Firmware Update feature:

  - Updated the FWU documentation to describe the additional images needed for
    Firmware update, and how they are used for both the Juno platform and the
    Arm FVP platforms.

- Enhancements to Trusted Board Boot feature:

  - Added support to cert_create tool for RSA PKCS1# v1.5 and SHA384, SHA512 and
    SHA256.
  - For Arm platforms added support to use ECDSA keys.
  - Enhanced the mbed TLS wrapper layer to include support for both RSA and
    ECDSA to enable runtime selection between RSA and ECDSA keys.

- Added support for secure interrupt handling in AArch32 sp_min, hardcoded to
  only handle FIQs.

- Added support to allow a platform to load images from multiple boot sources,
  for example from a second flash drive.

- Added a logging framework that allows platforms to reduce the logging level at
  runtime and additionally the prefix string can be defined by the platform.

- Further improvements to register initialisation:

  - Control register PMCR_EL0 / PMCR is set to prohibit cycle counting in the
    secure world. This register is added to the list of registers that are saved
    and restored during world switch.
  - When EL3 is running in AArch32 execution state, the Non-secure version of
    SCTLR is explicitly initialised during the warmboot flow rather than relying
    on the hardware to set the correct reset values.

- Enhanced support for Arm platforms:

  - Introduced driver for Shared-Data-Structure (SDS) framework which is used
    for communication between SCP and the AP CPU, replacing Boot-Over_MHU (BOM)
    protocol.

    The Juno platform is migrated to use SDS with the SCMI support added in v1.3
    and is set as default.

    The driver can be found in the plat/arm/css/drivers folder.

  - Improved memory usage by only mapping TSP memory region when the TSPD has
    been included in the build. This reduces the memory footprint and avoids
    unnecessary memory being mapped.

  - Updated support for multi-threading CPUs for FVP platforms - always check
    the MT field in MPDIR and access the bit fields accordingly.

  - Support building for platforms that model DynamIQ configuration by
    implementing all CPUs in a single cluster.

  - Improved nor flash driver, for instance clearing status registers before
    sending commands. Driver can be found plat/arm/board/common folder.

- Enhancements to QEMU platform:

  - Added support for TBB.
  - Added support for using OP-TEE pageable image.
  - Added support for LOAD_IMAGE_V2.
  - Migrated to use translation table library v2 by default.
  - Added support for SEPARATE_CODE_AND_RODATA.

- Applied workarounds CVE-2017-5715 on Arm Cortex-A57, -A72, -A73 and -A75, and
  for Armv7-A CPUs Cortex-A9, -A15 and -A17.

- Applied errata workaround for Arm Cortex-A57: 859972.

- Applied errata workaround for Arm Cortex-A72: 859971.

- Added support for Poplar 96Board platform.

- Added support for Raspberry Pi 3 platform.

- Added Call Frame Information (CFI) assembler directives to the vector entries
  which enables debuggers to display the backtrace of functions that triggered a
  synchronous abort.

- Added ability to build dtb.

- Added support for pre-tool (cert_create and fiptool) image processing enabling
  compression of the image files before processing by cert_create and fiptool.

  This can reduce fip size and may also speed up loading of images. The image
  verification will also get faster because certificates are generated based on
  compressed images.

  Imported zlib 1.2.11 to implement gunzip() for data compression.

- Enhancements to fiptool:

  - Enabled the fiptool to be built using Visual Studio.
  - Added padding bytes at the end of the last image in the fip to be facilitate
    transfer by DMA.

### Issues resolved since last release

- TF-A can be built with optimisations disabled (-O0).
- Memory layout updated to enable Trusted Board Boot on Juno platform when
  running TF-A in AArch32 execution mode (resolving [tf-issue#501]).

### Known Issues

- DTB creation not supported when building on a Windows host. This step in the
  build process is skipped when running on a Windows host.

## [1.4.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v1.3..refs/tags/v1.4) (2017-07-07)

### New features

- Enabled support for platforms with hardware assisted coherency.

  A new build option HW_ASSISTED_COHERENCY allows platforms to take advantage of
  the following optimisations:

  - Skip performing cache maintenance during power-up and power-down.
  - Use spin-locks instead of bakery locks.
  - Enable data caches early on warm-booted CPUs.

- Added support for Cortex-A75 and Cortex-A55 processors.

  Both Cortex-A75 and Cortex-A55 processors use the Arm DynamIQ Shared Unit
  (DSU). The power-down and power-up sequences are therefore mostly managed in
  hardware, reducing complexity of the software operations.

- Introduced Arm GIC-600 driver.

  Arm GIC-600 IP complies with Arm GICv3 architecture. For FVP platforms, the
  GIC-600 driver is chosen when FVP_USE_GIC_DRIVER is set to FVP_GIC600.

- Updated GICv3 support:

  - Introduced power management APIs for GICv3 Redistributor. These APIs allow
    platforms to power down the Redistributor during CPU power on/off. Requires
    the GICv3 implementations to have power management operations.

    Implemented the power management APIs for FVP.

  - GIC driver data is flushed by the primary CPU so that secondary CPU do not
    read stale GIC data.

- Added support for Arm System Control and Management Interface v1.0 (SCMI).

  The SCMI driver implements the power domain management and system power
  management protocol of the SCMI specification (Arm DEN 0056ASCMI) for
  communicating with any compliant power controller.

  Support is added for the Juno platform. The driver can be found in the
  plat/arm/css/drivers folder.

- Added support to enable pre-integration of TBB with the Arm TrustZone
  CryptoCell product, to take advantage of its hardware Root of Trust and crypto
  acceleration services.

- Enabled Statistical Profiling Extensions for lower ELs.

  The firmware support is limited to the use of SPE in the Non-secure state and
  accesses to the SPE specific registers from S-EL1 will trap to EL3.

  The SPE are architecturally specified for AArch64 only.

- Code hygiene changes aligned with MISRA guidelines:

  - Fixed signed / unsigned comparison warnings in the translation table
    library.
  - Added U(\_x) macro and together with the existing ULL(\_x) macro fixed some
    of the signed-ness defects flagged by the MISRA scanner.

- Enhancements to Firmware Update feature:

  - The FWU logic now checks for overlapping images to prevent execution of
    unauthenticated arbitrary code.
  - Introduced new FWU_SMC_IMAGE_RESET SMC that changes the image loading state
    machine to go from COPYING, COPIED or AUTHENTICATED states to RESET state.
    Previously, this was only possible when the authentication of an image
    failed or when the execution of the image finished.
  - Fixed integer overflow which addressed TFV-1: Malformed Firmware Update SMC
    can result in copy of unexpectedly large data into secure memory.

- Introduced support for Arm Compiler 6 and LLVM (clang).

  TF-A can now also be built with the Arm Compiler 6 or the clang compilers. The
  assembler and linker must be provided by the GNU toolchain.

  Tested with Arm CC 6.7 and clang 3.9.x and 4.0.x.

- Memory footprint improvements:

  - Introduced `tf_snprintf`, a reduced version of `snprintf` which has support
    for a limited set of formats.

    The mbedtls driver is updated to optionally use `tf_snprintf` instead of
    `snprintf`.

  - The `assert()` is updated to no longer print the function name, and
    additional logging options are supported via an optional platform define
    `PLAT_LOG_LEVEL_ASSERT`, which controls how verbose the assert output is.

- Enhancements to TF-A support when running in AArch32 execution state:

  - Support booting SP_MIN and BL33 in AArch32 execution mode on Juno. Due to
    hardware limitations, BL1 and BL2 boot in AArch64 state and there is
    additional trampoline code to warm reset into SP_MIN in AArch32 execution
    state.
  - Added support for Arm Cortex-A53/57/72 MPCore processors including the
    errata workarounds that are already implemented for AArch64 execution state.
  - For FVP platforms, added AArch32 Trusted Board Boot support, including the
    Firmware Update feature.

- Introduced Arm SiP service for use by Arm standard platforms.

  - Added new Arm SiP Service SMCs to enable the Non-secure world to read PMF
    timestamps.

    Added PMF instrumentation points in TF-A in order to quantify the overall
    time spent in the PSCI software implementation.

  - Added new Arm SiP service SMC to switch execution state.

    This allows the lower exception level to change its execution state from
    AArch64 to AArch32, or vice verse, via a request to EL3.

- Migrated to use SPDX\[0\] license identifiers to make software license
  auditing simpler.

  \:::\{note} Files that have been imported by FreeBSD have not been modified.
  \:::

  \[0\]: <https://spdx.org/>

- Enhancements to the translation table library:

  - Added version 2 of translation table library that allows different
    translation tables to be modified by using different 'contexts'. Version 1
    of the translation table library only allows the current EL's translation
    tables to be modified.

    Version 2 of the translation table also added support for dynamic regions;
    regions that can be added and removed dynamically whilst the MMU is enabled.
    Static regions can only be added or removed before the MMU is enabled.

    The dynamic mapping functionality is enabled or disabled when compiling by
    setting the build option PLAT_XLAT_TABLES_DYNAMIC to 1 or 0. This can be
    done per-image.

  - Added support for translation regimes with two virtual address spaces such
    as the one shared by EL1 and EL0.

    The library does not support initializing translation tables for EL0
    software.

  - Added support to mark the translation tables as non-cacheable using an
    additional build option `XLAT_TABLE_NC`.

- Added support for GCC stack protection. A new build option
  ENABLE_STACK_PROTECTOR was introduced that enables compilation of all BL
  images with one of the GCC -fstack-protector-\* options.

  A new platform function plat_get_stack_protector_canary() was introduced that
  returns a value used to initialize the canary for stack corruption detection.
  For increased effectiveness of protection platforms must provide an
  implementation that returns a random value.

- Enhanced support for Arm platforms:

  - Added support for multi-threading CPUs, indicated by `MT` field in MPDIR. A
    new build flag `ARM_PLAT_MT` is added, and when enabled, the functions
    accessing MPIDR assume that the `MT` bit is set for the platform and access
    the bit fields accordingly.

    Also, a new API `plat_arm_get_cpu_pe_count` is added when `ARM_PLAT_MT` is
    enabled, returning the Processing Element count within the physical CPU
    corresponding to `mpidr`.

  - The Arm platforms migrated to use version 2 of the translation tables.

  - Introduced a new Arm platform layer API `plat_arm_psci_override_pm_ops`
    which allows Arm platforms to modify `plat_arm_psci_pm_ops` and therefore
    dynamically define PSCI capability.

  - The Arm platforms migrated to use IMAGE_LOAD_V2 by default.

- Enhanced reporting of errata workaround status with the following policy:

  - If an errata workaround is enabled:

    - If it applies (i.e. the CPU is affected by the errata), an INFO message is
      printed, confirming that the errata workaround has been applied.
    - If it does not apply, a VERBOSE message is printed, confirming that the
      errata workaround has been skipped.

  - If an errata workaround is not enabled, but would have applied had it been,
    a WARN message is printed, alerting that errata workaround is missing.

- Added build options ARM_ARCH_MAJOR and ARM_ARM_MINOR to choose the
  architecture version to target TF-A.

- Updated the spin lock implementation to use the more efficient CAS (Compare
  And Swap) instruction when available. This instruction was introduced in
  Armv8.1-A.

- Applied errata workaround for Arm Cortex-A53: 855873.

- Applied errata workaround for Arm-Cortex-A57: 813419.

- Enabled all A53 and A57 errata workarounds for Juno, both in AArch64 and
  AArch32 execution states.

- Added support for Socionext UniPhier SoC platform.

- Added support for Hikey960 and Hikey platforms.

- Added support for Rockchip RK3328 platform.

- Added support for NVidia Tegra T186 platform.

- Added support for Designware emmc driver.

- Imported libfdt v1.4.2 that addresses buffer overflow in fdt_offset_ptr().

- Enhanced the CPU operations framework to allow power handlers to be registered
  on per-level basis. This enables support for future CPUs that have multiple
  threads which might need powering down individually.

- Updated register initialisation to prevent unexpected behaviour:

  - Debug registers MDCR-EL3/SDCR and MDCR_EL2/HDCR are initialised to avoid
    unexpected traps into the higher exception levels and disable secure
    self-hosted debug. Additionally, secure privileged external debug on Juno is
    disabled by programming the appropriate Juno SoC registers.
  - EL2 and EL3 configurable controls are initialised to avoid unexpected traps
    in the higher exception levels.
  - Essential control registers are fully initialised on EL3 start-up, when
    initialising the non-secure and secure context structures and when preparing
    to leave EL3 for a lower EL. This gives better alignment with the Arm ARM
    which states that software must initialise RES0 and RES1 fields with 0 / 1.

- Enhanced PSCI support:

  - Introduced new platform interfaces that decouple PSCI stat residency
    calculation from PMF, enabling platforms to use alternative methods of
    capturing timestamps.
  - PSCI stat accounting performed for retention/standby states when requested
    at multiple power levels.

- Simplified fiptool to have a single linked list of image descriptors.

- For the TSP, resolved corruption of pre-empted secure context by aborting any
  pre-empted SMC during PSCI power management requests.

### Issues resolved since last release

- TF-A can be built with the latest mbed TLS version (v2.4.2). The earlier
  version 2.3.0 cannot be used due to build warnings that the TF-A build system
  interprets as errors.
- TBBR, including the Firmware Update feature is now supported on FVP platforms
  when running TF-A in AArch32 state.
- The version of the AEMv8 Base FVP used in this release has resolved the issue
  of the model executing a reset instead of terminating in response to a
  shutdown request using the PSCI SYSTEM_OFF API.

### Known Issues

- Building TF-A with compiler optimisations disabled (-O0) fails.
- Trusted Board Boot currently does not work on Juno when running Trusted
  Firmware in AArch32 execution state due to error when loading the sp_min to
  memory because of lack of free space available. See [tf-issue#501] for more
  details.
- The errata workaround for A53 errata 843419 is only available from binutils
  2.26 and is not present in GCC4.9. If this errata is applicable to the
  platform, please use GCC compiler version of at least 5.0. See [PR#1002] for
  more details.

## [1.3.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v1.2..refs/tags/v1.3) (2016-10-13)

### New features

- Added support for running TF-A in AArch32 execution state.

  The PSCI library has been refactored to allow integration with **EL3 Runtime
  Software**. This is software that is executing at the highest secure privilege
  which is EL3 in AArch64 or Secure SVC/Monitor mode in AArch32. See
  \{ref}`Porting Guide`.

  Included is a minimal AArch32 Secure Payload, **SP-MIN**, that illustrates the
  usage and integration of the PSCI library with EL3 Runtime Software running in
  AArch32 state.

  Booting to the BL1/BL2 images as well as booting straight to the Secure
  Payload is supported.

- Improvements to the initialization framework for the PSCI service and Arm
  Standard Services in general.

  The PSCI service is now initialized as part of Arm Standard Service
  initialization. This consolidates the initializations of any Arm Standard
  Service that may be added in the future.

  A new function `get_arm_std_svc_args()` is introduced to get arguments
  corresponding to each standard service and must be implemented by the EL3
  Runtime Software.

  For PSCI, a new versioned structure `psci_lib_args_t` is introduced to
  initialize the PSCI Library. **Note** this is a compatibility break due to the
  change in the prototype of `psci_setup()`.

- To support AArch32 builds of BL1 and BL2, implemented a new, alternative
  firmware image loading mechanism that adds flexibility.

  The current mechanism has a hard-coded set of images and execution order
  (BL31, BL32, etc). The new mechanism is data-driven by a list of image
  descriptors provided by the platform code.

  Arm platforms have been updated to support the new loading mechanism.

  The new mechanism is enabled by a build flag (`LOAD_IMAGE_V2`) which is
  currently off by default for the AArch64 build.

  **Note** `TRUSTED_BOARD_BOOT` is currently not supported when `LOAD_IMAGE_V2`
  is enabled.

- Updated requirements for making contributions to TF-A.

  Commits now must have a 'Signed-off-by:' field to certify that the
  contribution has been made under the terms of the
  {download}`Developer Certificate of Origin <../dco.txt>`.

  A signed CLA is no longer required.

  The {ref}`Contributor's Guide` has been updated to reflect this change.

- Introduced Performance Measurement Framework (PMF) which provides support for
  capturing, storing, dumping and retrieving time-stamps to measure the
  execution time of critical paths in the firmware. This relies on defining
  fixed sample points at key places in the code.

- To support the QEMU platform port, imported libfdt v1.4.1 from
  <https://git.kernel.org/pub/scm/utils/dtc/dtc.git>

- Updated PSCI support:

  - Added support for PSCI NODE_HW_STATE API for Arm platforms.
  - New optional platform hook, `pwr_domain_pwr_down_wfi()`, in `plat_psci_ops`
    to enable platforms to perform platform-specific actions needed to enter
    powerdown, including the 'wfi' invocation.
  - PSCI STAT residency and count functions have been added on Arm platforms by
    using PMF.

- Enhancements to the translation table library:

  - Limited memory mapping support for region overlaps to only allow regions to
    overlap that are identity mapped or have the same virtual to physical
    address offset, and overlap completely but must not cover the same area.

    This limitation will enable future enhancements without having to support
    complex edge cases that may not be necessary.

  - The initial translation lookup level is now inferred from the virtual
    address space size. Previously, it was hard-coded.

  - Added support for mapping Normal, Inner Non-cacheable, Outer Non-cacheable
    memory in the translation table library.

    This can be useful to map a non-cacheable memory region, such as a DMA
    buffer.

  - Introduced the MT_EXECUTE/MT_EXECUTE_NEVER memory mapping attributes to
    specify the access permissions for instruction execution of a memory region.

- Enabled support to isolate code and read-only data on separate memory pages,
  allowing independent access control to be applied to each.

- Enabled SCR_EL3.SIF (Secure Instruction Fetch) bit in BL1 and BL31 common
  architectural setup code, preventing fetching instructions from non-secure
  memory when in secure state.

- Enhancements to FIP support:

  - Replaced `fip_create` with `fiptool` which provides a more consistent and
    intuitive interface as well as additional support to remove an image from a
    FIP file.
  - Enabled printing the SHA256 digest with info command, allowing quick
    verification of an image within a FIP without having to extract the image
    and running sha256sum on it.
  - Added support for unpacking the contents of an existing FIP file into the
    working directory.
  - Aligned command line options for specifying images to use same naming
    convention as specified by TBBR and already used in cert_create tool.

- Refactored the TZC-400 driver to also support memory controllers that
  integrate TZC functionality, for example Arm CoreLink DMC-500. Also added
  DMC-500 specific support.

- Implemented generic delay timer based on the system generic counter and
  migrated all platforms to use it.

- Enhanced support for Arm platforms:

  - Updated image loading support to make SCP images (SCP_BL2 and SCP_BL2U)
    optional.
  - Enhanced topology description support to allow multi-cluster topology
    definitions.
  - Added interconnect abstraction layer to help platform ports select the right
    interconnect driver, CCI or CCN, for the platform.
  - Added support to allow loading BL31 in the TZC-secured DRAM instead of the
    default secure SRAM.
  - Added support to use a System Security Control (SSC) Registers Unit enabling
    TF-A to be compiled to support multiple Arm platforms and then select one at
    runtime.
  - Restricted mapping of Trusted ROM in BL1 to what is actually needed by BL1
    rather than entire Trusted ROM region.
  - Flash is now mapped as execute-never by default. This increases security by
    restricting the executable region to what is strictly needed.

- Applied following erratum workarounds for Cortex-A57: 833471, 826977, 829520,
  828024 and 826974.

- Added support for Mediatek MT6795 platform.

- Added support for QEMU virtualization Armv8-A target.

- Added support for Rockchip RK3368 and RK3399 platforms.

- Added support for Xilinx Zynq UltraScale+ MPSoC platform.

- Added support for Arm Cortex-A73 MPCore Processor.

- Added support for Arm Cortex-A72 processor.

- Added support for Arm Cortex-A35 processor.

- Added support for Arm Cortex-A32 MPCore Processor.

- Enabled preloaded BL33 alternative boot flow, in which BL2 does not load BL33
  from non-volatile storage and BL31 hands execution over to a preloaded BL33.
  The User Guide has been updated with an example of how to use this option with
  a bootwrapped kernel.

- Added support to build TF-A on a Windows-based host machine.

- Updated Trusted Board Boot prototype implementation:

  - Enabled the ability for a production ROM with TBBR enabled to boot test
    software before a real ROTPK is deployed (e.g. manufacturing mode). Added
    support to use ROTPK in certificate without verifying against the platform
    value when `ROTPK_NOT_DEPLOYED` bit is set.
  - Added support for non-volatile counter authentication to the Authentication
    Module to protect against roll-back.

- Updated GICv3 support:

  - Enabled processor power-down and automatic power-on using GICv3.
  - Enabled G1S or G0 interrupts to be configured independently.
  - Changed FVP default interrupt driver to be the GICv3-only driver. **Note**
    the default build of TF-A will not be able to boot Linux kernel with GICv2
    FDT blob.
  - Enabled wake-up from CPU_SUSPEND to stand-by by temporarily re-routing
    interrupts and then restoring after resume.

### Issues resolved since last release

### Known issues

- The version of the AEMv8 Base FVP used in this release resets the model
  instead of terminating its execution in response to a shutdown request using
  the PSCI `SYSTEM_OFF` API. This issue will be fixed in a future version of the
  model.
- Building TF-A with compiler optimisations disabled (`-O0`) fails.
- TF-A cannot be built with mbed TLS version v2.3.0 due to build warnings that
  the TF-A build system interprets as errors.
- TBBR is not currently supported when running TF-A in AArch32 state.

## [1.2.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v1.1..refs/tags/v1.2) (2015-12-22)

### New features

- The Trusted Board Boot implementation on Arm platforms now conforms to the
  mandatory requirements of the TBBR specification.

  In particular, the boot process is now guarded by a Trusted Watchdog, which
  will reset the system in case of an authentication or loading error. On Arm
  platforms, a secure instance of Arm SP805 is used as the Trusted Watchdog.

  Also, a firmware update process has been implemented. It enables authenticated
  firmware to update firmware images from external interfaces to SoC
  Non-Volatile memories. This feature functions even when the current firmware
  in the system is corrupt or missing; it therefore may be used as a recovery
  mode.

- Improvements have been made to the Certificate Generation Tool (`cert_create`)
  as follows.

  - Added support for the Firmware Update process by extending the Chain of
    Trust definition in the tool to include the Firmware Update certificate and
    the required extensions.
  - Introduced a new API that allows one to specify command line options in the
    Chain of Trust description. This makes the declaration of the tool's
    arguments more flexible and easier to extend.
  - The tool has been reworked to follow a data driven approach, which makes it
    easier to maintain and extend.

- Extended the FIP tool (`fip_create`) to support the new set of images involved
  in the Firmware Update process.

- Various memory footprint improvements. In particular:

  - The bakery lock structure for coherent memory has been optimised.
  - The mbed TLS SHA1 functions are not needed, as SHA256 is used to generate
    the certificate signature. Therefore, they have been compiled out, reducing
    the memory footprint of BL1 and BL2 by approximately 6 KB.
  - On Arm development platforms, each BL stage now individually defines the
    number of regions that it needs to map in the MMU.

- Added the following new design documents:

  - {ref}`Authentication Framework & Chain of Trust`
  - {ref}`Firmware Update (FWU)`
  - {ref}`CPU Reset`
  - {ref}`PSCI Power Domain Tree Structure`

- Applied the new image terminology to the code base and documentation, as
  described in the {ref}`Image Terminology` document.

- The build system has been reworked to improve readability and facilitate
  adding future extensions.

- On Arm standard platforms, BL31 uses the boot console during cold boot but
  switches to the runtime console for any later logs at runtime. The TSP uses
  the runtime console for all output.

- Implemented a basic NOR flash driver for Arm platforms. It programs the device
  using CFI (Common Flash Interface) standard commands.

- Implemented support for booting EL3 payloads on Arm platforms, which reduces
  the complexity of developing EL3 baremetal code by doing essential baremetal
  initialization.

- Provided separate drivers for GICv3 and GICv2. These expect the entire
  software stack to use either GICv2 or GICv3; hybrid GIC software systems are
  no longer supported and the legacy Arm GIC driver has been deprecated.

- Added support for Juno r1 and r2. A single set of Juno TF-A binaries can run
  on Juno r0, r1 and r2 boards. Note that this TF-A version depends on a Linaro
  release that does *not* contain Juno r2 support.

- Added support for MediaTek mt8173 platform.

- Implemented a generic driver for Arm CCN IP.

- Major rework of the PSCI implementation.

  - Added framework to handle composite power states.
  - Decoupled the notions of affinity instances (which describes the
    hierarchical arrangement of cores) and of power domain topology, instead of
    assuming a one-to-one mapping.
  - Better alignment with version 1.0 of the PSCI specification.

- Added support for the SYSTEM_SUSPEND PSCI API on Arm platforms. When invoked
  on the last running core on a supported platform, this puts the system into a
  low power mode with memory retention.

- Unified the reset handling code as much as possible across BL stages. Also
  introduced some build options to enable optimization of the reset path on
  platforms that support it.

- Added a simple delay timer API, as well as an SP804 timer driver, which is
  enabled on FVP.

- Added support for NVidia Tegra T210 and T132 SoCs.

- Reorganised Arm platforms ports to greatly improve code shareability and
  facilitate the reuse of some of this code by other platforms.

- Added support for Arm Cortex-A72 processor in the CPU specific framework.

- Provided better error handling. Platform ports can now define their own error
  handling, for example to perform platform specific bookkeeping or post-error
  actions.

- Implemented a unified driver for Arm Cache Coherent Interconnects used for
  both CCI-400 & CCI-500 IPs. Arm platforms ports have been migrated to this
  common driver. The standalone CCI-400 driver has been deprecated.

### Issues resolved since last release

- The Trusted Board Boot implementation has been redesigned to provide greater
  modularity and scalability. See the
  \{ref}`Authentication Framework & Chain of Trust` document. All missing
  mandatory features are now implemented.
- The FVP and Juno ports may now use the hash of the ROTPK stored in the Trusted
  Key Storage registers to verify the ROTPK. Alternatively, a development public
  key hash embedded in the BL1 and BL2 binaries might be used instead. The
  location of the ROTPK is chosen at build-time using the `ARM_ROTPK_LOCATION`
  build option.
- GICv3 is now fully supported and stable.

### Known issues

- The version of the AEMv8 Base FVP used in this release resets the model
  instead of terminating its execution in response to a shutdown request using
  the PSCI `SYSTEM_OFF` API. This issue will be fixed in a future version of the
  model.
- While this version has low on-chip RAM requirements, there are further RAM
  usage enhancements that could be made.
- The upstream documentation could be improved for structural consistency,
  clarity and completeness. In particular, the design documentation is
  incomplete for PSCI, the TSP(D) and the Juno platform.
- Building TF-A with compiler optimisations disabled (`-O0`) fails.

## [1.1.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v1.0..refs/tags/v1.1) (2015-02-04)

### New features

- A prototype implementation of Trusted Board Boot has been added. Boot loader
  images are verified by BL1 and BL2 during the cold boot path. BL1 and BL2 use
  the PolarSSL SSL library to verify certificates and images. The OpenSSL
  library is used to create the X.509 certificates. Support has been added to
  `fip_create` tool to package the certificates in a FIP.

- Support for calling CPU and platform specific reset handlers upon entry into
  BL3-1 during the cold and warm boot paths has been added. This happens after
  another Boot ROM `reset_handler()` has already run. This enables a developer
  to perform additional actions or undo actions already performed during the
  first call of the reset handlers e.g. apply additional errata workarounds.

- Support has been added to demonstrate routing of IRQs to EL3 instead of S-EL1
  when execution is in secure world.

- The PSCI implementation now conforms to version 1.0 of the PSCI specification.
  All the mandatory APIs and selected optional APIs are supported. In
  particular, support for the `PSCI_FEATURES` API has been added. A capability
  variable is constructed during initialization by examining the `plat_pm_ops`
  and `spd_pm_ops` exported by the platform and the Secure Payload Dispatcher.
  This is used by the PSCI FEATURES function to determine which PSCI APIs are
  supported by the platform.

- Improvements have been made to the PSCI code as follows.

  - The code has been refactored to remove redundant parameters from internal
    functions.
  - Changes have been made to the code for PSCI `CPU_SUSPEND`, `CPU_ON` and
    `CPU_OFF` calls to facilitate an early return to the caller in case a
    failure condition is detected. For example, a PSCI `CPU_SUSPEND` call
    returns `SUCCESS` to the caller if a pending interrupt is detected early in
    the code path.
  - Optional platform APIs have been added to validate the `power_state` and
    `entrypoint` parameters early in PSCI `CPU_ON` and `CPU_SUSPEND` code paths.
  - PSCI migrate APIs have been reworked to invoke the SPD hook to determine the
    type of Trusted OS and the CPU it is resident on (if applicable). Also,
    during a PSCI `MIGRATE` call, the SPD hook to migrate the Trusted OS is
    invoked.

- It is now possible to build TF-A without marking at least an extra page of
  memory as coherent. The build flag `USE_COHERENT_MEM` can be used to choose
  between the two implementations. This has been made possible through these
  changes.

  - An implementation of Bakery locks, where the locks are not allocated in
    coherent memory has been added.
  - Memory which was previously marked as coherent is now kept coherent through
    the use of software cache maintenance operations.

  Approximately, 4K worth of memory is saved for each boot loader stage when
  `USE_COHERENT_MEM=0`. Enabling this option increases the latencies associated
  with acquire and release of locks. It also requires changes to the platform
  ports.

- It is now possible to specify the name of the FIP at build time by defining
  the `FIP_NAME` variable.

- Issues with dependencies on the 'fiptool' makefile target have been rectified.
  The `fip_create` tool is now rebuilt whenever its source files change.

- The BL3-1 runtime console is now also used as the crash console. The crash
  console is changed to SoC UART0 (UART2) from the previous FPGA UART0 (UART0)
  on Juno. In FVP, it is changed from UART0 to UART1.

- CPU errata workarounds are applied only when the revision and part number
  match. This behaviour has been made consistent across the debug and release
  builds. The debug build additionally prints a warning if a mismatch is
  detected.

- It is now possible to issue cache maintenance operations by set/way for a
  particular level of data cache. Levels 1-3 are currently supported.

- The following improvements have been made to the FVP port.

  - The build option `FVP_SHARED_DATA_LOCATION` which allowed relocation of
    shared data into the Trusted DRAM has been deprecated. Shared data is now
    always located at the base of Trusted SRAM.
  - BL2 Translation tables have been updated to map only the region of DRAM
    which is accessible to normal world. This is the region of the 2GB DDR-DRAM
    memory at 0x80000000 excluding the top 16MB. The top 16MB is accessible to
    only the secure world.
  - BL3-2 can now reside in the top 16MB of DRAM which is accessible only to the
    secure world. This can be done by setting the build flag
    `FVP_TSP_RAM_LOCATION` to the value `dram`.

- Separate translation tables are created for each boot loader image. The
  `IMAGE_BLx` build options are used to do this. This allows each stage to
  create mappings only for areas in the memory map that it needs.

- A Secure Payload Dispatcher (OPTEED) for the OP-TEE Trusted OS has been added.
  Details of using it with TF-A can be found in {ref}`OP-TEE Dispatcher`

### Issues resolved since last release

- The Juno port has been aligned with the FVP port as follows.

  - Support for reclaiming all BL1 RW memory and BL2 memory by overlaying the
    BL3-1/BL3-2 NOBITS sections on top of them has been added to the Juno port.
  - The top 16MB of the 2GB DDR-DRAM memory at 0x80000000 is configured using
    the TZC-400 controller to be accessible only to the secure world.
  - The Arm GIC driver is used to configure the GIC-400 instead of using a GIC
    driver private to the Juno port.
  - PSCI `CPU_SUSPEND` calls that target a standby state are now supported.
  - The TZC-400 driver is used to configure the controller instead of direct
    accesses to the registers.

- The Linux kernel version referred to in the user guide has DVFS and HMP
  support enabled.

- DS-5 v5.19 did not detect Version 5.8 of the Cortex-A57-A53 Base FVPs in CADI
  server mode. This issue is not seen with DS-5 v5.20 and Version 6.2 of the
  Cortex-A57-A53 Base FVPs.

### Known issues

- The Trusted Board Boot implementation is a prototype. There are issues with
  the modularity and scalability of the design. Support for a Trusted Watchdog,
  firmware update mechanism, recovery images and Trusted debug is absent. These
  issues will be addressed in future releases.
- The FVP and Juno ports do not use the hash of the ROTPK stored in the Trusted
  Key Storage registers to verify the ROTPK in the `plat_match_rotpk()`
  function. This prevents the correct establishment of the Chain of Trust at the
  first step in the Trusted Board Boot process.
- The version of the AEMv8 Base FVP used in this release resets the model
  instead of terminating its execution in response to a shutdown request using
  the PSCI `SYSTEM_OFF` API. This issue will be fixed in a future version of the
  model.
- GICv3 support is experimental. There are known issues with GICv3
  initialization in the TF-A.
- While this version greatly reduces the on-chip RAM requirements, there are
  further RAM usage enhancements that could be made.
- The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
  its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.
- The Juno-specific firmware design documentation is incomplete.

## [1.0.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v0.4..refs/tags/v1.0) (2014-08-28)

### New features

- It is now possible to map higher physical addresses using non-flat virtual to
  physical address mappings in the MMU setup.

- Wider use is now made of the per-CPU data cache in BL3-1 to store:

  - Pointers to the non-secure and secure security state contexts.
  - A pointer to the CPU-specific operations.
  - A pointer to PSCI specific information (for example the current power
    state).
  - A crash reporting buffer.

- The following RAM usage improvements result in a BL3-1 RAM usage reduction
  from 96KB to 56KB (for FVP with TSPD), and a total RAM usage reduction across
  all images from 208KB to 88KB, compared to the previous release.

  - Removed the separate `early_exception` vectors from BL3-1 (2KB code size
    saving).
  - Removed NSRAM from the FVP memory map, allowing the removal of one (4KB)
    translation table.
  - Eliminated the internal `psci_suspend_context` array, saving 2KB.
  - Correctly dimensioned the PSCI `aff_map_node` array, saving 1.5KB in the FVP
    port.
  - Removed calling CPU mpidr from the bakery lock API, saving 160 bytes.
  - Removed current CPU mpidr from PSCI common code, saving 160 bytes.
  - Inlined the mmio accessor functions, saving 360 bytes.
  - Fully reclaimed all BL1 RW memory and BL2 memory on the FVP port by
    overlaying the BL3-1/BL3-2 NOBITS sections on top of these at runtime.
  - Made storing the FP register context optional, saving 0.5KB per context (8KB
    on the FVP port, with TSPD enabled and running on 8 CPUs).
  - Implemented a leaner `tf_printf()` function, allowing the stack to be
    greatly reduced.
  - Removed coherent stacks from the codebase. Stacks allocated in normal memory
    are now used before and after the MMU is enabled. This saves 768 bytes per
    CPU in BL3-1.
  - Reworked the crash reporting in BL3-1 to use less stack.
  - Optimized the EL3 register state stored in the `cpu_context` structure so
    that registers that do not change during normal execution are re-initialized
    each time during cold/warm boot, rather than restored from memory. This
    saves about 1.2KB.
  - As a result of some of the above, reduced the runtime stack size in all BL
    images. For BL3-1, this saves 1KB per CPU.

- PSCI SMC handler improvements to correctly handle calls from secure states and
  from AArch32.

- CPU contexts are now initialized from the `entry_point_info`. BL3-1 fully
  determines the exception level to use for the non-trusted firmware (BL3-3)
  based on the SPSR value provided by the BL2 platform code (or otherwise
  provided to BL3-1). This allows platform code to directly run non-trusted
  firmware payloads at either EL2 or EL1 without requiring an EL2 stub or OS
  loader.

- Code refactoring improvements:

  - Refactored `fvp_config` into a common platform header.
  - Refactored the fvp gic code to be a generic driver that no longer has an
    explicit dependency on platform code.
  - Refactored the CCI-400 driver to not have dependency on platform code.
  - Simplified the IO driver so it's no longer necessary to call `io_init()` and
    moved all the IO storage framework code to one place.
  - Simplified the interface the the TZC-400 driver.
  - Clarified the platform porting interface to the TSP.
  - Reworked the TSPD setup code to support the alternate BL3-2 initialization
    flow where BL3-1 generic code hands control to BL3-2, rather than expecting
    the TSPD to hand control directly to BL3-2.
  - Considerable rework to PSCI generic code to support CPU specific operations.

- Improved console log output, by:

  - Adding the concept of debug log levels.
  - Rationalizing the existing debug messages and adding new ones.
  - Printing out the version of each BL stage at runtime.
  - Adding support for printing console output from assembler code, including
    when a crash occurs before the C runtime is initialized.

- Moved up to the latest versions of the FVPs, toolchain, EDK2, kernel, Linaro
  file system and DS-5.

- On the FVP port, made the use of the Trusted DRAM region optional at build
  time (off by default). Normal platforms will not have such a "ready-to-use"
  DRAM area so it is not a good example to use it.

- Added support for PSCI `SYSTEM_OFF` and `SYSTEM_RESET` APIs.

- Added support for CPU specific reset sequences, power down sequences and
  register dumping during crash reporting. The CPU specific reset sequences
  include support for errata workarounds.

- Merged the Juno port into the master branch. Added support for CPU hotplug and
  CPU idle. Updated the user guide to describe how to build and run on the Juno
  platform.

### Issues resolved since last release

- Removed the concept of top/bottom image loading. The image loader now
  automatically detects the position of the image inside the current memory
  layout and updates the layout to minimize fragmentation. This resolves the
  image loader limitations of previously releases. There are currently no plans
  to support dynamic image loading.
- CPU idle now works on the publicized version of the Foundation FVP.
- All known issues relating to the compiler version used have now been resolved.
  This TF-A version uses Linaro toolchain 14.07 (based on GCC 4.9).

### Known issues

- GICv3 support is experimental. The Linux kernel patches to support this are
  not widely available. There are known issues with GICv3 initialization in the
  TF-A.

- While this version greatly reduces the on-chip RAM requirements, there are
  further RAM usage enhancements that could be made.

- The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
  its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.

- The Juno-specific firmware design documentation is incomplete.

- Some recent enhancements to the FVP port have not yet been translated into the
  Juno port. These will be tracked via the tf-issues project.

- The Linux kernel version referred to in the user guide has DVFS and HMP
  support disabled due to some known instabilities at the time of this release.
  A future kernel version will re-enable these features.

- DS-5 v5.19 does not detect Version 5.8 of the Cortex-A57-A53 Base FVPs in CADI
  server mode. This is because the `<SimName>` reported by the FVP in this
  version has changed. For example, for the Cortex-A57x4-A53x4 Base FVP, the
  `<SimName>` reported by the FVP is `FVP_Base_Cortex_A57x4_A53x4`, while DS-5
  expects it to be `FVP_Base_A57x4_A53x4`.

  The temporary fix to this problem is to change the name of the FVP in
  `sw/debugger/configdb/Boards/ARM FVP/Base_A57x4_A53x4/cadi_config.xml`. Change
  the following line:

  ```
  <SimName>System Generator:FVP_Base_A57x4_A53x4</SimName>
  ```

  to System Generator:FVP_Base_Cortex-A57x4_A53x4

  A similar change can be made to the other Cortex-A57-A53 Base FVP variants.

## [0.4.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v0.3..refs/tags/v0.4) (2014-06-03)

### New features

- Makefile improvements:

  - Improved dependency checking when building.
  - Removed `dump` target (build now always produces dump files).
  - Enabled platform ports to optionally make use of parts of the Trusted
    Firmware (e.g. BL3-1 only), rather than being forced to use all parts. Also
    made the `fip` target optional.
  - Specified the full path to source files and removed use of the `vpath`
    keyword.

- Provided translation table library code for potential re-use by platforms
  other than the FVPs.

- Moved architectural timer setup to platform-specific code.

- Added standby state support to PSCI cpu_suspend implementation.

- SRAM usage improvements:

  - Started using the `-ffunction-sections`, `-fdata-sections` and
    `--gc-sections` compiler/linker options to remove unused code and data from
    the images. Previously, all common functions were being built into all
    binary images, whether or not they were actually used.
  - Placed all assembler functions in their own section to allow more unused
    functions to be removed from images.
  - Updated BL1 and BL2 to use a single coherent stack each, rather than one per
    CPU.
  - Changed variables that were unnecessarily declared and initialized as
    non-const (i.e. in the .data section) so they are either uninitialized (zero
    init) or const.

- Moved the Test Secure-EL1 Payload (BL3-2) to execute in Trusted SRAM by
  default. The option for it to run in Trusted DRAM remains.

- Implemented a TrustZone Address Space Controller (TZC-400) driver. A default
  configuration is provided for the Base FVPs. This means the model parameter
  `-C bp.secure_memory=1` is now supported.

- Started saving the PSCI cpu_suspend 'power_state' parameter prior to
  suspending a CPU. This allows platforms that implement multiple power-down
  states at the same affinity level to identify a specific state.

- Refactored the entire codebase to reduce the amount of nesting in header files
  and to make the use of system/user includes more consistent. Also split
  platform.h to separate out the platform porting declarations from the required
  platform porting definitions and the definitions/declarations specific to the
  platform port.

- Optimized the data cache clean/invalidate operations.

- Improved the BL3-1 unhandled exception handling and reporting. Unhandled
  exceptions now result in a dump of registers to the console.

- Major rework to the handover interface between BL stages, in particular the
  interface to BL3-1. The interface now conforms to a specification and is more
  future proof.

- Added support for optionally making the BL3-1 entrypoint a reset handler
  (instead of BL1). This allows platforms with an alternative image loading
  architecture to re-use BL3-1 with fewer modifications to generic code.

- Reserved some DDR DRAM for secure use on FVP platforms to avoid future
  compatibility problems with non-secure software.

- Added support for secure interrupts targeting the Secure-EL1 Payload (SP)
  (using GICv2 routing only). Demonstrated this working by adding an interrupt
  target and supporting test code to the TSP. Also demonstrated non-secure
  interrupt handling during TSP processing.

### Issues resolved since last release

- Now support use of the model parameter `-C bp.secure_memory=1` in the Base
  FVPs (see **New features**).
- Support for secure world interrupt handling now available (see **New
  features**).
- Made enough SRAM savings (see **New features**) to enable the Test Secure-EL1
  Payload (BL3-2) to execute in Trusted SRAM by default.
- The tested filesystem used for this release (Linaro AArch64 OpenEmbedded
  14.04) now correctly reports progress in the console.
- Improved the Makefile structure to make it easier to separate out parts of the
  TF-A for re-use in platform ports. Also, improved target dependency checking.

### Known issues

- GICv3 support is experimental. The Linux kernel patches to support this are
  not widely available. There are known issues with GICv3 initialization in the
  TF-A.
- Dynamic image loading is not available yet. The current image loader
  implementation (used to load BL2 and all subsequent images) has some
  limitations. Changing BL2 or BL3-1 load addresses in certain ways can lead to
  loading errors, even if the images should theoretically fit in memory.
- TF-A still uses too much on-chip Trusted SRAM. A number of RAM usage
  enhancements have been identified to rectify this situation.
- CPU idle does not work on the advertised version of the Foundation FVP. Some
  FVP fixes are required that are not available externally at the time of
  writing. This can be worked around by disabling CPU idle in the Linux kernel.
- Various bugs in TF-A, UEFI and the Linux kernel have been observed when using
  Linaro toolchain versions later than 13.11. Although most of these have been
  fixed, some remain at the time of writing. These mainly seem to relate to a
  subtle change in the way the compiler converts between 64-bit and 32-bit
  values (e.g. during casting operations), which reveals previously hidden bugs
  in client code.
- The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
  its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.

## [0.3.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v0.2..refs/tags/v0.3) (2014-02-28)

### New features

- Support for Foundation FVP Version 2.0 added. The documented UEFI
  configuration disables some devices that are unavailable in the Foundation
  FVP, including MMC and CLCD. The resultant UEFI binary can be used on the
  AEMv8 and Cortex-A57-A53 Base FVPs, as well as the Foundation FVP.

  \:::\{note} The software will not work on Version 1.0 of the Foundation FVP.
  \:::

- Enabled third party contributions. Added a new contributing.md containing
  instructions for how to contribute and updated copyright text in all files to
  acknowledge contributors.

- The PSCI CPU_SUSPEND API has been stabilised to the extent where it can be
  used for entry into power down states with the following restrictions:

  - Entry into standby states is not supported.
  - The API is only supported on the AEMv8 and Cortex-A57-A53 Base FVPs.

- The PSCI AFFINITY_INFO api has undergone limited testing on the Base FVPs to
  allow experimental use.

- Required C library and runtime header files are now included locally in TF-A
  instead of depending on the toolchain standard include paths. The local
  implementation has been cleaned up and reduced in scope.

- Added I/O abstraction framework, primarily to allow generic code to load
  images in a platform-independent way. The existing image loading code has been
  reworked to use the new framework. Semi-hosting and NOR flash I/O drivers are
  provided.

- Introduced Firmware Image Package (FIP) handling code and tools. A FIP
  combines multiple firmware images with a Table of Contents (ToC) into a single
  binary image. The new FIP driver is another type of I/O driver. The Makefile
  builds a FIP by default and the FVP platform code expect to load a FIP from
  NOR flash, although some support for image loading using semi- hosting is
  retained.

  \:::\{note} Building a FIP by default is a non-backwards-compatible change. :::

  \:::\{note} Generic BL2 code now loads a BL3-3 (non-trusted firmware) image
  into DRAM instead of expecting this to be pre-loaded at known location. This
  is also a non-backwards-compatible change. :::

  \:::\{note} Some non-trusted firmware (e.g. UEFI) will need to be rebuilt so
  that it knows the new location to execute from and no longer needs to copy
  particular code modules to DRAM itself. :::

- Reworked BL2 to BL3-1 handover interface. A new composite structure
  (bl31_args) holds the superset of information that needs to be passed from BL2
  to BL3-1, including information on how handover execution control to BL3-2 (if
  present) and BL3-3 (non-trusted firmware).

- Added library support for CPU context management, allowing the saving and
  restoring of

  - Shared system registers between Secure-EL1 and EL1.
  - VFP registers.
  - Essential EL3 system registers.

- Added a framework for implementing EL3 runtime services. Reworked the PSCI
  implementation to be one such runtime service.

- Reworked the exception handling logic, making use of both SP_EL0 and SP_EL3
  stack pointers for determining the type of exception, managing general purpose
  and system register context on exception entry/exit, and handling SMCs. SMCs
  are directed to the correct EL3 runtime service.

- Added support for a Test Secure-EL1 Payload (TSP) and a corresponding
  Dispatcher (TSPD), which is loaded as an EL3 runtime service. The TSPD
  implements Secure Monitor functionality such as world switching and EL1
  context management, and is responsible for communication with the TSP.

  \:::\{note} The TSPD does not yet contain support for secure world interrupts.
  \:::

  \:::\{note} The TSP/TSPD is not built by default. :::

### Issues resolved since last release

- Support has been added for switching context between secure and normal worlds
  in EL3.
- PSCI API calls `AFFINITY_INFO` & `PSCI_VERSION` have now been tested (to a
  limited extent).
- The TF-A build artifacts are now placed in the `./build` directory and
  sub-directories instead of being placed in the root of the project.
- TF-A is now free from build warnings. Build warnings are now treated as
  errors.
- TF-A now provides C library support locally within the project to maintain
  compatibility between toolchains/systems.
- The PSCI locking code has been reworked so it no longer takes locks in an
  incorrect sequence.
- The RAM-disk method of loading a Linux file-system has been confirmed to work
  with the TF-A and Linux kernel version (based on version 3.13) used in this
  release, for both Foundation and Base FVPs.

### Known issues

The following is a list of issues which are expected to be fixed in the future
releases of TF-A.

- The TrustZone Address Space Controller (TZC-400) is not being programmed yet.
  Use of model parameter `-C bp.secure_memory=1` is not supported.
- No support yet for secure world interrupt handling.
- GICv3 support is experimental. The Linux kernel patches to support this are
  not widely available. There are known issues with GICv3 initialization in
  TF-A.
- Dynamic image loading is not available yet. The current image loader
  implementation (used to load BL2 and all subsequent images) has some
  limitations. Changing BL2 or BL3-1 load addresses in certain ways can lead to
  loading errors, even if the images should theoretically fit in memory.
- TF-A uses too much on-chip Trusted SRAM. Currently the Test Secure-EL1 Payload
  (BL3-2) executes in Trusted DRAM since there is not enough SRAM. A number of
  RAM usage enhancements have been identified to rectify this situation.
- CPU idle does not work on the advertised version of the Foundation FVP. Some
  FVP fixes are required that are not available externally at the time of
  writing.
- Various bugs in TF-A, UEFI and the Linux kernel have been observed when using
  Linaro toolchain versions later than 13.11. Although most of these have been
  fixed, some remain at the time of writing. These mainly seem to relate to a
  subtle change in the way the compiler converts between 64-bit and 32-bit
  values (e.g. during casting operations), which reveals previously hidden bugs
  in client code.
- The tested filesystem used for this release (Linaro AArch64 OpenEmbedded
  14.01) does not report progress correctly in the console. It only seems to
  produce error output, not standard output. It otherwise appears to function
  correctly. Other filesystem versions on the same software stack do not exhibit
  the problem.
- The Makefile structure doesn't make it easy to separate out parts of the TF-A
  for re-use in platform ports, for example if only BL3-1 is required in a
  platform port. Also, dependency checking in the Makefile is flawed.
- The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
  its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.

## [0.2.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4b825dc642cb6eb9a060e54bf8d69288fbee4904..refs/tags/v0.2) (2013-10-25)

### New features

- First source release.
- Code for the PSCI suspend feature is supplied, although this is not enabled by
  default since there are known issues (see below).

### Issues resolved since last release

- The "psci" nodes in the FDTs provided in this release now fully comply with
  the recommendations made in the PSCI specification.

### Known issues

The following is a list of issues which are expected to be fixed in the future
releases of TF-A.

- The TrustZone Address Space Controller (TZC-400) is not being programmed yet.
  Use of model parameter `-C bp.secure_memory=1` is not supported.
- No support yet for secure world interrupt handling or for switching context
  between secure and normal worlds in EL3.
- GICv3 support is experimental. The Linux kernel patches to support this are
  not widely available. There are known issues with GICv3 initialization in
  TF-A.
- Dynamic image loading is not available yet. The current image loader
  implementation (used to load BL2 and all subsequent images) has some
  limitations. Changing BL2 or BL3-1 load addresses in certain ways can lead to
  loading errors, even if the images should theoretically fit in memory.
- Although support for PSCI `CPU_SUSPEND` is present, it is not yet stable and
  ready for use.
- PSCI API calls `AFFINITY_INFO` & `PSCI_VERSION` are implemented but have not
  been tested.
- The TF-A make files result in all build artifacts being placed in the root of
  the project. These should be placed in appropriate sub-directories.
- The compilation of TF-A is not free from compilation warnings. Some of these
  warnings have not been investigated yet so they could mask real bugs.
- TF-A currently uses toolchain/system include files like stdio.h. It should
  provide versions of these within the project to maintain compatibility between
  toolchains/systems.
- The PSCI code takes some locks in an incorrect sequence. This may cause
  problems with suspend and hotplug in certain conditions.
- The Linux kernel used in this release is based on version 3.12-rc4. Using this
  kernel with the TF-A fails to start the file-system as a RAM-disk. It fails to
  execute user-space `init` from the RAM-disk. As an alternative, the
  VirtioBlock mechanism can be used to provide a file-system to the kernel.

______________________________________________________________________

*Copyright (c) 2013-2025, Arm Limited and Contributors. All rights reserved.*

[mbed tls releases]: https://tls.mbed.org/tech-updates/releases
[pr#1002]: https://github.com/ARM-software/arm-trusted-firmware/pull/1002#issuecomment-312650193
[sdei specification]: http://infocenter.arm.com/help/topic/com.arm.doc.den0054a/ARM_DEN0054A_Software_Delegated_Exception_Interface.pdf
[tf-issue#501]: https://github.com/ARM-software/tf-issues/issues/501

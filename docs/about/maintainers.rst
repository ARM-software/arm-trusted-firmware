Project Maintenance
===================

Trusted Firmware-A (TF-A) is an open governance community project. All
contributions are reviewed and merged by the community members listed below.

For more details on the roles of `maintainers`, `code owners` and general
information about code reviews in TF-A project, please refer to the :ref:`Code
Review Guidelines`.

.. |M| replace:: **Mail**
.. |C| replace:: **CC**
.. |F| replace:: **Files**

.. _maintainers:

Maintainers
-----------

.. note::
   If you wish to become a maintainer for TF-A project, please refer to the
   :ref:`Project Maintenance Processes`.

:|M|: Dan Handley <dan.handley@arm.com>
:|M|: Soby Mathew <soby.mathew@arm.com>
:|M|: Sandrine Afsa <sandrine.afsa@arm.com>
:|M|: Alexei Fedorov <Alexei.Fedorov@arm.com>
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|M|: Mark Dykes <mark.dykes@arm.com>
:|M|: Olivier Deprez <olivier.deprez@arm.com>
:|M|: Bipin Ravi <bipin.ravi@arm.com>
:|M|: Joanna Farley <joanna.farley@arm.com>
:|M|: Julius Werner <jwerner@chromium.org>
:|M|: Varun Wadekar <vwadekar@nvidia.com>
:|M|: Andre Przywara <andre.przywara@arm.com>
:|M|: Lauren Wehrmeister <Lauren.Wehrmeister@arm.com>
:|M|: Madhukar Pappireddy <Madhukar.Pappireddy@arm.com>
:|M|: Raghu Krishnamurthy <raghuoss@raghushome.com>
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|M|: Yann Gautier <yann.gautier@st.com>
:|M|: Govindraj Raja <govindraj.raja@arm.com>
:|M|: Chris Kay <chris.kay@arm.com>
:|M|: Boyan Karatotev <boyan.karatotev@arm.com>
:|M|: Harrison Mutai <harrison.mutai@arm.com>

.. _lts maintainers:

LTS Maintainers
---------------

:|M|: Olivier Deprez <olivier.deprez@arm.com>

v2.14 branch
~~~~~~~~~~~~
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>

v2.12 branch
~~~~~~~~~~~~
:|M|: Jamie Fox <jamiefox@google.com>

v2.10 branch
~~~~~~~~~~~~
:|M|: Yann Gautier <yann.gautier@st.com>

v2.8 branch
~~~~~~~~~~~~
:|M|: Varun Wadekar <vwadekar@nvidia.com>

.. _code owners:

Code owners
-----------

This section is the authoritative map of who is a code owner for any particular
file. Gerrit will notify people listed under these rules whenever a patch
modifies a relevant file. Entries are either files, directories, or regular
expressions.

Please use ``scripts/generate_reviewers_config.py`` whenever modifying this list
to sync it to Gerrit's configuration.

Common Code
~~~~~~~~~~~

Armv7-A architecture port
^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Etienne Carriere <etienne.carriere@linaro.org>

Common platform code
^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|M|: Harrison Mutai <harrison.mutai@arm.com>
:|F|: plat/common/
:|F|: include/plat/common/

Software Delegated Exception Interface (SDEI)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jayanth Dodderi Chidanand <jayanthdodderi.chidanand@arm.com>
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|F|: services/std_svc/sdei/
:|F|: include/services/sdei.h
:|F|: include/services/sdei_flags.h

Trusted Boot
^^^^^^^^^^^^
:|M|: Sandrine Afsa <sandrine.afsa@arm.com>
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: drivers/auth/
:|F|: include/drivers/auth/

Exception Handling Framework (EHF)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|M|: Boyan Karatotev <boyan.karatotev@arm.com>
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: bl31/ehf.c
:|F|: include/bl31/ehf.h

Runtime Exceptions and Interrupt Management
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|M|: Boyan Karatotev <boyan.karatotev@arm.com>
:|F|: bl31/aarch64/
:|F|: bl31/interrupt_mgmt.c
:|F|: include/bl31/interrupt_mgmt.h

Drivers, Libraries and Framework Code
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Arm CPU libraries
^^^^^^^^^^^^^^^^^
:|M|: Boyan Karatotev <boyan.karatotev@arm.com>
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|F|: lib/cpus/
:|F|: include/lib/cpus/

Arm® Ethos™-N NPU driver
^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Joshua Slater <joshua.slater@arm.com>
:|M|: Ştefana Simion <stefana.simion@arm.com>
:|F|: drivers/arm/ethosn/
:|F|: include/drivers/arm/ethosn.h
:|F|: include/drivers/arm/ethosn_cert.h
:|F|: include/drivers/arm/ethosn_fip.h
:|F|: include/drivers/arm/ethosn_oid.h
:|F|: plat/arm/board/juno/juno_ethosn_tzmp1_def.h
:|F|: plat/arm/common/fconf/fconf_ethosn_getter.c
:|F|: include/plat/arm/common/fconf_ethosn_getter.h
:|F|: fdts/juno-ethosn.dtsi

Clock Framework
^^^^^^^^^^^^^^^
:|M|: Gabriel Fernandez <gabriel.fernandez@st.com>
:|F|: drivers/clk
:|F|: include/drivers/clk.h

Console API framework
^^^^^^^^^^^^^^^^^^^^^
:|M|: Julius Werner <jwerner@chromium.org>
:|F|: drivers/console/
:|F|: include/drivers/console.h
:|F|: include/drivers/console_assertions.h
:|F|: plat/common/aarch64/crash_console_helpers.S

Context Management
^^^^^^^^^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|M|: Boyan Karatotev <boyan.karatotev@arm.com>
:|F|: bl1/aarch32/bl1_context_mgmt.c
:|F|: bl1/aarch64/bl1_context_mgmt.c
:|F|: bl31/bl31_context_mgmt.c
:|F|: lib/el3_runtime/
:|F|: include/lib/el3_runtime/
:|F|: lib/extensions/
:|F|: include/lib/extensions/
:|F|: lib/locks/

Reliability Availability Serviceabilty (RAS) framework
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|M|: Olivier Deprez <olivier.deprez@arm.com>
:|M|: Boyan Karatotev <boyan.karatotev@arm.com>
:|F|: lib/extensions/ras/
:|F|: include/lib/extensions/ras.h
:|F|: include/lib/extensions/ras_arch.h

coreboot support libraries
^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Julius Werner <jwerner@chromium.org>
:|F|: drivers/coreboot/
:|F|: include/drivers/coreboot/
:|F|: include/lib/coreboot.h
:|F|: lib/coreboot/

DebugFS
^^^^^^^
:|M|: Olivier Deprez <olivier.deprez@arm.com>
:|F|: lib/debugfs/
:|F|: include/lib/debugfs.h

DRTM
^^^^
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|F|: services/std_svc/drtm
:|F|: include/plat/common/plat_drtm.h
:|F|: include/services/drtm_svc.h

eMMC/UFS drivers
^^^^^^^^^^^^^^^^
:|M|: Haojian Zhuang <haojian.zhuang@linaro.org>
:|F|: drivers/partition/
:|F|: include/drivers/partition/
:|F|: drivers/synopsys/emmc/
:|F|: drivers/synopsys/ufs/
:|F|: drivers/ufs/
:|F|: include/drivers/dw_ufs.h
:|F|: include/drivers/ufs.h
:|F|: include/drivers/synopsys/dw_mmc.h

Errata Management
^^^^^^^^^^^^^^^^^
:|M|: Boyan Karatotev <boyan.karatotev@arm.com>
:|F|: services/std_svc/errata_abi/
:|F|: include/services/errata_abi_svc.h

Firmware Configuration Framework (FCONF)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: lib/fconf/
:|F|: plat/arm/common/fconf/
:|F|: include/lib/fconf/
:|F|: include/plat/arm/common/arm_fconf\_.\*
:|F|: include/plat/arm/common/fconf\_.\*

Firmware Encryption Framework
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Sumit Garg <sumit.garg@kernel.org>
:|F|: drivers/io/io_encrypted.c
:|F|: include/drivers/io/io_encrypted.h
:|F|: include/tools_share/firmware_encrypted.h

Firmware Handoff Library (Transfer List)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Raymond Mao <raymond.mao@linaro.org>
:|M|: Harrison Mutai <harrison.mutai@arm.com>
:|F|: lib/transfer_list
:|F|: include/lib/transfer_list.h

GIC driver
^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|M|: Olivier Deprez <olivier.deprez@arm.com>
:|M|: Boyan Karatotev <boyan.karatotev@arm.com>
:|F|: drivers/arm/gic/
:|F|: include/drivers/arm/gic.h
:|F|: include/drivers/arm/gic600_multichip.h
:|F|: include/drivers/arm/gic600ae_fmu.h
:|F|: include/drivers/arm/gic_common.h
:|F|: include/drivers/arm/arm_gicv3_common.h
:|F|: include/drivers/arm/gicv2.h
:|F|: include/drivers/arm/gicv3.h

GICv5 driver
^^^^^^^^^^^^
:|M|: Boyan Karatotev <boyan.karatotev@arm.com>
:|F|: drivers/arm/gicv5
:|F|: include/drivers/arm/gicv5.h
:|F|: plat/common/plat_gicv5.c
:|F|: plat/arm/board/fvp/gicv5_config.yaml

Granule Protection Tables Library (GPT-RME)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Soby Mathew <soby.mathew@arm.com>
:|M|: Javier Almansa Sobrino <javier.almansasobrino@arm.com>
:|F|: lib/gpt_rme
:|F|: include/lib/gpt_rme

IO abstraction layer
^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|M|: Olivier Deprez <olivier.deprez@arm.com>
:|F|: drivers/io/
:|F|: include/drivers/io/

JTAG DCC console driver
^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Michal Simek <michal.simek@amd.com>
:|M|: Akshay Belsare <akshay.belsare@amd.com>
:|M|: Maheedhar Sai Bollapalli <maheedharsai.bollapalli@amd.com>
:|F|: drivers/arm/dcc/
:|F|: include/drivers/arm/dcc.h

Libfdt
^^^^^^
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: common/fdt_wrappers.c
:|F|: common/fdt_fixup.c
:|F|: common/fdt_wrappers.mk
:|F|: lib/libfdt

Library At ROM (ROMlib)
^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: lib/romlib/

Live Firmware Activation Service
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: services/std_svc/lfa
:|F|: include/plat/common/plat_lfa.h
:|F|: include/services/lfa_svc.h
:|F|: include/services/lfa_component_desc.h
:|F|: include/services/lfa_holding_pen.h

Max Power Mitigation Mechanism (MPMM)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Chris Kay <chris.kay@arm.com>
:|F|: include/lib/cpus/aarch64/cpu_macros.S

Measured Boot
^^^^^^^^^^^^^
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|M|: Harrison Mutai <harrison.mutai@arm.com>
:|M|: Javier Almansa Sobrino <javier.almansasobrino@arm.com>
:|M|: Alexei Fedorov <Alexei.Fedorov@arm.com>
:|M|: Sandrine Afsa <sandrine.afsa@arm.com>
:|F|: drivers/measured_boot
:|F|: include/drivers/measured_boot
:|F|: common/measured_boot_helpers.c
:|F|: include/common/measured_boot.h
:|F|: docs/components/measured_boot
:|F|: plat/arm/board/fvp/fvp.\*_measured_boot.c

Message Handling Unit (MHU) driver
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: David Vincze <david.vincze@arm.com>
:|F|: include/drivers/arm/mhu.h
:|F|: drivers/arm/mhu

Per-CPU Framework
^^^^^^^^^^^^^^^^^
:|M|: Rohit Mathew <rohit.mathew@arm.com>
:|M|: Sammit Joshi <sammit.joshi@arm.com>
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|M|: Chris Kay <chris.kay@arm.com>
:|F|: include/lib/per_cpu
:|F|: lib/per_cpu
:|F|: docs/components/numa-per-cpu.rst

Performance Measurement Framework (PMF)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Harrison Mutai <harrison.mutai@arm.com>
:|F|: lib/pmf/
:|F|: include/lib/pmf/

Platform Security Architecture (PSA) APIs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Sandrine Afsa <sandrine.afsa@arm.com>
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: include/lib/psa
:|F|: lib/psa

Power State Coordination Interface (PSCI)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|M|: Javier Almansa Sobrino <javier.almansasobrino@arm.com>
:|M|: Boyan Karatotev <boyan.karatotev@arm.com>
:|F|: lib/psci/
:|F|: include/lib/psci/

PSA Firmware Update
^^^^^^^^^^^^^^^^^^^
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|M|: Sandrine Afsa <sandrine.afsa@arm.com>
:|F|: drivers/fwu
:|F|: include/drivers/fwu

Runtime Security Engine (RSE) comms driver
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: David Vincze <david.vincze@arm.com>
:|F|: include/drivers/arm/rse_comms.h
:|F|: drivers/arm/rse

Runtime Services
^^^^^^^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|M|: Boyan Karatotev <boyan.karatotev@arm.com>
:|F|: services/std_svc/std_svc_setup.c
:|F|: common/runtime_svc.c
:|F|: include/common/runtime_svc.h
:|F|: include/services/arm_arch_svc.h
:|F|: include/services/std_svc.h
:|F|: services/arm_arch_svc/

Standard C library
^^^^^^^^^^^^^^^^^^
:|M|: Chris Kay <chris.kay@arm.com>
:|F|: lib/libc/
:|F|: include/lib/libc/

System Control and Management Interface (SCMI) Server
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Etienne Carriere <etienne.carriere@st.com>
:|M|: Peng Fan <peng.fan@nxp.com>
:|F|: drivers/scmi-msg
:|F|: include/drivers/scmi.\*

Translation tables (``xlat_tables``) library
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|M|: Harrison Mutai <harrison.mutai@arm.com>
:|M|: Javier Almansa Sobrino <javier.almansasobrino@arm.com>
:|F|: lib/xlat_tables\_.\*/.\*
:|F|: include/lib/xlat_tables\_.\*/.\*

Firmware Interface for RME (FIRME)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Harrison Mutai <harrison.mutai@arm.com>
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|M|: Arunachalam Ganapathy <arunachalam.ganapathy@arm.com>
:|F|: services/std_svc/firme/
:|F|: include/services/firme/
:|F|: include/services/firme_svc.h

Platform Ports
~~~~~~~~~~~~~~

Allwinner ARMv8 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Andre Przywara <andre.przywara@arm.com>
:|M|: Samuel Holland <samuel@sholland.org>
:|F|: docs/plat/allwinner.rst
:|F|: plat/allwinner/
:|F|: drivers/allwinner/
:|F|: include/drivers/allwinner/

Amlogic common code
^^^^^^^^^^^^^^^^^^^
:|M|: Carlo Caione <ccaione@baylibre.com>
:|F|: plat/amlogic/common/
:|F|: tools/amlogic/Makefile
:|F|: tools/amlogic/doimage.c

Amlogic Meson S905 (GXBB) platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Andre Przywara <andre.przywara@arm.com>
:|F|: docs/plat/meson-gxbb.rst
:|F|: drivers/amlogic/
:|F|: include/drivers/amlogic/
:|F|: plat/amlogic/gxbb/

Amlogic Meson S905x (GXL) platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Remi Pommarel <repk@triplefau.lt>
:|F|: docs/plat/meson-gxl.rst
:|F|: plat/amlogic/gxl/

Amlogic Meson S905X2 (G12A) platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Carlo Caione <ccaione@baylibre.com>
:|F|: docs/plat/meson-g12a.rst
:|F|: plat/amlogic/g12a/

Amlogic Meson A113D (AXG) platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Carlo Caione <ccaione@baylibre.com>
:|F|: docs/plat/meson-axg.rst
:|F|: plat/amlogic/axg/

Arm common code
^^^^^^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|C|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: plat/arm/common/
:|F|: plat/arm/css/common/
:|F|: plat/arm/soc/common/
:|F|: plat/arm/board/common/
:|F|: include/plat/arm/common/
:|F|: include/plat/arm/css/common/
:|F|: include/plat/arm/soc/common/
:|F|: include/plat/arm/board/common/
:|F|: docs/plat/arm/index.rst
:|F|: docs/plat/arm/arm-build-options.rst

Arm FPGA platform port
^^^^^^^^^^^^^^^^^^^^^^
:|M|: Andre Przywara <andre.przywara@arm.com>
:|M|: Javier Almansa Sobrino <Javier.AlmansaSobrino@arm.com>
:|F|: plat/arm/board/arm_fpga
:|F|: docs/plat/arm/arm_fpga/
:|F|: fdts/arm_fpga.dts

Arm FVP Platform port
^^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|M|: Harrison Mutai <harrison.mutai@arm.com>
:|F|: plat/arm/board/fvp
:|F|: fdts/fvp.\*
:|F|: docs/plat/arm/fvp/

Arm Juno Platform port
^^^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|M|: Chris Kay <chris.kay@arm.com>
:|F|: plat/arm/board/juno
:|F|: docs/plat/arm/juno/
:|F|: fdts/juno.dts

Arm Morello and N1SDP Platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Anurag Koul <anurag.koul@arm.com>
:|M|: Chandni Cherukuri <chandni.cherukuri@arm.com>
:|C|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: plat/arm/board/morello
:|F|: plat/arm/board/n1sdp
:|F|: docs/plat/arm/morello/
:|F|: fdts/morello/
:|F|: fdts/n1sdp/

Arm Rich IoT Platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
:|M|: Hugues Kamba Mpiana <hugues.kambampiana@arm.com>
:|C|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: plat/arm/board/corstone700
:|F|: plat/arm/board/corstone1000
:|F|: fdts/corstone700.\*
:|F|: docs/plat/arm/corstone1000/

Arm Reference Design platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Thomas Abraham <thomas.abraham@arm.com>
:|M|: Vijayenthiran Subramaniam <vijayenthiran.subramaniam@arm.com>
:|M|: Rohit Mathew <Rohit.Mathew@arm.com>
:|C|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: plat/arm/board/neoverse_rd/common
:|F|: plat/arm/board/neoverse_rd/platform/rdn2/
:|F|: plat/arm/board/neoverse_rd/platform/rdv3/

Arm Total Compute platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Artem Kopotev <artem.kopotev@arm.com>
:|M|: Oleksandr Tarhunakov <Oleksandr.Tarhunakov@arm.com>
:|C|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: plat/arm/board/tc
:|F|: fdts/tc.\*
:|F|: docs/plat/arm/tc/

Arm Automotive RD platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Diego Sueiro <diego.sueiro@arm.com>
:|M|: Peter Hoyes <peter.hoyes@arm.com>
:|M|: Divin Raj <divin.raj@arm.com>
:|M|: Ahmed Azeem <ahmed.azeem@arm.com>
:|F|: plat/arm/board/automotive_rd
:|F|: fdts/rd1ae.dts
:|F|: fdts/rdaspen.\*
:|F|: docs/plat/arm/automotive_rd/

Aspeed platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Chia-Wei Wang <chiawei_wang@aspeedtech.com>
:|M|: Neal Liu <neal_liu@aspeedtech.com>
:|F|: docs/plat/ast2700.rst
:|F|: plat/aspeed/

HiSilicon HiKey and HiKey960 platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Haojian Zhuang <haojian.zhuang@linaro.org>
:|F|: docs/plat/hikey.rst
:|F|: docs/plat/hikey960.rst
:|F|: plat/hisilicon/hikey/
:|F|: plat/hisilicon/hikey960/

HiSilicon Poplar platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Shawn Guo <shawn.gsc@gmail.com>
:|F|: docs/plat/poplar.rst
:|F|: plat/hisilicon/poplar/

Intel SocFPGA platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Sieu Mun Tang <sieu.mun.tang@intel.com>
:|M|: Benjamin Jit Loon Lim <jit.loon.lim@intel.com>
:|F|: plat/intel/soc/
:|F|: drivers/intel/soc/
:|F|: docs/plat/intel-agilex.rst
:|F|: docs/plat/intel-stratix10.rst

Marvell platform ports and SoC drivers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jaiprakash Singh <jaiprakashs@marvell.com>
:|M|: George Cherian <george.cherian@marvell.com>
:|C|: Vincent Jardin <vjardin@free.fr>
:|F|: docs/plat/marvell/
:|F|: plat/marvell/
:|F|: include/plat/marvell/
:|F|: drivers/marvell/
:|F|: include/drivers/marvell/
:|F|: tools/marvell/

Marvell Armada a80x0_nbx platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Vincent Jardin <vjardin@free.fr>
:|F|: docs/plat/marvell/armada/a80x0_nbx.rst
:|F|: plat/marvell/armada/a8k/a80x0_nbx/

MediaTek platform ports
^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Leon Chen <leon.chen@mediatek.com>
:|M|: Jason-CH Chen <jason-ch.chen@mediatek.com>
:|M|: Yidi Lin <yidilin@chromium.org>
:|F|: docs/plat/mt.\*.rst
:|F|: plat/mediatek/

Nuvoton npcm845x platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Hila Miranda-Kuzi <hila.miranda.kuzi1@gmail.com>
:|M|: Margarita Glushkin <rutigl@gmail.com>
:|M|: Avi Fishman <avi.fishman@nuvoton.com>
:|F|: docs/plat/npcm845x.rst
:|F|: include/drivers/nuvoton/
:|F|: include/plat/nuvoton/
:|F|: plat/nuvoton/

NVidia platform ports
^^^^^^^^^^^^^^^^^^^^^
:|M|: Varun Wadekar <vwadekar@nvidia.com>
:|C|: Manish Pandey <manish.pandey2@arm.com>
:|F|: docs/plat/nvidia-tegra.rst
:|F|: include/lib/cpus/aarch64/denver.h
:|F|: lib/cpus/aarch64/denver.S
:|F|: plat/nvidia/

NXP i.MX 7 WaRP7 platform port and SoC drivers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Bryan O'Donoghue <bryan.odonoghue@linaro.org>
:|M|: Jun Nie <jun.nie@linaro.org>
:|F|: docs/plat/warp7.rst
:|F|: plat/imx/common/
:|F|: plat/imx/imx7/
:|F|: drivers/imx/timer/
:|F|: drivers/imx/uart/
:|F|: drivers/imx/usdhc/

NXP i.MX 8 platform port
^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Peng Fan <peng.fan@nxp.com>
:|F|: docs/plat/imx8.rst
:|F|: plat/imx/

NXP i.MX8M platform port
^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jacky Bai <ping.bai@nxp.com>
:|F|: docs/plat/imx8m.rst
:|F|: plat/imx/imx8m/

NXP i.MX8ULP platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jacky Bai <ping.bai@nxp.com>
:|F|: docs/plat/imx8ulp.rst
:|F|: plat/imx/imx8ulp/

NXP i.MX9 platform port
^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jacky Bai <ping.bai@nxp.com>
:|F|: docs/plat/imx9.rst
:|F|: plat/imx/imx93/
:|F|: plat/imx/imx9/imx94/
:|F|: plat/imx/imx9/imx95/

NXP QorIQ Layerscape common code for platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Pankaj Gupta <pankaj.gupta@nxp.com>
:|M|: Jiafei Pan <jiafei.pan@nxp.com>
:|F|: docs/plat/nxp/
:|F|: plat/nxp/
:|F|: drivers/nxp/
:|F|: include/drivers/nxp/
:|F|: tools/nxp/

NXP SoC Part LX2160A and its platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Pankaj Gupta <pankaj.gupta@nxp.com>
:|F|: plat/nxp/soc-lx2160a
:|F|: plat/nxp/soc-lx2160a/lx2162aqds
:|F|: plat/nxp/soc-lx2160a/lx2160aqds
:|F|: plat/nxp/soc-lx2160a/lx2160ardb

Freebox Nodebox
^^^^^^^^^^^^^^^
:|M|: Vincent Jardin <vjardin@free.fr>
:|F|: plat/nxp/soc-lx2160a/nbxv3
:|F|: docs/plat/nxp/nbxv3.rst

NXP SoC Part LS1028A and its platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jiafei Pan <jiafei.pan@nxp.com>
:|F|: plat/nxp/soc-ls1028a
:|F|: plat/nxp/soc-ls1028a/ls1028ardb

NXP SoC Part LS1043A and its platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jiafei Pan <jiafei.pan@nxp.com>
:|F|: plat/nxp/soc-ls1043a
:|F|: plat/nxp/soc-ls1043a/ls1043ardb

NXP SoC Part LS1046A and its platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jiafei Pan <jiafei.pan@nxp.com>
:|F|: plat/nxp/soc-ls1046a
:|F|: plat/nxp/soc-ls1046a/ls1046ardb
:|F|: plat/nxp/soc-ls1046a/ls1046afrwy
:|F|: plat/nxp/soc-ls1046a/ls1046aqds

NXP SoC Part LS1088A and its platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jiafei Pan <jiafei.pan@nxp.com>
:|F|: plat/nxp/soc-ls1088a
:|F|: plat/nxp/soc-ls1088a/ls1088ardb
:|F|: plat/nxp/soc-ls1088a/ls1088aqds

NXP SoC Part S32G274A and its platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Ghennadi Procopciuc <ghennadi.procopciuc@nxp.com>
:|F|: docs/plat/s32g274a.rst
:|F|: drivers/nxp/clk/s32cc
:|F|: drivers/nxp/console/linflex_console.S
:|F|: include/drivers/nxp/console/linflex.h
:|F|: plat/nxp/s32

QEMU platform port
^^^^^^^^^^^^^^^^^^
:|M|: Jens Wiklander <jens.wiklander@oss.qualcomm.com>
:|F|: docs/plat/qemu.rst
:|F|: docs/plat/qemu-sbsa.rst
:|F|: plat/qemu/

QTI platform port and SoC drivers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Sumit Garg <sumit.garg@oss.qualcomm.com>
:|M|: Jorge Ramirez-Ortiz <jorge.ramirez@oss.qualcomm.com>
:|M|: Sreevyshanavi Kare <skare@qti.qualcomm.com>
:|C|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: docs/plat/qti/
:|F|: docs/plat/qti.rst
:|F|: plat/qti/
:|F|: drivers/qti/
:|F|: include/drivers/qti/
:|F|: tools/qti/

QTI MSM8916 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Stephan Gerhold <stephan@gerhold.net>
:|M|: Nikita Travkin <nikita@trvn.ru>
:|F|: docs/plat/qti/msm8916.rst
:|F|: docs/plat/qti-msm8916.rst
:|F|: plat/qti/bear/

Raspberry Pi 3 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Ying-Chun Liu (PaulLiu) <paul.liu@linaro.org>
:|F|: docs/plat/rpi3.rst
:|F|: plat/rpi/rpi3/
:|F|: plat/rpi/common/
:|F|: drivers/rpi3/
:|F|: include/drivers/rpi3/

Raspberry Pi 4 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Andre Przywara <andre.przywara@arm.com>
:|F|: docs/plat/rpi4.rst
:|F|: plat/rpi/rpi4/
:|F|: plat/rpi/common/
:|F|: drivers/rpi3/
:|F|: include/drivers/rpi3/

Raspberry Pi 5 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Mario Bălănică <mariobalanica02@gmail.com>
:|F|: docs/plat/rpi5.rst
:|F|: plat/rpi/rpi5

Renesas R-Car Gen3, Gen4, Gen5 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Marek Vasut <marek.vasut+renesas@mailbox.org>
:|C|: Sandrine Afsa <sandrine.afsa@arm.com>
:|F|: docs/plat/rcar-gen3.rst
:|F|: drivers/renesas/common
:|F|: drivers/renesas/rcar.\*
:|F|: include/drivers/renesas/rcar.\*
:|F|: plat/renesas/common
:|F|: plat/renesas/rcar.\*
:|F|: tools/renesas/rcar_layout_create

Renesas RZ/A platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Nhut Nguyen <nhut.nguyen.kc@renesas.com>
:|F|: docs/plat/rz-a3.rst
:|F|: plat/renesas/rza
:|F|: drivers/renesas/rza
:|F|: include/drivers/renesas/rza/
:|F|: tools/renesas/rza

Renesas RZ/G2 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Biju Das <biju.das.jz@bp.renesas.com>
:|M|: Marek Vasut <marek.vasut@gmail.com>
:|M|: Lad Prabhakar <prabhakar.mahadev-lad.rj@bp.renesas.com>
:|C|: Sandrine Afsa <sandrine.afsa@arm.com>
:|F|: docs/plat/rz-g2.rst
:|F|: plat/renesas/common
:|F|: plat/renesas/rzg
:|F|: drivers/renesas/common
:|F|: drivers/renesas/rzg
:|F|: tools/renesas/rzg_layout_create

RockChip platform port
^^^^^^^^^^^^^^^^^^^^^^
:|M|: Tony Xie <tony.xie@rock-chips.com>
:|M|: Heiko Stuebner <heiko@sntech.de>
:|F|: docs/plat/rockchip.rst
:|F|: plat/rockchip/

STMicroelectronics platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Yann Gautier <yann.gautier@st.com>
:|M|: Maxime Méré <maxime.mere@foss.st.com>
:|C|: Manish Pandey <manish.pandey2@arm.com>
:|F|: docs/plat/st/.\*
:|F|: docs/plat/stm32mp1.rst
:|F|: drivers/st/
:|F|: include/drivers/st/
:|F|: plat/st/
:|F|: tools/fiptool/plat_fiptool/st/
:|F|: fdts/stm32.\*
:|F|: include/dt-bindings/.\*/.\*stm32.\*
:|F|: tools/stm32image/

Synquacer platform port
^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Sumit Garg <sumit.garg@kernel.org>
:|M|: Masahisa Kojima <kojima.masahisa@socionext.com>
:|C|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: docs/plat/synquacer.rst
:|F|: plat/socionext/synquacer/

Texas Instruments platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Nishanth Menon <nm@ti.com>
:|F|: docs/plat/ti-k3.rst
:|F|: docs/plat/ti-k3low-am62lx.rst
:|F|: plat/ti/
:|F|: drivers/ti/
:|F|: include/drivers/ti/

UniPhier platform port
^^^^^^^^^^^^^^^^^^^^^^
:|F|: docs/plat/socionext-uniphier.rst
:|F|: plat/socionext/uniphier/

AMD-Xilinx platform port
^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Michal Simek <michal.simek@amd.com>
:|M|: Akshay Belsare <akshay.belsare@amd.com>
:|M|: Maheedhar Sai Bollapalli <maheedharsai.bollapalli@amd.com>
:|F|: docs/plat/amd.\*
:|F|: docs/plat/xilinx.\*
:|F|: plat/amd/
:|F|: plat/xilinx/

Altera common code
^^^^^^^^^^^^^^^^^^
:|M|: Benjamin Jit Loon Lim <jit.loon.lim@altera.com>
:|C|: Jit Loon Lim <jit.loon.lim@altera.com>
:|F|: plat/altera/soc/common

Altera agilex3 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Benjamin Jit Loon Lim <jit.loon.lim@altera.com>
:|C|: Jit Loon Lim <jit.loon.lim@altera.com>
:|F|: plat/altera/soc/agilex3

Secure Payloads and Dispatchers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OP-TEE dispatcher
^^^^^^^^^^^^^^^^^
:|M|: Jens Wiklander <jens.wiklander@oss.qualcomm.com>
:|F|: docs/components/spd/optee-dispatcher.rst
:|F|: services/spd/opteed/
:|F|: bl32/optee/optee.mk
:|F|: include/lib/optee_utils.h
:|F|: lib/optee/optee_utils.c

TLK
^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Varun Wadekar <vwadekar@nvidia.com>
:|F|: docs/components/spd/tlk-dispatcher.rst
:|F|: include/bl32/payloads/tlk.h
:|F|: services/spd/tlkd/

Trusty secure payloads
^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Arve Hjønnevåg <arve@android.com>
:|M|: Marco Nelissen <marcone@google.com>
:|M|: Varun Wadekar <vwadekar@nvidia.com>
:|F|: docs/components/spd/trusty-dispatcher.rst
:|F|: services/spd/trusty/

Test Secure Payload (TSP)
^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: bl32/tsp/
:|F|: services/spd/tspd/
:|F|: include/bl32/tsp/

ProvenCore Secure Payload Dispatcher
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jérémie Corbier <jeremie.corbier@provenrun.com>
:|F|: docs/components/spd/pnc-dispatcher.rst
:|F|: services/spd/pncd/
:|F|: include/bl32/pnc/pnc.h

Secure Partition Manager Core (EL3 FF-A SPMC)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Marc Bonnici <marc.bonnici@arm.com>
:|F|: services/std_svc/spm/el3_spmc/\*
:|F|: include/services/el3_spmc\_\*
:|F|: include/services/spmc_svc.h
:|F|: include/services/spm_core_manifest.h

Secure Partition Manager Dispatcher (SPMD)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Olivier Deprez <olivier.deprez@arm.com>
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|F|: services/std_svc/spmd/\*
:|F|: services/std_svc/spm/common/
:|F|: plat/common/plat_spmd_manifest.c
:|F|: include/services/ffa_svc.h
:|F|: include/services/el3_spmd_logical_sp.h
:|F|: include/services/spmd_svc.h

Realm Management Monitor Dispatcher (RMMD)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Javier Almansa Sobrino <javier.almansasobrino@arm.com>
:|M|: Alexei Fedorov <Alexei.Fedorov@arm.com>
:|F|: services/std_svc/rmmd/\*
:|F|: include/services/rmmd_svc.h
:|F|: include/services/rmm_core_manifest.h
:|F|: include/services/rmmd_rmm_lfa.h

Tools
~~~~~

Fiptool
^^^^^^^
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|M|: Harrison Mutai <harrison.mutai@arm.com>
:|F|: tools/fiptool/

Cert_create tool
^^^^^^^^^^^^^^^^
:|M|: Sandrine Afsa <sandrine.afsa@arm.com>
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: tools/cert_create/

Encrypt_fw tool
^^^^^^^^^^^^^^^
:|M|: Sumit Garg <sumit.garg@kernel.org>
:|F|: tools/encrypt_fw/

Sptool
^^^^^^
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|F|: tools/sptool/

cot-dt2c
^^^^^^^^
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: tools/cot_dt2c/

memory
^^^^^^
:|M|: Chris Kay <chris.kay@arm.com>
:|F|: tools/memory/

Build system
^^^^^^^^^^^^
:|M|: Chris Kay <chris.kay@arm.com>
:|M|: Harrison Mutai <harrison.mutai@arm.com>
:|C|: Manish Badarkhe <manish.badarkhe@arm.com>
:|F|: Makefile
:|F|: make_helpers/

Third party libraries
^^^^^^^^^^^^^^^^^^^^^
:|M|: Chris Kay <chris.kay@arm.com>
:|M|: Harrison Mutai <harrison.mutai@arm.com>
:|F|: lib/compiler-rt/
:|F|: include/lib/zlib/
:|F|: lib/zlib/

Threat Model
~~~~~~~~~~~~~
:|M|: Sandrine Afsa <sandrine.afsa@arm.com>
:|M|: Raghu Krishnamurthy <raghuoss@raghushome.com>
:|M|: Varun Wadekar <vwadekar@nvidia.com>
:|F|: docs/threat_model/
:|F|: docs/security_advisories/

Conventional Changelog Extensions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
:|M|: Chris Kay <chris.kay@arm.com>
:|F|: tools/conventional-changelog-tf-a


--------------

*Copyright (c) 2019-2026, Arm Limited and Contributors. All rights reserved.*

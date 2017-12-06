ARM Trusted Firmware Maintainers
================================

ARM Trusted Firmware is an ARM maintained project. All contributions are
ultimately merged by the maintainers listed below. Technical ownership of some
parts of the codebase is delegated to the sub-maintainers listed below. An
acknowledgement from these sub-maintainers may be required before the
maintainers merge a contribution.

Maintainers
-----------

Dan Handley (dan.handley@arm.com, `danh-arm`_)

David Cunado (david.cunado@arm.com, `davidcunado-arm`_)

OPTEE and QEMU platform sub-maintainer
--------------------------------------

Jens Wiklander (jens.wiklander@linaro.org, `jenswi-linaro`_)

Files:

-  docs/spd/optee-dispatcher.md
-  docs/plat/qemu.md
-  services/spd/opteed/\*
-  plat/qemu/\*

TLK/Trusty SPDs and NVidia platforms sub-maintainer
---------------------------------------------------

Varun Wadekar (vwadekar@nvidia.com, `vwadekar`_)

Files:

-  docs/spd/tlk-dispatcher.md
-  docs/spd/trusty-dispatcher.md
-  include/bl32/payloads/tlk.h
-  include/lib/cpus/aarch64/denver.h
-  lib/cpus/aarch64/denver.S
-  services/spd/tlkd/\*
-  services/spd/trusty/\*
-  plat/nvidia/\*

eMMC/UFS drivers and HiSilicon platform sub-maintainer
------------------------------------------------------

Haojian Zhuang (haojian.zhuang@linaro.org, `hzhuang1`_)

Files:

-  docs/plat/hikey.md
-  docs/plat/hikey960.md
-  drivers/emmc/\*
-  drivers/partition/\*
-  drivers/synopsys/emmc/\*
-  drivers/synopsys/ufs/\*
-  drivers/ufs/\*
-  include/drivers/dw\_ufs.h
-  include/drivers/emmc.h
-  include/drivers/ufs.h
-  include/drivers/synopsys/dw\_mmc.h
-  plat/hisilicon/\*

MediaTek platform sub-maintainer
--------------------------------

Yidi Lin (林以廸 yidi.lin@mediatek.com, `mtk09422`_)

Files:

-  plat/mediatek/\*

Raspberry Pi 3 platform sub-maintainer
--------------------------------------

Antonio Niño Díaz (antonio.ninodiaz@arm.com, `antonio-nino-diaz-arm`_)

Files:

-  plat/rpi3/\*

RockChip platform sub-maintainer
--------------------------------

Tony Xie (tony.xie@rock-chips.com, `TonyXie06`_
or `rkchrome`_)

Files:

-  plat/rockchip/\*

Xilinx platform sub-maintainer
------------------------------

Siva Durga Prasad Paladugu (siva.durga.paladugu@xilinx.com, `sivadur`_)

Files:

-  docs/plat/xilinx-zynqmp.md
-  plat/xilinx/\*

ARMv7 architecture sub-maintainer
---------------------------------

Etienne Carriere (etienne.carriere@linaro.org, `etienne-lms`_)

.. _antonio-nino-diaz-arm: https://github.com/antonio-nino-diaz-arm
.. _danh-arm: https://github.com/danh-arm
.. _davidcunado-arm: https://github.com/davidcunado-arm
.. _jenswi-linaro: https://github.com/jenswi-linaro
.. _vwadekar: https://github.com/vwadekar
.. _hzhuang1: https://github.com/hzhuang1
.. _mtk09422: https://github.com/mtk09422
.. _TonyXie06: https://github.com/TonyXie06
.. _rkchrome: https://github.com/rkchrome
.. _sivadur: https://github.com/sivadur
.. _etienne-lms: https://github.com/etienne-lms

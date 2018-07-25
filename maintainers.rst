Trusted Firmware-A maintainers
==============================

Trusted Firmware-A (TF-A) is an Arm maintained project. All contributions are
ultimately merged by the maintainers listed below. Technical ownership of some
parts of the codebase is delegated to the sub-maintainers listed below. An
acknowledgement from these sub-maintainers may be required before the
maintainers merge a contribution.

Main maintainers
----------------
:M: Dan Handley <dan.handley@arm.com>
:G: `danh-arm`_
:M: Dimitris Papastamos <dimitrs.papastamos@arm.com>
:G: `dp-arm`_
:M: Soby Mathew <soby.mathew@arm.com>
:G: `soby-mathew`_

Allwinner ARMv8 platform port
-----------------------------
:M: Andre Przywara <andre.przywara@arm.com>
:G: `Andre-ARM`_
:M: Samuel Holland <samuel@sholland.org>
:G: `smaeul`_
:F: docs/plat/allwinner.rst
:F: plat/allwinner/

Armv7-A architecture port
-------------------------
:M: Etienne Carriere <etienne.carriere@linaro.org>
:G: `etienne-lms`_

eMMC/UFS drivers
----------------
:M: Haojian Zhuang <haojian.zhuang@linaro.org>
:G: `hzhuang1`_
:F: drivers/emmc/
:F: drivers/partition/
:F: drivers/synopsys/emmc/
:F: drivers/synopsys/ufs/
:F: drivers/ufs/
:F: include/drivers/dw_ufs.h
:F: include/drivers/emmc.h
:F: include/drivers/ufs.h
:F: include/drivers/synopsys/dw_mmc.h

HiSilicon HiKey and HiKey960 platform ports
-------------------------------------------
:M: Haojian Zhuang <haojian.zhuang@linaro.org>
:G: `hzhuang1`_
:F: docs/plat/hikey.rst
:F: docs/plat/hikey960.rst
:F: plat/hisilicon/hikey/
:F: plat/hisilicon/hikey960/

HiSilicon Poplar platform port
------------------------------
:M: Shawn Guo <shawn.guo@linaro.org>
:G: `shawnguo2`_
:F: docs/plat/poplar.rst
:F: plat/hisilicon/poplar/

MediaTek platform ports
-----------------------
:M: Yidi Lin (林以廸) <yidi.lin@mediatek.com>
:G: `mtk09422`_
:F: plat/mediatek/

Marvell platform ports and SoC drivers
--------------------------------------
:M: Konstantin Porotchkin <kostap@marvell.com>
:G: `kostapr`_
:F: docs/plat/marvell/
:F: plat/marvell/
:F: drivers/marvell/

NVidia platform ports
---------------------
:M: Varun Wadekar <vwadekar@nvidia.com>
:G: `vwadekar`_
:F: docs/plat/nvidia-tegra.rst
:F: include/lib/cpus/aarch64/denver.h
:F: lib/cpus/aarch64/denver.S
:F: plat/nvidia/

NXP QorIQ Layerscape platform ports
-----------------------------------
:M: Jiafei Pan <jiafei.pan@nxp.com>
:G: `qoriq-open-source`_
:F: docs/plat/ls1043a.rst
:F: plat/layerscape/

NXP i.MX 8 platform port
------------------------
:M: Anson Huang <Anson.Huang@nxp.com>
:G: `Anson-Huang`_
:F: docs/plat/imx8.rst
:F: plat/imx/imx8/

NXP i.MX 7 WaRP7 platform port
------------------------------
:M: Bryan O'Donoghue <bryan.odonoghue@linaro.org>
:G: `bryanodonoghue`_
:F: docs/plat/warp7.rst
:F: plat/imx/common/
:F: plat/imx/imx7/

OP-TEE dispatcher
-----------------
:M: Jens Wiklander <jens.wiklander@linaro.org>
:G: `jenswi-linaro`_
:F: docs/spd/optee-dispatcher.rst
:F: services/spd/opteed/

QEMU platform port
------------------
:M: Jens Wiklander <jens.wiklander@linaro.org>
:G: `jenswi-linaro`_
:F: docs/plat/qemu.rst
:F: plat/qemu/

Raspberry Pi 3 platform port
----------------------------
:M: Antonio Niño Díaz <antonio.ninodiaz@arm.com>
:G: `antonio-nino-diaz-arm`_
:F: docs/plat/rpi3.rst
:F: plat/rpi3/

RockChip platform port
----------------------
:M: Tony Xie <tony.xie@rock-chips.com>
:G: `TonyXie06`_
:G: `rockchip-linux`_
:F: plat/rockchip/

Synquacer platform port
-----------------------
:M: Sumit Garg <sumit.garg@linaro.org>
:G: `b49020`_
:F: docs/plat/synquacer.rst
:F: plat/socionext/synquacer/

Texas Instruments platform port
-------------------------------
:M: Andrew F. Davis <afd@ti.com>
:G: `glneo`_
:F: docs/plat/ti-k3.rst
:F: plat/ti/

TLK/Trusty secure payloads
--------------------------
:M: Varun Wadekar <vwadekar@nvidia.com>
:G: `vwadekar`_
:F: docs/spd/tlk-dispatcher.rst
:F: docs/spd/trusty-dispatcher.rst
:F: include/bl32/payloads/tlk.h
:F: services/spd/tlkd/
:F: services/spd/trusty/

UniPhier platform port
----------------------
:M: Masahiro Yamada <yamada.masahiro@socionext.com>
:G: `masahir0y`_
:F: docs/plat/socionext-uniphier.rst
:F: plat/socionext/uniphier/

Xilinx platform port
--------------------
:M: Siva Durga Prasad Paladugu <siva.durga.paladugu@xilinx.com>
:G: `sivadur`_
:F: docs/plat/xilinx-zynqmp.rst
:F: plat/xilinx/

.. _Andre-ARM: https://github.com/Andre-ARM
.. _Anson-Huang: https://github.com/Anson-Huang
.. _antonio-nino-diaz-arm: https://github.com/antonio-nino-diaz-arm
.. _bryanodonoghue: https://github.com/bryanodonoghue
.. _b49020: https://github.com/b49020
.. _danh-arm: https://github.com/danh-arm
.. _dp-arm: https://github.com/dp-arm
.. _etienne-lms: https://github.com/etienne-lms
.. _glneo: https://github.com/glneo
.. _hzhuang1: https://github.com/hzhuang1
.. _jenswi-linaro: https://github.com/jenswi-linaro
.. _kostapr: https://github.com/kostapr
.. _masahir0y: https://github.com/masahir0y
.. _mtk09422: https://github.com/mtk09422
.. _qoriq-open-source: https://github.com/qoriq-open-source
.. _rockchip-linux: https://github.com/rockchip-linux
.. _shawnguo2: https://github.com/shawnguo2
.. _sivadur: https://github.com/sivadur
.. _smaeul: https://github.com/smaeul
.. _soby-mathew: https://github.com/soby-mathew
.. _TonyXie06: https://github.com/TonyXie06
.. _vwadekar: https://github.com/vwadekar

WORKDIR=`pwd`


#########
#Env Setting 
#########

USERNAME=kuanhsunc
OUT_DIR=/home/mtkuser/Downloads/vdk_out

DATE=`date +"%Y-%m-%d_%H_%M"`

PLAT=n1auto

BL31=${WORKDIR}/build/${PLAT}/debug/bl31.bin


make CROSS_COMPILE="./aarch64-linux-gnu-6.3.1/bin/aarch64-linux-gnu-" NEED_BL1=no NEED_BL2=no NEED_BL31=yes PLAT=n1auto COREBOOT=1 PRELOADED_BL33_BASE=0x90050000 DEBUG=1


cp -rf ${BL31} ${OUT_DIR}/
cd ${OUT_DIR}

rm -rf vdk_atf_image_*.zip
zip -r vdk_atf_image_${DATE}.zip bl31.bin



PLAT_INCLUDES		+=	\
				-Idrivers/scmi-msg/				\
				-I${PLAT_PATH}/common/drivers/scmi		\

BL31_SOURCES		+=	\
				${PLAT_PATH}/common/drivers/scmi/scmi.c		\

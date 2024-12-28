PLAT_INCLUDES		+=	\
				-Idrivers/scmi-msg/				\
				-I${PLAT_PATH}/common/drivers/scmi		\

BL31_SOURCES		+=	\
				${PLAT_PATH}/common/drivers/scmi/scmi.c		\
				${PLAT_PATH}/common/drivers/scmi/scmi_clock.c		\
				${PLAT_PATH}/common/drivers/scmi/scmi_pd.c		\

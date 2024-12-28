PLAT_INCLUDES		+=	\
				-I${PLAT_PATH}/board/am62l/pm/include/			\

BL31_SOURCES		+=	\
				${PLAT_PATH}/board/am62l/pm/clk_soc_hfosc0.c    \
				${PLAT_PATH}/board/am62l/pm/clk_soc_lfosc0.c    \
				${PLAT_PATH}/board/am62l/pm/clocks.c    \
				${PLAT_PATH}/board/am62l/pm/devices.c    \
				${PLAT_PATH}/board/am62l/pm/host_idx_mapping.c    \



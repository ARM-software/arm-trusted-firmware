PLAT_INCLUDES	+=	\
				-I${PLAT_PATH}/board/am62l/lpm			\
				-I${PLAT_PATH}/common				\

BL31_SOURCES	+=	\
				${PLAT_PATH}/board/am62l/lpm/call_sram.S	\
				${PLAT_PATH}/board/am62l/lpm/ddr.c		\
				${PLAT_PATH}/board/am62l/lpm/gtc.c		\
				${PLAT_PATH}/board/am62l/lpm/k3_lpm_timeout.c	\
				${PLAT_PATH}/board/am62l/lpm/lpm_stub.c		\
				${PLAT_PATH}/board/am62l/lpm/lpm_trace.c	\
				${PLAT_PATH}/board/am62l/lpm/pll_16fft_raw.c	\
				${PLAT_PATH}/board/am62l/lpm/psc_raw.c		\
				${PLAT_PATH}/board/am62l/lpm/rtc.c

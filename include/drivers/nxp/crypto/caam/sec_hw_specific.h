/*
 * Copyright 2017-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _SEC_HW_SPECIFIC_H_
#define _SEC_HW_SPECIFIC_H_

#include "caam.h"
#include "sec_jr_driver.h"

 /* DEFINES AND MACROS */

/* Used to retry resetting a job ring in SEC hardware. */
#define SEC_TIMEOUT 100000

/*
 * Offset to the registers of a job ring.
 *Is different for each job ring.
 */
#define CHAN_BASE(jr)   ((phys_addr_t)(jr)->register_base_addr)

#define unlikely(x)	 __builtin_expect(!!(x), 0)

#define SEC_JOB_RING_IS_FULL(pi, ci, ring_max_size, ring_threshold)    \
	((((pi) + 1 + ((ring_max_size) - (ring_threshold))) &	\
	  (ring_max_size - 1))  == ((ci)))

#define SEC_CIRCULAR_COUNTER(x, max)   (((x) + 1) & (max - 1))

 /* Struct representing various job ring registers */
struct jobring_regs {
#ifdef NXP_SEC_BE
	unsigned int irba_h;
	unsigned int irba_l;
#else
	unsigned int irba_l;
	unsigned int irba_h;
#endif
	unsigned int rsvd1;
	unsigned int irs;
	unsigned int rsvd2;
	unsigned int irsa;
	unsigned int rsvd3;
	unsigned int irja;
#ifdef NXP_SEC_BE
	unsigned int orba_h;
	unsigned int orba_l;
#else
	unsigned int orba_l;
	unsigned int orba_h;
#endif
	unsigned int rsvd4;
	unsigned int ors;
	unsigned int rsvd5;
	unsigned int orjr;
	unsigned int rsvd6;
	unsigned int orsf;
	unsigned int rsvd7;
	unsigned int jrsta;
	unsigned int rsvd8;
	unsigned int jrint;
	unsigned int jrcfg0;
	unsigned int jrcfg1;
	unsigned int rsvd9;
	unsigned int irri;
	unsigned int rsvd10;
	unsigned int orwi;
	unsigned int rsvd11;
	unsigned int jrcr;
};

 /* Offsets representing common SEC Registers */
#define SEC_REG_MCFGR_OFFSET		0x0004
#define SEC_REG_SCFGR_OFFSET		0x000C
#define SEC_REG_JR0ICIDR_MS_OFFSET	0x0010
#define SEC_REG_JR0ICIDR_LS_OFFSET	0x0014
#define SEC_REG_JR1ICIDR_MS_OFFSET	0x0018
#define SEC_REG_JR1ICIDR_LS_OFFSET	0x001C
#define SEC_REG_JR2ICIDR_MS_OFFSET	0x0020
#define SEC_REG_JR2ICIDR_LS_OFFSET	0x0024
#define SEC_REG_JR3ICIDR_MS_OFFSET	0x0028
#define SEC_REG_JR3ICIDR_LS_OFFSET	0x002C
#define SEC_REG_JRSTARTR_OFFSET		0x005C
#define SEC_REG_CTPR_MS_OFFSET		0x0FA8

 /* Offsets  representing various RNG registers */
#define RNG_REG_RTMCTL_OFFSET		0x0600
#define RNG_REG_RTSDCTL_OFFSET		0x0610
#define RNG_REG_RTFRQMIN_OFFSET		0x0618
#define RNG_REG_RTFRQMAX_OFFSET		0x061C
#define RNG_REG_RDSTA_OFFSET		0x06C0
#define ALG_AAI_SH_SHIFT		4

 /* SEC Registers Bitmasks */
#define	MCFGR_PS_SHIFT			16
#define	MCFGR_AWCACHE_SHIFT			 8
#define	MCFGR_AWCACHE_MASK	(0xF << MCFGR_AWCACHE_SHIFT)
#define	MCFGR_ARCACHE_SHIFT			12
#define	MCFGR_ARCACHE_MASK	(0xF << MCFGR_ARCACHE_SHIFT)

#define SCFGR_RNGSH0		0x00000200
#define	SCFGR_VIRT_EN		0x00008000

#define JRICID_MS_LICID		0x80000000
#define JRICID_MS_LAMTD		0x00020000
#define JRICID_MS_AMTDT		0x00010000
#define JRICID_MS_TZ		0x00008000
#define JRICID_LS_SDID_MASK	0x00000FFF
#define JRICID_LS_NSEQID_MASK	0x0FFF0000
#define JRICID_LS_NSEQID_SHIFT		16
#define JRICID_LS_SEQID_MASK	0x00000FFF

#define JRSTARTR_STARTJR0	0x00000001
#define JRSTARTR_STARTJR1	0x00000002
#define JRSTARTR_STARTJR2	0x00000004
#define JRSTARTR_STARTJR3	0x00000008

#define CTPR_VIRT_EN_POR	0x00000002
#define CTPR_VIRT_EN_INC	0x00000001

 /* RNG RDSTA bitmask */
#define RNG_STATE0_HANDLE_INSTANTIATED	0x00000001
#define RTMCTL_PRGM 0x00010000	/* 1 -> program mode, 0 -> run mode */
 /* use von Neumann data in both entropy shifter and statistical checker */
#define RTMCTL_SAMP_MODE_VON_NEUMANN_ES_SC	 0
 /* use raw data in both entropy shifter and statistical checker */
#define RTMCTL_SAMP_MODE_RAW_ES_SC			 1
 /* use von Neumann data in entropy shifter, raw data in statistical checker */
#define RTMCTL_SAMP_MODE_VON_NEUMANN_ES_RAW_SC 2
 /* invalid combination */
#define RTMCTL_SAMP_MODE_INVALID			   3
#define RTSDCTL_ENT_DLY_MIN	3200
#define RTSDCTL_ENT_DLY_MAX	12800
#define RTSDCTL_ENT_DLY_SHIFT	16
#define RTSDCTL_ENT_DLY_MASK	(U(0xffff) << RTSDCTL_ENT_DLY_SHIFT)
#define RTFRQMAX_DISABLE	   (1 << 20)

 /* Constants for error handling on job ring */
#define JR_REG_JRINT_ERR_TYPE_SHIFT	8
#define JR_REG_JRINT_ERR_ORWI_SHIFT	16
#define JR_REG_JRINIT_JRE_SHIFT			1

#define JRINT_JRE			(1 << JR_REG_JRINIT_JRE_SHIFT)
#define JRINT_ERR_WRITE_STATUS		(1 << JR_REG_JRINT_ERR_TYPE_SHIFT)
#define JRINT_ERR_BAD_INPUT_BASE	(3 << JR_REG_JRINT_ERR_TYPE_SHIFT)
#define JRINT_ERR_BAD_OUTPUT_BASE	(4 << JR_REG_JRINT_ERR_TYPE_SHIFT)
#define JRINT_ERR_WRITE_2_IRBA		(5 << JR_REG_JRINT_ERR_TYPE_SHIFT)
#define JRINT_ERR_WRITE_2_ORBA		(6 << JR_REG_JRINT_ERR_TYPE_SHIFT)
#define JRINT_ERR_RES_B4_HALT		(7 << JR_REG_JRINT_ERR_TYPE_SHIFT)
#define JRINT_ERR_REM_TOO_MANY		(8 << JR_REG_JRINT_ERR_TYPE_SHIFT)
#define JRINT_ERR_ADD_TOO_MANY		(9 << JR_REG_JRINT_ERR_TYPE_SHIFT)
#define JRINT_ERR_HALT_MASK		0x0C
#define JRINT_ERR_HALT_INPROGRESS	0x04
#define JRINT_ERR_HALT_COMPLETE		0x08

#define JR_REG_JRCR_VAL_RESET		0x00000001

#define JR_REG_JRCFG_LO_ICTT_SHIFT	0x10
#define JR_REG_JRCFG_LO_ICDCT_SHIFT	0x08
#define JR_REG_JRCFG_LO_ICEN_EN		0x02
#define JR_REG_JRCFG_LO_IMSK_EN		0x01

 /* Constants for Descriptor Processing errors */
#define SEC_HW_ERR_SSRC_NO_SRC			0x00
#define SEC_HW_ERR_SSRC_CCB_ERR			0x02
#define SEC_HW_ERR_SSRC_JMP_HALT_U	0x03
#define SEC_HW_ERR_SSRC_DECO		0x04
#define SEC_HW_ERR_SSRC_JR		0x06
#define SEC_HW_ERR_SSRC_JMP_HALT_COND   0x07

#define SEC_HW_ERR_DECO_HFN_THRESHOLD   0xF1
#define SEC_HW_ERR_CCB_ICV_CHECK_FAIL   0x0A

 /* Macros for extracting error codes for the job ring */

#define JR_REG_JRINT_ERR_TYPE_EXTRACT(value)			\
				((value) & 0x00000F00)

#define JR_REG_JRINT_ERR_ORWI_EXTRACT(value)			\
				(((value) & 0x3FFF0000) >>	\
				 JR_REG_JRINT_ERR_ORWI_SHIFT)

#define JR_REG_JRINT_JRE_EXTRACT(value)				\
				((value) & JRINT_JRE)

 /* Macros for manipulating JR registers */
typedef union {
	uint64_t m_whole;
	struct {
#ifdef NXP_SEC_BE
		uint32_t high;
		uint32_t low;
#else
		uint32_t low;
		uint32_t high;
#endif
	} m_halves;
} ptr_addr_t;

#if defined(CONFIG_PHYS_64BIT)
#define sec_read_addr(a)		sec_in64((a))
#define sec_write_addr(a, v)	sec_out64((a), (v))
#else
#define sec_read_addr(a)		sec_in32((a))
#define sec_write_addr(a, v)		sec_out32((a), (v))
#endif

#define JR_REG(name, jr)	(CHAN_BASE(jr) + JR_REG_##name##_OFFSET)
#define JR_REG_LO(name, jr)	(CHAN_BASE(jr) + JR_REG_##name##_OFFSET_LO)

#define GET_JR_REG(name, jr)	(sec_in32(JR_REG(name, (jr))))
#define GET_JR_REG_LO(name, jr)	(sec_in32(JR_REG_LO(name, (jr))))

#define SET_JR_REG(name, jr, val)		\
		(sec_out32(JR_REG(name, (jr)), (val)))

#define SET_JR_REG_LO(name, jr, val)	\
		(sec_out32(JR_REG_LO(name, (jr)), (val)))

 /* STRUCTURES AND OTHER TYPEDEFS */
 /*  Lists the possible states for a job ring. */
typedef enum sec_job_ring_state_e {
	SEC_JOB_RING_STATE_STARTED,	/* Job ring is initialized */
	SEC_JOB_RING_STATE_RESET,	/* Job ring reset is in progres */
} sec_job_ring_state_t;

struct sec_job_ring_t {
	/*
	 * Consumer index for job ring (jobs array).
	 * @note: cidx and pidx are accessed from
	 * different threads.
	 * Place the cidx and pidx inside the structure
	 *  so that they lay on different cachelines, to
	 * avoid false sharing between threads when the
	 * threads run on different cores!
	 */
	uint32_t cidx;

	/* Producer index for job ring (jobs array) */
	uint32_t pidx;

	/*  Ring of input descriptors. Size of array is power of 2 to allow
	 * fast update of producer/consumer indexes with  bitwise operations.
	 */
	phys_addr_t *input_ring;

	/*  Ring of output descriptors. */
	struct sec_outring_entry *output_ring;

	/* The file descriptor used for polling for interrupts notifications */
	uint32_t irq_fd;

	/* Model used by SEC Driver to receive  notifications from SEC.
	 *  Can be either of the three:
	 * #SEC_NOTIFICATION_TYPE_IRQ or
	 * #SEC_NOTIFICATION_TYPE_POLL
	 */
	uint32_t jr_mode;
	/* Base address for SEC's register memory for this job ring. */
	void *register_base_addr;
	/* notifies if coelescing is enabled for the job ring */
	uint8_t coalescing_en;
	/* The state of this job ring */
	sec_job_ring_state_t jr_state;
};

 /* Forward structure declaration */
typedef struct sec_job_ring_t sec_job_ring_t;

struct sec_outring_entry {
	phys_addr_t desc;	/* Pointer to completed descriptor */
	uint32_t status;	/* Status for completed descriptor */
} __packed;

 /* Lists the states possible for the SEC user space driver. */
typedef enum sec_driver_state_e {
	SEC_DRIVER_STATE_IDLE,	/*< Driver not initialized */
	SEC_DRIVER_STATE_STARTED,	/*< Driver initialized and */
	SEC_DRIVER_STATE_RELEASE,	/*< Driver release is in progress */
} sec_driver_state_t;

 /* Union describing the possible error codes that */
 /* can be set in the descriptor status word */

union hw_error_code {
	uint32_t error;
	union {
		struct {
			uint32_t ssrc:4;
			uint32_t ssed_val:28;
		} __packed value;
		struct {
			uint32_t ssrc:4;
			uint32_t res:28;
		} __packed no_status_src;
		struct {
			uint32_t ssrc:4;
			uint32_t jmp:1;
			uint32_t res:11;
			uint32_t desc_idx:8;
			uint32_t cha_id:4;
			uint32_t err_id:4;
		} __packed ccb_status_src;
		struct {
			uint32_t ssrc:4;
			uint32_t jmp:1;
			uint32_t res:11;
			uint32_t desc_idx:8;
			uint32_t offset:8;
		} __packed jmp_halt_user_src;
		struct {
			uint32_t ssrc:4;
			uint32_t jmp:1;
			uint32_t res:11;
			uint32_t desc_idx:8;
			uint32_t desc_err:8;
		} __packed deco_src;
		struct {
			uint32_t ssrc:4;
			uint32_t res:17;
			uint32_t naddr:3;
			uint32_t desc_err:8;
		} __packed jr_src;
		struct {
			uint32_t ssrc:4;
			uint32_t jmp:1;
			uint32_t res:11;
			uint32_t desc_idx:8;
			uint32_t cond:8;
		} __packed jmp_halt_cond_src;
	} __packed error_desc;
} __packed;

 /* FUNCTION PROTOTYPES */

/*
 * @brief Initialize a job ring/channel in SEC device.
 * Write configuration register/s to properly initialize a job ring.
 *
 * @param [in] job_ring     The job ring
 *
 * @retval 0 for success
 * @retval other for error
 */
int hw_reset_job_ring(sec_job_ring_t *job_ring);

/*
 * @brief Reset a job ring/channel in SEC device.
 * Write configuration register/s to reset a job ring.
 *
 * @param [in] job_ring     The job ring
 *
 * @retval 0 for success
 * @retval -1 in case job ring reset failed
 */
int hw_shutdown_job_ring(sec_job_ring_t *job_ring);

/*
 * @brief Handle a job ring/channel error in SEC device.
 * Identify the error type and clear error bits if required.
 *
 * @param [in]  job_ring    The job ring
 * @param [in]  sec_error_code  error code as first read from SEC engine
 */

void hw_handle_job_ring_error(sec_job_ring_t *job_ring,
			      uint32_t sec_error_code);
/*
 * @brief Handle a job ring error in the device.
 * Identify the error type and printout a explanatory
 * messages.
 *
 * @param [in]  job_ring    The job ring
 *
 */

int hw_job_ring_error(sec_job_ring_t *job_ring);

/* @brief Set interrupt coalescing parameters on the Job Ring.
 * @param [in]  job_ring       The job ring
 * @param [in]  irq_coalesing_timer
 *                             Interrupt coalescing timer threshold.
 *                     This value determines the maximum
 *                     amount of time after processing a descriptor
 *                     before raising an interrupt.
 * @param [in]  irq_coalescing_count
 *                             Interrupt coalescing count threshold.
 *                     This value determines how many descriptors
 *                     are completed before raising an interrupt.
 */

int hw_job_ring_set_coalescing_param(sec_job_ring_t *job_ring,
				     uint16_t irq_coalescing_timer,
				     uint8_t irq_coalescing_count);

/* @brief Enable interrupt coalescing on a job ring
 * @param [in]  job_ring       The job ring
 */

int hw_job_ring_enable_coalescing(sec_job_ring_t *job_ring);

/*
 * @brief Disable interrupt coalescing on a job ring
 * @param [in]  job_ring       The job ring
 */

int hw_job_ring_disable_coalescing(sec_job_ring_t *job_ring);

/*
 * @brief Poll the HW for already processed jobs in the JR
 * and notify the available jobs to UA.
 *
 * @param [in]  job_ring            The job ring to poll.
 * @param [in]  limit               The maximum number of jobs to notify.
 *                                  If set to negative value, all available
 *                                  jobs are notified.
 *
 * @retval >=0 for No of jobs notified to UA.
 * @retval -1 for error
 */

int hw_poll_job_ring(struct sec_job_ring_t *job_ring, int32_t limit);

/* @brief Poll the HW for already processed jobs in the JR
 * and silently discard the available jobs or notify them to UA
 * with indicated error code.

 * @param [in,out]  job_ring        The job ring to poll.
 * @param [in]  do_notify           Can be #TRUE or #FALSE.
 *                                 Indicates if descriptors to be discarded
 *                                  or notified to UA with given error_code.
 * @param [in]  error_code          The detailed SEC error code.
 * @param [out] notified_descs        Number of notified descriptors.
 *                                 Can be NULL if do_notify is #FALSE
 */
void hw_flush_job_ring(struct sec_job_ring_t *job_ring,
		       uint32_t do_notify,
		       uint32_t error_code, uint32_t *notified_descs);

/*
 * @brief Flush job rings of any processed descs.
 * The processed descs are silently dropped,
 *  WITHOUT being notified to UA.
 */
void flush_job_rings(void);

/*
 * @brief Handle desc that generated error in SEC engine.
 * Identify the exact type of error and handle the error.
 * Depending on the error type, the job ring could be reset.
 * All descs that are submitted for processing on this job ring
 * are notified to User Application with error status and detailed error code.

 * @param [in]  job_ring            Job ring
 * @param [in]  sec_error_code      Error code read from job ring's Channel
 *                                 Status Register
 * @param [out] notified_descs      Number of notified descs. Can be NULL if
 *                                 do_notify is #FALSE
 * @param [out] do_driver_shutdown  If set to #TRUE, then UA is returned code
 *                                 #SEC_PROCESSING_ERROR
 *                                  which is indication that UA must call
 *                                  sec_release() after this.
 */
void sec_handle_desc_error(struct sec_job_ring_t *job_ring,
			   uint32_t sec_error_code,
			   uint32_t *notified_descs,
			   uint32_t *do_driver_shutdown);

/*
 * @brief Release the software and hardware resources tied to a job ring.
 * @param [in] job_ring The job ring
 * @retval  0 for success
 * @retval  -1 for error
 */
int shutdown_job_ring(struct sec_job_ring_t *job_ring);

/*
 * @brief Enable irqs on associated job ring.
 * @param [in] job_ring The job ring
 * @retval  0 for success
 * @retval  -1 for error
 */
int jr_enable_irqs(struct sec_job_ring_t *job_ring);

/*
 * @brief Disable irqs on associated job ring.
 * @param [in] job_ring The job ring
 * @retval  0 for success
 * @retval  -1 for error
 */
int jr_disable_irqs(struct sec_job_ring_t *job_ring);

 /*
  * IRJA - Input Ring Jobs Added Register shows
  * how many new jobs were added to the Input Ring.
  */
static inline void hw_enqueue_desc_on_job_ring(struct jobring_regs *regs,
					       int num)
{
	sec_out32(&regs->irja, num);
}

#endif /* _SEC_HW_SPECIFIC_H_ */

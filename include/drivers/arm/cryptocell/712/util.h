/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef UTIL_H
#define UTIL_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */

#ifdef __cplusplus
extern "C"
{
#endif

/************************ Defines ******************************/

/* invers the bytes on a word- used for output from HASH */
#ifdef BIG__ENDIAN
#define UTIL_INVERSE_UINT32_BYTES(val)	(val)
#else
#define UTIL_INVERSE_UINT32_BYTES(val) \
	(((val) >> 24) | (((val) & 0x00FF0000) >> 8) | (((val) & 0x0000FF00) << 8) | (((val) & 0x000000FF) << 24))
#endif

/* invers the bytes on a word - used for input data for HASH */
#ifdef BIG__ENDIAN
#define UTIL_REVERT_UINT32_BYTES(val) \
	(((val) >> 24) | (((val) & 0x00FF0000) >> 8) | (((val) & 0x0000FF00) << 8) | (((val) & 0x000000FF) << 24))
#else
#define UTIL_REVERT_UINT32_BYTES(val)	(val)
#endif

 /* ------------------------------------------------------------
 **
 * @brief This function executes a reverse bytes copying from one buffer to another buffer.
 *
 * @param[in] dst_ptr - The pointer to destination buffer.
 * @param[in] src_ptr - The pointer to source buffer.
 * @param[in] size    - The size in bytes.
 *
 */

void UTIL_ReverseMemCopy(uint8_t *dst_ptr, uint8_t *src_ptr, uint32_t size);


 /* ------------------------------------------------------------
  **
  * @brief This function executes a reversed byte copy on a specified buffer.
  *
  *        on a 6 byte byffer:
  *
  *        buff[5] <---> buff[0]
  *        buff[4] <---> buff[1]
  *        buff[3] <---> buff[2]
  *
  * @param[in] dst_ptr - The counter buffer.
  * @param[in] src_ptr - The counter size in bytes.
  *
  */
void UTIL_ReverseBuff(uint8_t *buff_ptr, uint32_t size);


#ifdef __cplusplus
}
#endif

#endif

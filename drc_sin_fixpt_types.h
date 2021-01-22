/*
 * File: drc_sin_fixpt_types.h
 *
 */
 // SPDX - License - Identifier: BSD - 3 - Clause
 //
 //Copyright(c) 2021 Intel Corporation.All rights reserved.
 //
 //Author : Shriram Shastry <malladi.sastry@linux.intel.com>
#ifndef DRC_SIN_FIXPT_TYPES_H
#define DRC_SIN_FIXPT_TYPES_H

/* Include Files */
#include <stdint.h>

/* Custom Header Code */
// SPDX - License - Identifier: BSD - 3 - Clause
//
//Copyright(c) 2021 Intel Corporation.All rights reserved.
//
//Author : Shriram Shastry <malladi.sastry@linux.intel.com>
/* Type Definitions */
#ifndef typedef_int96m_T
#define typedef_int96m_T

typedef struct {
  uint32_t chunks[3];
} int96m_T;

#endif                                 /*typedef_int96m_T*/

#ifndef typedef_int64m_T
#define typedef_int64m_T

typedef struct {
  uint32_t chunks[2];
} int64m_T;

#endif                                 /*typedef_int64m_T*/

#ifndef typedef_int128m_T
#define typedef_int128m_T

typedef struct {
  uint32_t chunks[4];
} int128m_T;

#endif                                 /*typedef_int128m_T*/
#endif

/*
 * File trailer for drc_sin_fixpt_types.h
 *
 * [EOF]
 */

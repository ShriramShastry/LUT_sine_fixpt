/*
 * File: drc_sin_fixpt.h
 *
 */
 // SPDX - License - Identifier: BSD - 3 - Clause
 //
 //Copyright(c) 2021 Intel Corporation.All rights reserved.
 //
 //Author : Shriram Shastry <malladi.sastry@linux.intel.com>

#ifndef DRC_SIN_FIXPT_H
#define DRC_SIN_FIXPT_H

/* Include Files */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

/* Custom Header Code */
// SPDX - License - Identifier: BSD - 3 - Clause
//
//Copyright(c) 2021 Intel Corporation.All rights reserved.
//
//Author : Shriram Shastry <malladi.sastry@linux.intel.com>
#ifdef __cplusplus

extern "C" {

#endif

  /* Function Declarations */
  extern void drc_sin_fixpt(const int32_t x[2001], int32_t y[2001]);
  extern void drc_sin_fixpt_initialize(void);
  extern void drc_sin_fixpt_terminate(void);
  extern void init_struc_fixpt(int32_t x[2001]);

#ifdef __cplusplus

}
#endif
#endif

/*
 * File trailer for drc_sin_fixpt.h
 *
 * [EOF]
 */

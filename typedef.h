/*
 * File: rtwtypes.h
 *
 */
 // SPDX - License - Identifier: BSD - 3 - Clause
 //
 //Copyright(c) 2021 Intel Corporation.All rights reserved.
 //
 //Author : Shriram Shastry <malladi.sastry@linux.intel.com>
#ifndef RTWTYPES_H
#define RTWTYPES_H

/* Include Files */
#include "stdint.h"
#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus

extern "C" {

#endif

  /*=======================================================================*
   * Target hardware information
   *   Device type: Generic->32-bit Embedded Processor
   *   Number of bits:     char:   8    short:   16    int:  32
   *                       long:  32
   *                       native word size:  32
   *   Byte ordering: LittleEndian
   *   Signed integer division rounds to: Floor
   *   Shift right on a signed integer as arithmetic shift: on
   *=======================================================================*/

  /*=======================================================================*
   * Fixed width word size data types:                                     *
   *   int8_t, int16_t, int32_t     - signed 8, 16, or 32 bit integers     *
   *   uint8_t, uint16_t, uint32_t  - unsigned 8, 16, or 32 bit integers   *
   *   real32_T, real64_T           - 32 and 64 bit floating point numbers *
   *=======================================================================*/
  typedef signed char int8_t;
  typedef unsigned char uint8_t;
  typedef short int16_t;
  typedef unsigned short uint16_t;
  typedef int int32_t;
  typedef unsigned int uint32_t;
  typedef float real32_T;
  typedef double real64_T;

  /*===========================================================================*
   * Generic type definitions: real_T, time_T, boolean_t, int_T, uint_T,       *
   *                           ulong_T, char_T and byte_T.                     *
   *===========================================================================*/
  typedef double real_T;
  typedef double time_T;
  typedef bool boolean_t;
  typedef int int_T;
  typedef unsigned int uint_T;
  typedef unsigned long ulong_T;
  typedef char char_T;
  typedef char_T byte_T;

  /*===========================================================================*
   * Complex number type definitions                                           *
   *===========================================================================*/
#define CREAL_T

  typedef struct {
    real32_T re;
    real32_T im;
  } creal32_T;

  typedef struct {
    real64_T re;
    real64_T im;
  } creal64_T;

  typedef struct {
    real_T re;
    real_T im;
  } creal_T;

  typedef struct {
    int8_t re;
    int8_t im;
  } cint8_T;

  typedef struct {
    uint8_t re;
    uint8_t im;
  } cuint8_T;

  typedef struct {
    int16_t re;
    int16_t im;
  } cint16_T;

  typedef struct {
    uint16_t re;
    uint16_t im;
  } cuint16_T;

  typedef struct {
    int32_t re;
    int32_t im;
  } cint32_T;

  typedef struct {
    uint32_t re;
    uint32_t im;
  } cuint32_T;

  /*=======================================================================*
   * Min and Max:                                                          *
   *   int8_t, int16_t, int32_t     - signed 8, 16, or 32 bit integers     *
   *   uint8_t, uint16_t, uint32_t  - unsigned 8, 16, or 32 bit integers   *
   *=======================================================================*/
#define MAX_int8_T                     ((int8_t)(127))
#define MIN_int8_T                     ((int8_t)(-128))
#define MAX_uint8_T                    ((uint8_t)(255))
#define MIN_uint8_T                    ((uint8_t)(0))
#define MAX_int16_T                    ((int16_t)(32767))
#define MIN_int16_T                    ((int16_t)(-32768))
#define MAX_uint16_T                   ((uint16_t)(65535))
#define MIN_uint16_T                   ((uint16_t)(0))
#define MAX_int32_T                    ((int32_t)(2147483647))
#define MIN_int32_T                    ((int32_t)(-2147483647-1))
#define MAX_uint32_T                   ((uint32_t)(0xFFFFFFFFU))
#define MIN_uint32_T                   ((uint32_t)(0))

  /* Logical type definitions */
#if (!defined(__cplusplus)) && (!defined(__true_false_are_keywords)) && (!defined(__bool_true_false_are_defined))
#ifndef false
#define false                          (0U)
#endif

#ifndef true
#define true                           (1U)
#endif
#endif

#ifdef __cplusplus

}
#endif
#endif

/*
 * File trailer for rtwtypes.h
 *
 * [EOF]
 */

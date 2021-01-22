/*
 * File: main.c
 *
/* Include Files */
// SPDX - License - Identifier: BSD - 3 - Clause
//
//Copyright(c) 2021 Intel Corporation.All rights reserved.
//
//Author : Shriram Shastry <malladi.sastry@linux.intel.com>
#pragma warning (disable : 4996)
#pragma warning (disable : 4013)

#include "main.h"
#include "drc_sin_fixpt.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Function Declarations */
static void argInit_1x2001_sfix32_En30(int32_t result[2001]);
static int32_t argInit_sfix32_En30(void);
static void main_drc_sin_fixpt(void);
static void main_init_struc_fixpt(void);

/* Function Definitions */
/*
 * Arguments    : int32_t result[2001]
 * Return Type  : void
 */
static void argInit_1x2001_sfix32_En30(int32_t result[2001])
{
  int32_t idx1;

  /* Loop over the array to initialize each element. */
  for (idx1 = 0; idx1 < 2001; idx1++) {
    /* Set the value of the array element.
       Change this value to the value that the application requires. */
    result[idx1] = argInit_sfix32_En30();
  }
}

/*
 * Arguments    : void
 * Return Type  : int32_t
 */
static int32_t argInit_sfix32_En30(void)
{
  return 0;
}

/*
 * Arguments    : void
 * Return Type  : void
 */
static void main_drc_sin_fixpt(void)
{
  int32_t iv[2001];
  int32_t y[2001];

  /* Initialize function 'drc_sin_fixpt' input arguments. */
  /* Initialize function input argument 'x'. */
  /* Call the entry-point 'drc_sin_fixpt'. */
  argInit_1x2001_sfix32_En30(iv);
  drc_sin_fixpt(iv, y);
}

/*
 * Arguments    : void
 * Return Type  : void
 */
static void main_init_struc_fixpt(void)
{
  int32_t x[2001];

  /* Call the entry-point 'init_struc_fixpt'. */
  init_struc_fixpt(x);
}

/*
 * Arguments    : int32_t argc
 *                const char * const argv[]
 * Return Type  : int32_t
 */
int32_t main(int32_t argc, const char* const argv[])
{
    (void)argc;
    (void)argv;



    int32_t x[2001];          /*Input  x = Q2.30*/
    int32_t y[2001], i;      /*Output y = Q6.26*/

    mkdir("Results", 0777);
    FILE* fd = fopen("Results/f_sin_fixed.txt", "w");
    fprintf(fd, " %15s %15s %15s\n ", "Index", "Inval-X[q2.30]", "Outval-Y[q2.30]");

    /* Call the entry-point 'init_struc_fixpt'.Initialize test vectiors */
    init_struc_fixpt(x);
    /*drc_sin_fixpt(x, y);*/
    for (i = 0; i < 2001; i++)
    {
        
        fprintf(fd, "%15d %15li %15li\n ", i, x[i], y[i]);
    }

    /* Terminate the application.
       You do not need to do this more than one time. */
    drc_sin_fixpt_terminate();
    return 0;
}


/*
 * File trailer for main.c
 *
 * [EOF]
 */

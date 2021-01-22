/*
 * File: drc_sin_fixpt.c
 *
 */
 /* Custom Source Code */
 // SPDX - License - Identifier: BSD - 3 - Clause
 //
 //Copyright(c) 2021 Intel Corporation.All rights reserved.
 //
 //Author : Shriram Shastry <malladi.sastry@linux.intel.com>
/* Include Files */
#include "drc_sin_fixpt.h"
#include "drc_sin_fixpt_types.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "typedef.h"


/* Variable Definitions */
#define DEBUG_ON 1 
#ifdef DEBUG_ON
static int16_t FI_SIN_COS_LUT[256];
#else
static int32_t FI_SIN_COS_LUT[128];
#endif
static boolean_t isInitialized_drc_sin_fixpt = false;

/* Function Declarations */
static boolean_t MultiWord2Bool(const uint32_t u[], int32_t n);
static int32_t MultiWord2sLong(const uint32_t u[]);
static uint32_t MultiWord2uLong(const uint32_t u[]);
static void MultiWordNeg(const uint32_t u1[], uint32_t y[], int32_t n);
static void MultiWordSetSignedMax(uint32_t y[], int32_t n);
static void MultiWordSetSignedMin(uint32_t y[], int32_t n);
static void MultiWordSignedWrap(const uint32_t u1[], int32_t n1, uint32_t n2,
  uint32_t y[]);
static void MultiWordSub(const uint32_t u1[], const uint32_t u2[], uint32_t y[],
  int32_t n);
static int32_t asr_s32(int32_t u, uint32_t n);
static int32_t mul_ssu32_loSR(int32_t a, uint32_t b, uint32_t aShift);
static uint32_t mul_u32_loSR(uint32_t a, uint32_t b, uint32_t aShift);
static void mul_wide_su32(int32_t in0, uint32_t in1, uint32_t *ptrOutBitsHi,
  uint32_t *ptrOutBitsLo);
static void mul_wide_u32(uint32_t in0, uint32_t in1, uint32_t *ptrOutBitsHi,
  uint32_t *ptrOutBitsLo);
static void sMultiWord2MultiWord(const uint32_t u1[], int32_t n1, uint32_t y[],
  int32_t n);
static void sMultiWordDivFloor(const uint32_t u1[], int32_t n1, const uint32_t
  u2[], int32_t n2, uint32_t b_y1[], int32_t m1, uint32_t y2[], int32_t m2,
  uint32_t t1[], int32_t l1, uint32_t t2[], int32_t l2);
static void sMultiWordShl(const uint32_t u1[], int32_t n1, uint32_t n2, uint32_t
  y[], int32_t n);
static void sMultiWordShr(const uint32_t u1[], int32_t n1, uint32_t n2, uint32_t
  y[], int32_t n);
static void sin_init(void);
static void ssuMultiWordMul(const uint32_t u1[], int32_t n1, const uint32_t u2[],
  int32_t n2, uint32_t y[], int32_t n);
static void uLong2MultiWord(uint32_t u, uint32_t y[], int32_t n);
static int32_t uMultiWordDiv(uint32_t a[], int32_t na, uint32_t b[], int32_t nb,
  uint32_t q[], int32_t nq, uint32_t r[], int32_t nr);
static void uMultiWordInc(uint32_t y[], int32_t n);

/* Function Definitions */
/*
 * Arguments    : const uint32_t u[]
 *                int32_t n
 * Return Type  : boolean_t
 */
static boolean_t MultiWord2Bool(const uint32_t u[], int32_t n)
{
  int32_t i;
  boolean_t y;
  y = false;
  i = 0;
  while ((i < n) && (!y)) {
    if (u[i] != 0U) {
      y = true;
    }

    i++;
  }

  return y;
}

/*
 * Arguments    : const uint32_t u[]
 * Return Type  : int32_t
 */
static int32_t MultiWord2sLong(const uint32_t u[])
{
  return (int32_t)u[0];
}

/*
 * Arguments    : const uint32_t u[]
 * Return Type  : uint32_t
 */
static uint32_t MultiWord2uLong(const uint32_t u[])
{
  return u[0];
}

/*
 * Arguments    : const uint32_t u1[]
 *                uint32_t y[]
 *                int32_t n
 * Return Type  : void
 */
static void MultiWordNeg(const uint32_t u1[], uint32_t y[], int32_t n)
{
  int32_t carry = 1;
  int32_t i;
  uint32_t yi;
  for (i = 0; i < n; i++) {
    yi = (~u1[i]) + ((uint32_t)carry);
    y[i] = yi;
    carry = (int32_t)((yi < ((uint32_t)carry)) ? 1 : 0);
  }
}

/*
 * Arguments    : uint32_t y[]
 *                int32_t n
 * Return Type  : void
 */
static void MultiWordSetSignedMax(uint32_t y[], int32_t n)
{
  int32_t i;
  int32_t n1;
  n1 = n - 1;
  for (i = 0; i < n1; i++) {
    y[i] = MAX_uint32_T;
  }

  y[n - 1] = 2147483647U;
}

/*
 * Arguments    : uint32_t y[]
 *                int32_t n
 * Return Type  : void
 */
static void MultiWordSetSignedMin(uint32_t y[], int32_t n)
{
  int32_t n1;
  n1 = n - 1;
  if (0 <= (n1 - 1)) {
    memset(&y[0], 0, ((uint32_t)n1) * (sizeof(uint32_t)));
  }

  y[n - 1] = 2147483648U;
}

/*
 * Arguments    : const uint32_t u1[]
 *                int32_t n1
 *                uint32_t n2
 *                uint32_t y[]
 * Return Type  : void
 */
static void MultiWordSignedWrap(const uint32_t u1[], int32_t n1, uint32_t n2,
  uint32_t y[])
{
  int32_t n1m1;
  uint32_t mask;
  uint32_t ys;
  n1m1 = n1 - 1;
  if (0 <= (n1m1 - 1)) {
    memcpy(&y[0], &u1[0], ((uint32_t)n1m1) * (sizeof(uint32_t)));
  }

  mask = (1U << (31U - n2));
  if ((u1[n1 - 1] & mask) != 0U) {
    ys = MAX_uint32_T;
  } else {
    ys = 0U;
  }

  mask = (mask << 1U) - 1U;
  y[n1 - 1] = (u1[n1 - 1] & mask) | ((~mask) & ys);
}

/*
 * Arguments    : const uint32_t u1[]
 *                const uint32_t u2[]
 *                uint32_t y[]
 *                int32_t n
 * Return Type  : void
 */
static void MultiWordSub(const uint32_t u1[], const uint32_t u2[], uint32_t y[],
  int32_t n)
{
  int32_t borrow = 0;
  int32_t i;
  uint32_t u1i;
  uint32_t yi;
  for (i = 0; i < n; i++) {
    u1i = u1[i];
    yi = (u1i - u2[i]) - ((uint32_t)borrow);
    y[i] = yi;
    if (((uint32_t)borrow) != 0U) {
      borrow = (int32_t)((yi >= u1i) ? 1 : 0);
    } else {
      borrow = (int32_t)((yi > u1i) ? 1 : 0);
    }
  }
}

/*
 * Arguments    : int32_t u
 *                uint32_t n
 * Return Type  : int32_t
 * casts a signed integer to an unsigned integer, and then right shifts the unsigned integer
 */
static int32_t asr_s32(int32_t u, uint32_t n)
{
  int32_t y;
  if (u >= 0) {
    y = (int32_t)((uint32_t)(((uint32_t)u) >> n));
  } else {
    y = (-((int32_t)((uint32_t)(((uint32_t)((int32_t)(-1 - u))) >> n)))) - 1;
  }

  return y;
}

/*
 * Arguments    : int32_t a
 *                uint32_t b
 *                uint32_t aShift
 * Return Type  : int32_t
 */
static int32_t mul_ssu32_loSR(int32_t a, uint32_t b, uint32_t aShift)
{
  uint32_t u32_chi;
  uint32_t u32_clo;
  mul_wide_su32(a, b, &u32_chi, &u32_clo);
  u32_clo = (u32_chi << (32U - aShift)) | (u32_clo >> aShift);
  return (int32_t)u32_clo;
}

/*
 * Arguments    : uint32_t a
 *                uint32_t b
 *                uint32_t aShift
 * Return Type  : uint32_t
 */
static uint32_t mul_u32_loSR(uint32_t a, uint32_t b, uint32_t aShift)
{
  uint32_t result;
  uint32_t u32_chi;
  mul_wide_u32(a, b, &u32_chi, &result);
  return (u32_chi << (32U - aShift)) | (result >> aShift);
}

/*
 * Arguments    : int32_t in0
 *                uint32_t in1
 *                uint32_t *ptrOutBitsHi
 *                uint32_t *ptrOutBitsLo
 * Return Type  : void
 */
static void mul_wide_su32(int32_t in0, uint32_t in1, uint32_t *ptrOutBitsHi,
  uint32_t *ptrOutBitsLo)
{
  int32_t in0Hi;
  int32_t in0Lo;
  int32_t in1Hi;
  int32_t in1Lo;
  uint32_t absIn0;
  uint32_t outBitsLo;
  uint32_t productLoHi;
  uint32_t productLoLo;
  if (in0 < 0) {
    absIn0 = (~((uint32_t)in0)) + 1U;
  } else {
    absIn0 = (uint32_t)in0;
  }

  in0Hi = (int32_t)((uint32_t)(absIn0 >> 16U));
  in0Lo = (int32_t)((uint32_t)(absIn0 & 65535U));
  in1Hi = (int32_t)((uint32_t)(in1 >> 16U));
  in1Lo = (int32_t)((uint32_t)(in1 & 65535U));
  absIn0 = ((uint32_t)in0Hi) * ((uint32_t)in1Lo);
  productLoHi = ((uint32_t)in0Lo) * ((uint32_t)in1Hi);
  productLoLo = ((uint32_t)in0Lo) * ((uint32_t)in1Lo);
  in0Lo = 0;
  outBitsLo = productLoLo + (productLoHi << 16U);
  if (outBitsLo < productLoLo) {
    in0Lo = 1;
  }

  productLoLo = outBitsLo;
  outBitsLo += (absIn0 << 16U);
  if (outBitsLo < productLoLo) {
    in0Lo = (int32_t)((uint32_t)(((uint32_t)in0Lo) + 1U));
  }

  absIn0 = ((((uint32_t)in0Lo) + (((uint32_t)in0Hi) * ((uint32_t)in1Hi))) +
            (productLoHi >> 16U)) + (absIn0 >> 16U);
  if ((in1 != 0U) && (in0 < 0)) {
    absIn0 = ~absIn0;
    outBitsLo = ~outBitsLo;
    outBitsLo++;
    if (outBitsLo == 0U) {
      absIn0++;
    }
  }

  *ptrOutBitsHi = absIn0;
  *ptrOutBitsLo = outBitsLo;
}

/*
 * Arguments    : uint32_t in0
 *                uint32_t in1
 *                uint32_t *ptrOutBitsHi
 *                uint32_t *ptrOutBitsLo
 * Return Type  : void
 */
static void mul_wide_u32(uint32_t in0, uint32_t in1, uint32_t *ptrOutBitsHi,
  uint32_t *ptrOutBitsLo)
{
  int32_t in0Hi;
  int32_t in0Lo;
  int32_t in1Hi;
  int32_t in1Lo;
  uint32_t outBitsLo;
  uint32_t productHiLo;
  uint32_t productLoHi;
  uint32_t productLoLo;
  in0Hi = (int32_t)((uint32_t)(in0 >> 16U));
  in0Lo = (int32_t)((uint32_t)(in0 & 65535U));
  in1Hi = (int32_t)((uint32_t)(in1 >> 16U));
  in1Lo = (int32_t)((uint32_t)(in1 & 65535U));
  productHiLo = ((uint32_t)in0Hi) * ((uint32_t)in1Lo);
  productLoHi = ((uint32_t)in0Lo) * ((uint32_t)in1Hi);
  productLoLo = ((uint32_t)in0Lo) * ((uint32_t)in1Lo);
  in0Lo = 0;
  outBitsLo = productLoLo + (productLoHi << 16U);
  if (outBitsLo < productLoLo) {
    in0Lo = 1;
  }

  productLoLo = outBitsLo;
  outBitsLo += (productHiLo << 16U);
  if (outBitsLo < productLoLo) {
    in0Lo = (int32_t)((uint32_t)(((uint32_t)in0Lo) + 1U));
  }

  *ptrOutBitsHi = ((((uint32_t)in0Lo) + (((uint32_t)in0Hi) * ((uint32_t)in1Hi)))
                   + (productLoHi >> 16U)) + (productHiLo >> 16U);
  *ptrOutBitsLo = outBitsLo;
}

/*
 * Arguments    : const uint32_t u1[]
 *                int32_t n1
 *                uint32_t y[]
 *                int32_t n
 * Return Type  : void
 */
static void sMultiWord2MultiWord(const uint32_t u1[], int32_t n1, uint32_t y[],
  int32_t n)
{
  int32_t i;
  int32_t nm;
  uint32_t u1i;
  if (n1 < n) {
    nm = n1;
  } else {
    nm = n;
  }

  if (0 <= (nm - 1)) {
    memcpy(&y[0], &u1[0], ((uint32_t)nm) * (sizeof(uint32_t)));
  }

  if (n > n1) {
    if ((u1[n1 - 1] & 2147483648U) != 0U) {
      u1i = MAX_uint32_T;
    } else {
      u1i = 0U;
    }

    for (i = nm; i < n; i++) {
      y[i] = u1i;
    }
  }
}

/*
 * Arguments    : const uint32_t u1[]
 *                int32_t n1
 *                const uint32_t u2[]
 *                int32_t n2
 *                uint32_t b_y1[]
 *                int32_t m1
 *                uint32_t y2[]
 *                int32_t m2
 *                uint32_t t1[]
 *                int32_t l1
 *                uint32_t t2[]
 *                int32_t l2
 * Return Type  : void
 */
static void sMultiWordDivFloor(const uint32_t u1[], int32_t n1, const uint32_t
  u2[], int32_t n2, uint32_t b_y1[], int32_t m1, uint32_t y2[], int32_t m2,
  uint32_t t1[], int32_t l1, uint32_t t2[], int32_t l2)
{
  boolean_t denNeg;
  boolean_t numNeg;
  numNeg = ((u1[n1 - 1] & 2147483648U) != 0U);
  denNeg = ((u2[n2 - 1] & 2147483648U) != 0U);
  if (numNeg) {
    MultiWordNeg(u1, t1, n1);
  } else {
    sMultiWord2MultiWord(u1, n1, t1, l1);
  }

  if (denNeg) {
    MultiWordNeg(u2, t2, n2);
  } else {
    sMultiWord2MultiWord(u2, n2, t2, l2);
  }

  if (uMultiWordDiv(t1, l1, t2, l2, b_y1, m1, y2, m2) < 0) {
    if (numNeg) {
      MultiWordSetSignedMin(b_y1, m1);
    } else {
      MultiWordSetSignedMax(b_y1, m1);
    }
  } else {
    if (numNeg != denNeg) {
      if (MultiWord2Bool(y2, m2)) {
        uMultiWordInc(b_y1, m1);
      }

      MultiWordNeg(b_y1, b_y1, m1);
    }
  }
}

/*
 * Arguments    : const uint32_t u1[]
 *                int32_t n1
 *                uint32_t n2
 *                uint32_t y[]
 *                int32_t n
 * Return Type  : void
 */
static void sMultiWordShl(const uint32_t u1[], int32_t n1, uint32_t n2, uint32_t
  y[], int32_t n)
{
  int32_t i;
  int32_t nb;
  int32_t nc;
  uint32_t nl;
  uint32_t u1i;
  uint32_t yi;
  uint32_t ys;
  nb = ((int32_t)n2) / 32;
  if ((u1[n1 - 1] & 2147483648U) != 0U) {
    ys = MAX_uint32_T;
  } else {
    ys = 0U;
  }

  if (nb > n) {
    nc = n;
  } else {
    nc = nb;
  }

  u1i = 0U;
  if (0 <= (nc - 1)) {
    memset(&y[0], 0, ((uint32_t)nc) * (sizeof(uint32_t)));
  }

  for (i = 0; i < nc; i++) {
  }

  if (nb < n) {
    nl = n2 - (((uint32_t)nb) * 32U);
    nb += n1;
    if (nb > n) {
      nb = n;
    }

    nb -= i;
    if (nl > 0U) {
      for (nc = 0; nc < nb; nc++) {
        yi = (u1i >> (32U - nl));
        u1i = u1[nc];
        y[i] = yi | (u1i << nl);
        i++;
      }

      if (i < n) {
        y[i] = (u1i >> (32U - nl)) | (ys << nl);
        i++;
      }
    } else {
      for (nc = 0; nc < nb; nc++) {
        y[i] = u1[nc];
        i++;
      }
    }
  }

  while (i < n) {
    y[i] = ys;
    i++;
  }
}

/*
 * Arguments    : const uint32_t u1[]
 *                int32_t n1
 *                uint32_t n2
 *                uint32_t y[]
 *                int32_t n
 * Return Type  : void
 */
static void sMultiWordShr(const uint32_t u1[], int32_t n1, uint32_t n2, uint32_t
  y[], int32_t n)
{
  int32_t i;
  int32_t i1;
  int32_t nb;
  int32_t nc;
  uint32_t nr;
  uint32_t u1i;
  uint32_t yi;
  uint32_t ys;
  nb = ((int32_t)n2) / 32;
  i = 0;
  if ((u1[n1 - 1] & 2147483648U) != 0U) {
    ys = MAX_uint32_T;
  } else {
    ys = 0U;
  }

  if (nb < n1) {
    nc = n + nb;
    if (nc > n1) {
      nc = n1;
    }

    nr = n2 - (((uint32_t)nb) * 32U);
    if (nr > 0U) {
      u1i = u1[nb];
      for (i1 = nb + 1; i1 < nc; i1++) {
        yi = (u1i >> nr);
        u1i = u1[i1];
        y[i] = yi | (u1i << (32U - nr));
        i++;
      }

      if (nc < n1) {
        yi = u1[nc];
      } else {
        yi = ys;
      }

      y[i] = (u1i >> nr) | (yi << (32U - nr));
      i++;
    } else {
      for (i1 = nb; i1 < nc; i1++) {
        y[i] = u1[i1];
        i++;
      }
    }
  }

  while (i < n) {
    y[i] = ys;
    i++;
  }
}

/*
 * Arguments    : void
 * Return Type  : void
 */
static void sin_init(void)
{
#ifdef DEBUG_ON
  static const int16_t iv[256] = { 0, 804, 1608, 2411, 3212, 4011, 4808, 5602,
    6393, 7180, 7962, 8740, 9512, 10279, 11039, 11793, 12540, 13279, 14010,
    14733, 15447, 16151, 16846, 17531, 18205, 18868, 19520, 20160, 20788, 21403,
    22006, 22595, 23170, 23732, 24279, 24812, 25330, 25833, 26320, 26791, 27246,
    27684, 28106, 28511, 28899, 29269, 29622, 29957, 30274, 30572, 30853, 31114,
    31357, 31581, 31786, 31972, 32138, 32286, 32413, 32522, 32610, 32679, 32729,
    32758, MAX_int16_T, 32758, 32729, 32679, 32610, 32522, 32413, 32286, 32138,
    31972, 31786, 31581, 31357, 31114, 30853, 30572, 30274, 29957, 29622, 29269,
    28899, 28511, 28106, 27684, 27246, 26791, 26320, 25833, 25330, 24812, 24279,
    23732, 23170, 22595, 22006, 21403, 20788, 20160, 19520, 18868, 18205, 17531,
    16846, 16151, 15447, 14733, 14010, 13279, 12540, 11793, 11039, 10279, 9512,
    8740, 7962, 7180, 6393, 5602, 4808, 4011, 3212, 2411, 1608, 804, 0, -804,
    -1608, -2411, -3212, -4011, -4808, -5602, -6393, -7180, -7962, -8740, -9512,
    -10279, -11039, -11793, -12540, -13279, -14010, -14733, -15447, -16151,
    -16846, -17531, -18205, -18868, -19520, -20160, -20788, -21403, -22006,
    -22595, -23170, -23732, -24279, -24812, -25330, -25833, -26320, -26791,
    -27246, -27684, -28106, -28511, -28899, -29269, -29622, -29957, -30274,
    -30572, -30853, -31114, -31357, -31581, -31786, -31972, -32138, -32286,
    -32413, -32522, -32610, -32679, -32729, -32758, -32767, -32758, -32729,
    -32679, -32610, -32522, -32413, -32286, -32138, -31972, -31786, -31581,
    -31357, -31114, -30853, -30572, -30274, -29957, -29622, -29269, -28899,
    -28511, -28106, -27684, -27246, -26791, -26320, -25833, -25330, -24812,
    -24279, -23732, -23170, -22595, -22006, -21403, -20788, -20160, -19520,
    -18868, -18205, -17531, -16846, -16151, -15447, -14733, -14010, -13279,
    -12540, -11793, -11039, -10279, -9512, -8740, -7962, -7180, -6393, -5602,
    -4808, -4011, -3212, -2411, -1608, -804 };
#else
static const int32_t iv[128] = {52690944, 158008904, 262868108, 367137480,
                                470554873, 572792602, 673654056, 772877087,
                                870265084, 965555898, 1058487383, 1148928462,
                                1236551453, 1321225280, 1402687796, 1480807926,
                                1555323522, 1626103511, 1693016818, 1755801296,
                                1814325870, 1868525002, 1918202083, 1963291574,
                                2003596866, 2039117957, 2069723773, 2095348778,
                                2115927434, 2131394205, 2141683554, 2146861017,
                                2146861055, 2141683673, 2131394402, 2115927709,
                                2095349130, 2069724202, 2039118461, 2003597445,
                                1963292226, 1918202806, 1868525795, 1814326730,
                                1755802222, 1693017808, 1626104562, 1555324631,
                                1480809090, 1402689014, 1321226548, 1236552768,
                                1148929821, 1058488782, 965557335, 870266554,
                                772878588,  673655583,  572794152, 470556442,
                                367139065,  262869704,  158010508, 52692552,
                                -52690944, -157943368, -262802572, -367071944,
                                -470489337, -572727066, -673588520, -772811551,
                                -870199548, -965490362, -1058421847, -1148862926,
                                -1236485917, -1321159744, -1402622260, -1480742390,
                                -1555257986, -1626037975, -1692951282, -1755735760,
                                -1814260334, -1868459466, -1918136547, -1963226038,
                                -2003531330, -2039052421, -2069658237, -2095283242,
                                -2115861898, -2131328669, -2141618018, -2146795481,
                                -2146795519, -2141618137, -2131328866, -2115862173,
                                -2095283594, -2069658666, -2039052925, -2003531909,
                                -1963226690, -1918137270, -1868460259, -1814261194,
                                -1755736686, -1692952272, -1626039026, -1555259095,
                                -1480743554, -1402623478, -1321161012, -1236487232,
                                -1148864285, -1058423246, -965491799, -870201018,
                                -772813052, -673590047, -572728616, -470490906,
                                -367073529, -262804168, -157944972, -52627016};
#endif
#ifdef DEBUG_ON
memcpy(&FI_SIN_COS_LUT[0], &iv[0], 256U * (sizeof(int16_t)));
#else
memcpy(&FI_SIN_COS_LUT[0], &iv[0], 128U * (sizeof(int32_t)));
#endif // DEBUG_ON

  
  
}

/*
 * Arguments    : const uint32_t u1[]
 *                int32_t n1
 *                const uint32_t u2[]
 *                int32_t n2
 *                uint32_t y[]
 *                int32_t n
 * Return Type  : void
 */
static void ssuMultiWordMul(const uint32_t u1[], int32_t n1, const uint32_t u2[],
  int32_t n2, uint32_t y[], int32_t n)
{
  int32_t a0;
  int32_t a1;
  int32_t b0;
  int32_t b1;
  int32_t cb1;
  int32_t i;
  int32_t j;
  int32_t k;
  int32_t ni;
  uint32_t cb;
  uint32_t t;
  uint32_t u1i;
  uint32_t w01;
  uint32_t yk;
  boolean_t isNegative1;
  isNegative1 = ((u1[n1 - 1] & 2147483648U) != 0U);
  cb1 = 1;

  /* Initialize output to zero */
  if (0 <= (n - 1)) {
    memset(&y[0], 0, ((uint32_t)n) * (sizeof(uint32_t)));
  }

  for (i = 0; i < n1; i++) {
    cb = 0U;
    u1i = u1[i];
    if (isNegative1) {
      u1i = (~u1i) + ((uint32_t)cb1);
      cb1 = (int32_t)((u1i < ((uint32_t)cb1)) ? 1 : 0);
    }

    a1 = (int32_t)((uint32_t)(u1i >> 16U));
    a0 = (int32_t)((uint32_t)(u1i & 65535U));
    ni = n - i;
    if (n2 <= ni) {
      ni = n2;
    }

    k = i;
    for (j = 0; j < ni; j++) {
      u1i = u2[j];
      b1 = (int32_t)((uint32_t)(u1i >> 16U));
      b0 = (int32_t)((uint32_t)(u1i & 65535U));
      u1i = ((uint32_t)a1) * ((uint32_t)b0);
      w01 = ((uint32_t)a0) * ((uint32_t)b1);
      yk = y[k] + cb;
      cb = (uint32_t)((yk < cb) ? 1 : 0);
      t = ((uint32_t)a0) * ((uint32_t)b0);
      yk += t;
      cb += (uint32_t)((yk < t) ? 1 : 0);
      t = (u1i << 16U);
      yk += t;
      cb += (uint32_t)((yk < t) ? 1 : 0);
      t = (w01 << 16U);
      yk += t;
      cb += (uint32_t)((yk < t) ? 1 : 0);
      y[k] = yk;
      cb += (u1i >> 16U);
      cb += (w01 >> 16U);
      cb += ((uint32_t)a1) * ((uint32_t)b1);
      k++;
    }

    if (k < n) {
      y[k] = cb;
    }
  }

  /* Apply sign */
  if (isNegative1) {
    cb = 1U;
    for (k = 0; k < n; k++) {
      yk = (~y[k]) + cb;
      y[k] = yk;
      cb = (uint32_t)((yk < cb) ? 1 : 0);
    }
  }
}

/*
 * Arguments    : uint32_t u
 *                uint32_t y[]
 *                int32_t n
 * Return Type  : void
 */
static void uLong2MultiWord(uint32_t u, uint32_t y[], int32_t n)
{
  y[0] = u;
  if (1 <= (n - 1)) {
    memset(&y[1], 0, ((uint32_t)((int32_t)(n + -1))) * (sizeof(uint32_t)));
  }
}

/*
 * Arguments    : uint32_t a[]
 *                int32_t na
 *                uint32_t b[]
 *                int32_t nb
 *                uint32_t q[]
 *                int32_t nq
 *                uint32_t r[]
 *                int32_t nr
 * Return Type  : int32_t
 */
static int32_t uMultiWordDiv(uint32_t a[], int32_t na, uint32_t b[], int32_t nb,
  uint32_t q[], int32_t nq, uint32_t r[], int32_t nr)
{
  int32_t ka;
  int32_t kb;
  int32_t na1;
  int32_t nb1;
  int32_t nza;
  int32_t nzb;
  int32_t tpi;
  int32_t y;
  uint32_t ak;
  uint32_t bk;
  uint32_t kba;
  uint32_t kbb;
  uint32_t mask;
  uint32_t nba;
  uint32_t nbb;
  uint32_t nbq;
  uint32_t t;
  uint32_t tnb;
  nzb = nb;
  tpi = nb - 1;
  while ((nzb > 0) && (b[tpi] == 0U)) {
    nzb--;
    tpi--;
  }

  if (nzb > 0) {
    nza = na;
    if (0 <= (nq - 1)) {
      memset(&q[0], 0, ((uint32_t)nq) * (sizeof(uint32_t)));
    }

    tpi = na - 1;
    while ((nza > 0) && (a[tpi] == 0U)) {
      nza--;
      tpi--;
    }

    if ((nza > 0) && (nza >= nzb)) {
      nb1 = nzb - 1;
      na1 = nza - 1;
      if (0 <= (nr - 1)) {
        memset(&r[0], 0, ((uint32_t)nr) * (sizeof(uint32_t)));
      }

      /* Quick return if dividend and divisor fit into single word. */
      if (nza == 1) {
        ak = a[0];
        bk = b[0];
        nbq = ak / bk;
        q[0] = nbq;
        r[0] = ak - (nbq * bk);
        y = 7;
      } else {
        /* Remove leading zeros from both, dividend and divisor. */
        kbb = 1U;
        t = (b[nzb - 1] >> 1U);
        while (t != 0U) {
          kbb++;
          t >>= 1U;
        }

        kba = 1U;
        t = (a[nza - 1] >> 1U);
        while (t != 0U) {
          kba++;
          t >>= 1U;
        }

        /* Quick return if quotient is zero. */
        if ((nza > nzb) || (kba >= kbb)) {
          nba = (((uint32_t)((int32_t)(nza - 1))) * 32U) + kba;
          nbb = (((uint32_t)((int32_t)(nzb - 1))) * 32U) + kbb;

          /* Normalize b. */
          if (kbb != 32U) {
            bk = b[nzb - 1];
            for (kb = nzb - 1; kb > 0; kb--) {
              t = (bk << (32U - kbb));
              bk = b[kb - 1];
              t |= (bk >> kbb);
              b[kb] = t;
            }

            b[0] = (bk << (32U - kbb));
            mask = ~((1U << (32U - kbb)) - 1U);
          } else {
            mask = MAX_uint32_T;
          }

          /* Initialize quotient to zero. */
          tnb = 0U;
          y = 0;

          /* Until exit conditions have been met, do */
          do {
            /* Normalize a */
            if (kba != 32U) {
              tnb = (tnb - kba) + 32U;
              ak = a[na1];
              for (ka = na1; ka > 0; ka--) {
                t = (ak << (32U - kba));
                ak = a[ka - 1];
                t |= (ak >> kba);
                a[ka] = t;
              }

              a[0] = (ak << (32U - kba));
            }

            /* Compare b against the a. */
            ak = a[na1];
            bk = b[nzb - 1];
            if ((nzb - 1) == 0) {
              t = mask;
            } else {
              t = MAX_uint32_T;
            }

            if ((ak & t) == bk) {
              tpi = 0;
              ka = na1;
              kb = nzb - 1;
              while ((tpi == 0) && (kb > 0)) {
                ka--;
                ak = a[ka];
                kb--;
                bk = b[kb];
                if (kb == 0) {
                  t = mask;
                } else {
                  t = MAX_uint32_T;
                }

                if ((ak & t) != bk) {
                  if (ak > bk) {
                    tpi = 1;
                  } else {
                    tpi = -1;
                  }
                }
              }
            } else if (ak > bk) {
              tpi = 1;
            } else {
              tpi = -1;
            }

            /* If the remainder in a is still greater or equal to b, subtract normalized divisor from a. */
            if ((tpi >= 0) || (nba > nbb)) {
              nbq = nba - nbb;

              /* If the remainder and the divisor are equal, set remainder to zero. */
              if (tpi == 0) {
                ka = na1;
                for (kb = nzb - 1; kb > 0; kb--) {
                  a[ka] = 0U;
                  ka--;
                }

                a[ka] -= b[0];
              } else {
                /* Otherwise, subtract the divisor from the remainder */
                if (tpi < 0) {
                  ak = a[na1];
                  kba = 31U;
                  for (ka = na1; ka > 0; ka--) {
                    t = (ak << 1U);
                    ak = a[ka - 1];
                    t |= (ak >> 31U);
                    a[ka] = t;
                  }

                  a[0] = (ak << 1U);
                  tnb++;
                  nbq--;
                }

                tpi = 0;
                ka = (na1 - nzb) + 1;
                for (kb = 0; kb < nzb; kb++) {
                  t = a[ka];
                  ak = (t - b[kb]) - ((uint32_t)tpi);
                  if (((uint32_t)tpi) != 0U) {
                    tpi = (int32_t)((ak >= t) ? 1 : 0);
                  } else {
                    tpi = (int32_t)((ak > t) ? 1 : 0);
                  }

                  a[ka] = ak;
                  ka++;
                }
              }

              /* Update the quotient. */
              tpi = ((int32_t)nbq) / 32;
              q[tpi] |= (1U << (nbq - (((uint32_t)tpi) * 32U)));

              /* Remove leading zeros from the remainder and check whether the exit conditions have been met. */
              tpi = na1;
              while ((nza > 0) && (a[tpi] == 0U)) {
                nza--;
                tpi--;
              }

              if (nza >= nzb) {
                na1 = nza - 1;
                kba = 1U;
                t = (a[nza - 1] >> 1U);
                while (t != 0U) {
                  kba++;
                  t >>= 1U;
                }

                nba = ((((uint32_t)((int32_t)(nza - 1))) * 32U) + kba) - tnb;
                if (nba < nbb) {
                  y = 2;
                }
              } else if (nza == 0) {
                y = 1;
              } else {
                na1 = nza - 1;
                y = 4;
              }
            } else {
              y = 3;
            }
          } while (y == 0);

          /* Return the remainder. */
          if (y == 1) {
            r[0] = a[0];
          } else {
            tpi = ((int32_t)tnb) / 32;
            nbq = tnb - (((uint32_t)tpi) * 32U);
            if (nbq == 0U) {
              ka = tpi;
              for (nzb = 0; nzb <= nb1; nzb++) {
                r[nzb] = a[ka];
                ka++;
              }
            } else {
              ak = a[tpi];
              nzb = 0;
              for (ka = tpi + 1; ka <= na1; ka++) {
                t = (ak >> nbq);
                ak = a[ka];
                t |= (ak << (32U - nbq));
                r[nzb] = t;
                nzb++;
              }

              r[nzb] = (ak >> nbq);
            }
          }

          /* Restore b. */
          if (kbb != 32U) {
            bk = b[0];
            for (kb = 0; kb < nb1; kb++) {
              t = (bk >> (32U - kbb));
              bk = b[kb + 1];
              t |= (bk << kbb);
              b[kb] = t;
            }

            b[kb] = (bk >> (32U - kbb));
          }
        } else {
          if (0 <= (nr - 1)) {
            memcpy(&r[0], &a[0], ((uint32_t)nr) * (sizeof(uint32_t)));
          }

          y = 6;
        }
      }
    } else {
      if (0 <= (nr - 1)) {
        memcpy(&r[0], &a[0], ((uint32_t)nr) * (sizeof(uint32_t)));
      }

      y = 5;
    }
  } else {
    y = -1;
  }

  return y;
}

/*
 * Arguments    : uint32_t y[]
 *                int32_t n
 * Return Type  : void
 */
static void uMultiWordInc(uint32_t y[], int32_t n)
{
  int32_t carry = 1;
  int32_t i;
  uint32_t yi;
  for (i = 0; i < n; i++) {
    yi = y[i] + ((uint32_t)carry);
    y[i] = yi;
    carry = (int32_t)((yi < ((uint32_t)carry)) ? 1 : 0);
  }
}

/*
 * Arguments    : const int32_t x[2001]
 *                int32_t y[2001]
 * Return Type  : void
 */
void drc_sin_fixpt(const int32_t x[2001], int32_t y[2001])
{
  int128m_T r4;
  int64m_T r;
  int96m_T c[2001];
  int96m_T r1;
  int96m_T r10;
  int96m_T r2;
  int96m_T r3;
  int96m_T r5;
  int96m_T r6;
  int96m_T r7;
  int96m_T r8;
  int96m_T r9;
  int32_t m;
  int32_t y_tmp;
  uint32_t u;
  uint32_t u1;
  uint16_t idxUFIX16;
  int8_t i;
  uint8_t slice_temp;
  if ((isInitialized_drc_sin_fixpt ? 1U : 0U) == false) {
    drc_sin_fixpt_initialize();
  }

  u = 3373259426U;
  for (m = 0; m < 2001; m++) {
    u1 = (uint32_t)x[m];
    ssuMultiWordMul((uint32_t *)(&u1), 1, (uint32_t *)(&u), 1, (uint32_t *)
                    (&r.chunks[0U]), 2);
    sMultiWord2MultiWord((uint32_t *)(&r.chunks[0U]), 2, (uint32_t *)
                         (&r1.chunks[0U]), 3);
    sMultiWordShl((uint32_t *)(&r1.chunks[0U]), 3, 1U, (uint32_t *)(&r2.chunks
      [0U]), 3);
    MultiWordSignedWrap((uint32_t *)(&r2.chunks[0U]), 3, 31U, (uint32_t *)
                        (&r3.chunks[0U]));
    sMultiWordShr((uint32_t *)(&r3.chunks[0U]), 3, 1U, (uint32_t *)(&c[m]
      .chunks[0U]), 3);
  }

  u = 3373259426U;
  for (m = 0; m < 2001; m++) {
    if (2001 > (m + 1)) {
      y_tmp = m;
    } else {
      y_tmp = 2000;
    }

    sMultiWordShl((uint32_t *)(&c[y_tmp].chunks[0U]), 3, 29U, (uint32_t *)
                  (&r2.chunks[0U]), 3);
    uLong2MultiWord(3373259426U, (uint32_t *)(&r1.chunks[0U]), 3);
    sMultiWordDivFloor((uint32_t *)(&r2.chunks[0U]), 3, (uint32_t *)(&r1.chunks
      [0U]), 3, (uint32_t *)(&r4.chunks[0U]), 4, (uint32_t *)(&r5.chunks[0U]), 3,
                       (uint32_t *)(&r6.chunks[0U]), 3, (uint32_t *)(&r7.chunks
      [0U]), 3);
    sMultiWord2MultiWord((uint32_t *)(&r4.chunks[0U]), 4, (uint32_t *)
                         (&r3.chunks[0U]), 3);
    MultiWordSignedWrap((uint32_t *)(&r3.chunks[0U]), 3, 31U, (uint32_t *)
                        (&r8.chunks[0U]));
    sMultiWordShr((uint32_t *)(&r8.chunks[0U]), 3, 61U, (uint32_t *)(&r2.chunks
      [0U]), 3);
    i = (int8_t)MultiWord2sLong((uint32_t *)(&r2.chunks[0U]));
    if ((i & 8) != 0) {
      u1 = (uint32_t)((int8_t)(i | -8));
    } else {
      u1 = (uint32_t)((int8_t)(i & 7));
    }

    ssuMultiWordMul((uint32_t *)(&u1), 1, (uint32_t *)(&u), 1, (uint32_t *)
                    (&r.chunks[0U]), 2);
    sMultiWord2MultiWord((uint32_t *)(&r.chunks[0U]), 2, (uint32_t *)
                         (&r9.chunks[0U]), 3);
    sMultiWordShl((uint32_t *)(&r9.chunks[0U]), 3, 32U, (uint32_t *)
                  (&r10.chunks[0U]), 3);
    MultiWordSignedWrap((uint32_t *)(&r10.chunks[0U]), 3, 31U, (uint32_t *)
                        (&r7.chunks[0U]));
    MultiWordSub((uint32_t *)(&c[m].chunks[0U]), (uint32_t *)(&r7.chunks[0U]),
                 (uint32_t *)(&r6.chunks[0U]), 3);
    MultiWordSignedWrap((uint32_t *)(&r6.chunks[0U]), 3, 31U, (uint32_t *)
                        (&r5.chunks[0U]));
    sMultiWordShr((uint32_t *)(&r5.chunks[0U]), 3, 48U, (uint32_t *)(&r1.chunks
      [0U]), 3);
    idxUFIX16 = (uint16_t)(mul_u32_loSR(683563337U, (uint32_t)((uint16_t)
      MultiWord2uLong((uint32_t *)(&r1.chunks[0U]))), 13U) >> 16U);
    slice_temp = (uint8_t)(((uint32_t)idxUFIX16) >> 8U);
    y_tmp = ((int32_t)FI_SIN_COS_LUT[slice_temp]) * 32768;
    y[m] = ((int32_t)((int16_t)asr_s32(y_tmp + (((int32_t)((int16_t)asr_s32
      (mul_ssu32_loSR((((int32_t)FI_SIN_COS_LUT[(uint8_t)(((uint32_t)slice_temp)
      + 1U)]) * 32768) - y_tmp, (uint32_t)((int32_t)(((int32_t)idxUFIX16) &
      ((uint16_t)255))), 8U), 15U))) * 32768), 15U))) * 32768;
  }
}

/*
 * Arguments    : void
 * Return Type  : void
 */
void drc_sin_fixpt_initialize(void)
{
  sin_init();
  isInitialized_drc_sin_fixpt = true;
}

/*
 * Arguments    : void
 * Return Type  : void
 */
void drc_sin_fixpt_terminate(void)
{
  /* (no terminate code required) */
  isInitialized_drc_sin_fixpt = false;
}

/*
 * Arguments    : int32_t x[2001]
 * Return Type  : void
 */
void init_struc_fixpt(int32_t x[2001])
{

  static const int32_t iv[2001] = { -1073741824, -1072668082, -1071594340,
    -1070520599, -1069446857, -1068373115, -1067299373, -1066225631, -1065151889,
    -1064078148, -1063004406, -1061930664, -1060856922, -1059783180, -1058709438,
    -1057635697, -1056561955, -1055488213, -1054414471, -1053340729, -1052266988,
    -1051193246, -1050119504, -1049045762, -1047972020, -1046898278, -1045824537,
    -1044750795, -1043677053, -1042603311, -1041529569, -1040455827, -1039382086,
    -1038308344, -1037234602, -1036160860, -1035087118, -1034013377, -1032939635,
    -1031865893, -1030792151, -1029718409, -1028644667, -1027570926, -1026497184,
    -1025423442, -1024349700, -1023275958, -1022202216, -1021128475, -1020054733,
    -1018980991, -1017907249, -1016833507, -1015759766, -1014686024, -1013612282,
    -1012538540, -1011464798, -1010391056, -1009317315, -1008243573, -1007169831,
    -1006096089, -1005022347, -1003948605, -1002874864, -1001801122, -1000727380,
    -999653638, -998579896, -997506154, -996432413, -995358671, -994284929,
    -993211187, -992137445, -991063704, -989989962, -988916220, -987842478,
    -986768736, -985694994, -984621253, -983547511, -982473769, -981400027,
    -980326285, -979252543, -978178802, -977105060, -976031318, -974957576,
    -973883834, -972810093, -971736351, -970662609, -969588867, -968515125,
    -967441383, -966367642, -965293900, -964220158, -963146416, -962072674,
    -960998932, -959925191, -958851449, -957777707, -956703965, -955630223,
    -954556482, -953482740, -952408998, -951335256, -950261514, -949187772,
    -948114031, -947040289, -945966547, -944892805, -943819063, -942745321,
    -941671580, -940597838, -939524096, -938450354, -937376612, -936302871,
    -935229129, -934155387, -933081645, -932007903, -930934161, -929860420,
    -928786678, -927712936, -926639194, -925565452, -924491710, -923417969,
    -922344227, -921270485, -920196743, -919123001, -918049260, -916975518,
    -915901776, -914828034, -913754292, -912680550, -911606809, -910533067,
    -909459325, -908385583, -907311841, -906238099, -905164358, -904090616,
    -903016874, -901943132, -900869390, -899795649, -898721907, -897648165,
    -896574423, -895500681, -894426939, -893353198, -892279456, -891205714,
    -890131972, -889058230, -887984488, -886910747, -885837005, -884763263,
    -883689521, -882615779, -881542038, -880468296, -879394554, -878320812,
    -877247070, -876173328, -875099587, -874025845, -872952103, -871878361,
    -870804619, -869730877, -868657136, -867583394, -866509652, -865435910,
    -864362168, -863288426, -862214685, -861140943, -860067201, -858993459,
    -857919717, -856845976, -855772234, -854698492, -853624750, -852551008,
    -851477266, -850403525, -849329783, -848256041, -847182299, -846108557,
    -845034815, -843961074, -842887332, -841813590, -840739848, -839666106,
    -838592365, -837518623, -836444881, -835371139, -834297397, -833223655,
    -832149914, -831076172, -830002430, -828928688, -827854946, -826781204,
    -825707463, -824633721, -823559979, -822486237, -821412495, -820338754,
    -819265012, -818191270, -817117528, -816043786, -814970044, -813896303,
    -812822561, -811748819, -810675077, -809601335, -808527593, -807453852,
    -806380110, -805306368, -804232626, -803158884, -802085143, -801011401,
    -799937659, -798863917, -797790175, -796716433, -795642692, -794568950,
    -793495208, -792421466, -791347724, -790273982, -789200241, -788126499,
    -787052757, -785979015, -784905273, -783831532, -782757790, -781684048,
    -780610306, -779536564, -778462822, -777389081, -776315339, -775241597,
    -774167855, -773094113, -772020371, -770946630, -769872888, -768799146,
    -767725404, -766651662, -765577921, -764504179, -763430437, -762356695,
    -761282953, -760209211, -759135470, -758061728, -756987986, -755914244,
    -754840502, -753766760, -752693019, -751619277, -750545535, -749471793,
    -748398051, -747324310, -746250568, -745176826, -744103084, -743029342,
    -741955600, -740881859, -739808117, -738734375, -737660633, -736586891,
    -735513149, -734439408, -733365666, -732291924, -731218182, -730144440,
    -729070698, -727996957, -726923215, -725849473, -724775731, -723701989,
    -722628248, -721554506, -720480764, -719407022, -718333280, -717259538,
    -716185797, -715112055, -714038313, -712964571, -711890829, -710817087,
    -709743346, -708669604, -707595862, -706522120, -705448378, -704374637,
    -703300895, -702227153, -701153411, -700079669, -699005927, -697932186,
    -696858444, -695784702, -694710960, -693637218, -692563476, -691489735,
    -690415993, -689342251, -688268509, -687194767, -686121026, -685047284,
    -683973542, -682899800, -681826058, -680752316, -679678575, -678604833,
    -677531091, -676457349, -675383607, -674309865, -673236124, -672162382,
    -671088640, -670014898, -668941156, -667867415, -666793673, -665719931,
    -664646189, -663572447, -662498705, -661424964, -660351222, -659277480,
    -658203738, -657129996, -656056254, -654982513, -653908771, -652835029,
    -651761287, -650687545, -649613804, -648540062, -647466320, -646392578,
    -645318836, -644245094, -643171353, -642097611, -641023869, -639950127,
    -638876385, -637802643, -636728902, -635655160, -634581418, -633507676,
    -632433934, -631360193, -630286451, -629212709, -628138967, -627065225,
    -625991483, -624917742, -623844000, -622770258, -621696516, -620622774,
    -619549032, -618475291, -617401549, -616327807, -615254065, -614180323,
    -613106582, -612032840, -610959098, -609885356, -608811614, -607737872,
    -606664131, -605590389, -604516647, -603442905, -602369163, -601295421,
    -600221680, -599147938, -598074196, -597000454, -595926712, -594852970,
    -593779229, -592705487, -591631745, -590558003, -589484261, -588410520,
    -587336778, -586263036, -585189294, -584115552, -583041810, -581968069,
    -580894327, -579820585, -578746843, -577673101, -576599359, -575525618,
    -574451876, -573378134, -572304392, -571230650, -570156909, -569083167,
    -568009425, -566935683, -565861941, -564788199, -563714458, -562640716,
    -561566974, -560493232, -559419490, -558345748, -557272007, -556198265,
    -555124523, -554050781, -552977039, -551903298, -550829556, -549755814,
    -548682072, -547608330, -546534588, -545460847, -544387105, -543313363,
    -542239621, -541165879, -540092137, -539018396, -537944654, -536870912,
    -535797170, -534723428, -533649687, -532575945, -531502203, -530428461,
    -529354719, -528280977, -527207236, -526133494, -525059752, -523986010,
    -522912268, -521838526, -520764785, -519691043, -518617301, -517543559,
    -516469817, -515396076, -514322334, -513248592, -512174850, -511101108,
    -510027366, -508953625, -507879883, -506806141, -505732399, -504658657,
    -503584915, -502511174, -501437432, -500363690, -499289948, -498216206,
    -497142465, -496068723, -494994981, -493921239, -492847497, -491773755,
    -490700014, -489626272, -488552530, -487478788, -486405046, -485331304,
    -484257563, -483183821, -482110079, -481036337, -479962595, -478888854,
    -477815112, -476741370, -475667628, -474593886, -473520144, -472446403,
    -471372661, -470298919, -469225177, -468151435, -467077693, -466003952,
    -464930210, -463856468, -462782726, -461708984, -460635242, -459561501,
    -458487759, -457414017, -456340275, -455266533, -454192792, -453119050,
    -452045308, -450971566, -449897824, -448824082, -447750341, -446676599,
    -445602857, -444529115, -443455373, -442381631, -441307890, -440234148,
    -439160406, -438086664, -437012922, -435939181, -434865439, -433791697,
    -432717955, -431644213, -430570471, -429496730, -428422988, -427349246,
    -426275504, -425201762, -424128020, -423054279, -421980537, -420906795,
    -419833053, -418759311, -417685570, -416611828, -415538086, -414464344,
    -413390602, -412316860, -411243119, -410169377, -409095635, -408021893,
    -406948151, -405874409, -404800668, -403726926, -402653184, -401579442,
    -400505700, -399431959, -398358217, -397284475, -396210733, -395136991,
    -394063249, -392989508, -391915766, -390842024, -389768282, -388694540,
    -387620798, -386547057, -385473315, -384399573, -383325831, -382252089,
    -381178348, -380104606, -379030864, -377957122, -376883380, -375809638,
    -374735897, -373662155, -372588413, -371514671, -370440929, -369367187,
    -368293446, -367219704, -366145962, -365072220, -363998478, -362924737,
    -361850995, -360777253, -359703511, -358629769, -357556027, -356482286,
    -355408544, -354334802, -353261060, -352187318, -351113576, -350039835,
    -348966093, -347892351, -346818609, -345744867, -344671126, -343597384,
    -342523642, -341449900, -340376158, -339302416, -338228675, -337154933,
    -336081191, -335007449, -333933707, -332859965, -331786224, -330712482,
    -329638740, -328564998, -327491256, -326417514, -325343773, -324270031,
    -323196289, -322122547, -321048805, -319975064, -318901322, -317827580,
    -316753838, -315680096, -314606354, -313532613, -312458871, -311385129,
    -310311387, -309237645, -308163903, -307090162, -306016420, -304942678,
    -303868936, -302795194, -301721453, -300647711, -299573969, -298500227,
    -297426485, -296352743, -295279002, -294205260, -293131518, -292057776,
    -290984034, -289910292, -288836551, -287762809, -286689067, -285615325,
    -284541583, -283467842, -282394100, -281320358, -280246616, -279172874,
    -278099132, -277025391, -275951649, -274877907, -273804165, -272730423,
    -271656681, -270582940, -269509198, -268435456, -267361714, -266287972,
    -265214231, -264140489, -263066747, -261993005, -260919263, -259845521,
    -258771780, -257698038, -256624296, -255550554, -254476812, -253403070,
    -252329329, -251255587, -250181845, -249108103, -248034361, -246960620,
    -245886878, -244813136, -243739394, -242665652, -241591910, -240518169,
    -239444427, -238370685, -237296943, -236223201, -235149459, -234075718,
    -233001976, -231928234, -230854492, -229780750, -228707009, -227633267,
    -226559525, -225485783, -224412041, -223338299, -222264558, -221190816,
    -220117074, -219043332, -217969590, -216895848, -215822107, -214748365,
    -213674623, -212600881, -211527139, -210453398, -209379656, -208305914,
    -207232172, -206158430, -205084688, -204010947, -202937205, -201863463,
    -200789721, -199715979, -198642237, -197568496, -196494754, -195421012,
    -194347270, -193273528, -192199786, -191126045, -190052303, -188978561,
    -187904819, -186831077, -185757336, -184683594, -183609852, -182536110,
    -181462368, -180388626, -179314885, -178241143, -177167401, -176093659,
    -175019917, -173946175, -172872434, -171798692, -170724950, -169651208,
    -168577466, -167503725, -166429983, -165356241, -164282499, -163208757,
    -162135015, -161061274, -159987532, -158913790, -157840048, -156766306,
    -155692564, -154618823, -153545081, -152471339, -151397597, -150323855,
    -149250114, -148176372, -147102630, -146028888, -144955146, -143881404,
    -142807663, -141733921, -140660179, -139586437, -138512695, -137438953,
    -136365212, -135291470, -134217728, -133143986, -132070244, -130996503,
    -129922761, -128849019, -127775277, -126701535, -125627793, -124554052,
    -123480310, -122406568, -121332826, -120259084, -119185342, -118111601,
    -117037859, -115964117, -114890375, -113816633, -112742892, -111669150,
    -110595408, -109521666, -108447924, -107374182, -106300441, -105226699,
    -104152957, -103079215, -102005473, -100931731, -99857990, -98784248,
    -97710506, -96636764, -95563022, -94489281, -93415539, -92341797, -91268055,
    -90194313, -89120571, -88046830, -86973088, -85899346, -84825604, -83751862,
    -82678120, -81604379, -80530637, -79456895, -78383153, -77309411, -76235670,
    -75161928, -74088186, -73014444, -71940702, -70866960, -69793219, -68719477,
    -67645735, -66571993, -65498251, -64424509, -63350768, -62277026, -61203284,
    -60129542, -59055800, -57982058, -56908317, -55834575, -54760833, -53687091,
    -52613349, -51539608, -50465866, -49392124, -48318382, -47244640, -46170898,
    -45097157, -44023415, -42949673, -41875931, -40802189, -39728447, -38654706,
    -37580964, -36507222, -35433480, -34359738, -33285997, -32212255, -31138513,
    -30064771, -28991029, -27917287, -26843546, -25769804, -24696062, -23622320,
    -22548578, -21474836, -20401095, -19327353, -18253611, -17179869, -16106127,
    -15032386, -13958644, -12884902, -11811160, -10737418, -9663676, -8589935,
    -7516193, -6442451, -5368709, -4294967, -3221225, -2147484, -1073742, 0,
    1073742, 2147484, 3221225, 4294967, 5368709, 6442451, 7516193, 8589935,
    9663676, 10737418, 11811160, 12884902, 13958644, 15032386, 16106127,
    17179869, 18253611, 19327353, 20401095, 21474836, 22548578, 23622320,
    24696062, 25769804, 26843546, 27917287, 28991029, 30064771, 31138513,
    32212255, 33285997, 34359738, 35433480, 36507222, 37580964, 38654706,
    39728447, 40802189, 41875931, 42949673, 44023415, 45097157, 46170898,
    47244640, 48318382, 49392124, 50465866, 51539608, 52613349, 53687091,
    54760833, 55834575, 56908317, 57982058, 59055800, 60129542, 61203284,
    62277026, 63350768, 64424509, 65498251, 66571993, 67645735, 68719477,
    69793219, 70866960, 71940702, 73014444, 74088186, 75161928, 76235670,
    77309411, 78383153, 79456895, 80530637, 81604379, 82678120, 83751862,
    84825604, 85899346, 86973088, 88046830, 89120571, 90194313, 91268055,
    92341797, 93415539, 94489281, 95563022, 96636764, 97710506, 98784248,
    99857990, 100931731, 102005473, 103079215, 104152957, 105226699, 106300441,
    107374182, 108447924, 109521666, 110595408, 111669150, 112742892, 113816633,
    114890375, 115964117, 117037859, 118111601, 119185342, 120259084, 121332826,
    122406568, 123480310, 124554052, 125627793, 126701535, 127775277, 128849019,
    129922761, 130996503, 132070244, 133143986, 134217728, 135291470, 136365212,
    137438953, 138512695, 139586437, 140660179, 141733921, 142807663, 143881404,
    144955146, 146028888, 147102630, 148176372, 149250114, 150323855, 151397597,
    152471339, 153545081, 154618823, 155692564, 156766306, 157840048, 158913790,
    159987532, 161061274, 162135015, 163208757, 164282499, 165356241, 166429983,
    167503725, 168577466, 169651208, 170724950, 171798692, 172872434, 173946175,
    175019917, 176093659, 177167401, 178241143, 179314885, 180388626, 181462368,
    182536110, 183609852, 184683594, 185757336, 186831077, 187904819, 188978561,
    190052303, 191126045, 192199786, 193273528, 194347270, 195421012, 196494754,
    197568496, 198642237, 199715979, 200789721, 201863463, 202937205, 204010947,
    205084688, 206158430, 207232172, 208305914, 209379656, 210453398, 211527139,
    212600881, 213674623, 214748365, 215822107, 216895848, 217969590, 219043332,
    220117074, 221190816, 222264558, 223338299, 224412041, 225485783, 226559525,
    227633267, 228707009, 229780750, 230854492, 231928234, 233001976, 234075718,
    235149459, 236223201, 237296943, 238370685, 239444427, 240518169, 241591910,
    242665652, 243739394, 244813136, 245886878, 246960620, 248034361, 249108103,
    250181845, 251255587, 252329329, 253403070, 254476812, 255550554, 256624296,
    257698038, 258771780, 259845521, 260919263, 261993005, 263066747, 264140489,
    265214231, 266287972, 267361714, 268435456, 269509198, 270582940, 271656681,
    272730423, 273804165, 274877907, 275951649, 277025391, 278099132, 279172874,
    280246616, 281320358, 282394100, 283467842, 284541583, 285615325, 286689067,
    287762809, 288836551, 289910292, 290984034, 292057776, 293131518, 294205260,
    295279002, 296352743, 297426485, 298500227, 299573969, 300647711, 301721453,
    302795194, 303868936, 304942678, 306016420, 307090162, 308163903, 309237645,
    310311387, 311385129, 312458871, 313532613, 314606354, 315680096, 316753838,
    317827580, 318901322, 319975064, 321048805, 322122547, 323196289, 324270031,
    325343773, 326417514, 327491256, 328564998, 329638740, 330712482, 331786224,
    332859965, 333933707, 335007449, 336081191, 337154933, 338228675, 339302416,
    340376158, 341449900, 342523642, 343597384, 344671126, 345744867, 346818609,
    347892351, 348966093, 350039835, 351113576, 352187318, 353261060, 354334802,
    355408544, 356482286, 357556027, 358629769, 359703511, 360777253, 361850995,
    362924737, 363998478, 365072220, 366145962, 367219704, 368293446, 369367187,
    370440929, 371514671, 372588413, 373662155, 374735897, 375809638, 376883380,
    377957122, 379030864, 380104606, 381178348, 382252089, 383325831, 384399573,
    385473315, 386547057, 387620798, 388694540, 389768282, 390842024, 391915766,
    392989508, 394063249, 395136991, 396210733, 397284475, 398358217, 399431959,
    400505700, 401579442, 402653184, 403726926, 404800668, 405874409, 406948151,
    408021893, 409095635, 410169377, 411243119, 412316860, 413390602, 414464344,
    415538086, 416611828, 417685570, 418759311, 419833053, 420906795, 421980537,
    423054279, 424128020, 425201762, 426275504, 427349246, 428422988, 429496730,
    430570471, 431644213, 432717955, 433791697, 434865439, 435939181, 437012922,
    438086664, 439160406, 440234148, 441307890, 442381631, 443455373, 444529115,
    445602857, 446676599, 447750341, 448824082, 449897824, 450971566, 452045308,
    453119050, 454192792, 455266533, 456340275, 457414017, 458487759, 459561501,
    460635242, 461708984, 462782726, 463856468, 464930210, 466003952, 467077693,
    468151435, 469225177, 470298919, 471372661, 472446403, 473520144, 474593886,
    475667628, 476741370, 477815112, 478888854, 479962595, 481036337, 482110079,
    483183821, 484257563, 485331304, 486405046, 487478788, 488552530, 489626272,
    490700014, 491773755, 492847497, 493921239, 494994981, 496068723, 497142465,
    498216206, 499289948, 500363690, 501437432, 502511174, 503584915, 504658657,
    505732399, 506806141, 507879883, 508953625, 510027366, 511101108, 512174850,
    513248592, 514322334, 515396076, 516469817, 517543559, 518617301, 519691043,
    520764785, 521838526, 522912268, 523986010, 525059752, 526133494, 527207236,
    528280977, 529354719, 530428461, 531502203, 532575945, 533649687, 534723428,
    535797170, 536870912, 537944654, 539018396, 540092137, 541165879, 542239621,
    543313363, 544387105, 545460847, 546534588, 547608330, 548682072, 549755814,
    550829556, 551903298, 552977039, 554050781, 555124523, 556198265, 557272007,
    558345748, 559419490, 560493232, 561566974, 562640716, 563714458, 564788199,
    565861941, 566935683, 568009425, 569083167, 570156909, 571230650, 572304392,
    573378134, 574451876, 575525618, 576599359, 577673101, 578746843, 579820585,
    580894327, 581968069, 583041810, 584115552, 585189294, 586263036, 587336778,
    588410520, 589484261, 590558003, 591631745, 592705487, 593779229, 594852970,
    595926712, 597000454, 598074196, 599147938, 600221680, 601295421, 602369163,
    603442905, 604516647, 605590389, 606664131, 607737872, 608811614, 609885356,
    610959098, 612032840, 613106582, 614180323, 615254065, 616327807, 617401549,
    618475291, 619549032, 620622774, 621696516, 622770258, 623844000, 624917742,
    625991483, 627065225, 628138967, 629212709, 630286451, 631360193, 632433934,
    633507676, 634581418, 635655160, 636728902, 637802643, 638876385, 639950127,
    641023869, 642097611, 643171353, 644245094, 645318836, 646392578, 647466320,
    648540062, 649613804, 650687545, 651761287, 652835029, 653908771, 654982513,
    656056254, 657129996, 658203738, 659277480, 660351222, 661424964, 662498705,
    663572447, 664646189, 665719931, 666793673, 667867415, 668941156, 670014898,
    671088640, 672162382, 673236124, 674309865, 675383607, 676457349, 677531091,
    678604833, 679678575, 680752316, 681826058, 682899800, 683973542, 685047284,
    686121026, 687194767, 688268509, 689342251, 690415993, 691489735, 692563476,
    693637218, 694710960, 695784702, 696858444, 697932186, 699005927, 700079669,
    701153411, 702227153, 703300895, 704374637, 705448378, 706522120, 707595862,
    708669604, 709743346, 710817087, 711890829, 712964571, 714038313, 715112055,
    716185797, 717259538, 718333280, 719407022, 720480764, 721554506, 722628248,
    723701989, 724775731, 725849473, 726923215, 727996957, 729070698, 730144440,
    731218182, 732291924, 733365666, 734439408, 735513149, 736586891, 737660633,
    738734375, 739808117, 740881859, 741955600, 743029342, 744103084, 745176826,
    746250568, 747324310, 748398051, 749471793, 750545535, 751619277, 752693019,
    753766760, 754840502, 755914244, 756987986, 758061728, 759135470, 760209211,
    761282953, 762356695, 763430437, 764504179, 765577921, 766651662, 767725404,
    768799146, 769872888, 770946630, 772020371, 773094113, 774167855, 775241597,
    776315339, 777389081, 778462822, 779536564, 780610306, 781684048, 782757790,
    783831532, 784905273, 785979015, 787052757, 788126499, 789200241, 790273982,
    791347724, 792421466, 793495208, 794568950, 795642692, 796716433, 797790175,
    798863917, 799937659, 801011401, 802085143, 803158884, 804232626, 805306368,
    806380110, 807453852, 808527593, 809601335, 810675077, 811748819, 812822561,
    813896303, 814970044, 816043786, 817117528, 818191270, 819265012, 820338754,
    821412495, 822486237, 823559979, 824633721, 825707463, 826781204, 827854946,
    828928688, 830002430, 831076172, 832149914, 833223655, 834297397, 835371139,
    836444881, 837518623, 838592365, 839666106, 840739848, 841813590, 842887332,
    843961074, 845034815, 846108557, 847182299, 848256041, 849329783, 850403525,
    851477266, 852551008, 853624750, 854698492, 855772234, 856845976, 857919717,
    858993459, 860067201, 861140943, 862214685, 863288426, 864362168, 865435910,
    866509652, 867583394, 868657136, 869730877, 870804619, 871878361, 872952103,
    874025845, 875099587, 876173328, 877247070, 878320812, 879394554, 880468296,
    881542038, 882615779, 883689521, 884763263, 885837005, 886910747, 887984488,
    889058230, 890131972, 891205714, 892279456, 893353198, 894426939, 895500681,
    896574423, 897648165, 898721907, 899795649, 900869390, 901943132, 903016874,
    904090616, 905164358, 906238099, 907311841, 908385583, 909459325, 910533067,
    911606809, 912680550, 913754292, 914828034, 915901776, 916975518, 918049260,
    919123001, 920196743, 921270485, 922344227, 923417969, 924491710, 925565452,
    926639194, 927712936, 928786678, 929860420, 930934161, 932007903, 933081645,
    934155387, 935229129, 936302871, 937376612, 938450354, 939524096, 940597838,
    941671580, 942745321, 943819063, 944892805, 945966547, 947040289, 948114031,
    949187772, 950261514, 951335256, 952408998, 953482740, 954556482, 955630223,
    956703965, 957777707, 958851449, 959925191, 960998932, 962072674, 963146416,
    964220158, 965293900, 966367642, 967441383, 968515125, 969588867, 970662609,
    971736351, 972810093, 973883834, 974957576, 976031318, 977105060, 978178802,
    979252543, 980326285, 981400027, 982473769, 983547511, 984621253, 985694994,
    986768736, 987842478, 988916220, 989989962, 991063704, 992137445, 993211187,
    994284929, 995358671, 996432413, 997506154, 998579896, 999653638, 1000727380,
    1001801122, 1002874864, 1003948605, 1005022347, 1006096089, 1007169831,
    1008243573, 1009317315, 1010391056, 1011464798, 1012538540, 1013612282,
    1014686024, 1015759766, 1016833507, 1017907249, 1018980991, 1020054733,
    1021128475, 1022202216, 1023275958, 1024349700, 1025423442, 1026497184,
    1027570926, 1028644667, 1029718409, 1030792151, 1031865893, 1032939635,
    1034013377, 1035087118, 1036160860, 1037234602, 1038308344, 1039382086,
    1040455827, 1041529569, 1042603311, 1043677053, 1044750795, 1045824537,
    1046898278, 1047972020, 1049045762, 1050119504, 1051193246, 1052266988,
    1053340729, 1054414471, 1055488213, 1056561955, 1057635697, 1058709438,
    1059783180, 1060856922, 1061930664, 1063004406, 1064078148, 1065151889,
    1066225631, 1067299373, 1068373115, 1069446857, 1070520599, 1071594340,
    1072668082, 1073741824 };

  if ((isInitialized_drc_sin_fixpt ? 1U : 0U) == false) {
    drc_sin_fixpt_initialize();
  }

  memcpy(&x[0], &iv[0], 2001U * (sizeof(int32_t)));

  /* Q2.30 */
}

/*
 * File trailer for drc_sin_fixpt.c
 *
 * [EOF]
 */

/***********************************************************************
 * 
 *  LUSH Lisp Universal Shell
 *    Copyright (C) 2009 Leon Bottou, Yann Le Cun, Ralf Juengling.
 *    Copyright (C) 2002 Leon Bottou, Yann Le Cun, AT&T Corp, NECI.
 *  Includes parts of TL3:
 *    Copyright (C) 1987-1999 Leon Bottou and Neuristique.
 *  Includes selected parts of SN3.2:
 *    Copyright (C) 1991-2001 AT&T Corp.
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the Lesser GNU General Public License as 
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA
 * 
 ***********************************************************************/

/* exterior and dot products with square of one of the arguments */
/* this is only needed for computing second derivatives in lenet */

#include "idxmac.h"
#include "idxops.h"

/* exterior product with square of second argument */
#define Midx_m1squextm1(i1, i2, o1, Type1, Type2, Type3) \
{ \
  Type2 *c2; \
  Type3 *d1; \
  Type1 *c1; \
  Type2 *c2_0; \
  Type3 *d1_0; \
  ptrdiff_t c2_m0 = (i2)->mod[0], d1_m1 = (o1)->mod[1]; \
  ptrdiff_t c1_m0 = (i1)->mod[0], d1_m0 = (o1)->mod[0]; \
  size_t jmax = (o1)->dim[1]; \
  size_t imax = (o1)->dim[0]; \
  c1 = IDX_PTR((i1), Type1); \
  c2_0 = IDX_PTR((i2), Type2); \
  d1_0 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++) { \
    d1 = d1_0; \
    c2 = c2_0; \
    for (size_t j=0; j<jmax; j++) { \
      *d1 = (*c1)*(*c2)*(*c2); \
      d1 += d1_m1; \
      c2 += c2_m0; \
    } \
    d1_0 += d1_m0; \
    c1 += c1_m0; \
  } \
} 

#define Midx_m2squextm2(i1, i2, o1, Type1, Type2, Type3) \
{ \
  Type2 *c2_0, *c2_1; \
  Type3 *d1_2, *d1_3; \
  Type3  *d1_0, *d1_1; \
  Type1  *c1_0, *c1_1; \
  Type2  *ker; \
  ptrdiff_t c2_m0 = (i2)->mod[0], c2_m1 = (i2)->mod[1]; \
  ptrdiff_t d1_m2 = (o1)->mod[2], d1_m3 = (o1)->mod[3]; \
  ptrdiff_t c1_m0 = (i1)->mod[0], c1_m1 = (i1)->mod[1]; \
  ptrdiff_t d1_m0 = (o1)->mod[0], d1_m1 = (o1)->mod[1]; \
  size_t lmax = (o1)->dim[3], kmax = (o1)->dim[2]; \
  size_t imax = (o1)->dim[0], jmax = (o1)->dim[1]; \
  c1_0 = IDX_PTR((i1), Type1); \
  ker = IDX_PTR((i2), Type2); \
  d1_0 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++) { \
    d1_1 = d1_0; \
    c1_1 = c1_0; \
    for (size_t j=0; j<jmax; j++) { \
      d1_2 = d1_1; \
      c2_0 = ker; \
      for (size_t k=0; k<kmax; k++) { \
        d1_3 = d1_2; \
        c2_1 = c2_0; \
        for (size_t l=0; l<lmax; l++) { \
          *d1_3 = (*c1_1)*(*c2_1)*(*c2_1); \
          d1_3 += d1_m3; \
          c2_1 += c2_m1; \
        } \
        d1_2 += d1_m2; \
        c2_0 += c2_m0; \
      } \
      d1_1 += d1_m1; \
      c1_1 += c1_m1; \
    } \
    d1_0 += d1_m0; \
    c1_0 += c1_m0; \
  } \
}

/*           WITH accumulation */

#define Midx_m1squextm1acc(i1, i2, o1, Type1, Type2, Type3) \
{ \
  Type2 *c2; \
  Type3 *d1; \
  Type1 *c1; \
  Type2 *c2_0; \
  Type3 *d1_0; \
  ptrdiff_t c2_m0 = (i2)->mod[0], d1_m1 = (o1)->mod[1]; \
  ptrdiff_t c1_m0 = (i1)->mod[0], d1_m0 = (o1)->mod[0]; \
  size_t jmax = (o1)->dim[1]; \
  size_t imax = (o1)->dim[0]; \
  c1 = IDX_PTR((i1), Type1); \
  c2_0 = IDX_PTR((i2), Type2); \
  d1_0 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++) { \
    d1 = d1_0; \
    c2 = c2_0; \
    for (size_t j=0; j<jmax; j++) { \
      *d1 += (*c1)*(*c2)*(*c2); \
      d1 += d1_m1; \
      c2 += c2_m0; \
    } \
    d1_0 += d1_m0; \
    c1 += c1_m0; \
  } \
} 


#define Midx_m2squextm2acc(i1, i2, o1, Type1, Type2, Type3) \
{ \
  Type2 *c2_0, *c2_1; \
  Type3 *d1_2, *d1_3; \
  Type1  *c1_0, *c1_1; \
  Type2  *ker; \
  Type3  *d1_0, *d1_1; \
  ptrdiff_t c2_m0 = (i2)->mod[0], c2_m1 = (i2)->mod[1]; \
  ptrdiff_t d1_m2 = (o1)->mod[2], d1_m3 = (o1)->mod[3]; \
  ptrdiff_t c1_m0 = (i1)->mod[0], c1_m1 = (i1)->mod[1]; \
  ptrdiff_t d1_m0 = (o1)->mod[0], d1_m1 = (o1)->mod[1]; \
  size_t kmax = (o1)->dim[2], lmax = (o1)->dim[3]; \
  size_t imax = (o1)->dim[0], jmax = (o1)->dim[1]; \
  c1_0 = IDX_PTR((i1), Type1); \
  ker = IDX_PTR((i2), Type2); \
  d1_0 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++) { \
    d1_1 = d1_0; \
    c1_1 = c1_0; \
    for (size_t j=0; j<jmax; j++) { \
      d1_2 = d1_1; \
      c2_0 = ker; \
      for (size_t k=0; k<kmax; k++) { \
        d1_3 = d1_2; \
        c2_1 = c2_0; \
        for (size_t l=0; l<lmax; l++) { \
          *d1_3 += (*c1_1)*(*c2_1)*(*c2_1); \
          d1_3 += d1_m3; \
          c2_1 += c2_m1; \
        } \
        d1_2 += d1_m2; \
        c2_0 += c2_m0; \
      } \
      d1_1 += d1_m1; \
      c1_1 += c1_m1; \
    } \
    d1_0 += d1_m0; \
    c1_0 += c1_m0; \
  } \
}

#define Midx_m2squdotm1(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type1 *c1_0; \
  Type2 *ker; \
  ptrdiff_t c1_m1 = (i1)->mod[1], c2_m0 = (i2)->mod[0]; \
  size_t jmax = (i2)->dim[0]; \
  ptrdiff_t c1_m0 = (i1)->mod[0], d1_m0 = (o1)->mod[0]; \
  Type3 *d1, f; \
  size_t imax = (o1)->dim[0]; \
  c1_0 = IDX_PTR((i1), Type1); \
  ker = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++){ \
    f = 0; \
    c1 = c1_0; \
    c2 = ker; \
    if(c1_m1==1 && c2_m0==1) \
      for (size_t j=0; j<jmax; j++) { \
        f += (*c1)*(*c1)*(*c2++); \
	c1++; \
	}\
    else \
      for (size_t j=0; j<jmax; j++) { \
	f += (*c1)*(*c1)*(*c2); \
        c1 += c1_m1; \
        c2 += c2_m0; \
      } \
    *d1 = f; \
     d1 += d1_m0; \
     c1_0 += c1_m0; \
  } \
}


/* multiply M4 by M2, result in M2 */
#define Midx_m4squdotm2(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1, *c1_2; \
  Type2 *c2, *c2_0; \
  Type1 *c1_0, *c1_1; \
  Type2 *ker; \
  ptrdiff_t c1_m2 = (i1)->mod[2], c2_m0 = (i2)->mod[0]; \
  ptrdiff_t c1_m3 = (i1)->mod[3], c2_m1 = (i2)->mod[1]; \
  size_t kmax = (i2)->dim[0], lmax = (i2)->dim[1]; \
  Type3 *d1_0, *d1, f; \
  ptrdiff_t c1_m0 = (i1)->mod[0], d1_m0 = (o1)->mod[0]; \
  ptrdiff_t c1_m1 = (i1)->mod[1], d1_m1 = (o1)->mod[1];   \
  size_t imax = (o1)->dim[0], jmax = (o1)->dim[1]; \
  c1_0 = IDX_PTR((i1), Type1); \
  ker = IDX_PTR((i2), Type2); \
  d1_0 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++){ \
    c1_1 = c1_0; \
    d1 = d1_0; \
    for (size_t j=0; j<jmax; j++) { \
      f = 0; \
      c1_2 = c1_1; \
      c2_0 = ker; \
      for (size_t k=0; k<kmax; k++) { \
        c1 = c1_2; \
        c2 = c2_0; \
        for (size_t l=0; l<lmax; l++) { \
          f += (*c1)*(*c1)*(*c2); \
          c1 += c1_m3; \
          c2 += c2_m1; \
        } \
        c1_2 += c1_m2; \
        c2_0 += c2_m0; \
      } \
      *d1 = f; \
      d1 += d1_m1; \
      c1_1 += c1_m1; \
    } \
    d1_0 += d1_m0; \
    c1_0 += c1_m0; \
  } \
}

/*           with accumulation ============ */

/* multiply M2 by M1, result in M1 */
#define Midx_m2squdotm1acc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type1 *c1_0; \
  Type2 *ker; \
  ptrdiff_t c1_m1 = (i1)->mod[1], c2_m0 = (i2)->mod[0]; \
  size_t jmax = (i2)->dim[0]; \
  ptrdiff_t c1_m0 = (i1)->mod[0], d1_m0 = (o1)->mod[0]; \
  Type3 *d1, f; \
  size_t imax = (o1)->dim[0]; \
  c1_0 = IDX_PTR((i1), Type1); \
  ker = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++){ \
    f = *d1; \
    c1 = c1_0; \
    c2 = ker; \
    for (size_t j=0; j<jmax; j++) { \
      f += (*c1)*(*c1)*(*c2); \
      c1 += c1_m1; \
      c2 += c2_m0; \
    } \
    *d1 = f; \
    d1 += d1_m0; \
    c1_0 += c1_m0; \
  } \
}

/* multiply M4 by M2, result in M2 */
#define Midx_m4squdotm2acc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1, *c1_2; \
  Type2 *c2, *c2_0; \
  Type1 *c1_0, *c1_1; \
  Type2 *ker; \
  ptrdiff_t c1_m2 = (i1)->mod[2], c2_m0 = (i2)->mod[0]; \
  ptrdiff_t c1_m3 = (i1)->mod[3], c2_m1 = (i2)->mod[1]; \
  size_t kmax = (i2)->dim[0], lmax = (i2)->dim[1]; \
  Type3 *d1_0, *d1, f; \
  ptrdiff_t c1_m0 = (i1)->mod[0], d1_m0 = (o1)->mod[0]; \
  ptrdiff_t c1_m1 = (i1)->mod[1], d1_m1 = (o1)->mod[1]; \
  size_t imax = (o1)->dim[0], jmax = (o1)->dim[1]; \
  c1_0 = IDX_PTR((i1), Type1); \
  ker = IDX_PTR((i2), Type2); \
  d1_0 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++){ \
    c1_1 = c1_0; \
    d1 = d1_0; \
    for (size_t j=0; j<jmax; j++) { \
      f = *d1; \
      c1_2 = c1_1; \
      c2_0 = ker; \
      for (size_t k=0; k<kmax; k++) { \
        c1 = c1_2; \
        c2 = c2_0; \
        for (size_t l=0; l<lmax; l++) { \
          f += (*c1)*(*c1)*(*c2); \
          c1 += c1_m3; \
          c2 += c2_m1; \
        } \
        c1_2 += c1_m2; \
        c2_0 += c2_m0; \
      } \
      *d1 = f; \
      d1 += d1_m1; \
      c1_1 += c1_m1; \
    } \
    d1_0 += d1_m0; \
    c1_0 += c1_m0; \
  } \
}

/*                       accumulate result in a 0D vector */
/* compute square dot product M1^2 x M1 accumulated into M0 */ 
#define Midx_m1squdotm1acc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  ptrdiff_t c1_m0 = (i1)->mod[0], c2_m0 = (i2)->mod[0]; \
  Type3 *d1, f; \
  size_t imax = (i1)->dim[0]; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  f = *d1; \
  for (size_t i=0; i<imax; i++){ \
    f += (*c1)*(*c1)*(*c2); \
    c1 += c1_m0; \
    c2 += c2_m0; \
  } \
  *d1 = f; \
}

/* compute dot product M1 x M1 accumulated into M0 */ 
#define Midx_m2squdotm2acc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type1 *c1_0; \
  Type2 *c2_0; \
  ptrdiff_t imax = (i1)->dim[0], jmax = (i2)->dim[1]; \
  ptrdiff_t c1_m0 = (i1)->mod[0], c2_m0 = (i2)->mod[0]; \
  ptrdiff_t c1_m1 = (i1)->mod[1], c2_m1 = (i2)->mod[1]; \
  Type3 *d1, f; \
  c1_0 = IDX_PTR((i1), Type1); \
  c2_0 = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  f = *d1; \
  for (size_t i=0; i<imax; i++){ \
    c1 = c1_0; \
    c2 = c2_0; \
    for (size_t j=0; j<jmax; j++) { \
      f += (*c1)*(*c1)*(*c2); \
      c1 += c1_m1; \
      c2 += c2_m1; \
    } \
    c1_0 += c1_m0; \
    c2_0 += c2_m0; \
  } \
  *d1 = f; \
}



/* -------------------------------------------------------------
   Local Variables:
   c-font-lock-extra-types: (
     "FILE" "\\sw+_t" "at" "gptr" "real" "flt" "intg" "Type[0-9]*")
   End:
   ------------------------------------------------------------- */

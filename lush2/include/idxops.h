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

#ifndef IDXOPS_H
#define IDXOPS_H

/* put these in variables so that they don't get duplicated for each
   macro call! */

/* ============== clear operations, m0, m1, m2, ma =========== */

#define Midx_m0clear(i1, Type)  *IDX_PTR(i1, Type) = 0

#define Midx_m1clear(i1, Type) \
{ Type *c1; \
  size_t jmax = (i1)->dim[0]; \
  ptrdiff_t i1_m0 = (i1)->mod[0]; \
  c1 = IDX_PTR((i1), Type); \
  for (size_t i=0; i<jmax; i++){ \
      *c1 = 0; \
      c1 += i1_m0; \
  } \
}

#define Midx_m2clear(i1, Type) \
{ Type *c1, *c1_0; \
  size_t imax = (i1)->dim[0], jmax = (i1)->dim[1]; \
  ptrdiff_t i1_m0 = (i1)->mod[0], i1_m1 = (i1)->mod[1]; \
  c1_0 = IDX_PTR((i1), Type); \
  for (size_t i=0; i<imax; i++){ \
      c1 = c1_0; \
      for (size_t j=0; j<jmax; j++){  \
	  *c1 = 0; \
	  c1 += i1_m1; \
      } \
      c1_0 += i1_m0; \
  } \
}

#define Midx_maclear(i, Type) \
{ \
  Type *fp; \
  fp = IDX_PTR(i, Type); \
  begin_idx_aloop1((i), k) { \
      fp[k] = 0; \
  } end_idx_aloop1((i), k); \
}

#define Midx_m0clearwith(i1, val, Type)  *IDX_PTR(i1, Type) = val

#define Midx_m1clearwith(i1, val, Type) \
{ Type *c1 = IDX_PTR((i1), Type); \
  Type v = val; \
  size_t jmax = (i1)->dim[0]; \
  ptrdiff_t i1_m0 = (i1)->mod[0]; \
  for (size_t i=0; i<jmax; i++){ \
      *c1 = v; \
      c1 += i1_m0; \
  } \
}

#define Midx_m2clearwith(i1, val, Type) \
{ Type *c1, *c1_0 = IDX_PTR((i1), Type); \
  Type v = val; \
  size_t imax = (i1)->dim[0], jmax = (i1)->dim[1]; \
  ptrdiff_t i1_m0 = (i1)->mod[0], i1_m1 = (i1)->mod[1]; \
  for (size_t i=0; i<imax; i++){ \
      c1 = c1_0; \
      for (size_t j=0; j<jmax; j++){  \
	  *c1 = v; \
	  c1 += i1_m1; \
      } \
      c1_0 += i1_m0; \
  } \
}

#define Midx_maclearwith(i, val, Type) \
{ \
  Type *fp = IDX_PTR(i, Type); \
  Type v = val; \
  begin_idx_aloop1((i), k) { \
      fp[k] = v; \
  } end_idx_aloop1((i), k); \
}

/* ============== copy operations, m0, m1, m2, ma =========== */

#define Midx_m0copy(i1,i2, Type1, Type2) \
   *IDX_PTR(i2, Type2) = *IDX_PTR(i1, Type1)

#define Midx_m1copy(i1,i2, Type1, Type2) \
{ Type1 *c1 = IDX_PTR((i1), Type1); \
  Type2 *c2 = IDX_PTR((i2), Type2); \
  size_t imax = (i1)->dim[0]; \
  ptrdiff_t i1_m0 = (i1)->mod[0], i2_m0 = (i2)->mod[0]; \
  for (size_t i=0; i<imax; i++){ \
      *c2 = *c1; \
      c1 += i1_m0; \
      c2 += i2_m0; \
  } \
}

#define Midx_m2copy(i1,i2, Type1, Type2) \
{ \
  size_t imax = (i1)->dim[0], jmax = (i1)->dim[1]; \
  ptrdiff_t i1_m0 = (i1)->mod[0]; \
  ptrdiff_t i2_m0 = (i2)->mod[0]; \
  ptrdiff_t i1_m1 = (i1)->mod[1]; \
  ptrdiff_t i2_m1 = (i2)->mod[1]; \
  Type1 *c1_0 = IDX_PTR((i1), Type1); \
  Type2 *c2_0 = IDX_PTR((i2), Type2); \
  for (size_t i=0; i<imax; i++){ \
     Type1 *c1 = c1_0; \
     Type2 *c2 = c2_0; \
      for (size_t j=0; j<jmax; j++){  \
	  *c2 = *c1; \
	  c1 += i1_m1; \
	  c2 += i2_m1; \
      } \
      c1_0 += i1_m0; \
      c2_0 += i2_m0; \
  } \
}

#define Midx_macopy(i1, i2, Type1, Type2) \
{ \
  Type1 *fp1 = IDX_PTR(i1, Type1); \
  Type2 *fp2 = IDX_PTR(i2, Type2); \
  begin_idx_aloop2((i1), (i2), k1, k2) { \
      fp2[k2] =  fp1[k1]; \
  } end_idx_aloop2((i1), (i2), k1, k2); \
}

/* ===================== sum all the terms of an idx, m1, m2, ma ===== */
/*                       result in a 0D vector */

#define Midx_m1sum(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f; \
  size_t imax = (i1)->dim[0]; \
  ptrdiff_t i1_m0 = (i1)->mod[0]; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c1; \
  c1 += i1_m0; \
  for (size_t i=1; i<imax; i++){ \
      f += *c1; \
      c1 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_m1sup(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f; \
  size_t imax = (i1)->dim[0]; \
  ptrdiff_t i1_m0 = (i1)->mod[0]; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c1; \
  c1 += i1_m0; \
  for (size_t i=1; i<imax; i++){ \
      if(*c1>f) f = *c1; \
      c1 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_m1inf(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f; \
  size_t imax = (i1)->dim[0]; \
  ptrdiff_t i1_m0 = (i1)->mod[0]; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c1; \
  c1 += i1_m0; \
  for (size_t i=1; i<imax; i++){ \
      if(*c1<f) f = *c1; \
      c1 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_m2sum(i1, i2, Type1, Type2) \
{ Type1 *c1, *c1_0; \
  Type2 *c2, f=0; \
  size_t imax = (i1)->dim[0], jmax = (i1)->dim[1]; \
  ptrdiff_t i1_m0 = (i1)->mod[0], i1_m1 = (i1)->mod[1]; \
  c1_0 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  for (size_t i=0; i<imax; i++){ \
      c1 =c1_0; \
      for (size_t j=0; j<jmax; j++){  \
	  f += *c1; \
	  c1 += i1_m1; \
      } \
      c1_0 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_m2sup(i1, i2, Type1, Type2) \
{ Type1 *c1, *c1_0; \
  Type2 *c2, f; \
  size_t imax = (i1)->dim[0], jmax = (i1)->dim[1]; \
  ptrdiff_t i1_m0 = (i1)->mod[0], i1_m1 = (i1)->mod[1]; \
  c1_0 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c1_0; \
  for (size_t i=0; i<imax; i++){ \
      c1 =c1_0; \
      for (size_t j=0; j<jmax; j++){  \
	  if(*c1>f) f = *c1; \
	  c1 += i1_m1; \
      } \
      c1_0 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_m2inf(i1, i2, Type1, Type2) \
{ Type1 *c1, *c1_0; \
  Type2 *c2, f; \
  size_t imax = (i1)->dim[0], jmax = (i1)->dim[1]; \
  ptrdiff_t i1_m0 = (i1)->mod[0], i1_m1 = (i1)->mod[1]; \
  c1_0 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c1_0; \
  for (size_t i=0; i<imax; i++){ \
      c1 =c1_0; \
      for (size_t j=0; j<jmax; j++){  \
	  if(*c1<f) f = *c1; \
	  c1 += i1_m1; \
      } \
      c1_0 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_masum(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f=0; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  begin_idx_aloop1((i1), k) { \
	  f += c1[k]; \
  } end_idx_aloop1((i1), k); \
  *c2 = f; \
}

#define Midx_masup(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = c1[0]; \
  begin_idx_aloop1((i1), k) { \
      if (c1[k]>f) f = c1[k]; \
  } end_idx_aloop1((i1), k); \
  *c2 = f; \
}

#define Midx_mainf(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = c1[0]; \
  begin_idx_aloop1((i1), k) { \
      if (c1[k]<f) f = c1[k]; \
  } end_idx_aloop1((i1), k); \
  *c2 = f; \
}

/* ===================== sum all the terms of an idx, m1, m2, ma ===== */
/*                       accumulate result in a 0D vector */

#define Midx_m1sumacc(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f; \
  size_t imax = (i1)->dim[0]; \
  ptrdiff_t i1_m0 = (i1)->mod[0]; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c2; \
  for (size_t i=0; i<imax; i++){ \
      f += *c1; \
      c1 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_m1supacc(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f; \
  size_t imax = (i1)->dim[0]; \
  ptrdiff_t i1_m0 = (i1)->mod[0]; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c2; \
  for (size_t i=0; i<imax; i++){ \
      if(*c1>f) f = *c1; \
      c1 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_m1infacc(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f; \
  size_t imax = (i1)->dim[0]; \
  ptrdiff_t i1_m0 = (i1)->mod[0]; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c2; \
  for (size_t i=0; i<imax; i++){ \
      if(*c1<f) f = *c1; \
      c1 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_m2sumacc(i1, i2, Type1, Type2) \
{ Type1 *c1, *c1_0; \
  Type2 *c2, f; \
  size_t imax = (i1)->dim[0], jmax = (i1)->dim[1]; \
  ptrdiff_t i1_m0 = (i1)->mod[0], i1_m1 = (i1)->mod[1]; \
  c1_0 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c2; \
  for (size_t i=0; i<imax; i++){ \
      c1 = c1_0; \
      for (size_t j=0; j<jmax; j++){  \
	  f += *c1; \
	  c1 += i1_m1; \
      } \
      c1_0 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_m2supacc(i1, i2, Type1, Type2) \
{ Type1 *c1, *c1_0; \
  Type2 *c2, f; \
  size_t imax = (i1)->dim[0], jmax = (i1)->dim[1]; \
  ptrdiff_t i1_m0 = (i1)->mod[0], i1_m1 = (i1)->mod[1]; \
  c1_0 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c2; \
  for (size_t i=0; i<imax; i++){ \
      c1 = c1_0; \
      for (size_t j=0; j<jmax; j++){  \
	  if(*c1>f) f = *c1; \
	  c1 += i1_m1; \
      } \
      c1_0 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_m2infacc(i1, i2, Type1, Type2) \
{ Type1 *c1, *c1_0; \
  Type2 *c2, f; \
  size_t imax = (i1)->dim[0], jmax = (i1)->dim[1]; \
  ptrdiff_t i1_m0 = (i1)->mod[0], i1_m1 = (i1)->mod[1]; \
  c1_0 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c2; \
  for (size_t i=0; i<imax; i++){ \
      c1 = c1_0; \
      for (size_t j=0; j<jmax; j++){  \
	  if(*c1<f) f = *c1; \
	  c1 += i1_m1; \
      } \
      c1_0 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_masumacc(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c2; \
  begin_idx_aloop1((i1), k) { \
	  f += c1[k]; \
  } end_idx_aloop1((i1), k); \
  *c2 = f; \
}

#define Midx_masupacc(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c2; \
  begin_idx_aloop1((i1), k) { \
	  if(c1[k]>f)  f = c1[k]; \
  } end_idx_aloop1((i1), k); \
  *c2 = f; \
}

#define Midx_mainfacc(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c2; \
  begin_idx_aloop1((i1), k) { \
	  if(c1[k]<f) f = c1[k]; \
  } end_idx_aloop1((i1), k); \
  *c2 = f; \
}

/* ============ sum the square of the terms of an idx, m1, m2, ma ===== */
/*                       result in a 0D vector */

/* sum all squared terms of a 1D vector */ 
/* result in a 0D vector */ 
#define Midx_m1sumsqr(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f; \
  size_t imax = (i1)->dim[0]; \
  ptrdiff_t i1_m0 = (i1)->mod[0]; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = (*c1)*(*c1); \
  c1 += i1_m0; \
  for (size_t i=1; i<imax; i++){ \
    f += (*c1)*(*c1); \
    c1 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_m2sumsqr(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type1 *c1_0; \
  Type2 *c2, f=0; \
  size_t imax = (i1)->dim[0], jmax = (i1)->dim[1]; \
  ptrdiff_t i1_m0 = (i1)->mod[0], i1_m1 = (i1)->mod[1]; \
  c1_0 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  for (size_t i=0; i<imax; i++){ \
      c1 = c1_0; \
      for (size_t j=0; j<jmax; j++){  \
	  f += (*c1)*(*c1); \
	  c1 += i1_m1; \
      } \
      c1_0 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_masumsqr(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f=0; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  begin_idx_aloop1((i1), k) { \
	  f += c1[k]*c1[k]; \
  } end_idx_aloop1((i1), k); \
  *c2 = f; \
}

/* ============ sum the square of the terms of an idx, m1, m2, ma ===== */
/*              accumulate result in a 0D vector */

#define Midx_m1sumsqracc(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f=0; \
  size_t imax = (i1)->dim[0]; \
  ptrdiff_t i1_m0 = (i1)->mod[0]; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c2; \
  for (size_t i=0; i<imax; i++){ \
      f += (*c1)*(*c1); \
      c1 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_m2sumsqracc(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type1 *c1_0; \
  Type2 *c2, f; \
  size_t imax = (i1)->dim[0], jmax = (i1)->dim[1]; \
  ptrdiff_t i1_m0 = (i1)->mod[0], i1_m1 = (i1)->mod[1]; \
  c1_0 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c2; \
  for (size_t i=0; i<imax; i++){ \
      c1 = c1_0; \
      for (size_t j=0; j<jmax; j++){  \
	  f += (*c1)*(*c1); \
	  c1 += i1_m1; \
      } \
      c1_0 += i1_m0; \
  } \
  *c2 = f; \
}

#define Midx_masumsqracc(i1, i2, Type1, Type2) \
{ Type1 *c1; \
  Type2 *c2, f; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  f = *c2; \
  begin_idx_aloop1((i1), k) { \
	  f += c1[k]*c1[k]; \
  } end_idx_aloop1((i1), k); \
  *c2 = f; \
}


/* ===================== dot product on m1, m2, ma ===== */
/*                       result in a 0D vector */


/* compute dot product M0 x M0 to M0 */ 
#define Midx_m0dotm0(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type3 *d1; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  *d1 = c1[0] * c2[0]; \
}


/* compute dot product M1 x M1 to M0 */ 
#define Midx_m1dotm1(i1, i2, o1, Type1, Type2, Type3) \
{ \
  Type3 *d1; \
  d1 = IDX_PTR((o1), Type3); \
  if((i1)->mod[0]==1) {\
      if((i2)->mod[0]==1) {\
	  Type1 *c1; \
	  Type2 *c2; \
	  Type3 f;\
	  size_t j = (i1)->dim[0]; \
	  c1 = IDX_PTR((i1), Type1); \
	  c2 = IDX_PTR((i2), Type2); \
	  f = 0; \
	  for (size_t i=0; i<j; i++)\
	      f += (*c1++)*(*c2++); \
	  *d1 = f; \
      } else {\
	  Type1 *c1; \
	  Type2 *c2; \
	  Type3 f;\
	  size_t j = (i1)->dim[0]; \
	  c1 = IDX_PTR((i1), Type1); \
	  c2 = IDX_PTR((i2), Type2); \
	  f = 0; \
	  ptrdiff_t c2_m0 = (i2)->mod[0];\
	  for (size_t i=0; i<j; i++) {\
	      f += (*c1++)*(*c2); \
	      c2 += c2_m0;\
	  }\
	  *d1 = f; \
      } \
  } else { \
      if((i2)->mod[0]==1) {\
	  Type1 *c1;\
	  Type2 *c2;\
	  Type3 f;\
	  size_t j = (i1)->dim[0]; \
	  c1 = IDX_PTR((i1), Type1); \
	  c2 = IDX_PTR((i2), Type2); \
	  f = 0; \
	  ptrdiff_t c1_m0 = (i1)->mod[0];\
	  for (size_t i=0; i<j; i++) {\
	      f += (*c1)*(*c2++); \
	      c1 += c1_m0;\
	  }\
	  *d1 = f; \
      } else {\
	  Type1 *c1;\
	  Type2 *c2;\
	  Type3 f;\
	  size_t j = (i1)->dim[0]; \
	  c1 = IDX_PTR((i1), Type1); \
	  c2 = IDX_PTR((i2), Type2); \
	  f = 0; \
	  ptrdiff_t c1_m0 = (i1)->mod[0];\
	  ptrdiff_t c2_m0 = (i2)->mod[0];\
	  for (size_t i=0; i<j; i++) {\
	      f += (*c1)*(*c2); \
	      c1 += c1_m0;\
	      c2 += c2_m0;\
	  }\
	  *d1 = f; \
      }\
  }\
}

#define Midx_m2dotm2(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type1 *c1_0; \
  Type2 *c2_0; \
  Type3 f=0; \
  size_t imax = (i1)->dim[0], jmax = (i2)->dim[1]; \
  ptrdiff_t c1_m1 = (i1)->mod[1], c2_m1 = (i2)->mod[1]; \
  ptrdiff_t c1_m0 = (i1)->mod[0], c2_m0 = (i2)->mod[0]; \
  Type3 *d1; \
  c1_0 = IDX_PTR((i1), Type1); \
  c2_0 = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++){ \
    c1 = c1_0; \
    c2 = c2_0; \
    for (size_t j=0; j<jmax; j++) { \
      f += (*c1)*(*c2); \
      c1 += c1_m1; \
      c2 += c2_m1; \
    } \
    c1_0 += c1_m0; \
    c2_0 += c2_m0; \
  } \
  *d1 = f; \
}

#define Midx_madotma(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type3 *d1, f=0; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  begin_idx_aloop2((i1),(i2), k, l) { \
    f += c1[k]*c2[l]; \
  } end_idx_aloop2((i1),(i2), k, l); \
  *d1 = f; \
}

/* ===================== dot product on m1, m2, ma ===== */
/*                       accumulate result in a 0D vector */

/* compute dot product M0 x M0 accumulated into M0 */ 
#define Midx_m0dotm0acc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type3 *d1; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  *d1 += c1[0] * c2[0]; \
}

/* compute dot product M1 x M1 accumulated into M0 */ 
#define Midx_m1dotm1acc(i1, i2, o1, Type1, Type2, Type3) \
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
    f += (*c1)*(*c2); \
    c1 += c1_m0; \
    c2 += c2_m0; \
  } \
  *d1 = f; \
}

/* compute dot product M1 x M1 accumulated into M0 */ 
#define Midx_m2dotm2acc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type1 *c1_0; \
  Type2 *c2_0; \
  size_t imax = (i1)->dim[0], jmax = (i2)->dim[1]; \
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
      f += (*c1)*(*c2); \
      c1 += c1_m1; \
      c2 += c2_m1; \
    } \
    c1_0 += c1_m0; \
    c2_0 += c2_m0; \
  } \
  *d1 = f; \
}

#define Midx_madotmaacc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type3 *d1, f; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  f = *d1; \
  begin_idx_aloop2((i1),(i2), k, l) { \
    f += c1[k]*c2[l]; \
  } end_idx_aloop2((i1),(i2), k, l); \
  *d1 = f; \
}

/* ===================== square distance on m1, m2, ma ===== */
/*                       result in a 0D vector */

#define Midx_m0sqrdist(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type3 *d1, f; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  f = c1[0] - c2[0]; \
  *d1 = f*f; \
}

#define Midx_m1sqrdist(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  ptrdiff_t c1_m0 = (i1)->mod[0], c2_m0 = (i2)->mod[0]; \
  Type3 *d1, f, g; \
  size_t imax = (i1)->dim[0]; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  g = (*c1) - (*c2); \
  f = g*g; \
  c1 += c1_m0; \
  c2 += c2_m0; \
  for (size_t i=1; i<imax; i++){ \
    g = (*c1) - (*c2); \
    f += g*g; \
    c1 += c1_m0; \
    c2 += c2_m0; \
  } \
  *d1 = f; \
}

#define Midx_m2sqrdist(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type1 *c1_0; \
  Type2 *c2_0; \
  ptrdiff_t c1_m0 = (i1)->mod[0], c2_m0 = (i2)->mod[0]; \
  ptrdiff_t c1_m1 = (i1)->mod[1], c2_m1 = (i2)->mod[1]; \
  Type3 *d1, f, g; \
  size_t imax = (i1)->dim[0], jmax = (i2)->dim[1]; \
  c1_0 = IDX_PTR((i1), Type1); \
  c2_0 = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  f = 0; \
  for (size_t i=0; i<imax; i++){ \
    c1 = c1_0; \
    c2 = c2_0; \
    for (size_t j=0; j<jmax; j++) { \
      g = (*c1) - (*c2); \
      f += g*g; \
      c1 += c1_m1; \
      c2 += c2_m1; \
    } \
    c1_0 += c1_m0; \
    c2_0 += c2_m0; \
  } \
  *d1 = f; \
}

#define Midx_masqrdist(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type3 *d1, f=0, g; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  begin_idx_aloop2((i1),(i2), k, l) { \
    g = c1[k]-c2[l]; \
    f += g*g; \
  } end_idx_aloop2((i1),(i2), k, l); \
  *d1 = f; \
}

/* ===================== square distance on m1, m2, ma ===== */
/*                       accumulate result in a 0D vector */

#define Midx_m0sqrdistacc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type3 *d1, f; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  f = c1[0] - c2[0]; \
  *d1 += f*f; \
}

/* compute square distance M1 x M1 accumulated into M0 */ 
#define Midx_m1sqrdistacc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  ptrdiff_t c1_m0 = (i1)->mod[0], c2_m0 = (i2)->mod[0]; \
  Type3 *d1, f, g; \
  size_t imax = (i1)->dim[0]; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  f = *d1; \
  for (size_t i=0; i<imax; i++){ \
    g = (*c1) - (*c2); \
    f += g*g; \
    c1 += c1_m0; \
    c2 += c2_m0; \
  } \
  *d1 = f; \
}

/* compute square distance M2 x M2 accumulated into M0 */ 
#define Midx_m2sqrdistacc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type1 *c1_0; \
  Type2 *c2_0; \
  ptrdiff_t c1_m0 = (i1)->mod[0], c2_m0 = (i2)->mod[0];   \
  ptrdiff_t c1_m1 = (i1)->mod[1], c2_m1 = (i2)->mod[1];   \
  Type3 *d1, f, g; \
  size_t imax = (i1)->dim[0], jmax = (i2)->dim[1]; \
  c1_0 = IDX_PTR((i1), Type1); \
  c2_0 = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  f = *d1; \
  for (size_t i=0; i<imax; i++){ \
    c1 = c1_0; \
    c2 = c2_0; \
    for (size_t j=0; j<jmax; j++) { \
      g = (*c1) - (*c2); \
      f += g*g; \
      c1 += c1_m1; \
      c2 += c2_m1; \
    } \
    c1_0 += c1_m0; \
    c2_0 += c2_m0; \
  } \
  *d1 = f; \
}

#define Midx_masqrdistacc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type3 *d1, f, g; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  d1 = IDX_PTR((o1), Type3); \
  f = *d1; \
  begin_idx_aloop2((i1),(i2), k, l) { \
    g = c1[k]-c2[l]; \
    f += g*g; \
  } end_idx_aloop2((i1),(i2), k, l); \
  *d1 = f; \
}

/* ========= multiplication by scalars (m0) ============================== */

#define Midx_m1dotm0(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 f; \
  Type3 *d1; \
  ptrdiff_t c1_m0 = (i1)->mod[0], d1_m0 = (o1)->mod[0]; \
  size_t imax = (o1)->dim[0]; \
  c1 = IDX_PTR((i1), Type1); \
  f = *(IDX_PTR((i2), Type2)); \
  d1 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++) { \
    *d1 = f * (*c1); \
    c1 += c1_m0; \
    d1 += d1_m0; \
  } \
}

#define Midx_m2dotm0(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 f; \
  Type3 *d1; \
  size_t c1_m0 = (i1)->mod[0], d1_m0 = (o1)->mod[0]; \
  size_t imax = (o1)->dim[0], jmax = (o1)->dim[1]; \
  Type1 *c1_0; \
  Type3 *d1_0; \
  ptrdiff_t c1_m1 = (i1)->mod[1], d1_m1 = (o1)->mod[1]; \
  c1_0 = IDX_PTR((i1), Type1); \
  f = *(IDX_PTR((i2), Type2)); \
  d1_0 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++) { \
    c1 = c1_0; \
    d1 = d1_0; \
    for (size_t j=0; j<jmax; j++) { \
      *d1 = f * (*c1); \
      c1 += c1_m1; \
      d1 += d1_m1; \
    } \
    c1_0 += c1_m0; \
    d1_0 += d1_m0; \
  } \
}

#define Midx_madotm0(i1, i2, o1, Type1, Type2, Type3) \
{ \
  Type1 *c1; \
  Type2 f; \
  Type3 *d1; \
  c1 = IDX_PTR(i1, Type1); \
  f = *(IDX_PTR((i2), Type2)); \
  d1 = IDX_PTR(o1, Type3); \
  begin_idx_aloop2((i1), (o1), k1, k2) { \
      d1[k2] = f*c1[k1]; \
  } end_idx_aloop2((i1), (o1), k1, k2); \
}


/* ========= multiplication by scalars (m0) with accumulation ====== */

#define Midx_m1dotm0acc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 f; \
  Type3 *d1; \
  ptrdiff_t c1_m0 = (i1)->mod[0], d1_m0 = (o1)->mod[0]; \
  size_t imax = (o1)->dim[0]; \
  c1 = IDX_PTR((i1), Type1); \
  f = *(IDX_PTR((i2), Type2)); \
  d1 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++) { \
    *d1 += f * (*c1); \
    c1 += c1_m0; \
    d1 += d1_m0; \
  } \
}

#define Midx_m2dotm0acc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 f; \
  Type3 *d1; \
  ptrdiff_t c1_m0 = (i1)->mod[0], d1_m0 = (o1)->mod[0]; \
  size_t jmax = (o1)->dim[1]; \
  Type1 *c1_0; \
  Type3 *d1_0; \
  ptrdiff_t intg c1_m1 = (i1)->mod[1], d1_m1 = (o1)->mod[1]; \
  size_t imax = (o1)->dim[0]; \
  c1_0 = IDX_PTR((i1), Type1); \
  f = *(IDX_PTR((i2), Type2)); \
  d1_0 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++) { \
    c1 = c1_0; \
    d1 = d1_0; \
    for (size_t j=0; j<jmax; j++) { \
      *d1 += f * (*c1); \
      c1 += c1_m1; \
      d1 += d1_m1; \
    } \
    c1_0 += c1_m0; \
    d1_0 += d1_m0; \
  } \
}

#define Midx_madotm0acc(i1, i2, o1, Type1, Type2, Type3) \
{ \
  Type1 *c1; \
  Type2 f; \
  Type3 *d1; \
  c1 = IDX_PTR(i1, Type1); \
  f = *(IDX_PTR((i2), Type2)); \
  d1 = IDX_PTR(o1, Type3); \
  begin_idx_aloop2((i1), (o1), k1, k2) { \
      d1[k2] += f*c1[k1]; \
  } end_idx_aloop2((i1), (o1), k1, k2); \
}


/* ========= addition with a scalar (m0) ============================== */

#define Midx_m0addm0(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 f; \
  Type3 *d1; \
  c1 = IDX_PTR((i1), Type1); \
  f = *(IDX_PTR((i2), Type2)); \
  d1 = IDX_PTR((o1), Type3); \
  *d1 = (*c1) + f; \
}

#define Midx_m1addm0(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 f; \
  Type3 *d1; \
  ptrdiff_t c1_m0 = (i1)->mod[0], d1_m0 = (o1)->mod[0]; \
  size_t imax = (o1)->dim[0]; \
  c1 = IDX_PTR((i1), Type1); \
  f = *(IDX_PTR((i2), Type2)); \
  d1 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++) { \
    *d1 = f + (*c1); \
    c1 += c1_m0; \
    d1 += d1_m0; \
  } \
}

#define Midx_m2addm0(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 f; \
  Type3 *d1; \
  ptrdiff_t c1_m0 = (i1)->mod[0], d1_m0 = (o1)->mod[0]; \
  size_t jmax = (o1)->dim[1]; \
  Type1 *c1_0; \
  Type3 *d1_0; \
  ptrdiff_t c1_m1 = (i1)->mod[1], d1_m1 = (o1)->mod[1]; \
  size_t imax = (o1)->dim[0]; \
  c1_0 = IDX_PTR((i1), Type1); \
  f = *(IDX_PTR((i2), Type2)); \
  d1_0 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++) { \
    c1 = c1_0; \
    d1 = d1_0; \
    for (size_t j=0; j<jmax; j++) { \
      *d1 = f + (*c1); \
      c1 += c1_m1; \
      d1 += d1_m1; \
    } \
    c1_0 += c1_m0; \
    d1_0 += d1_m0; \
  } \
}

#define Midx_maaddm0(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 f; \
  Type3 *d1; \
  c1 = IDX_PTR(i1, Type1); \
  f = *(IDX_PTR((i2), Type2)); \
  d1 = IDX_PTR(o1, Type3); \
  begin_idx_aloop2((i1), (o1), k1, k2) { \
      d1[k2] = f+c1[k1]; \
  } end_idx_aloop2((i1), (o1), k1, k2); \
}


/* ========= additions with scalars (m0) with accumulation ====== */

#define Midx_m0addm0acc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 f; \
  Type3 *d1; \
  c1 = IDX_PTR((i1), Type1); \
  f = *(IDX_PTR((i2), Type2)); \
  d1 = IDX_PTR((o1), Type3); \
  *d1 += (*c1) + f; \
}

#define Midx_m1addm0acc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 f; \
  Type3 *d1; \
  ptrdiff_t c1_m0 = (i1)->mod[0], d1_m0 = (o1)->mod[0]; \
  size_t imax = (o1)->dim[0]; \
  c1 = IDX_PTR((i1), Type1); \
  f = *(IDX_PTR((i2), Type2)); \
  d1 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++) { \
    *d1 += f + (*c1); \
    c1 += c1_m0; \
    d1 += d1_m0; \
  } \
}

#define Midx_m2addm0acc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 f; \
  Type3 *d1; \
  ptrdiff_t c1_m0 = (i1)->mod[0], d1_m0 = (o1)->mod[0]; \
  size_t jmax = (o1)->dim[1]; \
  Type1 *c1_0; \
  Type3 *d1_0; \
  ptrdiff_t c1_m1 = (i1)->mod[1], d1_m1 = (o1)->mod[1]; \
  size_t imax = (o1)->dim[0]; \
  c1_0 = IDX_PTR((i1), Type1); \
  f = *(IDX_PTR((i2), Type2)); \
  d1_0 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++) { \
    c1 = c1_0; \
    d1 = d1_0; \
    for (size_t j=0; j<jmax; j++) { \
      *d1 += f + (*c1); \
      c1 += c1_m1; \
      d1 += d1_m1; \
    } \
    c1_0 += c1_m0; \
    d1_0 += d1_m0; \
  } \
}

#define Midx_maaddm0acc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 f; \
  Type3 *d1; \
  c1 = IDX_PTR(i1, Type1); \
  f = *(IDX_PTR((i2), Type2)); \
  d1 = IDX_PTR(o1, Type3); \
  begin_idx_aloop2((i1), (o1), k1, k2) { \
      d1[k2] += f+c1[k1]; \
  } end_idx_aloop2((i1), (o1), k1, k2); \
}


/* ========= partial dot products for convolutions and matrix products == */

/* multiply M2 by M1, result in M1 */
/* matrix - vector product */
#define Midx_m2dotm1(i1, i2, o1, Type1, Type2, Type3) \
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
      for (size_t j=0; j<jmax; j++)  \
        f += (*c1++)*(*c2++); \
    else \
      for (size_t j=0; j<jmax; j++) { \
        f += (*c1)*(*c2); \
        c1 += c1_m1; \
        c2 += c2_m0; \
      } \
    *d1 = f; \
     d1 += d1_m0; \
     c1_0 += c1_m0; \
  } \
}

/* multiply M4 by M2, result in M2 */
#define Midx_m4dotm2(i1, i2, o1, Type1, Type2, Type3) \
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
      f = 0; \
      c1_2 = c1_1; \
      c2_0 = ker; \
      for (size_t k=0; k<kmax; k++) { \
        c1 = c1_2; \
        c2 = c2_0; \
        for (size_t l=0; l<lmax; l++) { \
          f += (*c1)*(*c2); \
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

/* ========= partial dot products for convolutions and matrix products == */
/*           with accumulation ============ */

/* multiply M2 by M1, result in M1 */
#define Midx_m2dotm1acc(i1, i2, o1, Type1, Type2, Type3) \
{ Type1 *c1; \
  Type2 *c2; \
  Type1  *c1_0; \
  Type2  *ker; \
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
      f += (*c1)*(*c2); \
      c1 += c1_m1; \
      c2 += c2_m0; \
    } \
    *d1 = f; \
    d1 += d1_m0; \
    c1_0 += c1_m0; \
  } \
}

/* multiply M4 by M2, result in M2 */
#define Midx_m4dotm2acc(i1, i2, o1, Type1, Type2, Type3) \
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
          f += (*c1)*(*c2); \
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


/* ========= outer products for back convolutions etc ================ */

#define Midx_m1extm1(i1, i2, o1, Type1, Type2, Type3) \
{ \
  Type2 *c2; \
  Type3 *d1; \
  Type1 *c1; \
  Type2 *c2_0; \
  Type3 *d1_0; \
  ptrdiff_t c2_m0 = (i2)->mod[0], d1_m1 = (o1)->mod[1]; \
  ptrdiff_t c1_m0 = (i1)->mod[0], d1_m0 = (o1)->mod[0]; \
  size_t jmax = (o1)->dim[1], imax = (o1)->dim[0]; \
  c1 = IDX_PTR((i1), Type1); \
  c2_0 = IDX_PTR((i2), Type2); \
  d1_0 = IDX_PTR((o1), Type3); \
  for (size_t i=0; i<imax; i++) { \
    d1 = d1_0; \
    c2 = c2_0; \
    for (size_t j=0; j<jmax; j++) { \
      *d1 = (*c1)*(*c2); \
      d1 += d1_m1; \
      c2 += c2_m0; \
    } \
    d1_0 += d1_m0; \
    c1 += c1_m0; \
  } \
} 

#define Midx_m2extm2(i1, i2, o1, Type1, Type2, Type3) \
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
          *d1_3 = (*c1_1)*(*c2_1); \
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

/* ========= outer products for back convolutions etc ================ */
/*           WITH accumulation */

#define Midx_m1extm1acc(i1, i2, o1, Type1, Type2, Type3) \
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
      *d1 += (*c1)*(*c2); \
      d1 += d1_m1; \
      c2 += c2_m0; \
    } \
    d1_0 += d1_m0; \
    c1 += c1_m0; \
  } \
} 

#define Midx_m2extm2acc(i1, i2, o1, Type1, Type2, Type3) \
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
          *d1_3 += (*c1_1)*(*c2_1); \
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

/* ============= term by term operations (mult, add...) ===== */
#define Midx_m0TOP(i1,i2,o1,Type1, Type2, Type3, OP) \
  { *IDX_PTR(o1, Type3) = \
              (*IDX_PTR(i1, Type1)) OP (*IDX_PTR(i2, Type2)); }

#define Midx_m1TOP(i0, i1,i2, Type1, Type2, Type3, OP) \
{ Type1 *c0; \
  Type2 *c1; \
  Type3 *c2; \
  size_t imax = (i2)->dim[0]; \
  ptrdiff_t i0_m0 = (i0)->mod[0], i1_m0 = (i1)->mod[0], i2_m0 = (i2)->mod[0]; \
  c0 = IDX_PTR((i0), Type1); \
  c1 = IDX_PTR((i1), Type2); \
  c2 = IDX_PTR((i2), Type3); \
  if(i0_m0==1 && i1_m0==1 && i2_m0==1) \
    for (size_t i=0; i<imax; i++) \
        *c2++ = (*c0++) OP (*c1++); \
  else \
    for (size_t i=0; i<imax; i++){ \
      *c2 = (*c0) OP (*c1); \
      c0 += i0_m0; \
      c1 += i1_m0; \
      c2 += i2_m0; \
    } \
}

#define Midx_m2TOP(i0,i1,i2, Type1, Type2, Type3, OP) \
{ Type1 *c0; \
  Type2 *c1; \
  Type3 *c2; \
  Type1 *c0_0; \
  Type2 *c1_0; \
  Type3 *c2_0; \
  size_t imax = (i2)->dim[0], jmax = (i1)->dim[1]; \
  ptrdiff_t i0_m0 = (i0)->mod[0], i1_m0 = (i1)->mod[0], i2_m0 = (i2)->mod[0]; \
  ptrdiff_t i0_m1 = (i0)->mod[1], i1_m1 = (i1)->mod[1], i2_m1 = (i2)->mod[1]; \
  c0_0 = IDX_PTR((i0), Type1); \
  c1_0 = IDX_PTR((i1), Type2); \
  c2_0 = IDX_PTR((i2), Type3); \
  for (size_t i=0; i<imax; i++){ \
      c0 = c0_0; c1 = c1_0; c2 = c2_0; \
      if(i0_m1==1 && i1_m1==1 && i2_m1==1) \
        for (size_t j=0; j<jmax; j++)  \
	  *c2++ = (*c0++) OP (*c1++); \
      else \
	for (size_t j=0; j<jmax; j++){  \
	  *c2 = (*c0) OP (*c1); \
	  c0 += i0_m1; \
	  c1 += i1_m1; \
	  c2 += i2_m1; \
      } \
      c0_0 += i0_m0; \
      c1_0 += i1_m0; \
      c2_0 += i2_m0; \
  } \
}

#define Midx_maTOP(i0, i1, i2, Type1, Type2, Type3, OP) \
{ \
  Type1 *fp0; \
  Type2 *fp1; \
  Type3 *fp2; \
  fp0 = IDX_PTR(i0, Type1); \
  fp1 = IDX_PTR(i1, Type2); \
  fp2 = IDX_PTR(i2, Type3); \
  begin_idx_aloop3((i0), (i1), (i2), k0, k1, k2) { \
      fp2[k2] =  fp0[k0] OP fp1[k1]; \
  } end_idx_aloop3((i0), (i1), (i2), k0, k1, k2); \
}

#define Midx_m0add(i0,i1,i2,t1,t2,t3) Midx_m0TOP(i0,i1,i2,t1,t2,t3,+)
#define Midx_m0sub(i0,i1,i2,t1,t2,t3) Midx_m0TOP(i0,i1,i2,t1,t2,t3,-)
#define Midx_m0mul(i0,i1,i2,t1,t2,t3) Midx_m0TOP(i0,i1,i2,t1,t2,t3,*)
#define Midx_m0div(i0,i1,i2,t1,t2,t3) Midx_m0TOP(i0,i1,i2,t1,t2,t3,/)

#define Midx_m1add(i0,i1,i2,t1,t2,t3) Midx_m1TOP(i0,i1,i2,t1,t2,t3,+)
#define Midx_m1sub(i0,i1,i2,t1,t2,t3) Midx_m1TOP(i0,i1,i2,t1,t2,t3,-)
#define Midx_m1mul(i0,i1,i2,t1,t2,t3) Midx_m1TOP(i0,i1,i2,t1,t2,t3,*)
#define Midx_m1div(i0,i1,i2,t1,t2,t3) Midx_m1TOP(i0,i1,i2,t1,t2,t3,/)

#define Midx_m2add(i0,i1,i2,t1,t2,t3) Midx_m2TOP(i0,i1,i2,t1,t2,t3,+)
#define Midx_m2sub(i0,i1,i2,t1,t2,t3) Midx_m2TOP(i0,i1,i2,t1,t2,t3,-)
#define Midx_m2mul(i0,i1,i2,t1,t2,t3) Midx_m2TOP(i0,i1,i2,t1,t2,t3,*)
#define Midx_m2div(i0,i1,i2,t1,t2,t3) Midx_m2TOP(i0,i1,i2,t1,t2,t3,/)

#define Midx_maadd(i0,i1,i2,t1,t2,t3) Midx_maTOP(i0,i1,i2,t1,t2,t3,+)
#define Midx_masub(i0,i1,i2,t1,t2,t3) Midx_maTOP(i0,i1,i2,t1,t2,t3,-)
#define Midx_mamul(i0,i1,i2,t1,t2,t3) Midx_maTOP(i0,i1,i2,t1,t2,t3,*)
#define Midx_madiv(i0,i1,i2,t1,t2,t3) Midx_maTOP(i0,i1,i2,t1,t2,t3,/)

/* ============= scalar operations on m0, m1, m2, ma ======== */

#define m0fop(i1,i2, Type1, Type2, OPER) \
{ Type1 *c1; \
  Type2 *c2; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  *c2 = OPER(*c1); \
}

#define m1fop(i1,i2, Type1, Type2, OPER) \
{ Type1 *c1; \
  Type2 *c2; \
  size_t imax = (i1)->dim[0]; \
  ptrdiff_t i1_m0 = (i1)->mod[0], i2_m0 = (i2)->mod[0]; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  for (size_t i=0; i<imax; i++){ \
      *c2 = OPER(*c1); \
      c1 += i1_m0; \
      c2 += i2_m0; \
  } \
}

#define m2fop(i1,i2, Type1, Type2, OPER) \
{ Type1 *c1; \
  Type2 *c2; \
  Type1 *c1_0; \
  Type2 *c2_0; \
  size_t imax = (i1)->dim[0], jmax = (i1)->dim[1]; \
  ptrdiff_t i1_m1 = (i1)->mod[1], i2_m1 = (i2)->mod[1]; \
  ptrdiff_t i1_m0 = (i1)->mod[0], i2_m0 = (i2)->mod[0]; \
  c1_0 = IDX_PTR((i1), Type1); \
  c2_0 = IDX_PTR((i2), Type2); \
  for (size_t i=0; i<imax; i++){ \
      c1 = c1_0; \
      c2 = c2_0; \
      for (size_t j=0; j<jmax; j++){  \
	  *c2 = OPER(*c1); \
	  c1 += i1_m1; \
	  c2 += i2_m1; \
      } \
      c1_0 += i1_m0; \
      c2_0 += i2_m0; \
  } \
}

#define mafop(i1,i2, Type1, Type2, OPER) \
{ Type1 *c1; \
  Type2 *c2; \
  c1 = IDX_PTR((i1), Type1); \
  c2 = IDX_PTR((i2), Type2); \
  begin_idx_aloop2( (i1), (i2), k, l) { \
      c2[l] = OPER(c1[k]); \
  } end_idx_aloop2( (i1), (i2), k, l) \
}


#define Midx_m0minus(i1,i2,Type1,Type2)  m0fop(i1,i2,Type1,Type2,-)
#define Midx_m0abs(i1,i2,Type1,Type2)    m0fop(i1,i2,Type1,Type2,Dabs)
#define Midx_m0sqrt(i1,i2,Type1,Type2)   m0fop(i1,i2,Type1,Type2,Dsqrt)
#define Midx_m0inv(i1,i2,Type1,Type2)    m0fop(i1,i2,Type1,Type2,Dinv)
#define Midx_m0qtanh(i1,i2,Type1,Type2)  m0fop(i1,i2,Type1,Type2,FQtanh)
#define Midx_m0qdtanh(i1,i2,Type1,Type2) m0fop(i1,i2,Type1,Type2,FQDtanh)
#define Midx_m0stdsigmoid(i1,i2,Type1,Type2) \
                                         m0fop(i1,i2,Type1,Type2,FQstdsigmoid)
#define Midx_m0dstdsigmoid(i1,i2,Type1,Type2) \
                                         m0fop(i1,i2,Type1,Type2,FQDstdsigmoid)
#define Midx_m0expmx(i1,i2,Type1,Type2)  m0fop(i1,i2,Type1,Type2,FQexpmx)
#define Midx_m0dexpmx(i1,i2,Type1,Type2) m0fop(i1,i2,Type1,Type2,FQDexpmx)
#define Midx_m0sin(i1,i2,Type1,Type2)    m0fop(i1,i2,Type1,Type2,Dsin)
#define Midx_m0cos(i1,i2,Type1,Type2)    m0fop(i1,i2,Type1,Type2,Dcos)
#define Midx_m0atan(i1,i2,Type1,Type2)   m0fop(i1,i2,Type1,Type2,Datan)
#define Midx_m0log(i1,i2,Type1,Type2)    m0fop(i1,i2,Type1,Type2,Dlog)
#define Midx_m0exp(i1,i2,Type1,Type2)    m0fop(i1,i2,Type1,Type2,Dexp)

#define Midx_m1minus(i1,i2,Type1,Type2)  m1fop(i1,i2,Type1,Type2,-)
#define Midx_m1abs(i1,i2,Type1,Type2)    m1fop(i1,i2,Type1,Type2,Dabs)
#define Midx_m1sqrt(i1,i2,Type1,Type2)   m1fop(i1,i2,Type1,Type2,Dsqrt)
#define Midx_m1inv(i1,i2,Type1,Type2)    m1fop(i1,i2,Type1,Type2,Dinv)
#define Midx_m1qtanh(i1,i2,Type1,Type2)  m1fop(i1,i2,Type1,Type2,FQtanh)
#define Midx_m1qdtanh(i1,i2,Type1,Type2) m1fop(i1,i2,Type1,Type2,FQDtanh)
#define Midx_m1stdsigmoid(i1,i2,Type1,Type2) \
                                         m1fop(i1,i2,Type1,Type2,FQstdsigmoid)
#define Midx_m1dstdsigmoid(i1,i2,Type1,Type2) \
                                         m1fop(i1,i2,Type1,Type2,FQDstdsigmoid)
#define Midx_m1expmx(i1,i2,Type1,Type2)  m1fop(i1,i2,Type1,Type2,FQexpmx)
#define Midx_m1dexpmx(i1,i2,Type1,Type2) m1fop(i1,i2,Type1,Type2,FQDexpmx)
#define Midx_m1sin(i1,i2,Type1,Type2)    m1fop(i1,i2,Type1,Type2,Dsin)
#define Midx_m1cos(i1,i2,Type1,Type2)    m1fop(i1,i2,Type1,Type2,Dcos)
#define Midx_m1atan(i1,i2,Type1,Type2)   m1fop(i1,i2,Type1,Type2,Datan)
#define Midx_m1log(i1,i2,Type1,Type2)    m1fop(i1,i2,Type1,Type2,Dlog)
#define Midx_m1exp(i1,i2,Type1,Type2)    m1fop(i1,i2,Type1,Type2,Dexp)

#define Midx_m2minus(i1,i2,Type1,Type2)  m2fop(i1,i2,Type1,Type2,-)
#define Midx_m2abs(i1,i2,Type1,Type2)    m2fop(i1,i2,Type1,Type2,Dabs)
#define Midx_m2sqrt(i1,i2,Type1,Type2)   m2fop(i1,i2,Type1,Type2,Dsqrt)
#define Midx_m2inv(i1,i2,Type1,Type2)    m2fop(i1,i2,Type1,Type2,Dinv)
#define Midx_m2qtanh(i1,i2,Type1,Type2)  m2fop(i1,i2,Type1,Type2,FQtanh)
#define Midx_m2qdtanh(i1,i2,Type1,Type2) m2fop(i1,i2,Type1,Type2,FQDtanh)
#define Midx_m2stdsigmoid(i1,i2,Type1,Type2) \
                                         m2fop(i1,i2,Type1,Type2,FQstdsigmoid)
#define Midx_m2dstdsigmoid(i1,i2,Type1,Type2) \
                                         m2fop(i1,i2,Type1,Type2,FQDstdsigmoid)
#define Midx_m2expmx(i1,i2,Type1,Type2)  m2fop(i1,i2,Type1,Type2,FQexpmx)
#define Midx_m2dexpmx(i1,i2,Type1,Type2) m2fop(i1,i2,Type1,Type2,FQDexpmx)
#define Midx_m2sin(i1,i2,Type1,Type2)    m2fop(i1,i2,Type1,Type2,Dsin)
#define Midx_m2cos(i1,i2,Type1,Type2)    m2fop(i1,i2,Type1,Type2,Dcos)
#define Midx_m2atan(i1,i2,Type1,Type2)   m2fop(i1,i2,Type1,Type2,Datan)
#define Midx_m2log(i1,i2,Type1,Type2)    m2fop(i1,i2,Type1,Type2,Dlog)
#define Midx_m2exp(i1,i2,Type1,Type2)    m2fop(i1,i2,Type1,Type2,Dexp)

#define Midx_maminus(i1,i2,Type1,Type2)  mafop(i1,i2,Type1,Type2,-)
#define Midx_maabs(i1,i2,Type1,Type2)    mafop(i1,i2,Type1,Type2,Dabs)
#define Midx_masqrt(i1,i2,Type1,Type2)   mafop(i1,i2,Type1,Type2,Dsqrt)
#define Midx_mainv(i1,i2,Type1,Type2)    mafop(i1,i2,Type1,Type2,Dinv)
#define Midx_maqtanh(i1,i2,Type1,Type2)  mafop(i1,i2,Type1,Type2,FQtanh)
#define Midx_maqdtanh(i1,i2,Type1,Type2) mafop(i1,i2,Type1,Type2,FQDtanh)
#define Midx_mastdsigmoid(i1,i2,Type1,Type2) \
                                         mafop(i1,i2,Type1,Type2,FQstdsigmoid)
#define Midx_madstdsigmoid(i1,i2,Type1,Type2) \
                                         mafop(i1,i2,Type1,Type2,FQDstdsigmoid)
#define Midx_maexpmx(i1,i2,Type1,Type2)  mafop(i1,i2,Type1,Type2,FQexpmx)
#define Midx_madexpmx(i1,i2,Type1,Type2) mafop(i1,i2,Type1,Type2,FQDexpmx)
#define Midx_masin(i1,i2,Type1,Type2)    mafop(i1,i2,Type1,Type2,Dsin)
#define Midx_macos(i1,i2,Type1,Type2)    mafop(i1,i2,Type1,Type2,Dcos)
#define Midx_maatan(i1,i2,Type1,Type2)   mafop(i1,i2,Type1,Type2,Datan)
#define Midx_malog(i1,i2,Type1,Type2)    mafop(i1,i2,Type1,Type2,Dlog)
#define Midx_maexp(i1,i2,Type1,Type2)    mafop(i1,i2,Type1,Type2,Dexp)

#define IDX_FOPS

/* ======================== outer products ============================== */


/* ======================== permutation through a table of I32 ========== */
/* permute an M1 using a vector of I32 */

#define Midx_m1permute(i1, per, o1, Type) \
{ \
  Type *c1, *d1; \
  long *c2; \
  size_t imax = (o1)->dim[0]; \
  ptrdiff_t per_m0 = (per)->mod[0], o1_m0 = (o1)->mod[0]; \
  c1 = IDX_PTR((i1), Type); \
  c2 = IDX_PTR((per), long); \
  d1 = IDX_PTR((o1), Type); \
  for (size_t i=0; i<imax; i++) { \
    *(d1) = c1[ *(c2) ]; \
    c2 += per_m0; \
    d1 += o1_m0; \
  } \
}

#define Midx_m2permute(i1, per, o1, Type) \
{ \
  Type *c1, *d1; \
  Type *d1_0; \
  long *c2; \
  long  *c2_0; \
  size_t imax = (o1)->dim[0]; \
  ptrdiff_t per_m1 = (per)->mod[1], o1_m1 = (o1)->mod[1]; \
  ptrdiff_t per_m0 = (per)->mod[0], o1_m0 = (o1)->mod[0]; \
  size_t jmax = (o1)->dim[1]; \
  c1   = IDX_PTR((i1), Type); \
  c2_0 = IDX_PTR((per), long); \
  d1_0 = IDX_PTR((o1), Type); \
  for (size_t i=0; i<imax; i++) { \
    c2 = c2_0; \
    d1 = d1_0; \
    for (size_t j=0; j<jmax; j++) { \
      *(d1) = c1[ *(c2) ]; \
      c2 += per_m1; \
      d1 += o1_m1; \
    } \
    c2_0 += per_m0; \
    d1_0 += o1_m0; \
  } \
}


/* END OF IDXOPS_H */
#endif


/* -------------------------------------------------------------
   Local Variables:
   c-font-lock-extra-types: (
     "FILE" "\\sw+_t" "at" "gptr" "real" "flt" "intg" "Type[0-9]*")
   End:
   ------------------------------------------------------------- */

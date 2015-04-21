
/*--------------------------------------------------------------------*/
/*--- Cojac-grind numerical problem sniffer.              oa_utils ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Cojac-grind, which watches arithmetic operations to
   detect overflows, cancellation, smearing, and other suspicious phenomena.

   Copyright (C) 2011-2011 Frederic Bapst
      frederic.bapst@gmail.com

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
 */

#include "pub_tool_basics.h"
#include "oa_include.h"
#include "oa_utils.h"
/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/


//------------- 64 <--> 64
static ULong longFromDouble(Double d) {
  oa_mix64_t m;
  m.f=d;
  return m.u;
}
double OA_(doubleFromULong)(ULong l) {
  oa_mix64_t m;
  m.u=l;
  return m.f;
}

//------------- 32 <--> 32
UInt OA_(uintFromInt)(Int s) {
  oa_mix32_t m;
  m.s=s;
  return m.u;
}
static Int intFromUInt(UInt u) {
  oa_mix32_t m;
  m.u=u;
  return m.s;
}
Float OA_(floatFromInt)(Int i) {
  oa_mix32_t m;
  m.s=i;
  return m.f;
}

//------------- 32 <--> 64
static ULong longFromTwoUInts(UInt a, UInt b) {
  ULong c= a;
  c = (c<<32) | b;
  return c;
}
ULong OA_(ulongFromTwoInts)(Int a, Int b) {
  return longFromTwoUInts(OA_(uintFromInt)(a), OA_(uintFromInt)(b));
}
static void longToTwoUInts(ULong d, UInt *a, UInt *b) {
  *b = d;
  *a = d>>32;
}
void OA_(longToTwoInts)(ULong d, Int *a, Int *b) {
  UInt ua, ub;
  longToTwoUInts(d, &ua, &ub);
  *a=intFromUInt(ua);
  *b=intFromUInt(ub);
}
static double doubleFromTwoUInts(UInt a, UInt b) {
  ULong c= longFromTwoUInts(a,b);
  return OA_(doubleFromULong)(c);
}
double OA_(doubleFromTwoInts)(Int a, Int b) {
  return doubleFromTwoUInts(OA_(uintFromInt)(a), OA_(uintFromInt)(b));
}
static void doubleToTwoUInts(double d, UInt *a, UInt *b) {
  longToTwoUInts(longFromDouble(d), a, b);
}
static void doubleToTwoInts(double d, Int *a, Int *b) {
  UInt ua, ub;
  doubleToTwoUInts(d, &ua, &ub);
  *a=intFromUInt(ua);
  *b=intFromUInt(ub);
}

//------------- 32 <--> 16
Short OA_(shortFromInt)(Int i) {
  union {Int i; Short s;} mix;
  mix.i=i;
  return mix.s;
}


/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/

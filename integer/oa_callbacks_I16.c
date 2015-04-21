
/*--------------------------------------------------------------------*/
/*--- Cojac-grind numerical problem sniffer.      oa_callbacks_I16 ---*/
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


#include "oa_include.h"
#include "oa_utils.h"
#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_libcprint.h"
//#include "pub_tool_libcassert.h"
//#include "pub_tool_debuginfo.h"
#include "pub_tool_threadstate.h"
#include <limits.h>

/*--------------------------------------------------------------------*/

//-----------------------------------------------------------------
/* These are called from the instrumented code, just before the operation. */
//-----------------------------------------------------------------

//--- I16 operations --- ----------------------------------------------------

static void check_Add16(Short a, Short b, OA_InstrumentContext inscon) {
  Int limit;
  if (a>0 && b>0) {
    limit=SHRT_MAX-a;
    if (b>limit) OA_(maybe_error)(Err_Overflow, inscon);
  } else if (a<0 && b<0) {
    limit=SHRT_MIN-a;
    if (b<limit) OA_(maybe_error)(Err_Overflow, inscon);
  }
}

static void check_Sub16(Short a, Short b, OA_InstrumentContext inscon) {
  Int limit;
  if (a>=0 && b<0) {
    limit= -(SHRT_MAX-a);
    if (b<limit) OA_(maybe_error)(Err_Overflow, inscon);
  } else if (a<0 && b>0) {
    limit= -(SHRT_MIN-a);
    if (b>limit) OA_(maybe_error)(Err_Overflow, inscon);
  }
}

static void check_Mul16(Short a, Short b, OA_InstrumentContext inscon) {
  if (b==0 || SHRT_MAX/b>a) return;
  OA_(maybe_error)(Err_Overflow, inscon);
}

/*--------------------------------------------------------------------*/

VG_REGPARM(3) void oa_callbackI32_2x16(Int a, Int b, OA_InstrumentContext ic) {
  Short sa, sb;
  sa=OA_(shortFromInt)(a);
  sb=OA_(shortFromInt)(b);
  switch(ic->op) {
    case Iop_Add16:  check_Add16(sa, sb, ic); break;
    case Iop_Sub16:  check_Sub16(sa, sb, ic); break;
    case Iop_Mul16:  check_Mul16(sa, sb, ic); break;
    default: break;
  }
}

VG_REGPARM(3) void oa_callbackI64_2x16(ULong la, ULong lb, OA_InstrumentContext ic) {
  Int a, a1, b, b1;
  OA_(longToTwoInts)(la, &a, &a1);
  OA_(longToTwoInts)(lb, &b, &b1);
  oa_callbackI32_2x16(a1,b1,ic);
}

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/

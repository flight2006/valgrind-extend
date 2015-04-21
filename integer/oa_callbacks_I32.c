
/*--------------------------------------------------------------------*/
/*--- Cojac-grind numerical problem sniffer.       a_callbacks_I32 ---*/
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
#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_libcprint.h"
//#include "pub_tool_libcassert.h"
#include "pub_tool_debuginfo.h"
#include "pub_tool_threadstate.h"
#include "pub_tool_stacktrace.h"
#include "oa_utils.h"
#include <limits.h>

/*--------------------------------------------------------------------*/

//--- I32 operations --- ----------------------------------------------------

static void check_Add32(Int a, Int b, OA_InstrumentContext inscon) {
  // Limitation: only signed int detection is applied...
  //VG_(message)(Vg_UserMsg, "HAHA add32: %d + %d, %s \n", a,b,inscon->string);
  //VG_(get_and_pp_StackTrace(VG_(get_running_tid)(), 1));
  Int limit;
  if (a>0 && b>0) {
    limit=INT_MAX-a;
    if (b>limit) OA_(maybe_error)(Err_Overflow, inscon);
  } else if (a<0 && b<0) {
    limit=INT_MIN-a;
    if (b<limit) OA_(maybe_error)(Err_Overflow, inscon);
  }
}

//static void check_AddU32(UInt a, UInt b, OA_InstrumentContext inscon) {
//  UInt limit=UINT_MAX-a;
//  if (b>limit) {
//    VG_(message)(Vg_UserMsg, "HAHA addu32: %u + %u %s \n", a,b,inscon->string);
//    oa_maybe_error(Err_OverflowAddU32, inscon);
//  }
//}


static void check_Sub32(Int a, Int b, OA_InstrumentContext inscon) {
  // Limitation: only signed int detection is applied...
  Int limit;
  if (a>=0 && b<0) {
    limit= -(INT_MAX-a);
    if (b<limit) OA_(maybe_error)(Err_Overflow, inscon);
  } else if (a<0 && b>0) {
    limit= -(INT_MIN-a);
    if (b>limit) OA_(maybe_error)(Err_Overflow, inscon);
  }
}

static void check_Mul32(UInt a, UInt b, OA_InstrumentContext inscon) {
  if (b==0 || UINT_MAX/b>a) return;
  OA_(maybe_error)(Err_Overflow, inscon);
}

static void check_MullS32(Int a, Int b, OA_InstrumentContext inscon) {
  if (b == 0) return;
  Int r = a * b;

  if ((b == -1 && a == INT_MIN) || r / b != a) {
    OA_(maybe_error)(Err_Overflow, inscon);
  }
}

static void check_DivS32(Int a, Int b, OA_InstrumentContext inscon) {
  if (b==0) {
    OA_(maybe_error)(Err_DivByZero, inscon);
  } else if (b==-1 && a==INT_MIN) {
    OA_(maybe_error)(Err_Overflow, inscon);
  }
}

static void check_32to16(Int a, OA_InstrumentContext inscon) {
  if(a>SHRT_MAX || a<SHRT_MIN)
    OA_(maybe_error)(Err_Cast, inscon);
}

//-----------------------------------------------------------------
/* These are called from the instrumented code, just before the operation. */
//-----------------------------------------------------------------

VG_REGPARM(2) void oa_callbackI32_1x32(Int a, OA_InstrumentContext ic) {
  switch(ic->op) {
    case Iop_32to16:  check_32to16(a,ic); break;
    default: break;
  }
}

VG_REGPARM(3) void oa_callbackI32_2x32(Int a, Int b, OA_InstrumentContext ic) {
  UInt ua=OA_(uintFromInt)(a);
  UInt ub=OA_(uintFromInt)(b);
  switch(ic->op) {
    case Iop_Add32:  check_Add32(a,b,ic);   break;
    case Iop_Sub32:  check_Sub32(a,b,ic);   break;
    case Iop_Mul32:  check_Mul32(ua,ub,ic); break;
    case Iop_DivS32: check_DivS32(a,b,ic);  break;  //not that relevant (widening mul)
    case Iop_MullS32:check_MullS32(a,b,ic); break;
    default: break;
  }
}

VG_REGPARM(2) void oa_callbackI64_1x32(ULong la, OA_InstrumentContext ic) {
  Int a, a1;
  OA_(longToTwoInts)(la, &a, &a1);
  oa_callbackI32_1x32(a,ic);
}


VG_REGPARM(3) void oa_callbackI64_2x32(ULong la, ULong lb, OA_InstrumentContext ic) {
  Int a, a1, b, b1;
  OA_(longToTwoInts)(la, &a, &a1);
  OA_(longToTwoInts)(lb, &b, &b1);
  //VG_(message)(Vg_UserMsg, "HUU: %ld %d § %d, %s \n", la, a,a1,ic->string);
  //VG_(get_and_pp_StackTrace(VG_(get_running_tid)(), 1));
  oa_callbackI32_2x32(a1,b1,ic);
}



/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/

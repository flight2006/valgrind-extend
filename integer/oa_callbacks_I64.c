
/*--------------------------------------------------------------------*/
/*--- Cojac-grind numerical problem sniffer.     a_callbacks_I64.c ---*/
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

//--- I64 operations --- ----------------------------------------------------

static void check_Add64(Long a, Long b, OA_InstrumentContext inscon) {
  // Limitation: only signed int detection is applied...
  //VG_(message)(Vg_UserMsg, "HAHA add32: %d + %d, %s \n", a,b,inscon->string);
  //VG_(get_and_pp_StackTrace(VG_(get_running_tid)(), 1));
  Long limit;
  if (a>0 && b>0) {
    limit=LONG_MAX-a;
    if (b>limit) OA_(maybe_error)(Err_Overflow, inscon);
  } else if (a<0 && b<0) {
    limit=LONG_MIN-a;
    if (b<limit) OA_(maybe_error)(Err_Overflow, inscon);
  }
}

static void check_Sub64(Long a, Long b, OA_InstrumentContext inscon) {
  // Limitation: only signed int detection is applied...
  Long limit;
  if (a>=0 && b<0) {
    limit= -(LONG_MAX-a);
    if (b<limit) OA_(maybe_error)(Err_Overflow, inscon);
  } else if (a<0 && b>0) {
    limit= -(LONG_MIN-a);
    if (b>limit) OA_(maybe_error)(Err_Overflow, inscon);
  }
}

static void check_Mul64(ULong a, ULong b, OA_InstrumentContext inscon) {
  if (b==0 || ULONG_MAX/b>a) return;
  OA_(maybe_error)(Err_Overflow, inscon);
}

static void check_MullS64(Long a, Long b, OA_InstrumentContext inscon) {
  if (b == 0) return;
  Long r = a * b;

  if ((b == -1L && a == LONG_MIN) || r / b != a) {
    OA_(maybe_error)(Err_Overflow, inscon);
  }
}

static void check_DivS64(Long a, Long b, OA_InstrumentContext inscon) {
  if (b==0) {
    OA_(maybe_error)(Err_DivByZero, inscon);
  } else if (b==-1L && a==LONG_MIN) {
    OA_(maybe_error)(Err_Overflow, inscon);
  }
}

static void check_64to32(Long a, OA_InstrumentContext inscon) {
  if(a>INT_MAX || a<INT_MIN)
    OA_(maybe_error)(Err_Cast, inscon);
}

/*--------------------------------------------------------------------*/
VG_REGPARM(2) void oa_callbackI64_1x64(Long a, OA_InstrumentContext ic) {
  switch(ic->op) {
    case Iop_64to32:  check_64to32(a,ic); break;
    default: break;
  }
}

VG_REGPARM(3) void oa_callbackI64_2x64(Long a, Long b, OA_InstrumentContext ic) {
	oa_mix64_t m;
	ULong ua; m.s=a; ua=m.u;
	ULong ub; m.s=b; ub=m.u;
  switch(ic->op) {
    //case Iop_Add64x2:
    case Iop_Add64:  check_Add64(a,b,ic); break;
    //case Iop_Sub64x2:
    case Iop_Sub64:  check_Sub64(a,b,ic); break;
    //case Iop_Mul64x2:
    case Iop_Mul64:  check_Mul64(a,b,ic); break;
    case Iop_MullS64: check_MullS64(a,b,ic); break;
    //case Iop_Div64x2:
    case Iop_DivS64:  check_DivS64(a,b,ic); break;
    default: break;
  }
}


/* x86 platform only !
 * As only Int32 can be passed to the callback, the trick here is
 * to split the callback in 2 successive calls.
 * This horror is completely wrong when there are multiple threads!
 * TODO: consider threadId and manage one context per thread...
 * TODO: find a better way (pass pointers? ensure atomicity?...)
 */

static partOfF64op add64_buf;
static char        isAdd64Part=False;

VG_REGPARM(2) void oa_callbackI32_1x64(UInt a, OA_InstrumentContext ic) {
  if (isAdd64Part) { // second half of the callback
    isAdd64Part=False;
    oa_mix64_t m;
    Long la; m.halves.h1.s=a; m.halves.h2.s=add64_buf.a; la=m.s;
    oa_callbackI64_1x64(la, ic);
  } else {            // first half of the callback
    isAdd64Part=True;
    add64_buf.a=a;
    add64_buf.tid=VG_(get_running_tid)();
  }
}

VG_REGPARM(3) void oa_callbackI32_2x64(UInt a, UInt b, OA_InstrumentContext ic) {
  if (isAdd64Part) { // second half of the callback
    isAdd64Part=False;
    oa_mix64_t m;
    Long la; m.halves.h1.s=a; m.halves.h2.s=add64_buf.a; la=m.s;
    Long lb; m.halves.h1.s=b; m.halves.h2.s=add64_buf.b; lb=m.s;
    oa_callbackI64_2x64(la, lb, ic);
  } else {            // first half of the callback
    isAdd64Part=True;
    add64_buf.a=a;
    add64_buf.b=b;
    add64_buf.tid=VG_(get_running_tid)();
  }
}

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/

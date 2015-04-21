
/*--------------------------------------------------------------------*/
/*--- Cojac-grind numerical problem sniffer.          oa_include.h ---*/
/* This is a private header file for use only in the cojac/ folder.   */
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


#ifndef __OA_INCLUDE_H
#define __OA_INCLUDE_H

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"

#define OA_(str)    VGAPPEND(vgCojac_,str)

typedef enum {
	Err_Overflow,
	Err_Cast,
	Err_Cancellation,
	Err_NaN,
	Err_Infinity,
	Err_Precision,
	Err_Math,
	Err_DivByZero,
	Err_Underflow
} OA_ErrorTag;


#define COJAC_FILE_LEN  4096
#define COJAC_FCT_LEN   256

typedef struct {
	Addr  addr;
	IROp  op;
	Bool  isLocated;
	Char* string;
} OA_InstrumentContext_;

typedef OA_InstrumentContext_*  OA_InstrumentContext;

typedef struct {
  Int  stacktraceDepth;
  Bool isAggr;
  Bool i32;
  Bool f32;
  Bool f64;
  Bool i16;
  Bool i64;
  Bool castToI16;
} cojacOptions;

typedef struct {
  Int tid;
} cojacErrorExtra_;

typedef cojacErrorExtra_* cojacErrorExtra;

typedef struct {
  const char* name;
  void*       callbackI32;  // for x86   arch
  void*       callbackI64;  // for amd64 arch
  Long        occurrences;
} Iop_Cojac_attributes;


typedef struct {
	Int a;
	Int b;
	ThreadId tid;
} partOfF64op;

//-----------------------------------------------------------------
extern cojacOptions OA_(options);
//-----------------------------------------------------------------

Iop_Cojac_attributes* OA_(get_Iop_struct)(IROp op);


//-----------------------------------------------------------------
/* called from the instrumented code, just before the operation.
 * There are constraints, not always well-documented, for such "dirty calls":
 * - corresponding IExpr effective param must be "flat" (no nested subexpr)
 * - for x86:   only I32 params, maximum 3 params
 * - for amd64: only I64 params, max 5-6 params
 */
VG_REGPARM(3) void oa_callbackI32_2x32 ( Int a,  Int b, OA_InstrumentContext c);
VG_REGPARM(2) void oa_callbackI32_1x32 ( Int a,         OA_InstrumentContext c);
 VG_REGPARM(3) void oa_callbackI32_2x16 ( Int a,  Int b, OA_InstrumentContext c);
 VG_REGPARM(3) void oa_callbackI32_2x64 (UInt a, UInt b, OA_InstrumentContext c);
VG_REGPARM(2) void oa_callbackI32_1x64 (UInt a,         OA_InstrumentContext c);
//VG_REGPARM(3) void oa_callbackI32_2xF32( Int a,  Int b, OA_InstrumentContext c);
//VG_REGPARM(3) void oa_callbackI32_2xF64(UInt a, UInt b, OA_InstrumentContext c);

 VG_REGPARM(3) void oa_callbackI64_2x32 (ULong a, ULong b, OA_InstrumentContext c);
 VG_REGPARM(2) void oa_callbackI64_1x32 (ULong a,          OA_InstrumentContext c);
 VG_REGPARM(3) void oa_callbackI64_2x64 ( Long a,  Long b, OA_InstrumentContext c);
VG_REGPARM(2) void oa_callbackI64_1x64 ( Long a,          OA_InstrumentContext c);
 VG_REGPARM(3) void oa_callbackI64_2x16 (ULong a, ULong b, OA_InstrumentContext c);
//VG_REGPARM(3) void oa_callbackI64_2xF32(ULong a, ULong b, OA_InstrumentContext c);
//VG_REGPARM(3) void oa_callbackI64_2xF64(ULong a, ULong b, OA_InstrumentContext c);

/*------------------------------------------------------------*/
/*--- Errors and suppressions                              ---*/
/*------------------------------------------------------------*/

// For error signalling
void OA_(maybe_error)(ErrorKind ekind, OA_InstrumentContext inscon);

// As required for VG_(needs_tool_errors) (pub_tool_tooliface.h
Bool OA_(eq_Error)           ( VgRes res, Error* e1, Error* e2 );
void OA_(before_pp_Error)    ( Error* err );
void OA_(pp_Error)           ( Error* err );
UInt OA_(update_Error_extra) ( Error* err );
Bool OA_(is_recognised_suppression) ( Char* name, Supp* su );
Bool OA_(read_extra_suppression_info) ( Int fd, Char** buf, SizeT* nBuf, Supp *su );
Bool OA_(error_matches_suppression) ( Error* err, Supp* su );
Bool OA_(get_extra_suppression_info) ( Error* err, /*OUT*/Char* buf, Int nBuf );
Char* OA_(get_error_name) ( Error* err );


#endif /* ndef __OA_INCLUDE_H */

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/

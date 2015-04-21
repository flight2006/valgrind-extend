
/*--------------------------------------------------------------------*/
/*--- Cojac-grind numerical problem sniffer.          oa_error_mgt ---*/
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
//#include "pub_tool_debuginfo.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_threadstate.h"
#include "pub_tool_stacktrace.h"
#include <limits.h>
#include <inttypes.h>

/*--------------------------------------------------------------------*/
static ULong nErrors=0L;
static const ULong nErrorsMax=1000000L;  // after that, no more malloc in errors
/*--------------------------------------------------------------------*/
static const char * strFromErrorKind(ErrorKind errKind) {
  switch(errKind) {
    case Err_Overflow:        return "Overflow";
    case Err_Cast:            return "Cast";
    case Err_Cancellation:    return "Cancellation";
    case Err_NaN:             return "NaN";
    case Err_Infinity:        return "Infinity";
    case Err_Precision:       return "Precision";
    case Err_Math:            return "Math";
    case Err_DivByZero:       return "DivByZero";
    case Err_Underflow:       return "Underflow";
    default:                  return "Unknown error kind...";
    //VG_(message)(Vg_UserMsg, "strFromErrorKind %d ", errKind);
  }
}

/*--------------------------------------------------------------------*/
static void oa_maybe_error_extra(ErrorKind ekind, Char* s, Addr addr, void* extra)  {
  ThreadId tid=VG_(get_running_tid)();
  VG_(maybe_record_error)(tid, ekind, addr, s, extra);
}
void OA_(maybe_error)(ErrorKind ekind, OA_InstrumentContext inscon)  {
  // it would be possible to move this computation here:
  //get_debug_info((Addr)(inscon->addr), filename, fctname, &line);
  Char thisFct[]="maybe_error";
  cojacErrorExtra extra=NULL;
	nErrors++;
	if (nErrors<nErrorsMax) {
    extra=VG_(malloc)(thisFct, sizeof(cojacErrorExtra_));
    extra->tid=VG_(get_running_tid)();
  } else if (nErrors%(10L*nErrorsMax)==0) {
  	VG_(message)(Vg_UserMsg, "A lot of errors: %" PRIu64 "...\n", nErrors);
  }
  oa_maybe_error_extra(ekind, inscon->string, inscon->addr, extra);
}

/*--------------------------------------------------------------------*/
/*--- error reporting                                              ---*/
/*--------------------------------------------------------------------*/

Bool OA_(eq_Error) (VgRes res, Error* e1, Error* e2) {
  ErrorKind k1 = VG_(get_error_kind)(e1);
  ErrorKind k2 = VG_(get_error_kind)(e2);
  Addr addr1  =  VG_(get_error_address)(e1);
  Addr addr2  =  VG_(get_error_address)(e2);
  if (addr1 != (Addr)NULL && addr1==addr2  && k1==k2)
    return True;
  return False;
  //Char * msg1  = VG_(get_error_string) (e1);
  //Char * msg2  = VG_(get_error_string) (e2);
  //if (VG_STREQ(msg1, msg2)) ...
}

void OA_(before_pp_Error) ( Error* err ) { }

void OA_(pp_Error) ( Error* err ) {
  Char *detail=VG_(get_error_string)(err);
  if (detail==NULL) detail="";
  ErrorKind errKind = VG_(get_error_kind)(err);
  VG_(message)(Vg_UserMsg, "Cojac: %s, %s", strFromErrorKind(errKind), detail);
  cojacErrorExtra extra = (cojacErrorExtra)( VG_(get_error_extra)(err) );
  Int depth=OA_(options).stacktraceDepth;
  if (depth==0 || extra==NULL) return;
  VG_(get_and_pp_StackTrace)(extra->tid, depth);  // This stupidly adds an extra newline...
  
}

UInt OA_(update_Error_extra) ( Error* err ) {
  return 0;
}

Bool OA_(is_recognised_suppression) ( Char* name, Supp* su ) {
  return False;
}

Bool OA_(read_extra_suppression_info) ( Int fd, Char** buf, SizeT* nBuf, Supp *su ) {
  return True;
}

Bool OA_(error_matches_suppression) ( Error* err, Supp* su ) {
  return False;
}

Bool OA_(get_extra_suppression_info) ( Error* err, /*OUT*/Char* buf, Int nBuf ) {
  return False;
}

Char* OA_(get_error_name) ( Error* err ) {
  return NULL;
}

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/

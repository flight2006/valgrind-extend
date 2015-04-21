
/*--------------------------------------------------------------------*/
/*--- Nulgrind: The minimal Valgrind tool.               nl_main.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Nulgrind, the minimal Valgrind tool,
   which does no instrumentation or analysis.

   Copyright (C) 2002-2012 Nicholas Nethercote
      njn@valgrind.org

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
#include "pub_tool_tooliface.h"

/* Pulled in to get the threadstate */
#include "../coregrind/pub_core_basics.h"
#include "../coregrind/pub_core_vki.h"
#include "../coregrind/pub_core_vkiscnums.h"
#include "../coregrind/pub_core_threadstate.h"

static VG_REGPARM(0) void instrument_Unop(IRSB* sbOut,)
{
	
}
static VG_REGPARM(0) void instrument_Binop(IRSB* sbOut,VexGuestArchState* vex)
{
	struct vki_user_regs_struct regs;
	VG_(memset)(&regs, 0, sizeof(regs));
#if defined(VGP_x86_linux)
	   regs.eflags = LibVEX_GuestX86_get_eflags(vex);
#elif defined(VGP_amd64_linux)
	   regs.eflags = LibVEX_GuestAMD64_get_rflags(vex);
#elif defined(VGP_arm_linux)
	   regs.ARM_cpsr = LibVEX_GuestARM_get_cpsr(vex);
#else
#  error Unknown arch
#endif
}
static VG_REGPARM(0) void instrument_Triop(IRSB* sbOut,)
{
	
}
static VG_REGPARM(0) void instrument_Qop(IRSB* sbOut,)
{
	
}
static VG_REGPARM(0) void instrument_Mux0X(IRSB* sbOut,)
{
	
}

static void nl_post_clo_init(void)
{
}

static
IRSB* nl_instrument ( VgCallbackClosure* closure,
                      IRSB* sbIn,
                      VexGuestLayout* layout, 
                      VexGuestExtents* vge,
                      IRType gWordTy, IRType hWordTy )
{
	IRSB* sbOut;
        Addr current_addr = 0;
	int i;
	sbOut = deepCopyIRSBExceptStmts(sbIn);
	i = 0;
	while (i < sbIn->stmts_used && sbIn->stmts[i]->tag != Ist_IMark) {
		addStmtToIRSB( sbOut, sbIn->stmts[i] );
      		i++;
   	}

    // Iterate over the remaining stmts to generate instrumentation.
    tl_assert(bb_in->stmts_used > 0);
    tl_assert(i >= 0);
    tl_assert(i < bb_in->stmts_used);
    tl_assert(bb_in->stmts[i]->tag == Ist_IMark);
    
	for(; i < sbIn->stmts_used; i++) {
		IRStmt* st = sbIn->stmts[i];
      		if (!st || st->tag == Ist_NoOp) continue;
		switch (st->tag) {
         		case Ist_NoOp:
         		case Ist_AbiHint:
        	 	case Ist_Put:
         		case Ist_PutI:
         		case Ist_MBE:
            			break;
         		case Ist_IMark:	
				current_addr = st->Ist.IMark.addr;
               		 	break;
			case Ist_WrTmp:
				switch (expr->tag) {
                  			case Iex_Load:
            					break;
                  			case Iex_Unop:
                     				instrument_Unop( sbOut, OpLoad, type );
                     				break;
                  			case Iex_Binop:
                     				instrument_Binop( sbOut, OpLoad, type );
                     				break;
                  			case Iex_Triop:
                     				instrument_Triop( sbOut, OpLoad, type );
                     				break;
                  			case Iex_Qop:
                     				instrument_Qop( sbOut, OpLoad, type );
                     				break;
                  			case Iex_Mux0X:
                     				instrument_Mux0X( sbOut, OpAlu, type );
                     				break;
                  			default:
                     				break;
               			}
			case Ist_Store:
			case Ist_Dirty:
			case Ist_CAS:
			case Ist_LLSC:
			case Ist_Exit:
            			break;
			default:
            			tl_assert(0);
	}
    addStmtToIRSB( sbOut, st );
	
    return bb;
}

static void nl_fini(Int exitcode)
{
}

static void nl_pre_clo_init(void)
{
   VG_(details_name)            ("Nulgrind");
   VG_(details_version)         (NULL);
   VG_(details_description)     ("the minimal Valgrind tool");
   VG_(details_copyright_author)(
      "Copyright (C) 2002-2012, and GNU GPL'd, by Nicholas Nethercote.");
   VG_(details_bug_reports_to)  (VG_BUGS_TO);

   VG_(details_avg_translation_sizeB) ( 275 );

   VG_(basic_tool_funcs)        (nl_post_clo_init,
                                 nl_instrument,
                                 nl_fini);

   /* No needs, no core events to track */
}

VG_DETERMINE_INTERFACE_VERSION(nl_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/

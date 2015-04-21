
/*--------------------------------------------------------------------*/
/*--- Sconvcheck:                                        sc_main.c ---*/
/*--------------------------------------------------------------------*/

/*
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
#include "pub_tool_libcassert.h"

#include "pub_tool_hashtable.h" 
#include "pub_tool_libcprint.h"
#include "pub_tool_debuginfo.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcfile.h" 
#include "pub_tool_options.h"
#include "pub_tool_libcproc.h"

#include "symtrace.h"
#include "sc_malloc_wrappers.h"

/* --- Globals --- */ 

HWord caMainBBCounter = 0; 


static void sc_post_clo_init(void)
{
}

static
IRSB* sc_instrument ( VgCallbackClosure* closure,
                      IRSB* sb_in,
                      VexGuestLayout* layout, 
                      VexGuestExtents* vge,
                      IRType gWordTy, IRType hWordTy )
{
   Int i;
   IRStmt* st;
//   Addr current_addr = 0;
   IRSB* sb_out; 

   if (gWordTy != hWordTy) {
      /* We don't currently support this case. */
      VG_(tool_panic)("host/guest word size mismatch");
   }

   /* Check we're not completely nuts */
   tl_assert(sizeof(UWord)  == sizeof(void*));
   tl_assert(sizeof(Word)   == sizeof(void*));
   tl_assert(sizeof(Addr)   == sizeof(void*));
   tl_assert(sizeof(ULong)  == 8);
   tl_assert(sizeof(Long)   == 8);
   tl_assert(sizeof(Addr64) == 8);
   tl_assert(sizeof(UInt)   == 4);
   tl_assert(sizeof(Int)    == 4);

   /* Set up SB */
   sb_out = deepCopyIRSBExceptStmts(sb_in);

   /* Copy verbatim any IR preamble preceding the first IMark */
   i = 0;
   while (i < sb_in->stmts_used && sb_in->stmts[i]->tag != Ist_IMark) {

      st = sb_in->stmts[i];
      tl_assert(st);
      tl_assert(isFlatIRStmt(st));

      addStmtToIRSB( sb_out, sb_in->stmts[i] );
      i++;
   }

   // Iterate over the remaining stmts to generate instrumentation.
   tl_assert(sb_in->stmts_used > 0);
   tl_assert(i >= 0);
   tl_assert(i < sb_in->stmts_used);
   tl_assert(sb_in->stmts[i]->tag == Ist_IMark);

   for (/* use current i*/; i < sb_in->stmts_used; i++) {

      st = sb_in->stmts[i];
      if (!st || st->tag == Ist_NoOp) continue;
      
      if (st->tag == Ist_Exit)
	{
//	  cgAddExitPre(sb_out,st);
	}

      addStmtToIRSB(sb_out, st );      
      traceIRStmt(sb_out, st, caMainBBCounter);
//      isIRStmt(sb_out, st, caMainBBCounter); 
//      cgIRStmt(sb_out, st, caMainBBCounter); 
     
      if (st->tag == Ist_Exit)
	{
//	  cgAddExitPost(sb_out,st);  
	}
   }
   caMainBBCounter++;
   return sb_out;
}

static void sc_fini(Int exitcode)
{
/*
   IntType type;
   HWord nType = VG_(HT_count_nodes)(ogTypeOf); 
   HWord nVar = VG_(HT_count_nodes)(ogVarOf);
   VG_(message)(Vg_UserMsg, "the number of ogTypeOf is %lu,the number of ogVarOf is %lu\n",nType,nVar);
   HWord index = 0;
   while(index < nType)
   {
	OgTypeOfHashNode* typenode = VG_(HT_lookup)(ogTypeOf,index);
	type = typenode->type;
	VG_(message)(Vg_UserMsg, "varname %lu has a %u type  \n",typenode->varname,type );
	if(typenode && (typenode->type == Bot))
	{
		
		VG_(message)(Vg_UserMsg, "varname %lu has a conflict type  \n",typenode->varname );
		   	
	}
   	index++;
   }
*/
   if (VG_(clo_verbosity) == 1 && !VG_(clo_xml)) {                               
      VG_(message)(Vg_UserMsg,                                                            
                   "For counts of detected and suppressed errors, rerun with: -v\n");                  
                                                                     
   }                                                                                

}

static void sc_pre_clo_init(void)
{
   VG_(details_name)            ("SconvCheck");
   VG_(details_version)         (NULL);
   VG_(details_description)     ("a Valgrind tool to check the signedness conversion error ");
   VG_(details_copyright_author)(
      "Copyright (C) 2002-2012, and GNU GPL'd, by Nicholas Nethercote.");
   VG_(details_bug_reports_to)  (VG_BUGS_TO);

   VG_(details_avg_translation_sizeB) ( 275 );

   VG_(basic_tool_funcs)        (sc_post_clo_init,
                                 sc_instrument,
                                 sc_fini);

   ogVarOf  = VG_(HT_construct) ("ogVarOf");
   ogTypeOf = VG_(HT_construct) ("ogTypeOf"); 
   ogAddrOf = VG_(HT_construct) ("ogAddrOf");

   /* Track type constraints for new stack entries */ 
   VG_(track_new_mem_stack)(&EmitStackHelper); 

   /* Register callbacks before and after syscalls */ 
/*   VG_(needs_syscall_wrapper) ( sc_pre_syscall,
				sc_post_syscall); 
*/
   VG_(needs_var_info)();
   /* Declare that we need malloc replacements */ 
   VG_(needs_malloc_replacement)  ( SC_malloc,
                                    SC__builtin_new,
                                    SC__builtin_vec_new,
                                    SC_memalign,
                                    SC_calloc,
                                    SC_free,
                                    SC__builtin_delete,
                                    SC__builtin_vec_delete,
                                    SC_realloc,
				    SC_malloc_usable_size,
                                    SC_MALLOC_REDZONE_SZB );
   //add initialization for some variables with marking signed or unsigned
}

VG_DETERMINE_INTERFACE_VERSION(sc_pre_clo_init)


/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/

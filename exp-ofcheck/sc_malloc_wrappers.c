/*--------------------------------------------------------------------*/
/*--- CatchConversions                        sc_malloc_wrappers.c ---*/
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


/* Wrappers for malloc() and friends. Check for bogus args, then manage */
/* constraints for region-based memory allocation. */ 

#include "pub_tool_basics.h"
#include "pub_tool_errormgr.h"     
#include "pub_tool_execontext.h"   
#include "pub_tool_hashtable.h"    
#include "pub_tool_libcbase.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_options.h"
#include "pub_tool_replacemalloc.h"
#include "pub_tool_threadstate.h"
#include "sc_malloc_wrappers.h" 
#include "symtrace.h"
//#include "ca_interval.h" 

/* Here we report errors */
/* Do a printf-style operation on either the XML or normal output
   channel, depending on the setting of VG_(clo_xml).
*/
/*
static void emit_WRK ( HChar* format, va_list vargs )
{
   if (VG_(clo_xml)) {
      VG_(vprintf_xml)(format, vargs);
   } else {
      VG_(vmessage)(Vg_UserMsg, format, vargs);
   }
}
static void emit ( HChar* format, ... ) PRINTF_CHECK(1, 2);
static void emit ( HChar* format, ... )
{
   va_list vargs;
   va_start(vargs, format);
   emit_WRK(format, vargs);
   va_end(vargs);
}
static void emiN ( HChar* format, ... ) // NO FORMAT CHECK 
{
   va_list vargs;
   va_start(vargs, format);
   emit_WRK(format, vargs);
   va_end(vargs);
}
*/
/*SC_pp_error()
{
    
}
*/

HWord sc_output_comment = 0;      // Do we output comments in const. gen?
                                  // Default is no. 

void * SC_new_block(ThreadId tid, SizeT n)
{
  Addr p; 
  SizeT align;
  // caChunkTy * newChunk; 

  align = VG_(clo_alignment); 
  //VG_(message)(Vg_UserMsg, "before VG_(cli_malloc)\n"); 
  p = (Addr)VG_(cli_malloc)(align, n); 
  //VG_(message)(Vg_UserMsg, "after VG_(cli_malloc)\n"); 
  if (sc_output_comment)
  {
    VG_(message)(Vg_UserMsg, "XXX MALLOC_NEW_BLOCK tid %u ", tid); 
    VG_(message)(Vg_UserMsg, "base %lx ", p); 
    VG_(message)(Vg_UserMsg, "size %lu \n", n); 
  }

  if (p)
    {
      /*      newChunk = caInsertInterval(p,n,caIntervalTable); 
	      caDeclareChunk(newChunk); */
    }

  return (void *)p; 

}

Bool bad_malloc_arg(SizeT n)
{

  /* Check to see if malloc arg is negative. If so, we have a bug. */ 

  return ((SSizeT)n < 0); 
}

Bool bad_calloc_args(SizeT n1, SizeT n2)
{
  /* Check to see if either  arg is negative. If so, we have a bug. */ 
  return ( ((SSizeT)n1 < 0 || (SSizeT)n2 < 0)); 


}



void * SC_malloc(ThreadId tid, SizeT n)
{
//  VG_(message)(Vg_UserMsg, "Test 1 %p \n", &SC_malloc);  //test
  if (bad_malloc_arg(n))
    {
//      VG_(message)(Vg_UserMsg, "Test 2 \n");  //test
      VG_(message)(Vg_UserMsg, "XXX EVIL malloc arg %ld \n", (SSizeT)n); 
      return NULL;
    }
  else 
    {
//      VG_(message)(Vg_UserMsg, "Test 3 \n");  //test
      return SC_new_block(tid, n);
    }
}
 
void * SC__builtin_new(ThreadId tid, SizeT n)
{

 if (bad_malloc_arg(n))
    {
      VG_(message)(Vg_UserMsg, "XXX EVIL __builtin_new arg %ld \n", (SSizeT)n); 

      return NULL;
    }
  else 
    {
      return SC_new_block(tid, n);
    }

}

void * SC__builtin_vec_new(ThreadId tid, SizeT n) 
{

 if (bad_malloc_arg(n))
    {
	VG_(message)(Vg_UserMsg, "XXX EVIL __builtin_vec_new arg %ld \n", (SSizeT)n); 

      return NULL;
    }
  else 
    {
      return SC_new_block(tid, n);
    }

}

void * SC_memalign(ThreadId tid, SizeT align, SizeT n) 
{
 if (bad_calloc_args(align,n))
    {

	VG_(message)(Vg_UserMsg, "XXX EVIL memalign args %ld", align);
	VG_(message)(Vg_UserMsg, " %ld \n", n); 

      return NULL;
    }
  else 
    {
      return SC_new_block(tid, n);
    }
}

void * SC_calloc(ThreadId tid, SizeT n1, SizeT n2) 
{
  if (bad_calloc_args(n1,n2))
  {
      VG_(message)(Vg_UserMsg, "XXX EVIL calloc args %ld ",n1); 
      VG_(message)(Vg_UserMsg, " %ld \n", n2);

    return NULL; 
  }
      else
  {
    return SC_new_block(tid, n1*n2);
  }
      
}

void SC_free(ThreadId tid, void * ptr) 
{
  if (sc_output_comment)
  {
    VG_(message)(Vg_UserMsg, "XXX FREE %x \n", ptr); 
  }
  //  VG_(free)(ptr); 
}

void SC__builtin_delete(ThreadId tid, void * ptr) 
{
  if (sc_output_comment)
  {
    VG_(message)(Vg_UserMsg, "XXX FREE %x \n", ptr); 
  }
  // VG_(free)(ptr); 
}

void SC__builtin_vec_delete(ThreadId tid, void * ptr) 
{
  if (sc_output_comment)
  {
    VG_(message)(Vg_UserMsg, "XXX FREE %x \n", ptr); 
  }
  // VG_(free)(ptr); 
}

void * SC_realloc(ThreadId tid, void * ptr, SizeT n) 
{

  Addr p; 

  if (bad_malloc_arg(n))
    {
	VG_(message)(Vg_UserMsg, "XXX EVIL realloc arg %ld", (SSizeT)n); 

      return NULL;
    }
  else 
    {

      p = (Addr) SC_new_block(tid, n);
      if (p)
	{
	  VG_(memcpy)( (void *) p, ptr, n);  // this isn't right
	                                     // since n should be old_size
	  SC_free(tid, ptr); 
	}

      return (void *)p; 
    }

 
}
SizeT SC_malloc_usable_size( ThreadId tid, void* p )
{
  return 0;
}

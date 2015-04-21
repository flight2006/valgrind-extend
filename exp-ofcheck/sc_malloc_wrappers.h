/*--------------------------------------------------------------------*/
/*--- CatchConversions                    sc_malloc_wrappers.h     ---*/
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


/* Header file for catchconv malloc wrappers. */

#include "pub_tool_basics.h" 

#define SC_MALLOC_REDZONE_SZB 16 

/*
typedef
struct __caMemRegionHashNode {
  struct __caMemRegionHashNode * next;
  HWord addr;
  HWord size;  
}
caMemRegionHashNode; 
*/


/* --- Prototypes --- */ 

Bool bad_malloc_arg(SizeT n);
Bool bad_calloc_args(SizeT n1, SizeT n2);


void * SC_new_block(ThreadId tid,SizeT n);
void * SC_malloc(ThreadId tid, SizeT n);
void * SC__builtin_new(ThreadId tid, SizeT n);
void * SC__builtin_vec_new(ThreadId tid, SizeT n);
void * SC_memalign(ThreadId tid, SizeT n1, SizeT n2);
void * SC_calloc(ThreadId tid, SizeT n1, SizeT n2);
void SC_free(ThreadId tid, void * ptr);
void SC__builtin_delete(ThreadId tid, void * ptr);
void SC__builtin_vec_delete(ThreadId tid, void * ptr);
SizeT SC_malloc_usable_size( ThreadId tid, void* p );
void * SC_realloc(ThreadId tid, void * ptr, SizeT n);



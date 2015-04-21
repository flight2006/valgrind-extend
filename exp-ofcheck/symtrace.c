/*--------------------------------------------------------------------*/
/*--- symble trace                                      symtrace.c ---*/
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
#include "pub_tool_libcassert.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_debuginfo.h" 
#include "pub_tool_libcbase.h"
#include "pub_tool_options.h"
#include "pub_tool_hashtable.h" 
#include "pub_tool_mallocfree.h"
#include "pub_tool_machine.h"       // VG_(fnptr_to_fnentry)
#include "pub_tool_threadstate.h"
#include "valgrind.h"

#include "libvex_basictypes.h"
#include "libvex_ir.h"
#include "libvex.h"

#include "../include/vki/vki-posixtypes-amd64-linux.h"
#include "../include/vki/vki-linux.h"
#include "../include/vki/vki-amd64-linux.h"
#include "../include/vki/vki-scnums-amd64-linux.h"

#include "../VEX/pub/libvex_guest_amd64.h"
#include "../VEX/pub/libvex_guest_arm.h"
#include "../VEX/pub/libvex_basictypes.h"

#include "../coregrind/pub_core_libcsetjmp.h"    // to keep _threadstate.h happy
#include "../coregrind/pub_core_threadstate.h"
#include "../coregrind/pub_core_libcproc.h"

#include "sc_main_util.h"
#include "symtrace.h"
#include "sc_malloc_wrappers.h"

#include<stdlib.h>
/* --- Globals --- */

static HWord counter = 0; 
static HWord curtyvar = 0; 
static Bool isRealloc = 0;
static HWord argNum = 0;  


/* --- Utility Functions --- */ 
 void setHelperAnns ( IRDirty* di ) {
   /*   di->nFxState = 2;
   di->fxState[0].fx     = Ifx_Read;
   di->fxState[0].offset = mce->layout->offset_SP;
   di->fxState[0].size   = mce->layout->sizeof_SP;
   di->fxState[1].fx     = Ifx_Read;
   di->fxState[1].offset = mce->layout->offset_IP;
   di->fxState[1].size   = mce->layout->sizeof_IP;
   */
   di->nFxState = 0; 
}


IntTyStateHint opToHint(IROp op)
{
   IntTyStateHint ret; 

   switch (op){
      case (Iop_CmpLT32S):
      case (Iop_CmpLT64S):
      case (Iop_CmpLE32S):
      case (Iop_CmpLE64S):
      {
	 ret = SignedHint;
	 break;
      }
      case(Iop_CmpLT32U):
      case(Iop_CmpLT64U):
      case(Iop_CmpLE32U):
      case(Iop_CmpLE64U):
      {
	 ret = UnsignedHint;
	 break;
      }
      default:
         ret = HopelessHint; /* all our dreams are dying of overdoses */ 
         break; 
   }
   return ret; 
}

int clearVarOf(HWord loc)
{
  OgVarOfHashNode * node; 

  node = VG_(HT_remove)(ogVarOf,loc);
  if (!node) return -1; 
  else VG_(free)(node);

  return 0; 
}

int setAddrOf(HWord varname, Addr addr)
{
  OgAddrOfHashNode * node;

  node = VG_(HT_lookup)(ogAddrOf,varname);
  if (!node) 
    {
      /* allocate node for loc, since we are setting VarOf(loc) */ 
      node = (OgAddrOfHashNode *)VG_(malloc)("HashNode", sizeof(OgAddrOfHashNode));
      node->varname = varname; 
      VG_(HT_add_node)(ogAddrOf,node); 
    }  
  vassert(node->varname == varname); 
  node->addr = addr;
  return 0; 
}

Addr getAddrOf(HWord varname)
{
  HWord ret = 0; 
  OgAddrOfHashNode * node;

  node = VG_(HT_lookup)(ogAddrOf,varname);
  if (!node) return ret; // this isn't checked! 

  vassert(node->varname == varname);
  ret = node->addr;
  return ret; 
}

int setVarOf(HWord loc, HWord varname)
{
  OgVarOfHashNode * node;

  vassert(varname != -1); // if triggers, means we have an unchecked GetVarOf

  node = VG_(HT_lookup)(ogVarOf,loc);
  if (!node) 
    {
      /* allocate node for loc, since we are setting VarOf(loc) */ 
      node = (OgVarOfHashNode *)VG_(malloc)("HashNode", sizeof(OgVarOfHashNode));
      node->loc = loc; 
      VG_(HT_add_node)(ogVarOf,node); 
    }  
  vassert(node->loc == loc); 
  node->varname = varname;
  return 0; 
}

HWord getVarOf(HWord loc)
{
  HWord ret = -1; 
  OgVarOfHashNode * node;

  node = VG_(HT_lookup)(ogVarOf,loc);
  if (!node) return ret; // this isn't checked! 

  vassert(node->loc == loc);
  ret = node->varname;
  return ret; 
}

int setTypeOf(HWord varname, IntType type)
{
  OgTypeOfHashNode * node; 

  node = VG_(HT_lookup)(ogTypeOf,varname);
  if (!node)
    { 
      /* Allocate and add node for varname, since we are setting TypeOf()*/ 
      node = (OgTypeOfHashNode *)VG_(malloc)("HashNode", sizeof(OgTypeOfHashNode));
      node->varname = varname; 
      VG_(HT_add_node)(ogTypeOf,node);
    } 
  vassert(node->varname == varname); 
  node->type = type;

  /* Debugging only for now: */ 
/*  switch (type)
    {
    case (Bot) : VG_(printf)("XXX Varname var.%u set to Bot. \n",varname); break;
    case (Top) : VG_(printf)("XXX Varname var.%u set to Top. \n",varname); break;
    case (SignedTy) : VG_(printf)("XXX Varname var.%u set to SIGNED. \n",varname); 
      break;
    case (UnsignedTy) : VG_(printf)("XXX Varname var.%u set to UNSIGNED. \n",varname); 
      break;
    case (BogusTy) : vpanic("Set variable to bogus type!\n");
    }
*/
  return 0; 
}

IntType getTypeOf(HWord varname)
{
  IntType ret = BogusTy; 
  OgTypeOfHashNode * node;

  node = VG_(HT_lookup)(ogTypeOf,varname);
  if (!node) return ret;

  vassert(node->varname == varname); 
  ret = node->type; 
  return ret; 
}

/* Special case function for Top/Signed/Unsigned/Bot lattice */ 
IntType setTypeOfToMeet(HWord varname, IntType rhstype)
{
  IntType curType;
  
  curType = getTypeOf(varname); 

  switch (curType)
    { 
    case (BogusTy):
      setTypeOf(varname,rhstype); /* No current type, gets rhstype */
      return rhstype;
    case (Top):
      setTypeOf(varname,rhstype); 
      return rhstype;
      break;
    case (Bot):  
      setTypeOf(varname,Bot);
      return Bot; 
      break; 
    case (SignedTy):
      if ((rhstype == UnsignedTy)
	  || (rhstype == Bot))
	{
	  setTypeOf(varname,Bot);
	  return Bot; 
	}
      return SignedTy; 
      break; 
    case (UnsignedTy):
      if ((rhstype == SignedTy)
	  || (rhstype == Bot))
	{
	  setTypeOf(varname,Bot); 
	  return Bot;
	}
      return UnsignedTy; 
      break;
    default:
      /* Should never reach here */
      vpanic("Reached default in setTypeOfToMeet\n");
      break; 
    }
  /* Should never reach here */ 
  vpanic("Reached end of setTypeOfToMeet \n"); 
  return 0;  
}

/* Takes location to an HWord. */
/* Assumes that all memory addresses accessed > 60 decimal; this */
/* is realistic for Valgrind on Linux */ 
/* Call _before_ call to getVarOf or setVarOf */ 
HWord locToHashKey(HWord loc1, HWord loc2, LocType ltype)
{
  switch (ltype)
    {
    case (MemLoc): return loc1;
    case(RegLoc): return loc1;
    case(TmpLoc): return ( (loc1<<14) | loc2); 
    case(ErrorLoc): vpanic("ErrorLoc in locToHashKey.\n"); return 0;
    default: return 0;
    }
  return 0; 
}

/* --- Instrumentation Functions --- */ 

void EmitStackHelper(Addr stackAddr, SizeT len)
{
   int i;

   for (i = 0; i < len; i++)
     {
       setVarOf(locToHashKey(stackAddr+i,0,MemLoc),curtyvar);  
       curtyvar++;
     }

   return; 
}

void EmitGetTmp2RegHelper(HWord lhs_tmp, HWord offset, HWord eip)
{
  if (getVarOf(locToHashKey(offset,0,RegLoc)) == -1)
    {
	setVarOf(locToHashKey(offset,0,RegLoc),curtyvar);
	curtyvar++;
    }
  setVarOf(locToHashKey(lhs_tmp,eip,TmpLoc),getVarOf(locToHashKey(offset,0,RegLoc)));
  return;
}

static VG_REGPARM(2) void trace_error(SizeT reg, Addr addr, Addr relateaddr)
{
  if((SSizeT)reg < 0)
    {
	VG_(message)(Vg_UserMsg, "ERROR alloc arg %ld (instruction address at %lx )\n", (SSizeT)reg,addr);
	//VG_printf("ERROR alloc arg %ld ",(SSizeT)reg);
	if(relateaddr != 0)
		VG_(message)(Vg_UserMsg, "The origin source come from 0x%lx\n", relateaddr);
		//VG_printf("(come from 0x%x)",addr);
	//VG_(message)(Vg_UserMsg, "\n");
	//VG_printf("\n");
		  	
    }
}

void AddGetHelper(IRSB* sb, IRTemp lhs, Int offset, Addr addr)
{
  ULong offset_cur = (HWord)offset;
  ULong lhs_int = (HWord)lhs;
  IRDirty* d1,*d2;
 
  d1 = unsafeIRDirty_0_N(0, "EmitGetTmp2RegHelper",
		        &EmitGetTmp2RegHelper,
		        mkIRExprVec_3(
				      mkIRExpr_HWord(lhs_int),
				      mkIRExpr_HWord(offset_cur),
				      mkIRExpr_HWord(counter)
				     )
		       );
  setHelperAnns(d1);
  addStmtToIRSB(sb,IRStmt_Dirty(d1));
  
  Addr relateaddr = getAddrOf(getVarOf(locToHashKey(offset_cur,0,RegLoc)));

  ThreadId tid = VG_(get_running_tid)();
  tl_assert(VG_INVALID_THREADID != tid);
  ThreadState* tst = VG_(get_ThreadState)(tid);

//  SizeT argRDI = tst->arch.vex.guest_RDI;
//  SizeT argRSI = tst->arch.vex.guest_RSI;
  VexGuestArchState* t = &(tst->arch.vex);
  ULong argRDI;
  ULong argRSI;
/*
  if(offset == 64)
  {
	argRSI = *(&(t->guest_RSI));
	VG_(message)(Vg_UserMsg, "argRSI         %ld \n", argRSI);
	VG_(message)(Vg_UserMsg, "(SSizeT)argRSI %ld \n", (SSizeT)argRSI);
  }
*/
  //VG_(message)(Vg_UserMsg, "addr is %lx \n", addr);

  if(isRealloc)
    {
	if(argNum == 1 && offset == 64)
	  {
		//argRSI = *(&(tst->arch.vex) + offset);
		argRSI = *(&(t->guest_RSI));
//		VG_(message)(Vg_UserMsg, "realloc arg %lu \n", argRSI);
/*		if((SSizeT)argRSI < 0)
		  {
			VG_(message)(Vg_UserMsg, "ERROR realloc arg %ld ", (SSizeT)argRSI);
			//VG_printf("ERROR realloc arg %ld ",(SSizeT)argRSI);
			if(relateaddr != 0)
			  VG_(message)(Vg_UserMsg, "(come from 0x%lx)", relateaddr);
			  //VG_printf("(come from 0x%x)",relateaddr);
			VG_(message)(Vg_UserMsg, "\n");
		 	//VG_printf("\n");
		  	
		  }	
*/
		
  		d2 = unsafeIRDirty_0_N(2, "trace_error",
		        		&trace_error,
		        		mkIRExprVec_3(
				      			mkIRExpr_HWord(argRSI),
							mkIRExpr_HWord(addr),
				      			mkIRExpr_HWord(relateaddr)
				     			)
					);
  		setHelperAnns(d2);
  		addStmtToIRSB(sb,IRStmt_Dirty(d2));	

		argNum--;
		isRealloc = 0;	
	  }
    }
  else if(argNum > 0)
    {
	if(offset == 72)
	  {
		argRDI = *(&(t->guest_RDI));
//		VG_(message)(Vg_UserMsg, "alloc arg %lu \n", argRDI);
/*		if((SSizeT)argRDI < 0)
		  {
			VG_(message)(Vg_UserMsg, "ERROR malloc/calloc/new/[] new first arg %ld ", (SSizeT)argRDI);
			//VG_printf("ERROR malloc/calloc/new/[] new first arg %ld ",(SSizeT)argRDI);
			if(relateaddr != 0)
			  VG_(message)(Vg_UserMsg, "(come from 0x%lx)", relateaddr);
			  //VG_printf("(come from 0x%x)",relateaddr);
			VG_(message)(Vg_UserMsg, "\n");
		 	//VG_printf("\n");
		  	
		  }
*/
  		d2 = unsafeIRDirty_0_N(2, "trace_error",
		        		&trace_error,
		        		mkIRExprVec_3(
				      			mkIRExpr_HWord(argRDI),
							mkIRExpr_HWord(addr),
				      			mkIRExpr_HWord(relateaddr)
				     			)
					);
  		setHelperAnns(d2);
  		addStmtToIRSB(sb,IRStmt_Dirty(d2));

		argNum--;
	  }
	else if(offset == 64)
	  {
		argRSI = *(&(t->guest_RSI));
//		VG_(message)(Vg_UserMsg, "alloc arg %lu \n", argRSI);
/*		if((SSizeT)argRSI < 0)
		  {
			VG_(message)(Vg_UserMsg, "ERROR calloc/memalign second arg %ld ", (SSizeT)argRSI);
			//VG_printf("ERROR calloc/memalign second arg %ld ",(SSizeT)argRSI);
			if(relateaddr != -1)
		 	  VG_(message)(Vg_UserMsg, "(come from 0x%lx)", relateaddr);
			  //VG_printf("(come from 0x%x)",relateaddr);
			VG_(message)(Vg_UserMsg, "\n");
		 	//VG_printf("\n");
		  	
		  }
*/
  		d2 = unsafeIRDirty_0_N(2, "trace_error",
		        		&trace_error,
		        		mkIRExprVec_3(
				      			mkIRExpr_HWord(argRSI),
							mkIRExpr_HWord(addr),
				      			mkIRExpr_HWord(relateaddr)
				     			)
					);
  		setHelperAnns(d2);
  		addStmtToIRSB(sb,IRStmt_Dirty(d2));

		argNum--;
	  }
    }

  return;
}

void EmitRdTmpTmp2TmpHelper(HWord lhs, HWord rhs, HWord eip)
{
  if (getVarOf(locToHashKey(rhs,eip,TmpLoc)) == -1)
    {
	setVarOf(locToHashKey(rhs,eip,TmpLoc),curtyvar);
	curtyvar++;
    }
  setVarOf(locToHashKey(lhs,eip,TmpLoc),getVarOf(locToHashKey(rhs,eip,TmpLoc)));
  return;
}

void AddRdTmpHelper(IRSB* sb, IRTemp lhs, IRTemp rhs)
{
  IRDirty * d; 
  ULong lhs_int, rhs_int;
  
  lhs_int = (HWord)lhs;
  rhs_int = (HWord)rhs; 

  d = unsafeIRDirty_0_N(0, "EmitRdTmpTmp2TmpHelper", 
			&EmitRdTmpTmp2TmpHelper,
			mkIRExprVec_3(
				      mkIRExpr_HWord(lhs_int),
				      mkIRExpr_HWord(rhs_int),
				      mkIRExpr_HWord(counter)
				      )
			);
  setHelperAnns(d);
  addStmtToIRSB(sb,IRStmt_Dirty(d)); 

  return;  
}

void EmitTmpHelper(HWord tmp, IntTyStateHint hint, HWord eip, Addr addr)
{
  IntType newTy = 0;
  
  /* Check whether rhs arg has a tyvar */ 
  if (getVarOf(locToHashKey(tmp,eip,TmpLoc)) == -1)
    {
      /* No, so add one */ 
      setVarOf(locToHashKey(tmp,eip,TmpLoc),curtyvar);
      curtyvar++;
    }

  newTy = setTypeOfToMeet(getVarOf(locToHashKey(tmp,eip,TmpLoc)),hint); 

  if (newTy == Bot)
    {
      /* Check whether the tyvar has a addr */
      if(getAddrOf(getVarOf(locToHashKey(tmp,eip,TmpLoc))) == 0)
	{
	  /* No, so add one*/
	  setAddrOf(getVarOf(locToHashKey(tmp,eip,TmpLoc)),addr);
	} 
    }

  return; 
}

void AddOpRhsTypeHelper(IRSB* sb, IRExpr* arg, IntTyStateHint hint, Addr addr)
{
  IRDirty * d; 
  HWord tmpname; 

  switch (arg->tag)
    {
      case(Iex_RdTmp):
        tmpname = (HWord)arg->Iex.RdTmp.tmp; 

        d = unsafeIRDirty_0_N(0, "EmitTmpHelper", 
			      &EmitTmpHelper,
			      mkIRExprVec_4(
					    mkIRExpr_HWord(tmpname),
					    mkIRExpr_HWord(hint),
					    mkIRExpr_HWord(counter),
					    mkIRExpr_HWord(addr)
					    )
			      );
        setHelperAnns(d); 
        addStmtToIRSB(sb,IRStmt_Dirty(d)); 
        break;
      default: 
        break; 
    }
  return; 
}


IntType BinopTypeStateFunc(HWord op, IntType arg1Type, IntType arg2Type)
{
  IntType ret = Top;
  return ret;
}

void EmitNewTmpTyvarHelper(HWord lhs, HWord eip)
{
  setVarOf(locToHashKey(lhs,eip,TmpLoc),curtyvar);
  curtyvar++;
}

/* Now below two function have not any reality effect on lhs */
void EmitBinopTmpTmpTypeHelper(HWord lhs, HWord op, HWord arg1, HWord arg2, HWord eip)
{
  IntType lhsType = Top;
  IntType arg1Type = Top;
  IntType arg2Type = Top;

  arg1Type = getTypeOf(getVarOf(locToHashKey(arg1,eip,TmpLoc)));
  arg2Type = getTypeOf(getVarOf(locToHashKey(arg2,eip,TmpLoc)));
  lhsType = BinopTypeStateFunc(op, arg1Type, arg2Type);

  /* Check whether lhs arg has a tyvar */ 
  if (getVarOf(locToHashKey(lhs,eip,TmpLoc)) == -1)
    {
      /* No, so add one */ 
      setVarOf(locToHashKey(lhs,eip,TmpLoc),curtyvar);
      curtyvar++;
    }

  setTypeOf(getVarOf(locToHashKey(lhs,eip,TmpLoc)),lhsType);
  
  return;
}

void EmitBinopTmpConstTypeHelper(HWord lhs, HWord op, HWord tmp, HWord eip)
{
  IntType lhsType = Top;
  IntType arg1Type = Top;
  IntType constType = Top;

  arg1Type = getTypeOf(getVarOf(locToHashKey(tmp,eip,TmpLoc)));
  lhsType = BinopTypeStateFunc(op, arg1Type, constType);

  /* Check whether lhs arg has a tyvar */ 
  if (getVarOf(locToHashKey(tmp,eip,TmpLoc)) == -1)
    {
      /* No, so add one */ 
      setVarOf(locToHashKey(tmp,eip,TmpLoc),curtyvar);
      curtyvar++;
    }
  setTypeOf(getVarOf(locToHashKey(lhs,eip,TmpLoc)),lhsType);
  
  return;
}

void AddBinopHelper(IRSB* sb,IRStmt* st)
{
  IROp op;
  IRDirty* d1;
  IRDirty* d2;
  IRExpr* arg1;
  IRExpr* arg2;
  HWord lhs,tmpname;
//  HWord cur_ctr = (HWord)counter;

  vassert(st->tag = Ist_WrTmp);
  op = (HWord)st->Ist.WrTmp.data->Iex.Binop.op;
  arg1 = st->Ist.WrTmp.data->Iex.Binop.arg1;
  arg2 = st->Ist.WrTmp.data->Iex.Binop.arg2;
  
  lhs = (HWord)st->Ist.WrTmp.tmp;
  d1 = unsafeIRDirty_0_N(0, "EmitNewTmpTyvarHelper", 
			&EmitNewTmpTyvarHelper,
			mkIRExprVec_2(
				      mkIRExpr_HWord(lhs),
				      mkIRExpr_HWord(counter)
				      )
			);
  setHelperAnns(d1);
  addStmtToIRSB(sb,IRStmt_Dirty(d1));

  if (arg1->tag == Iex_RdTmp && arg2->tag == Iex_RdTmp)
    {
      d2 = unsafeIRDirty_0_N(0, "EmitBinopTmpTmpTypeHelper", 
			     &EmitBinopTmpTmpTypeHelper,
			     mkIRExprVec_5(
			        	   mkIRExpr_HWord(lhs),
				           mkIRExpr_HWord(op),
				           mkIRExpr_HWord((HWord)arg1->Iex.RdTmp.tmp),
				           mkIRExpr_HWord((HWord)arg2->Iex.RdTmp.tmp),
				           mkIRExpr_HWord(counter)
				           )
			     );
      setHelperAnns(d2);
      addStmtToIRSB(sb,IRStmt_Dirty(d2));
    }

  if ((arg1->tag == Iex_RdTmp && arg2->tag == Iex_Const)
      || (arg1->tag == Iex_Const && arg2->tag == Iex_RdTmp))
    {
      if (arg1->tag == Iex_RdTmp) tmpname = (HWord)arg1->Iex.RdTmp.tmp;
      else if (arg2->tag == Iex_RdTmp) tmpname = (HWord)arg2->Iex.RdTmp.tmp;
      else vpanic("Neither arg1 nor arg2 is a tmp! \n");
      d2 = unsafeIRDirty_0_N(0, "EmitBinopTmpConstTypeHelper", 
			     &EmitBinopTmpConstTypeHelper,
			     mkIRExprVec_4(
			        	   mkIRExpr_HWord(lhs),
				           mkIRExpr_HWord(op),
				           mkIRExpr_HWord(tmpname),
				           mkIRExpr_HWord(counter)
				           )
			     );
      setHelperAnns(d2);
      addStmtToIRSB(sb,IRStmt_Dirty(d2));
    }

  return;
}

void EmitLoadTmp2AddrHelper(HWord lhs, HWord addr, HWord eip)
{
  if (getVarOf(locToHashKey(addr,0,MemLoc)) == -1)
    {
	setVarOf(locToHashKey(addr,0,MemLoc),curtyvar);
	curtyvar++;
    }
  setVarOf(locToHashKey(lhs,eip,TmpLoc),getVarOf(locToHashKey(addr,0,MemLoc)));
  return; 
}

void AddLoadHelper(IRSB* sb, IRTemp lhs, IRExpr* addr)
{
  IRDirty * d; 
  ULong lhs_int;
  lhs_int = (HWord)lhs;

  d = unsafeIRDirty_0_N(0, "EmitLoadTmp2AddrHelper", 
			&EmitRdTmpTmp2TmpHelper,
			mkIRExprVec_3(
				      mkIRExpr_HWord(lhs_int),
				      addr,
				      mkIRExpr_HWord(counter)
				      )
			);
  setHelperAnns(d);
  addStmtToIRSB(sb,IRStmt_Dirty(d)); 

  return;
}

/*
void EmitCCallHelper(HWord cond, HWord cc_op, HWord eip)
{
  
}

void AddCCallHelper(IRSB* sb, IRTemp lhs, IRExpr* call)
{
  IRDirty * d; 
  ULong lhs_int; 
  lhs_int = (HWord)lhs;
  IRCallee* callee = call->Iex.CCall.cee;
  IRExpr** callee_args = call->Iex.CCall.args;

  if(!VG_(strncmp)(callee->name,"x86g_calculate_condition",24))
    {
      d = unsafeIRDirty_0_N(0, "EmitCCallHelper", 
			&EmitCCallHelper,
			mkIRExprVec_3(
				      callee_args[0],
				      callee_args[1],
				      mkIRExpr_HWord(counter)
				      )
			);
      setHelperAnns(d);
      addStmtToIRSB(sb,IRStmt_Dirty(d)); 
    }

  return;
}
*/


void EmitPutConstHelper(HWord offset, HWord eip)
{
   clearVarOf(locToHashKey(offset,0,RegLoc));
   return;
}

void EmitPutTmpHelper(HWord offset, HWord tmpname, HWord eip)
{
  /* Check whether tmpname has a tyvar */ 
   if (getVarOf(locToHashKey(tmpname,eip,TmpLoc)) == -1)
    {
      /* No, so add one */ 
      setVarOf(locToHashKey(tmpname,eip,TmpLoc),curtyvar);
      curtyvar++;
    }

   setVarOf( locToHashKey(offset,0,RegLoc),
	     getVarOf(locToHashKey(tmpname,eip,TmpLoc))); 

   return;
}

void AddPutHelper(IRSB* sb, Int offset, IRExpr* data)
{
   IRDirty * d;  
   HWord h_offset = (HWord)(offset);  
   HWord lhs_name;  

   switch (data->tag)
    {
      case(Iex_Const):
     
         d = unsafeIRDirty_0_N(0, "EmitPutConstHelper",
			       &EmitPutConstHelper,
			       mkIRExprVec_2(mkIRExpr_HWord(h_offset),
					     mkIRExpr_HWord(counter)
					     )
			       );
         setHelperAnns(d);
         addStmtToIRSB(sb, IRStmt_Dirty(d)); 
         break;
      
      case(Iex_RdTmp):
  
         lhs_name = (HWord)data->Iex.RdTmp.tmp; 

         d = unsafeIRDirty_0_N(0, "EmitPutTmpHelper",
			       &EmitPutTmpHelper,
			       mkIRExprVec_3(mkIRExpr_HWord(h_offset),
					     mkIRExpr_HWord(lhs_name),
					     mkIRExpr_HWord(counter)
					     )
			       );
         setHelperAnns(d);
         addStmtToIRSB(sb, IRStmt_Dirty(d)); 
         break;
      
      default: 
         break; 
    }

   return;    
}

void EmitStoreAddr2TmpHelper(HWord addr, HWord tmpname, HWord eip)
{
  /* Check whether tmp has a tyvar */ 
  if (getVarOf(locToHashKey(tmpname,eip,TmpLoc)) == -1)
    {
      /* No, so add one */ 
      setVarOf(locToHashKey(tmpname,eip,TmpLoc),curtyvar);
      curtyvar++;
    }

  setVarOf( locToHashKey(addr,0,MemLoc),
	    getVarOf(locToHashKey(tmpname,eip,TmpLoc))); 
  
  return; 
}

void EmitStoreAddr2ConstHelper(HWord addr)
{
  setVarOf(locToHashKey(addr,0,MemLoc),curtyvar);
  curtyvar++;
  return;
}

void AddStoreHelper(IRSB* sb, IRExpr* addr, IRExpr* data)
{
  IRDirty* d;
  HWord tmpname;

  switch (addr->tag)
    {
    case (Iex_RdTmp):
      switch (data->tag)
	{
	case (Iex_RdTmp):
	  tmpname = (HWord) data->Iex.RdTmp.tmp; 	

	  d = unsafeIRDirty_0_N(0,
			    "EmitStoreAddr2TmpHelper",
			    &EmitStoreAddr2TmpHelper,
			    mkIRExprVec_3(addr,
					  mkIRExpr_HWord(tmpname),
					  mkIRExpr_HWord(counter)
					  )
			    );
	  setHelperAnns(d);
	  addStmtToIRSB(sb, IRStmt_Dirty(d)); 
	  break; 
	case (Iex_Const):
	  /* add code to emit new tyvar for memory address */ 
	  d = unsafeIRDirty_0_N(0,
				"EmitStoreAddr2ConstHelper",
				&EmitStoreAddr2ConstHelper,
				mkIRExprVec_1(addr
					      )
				);
	  setHelperAnns(d);
	  addStmtToIRSB(sb,IRStmt_Dirty(d)); 
	  break;
        default:
	  /* Should not reach here. */
	  ppIRExpr(data); 
	  vpanic("Bad store address!\n"); 
	  break; 
	}
      break; 
    default:
      break; 
    }
  return;
} 

void traceIRExpr(IRSB* sb, IRStmt* st , Addr addr)
{
   IntTyStateHint hint;
   IRExpr* rhs = st->Ist.WrTmp.data;
   IRTemp lhs = st->Ist.WrTmp.tmp;

   switch (rhs->tag) {
      case Iex_Get:
	 AddGetHelper(sb,lhs,rhs->Iex.Get.offset,addr);
         break;
      case Iex_RdTmp:
	 AddRdTmpHelper(sb,lhs,rhs->Iex.RdTmp.tmp);
         break;
      case Iex_Const:
         break;
      case Iex_Unop: 
	 hint = opToHint(rhs->Iex.Unop.op);
	 if (hint != HopelessHint)
	   {
	      AddOpRhsTypeHelper(sb,rhs->Iex.Unop.arg,hint,addr);
           }
	 //AddUnopHelper(sb,st);

         break;
      case Iex_GetI:
         break;
      case Iex_Binop: 
	 hint = opToHint(rhs->Iex.Binop.op);
	 if (hint != HopelessHint)
	   {
	      AddOpRhsTypeHelper(sb,rhs->Iex.Binop.arg1,hint,addr);
	      AddOpRhsTypeHelper(sb,rhs->Iex.Binop.arg2,hint,addr);
	   }
	 AddBinopHelper(sb,st);
         break;
      case Iex_Triop: 
         break;
      case Iex_Qop: 
         break;
      case Iex_Mux0X:
         break;
      case Iex_Load: 
	 AddLoadHelper(sb,lhs,rhs->Iex.Load.addr);
         break;
      case Iex_CCall:
//	 AddCCallHelper(sb,lhs,rhs);
         break;
      default: 
         break;
   }   
}

void traceIRStmt (IRSB * sb,  IRStmt* st , ULong caMainBBCounter)
{
   //  unsigned long n_old_typestates_state_instance = 0; 
   IRExpr * rhs; 
   static Addr current_addr = 0;
   counter = caMainBBCounter; 
   if (!st) {
      vex_printf("IRStmt* which is NULL !!!");
      return;
   }
   switch (st->tag) {
      case Ist_NoOp:
	 break; /* Nothing to emit. */
      case Ist_IMark:
	 current_addr = st->Ist.IMark.addr;
//	 VG_(message)(Vg_UserMsg, "addr is %lx \n", current_addr);
//	 VG_(message)(Vg_UserMsg, "(void*)current_addr is %p\n ", (void*)current_addr);
//	 VG_(message)(Vg_UserMsg, "&malloc is %p\n ", &malloc);
/*
	 if((void*)current_addr == VG_(fnptr_to_fnentry)( &SC_malloc ) ||
	    (void*)current_addr == VG_(fnptr_to_fnentry)( &SC__builtin_new ) ||
	    (void*)current_addr == VG_(fnptr_to_fnentry)( &SC__builtin_vec_new )
	   )
	   {
		argNum = 1;
	   }
	 if((void*)current_addr == VG_(fnptr_to_fnentry)( &SC_calloc ) ||
	    (void*)current_addr == VG_(fnptr_to_fnentry)( &SC_memalign )
	   )
	   {
		argNum = 2;
	   }
	 if((void*)current_addr == VG_(fnptr_to_fnentry)( &SC_realloc ) )
	   {
		isRealloc = 1;
		argNum = 1;
	   }
*/
	 if(current_addr == 0x4c2b20d )
	   {
		argNum = 1;
	   }
	 if(current_addr == 0x4c294e0 )
	   {
		argNum = 2;
	   }
	 if(current_addr == 0x4c2b3b8 )
	   {
		isRealloc = 1;
		argNum = 1;
	   }

	 break; 
      case Ist_AbiHint:
	 break; /* Nothing to emit. */ 

      case Ist_Put:
	 /* TODO: update map of names -> typestate vars */ 
	 /* PUT(30) = t32 --> VarOf(reg.30) := VarOf(eipX.t32) */ 
	 /* PUT(30) = const --> VarOf(reg.30) := NONE */ 
	 rhs = st->Ist.Put.data;
	 AddPutHelper(sb, st->Ist.Put.offset,rhs);
         break;

      case Ist_PutI:
	 /* TODO: update map of names -> typestate vars */ 
         break;

      case Ist_WrTmp:
//	 rhs = st->Ist.Tmp.data; 
	 traceIRExpr(sb,st,current_addr); 

         break;
      case Ist_Store:
	 /* TODO: update map of names -> typestate vars */ 
	 AddStoreHelper(sb,st->Ist.Store.addr,st->Ist.Store.data); 
         break;
      case Ist_Dirty:
	 /* TODO: handle common dirty helpers here */ 
         break;
      case Ist_MBE: 
         break; /* Nothing to emit. */ 

      case Ist_Exit:
	 /* No need to emit constraints here; catch earlier at CMP or */
	 /* at x86g_calc_condition. Do nothing. */ 
         break;
      default: 
	 return; 
   }
}

/*--------------------------------------------------------------------*/
/*--- symble trace                                      symtrace.h ---*/
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

/* --- Types --- */ 

typedef enum {
  MemLoc,    /* Location is a memory address */ 
  RegLoc,    /* Location is a register name */ 
  TmpLoc,    /* Location is a temporary */
  ErrorLoc   /* Error condition */ 
} LocType;

typedef enum {
  HopelessHint,     /* Op neither implies signed nor unsigned */
  SignedHint,       /* Op implies arguments are signed */ 
  UnsignedHint,     /* Op implies arguments are unsigned */ 
  ErrorHint         /* Error condition */  
} 
IntTyStateHint;

typedef enum {
  Top,        /* Unknown type */ 
  SignedTy,  /* Signed type */
  UnsignedTy, /* Unsigned type */
  Bot,       /* Contradictory type */ 
  BogusTy,     /* Error condition */
} IntType; 

/* Hash node type for VarOf() map. First two fields match core's VgHashNode */
typedef
struct _OgVarOfHashNode {
  struct _OgVarOfHashNode * next; 
  HWord  loc; 
  HWord  varname; 
} 
OgVarOfHashNode;

/* Hash node type for TypeOf() map. First two fields match core's VgHashNode */
typedef
struct _OgTypeOfHashNode {
  struct _OgTypeOfHashNode * next; 
  HWord varname;
  IntType type;
} 
OgTypeOfHashNode; 

typedef
struct _OgAddrOfHashNode {
  struct _OgAddrOfHashNode * next; 
  HWord varname;
  Addr addr;
} 
OgAddrOfHashNode; 

/* --- Globals --- */ 

VgHashTable ogVarOf; /* Initialized in ca_main - pre_clo_init */
VgHashTable ogTypeOf; /* Also initialized in ca_main - pre_clo_init */
VgHashTable ogAddrOf; 

/* --- Prototypes --- */

void setHelperAnns ( IRDirty* di );
IntTyStateHint opToHint(IROp op);
//IntType BinopTypeStateFunc(HWord op, IntType arg1Type, IntType arg2Type);
int setAddrOf(HWord varname, Addr addr);
Addr getAddrOf(HWord varname);
int setVarOf(HWord loc, HWord varname);
HWord getVarOf(HWord loc);
int setTypeOf(HWord varname, IntType type);
IntType getTypeOf(HWord varname);
IntType setTypeOfToMeet(HWord varname, IntType rhstype);

HWord locToHashKey(HWord loc1, HWord loc2, LocType ltype);
int clearVarOf(HWord loc);

void EmitStackHelper(Addr stackAddr, SizeT len);

void EmitGetTmp2RegHelper(HWord lhs_tmp, HWord offset, HWord eip);
void AddGetHelper(IRSB* sb, IRTemp lhs, Int offset, Addr addr);
void EmitRdTmpTmp2TmpHelper(HWord lhs, HWord rhs, HWord eip);
void AddRdTmpHelper(IRSB* sb, IRTemp lhs, IRTemp rhs);
void EmitTmpHelper(HWord tmp, IntTyStateHint hint, HWord eip, Addr addr);
void AddOpRhsTypeHelper(IRSB* sb, IRExpr* arg, IntTyStateHint hint, Addr addr);
void EmitNewTmpTyvarHelper(HWord lhs, HWord eip);
IntType BinopTypeStateFunc(HWord op, IntType arg1Type, IntType arg2Type);
void EmitBinopTmpTmpTypeHelper(HWord lhs, HWord op, HWord arg1, HWord arg2, HWord eip);
void EmitBinopTmpConstTypeHelper(HWord lhs, HWord op, HWord tmp, HWord eip);
void AddBinopHelper(IRSB* sb,IRStmt* st);
void EmitLoadTmp2AddrHelper(HWord lhs, HWord addr, HWord eip);
void AddLoadHelper(IRSB* sb, IRTemp lhs, IRExpr* addr);
void EmitPutConstHelper(HWord offset, HWord eip);
void EmitPutTmpHelper(HWord offset, HWord tmpname, HWord eip);
void AddPutHelper(IRSB* sb, Int offset, IRExpr* data);
void EmitStoreAddr2TmpHelper(HWord addr, HWord tmpname, HWord eip);
void EmitStoreAddr2ConstHelper(HWord addr);
void AddStoreHelper(IRSB* sb, IRExpr* addr, IRExpr* data);
void traceIRExpr(IRSB* sb, IRStmt* st , Addr addr);
void traceIRStmt (IRSB * sb,  IRStmt* st , UInt caMainBBCounter);






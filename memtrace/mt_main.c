#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_machine.h"

// VG_REGPARM(N): pass N (up to 3) arguments in registers on x86 --
// more efficient than via stack. Ignored on other architectures.
static VG_REGPARM(2) void trace_load(Addr addr, SizeT size)
{
   VG_(printf)("load : %#lx, %ld\n", addr, size);
}

static VG_REGPARM(2) void trace_store(Addr addr, SizeT size)
{
   VG_(printf)("store : %#lx, %ld\n", addr, size);
}


static void add_call(IRSB* bb, IRExpr* dAddr, Int dSize,
		     Char* helperName, void* helperAddr)
{
   // Create argument vector with two IRExpr* arguments.
   IRExpr** argv = mkIRExprVec_2(dAddr, mkIRExpr_HWord(dSize));

   // Create call statement to function at "helperAddr".
   IRDirty* di = unsafeIRDirty_0_N( /*regparms*/2, helperName,
   				    VG_(fnptr_to_fnentry)(helperAddr), argv);
   addStmtToIRSB(bb, IRStmt_Dirty(di));
}

static void handle_load(IRSB* bb, IRExpr* dAddr, Int dSize) 
{
   add_call(bb, dAddr, dSize, "trace_load", trace_load);
}

static void handle_store(IRSB* bb, IRExpr* dAddr, Int dSize) 
{
   add_call(bb, dAddr, dSize, "trace_store", trace_store);
}

// Post-option-processing initialization function
static void mt_post_clo_init(void) { }

// Instrumentation function. "bbIn" is the code block.
// Others arguments are more obscure and often not needed -- see
// include/pub_tool_tooliface.h.
static IRSB* mt_instrument ( VgCallbackClosure* closure,
			     IRSB* bbIn,
			     VexGuestLayout* layout,
			     VexGuestExtents* vge,
			     IRType gWordTy, IRType hWordTy )
{
   // include/pub_tool_basics.h provides types such as "Int".
   Int i;

   // Setup bbOut:allocate, initialize non-statement parts: type
   // environment,block-ending jump's destination and kind.
   IRSB* bbOut;
   bbOut = deepCopyIRSBExceptStmts(bbIn);
  
   // Iterate through statements, copy to bbOut, instrumenting
   // loads and stores along the way.
   for (i = 0; i < bbIn->stmts_used; i++) {
   	IRStmt* st = bbIn->stmts[i];
	if (!st) continue;    		// Ignore null statements

	// <Instrument loads and stores here
	switch (st->tag) {
	  case Ist_Store: {
		// Pass to handle_store: bbOut, store address and store size.
		handle_store(bbOut, st->Ist.Store.addr,
			     sizeofIRType(typeOfIRExpr(bbIn->tyenv, st->Ist.Store.data)));
		break;
	  }
	  case Ist_WrTmp: { // A "Tmp" is an assignment to a temporary.
		// Expression trees are flattened here, so "Tmp" is the only
		// kind of statement a load may appear within.
		IRExpr* data = st->Ist.WrTmp.data; // Expr on RHS of assignment
		if (data->tag == Iex_Load) {
			// Is it a load expression?
			// Pass handle_load bbOut plus the load address and size.
		handle_load(bbOut, data->Iex.Load.addr,
			    sizeofIRType(data->Iex.Load.ty)); // Get load size from
	  	}					    // type environment
	 	break;
	  }
	  case Ist_Dirty: {
		IRDirty* d = st->Ist.Dirty.details;
		if (d->mFx == Ifx_Read || d->mFx == Ifx_Modify)
			handle_load(bbOut, d->mAddr, d->mSize);
		if (d->mFx == Ifx_Write || d->mFx == Ifx_Modify)
			handle_store(bbOut, d->mAddr, d->mSize);
	  	break;
	  }
	  default: break;

	}

	addStmtToIRSB(bbOut, st);
   }
   return bbOut;
   
   //return bbIn;
}

// Finalization function
static void mt_fini(Int exitcode) { }

static void mt_pre_clo_init(void)
{
   // Required details for start-up message
   VG_(details_name)		("Memtrace");
   VG_(details_version)		("0.1");
   VG_(details_description)	("a memory tracer");
   VG_(details_copyright_author)("Copyright (C) 2006, J. Random Hacker.");
   // Required detail for crash message
   VG_(details_bug_reports_to)  ("/dev/null");
   // Name the required functions #2, #3 and #4.
   VG_(basic_tool_funcs)(mt_post_clo_init,
			 mt_instrument,
			 mt_fini);
}

// This prevents core/tool interface problems, and names the required
// function #1, giving the core an entry point into the tool.
VG_DETERMINE_INTERFACE_VERSION(mt_pre_clo_init)






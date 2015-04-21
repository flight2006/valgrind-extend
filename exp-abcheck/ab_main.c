#include "pub_tool_basics.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_execontext.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_options.h"

//////////////////////////////////////////////////////////////
//                                                          //
// main                                                     //
//                                                          //
//////////////////////////////////////////////////////////////

static void pc_pre_clo_init(void)
{
#if defined(VGO_darwin)
   // This makes the (all-failing) regtests run much faster.
   VG_(printf)("ABCheck doesn't work on Darwin yet, sorry.\n");
   VG_(exit)(1);
#endif
#if defined(VGA_s390x)
   /* fixs390: to be done. */
   VG_(printf)("ABCheck doesn't work s390x yet, sorry.\n");
   VG_(exit)(1);
#endif

   // Can't change the name until we change the names in suppressions
   // too.
   VG_(details_name)            ("exp-abcheck");
   VG_(details_version)         (NULL);
   VG_(details_description)     ("a stack and global array "
                                 "overrun detector");
   VG_(details_copyright_author)(
      "Copyright (C) 2003-2012, and GNU GPL'd, by OpenWorks Ltd et al.");
   VG_(details_bug_reports_to)  (VG_BUGS_TO);
   VG_(details_avg_translation_sizeB) ( 496 );

   VG_(basic_tool_funcs)        (sg_post_clo_init,
                                 h_instrument,
                                 sg_fini);

   VG_(needs_malloc_replacement)( h_replace_malloc,
                                  h_replace___builtin_new,
                                  h_replace___builtin_vec_new,
                                  h_replace_memalign,
                                  h_replace_calloc,
                                  h_replace_free,
                                  h_replace___builtin_delete,
                                  h_replace___builtin_vec_delete,
                                  h_replace_realloc,
                                  h_replace_malloc_usable_size,
                                  0 /* no need for client heap redzones */ );

   VG_(needs_var_info)          ();

   VG_(needs_core_errors)       ();
   VG_(needs_tool_errors)       (pc_eq_Error,
                                 pc_before_pp_Error,
                                 pc_pp_Error,
                                 True,/*show TIDs for errors*/
                                 pc_update_Error_extra,
                                 pc_is_recognised_suppression,
                                 pc_read_extra_suppression_info,
                                 pc_error_matches_suppression,
                                 pc_get_error_name,
                                 pc_get_extra_suppression_info);

   VG_(needs_xml_output)        ();

   //VG_(needs_syscall_wrapper)( h_pre_syscall,
   //                            h_post_syscall );

   VG_(needs_command_line_options)( pc_process_cmd_line_options,
                                    pc_print_usage,
                                    pc_print_debug_usage );

   VG_(track_die_mem_stack)        ( sg_die_mem_stack );
   VG_(track_pre_thread_ll_create) ( sg_pre_thread_ll_create );
   VG_(track_pre_thread_first_insn)( sg_pre_thread_first_insn );

   VG_(track_new_mem_mmap)         ( sg_new_mem_mmap );
   VG_(track_new_mem_startup)      ( sg_new_mem_startup);
   VG_(track_die_mem_munmap)       ( sg_die_mem_munmap );

   /* Really we ought to give handlers for these, to
      check that syscalls don't read across array boundaries. */
   /*
   VG_(track_pre_mem_read)         ( NULL );
   VG_(track_pre_mem_read_asciiz)  ( NULL );
   VG_(track_pre_mem_write)        ( NULL );
   */

   sg_pre_clo_init();

   VG_(clo_vex_control).iropt_unroll_thresh = 0;
   VG_(clo_vex_control).guest_chase_thresh = 0;
}

VG_DETERMINE_INTERFACE_VERSION(pc_pre_clo_init)


/*--------------------------------------------------------------------*/
/*--- end                                                pc_main.c ---*/
/*--------------------------------------------------------------------*/

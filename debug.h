/* debug.h
 * Debug printout routines 
 *
 * Lo Wing Hang
 * Provides debug printout and printout management using trace function groups
 *
 * Changelog:
 * 27-9-05:
 * modified the macro for trace* when DEBUG is not defined
 * removed function groups setting
 * changed EXIT macro
 * 
 * ----------------------------------------------------------------------------
 * Functions:
 * trace(arg...)
 * ftrace(arg...)
 * EXIT(msg)
 * 
 * Options:
 * DEBUG_SHOW_TRACE
 * DEBUG_USE_LOG_FILE (note: avoid, very slow)
 * DEBUG_FILE -- name of the log
 * 
 */

#ifndef __DEBUG_H_INCLUDE
#define __DEBUG_H_INCLUDE

// user modifiable setting ----------------------
// print all debug message to a trace log
//#define DEBUG_USE_LOG_FILE
// name of the trace log
#define DEBUG_FILE "debug.log"

#include <stdio.h>

// tracef function which prints the file, function and line number along with
// the message
#define __trace(args...) (printf("%s[%d]: %s: ", __FILE__, __LINE__, __FUNCTION__), printf(args))
#define __ftrace(file, args...) { FILE *debug_file = fopen(DEBUG_FILE, "a"); fprintf(debug_file, "%s[%d]: %s: ", __FILE__,  __LINE__, __FUNCTION__); fprintf(file, args); fclose(debug_file);}

//-------------------------
#ifdef DEBUG_SHOW_TRACE

#ifdef DEBUG_USE_LOG_FILE
#define trace(args...) __ftrace(debug_file, args)
#define ftrace(args...) __ftrace(debug_file, args)
#else
#define trace(args...) __trace(args)
#define ftrace(args...) __ftrace(debug_file, args)
#endif

#define EXIT(msg) (trace("%s\n", (msg)), abort())

#define ASSERT(expr, args...) ((expr) ? 1 : (__trace((args)), abort(), 1))

#else
#define trace(args...)
#define ftrace(args...)
#define EXIT(msg) (trace("%s\n", (msg)), exit(0))
#define ASSERT(expr, args...)

#endif
//-------------------------
#else

#ifndef DEBUG_SHOW_TRACE

#undef trace
#undef ftrace
#undef EXIT
#undef ASSERT

#define trace(args...)
#define ftrace(args...)
#define EXIT(msg) (trace("%s\n", (msg)), exit(0))
#define ASSERT(expr, args...)

#endif

#endif

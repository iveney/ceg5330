/*
 * Revision Control Information
 *
 * $Source: /var/cvs/ig/util/ansi.h,v $
 * $Author: aderry $
 * $Revision: 1.1.1.1 $
 * $Date: 2005/10/07 11:31:57 $
 *
 */
#ifndef ANSI_H
#define ANSI_H

/*
 * ANSI Compiler Support
 *
 * David Harrison
 * University of California, Berkeley
 * 1988
 *
 * ANSI compatible compilers are supposed to define the preprocessor
 * directive __STDC__.  Based on this directive, this file defines
 * certain ANSI specific macros.
 *
 * ARGS:
 *   Used in function prototypes.  Example:
 *   extern int foo
 *     ARGS((char *, double));
 */

/* Function prototypes */
#if defined(__STDC__) || defined(__cplusplus)
#define ARGS(args)	args
#else
#define ARGS(args)	()
#endif

#if defined(__cplusplus)
#define NULLARGS	(void)
#else
#define NULLARGS	()
#endif

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

#if defined(__cplusplus) || defined(__STDC__) 
#define HAS_STDARG
#endif

#endif

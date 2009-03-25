/*
 * Revision Control Information
 *
 * $Source: /var/cvs/ig/util/prtime.c,v $
 * $Author: aderry $
 * $Revision: 1.1.1.1 $
 * $Date: 2005/10/07 11:31:57 $
 *
 */
/* LINTLIBRARY */

#include <stdio.h>
#include "util.h"


/*
 *  util_print_time -- massage a long which represents a time interval in
 *  milliseconds, into a string suitable for output 
 *
 *  Hack for IBM/PC -- avoids using floating point
 */

char *
util_print_time(t)
long t;
{
    static char s[40];

    (void) sprintf(s, "%ld.%02ld sec", t/1000, (t%1000)/10);
    return s;
}

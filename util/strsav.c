/*
 * Revision Control Information
 *
 * $Source: /var/cvs/ig/util/strsav.c,v $
 * $Author: aderry $
 * $Revision: 1.1.1.1 $
 * $Date: 2005/10/07 11:31:57 $
 *
 */
/* LINTLIBRARY */

#include <stdio.h>
#include "util.h"


/*
 *  util_strsav -- save a copy of a string
 */
char *
util_strsav(s)
char *s;
{
    return strcpy(ALLOC(char, strlen(s)+1), s);
}

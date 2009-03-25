/* $Id: datalimit.c,v 1.1.1.1 2005/10/07 11:31:57 aderry Exp $ */

#ifndef HAVE_SYS_RESOURCE_H
#define HAVE_SYS_RESOURCE_H 1
#endif
#ifndef HAVE_SYS_TIME_H
#define HAVE_SYS_TIME_H 1
#endif
#ifndef HAVE_GETRLIMIT
#define HAVE_GETRLIMIT 1
#endif

#if HAVE_SYS_RESOURCE_H == 1
#if HAVE_SYS_TIME_H == 1
#include <sys/time.h>
#endif
#include <sys/resource.h>
#endif

#ifndef RLIMIT_DATA_DEFAULT
#define RLIMIT_DATA_DEFAULT 16777216	/* assume 16MB by default */
#endif

long int
getSoftDataLimit()
{
#if HAVE_SYS_RESOURCE_H == 1 && HAVE_GETRLIMIT == 1 && defined(RLIMIT_DATA)
    struct rlimit rl;
    long int result;

    result = (long) getrlimit(RLIMIT_DATA, &rl);
    if (result != 0 || rl.rlim_cur == RLIM_INFINITY)
	return(RLIMIT_DATA_DEFAULT);
    else
	return(rl.rlim_cur);
#else
    return(RLIMIT_DATA_DEFAULT);
#endif

} /* end of getSoftDataLimit */

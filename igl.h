//====================================================
// igl.h - Internal header: DO NO USE IT DIRECTLY !!!
//====================================================

#ifndef __IGL_H
#define __IGL_H

//#include "igl_malloc.h"
// uncomment use the mpatrol to detect memory errors
//#include "mpatrol.h"

#define DEBUG
// array.h enabled:
// ARRAY_CHECK_BOUND
// ARRAY_INIT_ZERO




//====================================================
#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#define IGL_CONCAT(x,y) x##y
#define IGL_FORALL_VAR(y) IGL_CONCAT(IGL_FORALL_VAR,y)


#endif

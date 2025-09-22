/*
 * Copyright (C) 2015  University of Oregon
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Apache License, as specified in the LICENSE file.
 *
 * For more information, see the LICENSE file.
 */
/*
 */

#ifndef DEBUGH
#define DEBUGH


#define LOGIT 1
#define NOLOG 0

extern int acqerrno;	/* acquisition error number , like unix errno */
extern int DebugLevel;
void errLogRet(int prtOpt, char *fileNline, ...);
void complain();

#ifdef DEBUG
#define debugInfo (fileNline(__FILE__,__LINE__))

#define DPRINT(level, str) \
        if (DebugLevel > level) fprintf(stderr,str)

#define DPRINT1(level, str, arg1) \
        if (DebugLevel > level) fprintf(stderr,str,arg1)

#define DPRINT2(level, str, arg1, arg2) \
        if (DebugLevel > level) fprintf(stderr,str,arg1,arg2)
 
#define DPRINT3(level, str, arg1, arg2, arg3) \
        if (DebugLevel > level) fprintf(stderr,str,arg1,arg2,arg3)
 
#define DPRINT4(level, str, arg1, arg2, arg3, arg4) \
        if (DebugLevel > level) fprintf(stderr,str,arg1,arg2,arg3,arg4)
 
#define DPRINT5(level, str, arg1, arg2, arg3, arg4, arg5 ) \
        if (DebugLevel > level) fprintf(stderr,str,arg1,arg2,arg3,arg4,arg5)
 
#define DPRINT6(level, str, arg1, arg2, arg3, arg4, arg5, arg6 ) \
        if (DebugLevel > level) fprintf(stderr,str,arg1,arg2,arg3,arg4,arg5,arg6)
 
#define DPRINT7(level, str, arg1, arg2, arg3, arg4, arg5, arg6, arg7 ) \
   	if (DebugLevel > level)  \
	  fprintf(stderr,str,arg1,arg2,arg3,arg4,arg5,arg6,arg7)
 
#define DPRINT8(level, str, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 ) \
   	if (DebugLevel > level)  \
	  fprintf(stderr,str,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8)
 
#define DPRINT9(level, str, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9 ) \
        if (DebugLevel > level)  \
	  fprintf(stderr,str,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9)

#else

#define DPRINT(level, str)
#define DPRINT1(level, str, arg2)
#define DPRINT2(level, str, arg1, arg2)
#define DPRINT3(level, str, arg1, arg2, arg3)
#define DPRINT4(level, str, arg1, arg2, arg3, arg4)
#define DPRINT5(level, str, arg1, arg2, arg3, arg4, arg5)
#define DPRINT6(level, str, arg1, arg2, arg3, arg4, arg5, arg6 )
#define DPRINT7(level, str, arg1, arg2, arg3, arg4, arg5, arg6, arg7 )
#define DPRINT8(level, str, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 )
#define DPRINT9(level, str, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9 )

 
#define debugInfo NULL
 
#endif 

#endif

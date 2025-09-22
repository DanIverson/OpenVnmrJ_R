#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "debug.h"

void errLogRet(int prtOpt, char *fileNline, ...)
/* int prtOpt - Unix Version compatibility, No function */
/* char* fileNline - String containing file names & line number */
{
   va_list vargs;
   char *fmt;
   int len;
   char ErrMsgStr[1024];
 
   len = 0;
   if (fileNline != NULL)
   {
      sprintf(ErrMsgStr,"%s",fileNline);
      len = strlen(ErrMsgStr);
   }
 
   va_start(vargs, fileNline);
   fmt = va_arg(vargs, char *);
   vsprintf((ErrMsgStr + len), fmt, vargs);
   va_end(vargs);
 
   fprintf(stderr,"%s\n",ErrMsgStr);
       
   return;
}

void complain()
{
   errLogRet(LOGIT,debugInfo,"complain(): error\n");
}

/*
 * Copyright (C) 2015  University of Oregon
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Apache License, as specified in the LICENSE file.
 *
 * For more information, see the LICENSE file.
 */

/*   aptable.c   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "acqparms.h"
#include "ACode32.h"
#include "aptable.h"
#include "symtab.h"
#include "variables.h"
#include "group.h"
#include "vfilesys.h"
#include "pvars.h"
#include "abort.h"

#include "lc.h"

extern FILE *psgFile;
extern int ntIndex;
extern int res_4_internal;
extern void F_initval(double, int);

/*************************
*  Constant Definitions  *
*************************/

#define TBLERROR		-1
#define COMPLETE		0
#define INCOMPLETE		1
#define FALSE			0
#define TRUE			1

#define MAXELEMENTS		32768
#define MINDIVN			1
#define MAXDIVN			64
#define MINREP			1
#define MAXREP			64
#define MINTABLE		1
#define MINLOOPSIZE		1
#define MAXLOOPSIZE		64
#define MAXLOADTABLECALLS	64


#ifdef DEBUG
#define DPRINT0(str) \
        fprintf(stderr,str)
#define DPRINT1(str, arg1) \
        fprintf(stderr,str,arg1)
#define DPRINT2(str, arg1, arg2) \
        fprintf(stderr,str,arg1,arg2)
#define DPRINT3(str, arg1, arg2, arg3) \
        fprintf(stderr,str,arg1,arg2,arg3)
#define DPRINT4(str, arg1, arg2, arg3, arg4) \
        fprintf(stderr,str,arg1,arg2,arg3,arg4)
#define DPRINT7(str, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
        fprintf(stderr,str,arg1,arg2,arg3,arg4, arg5, arg6, arg7)
#else
#define DPRINT0(str)
#define DPRINT1(str, arg2)
#define DPRINT2(str, arg1, arg2)
#define DPRINT3(str, arg1, arg2, arg3)
#define DPRINT4(str, arg1, arg2, arg3, arg4)
#define DPRINT7(str, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#endif

/*******************
*  In-Line Macros  *
*******************/

#define isTable(name_of_table)					\
   ((name_of_table >= t1) && (name_of_table <= t60))

#define isValidNumber(number_of_table)				\
	((number_of_table >= 1) &&				\
	(number_of_table <= last_table))

#define isValidDest(arg1)					\
	(((arg1 >= v1) && (arg1 <= v42)) || (arg1 == oph) || (arg1 == tablert) ||	\
		(arg1 == vslice_ctr) || (arg1 == vslices) || (arg1 == virblock) ||	\
		(arg1 == vnirpulses) || (arg1 == vir) || (arg1 == virslice_ctr) ||	\
		(arg1 == vnir) || (arg1 == vnir_ctr) || (arg1 == vtest) ||              \
                (arg1 == vtabskip) || (arg1 == vtabskip2) )		

#define isValidIndex(arg1)					\
	(((arg1 >= v1) && (arg1 <= v42)) || (arg1 == oph) ||	\
   		(arg1 == ct) || (arg1 == bsctr) ||		\
		(arg1 == ssctr) || (arg1 == ctss) ||	\
		(arg1 == vslice_ctr) || (arg1 == vslices) || (arg1 == virblock) ||	\
		(arg1 == vnirpulses) || (arg1 == vir) || (arg1 == virslice_ctr) ||	\
		(arg1 == vnir) || (arg1 == vnir_ctr) || (arg1 == vtest) ||              \
                (arg1 == vtabskip) || (arg1 == vtabskip2) )


/*********************************
*  Global Variable Declarations  *
*********************************/

static int	parsetable,	/* table parsing flag */
		c,		/* variable for file parsing */
		debug = FALSE;	/* debug flag */

struct	_tablelim
{
   char previnfilename[MAXSTR];
   int	start;
   int	end;
};

typedef struct _tablelim	tablelim;
static tablelim			tablelimits[MAXLOADTABLECALLS];


/***********************************
*  External Variable Declarations  *
***********************************/

extern int	bgflag;

extern Acqparams *Alc;
extern int putFile(const char *fname);
extern double getval(const char *name);
		      
#define TABLEHEADER     0x40000000
extern void broadcastCodes(int, int, int*);

#define SET_RTPHASE90   42
#define SET_PHASE90     44

#define TBLERROR		-1
#define NONE   -1
#define PHASE1 11
#define PHASE2 12
#define PHASE3 13
#define CYCLE  21
#define CYCLE2 22
#define FAD    31

static int  cy0123[] = {0, 1, 2, 3 };
static int  cy0213[] = {0, 2, 1, 3 };

/* forward declarations */
static void checkforcomments(FILE *tfile);
static void writetable(Tableinfo *tblinfo);
static int reparse_release(int startindex,int endindex);
static int read_number(FILE *tablefile, int eofflag, int *dest);
static int check_bounds(int paramval, int maxvalue, int minvalue);
static void load_element(int outcount, int incount, int table_index, int **elempntrpntr,
		    int *values, int incntflag);
static FILE *open_table(const char *basename, const char *concatname,
                        const char *inputname, const char *permission);
static int store_in_table(int table_index, int *elementpntr);
static void writedebug(FILE *fd, int ntables);
static int checktable(Tableinfo *tblinfo, int tblname, int namechk, int numberchk,
		      int sizechk, int destchk, int indxchk, int writechk);
static int table_math(int optype, int *value1, int *value2);
void APsetreceiver(int tablename);
int tablertv(int tablename);
int tablesv(int tablename);



static void add_fad(int *main_phase,int len, const char *parname)
/* int main_phase[];
int  len;
char *parname; */
{
   int i;
   int phase = (int) getval(parname);

   DPRINT2("adding fad for %s = %d value\n",parname,phase);
   if ((phase == 1) || (phase == 2))
   {
      int idn = Alc->id2;

      DPRINT2("fad value for increment %d is %s\n",idn,
                      (idn % 2) ? "two" : "zero");
      if (idn % 2)
         for (i=0; i < len; i++)
            main_phase[i] += 2;
   }
}

static void add_phase(int *main_phase, int len, const char *parname)
/*int main_phase[];
int  len;
char *parname;*/
{
   int i;
   int phase = (int) getval(parname);

   DPRINT2("adding %s = %d value\n",parname,phase);
   if (phase == 2)
   {
      DPRINT0("increment value is one\n");
      for (i=0; i < len; i++)
         main_phase[i] += 1;
   }
   else if (phase == 3)
   {
      int idn = 0;

      if (strcmp(parname,"phase") == 0)
         idn = Alc->id2;
      else if (strcmp(parname,"phase2") == 0)
         idn = Alc->id3;
      else if (strcmp(parname,"phase3") == 0)
         idn = Alc->id4;
      DPRINT1("increment value is %d\n",idn);
      for (i=0; i < len; i++)
         main_phase[i] += idn;
   }
}

static void add_cycle(int main_phase[], int *len, int cycle[])
{
   int i,j;

   for (i=0; i < 4; i++)
      for (j=0; j< *len; j++)
      {
         main_phase[*len * i + j] = main_phase[j] + cycle[i];
         DPRINT7("i= %d j= %d ph[%d](%d) = ph[%d](%d) + %d\n",i,j, *len * i + j,
              main_phase[*len * i + j], j, main_phase[j], cycle[i]);
      }
   *len *= 4;
   DPRINT1("after cycle main phase elements is %d\n",*len);
}

void make_table(int main_phase[], int *phase_len, int mod_list[], int mod_count)
{
   int i=0;
   int j;

#ifdef DEBUG
   DPRINT1("main phase elements is %d\n",*phase_len);
   for (j=0; j< *phase_len; j++)
      DPRINT2("main phase element %d is %d\n",j+1, main_phase[j]);
   DPRINT1("%d modifiers supplied \n",mod_count);
#endif
   while (i < mod_count)
   {
      j = mod_list[i];
      switch (j)
      {
         case PHASE1:
                  DPRINT2("modifier %d is %s \n",i+1, "PHASE1");
                  add_phase(main_phase, *phase_len,"phase");
                  break;
         case PHASE2:
                  DPRINT2("modifier %d is %s \n",i+1, "PHASE2");
                  add_phase(main_phase, *phase_len,"phase2");
                  break;
         case PHASE3:
                  DPRINT2("modifier %d is %s \n",i+1, "PHASE3");
                  add_phase(main_phase, *phase_len,"phase3");
                  break;
         case CYCLE:
                  DPRINT2("modifier %d is %s \n",i+1, "CYCLE");
                  add_cycle(main_phase, phase_len,cy0123);
                  break;
         case CYCLE2:
                  DPRINT2("modifier %d is %s \n",i+1, "CYCLE2");
                  add_cycle(main_phase, phase_len,cy0213);
                  break;
         case FAD:
                  DPRINT2("modifier %d is %s \n",i+1, "FAD");
                  add_fad(main_phase, *phase_len,"phase");
                  break;
         default:
                  DPRINT3("modifier %d is %s (val = %d) \n",i+1, "NONE",j);
                  break;
      }
      i++;
   }
   for (j=0; j< *phase_len; j++)
   {
      DPRINT3("phase element[%d] is %d mod 4 = %d\n",j+1,
                      main_phase[j], main_phase[j] % 4);
      main_phase[j] %= 4;
   }
}

int modtype(char *modifier)
{
   if ( (strcmp("phase",modifier) == 0) ||
        (strcmp("phase1",modifier) == 0) )
      return(PHASE1);
   else if (strcmp("phase2",modifier) == 0)
      return(PHASE2);
   else if (strcmp("phase3",modifier) == 0)
      return(PHASE3);
   else if (strcmp("cycle",modifier) == 0)
      return(CYCLE);
   else if (strcmp("cycle2",modifier) == 0)
      return(CYCLE2);
   else if ( (strcmp("fad",modifier) == 0) ||
             (strcmp("fad1",modifier) == 0) )
      return(FAD);
   else
      return(NONE);
}
int get_mod(char arg[], int *index, int *modifier)
{
    int i = *index;
    int j = 0;
    int c;
    char mod[1024];

    while (((c = arg[i]) != '\0') && ((c == ' ') || (c == '\t') || (c == '+')))
       i++;   /* skip white space */
    while (((c = arg[i]) != '\0') && (c != ' ') && (c != '\t') && (c != '+') )
       mod[j++] = arg[i++];
    mod[j] = '\0';
    *index = i;
    DPRINT1("check modifier %s is ",mod);
    *modifier = modtype(mod);
    DPRINT1("%s\n",(*modifier == NONE) ? "not ok" : "ok");
    return(*modifier );
}
int
get_main_phase(char arg[], int *index, int val[])
{
   int i=0;
   int j=0;
   int length = strlen(arg);

   while ( (i < length) && (arg[i] != '\0') &&
           (arg[i] != '+') )
   {
      if ((arg[i] == '0') || (arg[i] == '1') ||
          (arg[i] == '2') || (arg[i] == '3') )
         val[j++] = arg[i] - '0';
      i++;
   }
   *index = i;
   DPRINT1("main phase elements is %d\n",j);
   return(j);
}

int phase_var_type(int val)
{
   return((t1 <= val && val <= t60) ? SET_RTPHASE90 : SET_PHASE90);
}

void setreceiver(int phaseptr)
{
   if (phase_var_type(phaseptr) == SET_RTPHASE90)
      APsetreceiver(phaseptr);
   else
   {
      text_error("Syntax Error:  invalid table name in setreceiver( ) psg command\n");
      psg_abort(1);
   }
}



/*---------------------------------------------------
|                                                   |
|                  loadtable()/1                    |
|                                                   |
|  This PSG function parses the UNIX table file     |
|  and initializes the appropriate structure ele-   |
|  ments for each loaded table.  The table values   |
|  are not loaded into the ACODE array until after  |
|  the pulsesequence function has been exited.      |
|                                                   |
|  The UNIX table file is always parsed on the      |
|  first increment and on subsequent increments if  |
|  and only if the name of the table file has       |
|  changed between increments.  If the parameter    |
|  "debug" is set to 1, an output file containing   |
|  the result of the parsing operation is produced. |
|  This file may be useful if one suspects that     |
|  table file was not parsed correctly.             |
|                                                   |
---------------------------------------------------*/
int loadtable(char *infilename)
{
   int		tnum = 0,		/* initialized to zero */
		repfactor = 0,
		divnfactor = 0,
		i,
		j,
		index,
		numeleminloop = 0,
		activeopen_flag,
		tflag = FALSE,
		equalflag = FALSE,
		parenthflag = FALSE,
		bracketflag = FALSE,
		divnrtrnflag = FALSE,
		numtables = last_table,
 		table_value,
		loopelem[MAXLOOPSIZE + 1],
		*startelempntr,
		*elempntr;
   int		result;
   double	tmp;
   FILE		*tblfile,
 		*outfile;

/*********************************************
*  Check for VNMR parameter "tbldbug".  If   *
*  "tbldbug" exists and is set to 'y', then  *
*  provide output.                           *
*********************************************/

   if (bgflag)
      fprintf(stderr, "loadtable():  parse AP table file\n");

   if (P_getreal(CURRENT, "debug", &tmp, 1) == 0)
      debug = (int) (tmp + 0.5);

/***************************
*  Check input file name.  *
***************************/

   if (strcmp(infilename, "") == 0)
      return(INCOMPLETE);

/*************************************************
*  Check to see if there is a new file for this  *
*  function call to LOADTABLE and/or if more     *
*  tables have been added prior to the ones to   *
*  be added in this call to LOADTABLE.           *
*************************************************/

   if (++loadtablecall > MAXLOADTABLECALLS)
   {
      text_error("Maximum number of calls to LOADTABLE exceeded\n");
      psg_abort(1);
   }

   if (strcmp(infilename, tablelimits[loadtablecall].previnfilename) == 0)
   {
      parsetable = FALSE;
   }
   else
   {
      parsetable = TRUE;
      strcpy(tablelimits[loadtablecall].previnfilename, infilename);
   }

/**********************************************************
*  Expand or compress the table_order array depending on  *
*  the number of tables created prior to this function    *
*  call to LOADTABLE.  Note that this number can change   *
*  as a function of the PSG increment.                    *
**********************************************************/

   index = tablelimits[loadtablecall].start;
   if (loadtablecall > 0)
      tablelimits[loadtablecall].start = tablelimits[loadtablecall - 1].end;

   if (index == last_table)
   {
      if (!parsetable)
         last_table = tablelimits[loadtablecall].end;
   }
   else
   {
      if (index < last_table)
      {
         for (i = last_table; i < MAXTABLE;)
            table_order[i++] = tmptable_order[index++];
      }
      else
      {
         j = last_table;
         for (i = index; i < MAXTABLE;)
            table_order[j++] = tmptable_order[i++];
      }
   }

/*********************************************************
*  Release all table pointers created from the nth call  *
*  to LOADTABLE for the previous PSG increment if one    *
*  is either re-parsing the same table file or parsing   *
*  a new table file.                                     *
*********************************************************/

   if ((ix > 1) && (parsetable))
   {
      if (reparse_release(tablelimits[loadtablecall].start,
    	  tablelimits[loadtablecall].end) == TBLERROR)
      {
         text_error("Unable to release Table pointers for re-allocation\n");
         psg_abort(1);
      }
   }

/************************************************************
*  Open table file.  Look in the user directory first and   *
*  then in the system directory.                            *
************************************************************/

   if (parsetable)
   {
      char path[MAXPATHL*2];

      tblfile = NULL;
      if (appdirFind(infilename,"tablib",path,"",R_OK) )
         tblfile = fopen(path,"r");
      if (tblfile == 0)
      {
         abort_message( "Unable to open input table file %s\n", infilename);
      }
      putFile(path);
   }

/********************
*  Read table file  *
********************/
 
   if (parsetable)
   {
      elempntr = (int *) (malloc((MAXELEMENTS + 1) *
      		sizeof(*elempntr)));
      if (elempntr == NULL)
      {
         text_error("Unable to allocate table scratch memory\n");
         psg_abort(1);
      }

      startelempntr = elempntr;
      while ((c = getc(tblfile)) != EOF)
      {
         checkforcomments(tblfile);
         if (numtables > MAXTABLE)
         {
            text_error("Maximum number of tables has been exceeded\n");
            psg_abort(1);
         }

         if ((c != '\t') && (c != ' ') && (c != '\n') && (c != EOF))
         {

/********************************
*  Check for table identifier.  *
********************************/

            if ((c == 't') || (c == 'T'))
            {
               if (tflag || parenthflag || bracketflag || divnrtrnflag)
               {
                  text_error("Syntax Error:  misplaced table name\n");
                  psg_abort(1);
               }

               numtables++;
               tflag = TRUE;
               equalflag = FALSE;
            }

/******************************************************
*  Check for an open curly bracket indicating a DIVN  *
*  return attribute with the table.                   *
******************************************************/

            else if (c == '{')
            {
               if (tflag || (!equalflag) || divnrtrnflag)
               {
                  text_error("Syntax Error: misplaced { symbol\n");
                  psg_abort(1);
               }

               divnrtrnflag = TRUE;
            }

/***********************************************************
*  Check for a close curly bracket indicating termination  *
*  of a DIVN return attribute for the table.  The N value  *
*  for this attribute must follow immediately after the    *
*  close curly bracket.                                    *
***********************************************************/

            else if (c == '}')
            {
               c = getc(tblfile);
               checkforcomments(tblfile);
	       result = read_number(tblfile, FALSE, &divnfactor);
               if ((result == TBLERROR) || 
		(check_bounds(divnfactor, MAXDIVN, MINDIVN) == TBLERROR))
               {
                  text_error("Syntax Error:  invalid DIVN factor\n");
                  psg_abort(1);
               }

               if (Table[tnum - 1]->divn_factor > 1)
               {
                  text_error("Attribute Error:  divn-return multiply defined\n");
                  psg_abort(1);
               }
               else
               {
                  Table[tnum - 1]->divn_factor = divnfactor;
               }

               divnrtrnflag = FALSE;
            }

/***********************************************************
*  Check for square brackets indicating a replication of   *
*  each element sequentially in the preceding table loop.  * 
***********************************************************/

            else if (c == '[')
            {
               if (tflag || parenthflag || bracketflag || (!equalflag))
               {
                  text_error("Syntax Error:  misplaced [ symbol\n");
                  psg_abort(1);
               }

               bracketflag = TRUE;
               repfactor = 1;
               numeleminloop = 0;
            }

/************************************************************
*  Check for a close square bracket indicating termination  *
*  of a replication attribute for the table.  The N value   *
*  for this attribute must follow immediately after the     *
*  close square bracket.                                    *
************************************************************/

            else if (c == ']')
            {
               c = getc(tblfile);
               checkforcomments(tblfile);
               result = read_number(tblfile, FALSE, &repfactor);

               if ((result == TBLERROR) ||
		(check_bounds(repfactor, MAXREP, MINREP) == TBLERROR))
               {
                  text_error("Syntax Error:  invalid REP factor\n");
                  psg_abort(1);
               }
               else if (check_bounds(numeleminloop, MAXLOOPSIZE, MINLOOPSIZE)
          		== TBLERROR)
               {
                  text_error("Invalid number of loops for [] table loop\n");
                  psg_abort(1);
               }

               load_element(numeleminloop, repfactor, tnum-1, &elempntr,
		     loopelem, FALSE);
               bracketflag = FALSE;
            }
          
/****************************************************
*  Check for parentheses indicating replication of  *
*  the immediately preceding table loop.            *
****************************************************/

            else if (c == '(')
            {
               if (bracketflag || parenthflag || tflag || (!equalflag))
               {
                  text_error("Syntax Error:  misplaced ( symbol\n");
                  psg_abort(1);
               }

               parenthflag = TRUE;
               repfactor = 1;
               numeleminloop = 0;
            }

/***********************************************************
*  Check for a close parentheses indicating termination    *
*  of a replication attribute for the table.  The N value  *
*  for this attribute must follow immediately after the    *
*  close parentheses.                                      *
***********************************************************/

            else if (c == ')')
            {
               c = getc(tblfile);
               checkforcomments(tblfile);
               result = read_number(tblfile, FALSE, &repfactor);

               if ((result == TBLERROR) ||
		(check_bounds(repfactor, MAXREP, MINREP) == TBLERROR))
               {
                  text_error("Syntax Error:  invalid REP factor\n");
                  psg_abort(1);
               }
               else if (check_bounds(numeleminloop, MAXLOOPSIZE, MINLOOPSIZE)
          		== TBLERROR)
               {
                  text_error("Invalid number of loops for () table loop\n");
                  psg_abort(1);
               }

               load_element(repfactor, numeleminloop, tnum-1, &elempntr,
		     loopelem, TRUE);
               parenthflag = FALSE;
            }

/*********************************************
*  Check for AUTOINCREMENT table attribute.  *
*********************************************/

            else if (c == '+')
            {
               if (tflag || parenthflag || bracketflag || divnrtrnflag
                     || equalflag)
               {
                  text_error("Syntax Error:  invalid auto-increment notation\n");
                  psg_abort(1);
               }
               else
               {
                  Table[tnum - 1]->auto_inc = TRUE;
               }
            }

/**************************
*  Check for table name.  *
**************************/

            else if (tflag)
            {

/****************************************************************
*  if tnum is not equal to zero, i.e., we are not parsing the   *
*  first table, the actual table pointer is initialized to      *
*  just the right of memory.  Then, the table elements stored   *
*  in the master pointer "elempntr" are copied into the memory  *
*  pointed to by the new table pointer.  Prior to this PIO,     *
*  however, "elempntr" must be set back to its initial value.   *
*  It must also be reinitialized after the PIO operation.       *
****************************************************************/

	       if (tnum > 0)
               {
                  elempntr = startelempntr;
                  if (store_in_table(tnum-1, elempntr) == TBLERROR)
                  {
                     text_error("Unable to store elements in table\n");
                     psg_abort(1);
                  }
               }

               result = read_number(tblfile, FALSE, &tnum);
               if ((result == TBLERROR) ||
		(check_bounds(tnum, MAXTABLE, MINTABLE) == TBLERROR))
               {
                  text_error("Syntax Error:  invalid table name\n");
                  psg_abort(1);
               }

               if (Table[tnum - 1]->table_size > 0)
               {
                  text_error("Table name used more than once\n");
                  psg_abort(1);
               }

               Table[tnum - 1]->table_number = numtables;

               Table[tnum - 1]->acodeloc     = tnum - 1;

               table_order[numtables - 1] = tnum - 1;
               tflag = FALSE;
            }

/*******************************************
*  Check for delimiter between table name  *
*  and table array.                        *
*******************************************/

            else if (c == '=')
            {
               if (parenthflag || equalflag || bracketflag || divnrtrnflag)
               {
                  text_error("Syntax Error:  misplacement of = symbol\n");
                  psg_abort(1);
               }

               equalflag = TRUE;
               tflag = FALSE;
            }

/***************************
*  Read in table elemens.  *
***************************/

            else if (equalflag)
            {
               checkforcomments(tblfile);
               activeopen_flag = (parenthflag || bracketflag);
               result = read_number(tblfile, activeopen_flag,
					&table_value);

               if (result == TBLERROR)
               {
                  text_error("Syntax Error:  invalid table loop element\n");
                  psg_abort(1);
               }

/***********************************************
*  If table loop is active, store table value  *
*  for replication.                            *
***********************************************/

               if (activeopen_flag)
               {
                  loopelem[numeleminloop++] = table_value;
                  if (numeleminloop > MAXLOOPSIZE)
                  {
                     text_error("Maximum number of elements within table loop exceeded\n");
                     psg_abort(1);
                  }
               }

/****************************************************
*  Table looping is not active; load table element  *
*  into memory.                                     *
****************************************************/

               else
               {
                  *elempntr++ = table_value;
                  Table[tnum - 1]->table_size++;
               }
            }
         }
      }
 
/***************************
*  Close table input file  *
***************************/

      fclose(tblfile);

/***********************************************************
*  Copy elements of last parsed table from master pointer  *
*  "elempntr" to the new table pointer by PIO.             *
***********************************************************/

      if (tnum)
      {
         elempntr = startelempntr;
         if (store_in_table(tnum-1, elempntr) == TBLERROR)
         {
            text_error("Unable to store elements in table memory\n");
            psg_abort(1);
         }
      }

/*******************************************
*  Close "parsetable" logical expression.  *
*******************************************/

      tablelimits[loadtablecall].end = numtables;
      free(startelempntr);
      startelempntr = NULL;
      elempntr = NULL;
   }

/************************************************
*  Outputs copy of parsed table to the current  *
*  experiment directory.                        *
************************************************/

   if ((ix == 1) && (debug))
   {
      outfile = open_table(curexp, "/table", ".out", "w");
      if (outfile == 0)
      {
         text_error("Unable to open table output file\n");
         psg_abort(1);
      }

      writedebug(outfile, numtables);
      fclose(outfile);
   }

   if (parsetable)
      last_table = numtables;

   return(COMPLETE);
}
/*---------------------------------------------------
|                                                   |
|               checkforcomments()/1                |
|                                                   |
|  This PSG function checks for comments in the     |
|  UNIX phase-table file.                           |
|                                                   |
|         tfile:  name of UNIX table file           |
|                                                   |
+--------------------------------------------------*/
static void checkforcomments(FILE *tfile)
{
   int	commentflag;

   if (c == '/')
   {
      c = getc(tfile);
      if (c != '*')
      {
         text_error("Syntax Error:  illegal delimiting of comments\n");
         psg_abort(1);
      }
      else
      {
         commentflag = TRUE;
         while (((c = getc(tfile)) != EOF) && (commentflag))
         {
            if (c == '*')
            {
               c = getc(tfile);
               if (c == EOF)
               {
                  text_error("Syntax Error:  improper EOF in comment construction\n");
                  psg_abort(1);
               }
               else if (c == '/')
               {
                  commentflag = FALSE;
               }
            }
         }
      }
   }

   return;
}


/*-------------------------------------------------------
|                                                       |
|                   APsetreceiver()/1                   |
|                                                       |
|  This PSG function is used to set the receiver phase  |
|  from a particular real-time AP table.                |
|                                                       |
|              tablename:  t1 - t60                     |
|                                                       |
+------------------------------------------------------*/
void APsetreceiver(int tablename)
{
   int	recphase;
   int		index,
		i,
		*table_pntr;

   if (bgflag)
      fprintf(stderr, "setreceiver():  specify receiver phase table\n");

   index = tablename - BASEINDEX;
   if (checktable(Table[index], tablename, TRUE, TRUE, TRUE,
   	FALSE, FALSE, FALSE) == TBLERROR)
   {
      text_error("Error in SETRECEIVER\n");
      psg_abort(1);
   }

   table_pntr = Table[index]->table_pointer;
   for (i = 0; i < Table[index]->table_size; i++)
      *table_pntr++ &= 3;

   fprintf(stdout,"setrecv Table[%d]indexptr= %d\n", index, Table[index]->indexptr);
   Table[index]->indexptr = ctss;
   Table[index]->destptr = oph;
   Table[index]->reset = TRUE;

   recphase = tablertv(tablename);
   return;  
}
/*------------------------------------------------------
|                                                      |
|                    tablertv()/1                      |
|                                                      |
|  This PSG function is used to allow the acquisition  |
|  computer to extract values from tables created in   |
|  the ACODE structure.                                |
|                                                      |
|              tablename:  t1 - t60                    |
|                                                      |
+-----------------------------------------------------*/
int tablertv(int tablename)
{
   int	index;
   int codearray[3];

      fprintf(stdout, "tablertv():  extract AP table value\n");

   index = tablename - BASEINDEX;

   if (checktable(Table[index], 0, FALSE, TRUE, FALSE, TRUE,
   	(!Table[index]->auto_inc), FALSE) == TBLERROR)
   {
      text_error("Error in retrieving table value\n");
      psg_abort(1);
   }

   if (!Table[index]->wrflag)
      writetable(Table[index]);	/* Write table to ACODE structure only
				   if a PSG routine attempts to access
				   an element in that table and only if
				   it is the first time that the table
				   is being accessed. */


   codearray[0] = Table[index]->acodeloc ;
   if (Table[index]->auto_inc)
   {
      text_error("tablertv() with AUTO-INCREMENT attribute");
   }
   else
   {
      codearray[1] = Table[index]->indexptr ;
   }
   codearray[2] = Table[index]->destptr ; 
   broadcastCodes(TASSIGN,3,codearray);
   if (psgFile) fprintf(psgFile,"TASSIGN:\n  table: %d\n  index: %d\n  dest: %d\n",
           index, codearray[1],  codearray[2]);


   return(Table[index]->destptr);
}

int tablesv(int tablename)
{
   int	index;
   int codearray[3];

      fprintf(stdout, "tablesv():  put AP table value\n");

   index = tablename - BASEINDEX;

   if (checktable(Table[index], 0, FALSE, TRUE, FALSE, FALSE,
   	(!Table[index]->auto_inc), FALSE) == TBLERROR)
   {
      text_error("Error in retrieving table value\n");
      psg_abort(1);
   }

   if (!Table[index]->wrflag)
      writetable(Table[index]);	/* Write table to ACODE structure only
				   if a PSG routine attempts to access
				   an element in that table and only if
				   it is the first time that the table
				   is being accessed. */


   codearray[0] = Table[index]->acodeloc ;
   if (Table[index]->auto_inc)
   {
      text_error("tablesv() with AUTO-INCREMENT attribute");
   }
   else
   {
      codearray[1] = Table[index]->indexptr ;
   }
   codearray[2] = Table[index]->destptr ; 
   broadcastCodes(TPUT,3,codearray);


   return(Table[index]->destptr);
}


/*------------------------------------------------------
|                                                      |
|              doWriteTable(tablename)                 |
|                                                      |
|  This PSG function write tables that do not have     |
|  explicit getlem(..) calls, but just used in         |
|  pulse statements.                                   |
|                                                      |
+-----------------------------------------------------*/
void doWriteTable(int tablename)
{
   int  index;

      fprintf(stdout, "doWriteTable():  extract AP table value\n");

   index = tablename - BASEINDEX;

   if (checktable(Table[index], 0, FALSE, TRUE, FALSE, TRUE,
        (!Table[index]->auto_inc), FALSE) == TBLERROR)
   {
      text_error("Error in retrieving table value\n");
      psg_abort(1);
   }

   if (!Table[index]->wrflag)
      writetable(Table[index]); /* Write table to ACODE structure only
                                   if a PSG routine attempts to access
                                   an element in that table and only if
                                   it is the first time that the table
                                   is being accessed. */
   return;
}

/*---------------------------------------------------------
|                                                         |
|                    writetable()/1                       |
|                                                         |
|  This PSG function writes the real-time table elements  |
|  into the ACODE structure.  The table contains 16-bit   |
|  integers.  The architecture of the table in the ACODE  |
|  structure is as follows:                               |
|                                                         |
|        element 1:  table number - 1                     |
|        element 2:  table size                           |
|        element 3:  auto-increment flag                  |
|        element 4:  divn-return factor                   |
|        element 5:  auto-increment counter               |
|        element 6:  first table value                    |
|                                                         |
|                                                         |
|  The argument for this function is the structure for    |
|  the particular table to be created in the ACODE        |
|  structure.                                             |
|                                                         |
+--------------------------------------------------------*/
static void writetable(Tableinfo *tblinfo)
{
   int	i, *tabpntr;
   int numcodes, buffersize, *buffer;
   int elemSize;

      fprintf(stderr, "writetable():  write out tables values\n");

   if (checktable(tblinfo, 0, FALSE, TRUE, TRUE, FALSE, FALSE,
   	FALSE) == TBLERROR)
   {
      text_error("Error in storing table values\n");
      psg_abort(1);
   }

/********************************************************
*  Check if this table has already been written to the  *
*  ACODE structure.  If not, set the writeflag to TRUE  *
*  and write the table into the ACODE structure.        *
********************************************************/

   if (tblinfo->wrflag) 
   {
      return;
   }
   else
   {
      tblinfo->wrflag = TRUE;
   }

/***********************************************
*  Write table data into the ACODE structure.  *
***********************************************/


  numcodes   = 7;
  buffersize = (tblinfo->table_size) + numcodes;
  buffer     = (int *) malloc(buffersize*sizeof(int));
  if (buffer == NULL)
  {
     text_error("Error in writetable. memory allocation failed\n");
     psg_abort(1);
  }

  elemSize = 32;
  /* Check if table can be compressed */
  if ( (tblinfo->table_size > 1) &&
       (tblinfo->compress) )
  {
     int val, max;
     max = 0;
     tabpntr = tblinfo->table_pointer;
     for (i = 0; i < tblinfo->table_size; i++)
     {
        val = (int) (*tabpntr++) ;
        if (val < 0)
        {
           max = 1024;
           break;
        }
        if (val > max)
           max = val;
     }
     if (max <= 1)
        elemSize = 1;  /* single bits */
  }
  buffer[0] = TABLEHEADER + (tblinfo->acodeloc) ; /* This needs to be revisited, cuurently Table Number */
  buffer[1] = tblinfo->table_size ;        /* size of table */
  buffer[2] = tblinfo->auto_inc ;          /* auto-inc flag */
  buffer[3] = tblinfo->divn_factor ;       /* divn-rtrn factor */
  buffer[4] = 0 ;                          /* auto-inc ct loc */
  buffer[5] = elemSize;
  buffer[6] = 0;                           /* space to numcodes */

   tabpntr = tblinfo->table_pointer;
   fprintf(stdout,"writetable\n");
   if (elemSize == 32)
      for (i = 0; i < tblinfo->table_size; i++)
      {
        buffer[numcodes] = (int) (*tabpntr++) ;
         fprintf(stdout,"buf[%d]= %d\n", i, buffer[numcodes]);
        numcodes++;
      }
   if (elemSize == 1)
   {
      int val;
      int j;
      int newval;
      
      i=1;
      while (i*32 <= tblinfo->table_size)
      {

        newval = 0;
        for (j = 0; j < 32; j++)
        {
           val = (int) (*tabpntr++) ;
           if (val)
              newval |= (val << j);
        }
        buffer[numcodes] = newval;
        numcodes++;
        i++;
      }
      if ( tblinfo->table_size % 32)
      {
        newval = 0;
        for (j = 0; j < tblinfo->table_size % 32; j++)
        {
           val = (int) (*tabpntr++) ;
           if (val)
              newval |= (val << j);
        }
        buffer[numcodes] = newval;
        numcodes++;
      }
   }
   buffer[6] = numcodes;                           /* space to numcodes */
   broadcastCodes(TABLE, numcodes, buffer);
   if (psgFile)
   {
      fprintf(psgFile,"TABLE:\n  index: %d\n  size: %d\n  index: %d\n  dest: %d\n",
            tblinfo->table_number, tblinfo->table_size, tblinfo->indexptr, tblinfo->destptr);
#ifdef XXX
      fprintf(psgFile,"TABLE:\n  index: %d\n", tblinfo->table_number);
      for (i = 1; i < 7; i++)
          fprintf(psgFile,"  info%d: %d\n", i,  buffer[i]);
#endif
      for (i = 0; i < tblinfo->table_size; i++)
          fprintf(psgFile,"  val: %d\n", buffer[7+i]);
   }

   return;
}

/*---------------------------------------------------
|                                                   |
|                    settable()/3                   |
|                                                   |
|  This PSG function creates a table in the ACODE   |
|  structure.  The values stored in tables created  |
|  in this fashion must be manually accessed.       |
|                                                   |
|           tablename:  t1 - t60                    |
|         numelements:  size of table               |
|          tablearray:  integer array containing    |
|                       the actual table values     |  
|                                                   |
+--------------------------------------------------*/
void settable(int tablename, int numelements, int tablearray[])
{
   int	index,
	i,
	*holdpntr,
	*tablepntr;

   if (bgflag)
      fprintf(stderr, "settable():  create AP table\n");

   index = tablename - BASEINDEX;
  
   table_order[last_table] = index;
   if (last_table == MAXTABLE)
   {
      text_error("Error in SETTABLE:  maximum number of tables exceeded\n");
      psg_abort(1);
   }
   else if (!isTable(tablename))
   {
      text_error("Error in SETTABLE:  invalid table name\n");
      psg_abort(1);
   }
   else if (Table[index]->table_size != numelements)
   {

      if (Table[index]->table_number > 0)
      {
         free(Table[index]->table_pointer);
         free(Table[index]->hold_pointer);
      }

      Table[index]->table_pointer = (int *) (malloc(sizeof(int)
      		 * (numelements + 1)));
      if (Table[index]->table_pointer == NULL)
      {
         text_error("Unable to allocate new table memory\n");
         psg_abort(1);
      }

      Table[index]->hold_pointer = (int *) (malloc(sizeof(int)
         	* (numelements + 1)));
      if (Table[index]->hold_pointer == NULL)
      {
         text_error("Unable to allocate new table copy memory\n");
         psg_abort(1);
      }

      tablepntr = Table[index]->table_pointer;
      holdpntr = Table[index]->hold_pointer;

      for (i = 0; i < numelements; )
      {
         *tablepntr++ = tablearray[i];
         *holdpntr++ = tablearray[i++];
      }
   }

   Table[index]->table_number = ++last_table;
   Table[index]->table_size = numelements;
   Table[index]->auto_inc = FALSE;
   Table[index]->divn_factor = 1;
   Table[index]->acodeloc = index;


   return;
}
void inittable(int tablename, int numelements)
{
   int	index,
	i,
	*holdpntr,
	*tablepntr;

   if (bgflag)
      fprintf(stderr, "inittable():  create AP table\n");

   index = tablename - BASEINDEX;
  
   table_order[last_table] = index;
   if (last_table == MAXTABLE)
   {
      text_error("Error in SETTABLE:  maximum number of tables exceeded\n");
      psg_abort(1);
   }
   else if (!isTable(tablename))
   {
      text_error("Error in SETTABLE:  invalid table name\n");
      psg_abort(1);
   }
   else if (Table[index]->table_size != numelements)
   {

      if (Table[index]->table_number > 0)
      {
         free(Table[index]->table_pointer);
         free(Table[index]->hold_pointer);
      }

      Table[index]->table_pointer = (int *) (malloc(sizeof(int)
      		 * (numelements + 1)));
      if (Table[index]->table_pointer == NULL)
      {
         text_error("Unable to allocate new table memory\n");
         psg_abort(1);
      }

      Table[index]->hold_pointer = (int *) (malloc(sizeof(int)
         	* (numelements + 1)));
      if (Table[index]->hold_pointer == NULL)
      {
         text_error("Unable to allocate new table copy memory\n");
         psg_abort(1);
      }

      tablepntr = Table[index]->table_pointer;
      holdpntr = Table[index]->hold_pointer;

      for (i = 0; i < numelements; i++ )
      {
         *tablepntr++ = 0;
         *holdpntr++ = 0;
      }
   }

   Table[index]->table_number = ++last_table;
   Table[index]->table_size = numelements;
   Table[index]->auto_inc = FALSE;
   Table[index]->compress = FALSE;
   Table[index]->divn_factor = 1;
   Table[index]->acodeloc = index;


   return;
}
/*---------------------------------------------------
|                                                   |
|                   getelem()/3                     |
|                                                   |
|  This PSG function retrieves a value from an AP   |
|  table and places it in an AP variable.           |
|                                                   |
|          tablename:  t1 - t60                     |
|            indxptr:  real-time AP variable        |
|             dstptr:  real-time AP variable;       |
|                      must be V1-V32 or OPH        |
|                                                   |
+--------------------------------------------------*/
void getelem(int tablename, int indxptr, int dstptr)
{
   int	index;

   if (bgflag)
      fprintf(stderr, "getelem():  get individual AP table value\n");

   if (!isTable(tablename))
   {
      text_error("Error in getelem:  invalid table name\n");
      psg_abort(1);
   }

   index = tablename - BASEINDEX;
   Table[index]->indexptr = indxptr;
   fprintf(stdout,"getelem Table[%d]indexptr= %d\n", index, Table[index]->indexptr);
   Table[index]->destptr = dstptr;

   tablertv(tablename);
   return;
}
void putelem(int tablename, int indxptr, int dstptr)
{
   int	index;

   if (bgflag)
      fprintf(stderr, "putelem():  put individual AP table value\n");

   if (!isTable(tablename))
   {
      text_error("Error in putelem:  invalid table name\n");
      psg_abort(1);
   }

   index = tablename - BASEINDEX;
   Table[index]->indexptr = indxptr;
   fprintf(stdout,"putelem Table[%d]indexptr= %d\n", index, Table[index]->indexptr);
   Table[index]->destptr = dstptr;

   tablesv(tablename);
   return;
}
/*---------------------------------------------------
|                                                   |
|                   getTabSkip()/3                  |
|                                                   |
|  This PSG function scans an on/off table for the  |
|  how many zeros to skip before the next one is    |
|  found.  It places the count in an AP variable.   |
|                                                   |
|          tablename:  t1 - t60                     |
|            indxptr:  real-time AP variable        |
|             dstptr:  real-time AP variable;       |
|                      must be V1-V32 or OPH        |
|                                                   |
+--------------------------------------------------*/
void getTabSkip(int tablename, int indxptr, int dstptr)
{
   int	index;
   int codearray[3];

   if (bgflag)
      fprintf(stderr, "getTabSkip():  get individual AP table value\n");

   if (!isTable(tablename))
   {
      text_error("Error in getTabSkip:  invalid table name\n");
      psg_abort(1);
   }

   index = tablename - BASEINDEX;
   Table[index]->indexptr = indxptr;
   fprintf(stdout,"getTabSkip Table[%d]indexptr= %d\n", index, Table[index]->indexptr);
   Table[index]->destptr = dstptr;

   doWriteTable(tablename);
   codearray[0] = Table[index]->acodeloc ;
   if (Table[index]->auto_inc)
   {
      text_error("getTabSkip with AUTO-INCREMENT attribute");
      psg_abort(1);
   }
   else
   {
      codearray[1] = Table[index]->indexptr ;
   }
   codearray[2] = Table[index]->destptr ; 
   broadcastCodes(TCOUNT,3,codearray);
   return;
}
/*----------------------------------------------------
|                                                    |
|                   tabletop()/4                     |
|                                                    |
|  This PSG function allows mathematical operations  |
|  between two tables.                               |
|                                                    |
|   operationtype:  type of mathematical operation,  |
|                   e.g., TADD for addition          |
|      table1name:  name of table 1, which is also   |
|                   the result (destination) table;  |
|                   t1 - t60 are valid names         |
|      table2name:  name of table 2                  |
|          modval:  active if > 0; each table ele-   |
|                   is taken modulo "modval"         |
|                                                    |
|                                                    |
|  The order of the operation is (table1 ? table2)   |
|  where ? symbolizes a particular mathematical op-  |
|  erational symbol.                                 |
|                                                    |
+---------------------------------------------------*/
void tabletop(int operationtype, int table1name, int table2name, int modval)
{
   int	index1,
	index2,
	index,
	i,
   	*elem1pntr,
	*elem2pntr;

   if (bgflag)
      fprintf(stderr, "tabletop():  table-table math operation\n");

   index1 = table1name - BASEINDEX;
   index2 = table2name - BASEINDEX;

   if (checktable(Table[index1], table1name, TRUE, TRUE, TRUE,
   	FALSE, FALSE, TRUE) == TBLERROR)
   {
      text_error("Error in table-table math operation\n");
      psg_abort(1);
   }
   else if (checktable(Table[index2], table2name, TRUE, TRUE, TRUE,
        FALSE, FALSE, FALSE) == TBLERROR)
   {
      text_error("Error in table-table math operation\n");
      psg_abort(1);
   }
   else if (Table[index1]->table_size < Table[index2]->table_size)
   {
      text_error("\nThe first table must have >= the number of elements\n");
      text_error("as the second table for table-table math operations\n");
      text_error("\n");
      psg_abort(1);
   }

/**********************************
*  Perform arithmetic operation.  *
**********************************/
   
   for (i = 0; i < Table[index1]->table_size; i++)
   {
      index = i % Table[index2]->table_size;
      elem1pntr = Table[index1]->table_pointer + i;
      elem2pntr = Table[index2]->table_pointer + index;

      if (table_math(operationtype, elem1pntr, elem2pntr)
      		== TBLERROR)
      {
         text_error("Error in table-table math operation\n");
         psg_abort(1);
      }

      if (modval > 0)
      {
         *elem1pntr %= modval;
         if (*elem1pntr < 0)
            *elem1pntr += modval;
      }
   }

   Table[index1]->reset = TRUE;
   return;
}
/*-----------------------------------------------------
|                                                     |
|                    tablesop()/4                     |
|                                                     |
|  This PSG function allows mathematical operations   |
|  between a table and a scalar integer.              |
|                                                     |
|   operationtype:  type of mathematical operation,   |
|                   e.g., TADD for addition           |
|       tablename:  t1 - t60 are valid names          |
|       scalarval:  scalar integer                    |
|          modval:  active if > 0; each table ele-    |
|                   is taken modulo "modval"          |
|                                                     |
|                                                     |
|  The order of the operation is (table ? scalarval)  |
|  where ? symbolizes a particular mathematical op-   |
|  erational symbol.                                  |
|                                                     | 
+----------------------------------------------------*/
void tablesop(int operationtype, int tablename, int scalarval, int modval)
{
   int		index,
		i,
   		*elempntr;

   if (bgflag)
      fprintf(stderr, "tablesop():  table-scalar math operation\n");

   index = tablename - BASEINDEX;

   if (checktable(Table[index], tablename, TRUE, TRUE, TRUE,
        FALSE, FALSE, TRUE) == TBLERROR)
   {
      text_error("Error in table-scalar math operation\n");
      psg_abort(1);
   }

/**********************************
*  Perform arithmetic operation.  *
**********************************/

   elempntr = Table[index]->table_pointer;
   for (i = 0; i < Table[index]->table_size; i++)
   {
      if (table_math(operationtype, elempntr, &scalarval)
                == TBLERROR)
      {
         text_error("Error in table-scalar math operation\n");
         psg_abort(1);
      }

      if (modval > 0)
      {
         *elempntr %= modval;
         if (*elempntr < 0)
            *elempntr += modval;
      }

      elempntr++;
   }

   Table[index]->reset = TRUE;
   return;
}

/*---------------------------------------------------
|                                                   |
|                  inittablevar()/0                 |
|                                                   |
|  This PSG function initializes all relevant var-  |
|  iables associated with AP (real-time) tables.    |
|                                                   |
+--------------------------------------------------*/
void inittablevar()
{
   int	i;
   int index;

   if (bgflag)
      fprintf(stderr, "inittablevar():  initialize PSG table variables\n");

   last_table = 0;
   if (ix == 1)
   {
      t1 = BASEINDEX ;
      t2 = t1 + 1;  t3 = t1 + 2;  t4 = t1 + 3;
      t5 = t1 + 4;  t6 = t1 + 5;  t7 = t1 + 6;
      t8 = t1 + 7;  t9 = t1 + 8;  t10 = t1 + 9;
      t11 = t1 + 10;  t12 = t1 + 11;  t13 = t1 + 12;
      t14 = t1 + 13;  t15 = t1 + 14;  t16 = t1 + 15;
      t17 = t1 + 16;  t18 = t1 + 17;  t19 = t1 + 18;
      t20 = t1 + 19;  t21 = t1 + 20;  t22 = t1 + 21;
      t23 = t1 + 22;  t24 = t1 + 23;  t25 = t1 + 24;
      t26 = t1 + 25;  t27 = t1 + 26;  t28 = t1 + 27;
      t29 = t1 + 28;  t30 = t1 + 29;  t31 = t1 + 30;
      t32 = t1 + 31;  t33 = t1 + 32;  t34 = t1 + 33;
      t35 = t1 + 34;  t36 = t1 + 35;  t37 = t1 + 36;
      t38 = t1 + 37;  t39 = t1 + 38;  t40 = t1 + 39;
      t41 = t1 + 40;  t42 = t1 + 41;  t43 = t1 + 42;
      t44 = t1 + 43;  t45 = t1 + 44;  t46 = t1 + 45;
      t47 = t1 + 46;  t48 = t1 + 47;  t49 = t1 + 48;
      t50 = t1 + 49;  t51 = t1 + 50;  t52 = t1 + 51;
      t53 = t1 + 52;  t54 = t1 + 53;  t55 = t1 + 54;
      t56 = t1 + 55;  t57 = t1 + 56;  t58 = t1 + 57;
      t59 = t1 + 58;  t60 = t1 + 59;

/*****************************************
*  Create pointers to table structures.  *
*****************************************/



      for (i = 0; i < MAXTABLE; i++)
      {
         Table[i] = (Tableinfo *) (malloc(sizeof(Tableinfo)));
         if (Table[i] == NULL)
         {
            text_error("Unable to allocate memory for table structures\n");
            psg_abort(1);
         }
      }

/*********************************************
*  Initialize elements of table structures.  *
*********************************************/

      if (ntIndex > 0)
         index = res_4_internal;
      else
         index = ctss;
      for (i = 0; i < MAXTABLE; i++)
      {
         Table[i]->indexptr = index;
         Table[i]->destptr = tablert;
         Table[i]->table_number = 0;
         Table[i]->table_size = 0;
         Table[i]->divn_factor = 1;
         Table[i]->auto_inc = FALSE;
         Table[i]->reset = FALSE;
         Table[i]->compress = TRUE;

         table_order[i] = 0;
         tmptable_order[i] = 0;
      }

      for (i = 0; i < MAXLOADTABLECALLS; i++)
      {
         tablelimits[i].start = 0;
         tablelimits[i].end = 0;
      }
   }

   if (ntIndex > 0)
   {
      F_initval( (double) (ntIndex-1), res_4_internal);
      F_initval( (double) (ntIndex-1), oph);
   }
   for (i = 0; i < MAXTABLE; i++)	/* must be initialized each time */
      Table[i]->wrflag = FALSE;

   return;
}
/*----------------------------------------------------
|						     |
|		reparse_release()/2		     |
|						     |
|  This function releases all table pointers crea-   |
|  ted from the nth call to LOADTABLE for the pre-   |
|  vious PSG increment if one is either re-pars-     |
|  the same table file or parsing a new table file.  |
|						     |
+---------------------------------------------------*/
static int reparse_release(int startindex,int endindex)
{
   int	i,
	index;

   for (i = startindex; i < endindex; )
   {
      index = table_order[i++];
 
      Table[index]->indexptr = ctss;
      Table[index]->destptr = tablert;
      Table[index]->wrflag = FALSE;
      Table[index]->table_number = 0;
      Table[index]->table_size = 0;
      Table[index]->divn_factor = 1;
      Table[index]->auto_inc = FALSE;
      Table[index]->reset = FALSE;
      Table[index]->compress = TRUE;
 
      free(Table[index]->table_pointer);
      free(Table[index]->hold_pointer);

      Table[index]->table_pointer = NULL;
      Table[index]->hold_pointer = NULL;
   }

   return(COMPLETE);
}
/*----------------------------------------------------
|						     |
|		   open_table()/4		     |
|						     |
|  This function opens the table file.  It looks     |
|  first in the user table directory and then in     |
|  the system table directory.  If the requested     |
|  table file is not found in either directory,      |
|  an error is returned and PSG is psg_aborted.          |
|						     |
+---------------------------------------------------*/
static FILE *open_table(const char *basename, const char *concatname,
                        const char *inputname, const char *permission)
{
   char	tblfilename[MAXSTR];
   FILE	*tablefile;

   strcpy(tblfilename, basename);
   strcat(tblfilename, concatname);
   strcat(tblfilename, inputname);
   tablefile = fopen(tblfilename, permission);

   return(tablefile);
}

/*----------------------------------------------------
|						     |
|		   read_number()/3		     |
|						     |
|  This function reads a number out of the table     |
|  text file. Modified to allow for negative 	     |
|  values (S. York 11/90).			     |
|						     |
+---------------------------------------------------*/
static int read_number(FILE *tablefile, int eofflag, int *dest)
{
   int	value = 0,
	tmp;
   int neg_flag = FALSE;  /* flag added so that negative table values can */
			  /* be parsed. 9/18/1990  SPY */

   if (c == EOF)
   {
      if (eofflag)
         return(TBLERROR);

      value = 1;
   }
   else
   {   
      while ((c != ' ') && (c != '\t') && (c != '\n'))
      {
         if ((c == EOF) && (eofflag))
         {
            return(TBLERROR);
         }
         else if (c == EOF)
         {
            break;
         }

         if ((c == '}') || (c == ']') || (c == ')'))
         {
            ungetc(c, tablefile);
            break;
         }

	 if (c == '-')
	 {
	    neg_flag = TRUE;
	 }
	 else
	 {
            tmp = c - '0';
            if ((tmp < 0) || (tmp > 9))
            {
                return(TBLERROR);
            }
            else
            {
                value *= 10;
                value += tmp;
            }
	 }

         c = getc(tablefile);
         checkforcomments(tablefile);
      }
   }

   /* check to see if the value was < 0 */
   if (neg_flag)
	*dest = -value;
   else
	*dest = value;

   return(0);    /* return 'no error' */
}

/*----------------------------------------------------
|						     |
|		   check_bounds()/3		     |
|						     |
|  This function checks the bounds for a particular  |
|  parameter.					     |
|						     |
+---------------------------------------------------*/
static int check_bounds(int paramval, int maxvalue, int minvalue)
{
   if ((paramval < minvalue) || (paramval > maxvalue))
   {
      return(TBLERROR);
   }
   else
   {
      return(COMPLETE);
   }
}
/*----------------------------------------------------
|						     |
|		 load_element()/6		     |
|						     |
|  This function loads a single element or an array  |
|  of elements into the table memory.		     |
|						     |
+---------------------------------------------------*/
static void load_element(int outcount, int incount, int table_index, int **elempntrpntr,
		int *values, int incntflag)
{
   int	i,
	k,
	cnt;

   for (k = 0; k < outcount; k++)
   {
      for (i = 0; i < incount; i++)
      {
         Table[table_index]->table_size++;
         if (Table[table_index]->table_size > MAXELEMENTS)
         {
            text_error("Error in LOADTABLE:  max table size exceeded\n");
            psg_abort(1);
         }
 
         if (incntflag)
         {
            cnt = i;
         }
         else
         {
            cnt = k;
         }

         **elempntrpntr = *(values + cnt);
         *elempntrpntr += 1;
      }
   }

   return;
}
/*----------------------------------------------------
|						     |
|		 store_in_table()/2		     |
|						     |
|  This function puts the table values in scratch    |
|  memory into the region of memory reserved for     |
|  the particular table and its master copy.         |
|						     |
+---------------------------------------------------*/
static int store_in_table(int table_index, int *elementpntr)
{
   int	*startholdpntr,
	*starttablepntr,
	i;

   Table[table_index]->table_pointer = (int *) (malloc(sizeof
   		(*elementpntr) * (Table[table_index]->table_size + 1)));
   if (Table[table_index]->table_pointer == NULL)
   {
      return(TBLERROR);
   }

   Table[table_index]->hold_pointer = (int *) (malloc(sizeof
   		(*elementpntr) * (Table[table_index]->table_size + 1)));
   if (Table[table_index]->hold_pointer == NULL)
   {
      return(TBLERROR);
   }

   starttablepntr = Table[table_index]->table_pointer;
   startholdpntr = Table[table_index]->hold_pointer;

   if (debug)
      printf("Size of table T%d = %d\n", table_index+1,
   		Table[table_index]->table_size);

   for (i = 0; i < Table[table_index]->table_size; i++)
   {
      *startholdpntr++ = *elementpntr;
      *starttablepntr++ = *elementpntr++;
   }

   return(COMPLETE);
}
/*----------------------------------------------------
|						     |
|		   writedebug()/2		     |
|						     |
|  This function produces a debug output file to     |
|  allow the table parsing operation to be checked.  |
|						     |
+---------------------------------------------------*/
static void writedebug(FILE *fd, int ntables)
{
   int	index,
	*elementpntr,
	i,
	j;

   fprintf(fd, "Number of tables = %d\n", ntables - last_table);
   fprintf(fd, "\n");
   fprintf(fd, "\n");
 
   for (i = last_table; i < ntables; i++)
   {
      index = table_order[i];
      fprintf(fd, "Number of phases in table T%d = %d\n", index + 1,
          Table[index]->table_size);
 
      if (Table[index]->auto_inc)
         fprintf(fd, "Table %d is AUTOINCREMENT.\n", index + 1);
 
      if (Table[index]->divn_factor > 1)
         fprintf(fd, "Table T%d is DIVN return where N = %d.\n",         
            index + 1, Table[index]->divn_factor);
 
      fprintf(fd, "\n");
      elementpntr = Table[index]->hold_pointer;
 
      for (j = 0; j < Table[index]->table_size; j++)
         fprintf(fd, "    Phase[%d] = %d\n", j + 1, *elementpntr++);
 
      fprintf(fd, "\n");
      fprintf(fd, "\n");
   }

   return;
}
/*----------------------------------------------------
|						     |
|		   checktable()/9		     |
|						     |
|  This function checks the requested table param-   |
|  eters.					     |
|						     |
+---------------------------------------------------*/
static int checktable(Tableinfo *tblinfo, int tblname, int namechk, int numberchk,
	int sizechk, int destchk, int indxchk, int writechk)
{
   if ((!isTable(tblname)) && (namechk))
   {
      text_error("Invalid table name\n");
      return(TBLERROR);
   }
   else if ((!isValidNumber(tblinfo->table_number)) &&
   	(numberchk))
   {
      text_error("Invalid table number\n");
      return(TBLERROR);
   }
   else if ((tblinfo->table_size < 1) && (sizechk))
   {
      text_error("Invalid table size\n");
      return(TBLERROR);
   }
   else if ((!isValidDest(tblinfo->destptr)) && (destchk))
   {
      text_error("Invalid table destination AP variable\n");
      return(TBLERROR);
   }
   else if ((!isValidIndex(tblinfo->indexptr)) && (indxchk))
   {
      text_error("Invalid table index AP variable\n");
      return(TBLERROR);
   }
   else if ((tblinfo->wrflag) && (writechk))
   {
      text_error("Table already written to ACODE structure\n");
      return(TBLERROR);
   }

   return(COMPLETE);
}
/*----------------------------------------------------
|						     |
|		   reset_table()/1		     |
|						     |
|  This function copies the master table into the    |
|  work table and resets certain elements in the     |
|  table information structure.                      |
|						     |
+---------------------------------------------------*/
void reset_table(Tableinfo *tblinfo)
{
   int	i,
	*table_pntr,
	*hold_pntr;

   table_pntr = tblinfo->table_pointer;
   hold_pntr = tblinfo->hold_pointer;
   for (i = 0; i < tblinfo->table_size; i++)
      *table_pntr++ = *hold_pntr++;

   return;
}
/*----------------------------------------------------
|						     |
|		   table_math()/3		     |
|						     |
|  This function performs the appropriate math op-   |
|  eration on two pointers to scalar values.         |
|						     |
+---------------------------------------------------*/
static int table_math(int optype, int *value1, int *value2)
{
   switch(optype)
   {
      case TADD:
      {
         *value1 += *value2;
         break;
      }
      case TSUB:
      {
         *value1 -= *value2;
         break;
      }
      case TMULT:
      {
         *value1 *= *value2;
         break;
      }
      case TDIV:
      {
         if (*value2 == 0)
         {
            text_error("Illegal divide by 0\n");
            return(TBLERROR);
         }

         *value1 /= *value2;
         break;
      }
      default:
      {
         text_error("Invalid math operation for tables\n");
         return(TBLERROR);
      }
   }

   return(COMPLETE);
}

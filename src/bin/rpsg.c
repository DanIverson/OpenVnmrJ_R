

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DELAY 100
#define PULSE 110
#define INITSCAN 120
#define NEXTSCAN 130
#define ENDSCAN  140
#define FREQ  150
#define ACQUIRE  160
#define CNT 121
#define RTOP  170
#define RT2OP  180
#define RT3OP  190
#define TABLE  200
#define RTINIT  210
#define FRTINIT 220
#define TASSIGN 230
#define SHAPEPULSE 240

// RTOP
#define     INC         (2)
#define    DEC       (3)
// RT2OP
#define     SET         (20)
#define     MOD2        (21)
#define     MOD4        (22)
#define     HLV         (23)
#define     DBL         (24)
#define     NOT         (25)
#define     NEG         (26)
// RT3OP
#define     ADD         (30)
#define     SUB         (31)
#define     MUL         (32)
#define     DIV         (33)
#define     MOD         (34)
#define     OR       (35)

#define LINE_LIMIT 1024

struct _node {
          int flavour;
          int *tbl;
          int numVals;
          double aval[7];
          struct _node   *next;
          struct _node   *stm1;
};
typedef struct _node node;

#define MAXTABLE 60
#define MAXLC 90
#define NP_  0
#define NT_  1
#define CT_  2
#define RTVPTR_  10
#define OPH_  14
#define SSCT_ 18
#define ZERO_ 27
#define V1_   31
#define V42_  72

int lc[MAXLC];
node *table[MAXTABLE];

void execTree(node *p);

static void initParser()
{
   int i;
   for (i=0; i < MAXLC; i++)
      lc[i] = 0;
   lc[ZERO_ + 1] = 1;
   lc[ZERO_ + 2] = 2;
   lc[ZERO_ + 3] = 3;
   for (i=0; i < MAXTABLE; i++)
      table[i] = NULL;
}

static void resetRTVars()
{
   int i;
   for (i=V1_; i <= V42_; i++)
      lc[i] = 0;
   lc[OPH_] = 0;
}

static int getOpAction(char *token)
{
   if ( ! strcmp(token, "INC") )
      return(INC);
   else if ( ! strcmp(token, "DEC") )
      return(DEC);
   else if ( ! strcmp(token, "SET") )
      return(SET);
   else if ( ! strcmp(token, "MOD2") )
      return(MOD2);
   else if ( ! strcmp(token, "MOD4") )
      return(MOD4);
   else if ( ! strcmp(token, "HLV") )
      return(HLV);
   else if ( ! strcmp(token, "DBL") )
      return(DBL);
   else if ( ! strcmp(token, "NOT") )
      return(NOT);
   else if ( ! strcmp(token, "NEG") )
      return(NEG);
   else if ( ! strcmp(token, "ADD") )
      return(ADD);
   else if ( ! strcmp(token, "SUB") )
      return(SUB);
   else if ( ! strcmp(token, "MUL") )
      return(MUL);
   else if ( ! strcmp(token, "DIV") )
      return(DIV);
   else if ( ! strcmp(token, "MOD") )
      return(MOD);
   else if ( ! strcmp(token, "OR") )
      return(OR);
   fprintf(stderr,"Unknow OP function!\n");
   return(0);
}

static int getToken(FILE *file, char *key, char *attr)
{
  int ch,i;
  char line[LINE_LIMIT];
  int ret;

  line[0] = '\0';
  i = 0;
  ch = EOF;
  while ((i < LINE_LIMIT -1) && ((ch = getc(file)) != EOF) && (ch != '\n'))
    line[i++] = ch;
  // check for windows end of line '\r\n'
  if (i && (line[i-1] == '\r') && (ch == '\n'))
    i--;
  line[i] = '\0';
  if ((i == 0) && (ch == EOF))
     return(0);
  fprintf(stderr,"Line: %s\n",line);
  ret = sscanf(line, "%[^:]: %s[^\n]\n", key, attr);
  if (ret != 2)
     attr[0] = '\0';
  fprintf(stderr,"key: %s attr: %s\n",key, attr);
  return(1);
}

#define DEBUG
#ifdef DEBUG
void space(FILE *f, int n)
{
   while (0 < n--)
      fprintf(f," ");
}

/*------------------------------------------------------------------------------
|
|       showFlavour/2
|
|       This procedure is used to output the flavour of a single node.
|
+-----------------------------------------------------------------------------*/

void showFlavour(FILE *f, node *p)
{   switch(p->flavour)
    {
      case DELAY:   fprintf(f,"DELAY");
                    break;
      case PULSE:   fprintf(f,"PULSE");
                    break;
      case SHAPEPULSE:   fprintf(f,"SHAPEPULSE");
                    break;
      case INITSCAN:    fprintf(f,"INITSCAN");
                    break;
      case NEXTSCAN:    fprintf(f,"NEXTSCAN");
                    break;
      case ENDSCAN:    fprintf(f,"ENDSCAN");
                    break;
      case FREQ:    fprintf(f,"FREQ");
                    break;
      case ACQUIRE: fprintf(f,"ACQUIRE");
                    break;
      case RTOP:    fprintf(f,"RTOP");
                    break;
      case RT2OP:   fprintf(f,"RT2OP");
                    break;
      case RT3OP:   fprintf(f,"RT3OP");
                    break;
      case TABLE:   fprintf(f,"TABLE");
                    break;
      case TASSIGN:  fprintf(f,"TASSIGN");
                    break;
      case RTINIT:  fprintf(f,"RTINIT");
                    break;
      case FRTINIT: fprintf(f,"FRTINIT");
                    break;
      default:      fprintf(f,"unknown (=%d)",p->flavour);
                    break;
    }
}

/*------------------------------------------------------------------------------
|
|       showTree/3
|
|       This procedure is used to output a code tree in a nice readable form
|       reflecting its tree structure.
|
+-----------------------------------------------------------------------------*/

void showTree(int n, node *p)
{
   node *q;

   if (p)
   {
      space(stderr,n);
      showFlavour(stderr,p);
      fprintf(stderr,"\n");
      for (int i=0; i < p->numVals; i++)
      {
         space(stderr,n);
         fprintf(stderr,"  %f\n", p->aval[i]);
      }
      if (p->stm1)
         showTree(n+2,p->stm1);
      q = p->next;
      if (q)
      {
         showTree(n,q);
      }
   }
}
#endif  // DEBUG

void doDelay(node *p)
{
   fprintf(stderr,"delay %f seconds\n", p->aval[0]);
}

void doPulse(node *p)
{
   int index;

   index = (int) p->aval[2];
   if (index > MAXLC)
   {
      int phase;
      index -= 1024;
      node *p2tbl;

      p2tbl = table[index];
      fprintf(stderr,"destptr= %d\n",(int) p2tbl->aval[3]);
      phase = lc[(int) p2tbl->aval[3]];

      // val = *(ptr+(lc[CT_] % tableSize[index]));
      fprintf(stderr,"pulse  on channel %d for %f seconds using table %d for phase %d\n",
         (int) p->aval[0], p->aval[1], index,phase);
   }
   else
      fprintf(stderr,"pulse  on channel %d for %f seconds with phase %d\n",
         (int) p->aval[0], p->aval[1], lc[(int) p->aval[2]] % 4);

}

void doShapepulse(node *p)
{
   int index;

   index = (int) p->aval[4];
   if (index > MAXLC)
   {
      int phase;
      index -= 1024;
      node *p2tbl;

      p2tbl = table[index];
      fprintf(stderr,"destptr= %d\n",(int) p2tbl->aval[3]);
      phase = lc[(int) p2tbl->aval[3]];

      // val = *(ptr+(lc[CT_] % tableSize[index]));
      fprintf(stderr,"shapepulse with shape index %d on channel %d for %f seconds using table %d for phase %d\n",
         (int) p->aval[1], (int) p->aval[0], p->aval[2]*p->aval[3],
         index,phase);
   }
   else
      fprintf(stderr,"shapepulse with shape index %d on channel %d for %f seconds with phase %d\n",
         (int) p->aval[1], (int) p->aval[0], p->aval[2]*p->aval[3],
         lc[(int) p->aval[4]] % 4);

}

void doFreq(node *p)
{
   fprintf(stderr,"set frequency on RF channel %d to %f\n",
         (int) p->aval[0], p->aval[1]);
}

void doAcquire(node *p)
{
   fprintf(stderr,"acquire %d points with dwell %f and phase %d\n",
         (int) p->aval[1], p->aval[2], lc[ OPH_ ] % 4);
}

void doInitscan(node *p)
{
   fprintf(stderr,"init scan for array element %d with np=%d and nt=%d\n",
         (int) p->aval[0],
         (int) p->aval[1],
         (int) p->aval[2]);
   lc[NP_] = (int) p->aval[1];
   lc[NT_] = (int) p->aval[2];
   lc[CT_] = 0;
   resetRTVars();
}

void doNextscan(node *p)
{
   fprintf(stderr,"next scan RTVPTR_ = %d  \n", RTVPTR_);
   for (int i=0; i < lc[NT_]; i++)
   {
      lc[RTVPTR_] = ( lc[NT_] + lc[CT_] ) % lc[NT_];

      lc[OPH_] = lc[RTVPTR_];
      execTree(p->stm1);
      if ( lc[SSCT_] > 0 )
         lc[SSCT_] -= 1;
      else
         lc[CT_] += 1;
   }
}

void doRtop(node *p)
{
   switch( (int) p->aval[0])
   {
      case INC:     fprintf(stderr,"INC %d\n",
                            (int)  p->aval[1]);
                    lc[ (int) p->aval[1]] += 1;
                    break;
      case DEC:     fprintf(stderr,"DEC %d\n",
                            (int)  p->aval[1]);
                    lc[ (int) p->aval[1]] -= 1;
                    break;
      default:      fprintf(stderr,"Unknown RTOP cmd %d\n", (int) p->aval[0]);
                    break;
   }
}

void doRt2op(node *p)
{
   int temp;
   temp = lc[ (int) p->aval[1]];
   switch( (int) p->aval[0])
   {
      case SET:     fprintf(stderr,"SET %d %d\n",
                            (int)  p->aval[1],
                            (int)  p->aval[2]);
                    break;
      case MOD2:     fprintf(stderr,"MOD2 %d %d\n",
                            (int)  p->aval[1],
                            (int)  p->aval[2]);
                    temp %= 2;
                    break;
      case MOD4:     fprintf(stderr,"MOD4 %d %d\n",
                            (int)  p->aval[1],
                            (int)  p->aval[2]);
                    temp %= 4;
                    break;
      case HLV:     fprintf(stderr,"HLV %d %d\n",
                            (int)  p->aval[1],
                            (int)  p->aval[2]);
                    temp /= 2;
                    break;
      case DBL:     fprintf(stderr,"DBL %d %d\n",
                            (int)  p->aval[1],
                            (int)  p->aval[2]);
                    temp *= 2;
                    break;
      case NOT:     fprintf(stderr,"NOT %d %d\n",
                            (int)  p->aval[1],
                            (int)  p->aval[2]);
                    temp = ~temp;
                    break;
      case NEG:     fprintf(stderr,"NEG %d %d\n",
                            (int)  p->aval[1],
                            (int)  p->aval[2]);
                    temp = -temp;
                    break;
      default:      fprintf(stderr,"Unknown RT2OP cmd %d\n", (int) p->aval[0]);
                    break;
   }
   lc[ (int) p->aval[2]] = temp;
}

void doRt3op(node *p)
{
   int temp, arg1, arg2;
   arg1= lc[ (int) p->aval[1]];
   arg2= lc[ (int) p->aval[2]];
   switch( (int) p->aval[0])
   {
      case ADD:     fprintf(stderr,"ADD %d %d %d\n",
                            (int)  p->aval[1],
                            (int)  p->aval[2],
                            (int)  p->aval[3]);
                    temp = arg1 + arg2;
                    break;
      case SUB:
                    fprintf(stderr,"SUB %d %d %d\n",
                            (int)  p->aval[1],
                            (int)  p->aval[2],
                            (int)  p->aval[3]);
                    temp = arg1 - arg2;
                    break;
      case MUL:
                    fprintf(stderr,"MUL %d %d %d\n",
                            (int)  p->aval[1],
                            (int)  p->aval[2],
                            (int)  p->aval[3]);
                    temp = arg1 * arg2;
                    break;
      case DIV:
                    fprintf(stderr,"DIV %d %d %d\n",
                            (int)  p->aval[1],
                            (int)  p->aval[2],
                            (int)  p->aval[3]);
                    temp = arg1 / arg2;
                    break;
      case MOD:
                    fprintf(stderr,"MOD %d %d %d\n",
                            (int)  p->aval[1],
                            (int)  p->aval[2],
                            (int)  p->aval[3]);
                    temp = arg1 % arg2;
                    break;
      case OR:
                    fprintf(stderr,"OR %d %d %d\n",
                            (int)  p->aval[1],
                            (int)  p->aval[2],
                            (int)  p->aval[3]);
                    temp = arg1 | arg2;
                    break;
      default:      fprintf(stderr,"Unknown RT3OP cmd %d\n", (int) p->aval[0]);
                    temp = 0;
                    break;
   }
   lc[ (int) p->aval[3]] = temp;
}

void readTable(FILE *inFile, node *p)
{
   int i;
   int *ptr;
   int size = (int) p->aval[1];
   p->tbl = (int *) malloc(size *sizeof(int));
   ptr = p->tbl;
   for (i=0; i<size; i++)
   {
      char key[32], attr[32];

      getToken(inFile, key, attr);
      *ptr++ = atoi(attr);
   }
   ptr = p->tbl;
   for (i=0; i<size; i++)
   {
      fprintf(stderr,"Table[%d][%d]: %d\n", (int) p->aval[0], i, *ptr);
      ptr++;
   }
}

void doTable(node *p)
{
   table[(int) p->aval[0]- 1] = p;
   fprintf(stderr,"Table %d with %d elements\n", 
           (int) p->aval[0], (int) p->aval[1]);
}

void doTassign(node *p)
{
   node *p2tbl;
   int *val;
   int offset;

   fprintf(stderr,"doTassign index = %d\n",(int) p->aval[0]);
   p2tbl = table[(int) p->aval[0]];
   val = p2tbl->tbl;
   offset = lc[(int) p->aval[1]];

   fprintf(stderr,"Tassign Table[%d] with src %d and dest %d\n", 
           (int) p->aval[0], (int) p->aval[1], (int) p->aval[2]);
   fprintf(stderr,"table offset = %d\n", offset);
   p2tbl->aval[2]= p->aval[1];

   fprintf(stderr,"Tassign set lc[%d] to %d\n", 
      (int) p->aval[2],  *(val+ offset) );
   lc[(int) p->aval[2]] =  *(val+ offset);
   p2tbl->aval[3]= p->aval[2];
   fprintf(stderr,"Tassign set destptr to %d\n", 
      (int) p2tbl->aval[3]);
}

void doRtinit(node *p)
{
   fprintf(stderr,"RTINIT lc[%d] = %d\n", 
           (int) p->aval[1], (int) p->aval[0]);
   lc[ (int) p->aval[1]] = (int) p->aval[0];
}

void doFrtinit(node *p)
{
   fprintf(stderr,"FRTINIT lc[%d] = %d\n", 
           (int) p->aval[1], (int) p->aval[0]);
   lc[ (int) p->aval[1]] = (int) p->aval[0];
}

void execTree(node *p)
{
   while (p)
   {
      switch (p->flavour)
      {
         case DELAY: doDelay(p);
                     break;
         case PULSE: doPulse(p);
                     break;
         case SHAPEPULSE: doShapepulse(p);
                     break;
         case FREQ:  doFreq(p);
                     break;
         case INITSCAN:  doInitscan(p);
                     break;
         case NEXTSCAN:  doNextscan(p);
                     break;
         case ACQUIRE:  doAcquire(p);
                     break;
         case RTOP:  doRtop(p);
                     break;
         case RT2OP: doRt2op(p);
                     break;
         case RT3OP: doRt3op(p);
                     break;
         case TABLE: doTable(p);
                     break;
         case TASSIGN: doTassign(p);
                     break;
         case RTINIT: doRtinit(p);
                     break;
         case FRTINIT: doFrtinit(p);
                     break;
         default:    fprintf(stderr,"Unknown flavour %d\n", p->flavour);
                     break;
      }
      p = p->next;
   }
}

int getFlavour(char *key)
{
    if ( ! strcmp(key,"DELAY") )
       return(DELAY);
    else if ( ! strcmp(key,"PULSE") )
       return(PULSE);
    else if ( ! strcmp(key,"SHAPEPULSE") )
       return(SHAPEPULSE);
    else if ( ! strcmp(key,"INITSCAN") )
       return(INITSCAN);
    else if ( ! strcmp(key,"NEXTSCAN") )
       return(NEXTSCAN);
    else if ( ! strcmp(key,"ENDSCAN") )
       return(ENDSCAN);
    else if ( ! strcmp(key,"FREQ") )
       return(FREQ);
    else if ( ! strcmp(key,"ACQUIRE") )
       return(ACQUIRE);
    else if ( ! strcmp(key,"RTOP") )
       return(RTOP);
    else if ( ! strcmp(key,"RT2OP") )
       return(RT2OP);
    else if ( ! strcmp(key,"RT3OP") )
       return(RT3OP);
    else if ( ! strcmp(key,"TABLE") )
       return(TABLE);
    else if ( ! strcmp(key,"TASSIGN") )
       return(TASSIGN);
    else if ( ! strcmp(key,"RTINIT") )
       return(RTINIT);
    else if ( ! strcmp(key,"FRTINIT") )
       return(FRTINIT);
    return(0);
}

static void addNode(node **codeTree, node *newNode)
{
   if (*codeTree == NULL)
   {
      *codeTree = newNode;
   }
   else
   {
      node *tmpCodeTree;

      tmpCodeTree = *codeTree;
      while (tmpCodeTree->next != NULL)
         tmpCodeTree = tmpCodeTree->next;
      tmpCodeTree->next = newNode;
   }
}

static node *makeNode(int flavour, char *attr, FILE *inFile)
{
   node *p;


   if (flavour == 0)
      return(NULL);

   if ((p = (node *) calloc(1,sizeof(node))) == NULL)
   {  fprintf(stderr,"out of memory!\n");
      exit(1);
   }
   p->flavour         = flavour;
   p->tbl             = NULL;
   p->next            = NULL;
   p->stm1            = NULL;
   if (flavour == PULSE)
   {
      p->numVals = 5;
      for (int i=0; i<5; i++)
      {
         char key[32], attr[32];

         getToken(inFile, key, attr);
         p->aval[i]= atof(attr);
      }
   }
   else if (flavour == SHAPEPULSE)
   {
      p->numVals = 7;
      for (int i=0; i<7; i++)
      {
         char key[32], attr[32];

         getToken(inFile, key, attr);
         p->aval[i]= atof(attr);
      }
   }
   else if (flavour == FREQ)
   {
      p->numVals = 2;
      for (int i=0; i<2; i++)
      {
         char key[32], attr[32];

         getToken(inFile, key, attr);
         p->aval[i]= atof(attr);
      }
   }
   else if (flavour == ACQUIRE)
   {
      p->numVals = 3;
      for (int i=0; i<3; i++)
      {
         char key[32], attr[32];

         getToken(inFile, key, attr);
         p->aval[i]= atof(attr);
      }
   }
   else if (flavour == INITSCAN)
   {
      p->numVals = 3;
      for (int i=0; i<3; i++)
      {
         char key[32], attr[32];

         getToken(inFile, key, attr);
         p->aval[i]= atof(attr);
      }
   }
   else if (flavour == NEXTSCAN)
   {
      p->numVals = 0;
      while (flavour != ENDSCAN)
      {
         char key[32], attr[32];

         getToken(inFile, key, attr);
         flavour = getFlavour(key);
         if (flavour != ENDSCAN)
            addNode(&(p->stm1), makeNode(flavour, attr, inFile));
      }
   }
   else if ( (flavour == RTOP) ||
             (flavour == RT2OP) ||
             (flavour == RT3OP) )
   {
      char key[32], attr[32];
      p->numVals = 2;
      getToken(inFile, key, attr);
      p->aval[0]= getOpAction(attr);
      getToken(inFile, key, attr);
      p->aval[1]= atoi(attr);
      if ( flavour != RTOP )
      {
         p->numVals = 3;
         getToken(inFile, key, attr);
         p->aval[2]= atoi(attr);
         if (flavour == RT3OP)
         {
            p->numVals = 4;
            getToken(inFile, key, attr);
            p->aval[3]= atoi(attr);
         }
      }
   }
   else if ( (flavour == RTINIT) ||
             (flavour == FRTINIT) )
   {
      char key[32], attr[32];
      p->numVals = 2;
      getToken(inFile, key, attr);
      p->aval[0]= atoi(attr);
      getToken(inFile, key, attr);
      p->aval[1]= atoi(attr);
   }
   else if (flavour == TABLE)
   {
      char key[32], attr[32];
      p->numVals = 4;
      for (int i=0; i<4; i++)
      {
         getToken(inFile, key, attr);
         p->aval[i]= atoi(attr);
      }
      readTable(inFile, p);
   }
   else if (flavour == TASSIGN)
   {
      char key[32], attr[32];
      getToken(inFile, key, attr);
      p->numVals = 3;
      p->aval[0]= atoi(attr);
      getToken(inFile, key, attr);
      p->aval[1]= atoi(attr);
      getToken(inFile, key, attr);
      p->aval[2]= atoi(attr);
   }
   else if (attr[0] != '\0' )
   {
      p->numVals = 1;
      p->aval[0]= atof(attr);
   }
   return(p);
}

void clean(node *codeTree)
{
   node *tmp;

   while (codeTree)
   {
      if (codeTree->tbl)
         free(codeTree->tbl);
      if (codeTree->stm1)
         clean(codeTree->stm1);
      tmp = codeTree->next;
      free(codeTree);
      codeTree = tmp;
   }
}

int main(int argc, char *argv[])
{
   FILE *inFile;
   node *codeTree = NULL;
   char key[32], attr[32];

   if (argc != 2)
   {
      fprintf(stderr,"File name must be provided\n");
      exit(EXIT_FAILURE);
   }
   inFile = fopen(argv[1],"r");
   if (inFile == NULL)
   {
      fprintf(stderr,"File name %s does not exist\n", argv[1]);
      exit(EXIT_FAILURE);
   }
   while (getToken(inFile, key, attr))
   {
      addNode(&codeTree, makeNode(getFlavour(key), attr, inFile));
   }

   fclose(inFile);

#ifdef DEBUG
   showTree(0, codeTree);
#endif
   initParser();
   execTree(codeTree);
   clean(codeTree);
   exit(EXIT_SUCCESS);
}


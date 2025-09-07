


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PATTERNHEADER   0x30000000

typedef struct _ACODEHEADER_ {
    int compressed_size;
    int full_size;
    int key;
} AcodeHeader;

typedef struct _TABLEHEADER_ {
    int auto_inc_flag;
    int numberElements;
    int divn_factor;
    int autoIndex;
} TableHeader;

typedef struct _PATTERNHEADER_ { 
    int pattern_element_size;
    int numberElements;
} PatternHeader;

typedef struct _COMBOHEADER_ {
  int comboID_and_Number;
  int sizeInBytes;
  union {  AcodeHeader AC;
           TableHeader TBL; 
           PatternHeader PAT;
         } details ;
} ComboHeader ;

int main(int argc, char *argv[])
{
   int sfd;
   ssize_t num;
   ComboHeader header;
   unsigned int len;


   if (argc != 2)
   {
      fprintf(stderr,"File name must be provided\n");
      exit(EXIT_FAILURE);
   }
   sfd = open(argv[1],O_RDONLY);
   if (sfd == -1)
   {
      fprintf(stderr,"File name %s does not exist\n", argv[1]);
      exit(EXIT_FAILURE);
   }
   num = read(sfd, &header, sizeof(struct _COMBOHEADER_) );
   fprintf(stderr,"num read= %zd\n", num);
   fprintf(stderr,"header 0x%x\n", header.comboID_and_Number);
   fprintf(stderr,"shape length %d (0x%x)\n",
         header.sizeInBytes, header.sizeInBytes);
   fprintf(stderr,"pattern header 0x%x\n", PATTERNHEADER);
   fprintf(stderr,"sizeof int %zd\n", sizeof(int));
   lseek(sfd, header.sizeInBytes,  SEEK_CUR);
   num = read(sfd, &len, sizeof(int) );
   fprintf(stderr,"second read %u (0x%x)\n", len, len);


   exit(EXIT_SUCCESS);
}


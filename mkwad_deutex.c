#include <stdio.h>
#include <stdlib.h>

#include "deutex.h"
#include "ident.h"
#include "mkwad.h"

/*
**  copy data from WAD to file
*/
void WADRsaveEntry(struct WADINFO *info,Int16 n, const char *file)
{  Int32 wsize,sz=0;
   char  *buffer;
   Int32 start,size;
   FILE *fd;
   if(!(info->ok&WADR_READ)) printf("BUG: WadSE");
   if(n>=(info->ntry))printf("BUG: WadSE>");
   start = info->dir[n].start;
   size  = info->dir[n].size;
   fd=fopen(file,FOPEN_WB);
   if(fd==NULL) printf("ERROR: Can't open file %s",file);
   buffer = (char  *)malloc( MEMORYCACHE);
   WADRseek(info,start);
   for(wsize=0; wsize<size ;wsize+=sz)
   {  sz= (size-wsize>MEMORYCACHE)? MEMORYCACHE : size-wsize;
      WADRreadBytes(info,buffer,sz);
      if(fwrite(buffer,(size_t)sz,1,fd)!=1)
      { free(buffer);
        printf("ERROR: Can't write file %s",file);
      }
   }    /*declare in WAD directory*/
   free( buffer);
   fclose(fd);
}

/*
** copy lump from file into WAD
** returns size
*/
Int32 WADRwriteLump(struct WADINFO *info, const char *file)
{  Int32      size,sz=0;
   FILE      *fd;
   char  *data;
   WADRcheckWritePos(info);
   /*Look for entry in master directory */
   fd=fopen(file,FOPEN_RB);
   if(fd==NULL) printf("ERROR: Can't read file %s",file);
   data = (char  *)malloc( MEMORYCACHE);
   for(size=0;;)
   { sz = fread(data,1,(size_t)MEMORYCACHE,fd);
     if(sz<=0)break;
     size+=WADRwriteBlock(info,data,sz);
   }
   free( data);
   fclose(fd);
   return size;
}
Int32 WADRwriteWADentry(struct WADINFO *info,struct WADINFO *src,Int16 n)
{  if(n>(src->ntry)) printf("BUG: WadWW>");
   return WADRwriteWADbytes(info,src,src->dir[n].start,src->dir[n].size);
}

/*
** copy level parts
*/
void WADRwriteWADlevelParts(struct WADINFO *info,struct WADINFO *src,Int16 N,
    size_t nlumps)
{ Int32 start,size;
  Int16 n;

  for (n = N + 1; n < src->ntry && n < (Int16)(N + nlumps); n++)
  {
    WADRalign4 (info);
    start = WADRposition (info);
    size = WADRwriteWADentry (info, src, n);
    WADRdirAddEntry (info, start, size, src->dir[n].name);
  }
}

/*
** copy level from WAD
** try to match level name (multi-level)
** if level name not found, then take the first level...
*/
void WADRwriteWADlevel(struct WADINFO *info, const char *file, const char *level)
{ Int16 N,l;
  Int32 pos;
  /*char Level[8];*/
  struct WADINFO src;
  if(IDENTlevel(level)<0)printf("ERROR: Bad level name %s",level);
  src.ok=0;
  WADRopenR(&src,file);
  /*search for entry in level*/
  N=WADRfindEntry(&src,level);
  if(N<0) /* no? then search first level*/
  { for(N=0;;N++)
    { l=IDENTlevel(src.dir[N].name);
      if(l>=0)break;
      if(N>=src.ntry) printf("ERROR: No level in WAD %s",file);
    }
  }
  /*set level name*/
  WADRalign4(info);
  pos=WADRposition(info); /*BC++ 4.5 bug!*/
  WADRdirAddEntry(info,pos,0L,level);
  /* 9999 is a way of saying "copy all the lumps". The rationale
     is "let's assume the user knows what he/she is doing. If
     he/she wants us to include a 100-lump level, let's do it".

     On the other hand, this stance is in contradiction with using
     WADRfindEntry() (see above). This needs to be fixed.
     
     There are two choices :
     - make this function discriminating and prevent
       experimentation
     - make it dumb but allow one to put multi-level wads in
       levels/.

     My real motivation for doing things the way I did was that I
     didn't want to copy-paste from IDENTdirLevels() into
     WADRwriteWADlevelParts() (these things should be at a single
     place). */
  WADRwriteWADlevelParts(info,&src,N, 9999);
  WADRclose(&src);
}

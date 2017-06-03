/*
This file is part of DeuTex.

DeuTex incorporates code derived from DEU 5.21 that was put in the public
domain in 1994 by Raphaël Quinet and Brendon Wyber.

DeuTex is Copyright © 1994-1995 Olivier Montanuy,
          Copyright © 1999-2000 André Majorel.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this library; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307, USA.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/****MSDOS****/
#if DT_OS == 'd'
#  include <io.h>
/****OS/2****/
#elif DT_OS == 'o'
#  include <io.h>
#  if DT_CC != 'b'
#    define filelength _filelength
#  endif
/****UNIX****/
#else
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include "deutex.h"
#include "tools.h"
#include "mkwad.h"
#include "ident.h"
#include "wadio.h"

/*************Begin WAD module **********************
** Open PWAD file. *pcount=global byte counter
** leaves number of entries and directory pointer NULL
*/


/******************End WAD module ******************/

/************ begin   WAD module ***********/

void WADRopenPipo(struct WADINFO *info,Int32 ntry)
{  /*directory */
   if((info->ok&WADR_RDWR)) printf("BUG: WadPpk");
   info->ok=WADR_PIPO;
   if(ntry<=0)             printf("BUG: WadPpo");
   info->maxdir=ntry;
   info->dir=(struct WADDIR  *)malloc((info->maxdir)*sizeof(struct WADDIR));
   info->maxpos=ntry*sizeof(struct WADDIR);
   info->ntry=0;
   info->wposit=info->maxpos;
}

struct WADDIR  *WADRclosePipo(struct WADINFO *info,Int32  *ntry)
{  if((info->ok!=WADR_PIPO)) printf("BUG: WadPpc");
   info->ok=FALSE;
   if(info->ntry<0)info->ntry=0;
   info->dir=(struct WADDIR  *)
             realloc(info->dir,(info->ntry)*sizeof(struct WADDIR));
   *ntry=info->ntry;
   return info->dir;
}

Int32 WADRdirAddPipo(struct WADINFO *info,Int32 start,Int32 size, const char
    *entry)
{ Int16 n;
  if(info->ok!=WADR_PIPO) printf("BUG: WadDaP");
  n=(Int16)info->ntry; /*position of new entry*/
  if(n<0) printf("BUG: WadDa2");
  if(n<info->maxdir) /*can add to the dir*/
  { info->ntry++; /*new dir size*/
    info->dir[n].size=size;
    info->dir[n].start=start;
    Normalise(info->dir[n].name,entry);
  }
  return n; /* nb entries */
}

void WADRopenR(struct WADINFO *info, const char *wadin)
{  /*directory */
   Int32 ntry,dirpos;
   Int16 n;
   struct WADDIR dir;
   if((info->ok&WADR_RDWR)) printf("BUG: WadOpr");
   info->fd=fopen(wadin,FOPEN_RB);
   if((info->fd)==NULL)printf("ERROR: Can't open WAD %s for reading",wadin);
   info->filename = malloc(strlen (wadin) + 1);
   strcpy (info->filename, wadin);
   info->ok=WADR_READ;
   /*signature*/
   switch(WADRreadShort(info))
   { case PWAD: case IWAD: break;
     default:   printf("ERROR: WAD %s has a bad header",wadin);
   }
   if(WADRreadShort(info)!=WADMAGIC)
     printf("ERROR: WAD %s has a bad header2",wadin);
   /*start of directory*/
   ntry  = WADRreadLong(info);
   if(ntry<=0)
     printf("ERROR: WAD %s is empty",wadin);
   if(ntry>=0x2000)
     printf("ERROR: WAD has too many entries");
   info->dirpos= dirpos= WADRreadLong(info);
   if((dirpos<0)||(dirpos>0x10000000L))
     printf("ERROR: WAD dir references are incorrect");
   /*allocate directory*/
   info->maxdir=ntry;
   info->dir=(struct WADDIR  *)malloc((info->maxdir)*sizeof(struct WADDIR));
   /*read directory, calculate further byte in wad*/
   info->maxpos=dirpos+(ntry*sizeof(struct WADDIR));
   WADRseek(info,dirpos);
   info->ntry=0;
   for(n=0;n<ntry;n++)
   {
     if (wad_read_i32 (info->fd, &dir.start)
	  || wad_read_i32 (info->fd, &dir.size)
	  || wad_read_name (info->fd, dir.name))
       printf("ERROR: Error reading directory of \"%s\"", wadin);
     WADRdirAddEntry (info, dir.start, dir.size, dir.name);
   }
   if(info->ntry!=ntry)printf("BUG: WadOrN");
   info->wposit=info->maxpos;
   printf("PHASE: Reading WAD %s:\t(%d entries)\n",wadin,ntry);
}

static char signature[19 + 32];

void WADRopenW(struct WADINFO *info, const char *wadout,WADTYPE type,
    int verbose)
{  
   if (verbose)
     printf("PHASE: Creating %cWAD %s\n",(type==IWAD)?'I':'P',wadout);
   if((info->ok&WADR_RDWR)) printf("BUG: WadOpW");

   /*check file*/
   info->fd = fopen( wadout, FOPEN_RB);
   if(info->fd!=NULL) printf("ERROR: Won't overwrite existing file %s", wadout);
   /*open file*/
   info->fd = fopen( wadout, FOPEN_WB);
   if(info->fd==NULL)
    printf("ERROR: Can't create file %s (%s)", wadout, strerror (errno));
   info->filename = malloc(strlen (wadout) + 1);
   strcpy (info->filename, wadout);
   info->ok=WADR_WRITE;
   info->wposit=0;
   info->ntry  =0;
   info->maxdir=MAXPWADDIR;
   info->dir   =(struct WADDIR  *)malloc((info->maxdir)*sizeof(struct WADDIR));
   WADRwriteShort(info,type);   /* WAD type: PW or IW*/
   WADRwriteShort(info,WADMAGIC); /* WAD type: AD*/
   /* will be fixed when closing the WAD*/
   WADRwriteLong(info,-1);     /* no counter of dir entries */
   WADRwriteLong(info,-1);     /* no dir pointer */
   /* DeuTex notice*/
   sprintf(signature," DeuTex %.32s %cOJM 94 ", _VERSION_, 0xB8);
/********----**----**-********/
   WADRwriteBytes(info,signature,strlen(signature));
   WADRalign4(info);
}

/*
** Assumes file already opened write
** if not, open it first
** OPEN READ-WRITE,not APPEND
** because APPEND can't FSEEK to start of file
*/
void WADRopenA(struct WADINFO *info, const char *wadinout)
{  printf("PHASE: Modifying WAD %s\n",wadinout);
   if((info->ok&WADR_WRITE)) printf("BUG: WadOpA");
   if(!(info->ok&WADR_READ))
   { WADRopenR(info,wadinout);
   }
   /*reopen for append*/
   fclose(info->fd);
   info->fd = fopen( wadinout, FOPEN_RBP); /*rb+ = read/write binary*/
   if(info->fd==NULL)printf("ERROR: Can't append to file %s", wadinout);
   info->filename = malloc(strlen (wadinout) + 1);
   strcpy (info->filename, wadinout);
   info->ok = WADR_RDWR;
   WADRseek(info,info->wposit);
}



/***************** Directory ***************/
/*
** Add a new entry in the directory
** increase ntry, redim dir
** update maxdir and maxpos
** returns entry ref
*/
Int32 WADRdirAddEntry(struct WADINFO *info,Int32 start,Int32 size, const char
    *entry)
{ Int16 n;
  Int32 sz;
  if(!(info->ok&(WADR_RDWR))) printf("BUG: WadDAE");
  n=(Int16)info->ntry; /*position of new entry*/
  if(n>=info->maxdir) /*shall we move the dir?*/
  { info->maxdir+=MAXPWADDIR;
    info->dir=(struct WADDIR  *)realloc((char  *)info->dir,(info->maxdir)*sizeof(struct WADDIR));
  }
  info->ntry++; /*new dir size*/
  info->dir[n].size=size;
  info->dir[n].start=start;
  Normalise(info->dir[n].name,entry);
  sz=start+size;
  if(sz>info->maxpos)
    info->maxpos=sz;
  return n; /* nb entries */
}
/*
** write the directory (names, counts, lengths)
** then update the number of  entries and dir pointer
*/
void WADRwriteDir(struct WADINFO *info, int verbose)
{  Int16 n;
   if(!(info->ok&WADR_WRITE)) printf("BUG: WadWD");
   WADRalign4(info);      /*align entry on Int32 word*/
   info->dirpos=info->wposit; /*current position*/
   /* write the new WAD directory*/
   for(n=0;n<info->ntry;n++)
   {
     if (wad_write_i32  (info->fd, info->dir[n].start)
	 || wad_write_i32  (info->fd, info->dir[n].size)
	 || wad_write_name (info->fd, info->dir[n].name))
       printf("ERROR: Error writing wad directory");
   }
   /* fix up the directory start information */
   WADRsetDirRef(info,info->ntry,info->dirpos);
   n=(Int16)( info->dirpos+(sizeof(struct WADDIR)*info->ntry));
   if(n>info->maxpos) info->maxpos=n;
   if (verbose)
     printf("PHASE: WAD is complete: size %d bytes.\n",info->wposit);
}

/***************** Wad structure *******************/

void WADRsetDirRef(struct WADINFO *info,Int32 ntry,Int32 dirpos)
{
#if 0
  struct { Int32 ntry;Int32 dirpos;} Head;
#endif
   if(!(info->ok&WADR_WRITE))printf("BUG: WadSDR");
#if 0
   Head.ntry=BE_Int32(ntry);
   Head.dirpos=BE_Int32(dirpos);
#endif
   WADRseek(info,4);
   if (wad_write_i32 (info->fd, ntry)
       || wad_write_i32 (info->fd, dirpos))
     printf("ERROR: Error fixing wad header");
#if 0
   if(fwrite(&Head,sizeof(Head),1,info->fd)!=1)
   { printf("WARNING: That WAD might not be usable anymore!");
     printf("WARNING: Restore bytes 4 to 11 manually if you can.");
     printf("ERROR: Failed writing WAD directory References");
   }
#endif
   WADRseek(info,info->wposit);
   info->ntry=ntry;
   info->dirpos=dirpos;
}

void WADRchsize(struct WADINFO *info,Int32 fsize)
{ if(!(info->ok&WADR_WRITE)) printf("BUG: WadcSz");
  if(Chsize(fileno(info->fd),fsize)!=0)
    printf("ERROR: Can't change size of WAD.");
  info->maxpos=fsize;
  info->wposit=fsize;
}

#if 0
Bool WADRchsize2(struct WADINFO *info,Int32 fsize)
{ if(!(info->ok&WADR_WRITE)) printf("BUG: WadcSz");
  if(Chsize(fileno(info->fd),fsize)!=0) return FALSE;
  return TRUE;
}
#endif


/****************Read********************/

void WADRseek(struct WADINFO *info,Int32 position)
{
   long ofs;
   if(!(info->ok&WADR_RDWR)) printf("BUG: WadSk");
   ofs = ftell (info->fd);
   if(position>info->maxpos) printf("BUG: WadSk>");
   if(fseek(info->fd,position,SEEK_SET))
     printf("ERROR: %s: Can't seek to %06lXh",
	   fnameofs (info->filename, ofs), (unsigned long) position);
}

/*
 *	WADRreadBytes2
 *	Attempt to read up to <nbytes> bytes from wad <info>.
 *	Return the actual number of bytes read.
 */
iolen_t WADRreadBytes2 (struct WADINFO *info, char *buffer, iolen_t nbytes)
{
  size_t attempt = MEMORYCACHE;
  iolen_t bytes_read = 0;

  if (!(info->ok&WADR_READ))
    printf("BUG: WadRdB");

  while (nbytes > 0)
  {
    size_t result;
    if (attempt > nbytes)
      attempt = nbytes;
    result = fread (buffer, 1, attempt, info->fd);
    bytes_read += result;
    if (result == 0)  /* Hit EOF */
      break;
    buffer += result;
    nbytes -= result;
  }

  return bytes_read;
}

/*
 *	WADRreadBytes
 *	Attempt to read <nbytes> bytes from wad <info>. If EOF
 *	is hit too soon, trigger a fatal error. Else, return
 *	<nbytes>.
 */
iolen_t WADRreadBytes (struct WADINFO *info, char *buffer, iolen_t nbytes)
{
  long    ofs    = ftell (info->fd);
  iolen_t result = WADRreadBytes2 (info, buffer, nbytes);
  if (result != nbytes)
    printf("ERROR: %s: Can't read %lu bytes (%lu)",
	fnameofs (info->filename, ofs),
	(unsigned long) nbytes,
	(unsigned long) result);
  return nbytes;
}

Int16 WADRreadShort(struct WADINFO *info)
{ Int16 res;
  long ofs = ftell (info->fd);
  if (!(info->ok&WADR_READ)) printf("BUG: WadRdS");
  if (wad_read_i16 (info->fd, &res))
    printf("ERROR: %s: Can't read a short", fnameofs (info->filename, ofs));
   return res;
}

Int32 WADRreadLong(struct WADINFO *info)
{ Int32 res;
  long ofs = ftell (info->fd);
  if (!(info->ok&WADR_READ)) printf("BUG: WadRdL");
  if (wad_read_i32 (info->fd, &res))
    printf("ERROR: %s: Can't read a long", fnameofs (info->filename, ofs));
  return res;
}

void  WADRclose(struct WADINFO *info)
{  if(!(info->ok&WADR_RDWR)) printf("BUG: WadClo");
   info->ok=FALSE;
   free(info->filename);
   free(info->dir);
   fclose (info->fd);
}

Int16 WADRfindEntry(struct WADINFO *info, const char *entry)
{ Int16 i;
  static char name[8];
  struct WADDIR  *dir;
  if(!(info->ok&WADR_RDWR)) printf("BUG: WadFE");
  for(i=0,dir=info->dir;i<info->ntry;i++,dir+=1)
  { Normalise(name,dir->name);
    if(strncmp(name,entry,8)==0)
        return i;
  }
  return -1;
}

/*
** load data in buffer
*/
char  *WADRreadEntry(struct WADINFO *info,Int16 n,Int32 *psize)
{ char  *buffer;
  Int32 start,size;
  if(!(info->ok&WADR_READ)) printf("BUG: WadRE");
  if(n>=(info->ntry))printf("BUG: WadRE>");
  start = info->dir[n].start;
  size  = info->dir[n].size;
  buffer=(char  *)malloc(size);
  WADRseek(info,start);
  WADRreadBytes(info,buffer,size);
  *psize=size;
  return buffer;
}

/*
 *	WADRreadEntry2
 *	Read at most the *psize first bytes of a lump.
 *	If the lump is shorter than *psize, it's _not_ an error.
 *	Return the actual number of bytes read in *psize.
 */
char *WADRreadEntry2 (struct WADINFO *info, Int16 n, Int32 *psize)
{
  char *buffer;
  long   start;
  iolen_t size;
  iolen_t actual_size;

  if (! (info->ok & WADR_READ))
    printf("BUG: WadRE");
  if (n >= info->ntry)
    printf("BUG: WadRE>");
  start = info->dir[n].start;
  size  = *psize < info->dir[n].size ? *psize : info->dir[n].size;
  buffer = malloc(size);
  WADRseek (info, start);
  actual_size = WADRreadBytes2 (info, buffer, size);
  if (actual_size < size)
    printf("ERROR: Lump %s: unexpected EOF at byte %ld",
	lump_name (info->dir[n].name), (long) actual_size);
  *psize = actual_size;
  return buffer;
}

/******************** Write ************************/

void WADRsetLong(struct WADINFO *info, Int32 pos, Int32 val)
{
  if(!(info->ok&WADR_WRITE))		printf("BUG: WadStL");
  if(pos>(info->maxpos))		printf("BUG: WadSL>");
  if(fseek(info->fd, pos, SEEK_SET))
    printf("ERROR: %s: Can't seek to %06Xh", fname( info -> filename ), pos );
  if(wad_write_i32 (info->fd, val))
    printf("ERROR: %s: Can't write a long", fnameofs( info -> filename, pos ));
}

void WADRsetShort(struct WADINFO *info,Int32 pos,Int16 val)
{
  if(!(info->ok&WADR_WRITE))		printf("BUG: WadStS");
  if(pos>(info->maxpos))		printf("BUG: WadSS>");
  if(fseek(info->fd, pos, SEEK_SET))
    printf("ERROR: %s: Can't seek to %06Xh", fname( info -> filename ), pos);
  if(wad_write_i16 (info->fd, val))
    printf("ERROR: %s: Can't write a short", fnameofs( info -> filename, pos ));
}

/*
** internal functions... NO MORE!
**
*/
void WADRcheckWritePos(struct WADINFO *info)
{ if(!(info->ok&WADR_WRITE)) printf("BUG: WadCkW");
  if (fseek( info->fd, info->wposit, SEEK_SET))
    printf("ERROR: %s: Can't seek to %06Xh",
	fnameofs (info->filename, ftell (info->fd)), info->wposit);
}

Int32 WADRwriteBlock(struct WADINFO *info,char  *data,Int32 sz)
{ if(fwrite(data,(size_t)sz,1,info->fd) != 1)  printf("ERROR: Wad write failed");
  info->wposit += sz;
  if(info->maxpos<info->wposit)info->maxpos=info->wposit;
  return sz;
}

/*
** align, give position
*/
void WADRalign4(struct WADINFO *info)
{ Int16 remain;
  static char buffer[] ={0,0x24,0x6,0x68};
  WADRcheckWritePos(info);
  remain = (Int16)(info->wposit&0x03);   /*0 to 3*/
  if(remain>0) WADRwriteBytes(info,buffer,4-remain);
}
/*must be equal to ftell*/
Int32 WADRposition(struct WADINFO *info)
{ WADRcheckWritePos(info);
  return info->wposit;
}

/*
** write
*/
Int32 WADRwriteLong(struct WADINFO *info,Int32 val)
{
  WADRcheckWritePos(info);
  if (wad_write_i32 (info->fd, val))
    printf("ERROR: Wad write failed");
  info->wposit += sizeof val;
  if (info->maxpos < info->wposit)
    info->maxpos = info->wposit;
  return sizeof val;
}

Int32 WADRwriteShort(struct WADINFO *info,Int16 val)
{
  WADRcheckWritePos(info);
  if (wad_write_i16 (info->fd, val))
    printf("ERROR: Wad write failed");
  info->wposit += sizeof val;
  if (info->maxpos < info->wposit)
    info->maxpos = info->wposit;
  return sizeof val;
}

Int32 WADRwriteBytes(struct WADINFO *info,char  *data,Int32 size)
{ Int32 wsize,sz=0;
  WADRcheckWritePos(info);
  if(size<=0) printf("BUG: WadWb<");
  for(wsize=0;wsize<size;)
  { sz=(size-wsize>MEMORYCACHE)? MEMORYCACHE : size-wsize;
    wsize+=WADRwriteBlock(info,&data[wsize],sz);
  }
  return wsize;
}

static Int32 WADRwriteBlock2(struct WADINFO *info,char  *data,Int32 sz)
{ if(fwrite(data,(size_t)sz,1,info->fd) != 1)return -1;
  info->wposit += sz;
  if(info->maxpos<info->wposit)info->maxpos=info->wposit;
  return sz;
}

Int32 WADRwriteBytes2(struct WADINFO *info,char  *data,Int32 size)
{ Int32 wsize,sz=0;
  WADRcheckWritePos(info);
  if(size<=0) printf("BUG: WadWb<");
  for(wsize=0;wsize<size;)
  { sz=(size-wsize>MEMORYCACHE)? MEMORYCACHE : size-wsize;
    sz=WADRwriteBlock2(info,&data[wsize],sz);
    if(sz<0) return -1;
    wsize+=sz;
  }
  return wsize;
}

/*
**  copy data from SOURCE WAD to WAD
*/
Int32 WADRwriteWADbytes(struct WADINFO *info,struct WADINFO *src,Int32 start,Int32 size)
{  Int32 wsize,sz=0;
   char  *data;
   data = (char  *)malloc( MEMORYCACHE);
   WADRseek(src,start);
   WADRcheckWritePos(info);
   for(wsize=0; wsize<size;)
   {  sz= (size-wsize>MEMORYCACHE)? MEMORYCACHE : size-wsize;
      WADRreadBytes(src,data,sz);
      wsize+=WADRwriteBlock(info,data,sz);
   }    /*declare in WAD directory*/
   free( data);
   return wsize;
}

/*
** replace dir of rwad by dir of newwad
** prepare to write at the end of rwad
** open for append
*/
Int32 WADRprepareAppend(const char *wadres,struct WADINFO *rwad,
     struct WADDIR  *NewDir,Int32 NewNtry,
     Int32 *dirpos,Int32 *ntry, Int32 *size)
{ Int32 ewadstart;
  Int32 rwadsize;
  Int32 time;
  time=GetFileTime(wadres);
  /* append to the Result WAD*/
   WADRopenA(rwad,wadres);
  /*get original size*/
#if DT_OS == 'd'
#  if DT_CC == 'd'
   rwadsize=rwad->maxpos;
#  else
   rwadsize=filelength(fileno(rwad->fd));
#  endif
#elif DT_OS == 'o'
   rwadsize=filelength(fileno(rwad->fd));
#else
   rwadsize=rwad->maxpos;
#endif
   /*last warning*/
   printf("The WAD file %s will be modified, but it can be restored with:\n",wadres);
   printf("\"-res %s\" option\n", wadres);
   printf("Restoration may fail if you modified the WAD with another tool.\n");
   printf("In case of failure, you can salvage your WAD by:\n");
   /* Assuming wad is little endian... */
   printf("- setting bytes 4-7  to \t%02Xh %02Xh %02Xh %02Xh\n",
       (unsigned short) (rwad->ntry & 0xff),
       (unsigned short) ((rwad->ntry >> 8) & 0xff),
       (unsigned short) ((rwad->ntry >> 16) & 0xff),
       (unsigned short) ((rwad->ntry >> 24) & 0xff));
   printf("- and setting bytes 8-11 to \t%02Xh %02Xh %02Xh %02Xh\n",
       (unsigned short) (rwad->dirpos & 0xff),
       (unsigned short) ((rwad->dirpos >> 8) & 0xff),
       (unsigned short) ((rwad->dirpos >> 16) & 0xff),
       (unsigned short) ((rwad->dirpos >> 24) & 0xff));
   printf("If possible, set the file size to %d bytes.\n",rwadsize);
   /*align*/
   ewadstart=((rwadsize+0xF)&(~0xFL));
   if((ewadstart|rwadsize)&EXTERNAL) printf("ERROR: Too big WADs");
   *dirpos=rwad->dirpos;
   *ntry=rwad->ntry;
   *size=rwadsize;
   /*Change size*/
   WADRchsize(rwad,ewadstart);
   /*Write will start at file end*/
   rwad->maxpos=ewadstart;
   rwad->wposit=ewadstart;
   WADRseek(rwad,ewadstart);
   /*Change to New directory*/
   free(rwad->dir);
   rwad->dir=NewDir;
   rwad->ntry=NewNtry;
   rwad->maxdir=NewNtry;
   rwad->dirpos=-1;
   return time;
}


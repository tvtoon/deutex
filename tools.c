/*
This file is part of DeuTex.

DeuTex incorporates code derived from DEU 5.21 that was put in the public
domain in 1994 by Rapha�l Quinet and Brendon Wyber.

DeuTex is Copyright � 1994-1995 Olivier Montanuy,
          Copyright � 1999-2000 Andr� Majorel.

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

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
/*
** This code should contain all the tricky O/S related
** functions. If you're porting DeuTex/DeuSF, look here!
*/

/*MSDOS*/
#if DT_OS == 'd'
#  define SEPARATOR "\\"
#  if DT_CC == 'd'		/* DJGPP for DOS */
#    include <unistd.h>
#    include <malloc.h>
#    include <dos.h>
#    include <dir.h>
#    include <io.h>
#  elif DT_CC == 'b'		/* Borland C for DOS */
#    include <alloc.h>
#    include <dir.h>
#    include <io.h>
#  else				/* Other compiler (MSC ?) for DOS */
#    include <malloc.h>
#    include <direct.h>
#    include <io.h>
#  endif
/*OS/2*/
#elif DT_OS == 'o'
#  define SEPARATOR "\\"
#  include <malloc.h>
#  include <direct.h>
#  include <io.h>
/*UNIX*/
#else
#  define SEPARATOR "/"
#  include <unistd.h>
#  include <malloc.h>
#  include <memory.h>
#endif

#if DT_OS == 'o' && DT_CC == 'i'\
 || DT_OS == 'd' && DT_CC == 'm'
#  include <sys/utime.h>
#else
#  include <utime.h>
#endif

#include "deutex.h"
#include "tools.h"

#define SIZE_THRESHOLD        0x400L
#define SIZE_OF_BLOCK        0xFFFL

static const char hex_digit[16] = "0123456789ABCDEF";
/*
 *	check_types
 *	Sanity checks on the specified-width types.
 *	Will help catching porting errors.
 */
typedef struct
{
  const char *name;
  size_t mandated_size;
  size_t actual_size;
} type_check_t;

static const type_check_t type_checks[] =
{
  { "Int8",   1, sizeof (Int8)   },
  { "Int16",  2, sizeof (Int16)  },
  { "Int32",  4, sizeof (Int32)  },
  { "UInt8",  1, sizeof (UInt8)  },
  { "UInt16", 2, sizeof (UInt16) },
  { "UInt32", 4, sizeof (UInt32) }
};

void check_types (void)
{
 const type_check_t *t;
 printf("Inicializando a variável...\n");
 for (t = type_checks; (unsigned int)( t - type_checks ) < ( sizeof(type_checks) / sizeof(*t) ); t++)
{
/* FIXME Perhaps too strict. Wouldn't "<" suffice ? */
  if ( t -> actual_size != t -> mandated_size )
   printf ("Type %s has size %d (should be %d)."
    " Fix deutex.h and recompile.",
    t->name, (int)t -> actual_size, (int)t -> mandated_size);
  printf("Tamanho: %d.\n", (int)t -> actual_size);
}
}


/*
** Resize a file
** returns   0 if okay    -1 if failed.
*/
Int16 Chsize(int handle,Int32 newsize)
{
#if DT_OS == 'd'
#  if DT_CC == 'd'
  return (Int16)ftruncate(handle, newsize);
#  elif DT_CC == 'b' || DT_CC == 'm'
  return (Int16)chsize(handle,newsize);
#  else
#    error Chsize unimplemented
#  endif
#elif DT_OS == 'o'
#  if DT_CC == 'b'
   return (Int16)chsize(handle,newsize);
#  else
   return (Int16)_chsize(handle,newsize);
#  endif
#else
  return (Int16)ftruncate(handle, newsize);
#endif
}

/*
** Delete a file
*/
void Unlink(const char *file)
{  remove (file);
}

/*
** Get a file time stamp. (date of last modification)
*/
Int32 GetFileTime(const char *path)
{ Int32 time;
  struct stat statbuf;
  stat(path,&statbuf);
  time =statbuf.st_ctime;
  return time;
}
/*
** Set a file time stamp.
*/
void SetFileTime(const char *path, Int32 time)
{
  struct utimbuf stime;
  stime.modtime=stime.actime=time;
#if DT_OS == 'o' && DT_CC != 'b'
  _utime(path, &stime);
#else
  utime(path, &stime);
#endif
}
/*code derived from DEU*/
/* actually, this is (size - 1) */
/*****************************************************/
/*
** Use only lower case file names
*/
void ToLowerCase(char *file)
{ Int16 i;
  for(i=0;(i<128)&&(file[i]!='\0');i++)
         file[i]=tolower((((Int16)file[i])&0xFF));
}

static void NameDir(char file[128], const char *path, const char *dir, const
    char *sdir)
{
   file[0]='.';
   file[1]='\0';
   if(path!=NULL) if(strlen(path)>0){ strncpy(file,path,80);}
   if(dir!=NULL)  if(strlen(dir)>0)
   { strcat(file,SEPARATOR);strncat(file,dir,12);}
   if(sdir!=NULL) if(strlen(sdir)>0)
   { strcat(file,SEPARATOR);strncat(file,sdir,12);}
   ToLowerCase(file);
}
/*
** Create directory if it does not exists
*/
void MakeDir(char file[128], const char *path, const char *dir, const char
    *sdir)
{  NameDir(file,path,dir,sdir);
#if DT_OS == 'd'
#  if DT_CC == 'd'
   mkdir(file,0); /*2nd argument not used in DOS*/
#  elif DT_CC == 'b' || DT_CC == 'm'
   mkdir(file);
#  else
#    error MakeDir unimplemented
#  endif
#elif DT_OS == 'o'
#  if DT_CC == 'b'
   mkdir(file);
#  else
   _mkdir(file);
#  endif
#else
   mkdir(file,(mode_t)S_IRWXU); /*read write exec owner*/
#endif
}

/*
** Create a file name, by concatenation
** returns TRUE if file exists FALSE otherwise
*/
Bool MakeFileName(char file[128], const char *path, const char *dir, const
    char *sdir, const char *name, const char *extens)
{  FILE *fp;
   char name2[8];  /* AYM 1999-01-13: keep checker happy */
   /* deal with VILE strange name
   ** replace the VILE[ VILE\ VILE]
   ** by          VIL@A VIL@B VIL@C
   */
   Normalise(name2,name);

   /* FIXME AYM 1999-06-09: Not sure whether it is a good thing
      to keep this translation scheme for the Unix version.
      However, removing it would make the DOS version and the
      Unix version incompatible. */
   switch(name2[4])
   { case '[':  name2[4]='$';break;
     case '\\': name2[4]='@';break;
     case ']':  name2[4]='#';break;
   }
   switch(name2[6])
   { case '[':  name2[6]='$';break;
     case '\\': name2[6]='@';break;
     case ']':  name2[6]='#';break;
   }

   NameDir(file,path,dir,sdir);
   /*
   ** file name
   */
   strcat(file,SEPARATOR);
   strncat(file,name2,8);
   strcat(file,".");
   strncat(file,extens,4);
   ToLowerCase(file);
   /*
   ** check if file exists
   */
   fp=fopen(file,FOPEN_RB);
   if(fp!=NULL)
   { fclose(fp);  /* AYM 1999-01-??: fclose() used to be called even if
		     fopen() had returned NULL! It gave beautiful segfaults. */
     return TRUE;
   }
   return FALSE;
}
/*
** Get the root name of a WAD file
*/
void GetNameOfWAD(char name[8], const char *path)
{ Int16 n, nam,len;
  len=(Int16)strlen(path);
  /*find end of DOS or Unix path*/
  for(nam=n=0;n<len;n++)
    switch(path[n])
    {
      /* FIXME AYM 1999-06-09: I don't understand what "$" is
	 doing here. */
      /* FIXME AYM 1999-06-09: Is it really a good idea to
         consider "\" a path separator under Unix ? */
      case '\\': case '/': case '$':  nam=n+1;
    }
  /*find root name*/
  /* FIXME AYM 1999-06-09: Do we really have to truncate to 8 ? */
  for(n=0;n<8;n++)
    switch(path[nam+n])
    { case '.': case '\0': case '\n':
      name[n]='\0'; return;
      default:   name[n]=toupper(path[nam+n]); break;
    }
    return;
}

/*****************************************************/
/*****************************************************/

/* convert 8 byte string to upper case, 0 padded*/
void Normalise(char dest[8], const char *src)  /*strupr*/
{ Int16 n;Bool pad=FALSE; char c='A';
  for(n=0;n<8;n++)
  { c= (pad==TRUE)? '\0': src[n];
	 if(c=='\0')   pad=TRUE;
	 else c=(isprint(c))? toupper(c) : '*';
	 dest[n] = c;}
}


/*
** Output auxilliary functions
*/

/*
 *	fnameofs - return string containing file name and offset
 *
 *	Not reentrant (returns pointer on static buffer).
 *	FIXME: should encode non-printable characters.
 *	FIXME: should have shortening heuristic (E.G. print only basename).
 */
char *fnameofs (const char *name, long ofs)
{
  static char buf[81];
  *buf = '\0';
  strncat (buf, name, sizeof(buf) - 12);
  sprintf (buf + strlen (buf), "(%06lXh)", ofs);
  return buf;
}


/*
 *	fname - return string containing file name
 *
 *	Not reentrant (returns pointer on static buffer).
 *	FIXME: should encode non-printable characters.
 *	FIXME: should have shortening heuristic (E.G. print only basename).
 */
char *fname (const char *name)
{
  static char buf[81];
  *buf = '\0';
  strncat (buf, name, sizeof(buf) - 1);
  return buf;
}


/*
 *	lump_name - return string containing lump name
 *
 *	Partially reentrant (returns pointer on one of two
 *	static buffer). The string is guaranteed to have at most
 *	32 characters and to contain only graphic characters.
 */
char *lump_name (const char *name)
{
  static char buf1[9];
  static char buf2[9];
  static int  buf_toggle = 0;
  const char *const name_end = name + 8;
  char *buf = buf_toggle ? buf2 : buf1;
  char *p   = buf;

  buf_toggle = ! buf_toggle;
  if (*name == '\0')
    strcpy (buf, "(empty)");
  else
  {
    for (; *name != '\0' && name < name_end; name++)
    {
      if (isgraph ((unsigned char) *name))
	*p++ = toupper ((unsigned char) *name);
      else
      {
	*p++ = '\\';
	*p++ = 'x';
	*p++ = ((unsigned char) *name) >> 4;
	*p++ = *name & 0x0f;
      }
    }
    *p = '\0';
  }
  return buf;
}


/*
 *	short_dump - return string containing hex dump of buffer
 *
 *	Not reentrant (returns pointer on static buffer). Length
 *	is silently limited to 16 bytes.
 */
char *short_dump (const char *data, size_t size)
{
#define MAX_BYTES 16
  static char buf[3 * MAX_BYTES];
  char *b = buf;
  size_t n;

  for (n = 0; n < size && n < MAX_BYTES; n++)
  {
    if (n > 0)
      *b++ = ' ';
    *b++ = hex_digit[((unsigned char) data[n]) >> 4];
    *b++ = hex_digit[((unsigned char) data[n]) & 0x0f];
  }
  *b++ = '\0';
  return buf;
}


/*
** Output and Error handling
*/

#if 0
Int16 NbP=0;
void Progress(void)
{ NbP++;
  if(NbP&0xF==0) fprintf(Stdinfo,".");
  if(NbP>0x400)
  { NbP=0;
    fprintf(Stdinfo,"\n");
  }
}

void ProgressEnds(void)
{ fprintf(Stdinfo,"\n");
}
#endif

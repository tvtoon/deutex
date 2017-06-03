/*
  DeuTex is a sequel to the DOOM editor DEU 5.21 (hence the name)
  Originaly it used lots of code from DEU, and even some DEU modules
  Now it doesn't use anymore DEU modules, but large parts of this
  code are still inspired by the great DEU code, though the style
  is quite different.

  I wrote this code with the intent to limit bug generation and
  propagation to a minimum. That means modules, no global variables,
  a lot of mess in the parameters.
  I deliberately coded very basic functions, not optimised at all.
  Optimisation will come later.
  That means: No hash table, No fast color quantisation...

  If you find that code verbose, you are damn right.
  But I'm quite proud of my coding time/testing time ratio of 5,
  considering that code already works on so many PWADs.

  I would strongly advise *not* to reuse this code, unless you
  like my bugs and take the engagement to treat them well.
  I'm so fond of them now...  -- Olivier Montanuy
*/


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

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deutex.h"
#include "deuboth.h"
#include "tools.h"
#include "mkwad.h"
#include "extract.h"
#include "wadio.h"
#include "picture.h"
#include "usedidx.h"

/*compile only for DeuTex*/
static char WadInf[128];	/* name of the wadinfo file */
static Bool WadInfOk;
Int16 HowMuchJunk;		/* junk to add*/
static IMGTYPE Picture;		/* save as PPM, BMP or GIF ? */
static SNDTYPE Sound;		/* save as WAV, VOC or AU ? */
static Bool fullSND;
static Bool WSafe;
static Bool George;

extern NTRYB Select;
extern const char *DataDir;
extern const char *DoomDir;
extern char MainWAD[128];
extern Int16 HowMuchJunk;

extern char file[128];
extern char trnR,trnG,trnB;
extern rate_policy_t rate_policy;
extern char anon[1];
static WADTYPE Type;

void COMmanopt(int argc, char *argv[]);
void COMwintxn(int argc, char *argv[]);
void COMwintxm(int argc, char *argv[]);
void COMdoom02(int argc, char *argv[]);
void COMdoom04(int argc, char *argv[]);
void COMdoom05(int argc, char *argv[]);
void COMdoompr(int argc, char *argv[]);
void COMsprit(int argc, char *argv[]);
void COMflat(int argc, char *argv[]);
void COMlevel(int argc, char *argv[]);
void COMlump(int argc, char *argv[]);
void COMtextur(int argc, char *argv[]);
void COMsound(int argc, char *argv[]);
void COMmusic(int argc, char *argv[]);
void COMgraphic(int argc, char *argv[]);
void COMsneas(int argc, char *argv[]);
void COMsneaps(int argc, char *argv[]);
void COMsneats(int argc, char *argv[]);
void COMpatch(int argc, char *argv[]);
void COMgeorge(int argc, char *argv[]);
void COMdebug(int argc, char *argv[]);
void COMdi (int argc, char *argv[]);
void COMmusid (int argc, char *argv[]);
void COMdeu(int argc, char *argv[]);
void COMdir(int argc, char *argv[]);
void COMfullsnd(int argc, char *argv[]);
void COMrate(int argc, char *argv[]);
void COMstroy(int argc, char *argv[]);
void COMgif(int argc, char *argv[]);
void COMbmp(int argc, char *argv[]);
void COMppm(int argc, char *argv[]);
void COMau(int argc, char *argv[]);
void COMwave(int argc, char *argv[]);
void COMvoc(int argc, char *argv[]);
void COMrgb(int argc, char *argv[]);
void COMle (int argc, char *argv[]);
void COMbe (int argc, char *argv[]);
void COMile (int argc, char *argv[]);
void COMibe (int argc, char *argv[]);
void COMole (int argc, char *argv[]);
void COMobe (int argc, char *argv[]);
void COMipf (int argc, char *argv[]);
void COMitl (int argc, char *argv[]);
void COMiwad(int argc, char *argv[]);
void COMmake(int argc, char *argv[]);
void COMxtra(int argc, char *argv[]);
void COMget(int argc, char *argv[]);
void COMpackNorm(int argc, char *argv[]);
void COMpackGfx(int argc, char *argv[]);
void COMvoid(int argc, char *argv[]);
void COMusedtex(int argc, char *argv[]);
void COMusedidx(int argc, char *argv[]);

const char *Format[] =
{
 "* Format of PWAD creation directives *",
 "This format is conform to MS-Windows .INI Files.",
 "Sections are named [LEVELS] [LUMPS] [SOUNDS]",
 "[MUSICS] [TEXTURE1] [TEXTURE2] [GRAPHICS]",
 "[SPRITES] [PATCHES] [FLATS] [SNEAPS] [SNEATS]",
 "Entries have format:",
 "{name}= {filename} {offsetX} {offsetY}",
 "A '*' at the end of the definition means that the",
 "entry will be exactly the same as the previous one.",
 NULL
};

const comdef_t Com[] =
{
 {NIL,0,"?",        COMhelp,   NULL,   "print list of options"},
 {NIL,0,"h",        COMhelp,   NULL,   NULL},
 {NIL,0,"help",     COMhelp,   NULL,   NULL},
 {NIL,0,"-help",    COMhelp,   NULL,   NULL},
 {NIL,0,"man",      COMmanopt, NULL,   "print list of options in troff -man format"},
 {NIL,0,"-version", COMvers,   NULL,   "print version number and exit successfully"},
 {NIL,0,"syntax",   COMformat, NULL,   "print the syntax of wad creation directives"},
 {OPT,5,"win",      COMwintxn, "<doom> <data> <info> <select> <colour>","WinTex shortcut"},
 {OPT,2,"wim",      COMwintxm, "<doom> <select>","WinTex shortcut"},
 {CMD,0,"debug",    COMdebug,  NULL,   "Debug mode"},
 {OPT,1,"di",       COMdi,     "<name>", "Debug identification of entry"},
 {OPT,0,"musid",    COMmusid,  NULL,   "use old music identification method"},
 {OPT,0,"overwrite",COMstroy,  NULL,   "overwrite all"},
 {OPT,1,"wtx",      COMwintex, "<iwad>","WinTex shortcut"},
 {OPT,1,"doom",     COMdoom,   "<dir>","indicate the directory of Doom"},
 {OPT,1,"doom2",    COMdoom,   "<dir>","indicate the directory of Doom II"},
 {OPT,1,"doom02",   COMdoom02, "<dir>","indicate the directory of Doom alpha 0.2"},
 {OPT,1,"doom04",   COMdoom04, "<dir>","indicate the directory of Doom alpha 0.4"},
 {OPT,1,"doom05",   COMdoom05, "<dir>","indicate the directory of Doom alpha 0.5"},
 {OPT,1,"doompr",   COMdoompr, "<dir>","indicate the directory of Doom PR pre-beta"},
 {OPT,1,"heretic",  COMdoom,   "<dir>","indicate the directory of Heretic"},
 {OPT,1,"hexen",    COMdoom,   "<dir>","indicate the directory of Hexen"},
 {OPT,1,"strife",   COMstrife, "<dir>","indicate the directory of Strife"},
 {OPT,1,"strife10", COMdoom,   "<dir>","indicate the directory of Strife 1.0"},
 {OPT,1,"dir",      COMdir,    "<dir>","indicate the working directory"},
 {OPT,0,"deu",      COMdeu,    NULL,   "add 64k of junk to workaround a bug of DEU 5.21"},

 {OPT,3,"rgb",      COMrgb,    "<r> <g> <b>","specify the transparent colour (default 0/47/47)"},
 {OPT,0,"gif",      COMgif,    NULL,   "save pictures as GIF (.gif)"},
 {OPT,0,"bmp",      COMbmp,    NULL,   "save pictures as BMP (.bmp)"},
 {OPT,0,"ppm",      COMppm,    NULL,   "save pictures as rawbits (P6) PPM (.ppm)"},
 {OPT,0,"au",       COMau,     NULL,   "save sound as Sun audio (.au)"},
 {OPT,0,"wav",      COMwave,   NULL,   "save sound as wave (.wav)"},
 {OPT,0,"voc",      COMvoc,    NULL,   "save sound as voc (.voc)"},
 {OPT,0,"fullsnd",  COMfullsnd,NULL,   "save sound lumps beyond declared length"},
 {OPT,1,"rate",     COMrate,	"<code>",	"policy for != 11025 Hz (\1reject\3, \1force\3, *\1warn\3, \1accept\3)"},
 {OPT,0,"levels",   COMlevel,  NULL,   "select levels"},
 {OPT,0,"lumps",    COMlump ,  NULL,   "select lumps"},
 {OPT,0,"sounds",   COMsound,  NULL,   "select sounds"},
 {OPT,0,"musics",   COMmusic,  NULL,   "select musics"},
 {OPT,0,"textures", COMtextur, NULL,   "select textures"},
 {OPT,0,"graphics", COMgraphic,NULL,   "select graphics"},
 {OPT,0,"sneas",    COMsneas,  NULL,   "select sneas (sneaps and sneats)"},
 {OPT,0,"sneaps",   COMsneaps, NULL,   "select sneaps"},
 {OPT,0,"sneats",   COMsneats, NULL,   "select sneats"},
 /*by request from George Hamlin*/
 {OPT,0,"s_end",    COMgeorge, NULL,   "use S_END for sprites, not SS_END"},
 {OPT,0,"george",   COMgeorge, NULL,   NULL},
 {OPT,0,"sprites",  COMsprit,  NULL,   "select sprites"},
 {OPT,0,"patches",  COMpatch,  NULL,   "select patches"},
 {OPT,0,"flats",    COMflat,   NULL,   "select flats"},
 {OPT,0,"v0",       COMverbose,NULL,   "set verbosity level to 0"},
 {OPT,0,"v1",       COMverbose,NULL,   "set verbosity level to 1"},
 {OPT,0,"v2",       COMverbose,NULL,   "set verbosity level to 2 (default)"},
 {OPT,0,"v3",       COMverbose,NULL,   "set verbosity level to 3"},
 {OPT,0,"v4",       COMverbose,NULL,   "set verbosity level to 4"},
 {OPT,0,"v5",       COMverbose,NULL,   "set verbosity level to 5"},
 {OPT,0,"be",       COMbe,     NULL,   "assume all wads are big endian (default LE)"},
 {OPT,0,"le",       COMle,     NULL,   "assume all wads are little endian (default)"},
 {OPT,0,"ibe",      COMibe,    NULL,   "input wads are big endian (default LE)"},
 {OPT,0,"ile",      COMile,    NULL,   "input wads are little endian (default)"},
 {OPT,0,"obe",      COMobe,    NULL,   "create big endian wads (default LE)"},
 {OPT,0,"ole",      COMole,    NULL,   "create little endian wads (default)"},
 {OPT,1,"ipf",      COMipf,    "{alpha|normal|pr}", "picture format (default is \"normal\")"},
 {OPT,1,"tf",       COMtf,     "{nameless|none|normalstrife11}", "texture format (default is \"normal\")"},
 {OPT,1,"itf",      COMtf,     "{nameless|none|normalstrife11}", "input texture format (default is \"normal\")"},
 {OPT,1,"otf",      COMtf,     "{nameless|none|normalstrife11}", "output texture format (default is \"normal\")"},
 {OPT,1,"itl",      COMitl,    "{none|normal|textures}", "texture lump (default is \"normal\")"},

 {OPT,0,"iwad",     COMiwad,   NULL,   "compose iwad, not pwad"},

 {CMD,1,"make",     COMmake,   "[<dirctivs.txt>] <out.wad>","make a pwad, from the creation directives"},
 {CMD,1,"build",    COMmake,   "[<dirctivs.txt>] <out.wad>",NULL},
 {CMD,1,"create",   COMmake,   "[<dirctivs.txt>] <out.wad>",NULL},

 {CMD,0,"wadir",    COMwadir,  "<in.wad>","list and identify entries in a wad"},
 {CMD,1,"check",    COMcheck,  "<in.wad>","check the textures"},
 {CMD,1,"test",     COMcheck,  "<in.wad>",NULL},
 {CMD,0,"usedtex",  COMusedtex,"[<in.wad>]","list textures used in all levels"},
 {CMD,0,"usedidx",  COMusedidx,"[<in.wad>]","colour index usage statistics"},
 {NIL,1,"unused",   COMvoid,   "<in.wad>","find unused spaces in a wad"},
 {CMD,0,"xtract",   COMxtra,   "[<in.wad> [<dirctivs.txt>]]","extract some/all entries from a wad"},
 {CMD,0,"extract",  COMxtra,   "[<in.wad> [<dirctivs.txt>]]",NULL},
 {CMD,1,"get",      COMget,    "<entry> [<in.wad>]","get a wad entry from main wad or in.wad"},
 {CMD,1,"pknormal", COMpackNorm,"[<in.wad> [<out.txt>]]","Detect identical normal"},
 {CMD,1,"pkgfx",    COMpackGfx,"[<in.wad> [<out.txt>]]","Detect identical graphics"},

 {CMD,2,"add",      COMadd,    "<incomplete.wad> <out.wad>","add sprites & flats of a pwad to those of Doom"},
 {CMD,1,"append",   COMapp,    "<incomplete.wad>","append sprites & flats of Doom to a pwad"},
 {CMD,2,"join",     COMjoin,   "<incomplete.wad> <in.wad>","append sprites & flats of Doom to a pwad"},
 {CMD,1,"merge",    COMmerge,  "<in.wad>","merge doom.wad and a pwad"},
 {CMD,0,"restore",  COMrestor, NULL,   "restore doom.wad and the pwad"},
 {CMD,1,"as",       COMapps,   "<sprite.wad>","append all sprites to the wad"},
 {CMD,1,"af",       COMappf,   "<flats.wad>","append all floors/ceilings to the wad"},
 {END,0,"",         COMhelp,   NULL,   ""}
};


/*
 *	COMmanopt
 *	List options like COMhelp but in troff -man source format.
 */
static char stack[10];

static void spush (char c)
{
  if (strlen (stack) + 1 >= sizeof stack)
    return;
  stack[strlen (stack) + 1] = '\0';
  stack[strlen (stack)] = c;
}


#if 0
static char speektop (void)
{
  if (! *stack)
    return '\0';
  return stack[strlen (stack) - 1];
}
#endif


static char spop (void)
{
  char c;
  if (! *stack)
    return '\0';
  c = stack[strlen (stack) - 1];
  stack[strlen (stack) - 1] = '\0';
  return c;
}


static char last_font;
static char troff_stack[10];

static void troff_start (void)
{
  last_font = '\0';
  *troff_stack = '\0';
}


static void troff_end (void)
{
  if (last_font)
    fputs ("\\fP", stdout);
  last_font = '\0';
}


static void troff_push (char font)
{
  if (strlen (troff_stack) + 1 >= sizeof troff_stack)
    return;
  troff_stack[strlen (troff_stack) + 1] = '\0';
  troff_stack[strlen (troff_stack)] = font;
}

 
static void troff_pop (void)
{
  if (! *troff_stack)
    return;
  troff_stack[strlen (troff_stack) - 1] = '\0';
}


static char troff_tos (void)
{
  if (! *troff_stack)
    return '\0';
  return troff_stack[strlen (troff_stack) - 1];
}


static void troff_putc (char c/*, char font*/)
{
  if (troff_tos () && troff_tos () != last_font)
  {
    if (last_font)
      fputs ("\\fP", stdout);
    fprintf (stdout, "\\f%c", troff_tos ());
    last_font = troff_tos ();
  }
  fputc (c, stdout);
}


static void troff_puts (const char *s/*, char font*/)
{
  if (troff_tos () && troff_tos () != last_font)
  {
    if (last_font)
      fputs ("\\fP", stdout);
    fprintf (stdout, "\\f%c", troff_tos ());
    last_font = troff_tos ();
  }
  fputs (s, stdout);
}


void COMmanopt(int argc, char *argv[])
{ const comdef_t *d;
  for (d = Com; d->type != END; d++)
  {
    const char *p = d->use;
    *stack = '\0';
    troff_start ();
    troff_puts (".TP 14\n");

    /* Print the option name in bold */
    troff_push ('B');
    troff_puts ("\\-");
    troff_puts (d->com);
    troff_pop ();
    if (p)
      troff_putc (' ');

    /* Print the arguments. What's between < > is in italic,
       [ ] { | } are in normal roman, the rest is in bold. */
    if (p)
      while (*p)			/* Print one arg. for each iteration */
      {
	if (isspace (*p))
	  troff_putc (' ');
	while (isspace (*p))
	  p++;
	troff_push ('B');
	for (; *p && *p != ' '; p++)
	{
	  if (*p == '[' || *p == '{')
	  {
	    spush (*p);
	    troff_push ('R');
	    troff_putc (*p);
	    troff_pop ();
	  }
	  else if (*p == '<')
	  {
	    spush (*p);
	    troff_push ('I');
	  }
	  else if (*p == ']' || *p == '}')
	  {
	    spop ();
	    troff_push ('R');
	    troff_putc (*p);
	    troff_pop ();
	  }
	  else if (*p == '>')
	  {
	    spop ();
	    troff_pop ();
	  }
	  else if (*p == '|')
	  {
	    troff_push ('R');
	    troff_putc ('|');
	    troff_pop ();
	  }
	  else
	    troff_putc (*p);
	}
	troff_pop ();
      }
    troff_end ();
    troff_putc ('\n');

    /* Print the description */
    troff_start ();
    if (d->help == NULL)
    {
      if (d > Com && d->exec == d[-1].exec)
	troff_puts ("Same as above.\n");
      else
	troff_puts ("(no description)\n");
    }
    else
    {
      if (d->type == OPT)
	troff_puts ("(opt.) ");
      troff_putc (toupper (*d->help));
      troff_puts (d->help + 1);
      troff_putc ('.');
      troff_putc ('\n');
    }
    troff_end ();
  }
  (void)argc;(void)argv;
}

/*
** WinTex comand line replaces
** -out
** -v3
** -doom       doom directory
** -dir        data directory
** -sel        select byte
** -colRrGgBb  red = (R-'A')*16+(r-'A')
*/
void COMwintxn(int argc, char *argv[])
{  const char *num;
   DoomDir=argv[1];                /*doom*/
   DataDir=argv[2];                /*data*/
   MakeFileName(WadInf,DataDir,"","",argv[3],"TXT");
   WadInfOk=TRUE;
   Select|= atoi(argv[4]);        /*select*/
   num = argv[5];                /*colour*/
   if(strlen(num)<6) printf("ERROR: Bad Color");
   trnR= (((num[0]&0xF)<<4)+(num[1]&0xF))&0xFF;
   trnG= (((num[2]&0xF)<<4)+(num[3]&0xF))&0xFF;
   trnB= (((num[4]&0xF)<<4)+(num[5]&0xF))&0xFF;
   printf("INFO: Transparent colour is R=%d G=%d B=%d\n",
    ((int)trnR&0xFF),((int)trnG&0xFF),((int)trnB&0xFF));
   (void)argc;
}

void COMwintxm(int argc, char *argv[])
{
   DoomDir=argv[1];                /*doom*/
   DataDir=".";
   Select|= atoi(argv[2]);        /*select*/
   trnR= 0;trnG= 255;trnB= 255;
   (void)argc;
}

void COMdoom02(int argc, char *argv[])
{  call_opt (COMdoom, anon, argv[1], NULL);
   call_opt (COMipf,  anon, "alpha", NULL);
   call_opt (COMtf,   "tf", "none",  NULL);
   call_opt (COMitl,  anon, "none",  NULL);
   (void)argc;
}

void COMdoom04(int argc, char *argv[])
{  call_opt (COMdoom, anon, argv[1],    NULL);
   call_opt (COMipf,  anon, "alpha",    NULL);
   call_opt (COMtf,   "tf", "nameless", NULL);
   call_opt (COMitl,  anon, "textures", NULL);
   (void)argc;
}

void COMdoom05(int argc, char *argv[])
{  call_opt (COMdoom, anon, argv[1],    NULL);
   call_opt (COMipf,  anon, "alpha",    NULL);
   call_opt (COMitl,  anon, "textures", NULL);
   (void)argc;
}

void COMdoompr(int argc, char *argv[])
{  call_opt (COMdoom, anon, argv[1],  NULL);
   call_opt (COMipf,  anon, "pr",     NULL);
   (void)argc;
}


/*
** Selections
*/
void COMsprit(int argc, char *argv[])
{ Select|=BSPRITE;
  printf("INFO: Select sprites\n");
  (void)argc;(void)argv;
}

void COMflat(int argc, char *argv[])
{ Select|=BFLAT;
  printf("INFO: Select flats\n");
  (void)argc;(void)argv;
}

void COMlevel(int argc, char *argv[])
{ Select|=BLEVEL;
  printf("INFO: Select levels\n");
  (void)argc;(void)argv;
}

void COMlump(int argc, char *argv[])
{ Select|=BLUMP;
  printf("INFO: Select lumps\n");
  (void)argc;(void)argv;
}

void COMtextur(int argc, char *argv[])
{ Select|=BTEXTUR;
  printf("INFO: Select textures\n");
  (void)argc;(void)argv;
}

void COMsound(int argc, char *argv[])
{ Select|=BSOUND;
  printf("INFO: Select sounds\n");
  (void)argc;(void)argv;
}

void COMmusic(int argc, char *argv[])
{ Select|=BMUSIC;
  printf("INFO: Select musics\n");
  (void)argc;(void)argv;
}

void COMgraphic(int argc, char *argv[])
{ Select|=BGRAPHIC;
  printf("INFO: Select graphics\n");
  (void)argc;(void)argv;
}

void COMsneas(int argc, char *argv[])
{ Select|=BSNEA;
  printf("INFO: Select sneas\n");
  (void)argc;(void)argv;
}

void COMsneaps(int argc, char *argv[])
{ Select|=BSNEAP;
  printf("INFO: Select sneaps\n");
  (void)argc;(void)argv;
}

void COMsneats(int argc, char *argv[])
{ Select|=BSNEAT;
  printf("INFO: Select sneats\n");
  (void)argc;(void)argv;
}

void COMpatch(int argc, char *argv[])
{ Select|=BPATCH;
  printf("INFO: Select patches\n");
  (void)argc;(void)argv;
}

void COMgeorge(int argc, char *argv[])
{ George=TRUE;
  printf("INFO: Use S_END for sprites.\n");
  (void)argc;(void)argv;
}


void PicDebug(char *file, const char *DataDir, const char *name);


void COMdebug(int argc, char *argv[])
{
#if 1
#include "color.h"
  static struct WADINFO iwad;
  Int16 pnm;
  char  *Colors; Int32 Pnamsz=0;
  iwad.ok=0;
  WADRopenR(&iwad,MainWAD);
  pnm=WADRfindEntry(&iwad,"PLAYPAL");
  if(pnm<0) printf("ERROR: Can't find PLAYPAL in Main WAD");
  Colors=WADRreadEntry(&iwad,pnm,&Pnamsz);
  COLinit(trnR,trnG,trnB,Colors,(Int16)Pnamsz);
  free(Colors);
  WADRclose(&iwad);
  PicDebug(file,DataDir,(argc<2)? "test":argv[1]);
  COLfree();
#endif
  (void)argc;(void)argv;
}

void COMdi (int argc, char *argv[])
{
  printf("INFO: Debugging identification of entry %s\n", lump_name (argv[1]));
  debug_ident = argv[1];
  (void) argc;
}

void COMmusid (int argc, char *argv[])
{
  printf("INFO: Using old music identification method.\n");
  old_music_ident_method = 1;
  (void) argc;
  (void) argv;
}

void COMdeu(int argc, char *argv[])
{ HowMuchJunk=MAXJUNK64;
  printf("INFO: Some junk will be added at end of WAD, for DEU 5.21.\n");
  (void)argc;(void)argv;
}

void COMdir(int argc, char *argv[])
{ DataDir=argv[1];
  printf("INFO: Files will be saved in directory %s.\n",DataDir);
  (void)argc;
}

void COMfullsnd(int argc, char *argv[])
{ fullSND=TRUE;
  printf("INFO: Saving complete sounds.\n");
  (void)argc;(void)argv;
}

void COMrate(int argc, char *argv[])
{ 
  if (argc >= 2 && ! strcmp (argv[1], "reject")) rate_policy = RP_REJECT;
  else if (argc >= 2 && ! strcmp (argv[1], "force")) rate_policy = RP_FORCE;
  else if (argc >= 2 && ! strcmp (argv[1], "warn")) rate_policy = RP_WARN;
  else if (argc >= 2 && ! strcmp (argv[1], "accept")) rate_policy = RP_ACCEPT;
  else printf("Usage is \"-rate {reject|force|warn|accept}\"");
  printf("Sample rate policy is \"%s\"", argv[1]);
  (void)argc; 
}

void COMstroy(int argc, char *argv[])
{ WSafe=FALSE;
  printf("INFO: Overwrite existing files.\n");
  if(argc)(void)argv;
}

void COMgif(int argc, char *argv[])
{ Picture=PICGIF;
  printf("INFO: Saving pictures as GIF (.gif)\n");
  (void)argc;(void)argv;
}

void COMbmp(int argc, char *argv[])
{ Picture=PICBMP;
  printf("INFO: Saving pictures as BMP (.bmp)\n");
  (void)argc;(void)argv;
}

void COMppm(int argc, char *argv[])
{ Picture=PICPPM;
  printf("INFO: Saving pictures as rawbits (P6) PPM (.ppm)\n");
  (void)argc;(void)argv;
}

void COMau(int argc, char *argv[])
{ Sound=SNDAU;
  printf("INFO: Save sounds as Sun audio (.au)\n");
  (void)argc;(void)argv;
}

void COMwave(int argc, char *argv[])
{ Sound=SNDWAV;
  printf("INFO: Save sounds as wave (.wav)\n");
  (void)argc;(void)argv;
}

void COMvoc(int argc, char *argv[])
{ Sound=SNDVOC;
  printf("INFO: Save sounds as voc (.voc)\n");
  (void)argc;(void)argv;
}

void COMrgb(int argc, char *argv[])
{ trnR=(char)(atoi(argv[1])&0xFF);
  trnG=(char)(atoi(argv[2])&0xFF);
  trnB=(char)(atoi(argv[3])&0xFF);
  printf("INFO: Transparent colour is R=%d G=%d B=%d\n",
  ((int)trnR&0xFF),((int)trnG&0xFF),((int)trnB&0xFF));
  (void)argc;
}

void COMle (int argc, char *argv[])
{
  set_input_wad_endianness (0);
  set_output_wad_endianness (0);
  (void) argc;
  (void) argv;
}

void COMbe (int argc, char *argv[])
{
  set_input_wad_endianness (1);
  set_output_wad_endianness (1);
  (void) argc;
  (void) argv;
}

void COMile (int argc, char *argv[])
{
  set_input_wad_endianness (0);
  (void) argc;
  (void) argv;
}

void COMibe (int argc, char *argv[])
{
  set_input_wad_endianness (1);
  (void) argc;
  (void) argv;
}

void COMole (int argc, char *argv[])
{
  set_output_wad_endianness (0);
  (void) argc;
  (void) argv;
}

void COMobe (int argc, char *argv[])
{
  set_output_wad_endianness (1);
  (void) argc;
  (void) argv;
}

void COMipf (int argc, char *argv[])
{
  if (argc >= 2 && ! strcmp (argv[1], "alpha"))
    picture_format = PF_ALPHA;
  else if (argc >= 2 && ! strcmp (argv[1], "pr"))
    picture_format = PF_PR;
  else if (argc >= 2 && ! strcmp (argv[1], "normal"))
    picture_format = PF_NORMAL;
  else
    printf("ERROR: Usage is \"-ipf {alpha|pr|normal}\"");
  printf("INFO: Input picture format is \"%s\"\n", argv[1]);
}

void COMitl (int argc, char *argv[])
{
  if (argc >= 2 && ! strcmp (argv[1], "none"))
    texture_lump = TL_NONE;
  else if (argc >= 2 && ! strcmp (argv[1], "textures"))
    texture_lump = TL_TEXTURES;
  else if (argc >= 2 && ! strcmp (argv[1], "normal"))
    texture_lump = TL_NORMAL;
  else
    printf("ERROR: Usage is \"-itl {none|textures|normal}\"");
  printf("INFO: Input texture lump is \"%s\"\n", argv[1]);
}

/*
** Build an IWAD
**
*/
void COMiwad(int argc, char *argv[])
{ Type=IWAD;
  printf("INFO: Build an IWAD file.\n");
  (void)argc;(void)argv;
}
/*
** Main Commands
**
*/

void COMmake(int argc, char *argv[])
{ const char *wadinf,*wadout;
  if(WadInfOk==FALSE)
  { MakeFileName(WadInf,DataDir,"","","WADINFO","TXT");
  }
  if(argc<=2){wadinf=WadInf;wadout=argv[1];}
  else       {wadinf=argv[1];wadout=argv[2];}
  CMPOmakePWAD(MainWAD,Type,wadout,DataDir,wadinf,Select,trnR,trnG,trnB,George);
  (void)argc;
}

void COMxtra(int argc, char *argv[])
{ const char *wadinf, *wadin;
  if(WadInfOk==FALSE)
  { MakeFileName(WadInf,DataDir,"","","WADINFO","TXT");
  }
  if(argc<=1){wadin=MainWAD;}else{wadin=argv[1];}
  if(argc<=2){wadinf=WadInf;}else{wadinf=argv[2];}
  XTRextractWAD (MainWAD, DataDir, wadin, wadinf, Picture, Sound, fullSND,
      Select, trnR, trnG, trnB, WSafe, NULL);
}

void COMget(int argc, char *argv[])
{ XTRgetEntry(MainWAD,DataDir,((argc<3)? MainWAD: argv[2]),argv[1],Picture,Sound,fullSND,trnR,trnG,trnB);
}

void COMpackNorm(int argc, char *argv[])
{ XTRcompakWAD(DataDir,(argc>1)? argv[1]: MainWAD,(argc>2)? argv[2]:NULL,FALSE, file );
}

void COMpackGfx(int argc, char *argv[])
{ XTRcompakWAD(DataDir,(argc>1)? argv[1]: MainWAD,(argc>2)? argv[2]:NULL,TRUE, file );
}

void COMvoid(int argc, char *argv[])
{  XTRvoidSpacesInWAD(argv[1]);
	(void)argc;
}

void COMusedtex(int argc, char *argv[])
{ XTRtextureUsed((argc>1)? argv[1]: MainWAD);
}

void COMusedidx(int argc, char *argv[])
{ const char *wadinf, *wadin;
  cusage_t *cusage = NULL;
  if(WadInfOk==FALSE)
  { MakeFileName(WadInf,DataDir,"","","WADINFO","TXT");  /* Not used anyway */
  }
  if(argc<=1){wadin=MainWAD;}else{wadin=argv[1];}
  if(argc<=2){wadinf=WadInf;}else{wadinf=argv[2];}
  cusage = malloc(sizeof *cusage);
  {
    int n;
    for (n = 0; n < NCOLOURS; n++)
    {
      cusage->uses[n] = 0;
      cusage->nlumps[n] = 0;
      cusage->where_first[n][0] = '\0';
    }
  }
  XTRextractWAD (MainWAD, DataDir, wadin, wadinf, Picture, Sound, fullSND,
      Select, trnR, trnG, trnB, WSafe, cusage);
  free(cusage);
}



int main( int argc, char *argv[])
{
 FILE *fp;
/* Options for which you don't want the banners */
   if (argc == 2 && ! strcmp (argv[1], "--version")) COMvers (argc, argv);

   if (argc == 2 && ! strcmp (argv[1], "-man"))  /* Sorry for the ugliness */
   {
     COMmanopt (argc, argv);
     return 0;
   }

#ifdef DT_ALPHA
 printf ( "+-----------------------------------------------------------+\n");
 printf ( "|  THIS IS ALPHA SOFTWARE. DON'T EXPECT IT TO COMPILE OR    |\n");
 printf ( "|  RUN SMOOTHLY. DON'T EXPECT THE DOCUMENTATION TO BE       |\n");
 printf ( "|  ACCURATE OR UP TO DATE. THERE MIGHT BE SERIOUS BUGS.     |\n");
 printf ( "|  MAKE BACKUP COPIES OF YOUR DATA.                         |\n");
 printf ( "+-----------------------------------------------------------+\n\n");
#elseif DT_PRIVATE
 printf ( "+------------------------------------+\n");
 printf ( "|   THIS RELEASE OF DEUTEX IS NOT    |\n");
 printf ( "|  INTENDED FOR PUBLIC CONSUMPTION.  |\n");
 printf ( "|     DO NOT FURTHER DISTRIBUTE.     |\n");
 printf ( "+------------------------------------+\n\n");
#else
 printf ( "+---------------------------------------------------+\n");
 printf ( "|  Send questions and bug reports to the current    |\n");
 printf ( "|  maintainer, André Majorel <amajorel@teaser.fr>,  |\n");
 printf ( "|  NOT to Olivier Montanuy !                        |\n");
 printf ( "+---------------------------------------------------+\n\n");
#endif
/* Sanity checks */
 printf("Checando tipos...\n");
 check_types ();

/* default parameters */
 printf("Ligando opÃ§Ãµes...\n");
 WadInfOk=FALSE;
 George=FALSE;
#if DT_OS == 'd'
 Picture    = PICBMP;
 Sound      = SNDWAV;
#elif DT_OS == 'o'
 Picture    = PICBMP;
 Sound      = SNDWAV;
#else   /*Unix*/
 Picture    = PICPPM;
 Sound      = SNDAU;
#endif
 trnR=0;
 trnG=47;
 trnB=47;
 fullSND    = FALSE;
 WSafe      = TRUE;
 HowMuchJunk= 0;
 Select     = 0;
 Type       = PWAD;
 printf("Vendo troÃ§o do erro...\n");
/*no error handler defined*/

#if defined __OS2__ || defined (__GNUC__)
/*setbuf(stdout,(char *)NULL);*/
 printf("Pondo buffer...\n");
 setvbuf(stdout,NULL,_IOLBF,BUFSIZ);
#endif
/*OS2*/
/*
 print on screen or file?
*/
 printf("Decidindo trecos...\n");
/* directories  */
 DataDir     = ".";  /*work directory*/
 DoomDir    = getenv("DOOMWADDIR");
 if(DoomDir == NULL) DoomDir    = ".";
 else printf("PHASE: Doom directory is %.128s\n",DoomDir);
/* Command line */
 printf("Esquisitice dos argumentos...\n");
 argc--;
 argv++;
 if(argc>30) printf("ERROR: too many arguments");

/*
Parse the command line from left to right.
Try to match the each argument against the
longest possible option defined.
*/
 printf("Checando argumentos...\n");
   while(argc>0)
   { const comdef_t *d = Com + sizeof Com / sizeof *Com - 1;
     const char *arg = *argv;
     if (*arg != '-')
        printf("ERROR: Argument \"%s\": expected an option", arg);
     arg++;

     {
       const comdef_t *w;
       for (w = Com; w->type != END; w++)
       {
	 if (is_prefix (arg, w->com))
	 {
	   if (! strcmp (arg, w->com))  /* Exact match. */
	   {
	     d = w;
	     goto got_it;
	   }
	   if (d->type != END)  /* Ambiguous partial match. */
	     printf("ERROR: \"-%s\" is ambiguous (-%s, -%s)", arg, d->com, w->com);
           /* Unambiguous partial match. */
	   d = w;
	 }
       }
     }
got_it:

     if (argc - 1 < d->argc)
       printf("ERROR: Usage: %s%s%s",
	   d->com,
	   d->use ? " "    : "",
	   d->use ? d->use : "");
     switch(d->type)
     { case END:
         printf("ERROR: Option %s is not defined.",argv[0]);
         return -1;
       case NIL:
         d->exec(argc,argv);
         return 0;
       case OPT:
         d->exec(argc,argv);
         break;
       case CMD:
         /*find DOOM,HERETIC*/
         if(DoomDir==NULL)
         { /*
           ** check if file exists
           */
           fp=fopen(MainWAD,FOPEN_RB);
	   if(fp!=NULL)
             fclose(fp);  /* AYM 1999-01-13 once again, the code assumed
			     that calling fclose with a NULL pointer is
			     a normal thing to do. */
           if(fp==NULL) DoomDir=".";
         }
         if(DoomDir!=NULL)
         { /*
           ** look for main wad
           */
           if(    MakeFileName(MainWAD,DoomDir,"","","doompres","wad")!=TRUE
	       && MakeFileName(MainWAD,DoomDir,"","","doom",    "wad")!=TRUE
	       && MakeFileName(MainWAD,DoomDir,"","","doom2",   "wad")!=TRUE
               && MakeFileName(MainWAD,DoomDir,"","","heretic", "wad")!=TRUE
               && MakeFileName(MainWAD,DoomDir,"","","hexen",   "wad")!=TRUE
               && MakeFileName(MainWAD,DoomDir,"","","strife1", "wad")!=TRUE
               && MakeFileName(MainWAD,".",    "","","doompres","wad")!=TRUE
               && MakeFileName(MainWAD,".",    "","","doom",    "wad")!=TRUE
	       && MakeFileName(MainWAD,".",    "","","doom2",   "wad")!=TRUE
               && MakeFileName(MainWAD,".",    "","","heretic", "wad")!=TRUE
               && MakeFileName(MainWAD,".",    "","","hexen",   "wad")!=TRUE
               && MakeFileName(MainWAD,".",    "","","strife1", "wad")!=TRUE)
             printf("ERROR: Can't find any of doom.wad, doom2.wad, doompres.wad,"
		 " heretic.wad, hexen.wad, strife1.wad");
         }
         /*selection bytes*/
         if(!(Select&BALL))  Select = BALL;
         if (Picture == PICGIF)
	 {
	   printf("WARNING: GIF support may go away in the future (see");
	   printf("WARNING: http://lpf.ai.mit.edu/Patents/Gif/Gif.html).");
	   printf("WARNING: Use PPM or BMP instead.");
	 }

         d->exec(argc,argv);
         return 0;
     }
   argv += d->argc + 1;
   argc -= d->argc + 1;
   }
   return -1;
}

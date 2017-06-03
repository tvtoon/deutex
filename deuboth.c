#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deutex.h"
#include "deuboth.h"
#include "merge.h"

#define COLUMN1_WIDTH 22

extern const comdef_t Com[];
extern const char *Format[];
/*
**  global variables for commands
*/
char file[128];        /* general use file name*/

NTRYB Select;
const char *DataDir=NULL;
const char *DoomDir=NULL;
char MainWAD[128];
Int16 HowMuchJunk;		/* junk to add*/

char trnR,trnG,trnB;
picture_format_t picture_format        = PF_NORMAL;
texture_format_t input_texture_format  = TF_NORMAL;
texture_format_t output_texture_format = TF_NORMAL;
texture_lump_t   texture_lump          = TL_NORMAL;
rate_policy_t    rate_policy            = RP_WARN;
const char *debug_ident    = NULL;
int old_music_ident_method = 0;
char anon[1]        = { '\0' };

/*
 *	call_opt
 *	Equivalent to having the same option on the command line
 */
void call_opt (comfun_t func, ...)
{
   int argc;
   char *argv[10];
   va_list args;

   va_start (args, func);
   for (argc = 0; argc < (int)(sizeof argv / sizeof *argv); argc++)
   {
      argv[argc] = va_arg (args, char *);
      if (argv[argc] == NULL)
      {
	 argc++;
	 break;
      }
   }
   func (argc, argv);
}


/*
** commands
*/

void COMwintex(int argc, char *argv[])
{
   DoomDir=NULL;
   strncpy(MainWAD,argv[1],128);/*main*/
   DataDir=".";
   Select|= BALL;                /*select*/
   trnR= 0;trnG= 255;trnB= 255;
   (void)argc;
}

void COMverbose(int argc, char *argv[])
{
  printf("INFO: Verbosity level is %c.\n",argv[0][2]);
  (void)argc;
}

void COMdoom(int argc, char *argv[])
{
 DoomDir=argv[1];
 printf("INFO: Main directory: %s.\n",DoomDir);
 (void)argc;
}

void COMstrife(int argc, char *argv[])
{  call_opt (COMdoom, anon, argv[1],    NULL);
   call_opt (COMtf,   "tf", "strife11", NULL);
   (void)argc;
}

void COMmain(int argc, char *argv[])
{  DoomDir=NULL;
   strncpy(MainWAD,argv[1],128);
   printf("INFO: Main IWAD file: %s.\n",MainWAD);
   (void)argc;
}

void COMwadir(int argc, char *argv[])
{ XTRlistDir(MainWAD,((argc<2)? NULL: argv[1]),Select);
}

void COMadd(int argc, char *argv[])
{ ADDallSpriteFloor(argv[2],MainWAD,argv[1],Select);
  (void)argc;
}

void COMapp(int argc, char *argv[])
{ ADDappendSpriteFloor(MainWAD,argv[1],Select);
  (void)argc;
}

void COMapps(int argc, char *argv[])
{ Select= (BALL) & (~BFLAT); /*no flats*/
  ADDappendSpriteFloor(MainWAD,argv[1],Select);
  (void)argc;
}

void COMappf(int argc, char *argv[])
{ Select= (BALL) & (~BSPRITE); /*no sprites*/
  ADDappendSpriteFloor(MainWAD,argv[1],Select);
  (void)argc;
}

void COMjoin(int argc, char *argv[])
{ ADDjoinWads(MainWAD,argv[1],argv[2],Select);
  (void)argc;
}

void COMmerge(int argc, char *argv[])
{ Select     = BALL;
  PSTmergeWAD(MainWAD,argv[1],Select);
  (void)argc;
}

void COMrestor(int argc, char *argv[])
{ HDRrestoreWAD((argc>=2)? argv[1]:MainWAD);
}

void COMtf (int argc, char *argv[])
{
  int set_in = 0;
  int set_out = 0;

  if (! strcmp (argv[0], "itf"))
    set_in = 1;
  else if (! strcmp (argv[0], "otf"))
    set_out = 1;
  else if (! strcmp (argv[0], "tf"))
  {
    set_in = 1;
    set_out = 1;
  }
  else
    printf("BUG: COMtf: bad argv[0] \"%.32s\"", argv[0]);

  if (argc >= 2 && ! strcmp (argv[1], "nameless"))
  {
    if (set_in)
      input_texture_format = TF_NAMELESS;
    if (set_out)
      output_texture_format = TF_NAMELESS;
  }
  else if (argc >= 2 && ! strcmp (argv[1], "none"))
  {
    if (set_in)
      input_texture_format = TF_NONE;
    if (set_out)
      output_texture_format = TF_NONE;
  }
  else if (argc >= 2 && ! strcmp (argv[1], "normal"))
  {
    if (set_in)
      input_texture_format = TF_NORMAL;
    if (set_out)
      output_texture_format = TF_NORMAL;
  }
  else if (argc >= 2 && ! strcmp (argv[1], "strife11"))
  {
    if (set_in)
      input_texture_format = TF_STRIFE11;
    if (set_out)
      output_texture_format = TF_STRIFE11;
  }
  else
    printf("ERROR: Usage is \"-%.32s {nameless|none|normal|strife11}\"", argv[0]);
  if (set_in)
    printf("INFO: Input texture format is \"%s\"\n", argv[1]);
  if (set_out)
    printf("INFO: Output texture format is \"%s\"\n", argv[1]);
}

void COMcheck(int argc, char *argv[])
{ XTRstructureTest(MainWAD,argv[1]);
  (void)argc;
}

/*
** Print Help
*/

void COMhelp(int argc, char *argv[])
{ const comdef_t *d;
  printf("Help for %s:\n", argv[0] );
  for(d = Com; d->type != END; d++)
  {
    char buf[200];
    sprintf (buf, "-%s", d->com);
    if (d->use)
    {
      strcat (buf, " ");
      strcat (buf, d->use);
    }
    if (strlen (buf) > COLUMN1_WIDTH)
      printf("%s\n%*s ", buf, COLUMN1_WIDTH, "");
    else
      printf("%-*s ", COLUMN1_WIDTH, buf);
    printf("%s", d->type == OPT ? "(opt.) " : "");
    if (d->help)
      printf("%c%s.", toupper (d->help[0]), d->help + 1);
    else
      printf("Same as above.");
    putchar ('\n');
  }
  (void)argc;(void)argv;
}

/*
 *	Print version and exit successfully.
 *	All --version does.
 */
void COMvers (int argc, char *argv[])
{
  (void) argc;
  (void) argv;
  printf("%s.\n", _VERSION_);
  exit(0);
}

void COMformat(int argc, char *argv[])
{ int i;
  for(i=0;Format[i]!=NULL;i++)
  { printf("%s\n",Format[i]);
  }
  (void)argc;(void)argv;
}


/*
 *	is_prefix
 *	Return non-zero iff s1 is a prefix of s2
 */
int is_prefix (const char *s1, const char *s2)
{
  for (;; s1++, s2++)
  {
    if (*s1 == '\0')
      return 1;
    if (*s2 != *s1)
      return 0;
  }
}

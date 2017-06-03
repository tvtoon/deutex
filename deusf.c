#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deutex.h"
#include "deuboth.h"
#include "tools.h"

void COMsprit(int argc, char *argv[]);
void COMflat(int argc, char *argv[]);
/* take this shit out of here*/
const WADTYPE Type = PWAD;

extern NTRYB Select;
extern const char *DataDir;
extern const char *DoomDir;
extern char MainWAD[128];
extern Int16 HowMuchJunk;

const char *Format[] =
{
 "To work with this progam, your PWAD must respect this format:",
 "- Either S_START or SS_START before the first SPRITE entry",
 "- Either S_END or SS_END after the last SPRITE entry",
 "- Either F_START or FF_START before the first FLAT entry",
 "- Either F_END or FF_END after the last FLAT entry",
 "This format has been found compatible with DMADDS 1.0.",
 "Warning:",
 "Flats declared in PWAD will be put at the end of the FLAT list.",
 "You can completely replace an animation of FLATs, but you cannot",
 "replace only part of an animation.",
 NULL
};

const comdef_t Com[] =
{
 {NIL,0,"?",        COMhelp,   NULL,   "print list of options"},
 {NIL,0,"h",        COMhelp,   NULL,   NULL},
 {NIL,0,"help",     COMhelp,   NULL,   NULL},
 {NIL,0,"-help",    COMhelp,   NULL,   NULL},
 {NIL,0,"-version", COMvers,   NULL,   "print version number and exit successfully"},
 {NIL,0,"syntax",   COMformat, NULL,   "print the syntax of wad creation directives"},
 {OPT,1,"wtx",      COMwintex, "<iwad>","WinTex shortcut"},
 {OPT,1,"doom",     COMdoom,   "<dir>","indicate the directory of Doom"},
 {OPT,1,"doom2",    COMdoom,   "<dir>","indicate the directory of Doom II"},
 {OPT,1,"heretic",  COMdoom,   "<dir>","indicate the directory of Heretic"},
 {OPT,1,"hexen",    COMdoom,   "<dir>","indicate the directory of Hexen"},
 {OPT,1,"strife",   COMstrife, "<dir>","indicate the directory of Strife"},
 {OPT,1,"strife10", COMdoom,   "<dir>","indicate the directory of Strife 1.0"},
 {OPT,0,"sprites",  COMsprit,  NULL,   "select sprites"},
 {OPT,0,"flats",    COMflat,   NULL,   "select flats"},
 {OPT,0,"v0",       COMverbose,NULL,   "set verbosity level to 0"},
 {OPT,0,"v1",       COMverbose,NULL,   "set verbosity level to 1"},
 {OPT,0,"v2",       COMverbose,NULL,   "set verbosity level to 2 (default)"},
 {OPT,0,"v3",       COMverbose,NULL,   "set verbosity level to 3"},
 {OPT,0,"v4",       COMverbose,NULL,   "set verbosity level to 4"},
 {OPT,0,"v5",       COMverbose,NULL,   "set verbosity level to 5"},

 {OPT,1,"main",     COMmain,   NULL,   "Indicate main wad"},
 {CMD,0,"wadir",    COMwadir,  "<in.wad>","list and identify entries in a wad"},
 {CMD,1,"check",    COMcheck,  "<in.wad>","check the textures"},
 {CMD,1,"test",     COMcheck,  "<in.wad>",NULL},

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
** Selections
*/
void COMsprit(int argc, char *argv[])
{ Select&= ~BFLAT;
  printf("INFO: Select SPRITES\n");
  (void)argc;(void)argv;
}

void COMflat(int argc, char *argv[])
{ Select&= ~BSPRITE;
  printf("INFO: Select FLATS\n");
  (void)argc;(void)argv;
}

int main( int argc, char *argv[])
{
 FILE *fp;
/* Options for which you don't want the banners */
 if (argc == 2 && ! strcmp (argv[1], "--version")) COMvers (argc, argv);

 printf ( "+---------------------------------------------------+\n");
 printf ( "|  Send questions and bug reports to the current    |\n");
 printf ( "|  maintainer, André Majorel <amajorel@teaser.fr>,  |\n");
 printf ( "|  NOT to Olivier Montanuy !                        |\n");
 printf ( "+---------------------------------------------------+\n\n");
/* Sanity checks */
 printf("Checando tipos...\n");
 check_types ();

/* default parameters */
 printf("Ligando opções...\n");
 Select     = BALL;

 printf("Vendo troço do erro...\n");
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

         d->exec(argc,argv);
         return 0;
     }
   argv += d->argc + 1;
   argc -= d->argc + 1;
   }
   return -1;
}

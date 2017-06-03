#include <stdio.h>

#include "deutex.h"
#include "text.h"
#include "texture.h"
#include "tools.h"

extern Bool TXUok;
extern Int16 TXUtexTop;

extern struct PATCH *TXUpat;
extern Int16 TXUpatTop;
extern struct TEXTUR *TXUtex;
/*
** write texture as text file
*/
void TXUwriteTexFile(const char *file)
{  Int16 t,p,pat,top;
   char pname[8];
   FILE *out;

   if(TXUok!=TRUE)         printf("BUG: TXUok");
   if(TXUtexTop<1)         printf("BUG: TxunTx");

   out=fopen(file,FOPEN_WT);
   if(out==NULL)         printf("ERROR: Can't write file %s\n",file);
   TXUrealTexture();
   fprintf(out,";Format of textures:\n");
   fprintf(out,";TextureName\tWidth\tHeight\n");
   fprintf(out,";*\tPatchName\tXoffset\tYoffset\n");
   for (pat=0, t= 0; t <TXUtexTop; t++)
   {  if(TXUtex[t].Name[0]!='\0') /*if tex was not redefined*/
      { fprintf(out,"%-8.8s    ",TXUtex[t].Name);
        fprintf(out,"\t\t%d\t%d\n",TXUtex[t].szX,TXUtex[t].szY);
        for (p = 0; p < TXUtex[t].Npatches; p++)
        { top=pat+p;
          if(top>=TXUpatTop) printf("BUG: TxuP>D");
          PNMgetPatchName(pname,TXUpat[pat+p].Pindex);
          fprintf(out,"*\t%-8.8s    ",pname);
          fprintf(out,"\t%d\t%d\n",TXUpat[pat+p].ofsX,TXUpat[pat+p].ofsY);
        }
      }
      pat+=TXUtex[t].Npatches;
   }
   fprintf(out,";End\n");
   fclose(out);                                                
}
/*
** read texture as text file
**
*/
void TXUreadTexFile(const char *file,Bool Redefn)
{  Int16 Pindex;
   Int16 xsize=0,ysize=0,ofsx=0,ofsy=0;
   char tname[8];  
   char pname[8];
   Int16 t,p,bit,C,b;    /*to check Xsize*/
   struct TXTFILE *TXT;
   TXT=TXTopenR(file);
/*   if(TXTbeginBloc(TXT,"TEXTURES")!=TRUE)printf("ERROR: Invalid texture file format: %s",file);
*/
   for(t=0;t<MAXTEXTURES;t++)
   { if(TXTreadTexDef(TXT,tname,&xsize,&ysize)==FALSE) break;
     /* check X size */
     if((xsize<0)||(xsize>4096))
       printf("ERROR: texture width out of bound");
     for(bit=1,C=0,b=0;b<16;b++,bit<<=1) if(xsize&bit) C++;
     if(C>1)
       printf("WARNING: Bogus texture %s. Width should be a power of 2",
	   lump_name (tname));
     /* check Y size */
     if((ysize<0)||(ysize>4096))
       printf("ERROR: texture height out of bound");
     // AYM 1999-05-17
     if(ysize>509)
       printf("WARNING: Bogus texture %s. Heights above 509 are ignored",
	   lump_name (tname));
     /* declare texture */
     TXUdefineCurTex(tname,xsize,ysize, Redefn);
     for(p=0;p<MAXPATCHPERTEX;p++)
     { /* exit if no new patch to be read*/
       if(TXTreadPatchDef(TXT,pname,&ofsx,&ofsy)==FALSE) break;
       /* declare a patch wall, getting index or declaring a new index */
       Pindex=PNMgetPatchIndex(pname);
       TXUaddPatchToCurTex(Pindex,ofsx,ofsy);
     }
   }
   printf("PHASE: Read %d Textures in %s\n",t,file);
   TXTcloseR(TXT);
}
/***************End TEXTURE module*********************/

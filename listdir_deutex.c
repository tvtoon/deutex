#include <stdio.h>
#include <stdlib.h>

#include "deutex.h"
#include "listdir.h"
#include "mkwad.h"
#include "texture.h"
#include "tools.h"

#if DT_OS == 'o'
#  if DT_CC = 'b'
int _USERENTRY XTRdirCmp(const void *d1,const void *d2)
#  else
int _Optlink XTRdirCmp(const void *d1,const void *d2)
#  endif
#else
int XTRdirCmp(const void *d1,const void *d2)
#endif
{  Int32  res;
	struct WADDIR *dir1=(struct WADDIR *)d1;
	struct WADDIR *dir2=(struct WADDIR *)d2;
	res= (dir1->start)-(dir2->start);
	if(res<0)return -1;
	if(res>0)return 1;
	res = (dir1->size)-(dir2->size);
	if(res<0)return -1;
	if(res>0)return 1;
	return 0;
}

void XTRvoidSpacesInWAD(const char *wadin)
{  Int16 n;
	static struct WADINFO pwad;
	Int32 ntry;
	struct WADDIR  *dir;
	Int32 startpos,lastpos,ll,diff,wtotal;
	Int32 w3,w20,w100,w1000,w10000,w100000;
	wtotal=w3=w20=w100=w1000=w10000=w100000=0;

	pwad.ok=0;
	WADRopenR(&pwad,wadin);
	ntry= pwad.ntry;
	dir=  pwad.dir;
	qsort(dir,(size_t)ntry,sizeof(struct WADDIR),XTRdirCmp);
	printf("\nListing of unused spaces in WAD %s\n",wadin);
	lastpos=4+4+4;
	for(n=0;n<ntry;n++)
	{  diff=dir[n].start-lastpos;
		startpos=lastpos;
		ll=dir[n].start+dir[n].size;
		if(lastpos<ll) lastpos=ll;
		if(diff<0)continue;
		wtotal+=diff;
		if(diff<=3)
		w3+=diff;
		else if(diff<=20)
		w20+=diff;
		else if(diff<=100)
		w100+=diff;
		else if(diff<=1000)
		w1000+=diff;
		else if(diff<=10000)
		w10000+=diff;
		else
		w100000+=diff;
		if(diff>=4) /*suppress word alignement*/
		 printf("  At offset 0x%08x, %d bytes wasted\n",startpos,diff);
	}
	printf("\nRepartition of wasted bytes:\n");
	printf(" All blocks<=3    \t%d\n",w3);
	printf(" All blocks<=20   \t%d\n",w20);
	printf(" All blocks<=100  \t%d\n",w100);
	printf(" All blocks<=1000 \t%d\n",w1000);
	printf(" All blocks<=10000\t%d\n",w10000);
	printf(" All blocks> 10000\t%d\n",w100000);
	printf("                  \t-------\n");
	printf(" Total wasted bytes\t%d\n",wtotal);
	WADRclose(&pwad);
}

/*
** Test a PWAD
** 
*/
void XTRtextureUsed(const char *wadin)
{ static struct WADINFO pwad;
  /*read PNAME in wad, if defined*/
  printf("PHASE: listing texture used in the levels of %s\n",wadin);
  pwad.ok=0;
  WADRopenR(&pwad,wadin);
  /*
  ** list all textures composing walls
  */
  TXUinit();
  CheckLevels(&pwad,TRUE);
  printf("List of textures used in %s\n\n",wadin);
  TXUlistTex();
  TXUfree();
  WADRclose(&pwad);
}

/*
** Detect duplicate entries
** ShowIdx = TRUE if we also output the indexes
**
** Optimise for speed with a CRC-based check
*/
void XTRcompakWAD(const char *DataDir,const char *wadin, const char *texout,Bool ShowIdx, char *file )
{  static struct WADINFO pwad;
   struct WADDIR  *pdir;
   Int16 pnb;
   Int16 p,bas,tst,ofsx,ofsy;
   Int32 size,rsize,sz;
   Bool  *psame;
   FILE *out;
   char  *bbas;
   char  *btst;
   printf("PHASE: Detecting duplicate entries in WAD %s\n",wadin);
   pwad.ok=0;
   WADRopenR(&pwad,wadin);
   pnb=(Int16)pwad.ntry;
   pdir=pwad.dir;
   psame=(Bool  *)malloc(pnb*sizeof(Bool));
   for(p=0;p<pnb;p++)
   { psame[p]=FALSE;}
   if(texout==NULL)
     MakeFileName(file,DataDir,"","","WADINFOP","TXT");
   else
     sprintf(file,"%.120s",texout);
   out=fopen(file,FOPEN_WT);
   fprintf(out,"; Indication of similar entries\n\n");
   bbas=(char  *)malloc(MEMORYCACHE);
   btst=(char  *)malloc(MEMORYCACHE);
   for(bas=0;bas<pnb;bas++)
     if(psame[bas]==FALSE) /*skip already treated*/
       { size=pdir[bas].size;
         if(pdir[bas].start<=8)continue;
         if(size<1)continue;
         if((size>=8)&&(ShowIdx==TRUE))
         { WADRseek(&pwad,pdir[bas].start);
           WADRreadBytes(&pwad,bbas,8);
           ofsx=((bbas[5]<<8)&0xFF00)+(bbas[4]&0xFF);
           ofsy=((bbas[7]<<8)&0xFF00)+(bbas[6]&0xFF);
           fprintf(out,"%-8.8s\t%d\t%d\n",pdir[bas].name,ofsx,ofsy);
         }
         else
           fprintf(out,"%-8.8s\n",pdir[bas].name);
         for(tst=bas+1;tst<pnb;tst++)
         { /*if same size*/
           if(pdir[tst].start<0)continue;
           if(pdir[tst].size!=size)continue;
           /*check that equal*/
           for(sz=rsize=0;rsize<size;rsize+=sz)
           { sz = (size-rsize>MEMORYCACHE)? MEMORYCACHE : size-rsize;
             WADRseek(&pwad,pdir[bas].start+rsize);
             WADRreadBytes(&pwad,bbas,sz);
             WADRseek(&pwad,pdir[tst].start+rsize);
             WADRreadBytes(&pwad,btst,sz);
             for(p=0;p<sz;p++)
             { if(bbas[p]!=btst[p]) break;
             }
             if(p<sz)break;
           }
           if(rsize==size) /*entry identical to reference*/
              { psame[tst]=TRUE;
             fprintf(out,"%-8.8s\t*\n",pdir[tst].name);
           }
         }
       }
   fclose(out);
   WADRclose(&pwad);
   free(bbas);
   free(btst);
   free(psame);
}

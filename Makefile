PROJECT = deutex
MAJVER = 4.4
MINVER = 1
LIBS = deutex
PROGS = deusf deutex

include make/conf
include make/libmath.mk

#CFLAGS = -DDT_ALPHA -DDT_PRIVATE
# -DDeuTex for deutex
# -DDeuSF for deusf



# To generate CHANGES and FAQ: lynx -dump $< >>$@
DOCS = CHANGES FAQ README TODO
DATA =
ICONS =
INFOS =
INCLUDES = color.h deutex.h endianio.h endianm.h extract.h gifcodec.h ident.h
INCLUDES += listdir.h lists.h merge.h mkwad.h picture.h sound.h text.h
INCLUDES += texture.h tools.h usedidx.h wadio.h
MANS = deutex.6
SRC = endianio.c endianm.c ident.c listdir.c lists.c merge.c mkwad.c picture.c
SRC += texture.c tools.c usedidx.c wadio.c
#Useless: substit.c.
# Deutex only. Also gifcodec.c.
SRC2 += color.c compose.c extract.c listdir_deutex.c lzw.c mkwad_deutex.c
SRC2 += picture_deutex.c sound.c text.c texture_deutex.c
PROGSRC = deusf.c deuboth.c deutex.c
LIBSRC = ${SRC} ${SRC2}

include make/exconf
include make/build

${PROGS}: deuboth.o
deusf deusf-static: deusf.o
deutex deutex-static: deutex.o
deutex-static: ${SRC2:.c=.o}
dist-clean: clean

include make/pack
include make/rules
include make/thedep

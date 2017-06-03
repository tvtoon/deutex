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
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "deutex.h"
#include "endianm.h"
#include "picture.h"

/*
 *	parse_pic_header
 *
 *	buf      Data for the entire lump containing the
 *	         hypothethical image.
 *	bufsz    Size of buf in bytes.
 * 	h        Pointer on struct through which
 * 	         parse_pic_header() returns the characteristic
 * 	         of the picture header.
 *	message  Pointer on char[81]. If an error occurs (non-zero
 *	         return value) and <message> is not NULL, a
 *	         string of at most 80 characters is written
 *	         there.
 *
 *	Return 0 on success, non-zero on failure.
 */
#define FAIL0(s)   do{if (message) strcpy (message, s);     return 1;}while(0)
#define FAIL1(f,a) do{if (message) sprintf (message, f, a); return 1;}while(0)

int parse_pic_header (
    const unsigned char *buf,
    long bufsz,
    pic_head_t *h,
    char *message)
{
   /* Current byte of pic buffer */
   const unsigned char *p = (const unsigned char *) buf;
   /* Last byte of pic buffer */
   const unsigned char *buf_end = ((const unsigned char *) buf) + bufsz - 1;

   /* Details of picture format in wad */
   h->dummy_bytes = picture_format == PF_NORMAL;
   h->colofs_size = (picture_format == PF_NORMAL) ? 4 : 2;

   /* Read the picture header */
   if (picture_format != PF_ALPHA)
   {
      if (p + 8 - 1 > buf_end)
	 FAIL0 ("header too short");
      read_i16_le (p, &h->width);
      p += 2;
      read_i16_le (p, &h->height);
      p += 2;
      read_i16_le (p, &h->xofs);
      p += 2;
      read_i16_le (p, &h->yofs);
      p += 2;
   }
   else
   {
      if (p + 4 - 1 > buf_end)
	 FAIL0 ("header too short");
      h->width  = *p++;
      h->height = *p++;
      h->xofs   = *p++;
      h->yofs   = *p++;
   }

   /* Sanity checks on picture size and offsets */
   if (h->width < 1)
      FAIL1 ("width < 1 (%d)", (int) h->width);
   if (h->width > 4096)
      FAIL1 ("width > 4096 (%d)", (int) h->width);
   if (h->height < 1)
      FAIL1 ("height < 1 (%d)", (int) h->height);
   if (h->height > 4096)
      FAIL1 ("height > 4096 (%d)", (int)h->height);
   if (h->xofs < -4096)
      FAIL1 ("X-offset < -4096 (%d)", (int) h->xofs);
   if (h->xofs > 4096)
      FAIL1 ("X-offset > 4096 (%d)", (int) h->xofs);
   if (h->yofs < -4096)
      FAIL1 ("Y-offset < -4096 (%d)", (int) h->yofs);
   if (h->yofs > 4096)
      FAIL1 ("Y-offset > 4096 (%d)", (int) h->yofs);

   /* Array of column offsets. Column data. */ 
   h->colofs = p;
   p += (long) h->colofs_size * h->width;
   if (p > buf_end)
      FAIL0 ("offsets table too short");
   h->data = p;
   return 0;  /* Success */
}

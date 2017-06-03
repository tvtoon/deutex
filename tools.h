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


void check_types (void);

char *fnameofs (const char *name, long ofs);
char *fname (const char *name);
char *lump_name (const char *name);
char *short_dump (const char *data, size_t size);
/******************tools.c***********************/

/** FILE name , for lumps and BMP **/
void ToLowerCase(char *file);
void MakeDir(char file[128], const char *path, const char *dir, const char
    *sdir);
Bool MakeFileName(char file[128], const char *path, const char *dir, const
    char *sdir, const char *name, const char *extens);
void GetNameOfWAD(char name[8], const char *path);
Int16 Chsize(int handle,Int32 newsize);
Int32 GetFileTime(const char *path);
void SetFileTime(const char *path, Int32 time);
void Unlink(const char *file);

void Normalise(char dest[8], const char *src);

void Progress(void);
void ProgressEnds(void);

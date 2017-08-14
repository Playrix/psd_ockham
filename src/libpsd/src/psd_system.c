/**
 * libpsd - Photoshop file formats (*.psd) decode library
 * Copyright (C) 2004-2007 Graphest Software.
 *
 * psd_ockham - Photoshop file size reducing utility
 * Copyright (C) 2017 Playrix.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include "psd.h"
#include "psd_system.h"

#include <string.h>

void * psd_malloc(psd_int size)
{
	return malloc(size);
}

void * psd_realloc(void * block, psd_int size)
{
	return realloc(block, size);
}

void psd_free(void * block)
{
	free(block);
}

void psd_freeif(void * block)
{
	if (block != NULL)
		psd_free(block);
}

void * psd_fopen(const psd_char * file_name)
{
	FILE *d = fopen(file_name, "rb");
	return (void *)d;
}

void * psd_fopenw(const psd_char * file_name)
{
	FILE *d = fopen(file_name, "wb");
	return (void *)d;
}

long psd_fsize(void * file)
{
	long offset, size;

	offset = ftell((FILE *)file);
	fseek((FILE *)file, 0, SEEK_END);
	size = ftell(file);
	fseek((FILE *)file, 0, SEEK_SET);
	fseek((FILE *)file, offset, SEEK_CUR);

	return size;
}

size_t psd_fread(psd_uchar * buffer, psd_int count, void * file)
{
	size_t result = fread(buffer, 1, count, (FILE *)file);
	return result;
}

psd_int _psd_fseek(void * file, psd_long length, psd_int origin)
{
	return fseek((FILE *)file, (long)length, origin);
}

psd_int psd_fseek_cur(void * file, psd_long length)
{
	return _psd_fseek(file, (long)length, SEEK_CUR);
}

psd_int psd_fseek_set(void * file, psd_long length)
{
	return _psd_fseek(file, (long)length, SEEK_SET);
}

psd_int psd_fseek_end(void * file, psd_long length)
{
	return _psd_fseek(file, (long)length, SEEK_END);
}


long psd_ftell(void * file)
{
	long offset = ftell((FILE *)file);
	
	return offset;
}

size_t psd_fwrite(psd_uchar * buffer, size_t count, void * file)
{
	return fwrite(buffer, 1, count, (FILE *)file);
}

void psd_fclose(void * file)
{
	fclose((FILE *)file);
}


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

#include <sys/stat.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
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

psd_int psd_fopen(const psd_char * file_name)
{
	int f = _open(file_name, _O_RDONLY | O_BINARY);
	return f;
}

psd_int psd_fopenw(const psd_char * file_name)
{
	psd_int f = _open(file_name, _O_RDWR | _O_CREAT | O_BINARY);
	return f;
}

psd_long psd_fsize(psd_int file)
{
	struct _stat64 st = {0};
	if (_fstat64(file, &st) == 0)
		return st.st_size;

	return -1;
}

psd_int psd_fread(psd_uchar * buffer, psd_int count, psd_int file)
{
	psd_int result = _read(file, buffer, count);
	return result;
}

psd_long _psd_fseek(psd_int file, psd_long length, psd_int origin)
{
	psd_long result = _lseeki64(file, length, origin);
	return result;
}

psd_long psd_fseek_set(psd_int file, psd_long length)
{
	return _psd_fseek(file, length, 0);
}

psd_long psd_fseek_end(psd_int file, psd_long length)
{
	return _psd_fseek(file, length, 2);
}

psd_long psd_ftell(psd_int file)
{
	psd_long result = _telli64(file);
	return result;
}

psd_int psd_fwrite(psd_uchar * buffer, psd_int count, psd_int file)
{
	psd_int result = _write(file, buffer, count);
	return result;
}

void psd_fclose(psd_int file)
{
	_close(file);
}


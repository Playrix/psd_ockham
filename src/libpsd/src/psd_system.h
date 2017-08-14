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

#ifndef __PSD_SYSTEM_H__
#define __PSD_SYSTEM_H__

#include <assert.h>
#include <string.h>
#include "types.h"

#if 1
#define psd_assert(x)			assert(x)
#else
#define psd_assert(x)			do {} while(0)
// or
// #define psd_assert(x)		return psd_status_unkown_error
#endif


void * psd_malloc(psd_int size);
void * psd_realloc(void * block, psd_int size);
void psd_free(void * block);
void psd_freeif(void * block);
void * psd_fopen(const psd_char * file_name);
void * psd_fopenw(const psd_char * file_name);
long psd_fsize(void * file);
size_t psd_fread(psd_uchar * buffer, psd_int count, void * file);
size_t psd_fwrite(psd_uchar * buffer, size_t count, void * file);
psd_int psd_fseek_cur(void * file, psd_long length);
psd_int psd_fseek_set(void * file, psd_long length);
psd_int psd_fseek_end(void * file, psd_long length);
long psd_ftell(void * file);
void psd_fclose(void * file);

#endif

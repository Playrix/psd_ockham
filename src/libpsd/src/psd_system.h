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

#include "types.h"

void * psd_malloc(psd_int size);
void * psd_realloc(void * block, psd_int size);
void psd_free(void * block);
void psd_freeif(void * block);

psd_int psd_fopen(const psd_char * file_name);
psd_int psd_fopenw(const psd_char * file_name);
psd_long psd_fsize(psd_int file);
psd_int psd_fread(psd_uchar * buffer, psd_int count, psd_int file);
psd_int psd_fwrite(psd_uchar * buffer, psd_int count, psd_int file);
psd_long psd_fseek_set(psd_int file, psd_long length);
psd_long psd_fseek_end(psd_int file, psd_long length);
psd_long psd_ftell(psd_int file);
void psd_fclose(psd_int file);

#endif

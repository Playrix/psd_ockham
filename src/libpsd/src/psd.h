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

#ifndef __PSD_H__
#define __PSD_H__

#include "types.h"
#include "libpsd_ockham.h"

typedef struct _psd_stream
{
	psd_uchar *					buffer;
	psd_int						read_in_length;
	psd_int						read_out_length;
	psd_int						file_end;
	psd_int						current_pos;
	psd_bool					autowrite;
} psd_stream;

typedef struct _psd_context
{
	psd_char *					file_name;
	psd_char *					out_file_name;
	void *						file;
	void *						out_file;
	psd_stream *				stream;
	psd_short					version;
} psd_context;

typedef struct _psd_linked_layer
{
	psd_int						file_begin;
	psd_int						data_length;
	psd_context *				context;
} psd_linked_layer;

#endif //ifndef __LIB_PSD_H__

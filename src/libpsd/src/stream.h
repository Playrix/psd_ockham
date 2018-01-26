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

#ifndef __STREAM_H__
#define __STREAM_H__

#define PSD_STREAM_MAX_READ_LENGTH		4096

#define PSD_CHAR_TO_SHORT(str)			((*(str) << 8) | *((str) + 1))
#define PSD_CHAR_TO_INT(str)			((*(str) << 24) | (*((str) + 1) << 16) | (*((str) + 2) << 8) | *((str) + 3))


psd_int psd_stream_get(psd_context * context, psd_uchar * buffer, psd_int length);
psd_int psd_stream_get_w_null(psd_context * context, psd_int length);
psd_long psd_stream_get_null(psd_context * context, psd_long length);
psd_bool psd_stream_get_bool(psd_context * context);
psd_uchar psd_stream_get_char(psd_context * context);
psd_short psd_stream_get_short(psd_context * context);
psd_int psd_stream_get_int(psd_context * context);
psd_long psd_stream_get_long(psd_context * context);
psd_float psd_stream_get_float(psd_context * context);
psd_double psd_stream_get_double(psd_context * context);
void psd_stream_free(psd_context * context);

psd_int psd_stream_write_int(psd_context * context, psd_int value);
psd_int psd_stream_write_long(psd_context * context, psd_long value);
psd_long psd_stream_write_null(psd_context * context, psd_long count);

psd_status psd_stream_write_new_size_int(psd_context * context, psd_int old_length, psd_char rounding, psd_long write_pos, psd_long data_start_pos);
psd_status psd_stream_write_new_size_long(psd_context * context, psd_long old_length, psd_char rounding, psd_long write_pos, psd_long data_start_pos);

#endif

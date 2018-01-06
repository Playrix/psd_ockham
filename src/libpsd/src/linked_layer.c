/**
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

#include "psd.h"

#include "descriptor.h"
#include "stream.h"
#include "psd_system.h"

psd_status psd_get_linked_layer(psd_context * context)
{
	psd_status status = psd_status_done;

	psd_long block_length_pos = psd_ftell(context->out_file);

	psd_int block_length = (psd_int)psd_stream_get_long(context);
	if (block_length % 4 != 0)
		block_length += 4 - block_length % 4;
	psd_int prev_stream_pos = context->stream->current_pos;

	if (psd_stream_get_int(context) == 'liFD')
	{
		psd_stream_get_int(context);

		psd_int str_length = (psd_int)psd_stream_get_char(context);
		if (psd_stream_get_null(context, str_length) != str_length)
			return psd_status_linked_layer_error;

		psd_stream_get_object_wstring_null(context);
		psd_int file_type = psd_stream_get_int(context);
		psd_stream_get_int(context);

		psd_linked_layer linked_layer;

		psd_long data_length_pos = psd_ftell(context->out_file);
		linked_layer.data_length = (psd_int)psd_stream_get_long(context);

		psd_uchar has_descriptor = psd_stream_get_bool(context);

		if (has_descriptor)
		{
			psd_stream_get_int(context);
			status = psd_stream_get_object_descriptor(context);
			if (status != psd_status_done)
				return status;
		}

		psd_long data_start_pos = psd_ftell(context->out_file);
		linked_layer.file_begin = context->stream->current_pos;

		if (file_type == '8BPB' || file_type == '8BPS' || file_type == '    ')
		{
			status = psd_image_load_linked_layer(&linked_layer, context);
			if (status == psd_status_done)
			{
				status = psd_stream_write_new_size_long(context, linked_layer.data_length, 0, data_length_pos, data_start_pos);

				if (status != psd_status_done)
					return status;
			}
			else if (file_type != '    ' || status != psd_status_file_signature_error)
			{
				return status;
			}
			else
			{
				status = psd_status_done;
			}
		}
	}

	psd_int length = prev_stream_pos + block_length - context->stream->current_pos;
	if (psd_stream_get_null(context, length) != length)
		return psd_status_linked_layer_error;

	status = psd_stream_write_new_size_long(context, block_length, 4, block_length_pos, block_length_pos + 8);

	return status;
}

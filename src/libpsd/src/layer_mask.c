/**
 * libpsd - Photoshop file formats (*.psd) decode library
 * Copyright (C) 2004-2007 Graphest Software.
 *
 * psd_ockham - Photoshop file size reducing utility
 * Copyright (C) 2017-2018 Playrix.
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
#include "psd_system.h"
#include "stream.h"
#include "descriptor.h"


extern psd_status psd_get_linked_layer(psd_context * context);
extern void psd_linked_layer_free(psd_context * context);


// shows the high-level organization of the layer information.
static psd_status psd_get_layer_info(psd_context * context)
{
	psd_long length;

	// Length of the layers info section. (**PSB** length is 8 bytes.)
	if (context->version == 1)
		length = psd_stream_get_int(context);
	else
		length = psd_stream_get_long(context);
	
	if (psd_stream_get_null(context, length) != length)
		return psd_status_layer_info_error;

	return psd_status_done;
}

// Global layer mask info
static psd_status psd_get_mask_info(psd_context * context)
{
	// Length of global layer mask info section.
	psd_int length = psd_stream_get_int(context);
	// Filler: zeros
	if (psd_stream_get_null(context, length) != length)
		return psd_status_mask_info_error;

	return psd_status_done;
}

// The fourth section of a Photoshop file contains information about layers and masks.
// This section of the document describes the formats of layer and mask records.
psd_status psd_get_layer_and_mask(psd_context * context)
{
	psd_long prev_stream_pos, extra_stream_pos, length, size, full_size, remain_length;
	psd_status status;
	psd_uint tag;

	// Length of the layer and mask information section. (**PSB** length is 8 bytes.)
	psd_long length_pos = psd_ftell(context->out_file);
	if (context->version == 1)
		length = psd_stream_get_int(context);
	else
		length = psd_stream_get_long(context);
	if(length <= 0)
		return psd_status_done;
	
	prev_stream_pos = context->stream->current_pos;

	// Layer info
	status = psd_get_layer_info(context);
	
	if(status != psd_status_done)
		return status;

	// Global layer mask info
	if (prev_stream_pos + length - context->stream->current_pos > 0)
		status = psd_get_mask_info(context);

	while(prev_stream_pos + length - context->stream->current_pos > 12)
	{
		tag = psd_stream_get_int(context);
		if(tag == '8BIM' || tag == '8B64')
		{
			tag = psd_stream_get_int(context);

			psd_long size_pos = psd_ftell(context->out_file);

			if (context->version == 2)
			{
				switch (tag)
				{
					case 'LMsk':
					case 'Lr16':
					case 'Lr32':
					case 'Layr':
					case 'Mt16':
					case 'Mt32':
					case 'Mtrn':
					case 'Alph':
					case 'FMsk':
					case 'lnk2':
					case 'FEid':
					case 'FXid':
					case 'PxSD':

					case 'lnkE':
					case 'pths':
					case 'FELS':
					case 'extd':
					case 'extn':
						full_size = psd_stream_get_long(context);
						break;
					default:
						full_size = psd_stream_get_int(context);
						break;
				}
			}
			else
			{
				full_size = psd_stream_get_int(context);
			}

			size = full_size;
			
			switch(tag)
			{
				case 'lnkD':
				case 'lnk2':
				case 'lnk3':
					while(size >= 4)
					{
						extra_stream_pos = context->stream->current_pos;
						status = psd_get_linked_layer(context);
						size -= context->stream->current_pos - extra_stream_pos;

						if (status != psd_status_done)
							return status;
					}

					if (context->version == 1 || tag != 'lnk2')
						status = psd_stream_write_new_size_int(context, (psd_int)full_size, 4, size_pos, size_pos+4);
					else
						status = psd_stream_write_new_size_long(context, full_size, 4, size_pos, size_pos+8);

					if (status != psd_status_done)
						return status;
					
					context->stream->autowrite = psd_false;
					psd_stream_get_null(context, size);
					context->stream->autowrite = psd_true;

					break;
				default:
					if(size > 0) {
						if (size % 4 != 0)
							size += 4 - size % 4;
						psd_stream_get_null(context, size);
					}
					break;
			}
		}
		else
		{
			return psd_status_layer_and_mask_error;
		}
	}

	if (context->version == 1)
		status = psd_stream_write_new_size_int(context, (psd_int)length, 0, length_pos, length_pos+4);
	else
		status = psd_stream_write_new_size_long(context, length, 0, length_pos, length_pos+8);
	
	// Filler: zeros
	context->stream->autowrite = psd_false;
	remain_length = prev_stream_pos + length - context->stream->current_pos;
	if (psd_stream_get_null(context, remain_length) != remain_length)
		status = psd_status_layer_and_mask_error;
	context->stream->autowrite = psd_true;

	return status;
}



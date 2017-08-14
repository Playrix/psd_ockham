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

#include "psd.h"
#include "psd_system.h"
#include "stream.h"


enum {
	PSD_FILE_HEADER,
	PSD_COLOR_MODE_DATA,
	PSD_IMAGE_RESOURCE,
	PSD_LAYER_AND_MASK_INFORMATION,
	PSD_IMAGE_DATA,
	PSD_DONE
};

extern psd_status psd_get_file_header(psd_context * context);
extern psd_status psd_get_image_resource(psd_context * context);
extern psd_status psd_get_layer_and_mask(psd_context * context);

static psd_status psd_main_loop(psd_context * context);
static void psd_image_free(psd_context * context);


psd_status psd_image_load(const psd_char * file_name, const psd_char * out_file_name)
{
	psd_context * context;
	psd_status status;

	if(file_name == NULL)
		return psd_status_invalid_file;

	context = (psd_context *)psd_malloc(sizeof(psd_context));
	if(context == NULL)
		return psd_status_malloc_failed;
	memset(context, 0, sizeof(psd_context));

	context->file_name = (psd_char *)file_name;
	context->file = psd_fopen(file_name);
	if (context->file == NULL)
	{
		psd_free(context);
		return psd_status_invalid_file;
	}
	context->out_file_name = (psd_char *)out_file_name;
	context->out_file = psd_fopenw(out_file_name);
	if (context->out_file == NULL)
	{
		psd_free(context);
		return psd_status_invalid_out_file;
	}
	
	context->stream = (psd_stream *)psd_malloc(sizeof(psd_stream));
	if(context->stream == NULL)
		return psd_status_malloc_failed;
	memset(context->stream, 0, sizeof(psd_stream));

	context->stream->file_end = psd_fsize(context->file);
	context->stream->autowrite = psd_true;

	status = psd_main_loop(context);

	psd_image_free(context);

	return status;
}

psd_status psd_image_load_linked_layer(psd_linked_layer * layer, psd_context * parent_context)
{
	psd_context context = {0};
	psd_status status;

	context.file = parent_context->file;
	context.out_file = parent_context->out_file;
	
	context.stream = parent_context->stream;

	psd_int old_file_end = parent_context->stream->file_end;
	context.stream->file_end = layer->file_begin + layer->data_length;
	
	status = psd_main_loop(&context);
	
	context.stream->file_end = old_file_end;

	return status;
}

void psd_image_free(psd_context * context)
{
	psd_stream_free(context);
	psd_free(context);
}

static psd_status psd_main_loop(psd_context * context)
{
	psd_status status = psd_status_done;
	psd_uint state = PSD_FILE_HEADER;
	psd_int length;

	while(status == psd_status_done)
	{
		if(context->stream->file_end <= 0)
		{
			status = psd_status_fread_error;
			break;
		}
		
		switch(state)
		{
			case PSD_FILE_HEADER:
				status = psd_get_file_header(context);
				if(status == psd_status_done)
				{
					state = PSD_COLOR_MODE_DATA;
				}
				else if(status == psd_status_unkown_error)
					status = psd_status_file_header_error;
				break;
				
			case PSD_COLOR_MODE_DATA:
				length = psd_stream_get_int(context);
				if(length > 0)
				{
					if (psd_stream_get_null(context, length) != length)
						status = psd_status_color_mode_data_error;
				}
				state = PSD_IMAGE_RESOURCE;
				break;
				
			case PSD_IMAGE_RESOURCE:
				status = psd_get_image_resource(context);
				if(status == psd_status_done)
					state = PSD_LAYER_AND_MASK_INFORMATION;
				else if(status == psd_status_unkown_error)
					status = psd_status_image_resource_error;
				break;
				
			case PSD_LAYER_AND_MASK_INFORMATION:
				status = psd_get_layer_and_mask(context);
				if(status == psd_status_done)
					state = PSD_IMAGE_DATA;
				else if(status == psd_status_unkown_error)
					status = psd_status_layer_and_mask_error;
				break;
				
			case PSD_IMAGE_DATA:
				length = context->stream->file_end - context->stream->current_pos;
				if (psd_stream_get_null(context, length) != length)
					status = psd_status_image_data_error;
				state = PSD_DONE;
				break;

			case PSD_DONE:
				return psd_status_done;

			default:
				return psd_status_unkown_error;
		}
	}

	return status;
}


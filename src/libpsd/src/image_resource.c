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

#include "psd.h"
#include "psd_system.h"
#include "stream.h"

#include <string.h>
#include <stdio.h>

static const psd_char * const AncestorsOpenTag = "<photoshop:DocumentAncestors>";
static const psd_char * const AncestorsCloseTag = "</photoshop:DocumentAncestors>";

void writeToTempFile(psd_char * data, size_t sizeofdata, psd_char* out_file_name, psd_char* suffix)
{
	if (out_file_name != NULL)
	{
		size_t s_len = strlen(suffix);
		psd_char * xml_filename = (psd_char *)psd_malloc(strlen(out_file_name) + s_len + 1);
		strcpy(xml_filename, out_file_name);
		strcpy(xml_filename + strlen(out_file_name), suffix);
		xml_filename[strlen(out_file_name) + s_len] = 0;
		int f = psd_fopenw(xml_filename);
		if (f >= 0)
		{
			psd_fwrite((psd_uchar*)data, sizeofdata, f);
		}
		psd_free(xml_filename);
		psd_fclose(f);
	}
}


psd_bool is_space(psd_char c)
{
	switch (c)
	{
		case ' ':
		case '\t':
		case '\r':
			return psd_true;
	}
	return psd_false;
}

psd_bool is_newline(psd_char c)
{
	switch (c)
	{
		case '\n':
			return psd_true;
	}
	return psd_false;
}

psd_bool is_tag(const psd_char * metadata, size_t start, size_t end, const psd_char * tag)
{
	if (start >= end)
		return psd_false;

	size_t length = strlen(tag);
	if (end - start != length-1)
		return psd_false;

	for (size_t i = 0; i < length; i++)
	{
		if (metadata[start + i] != tag[i])
			return psd_false;
	}

	return psd_true;
}

psd_char * makeCutMetadata(psd_char * metadata, size_t sizeofdata)
{
	psd_char * result = (psd_char *)psd_malloc(sizeofdata+1);
	size_t result_pos = 0;
	if (result == NULL)
		return NULL;

	size_t i, line_start, trimmed_start, trimmed_end;

	psd_bool is_empty = psd_true;
	line_start = 0;
	trimmed_start = trimmed_end = 0;

	psd_char const * waiting_tag = NULL;

	for (i = 0; i < sizeofdata; i++)
	{
		if (is_newline(metadata[i]) || i == sizeofdata-1)
		{
			// if (!is_empty)
			{
				if (waiting_tag != NULL)
				{
					if (is_tag(metadata, trimmed_start, trimmed_end, waiting_tag))
						waiting_tag = NULL;
				}
				else if (is_tag(metadata, trimmed_start, trimmed_end, AncestorsOpenTag))
				{
					waiting_tag = AncestorsCloseTag;
				}
				else
				{
					strncpy(result + result_pos, metadata + line_start, i - line_start+1);
					result_pos += i - line_start + 1;
				}
			}
			is_empty = psd_true;
			line_start = i+1;
			trimmed_start = trimmed_end = i+1;
			continue;
		}
		if (!is_space(metadata[i]))
		{
			if (is_empty)
				trimmed_start = i;
			trimmed_end = i;
			is_empty = psd_false;
		}
	}
	if (result[result_pos - 1] != 0)
		result[result_pos] = 0;
	return result;
}

psd_status psd_get_image_resource(psd_context * context)
{
	psd_status status = psd_status_done;
	psd_int length, full_length;
	psd_long length_pos, sizeofdata_pos, prev_stream_pos, remain_length;
	psd_ushort ID;
	psd_uint tag;
	psd_uchar sizeofname;
	psd_int sizeofdata;

	length_pos = psd_ftell(context->out_file);
	// Length of image resource section
	full_length = length = psd_stream_get_int(context);
	if(length <= 0)
		return psd_status_done;

	while(length > 0)
	{
		// Signature: '8BIM'
		tag = psd_stream_get_int(context);
		if(tag == '8BIM')
		{
			length -= 4;
			// Unique identifier for the resource
			ID = psd_stream_get_short(context);
			length -= 2;
			// Name: Pascal string, padded to make the size even (a null name consists of two bytes of 0)
			sizeofname = psd_stream_get_char(context);
			if((sizeofname & 0x01) == 0)
				sizeofname ++;
			psd_stream_get_null(context, sizeofname);
			length -= sizeofname + 1;
			
			sizeofdata_pos = psd_ftell(context->out_file);
			// Actual size of resource data that follows
			sizeofdata = psd_stream_get_int(context);
			length -= 4;
			// resource data must be even
			if(sizeofdata & 0x01)
				sizeofdata ++;
			length -= sizeofdata;

			prev_stream_pos = context->stream->current_pos;

			if(sizeofdata > 0)
			{
				if (ID == 1060)
				{
					psd_char * metadata = (psd_char *)psd_malloc(sizeofdata+1);
					if (metadata == NULL)
						return psd_status_malloc_failed;
					metadata[sizeofdata] = 0;

					context->stream->autowrite = psd_false;
					psd_stream_get(context, (psd_uchar*)metadata, sizeofdata);

					psd_char * cut_metadata = makeCutMetadata(metadata, sizeofdata);
					psd_int new_sizeofdata = strlen(cut_metadata);
					
					// writeToTempFile(metadata, strlen(metadata), context->out_file_name, ".xml");
					// 	writeToTempFile(cut_metadata, new_sizeofdata, context->out_file_name, "_cut.xml");

					if (psd_fwrite((psd_uchar*)cut_metadata, new_sizeofdata, context->out_file) != new_sizeofdata)
						return psd_status_out_file_error;

					status = psd_stream_write_new_size_int(context, sizeofdata, 2, sizeofdata_pos, sizeofdata_pos+4);

					psd_free(cut_metadata);
					psd_free(metadata);
				}

				// Filler
				remain_length = prev_stream_pos + sizeofdata - context->stream->current_pos;
				if (psd_stream_get_null(context, remain_length) != remain_length)
					status = psd_status_resource_error;

				context->stream->autowrite = psd_true;

				if (status != psd_status_done)
					return status;
			}
		}
		else
		{
			return psd_status_resource_signature_error;
		}
	}

	status = psd_stream_write_new_size_int(context, full_length, 0, length_pos, length_pos+4);

	return status;
}

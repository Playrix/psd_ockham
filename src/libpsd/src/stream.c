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

#include <string.h>

static
#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
psd_bool psd_test_big_endian(void)
{
	static psd_bool result = 2;
	const psd_int byte_order[1] = { 1 };

	if (result == 2)
	{
		if(((char *)byte_order)[0] == 1)
			result = psd_false;
		else
			result = psd_true;
	}

	return result;
}

psd_int psd_stream_get(psd_context * context, psd_uchar * buffer, psd_int length)
{
	psd_stream * stream;
	psd_int left, read = 0;

	if(buffer == NULL)
		return 0;
	
	stream = context->stream;
	
	if(stream->buffer == NULL)
	{
		stream->buffer = (psd_uchar *)psd_malloc(PSD_STREAM_MAX_READ_LENGTH);
		if(stream->buffer == NULL)
			return 0;
	}

	left = stream->read_in_length - stream->read_out_length;
	if(left > 0 && left <= length)
	{
		memcpy(buffer, stream->buffer + stream->read_out_length, left);
		if (stream->autowrite)
			psd_fwrite(stream->buffer + stream->read_out_length, left, context->out_file);
		buffer += left;
		length -= left;
		stream->read_out_length = stream->read_in_length;
		read += left;
	}

	if(length > PSD_STREAM_MAX_READ_LENGTH)
	{
		read += psd_fread(buffer, length, context->file);
		if (stream->autowrite)
			psd_fwrite(buffer, length, context->out_file);
		stream->read_out_length = stream->read_in_length;
	}
	else if(stream->read_out_length == stream->read_in_length)
	{
		if(length > 0)
		{
			stream->read_in_length = psd_fread(stream->buffer, PSD_STREAM_MAX_READ_LENGTH, context->file);
			if(length > stream->read_in_length)
				length = stream->read_in_length;
			memcpy(buffer, stream->buffer, length);
			if (stream->autowrite)
				psd_fwrite(stream->buffer, length, context->out_file);
			read += length;
			stream->read_out_length = length;
		}
	}
	else
	{
		left = stream->read_in_length - stream->read_out_length;
		if(length > left)
			length = left;
		memcpy(buffer, stream->buffer + stream->read_out_length, length);
		if (stream->autowrite)
			psd_fwrite(stream->buffer + stream->read_out_length, length, context->out_file);
		stream->read_out_length += length;
		read += length;
	}

	stream->current_pos += read;

	return read;
}

psd_int psd_stream_get_w_null(psd_context * context, psd_int length)
{
	psd_long k = sizeof(psd_wchar) / sizeof(psd_uchar);
	psd_long count = psd_stream_get_null(context, k * length);
	return (psd_int)count;
}

psd_long psd_stream_get_null(psd_context * context, psd_long length)
{
	psd_stream * stream;
	psd_int left;
	psd_long read = 0;

	if(length <= 0)
		return 0;

	stream = context->stream;
	
	if(stream->buffer == NULL)
	{
		stream->buffer = (psd_uchar *)psd_malloc(PSD_STREAM_MAX_READ_LENGTH);
		if(stream->buffer == NULL)
			return 0;
	}

	left = (psd_int)(stream->read_in_length - stream->read_out_length);
	if(left > 0 && left <= length)
	{
		if (stream->autowrite)
			psd_fwrite(stream->buffer + stream->read_out_length, left, context->out_file);
		length -= left;
		stream->read_out_length = stream->read_in_length;
		read += left;
	}

	if(length > PSD_STREAM_MAX_READ_LENGTH)
	{
		psd_long cur_length = length;
		while (cur_length >= PSD_STREAM_MAX_READ_LENGTH)
		{
			stream->read_in_length = psd_fread(stream->buffer, PSD_STREAM_MAX_READ_LENGTH, context->file);
			if (stream->autowrite)
				psd_fwrite(stream->buffer, PSD_STREAM_MAX_READ_LENGTH, context->out_file);
			cur_length -= PSD_STREAM_MAX_READ_LENGTH;
			read += PSD_STREAM_MAX_READ_LENGTH;
		}
		stream->read_out_length = stream->read_in_length;
		length = cur_length;
	}

	if(stream->read_out_length == stream->read_in_length)
	{
		if(length > 0)
		{
			stream->read_in_length = psd_fread(stream->buffer, PSD_STREAM_MAX_READ_LENGTH, context->file);
			if(length > stream->read_in_length)
				length = stream->read_in_length;
			if (stream->autowrite)
				psd_fwrite(stream->buffer, (psd_int)length, context->out_file);
			read += length;
			stream->read_out_length = (psd_int)length;
		}
	}
	else
	{
		left = (psd_int)(stream->read_in_length - stream->read_out_length);
		if(length > left)
			length = left;
		if(stream->autowrite)
			psd_fwrite(stream->buffer + stream->read_out_length, (psd_int)length, context->out_file);
		stream->read_out_length += (psd_int)length;
		read += length;
	}

	stream->current_pos += read;

	return read;
}

psd_bool psd_stream_get_bool(psd_context * context)
{
	return psd_stream_get_char(context) > 0 ? psd_true : psd_false;
}

psd_uchar psd_stream_get_char(psd_context * context)
{
	psd_uchar str[4];
	psd_uchar value = 0;

	if(psd_stream_get(context, str, 1) == 1)
		value = str[0];

	return value;
}

psd_short psd_stream_get_short(psd_context * context)
{
	psd_uchar str[4];
	psd_short value = 0;

	if(psd_stream_get(context, str, 2) == 2)
		value = PSD_CHAR_TO_SHORT(str);

	return value;
}

psd_int psd_stream_get_int(psd_context * context)
{
	psd_uchar str[4];
	psd_int value = 0;

	if(psd_stream_get(context, str, 4) == 4)
		value = PSD_CHAR_TO_INT(str);

	return value;
}

psd_long psd_stream_get_long(psd_context * context)
{
	psd_uchar str[8];
	psd_long value = 0;

	if(psd_stream_get(context, str, 8) == 8)
	{
		for (psd_int i = 0; i < 8; i++)
		{
			value = (value << 8) | str[i];
		}
	}

	return value;
}

psd_float psd_stream_get_float(psd_context * context)
{
	psd_uchar str[4], *dst;
	psd_float value = 0.0;
	psd_int i;

	if(psd_stream_get(context, str, 4) == 4)
	{
		if (psd_test_big_endian() == psd_true)
		{
			memcpy(&value, str, 4);
		}
		else
		{
			dst = (psd_uchar *)&value;
			for(i = 0; i < 4; i ++)
			{
				*dst++ = str[3 - i];
			}
		}
	}

	return value;
}

psd_double psd_stream_get_double(psd_context * context)
{
	psd_uchar str[8], *dst;
	psd_double value = 0.0;
	psd_int i;

	if(psd_stream_get(context, str, 8) == 8)
	{
		if (psd_test_big_endian() == psd_true)
		{
			memcpy(&value, str, 8);
		}
		else
		{
			dst = (psd_uchar *)&value;
			for(i = 0; i < 8; i ++)
			{
				*dst++ = str[7 - i];
			}
		}
	}

	return value;
}

void psd_stream_free(psd_context * context)
{
	if (context == NULL)
		return;
	
	if (context->stream != NULL)
	{

		if (context->stream->buffer != NULL)
		{
			psd_free(context->stream->buffer);
		}
		psd_free(context->stream);
		context->stream = NULL;
	}

	if (context->file >= 0)
	{
		psd_fclose(context->file);
		context->file = -1;
	}
	if (context->out_file >= 0)
	{
		psd_fclose(context->out_file);
		context->out_file = -1;
	}

}

psd_int psd_stream_write_int(psd_context * context, psd_int value)
{
	psd_uchar str[4];
	str[0] = (value >> 24) & 0xFF;
	str[1] = (value >> 16) & 0xFF;
	str[2] = (value >> 8)  & 0xFF;
	str[3] = (value)       & 0xFF;
	
	psd_int size = (psd_int)psd_fwrite(str, 4, context->out_file);
	return size;
}

psd_int psd_stream_write_long(psd_context * context, psd_long value)
{
	psd_uchar str[8];
	str[0] = (value >> 56) & 0xFF;
	str[1] = (value >> 48) & 0xFF;
	str[2] = (value >> 40) & 0xFF;
	str[3] = (value >> 32) & 0xFF;
	str[4] = (value >> 24) & 0xFF;
	str[5] = (value >> 16) & 0xFF;
	str[6] = (value >> 8)  & 0xFF;
	str[7] = (value)       & 0xFF;
	
	psd_int size = (psd_int)psd_fwrite(str, 8, context->out_file);
	return size;
}


psd_long psd_stream_write_null(psd_context * context, psd_long count)
{
	psd_long result = 0;
	psd_uchar zero = 0;
	while (count--)
	{
		result += psd_fwrite(&zero, 1, context->out_file);
	}
	return result;
}

psd_status _psd_stream_write_new_size(psd_context * context, psd_long old_length, psd_char rounding, psd_long write_pos, psd_long data_start_pos, psd_bool is_long)
{
	psd_long cur_pos = psd_ftell(context->out_file);
	psd_long length = cur_pos - data_start_pos;
	psd_long rounded_length = length;

	if (rounding != 0)
	{
		if (length % rounding != 0)
		{
			rounded_length += rounding - (length % rounding);
		}
	}

	if (length != old_length)
	{
		if (psd_fseek_set(context->out_file, write_pos) != write_pos)
			return psd_status_out_file_error;

		if (is_long)
		{
			if (psd_stream_write_long(context, length) != 8)
				return psd_status_out_file_error;
		}
		else
		{
			if (psd_stream_write_int(context, (psd_int)length) != 4)
				return psd_status_out_file_error;
		}

		if (psd_fseek_end(context->out_file, 0) < 0)
			return psd_status_out_file_error;
	}

	if (length != rounded_length)
	{
		psd_long count = rounded_length - length;
		if (psd_stream_write_null(context, count) != count)
			return psd_status_out_file_error;
	}

	return psd_status_done;
}

psd_status psd_stream_write_new_size_int(psd_context * context, psd_int old_length, psd_char rounding, psd_long write_pos, psd_long data_start_pos)
{
	return _psd_stream_write_new_size(context, old_length, rounding, write_pos, data_start_pos, psd_false);
}


psd_status psd_stream_write_new_size_long(psd_context * context, psd_long old_length, psd_char rounding, psd_long write_pos, psd_long data_start_pos)
{
	return _psd_stream_write_new_size(context, old_length, rounding, write_pos, data_start_pos, psd_true);
}

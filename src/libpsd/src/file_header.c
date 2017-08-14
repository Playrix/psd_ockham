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
#include "stream.h"
#include "psd_system.h"

psd_status psd_get_file_header(psd_context * context)
{
	// Signature: always equal to '8BPS'
	psd_int signature = psd_stream_get_int(context);

	if(signature != '8BPS')
		return psd_status_file_signature_error;

	// Version: always equal to 1
	context->version = psd_stream_get_short(context);
	if(context->version != 1 && context->version != 2)
		return psd_status_file_version_error;

	psd_stream_get_null(context, 6);

	// The number of channels in the image, including any alpha channels
	psd_short channels = psd_stream_get_short(context);
	if(!(channels >= 1 && channels <= 56))
		return psd_status_file_header_error;

	// The height of the image in pixels
	psd_int height = psd_stream_get_int(context);
	if(!(height >= 1 && height <= 300000))
		return psd_status_file_header_error;

	// The width of the image in pixels
	psd_int width = psd_stream_get_int(context);
	if(!(width >= 1 && width <= 300000))
		return psd_status_file_header_error;
	
	psd_stream_get_short(context);
	psd_stream_get_short(context);

	return psd_status_done;
}

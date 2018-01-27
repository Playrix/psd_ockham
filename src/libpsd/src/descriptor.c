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

#include <math.h>
#include "psd.h"
#include "psd_system.h"
#include "descriptor.h"
#include "stream.h"


static psd_status psd_stream_get_typed_object(psd_context * context, psd_uint type);

void psd_stream_get_object_wstring_null(psd_context * context)
{
	psd_int length = psd_stream_get_int(context);
	psd_stream_get_w_null(context, length);
}

void psd_stream_get_object_string_null(psd_context * context)
{
	psd_int length = psd_stream_get_int(context);
	if (length == 0)
		length = 4;

	psd_stream_get_null(context, length);
}

// 'prop' = Property
static void psd_stream_get_object_property(psd_context * context)
{
	psd_stream_get_object_wstring_null(context);
	psd_stream_get_object_string_null(context);
	psd_stream_get_object_string_null(context);
}

// 'type' or GlbC'= Class
static void psd_stream_get_object_class(psd_context * context)
{
	psd_stream_get_object_wstring_null(context);
	psd_stream_get_object_string_null(context);
}

// 'rele' = Offset
static void psd_stream_get_object_offset(psd_context * context)
{
	psd_stream_get_object_wstring_null(context);
	psd_stream_get_object_string_null(context);
	psd_stream_get_int(context);
}

// 'Enmr' = Enumerated Reference
static void psd_stream_get_object_enumerated_reference(psd_context * context)
{
	psd_stream_get_object_wstring_null(context);
	psd_stream_get_object_string_null(context);
	psd_stream_get_object_string_null(context);
	psd_stream_get_object_string_null(context);
}

// 'obj ' = Reference
static psd_status psd_stream_get_object_reference(psd_context * context)
{
	psd_uint type;
	psd_int number_items;

	number_items = psd_stream_get_int(context);

	while (number_items--)
	{
		type = psd_stream_get_int(context);

		switch (type)
		{
		case 'prop':    // Property
			psd_stream_get_object_property(context);
			break;
		case 'Clss':    // Class
			psd_stream_get_object_class(context);
			break;
		case 'Enmr':    // Enumerated Reference
			psd_stream_get_object_enumerated_reference(context);
			break;
		case 'rele':    // Offset
			psd_stream_get_object_offset(context);
			break;
		case 'Idnt':    // Identifier
		case 'indx':    // Index
			psd_stream_get_int(context);
			break;
		case 'name':    // Name
			psd_stream_get_object_wstring_null(context);
			break;

		default:
			return psd_status_unknown_object_reference;
		}
	}

	return psd_status_done;
}

// 'UnFl' = Unit floats
static void psd_stream_get_object_unit_floats(psd_context * context)
{
	psd_stream_get_int(context);
	psd_int number_items = psd_stream_get_int(context);

	while (number_items--)
	{
		psd_stream_get_double(context);
	}
}

// 'enum'= Enumerated
static void psd_stream_get_object_enumerated(psd_context * context)
{
	psd_stream_get_object_string_null(context);
	psd_stream_get_object_string_null(context);
}

// 'tdta' = Raw Data
void psd_stream_get_object_raw_data(psd_context * context)
{
	psd_int length = psd_stream_get_int(context);
	psd_stream_get_null(context, length);
}

// 'Objc' = Descriptor
psd_status psd_stream_get_object_descriptor(psd_context * context)
{
	psd_uint type;
	psd_int number_items;
	psd_status status;

	psd_stream_get_object_wstring_null(context);
	psd_stream_get_object_string_null(context);

	number_items = psd_stream_get_int(context);

	while (number_items--)
	{
		psd_stream_get_object_string_null(context);
		type = psd_stream_get_int(context);
		status = psd_stream_get_typed_object(context, type);
		if (status != psd_status_done)
			return status;
	}

	return psd_status_done;
}

// 'VlLs' = List
static psd_status psd_stream_get_object_list(psd_context * context)
{
	psd_uint type;
	psd_int number_items;
	psd_status status;

	number_items = psd_stream_get_int(context);

	while (number_items--)
	{
		type = psd_stream_get_int(context);
		status = psd_stream_get_typed_object(context, type);
		if (status != psd_status_done)
			return status;
	}

	return psd_status_done;
}

// 'ObAr' = Object Array
static psd_status psd_stream_get_object_object_array(psd_context * context)
{
	psd_int key;
	psd_status status;

	psd_stream_get_int(context);
	
	psd_stream_get_object_class(context);
	
	psd_int number_items = psd_stream_get_int(context);

	while (number_items--)
	{
		psd_stream_get_object_string_null(context);
		key = psd_stream_get_int(context);
		status = psd_stream_get_typed_object(context, key);
		if (status != psd_status_done)
			return status;
	}

	return psd_status_done;
}

static psd_status psd_stream_get_typed_object(psd_context * context, psd_uint type)
{
	psd_status status = psd_status_done;
	switch (type)
	{
	case 'obj ':    // Reference
		status = psd_stream_get_object_reference(context);
		break;
	case 'Objc':    // Descriptor
	case 'GlbO':    // GlobalObject same as Descriptor
		status = psd_stream_get_object_descriptor(context);
		break;
	case 'VlLs':    // List
		status = psd_stream_get_object_list(context);
		break;
	case 'doub':    // Double
		psd_stream_get_double(context);
		break;
	case 'UntF':    // Unit float
		psd_stream_get_int(context);
		psd_stream_get_double(context);
		break;
	case 'UnFl':    // Unit floats
		psd_stream_get_object_unit_floats(context);
		break;
	case 'TEXT':    // String
		psd_stream_get_object_wstring_null(context);
		break;
	case 'enum':    // Enumerated
		psd_stream_get_object_enumerated(context);
		break;
	case 'long':    // Integer
		psd_stream_get_int(context);
		break;
	case 'bool':    // Boolean
		psd_stream_get_bool(context);
		break;
	case 'type':    // Class
	case 'GlbC':    // Class
		psd_stream_get_object_class(context);
		break;
	case 'alis':    // Alias
	case 'tdta':    // Raw Data
		psd_stream_get_object_raw_data(context);
		break;
	case 'ObAr':    // Object Array
		status = psd_stream_get_object_object_array(context);
		break;
	default:
		status = psd_status_unknown_typed_object;
		break;
	}
	return status;
}


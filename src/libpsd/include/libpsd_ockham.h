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

#ifndef __LIB_PSD_OCKHAM_H__
#define __LIB_PSD_OCKHAM_H__

#ifdef __cplusplus
extern "C" {
#endif

// global status
typedef enum {
	psd_status_done								= 0,
	psd_status_unkown_error						= 1,
	psd_status_malloc_failed					= 2,
	psd_status_out_file_error					= 3,
	psd_status_fread_error						= 4,
	
	psd_status_invalid_file						= 10,
	psd_status_invalid_out_file					= 11,
	
	psd_status_file_signature_error				= 20,
	psd_status_file_version_error				= 21,
	psd_status_file_header_error				= 22,
	
	psd_status_image_resource_error				= 30,
	psd_status_layer_and_mask_error 			= 31,
	psd_status_resource_signature_error			= 32,
	psd_status_resource_error					= 33,
	psd_status_color_mode_data_error			= 34,
	psd_status_image_data_error					= 35,
	psd_status_unknown_object_reference			= 36,
	psd_status_unknown_typed_object				= 37,
	psd_status_linked_layer_error				= 38,
	psd_status_layer_info_error					= 39,
	psd_status_mask_info_error					= 40,

} psd_status;

typedef struct _psd_result
{
	psd_status					status;
	char *						out_file;
} psd_result;

psd_result psd_process_file(const char * file_name, const char * out_file_name);
psd_status psd_image_load(const char * file_name, const char * out_file_name);

void psd_get_error_message(char* buffer, psd_status status, const char* out_file_name);

#ifdef __cplusplus
}
#endif

#endif //ifndef __LIB_PSD_OCKHAM_H__

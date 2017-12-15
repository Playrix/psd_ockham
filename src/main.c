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

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "libpsd_ockham.h"

static const char* const VERSION = "1.08";

static const char* const FNAME_POSTFIX = "_cut";
static const int EXT_LENGTH = 4;

void print_help()
{
	puts("Usage: psd_ockham INPUT_FILE [OUTPUT_FILE]");
	puts("");
	puts("psd_ockham reduces size of psd files by removing excessive parts of metadata");
	printf("Version %s\n", VERSION);
}

void print_error(psd_status error)
{
	const char* error_message = "Unknown error";
	switch (error)
	{
		case psd_status_invalid_file:
			error_message = "Can't open source file";
			break;
		case psd_status_invalid_out_file:
			error_message = "Can't open destination file";
			break;

		case psd_status_malloc_failed:
			error_message = "Memory allocation failed";
			break;
		case psd_status_out_file_error:
			error_message = "Writing to destination file failed";
			break;
		case psd_status_fread_error:
			error_message = "Reading from source file failed";
			break;

		case psd_status_file_signature_error:
		case psd_status_file_version_error:
		case psd_status_file_header_error:
			error_message = "Broken or unsupported PSD file";
			break;

		case psd_status_image_resource_error:
		case psd_status_layer_and_mask_error:
		case psd_status_resource_signature_error:
		case psd_status_resource_error:
		case psd_status_color_mode_data_error:
		case psd_status_image_data_error:
		case psd_status_unknown_object_reference:
		case psd_status_unknown_typed_object:
		case psd_status_linked_layer_error:
		case psd_status_layer_info_error:
		case psd_status_mask_info_error:
			error_message = "Error parsing PSD";

		default:
			break;
	}

	printf("Error %02d: %s\n", error, error_message);
}

int main(int argc, char **argv)
{
	if (argc < 2 || argc > 3)
	{
		print_help();
		return 1;
	}

	char* input = argv[1];
	char* output = 0;

	if (argc == 3)
	{
		output = argv[2];
	}
	else
	{
		size_t input_len = strlen(input);
		char* dot_pos = strrchr(input, '.');

		size_t output_len = input_len + strlen(FNAME_POSTFIX) + 1;
		output = (char *)malloc(output_len);

		if (dot_pos != NULL && dot_pos + EXT_LENGTH >= input + input_len)
		{
			size_t dot_index = dot_pos - input;

			strncpy(output, input, dot_index);
			strcpy(output + dot_index, FNAME_POSTFIX);
			strcpy(output + dot_index + strlen(FNAME_POSTFIX), input + dot_index);
		}
		else
		{
			strcpy(output, input);
			strcpy(output + input_len, FNAME_POSTFIX);
		}
	}

	char* tmp_file = tmpnam(0);

	psd_status status = psd_image_load(input, tmp_file);

	if (status != psd_status_done) {
		print_error(status);
		remove(tmp_file);
		return 1;
	}

	remove(output);

	if (rename(tmp_file, output) != 0) {
		print_error(psd_status_invalid_out_file);
		remove(tmp_file);
		return 1;
	}

	return 0;
}

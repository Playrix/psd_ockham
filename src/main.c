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

#include <stdio.h>
#include <stdlib.h>

#include "libpsd_ockham.h"
#include "version.h"

void print_help()
{
	puts("Usage: psd_ockham INPUT_FILE [OUTPUT_FILE]");
	puts("");
	puts("psd_ockham reduces size of psd files by removing excessive parts of metadata");
	printf("Version %s\n", PSD_OCKHAM_VERSION);
}

int main(int argc, char **argv)
{
	if (argc < 2 || argc > 3)
	{
		print_help();
		return 1;
	}

	const char* input = argv[1];
	const char* output = NULL;

	if (argc == 3)
	{
		output = argv[2];
	}
	
	psd_result result = psd_process_file(input, output);

	if (result.status != psd_status_done)
	{
		char* error_message = (char*)malloc(2048);
		psd_get_error_message(error_message, result.status, result.out_file);
		puts(error_message);
		return 1;
	}

	return 0;
}

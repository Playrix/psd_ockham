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

#ifndef __TYPES_H__
#define __TYPES_H__

#include <stddef.h>

typedef unsigned char			psd_bool;
#define psd_true				1
#define psd_false				0


typedef char					psd_char;
typedef unsigned char			psd_uchar;
typedef unsigned short			psd_wchar;
typedef short					psd_short;
typedef unsigned short			psd_ushort;
typedef int						psd_int;
typedef unsigned int			psd_uint;
typedef long long				psd_long;
typedef float					psd_float;
typedef double					psd_double;

#endif

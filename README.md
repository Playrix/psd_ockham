> Entities are not to be multiplied without necessity

_Occam's razor_

----

**psd_ockham** is a command-line utility that reduces Photoshop .psd and .psb file size by removing parts of excessive XMP metadata.

Latter versions of Photoshop have issue when XMP gets bloated with useless identifiers in the `<photoshop:DocumentAncestors>` element (https://forums.adobe.com/thread/1983397). Sometimes the size of blotated metadata exceeds the size of the rest file multiple times.

Utility does not require Photoshop and can be run on Windows, MacOS and Unix.

## Usage

```
psd_ockham SOURCE_FILE [DESTINATION_FILE]
```

Results will be written to destination file. If there's no destination file provided, results will be written to new file near source file with suffix `_cut`.

Running psd_ockham without parameters prints help message and version.

## Copyright

Copyright © 2017 Playrix.

psd_ockham is based on [libpsd](https://sourceforge.net/projects/libpsd/)

Copyright © 2004-2007 Graphest Software.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

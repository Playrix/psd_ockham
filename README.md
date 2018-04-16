> Entities are not to be multiplied without necessity

_Ockham's razor_

----

![Icon](src-gui/mac/res/Images.xcassets/AppIcon.appiconset/ico_128.png)

**psd_ockham** is a command-line utility that reduces Photoshop .psd and .psb file size by removing parts of excessive XMP metadata.

Latter versions of Photoshop have issue when XMP gets bloated with numerous identifiers in the `<photoshop:DocumentAncestors>` element (https://forums.adobe.com/thread/1983397). These tags doesn't contain any useful information and can be safely removed. Sometimes psd files get extremely large and the size of bloated metadata exceeds the size of the rest of the file multiple times.

psd_ockham removes all `DocumentAncestors`, `rdf:Bag` and `rdf:li` tags from metadata in psd and included smart-objects. It doesn't change psd structure, contents of the file or graphical layers.

Utility does not require Photoshop and can be run on Windows, MacOS and Unix.

## Usage from command line

```
psd_ockham SOURCE_FILE [DESTINATION_FILE]
```

Results will be written to destination file. If there's no destination file provided, results will be written to new file near source file with suffix `_cut`.

Running psd_ockham without parameters prints help message and version.

## Usage from GUI

Version with graphical user interface is provided for Windows only. It is simple and straightforward, just drag-n-drop files and folders onto main window. GUI version is independent and doesn't require command-line executable.

## Copyright

Copyright © 2017-2018 Playrix.

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

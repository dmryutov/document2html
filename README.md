# Document2HTML converter

[![Build Status](https://travis-ci.org/dmryutov/document2html.svg?branch=master)](https://travis-ci.org/dmryutov/document2html)
[![Build status](https://ci.appveyor.com/api/projects/status/o3p0or25fdlnw22v?svg=true)](https://ci.appveyor.com/project/dmryutov/document2html)
[![Github Releases](https://img.shields.io/github/release/dmryutov/document2html.svg)](https://github.com/dmryutov/document2html/releases)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue.svg)](http://dmryutov.github.io/document2html)
[![MIT Licence](https://badges.frapsoft.com/os/mit/mit.svg?v=103)](https://opensource.org/licenses/mit-license.php)

Documents to HTML converter

## Features
| Extension | Text | Styles extraction | Images extraction |
|   :---:   | :---:|       :---:       |       :---:       |
| [HTML/XHTML](https://github.com/dmryutov/converter/tree/master/src/libs/fileext/html) | Yes  | Yes               | Yes               |
| [XML](https://github.com/dmryutov/converter/tree/master/src/libs/fileext/xml) | Yes  | Not applicable    | Not applicable    |
| [DOCX](https://github.com/dmryutov/converter/tree/master/src/libs/fileext/docx) | Yes  | Yes               | Yes               |
| [DOC](https://github.com/dmryutov/converter/tree/master/src/libs/fileext/doc) | Yes  | No                | No                |
| [RTF](https://github.com/dmryutov/converter/tree/master/src/libs/fileext/rtf) | Yes  | Yes               | Yes               |
| [ODT](https://github.com/dmryutov/converter/tree/master/src/libs/fileext/odt) | Yes  | Yes               | Yes               |
| [XLSX](https://github.com/dmryutov/converter/tree/master/src/libs/fileext/excel) | Yes  | Yes               | Yes               |
| [XLS](https://github.com/dmryutov/converter/tree/master/src/libs/fileext/excel) | Yes  | Yes               | No                |
| [CSV](https://github.com/dmryutov/converter/tree/master/src/libs/fileext/csv) | Yes  | Not applicable    | Not applicable    |
| [TXT/MD](https://github.com/dmryutov/converter/tree/master/src/libs/fileext/txt) | Yes  | Yes               | Yes               |
| [JSON](https://github.com/dmryutov/converter/tree/master/src/libs/fileext/json) | Yes  | Not applicable    | Not applicable    |
| [EPUB](https://github.com/dmryutov/converter/tree/master/src/libs/fileext/epub) | Yes  | Yes               | Yes               |
| [PDF](https://github.com/dmryutov/converter/tree/master/src/libs/fileext/pdf) | Yes  | No                | Yes               |
| [PPT](https://github.com/dmryutov/converter/tree/master/src/libs/fileext/ppt) | Yes  | No                | No                |

## Dependencies
__cURL__ for downloading images:
```
apt-get install libcurl4-openssl-dev
or
brew install curl
```
__iconv__ for encoding conversion
```
sudo apt-get install libc6
or
brew install libiconv
```
__Tidy__ for cleaning and repairing HTML
```
sudo apt-get install libtidy-dev
or
brew install tidy-html5
```
__file__ for determining file extension

## Third-party
- [getoptpp](https://github.com/timstaley/getoptpp) - Command line options parser
- [lodepng](https://github.com/lvandeve/lodepng) - PNG encoder and decoder
- [miniz](https://github.com/richgel999/miniz) - Data compression library
- [json](https://github.com/nlohmann/json) - JSON parser
- [pygixml](https://github.com/zeux/pugixml) - XML parser

## Building
Make sure the Qt (>= 5.6) development libraries are installed:

* In Ubuntu/Debian: `apt-get install qt5-default qttools5-dev-tools zlib1g-dev`
* In Fedora:        `sudo dnf builddep tiled`
* In Arch Linux:    `pacman -S qt`
* In Mac OS X with [Homebrew](http://brew.sh/):
  + `brew install qt5`
  + `brew link qt5 --force`
* Or you can download Qt from: https://www.qt.io/download-open-source/

Now you can compile by running:
```
qmake (or qmake-qt5 on some systems)
make
```
To do a shadow build, you can run qmake from a different directory and refer
it to space-invaders.pro, for example:
```
mkdir build
cd build
qmake ../src/document2html.pro
make
```
If you have ideas how to build project with CMake instead of Qt please contact me.

## Tool usage
Usage:
```
    document2html -f|-d <input file|dir> -o <output dir> [-si]
    document2html -h
    document2html -v
```
Options:

| Short Flag | Long Flag | Description             |
| :---:      | :---:     | :---                    |
| -f         | --file    | Input file              |
| -d         | --dir     | Input directory         |
| -o         | --out     | Output directory        |
| -s         | --style   | Extract styles          |
| -i         | --image   | Extract images          |
| -h         | --help    | Display help message    |
| -v         | --version | Display package version |

## Thanks
- [rembish](https://github.com/rembish/TextAtAnyCost) - DOC, PPT and PDF converter (PHP)
- [PolicyStat](https://github.com/PolicyStat/docx2html) - DOCX converter (Python)
- [python-excel](https://github.com/python-excel/xlrd) - XLSX and XLS converter (Python)
- [lvu](https://github.com/lvu/rtf2html) - RTF converter (C++)
- [adhocore](https://github.com/adhocore/htmlup) - TXT/MD converter (PHP)
- [ahupp](https://github.com/ahupp/python-magic) - libmagic wrapper (Python)

## Contact
If you have questions regarding the libraries, I would like to invite you to open an issue at Github. Please describe your request, problem, or question as detailed as possible, and also mention the version of the libraries you are using as well as the version of your compiler and operating system. Opening an issue at Github allows other users and contributors to this libraries to collaborate.

You're welcome! :)

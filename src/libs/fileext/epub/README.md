# EPUB2HTML

EPUB files into HTML сonverter

## Usage:
```
#include "epub/epub.hpp"

epub::Epub document("test.epub");
document.convert(true, true, 0);
document.saveHtml("out_dir", "test.html");
```

## Features
| Text | Styles extraction | Images extraction |
| :---:|       :---:       |       :---:       |
| Yes  | Yes               | Yes               |

## Dependencies
None

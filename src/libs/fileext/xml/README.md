# XML2HMTL

XML files into HTML сonverter

## Features
| Text | Styles extraction | Images extraction |
| :---:|       :---:       |       :---:       |
| Yes  | Not applicable    | Not applicable    |

## Usage:
```
#include "xml/xml.hpp"

xml::Xml document("test.xml");
document.convert(true, true, 0);
document.saveHtml("out_dir", "test.html");
```

## Dependencies
None

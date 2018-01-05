# HTML

HTML/XHTML files cleaner

## Usage:
```
#include "html/html.hpp"

html::Html document("test.html");
document.convert(true, true, 0);
document.saveHtml("out_dir", "test.html");
```

## Features
| Text | Styles extraction | Images extraction |
| :---:|       :---:       |       :---:       |
| Yes  | Yes               | Yes               |

## Dependencies
- cURL
- Tidy

# JSON2HTML

JSON files into HTML сonverter

## Usage:
```
#include "json/json.hpp"

json::Json document("test.json");
document.convert(true, true, 0);
document.saveHtml("out_dir", "test.html");
```

## Features
| Text | Styles extraction | Images extraction |
| :---:|       :---:       |       :---:       |
| Yes  | Not applicable    | Not applicable    |

## Dependencies
None

![platform](https://img.shields.io/static/v1?label=platform&message=mac-intel%20|%20mac-arm%20|%20win-64&color=blue)
[![license](https://img.shields.io/github/license/miyako/pdfium-parser)](LICENSE)
![downloads](https://img.shields.io/github/downloads/miyako/pdfium-parser/total)

### Dependencies and Licensing

* the source code of this CLI tool is licensed under the MIT license.
* see [pdfium](https://pdfium.googlesource.com/pdfium/+/main/LICENSE) for the licensing of **pdfium** (Apache License 2.0).
 
# pdfium-parser
CLI tool to extract text from PDF

## usage

```
pdfium-parser -i example.pdf -o example.json

 -i path    : document to parse
 -o path    : text output (default=stdout)
 -          : use stdin for input
 -r         : raw text output (default=json)
 -p pass    : password if encrypted
```

## output (JSON)

```
{
    "type: "pdf",
    "pages": [{array of string}]
}
```

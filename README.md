# pdfium-parser
CLI tool to extract text from PDF

### Dependencies and Licensing

* the source code of this CLI tool is licensed under the MIT license.
* **pdfium** is primarily licensed under the [Apache License 2.0](https://pdfium.googlesource.com/pdfium/+/main/LICENSE) license.

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

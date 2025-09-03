# pdfium-parser
CLI tool to extract text from PDF

## usage

```
pdfium-parser -i example.pdf -o example.json

 -i path: document to parse
 -o path: text output (default=stdout)
 -: use stdin for input
 -r: raw text output (default=json)
 -p password: password if encrypted
```

## output (JSON)

```
{
    "type: "pdf",
    "pages": [{array of string}]
}
```

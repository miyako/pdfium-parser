//
//  main.cpp
//  pdfium-parser
//
//  Created by miyako on 2025/09/03.
//

#include "pdfium-parser.h"

static void usage(void)
{
    fprintf(stderr, "Usage:  pdfium-parser -r -i in -o out -\n\n");
    fprintf(stderr, "text extractor for pdf documents\n\n");
    fprintf(stderr, " -%c path: %s\n", 'i' , "document to parse");
    fprintf(stderr, " -%c path: %s\n", 'o' , "text output (default=stdout)");
    fprintf(stderr, " %c: %s\n", '-' , "use stdin for input");
    fprintf(stderr, " -%c: %s\n", 'r' , "raw text output (default=json)");
    fprintf(stderr, " -%c: %s\n", 'p' , "password");
    exit(1);
}

extern OPTARG_T optarg;
extern int optind, opterr, optopt;

#ifdef WIN32
optarg = 0;
opterr = 1;
optind = 1;
optopt = 0;
int getopt(int argc, OPTARG_T *argv, OPTARG_T opts) {

    static int sp = 1;
    register int c;
    register OPTARG_T cp;
    
    if(sp == 1)
        if(optind >= argc ||
             argv[optind][0] != '-' || argv[optind][1] == '\0')
            return(EOF);
        else if(wcscmp(argv[optind], L"--") == NULL) {
            optind++;
            return(EOF);
        }
    optopt = c = argv[optind][sp];
    if(c == ':' || (cp=wcschr(opts, c)) == NULL) {
        ERR(L": illegal option -- ", c);
        if(argv[optind][++sp] == '\0') {
            optind++;
            sp = 1;
        }
        return('?');
    }
    if(*++cp == ':') {
        if(argv[optind][sp+1] != '\0')
            optarg = &argv[optind++][sp+1];
        else if(++optind >= argc) {
            ERR(L": option requires an argument -- ", c);
            sp = 1;
            return('?');
        } else
            optarg = argv[optind++];
        sp = 1;
    } else {
        if(argv[optind][++sp] == '\0') {
            sp = 1;
            optind++;
        }
        optarg = NULL;
    }
    return(c);
}
#define ARGS (OPTARG_T)L"i:o:-rhp:"
#else
#define ARGS "i:o:-rhp:"
#endif

struct Page {
    std::string text;
};

struct Document {
    std::string type;
    std::vector<Page> pages;
};

static void document_to_json(Document& document, std::string& text, bool rawText) {
    
    if(rawText){
        text = "";
        for (const auto &page : document.pages) {
            text += page.text;
        }
    }else{
        Json::Value documentNode(Json::objectValue);
        documentNode["type"] = document.type;
        documentNode["pages"] = Json::arrayValue;
        
        for (const auto &page : document.pages) {
            documentNode["pages"].append(page.text);
        }
        Json::StreamWriterBuilder writer;
        writer["indentation"] = "";
        text = Json::writeString(writer, documentNode);
    }
}

int main(int argc, OPTARG_T argv[]) {
    
    FPDF_InitLibrary();
    
    const OPTARG_T input_path  = NULL;
    const OPTARG_T output_path = NULL;
    
    std::vector<unsigned char>pdf_data(0);

    int ch;
    std::string text;
    bool rawText = false;
    std::string password;
    
    while ((ch = getopt(argc, argv, ARGS)) != -1){
        switch (ch){
            case 'i':
                input_path  = optarg;
                break;
            case 'o':
                output_path = optarg;
                break;
            case 'p':
                password = optarg;
                break;
            case '-':
            {
                _fseek(stdin, 0, SEEK_END);
                size_t len = (size_t)_ftell(stdin);
                _fseek(stdin, 0, SEEK_SET);
                pdf_data.resize(len);
                fread(pdf_data.data(), 1, pdf_data.size(), stdin);
            }
                break;
            case 'r':
                rawText = true;
                break;
            case 'h':
            default:
                usage();
                break;
        }
    }
        
    if((!pdf_data.size()) && (input_path != NULL)) {
        FILE *f = _fopen(input_path, _rb);
        if(f) {
            _fseek(f, 0, SEEK_END);
            size_t len = (size_t)_ftell(f);
            _fseek(f, 0, SEEK_SET);
            pdf_data.resize(len);
            fread(pdf_data.data(), 1, pdf_data.size(), f);
            fclose(f);
        }
    }
    
    if(!pdf_data.size()) {
        usage();
    }
    
    Document document;
    
    FPDF_DOCUMENT doc = FPDF_LoadMemDocument(
                                             pdf_data.data(),
                                             (int)pdf_data.size(),
                                             password.length() ? password.c_str() : nullptr);

    if(doc){
        
        document.type = "pdf";
        
        int page_count = FPDF_GetPageCount(doc);
        
        auto extract_page_text = [&](int i, std::string& t) {
            
            FPDF_PAGE page = FPDF_LoadPage(doc, i);
            if (!page) return;
            
            FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page);
            int nChars = FPDFText_CountChars(text_page);
            
            std::u16string utf16;
            utf16.resize(nChars);
            
            FPDFText_GetText(text_page, 0, nChars, reinterpret_cast<unsigned short*>(&utf16[0]));
            
            std::string utf8;
            for (char16_t ch : utf16) {
                if (ch == 0) break;
                if (ch < 0x80) {
                    utf8.push_back(static_cast<char>(ch));
                } else if (ch < 0x800) {
                    utf8.push_back(0xC0 | (ch >> 6));
                    utf8.push_back(0x80 | (ch & 0x3F));
                } else {
                    utf8.push_back(0xE0 | (ch >> 12));
                    utf8.push_back(0x80 | ((ch >> 6) & 0x3F));
                    utf8.push_back(0x80 | (ch & 0x3F));
                }
            }
            
            t = utf8;
            
            FPDFText_ClosePage(text_page);
            FPDF_ClosePage(page);
        };
        
        for (int i = 0; i < page_count; ++i) {
            Page page;
            extract_page_text(i, page.text);
            document.pages.push_back(page);
        }
        
        document_to_json(document, text, rawText);

        FPDF_CloseDocument(doc);
    }else{
        std::cerr << "Failed to load PDF!" << std::endl;
        goto end;
    }
        
    if(!output_path) {
        std::cout << text << std::endl;
    }else{
        FILE *f = _fopen(output_path, _wb);
        if(f) {
            fwrite(text.c_str(), 1, text.length(), f);
            fclose(f);
        }
    }

    end:
    
    FPDF_DestroyLibrary();
    
    return 0;
}

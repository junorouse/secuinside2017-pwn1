#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <string>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>

/*
    g++ -o b b.cpp -std=c++11 -I/usr/include/leptonica -I/usr/local/include/tesseract -llept -ltesseract
    (python -c 'print "10593"+"\x00"*4091+open("ex.png").read()+"\n"';cat) | ./b

    https://github.com/tesseract-ocr/tesseract/wiki/Compiling
*/


std::string ReplaceAll(std::string &str, const std::string& from, const std::string& to){
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

int main(int argc, char ** argv) {
    char *outText;
    int fd;
    int fileSize, inputSize;
    unsigned int codeLen;
    char buffer[4096];

    std::string s{};
    std::string s2{};

    scanf("%d", &fileSize);


    char filename[] = "/tmp/prob/image.XXXXXX";
    fd = mkstemp(filename);

    while (fileSize) {
        inputSize = read(0, buffer, 4096);
        write(fd, buffer, inputSize);
        fileSize -= inputSize;
        if (fileSize < 0) {
            fileSize = 0;
        }
    }

    close(fd);

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "kor")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }

    // Open input image with leptonica library
    Pix *image = pixRead(filename);
    api->SetImage(image);

    // Get OCR result
    outText = api->GetUTF8Text();
	s += std::string(outText);
    s2 = ReplaceAll(s, std::string("\n"), std::string(""));
    s2 = ReplaceAll(s2, std::string(" "), std::string(""));

    std::cout << s2 << "\n";

    codeLen = s2.length() / 3;

    printf("codeLen: %d\n", codeLen);

    if (!s.substr(0,3).compare(std::string("냐"))) {
        puts("ok");
    } else {
        puts("no");
    }

    // Destroy used object and release memory
    api->End();
    delete [] outText;
    pixDestroy(&image);

    return 0;
}

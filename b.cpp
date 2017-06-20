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

/*

한글인식 -> Malbolge로  encrypted 매핑 (걍 vm code(숫자로 매핑하는거라 생각하셈))
-> 온라인 저지가 컨셉이라 인풋받는 부분이 있는데 (힙)
-> 여기서 졸라 인풋을 계속 받으면 뒷부분에 main_arena 박힌걸 레지스터가 들고옴
-> 옵코드중에 mov [레지스터], 레지스터가 있는데
-> 저 부분이 인식이 안되는 한글 에를들어 뱕, 밝 뀭 이런걸로 매핑해둘꺼임
-> 그래서 정상적으로는 (input vm code로는 불가능),
-> code저장하는 영역뒤에 주소를 담는 영역을 박아둠
-> 그래서 만약에 그 뒤에 주소가 0x7f3exxx830 이라 치면 30이 저 인스트럭션이면
-> 코드가 이어져서 mov명령이 실행되서 익스댐

*/

using namespace std; // check

// global variables

std::string MYSTDOUT;

unsigned int REGISTERS[32];

unsigned int SAME_FLAG, LESS_FLAG, BIGGER_FLAG;

// functions

std::string ReplaceAll(std::string &str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

int mapping(int idx, std::string &str) {
    return str.compare(std::string("냐"));
}

// check answer WTF

void correctAnswer() {
    cout << "Correct" << endl;
    // if pwn the system, attacker also can get reversing flag.
    system("cat reversing_flag.txt");
}

void wrongAnswer() {
    cout << "Wrong" << endl;
}


void makeTestCase() {
    int fd;

    int buffer;

    int x[20] = {0, };

    fd = open("/dev/urandom", O_RDONLY); // right?

    read(fd, &buffer, 2); // wtf


}

void emulate(int code[]) {
    /*
    code spec:

    GET REGISTER[index]
    PRT REGISTER[index]

    CMP REGISTER[index], REGISTER[index]
    JMP REGISTER[index]
    SAME_JMP REGISTER[index]
    LESS_JMP REGISTER[index]
    BIGGER_JMP REGISTER[index]

    MOV REGISTER[index], REGISTER[index]
    
    INC REGISTER[index]
    DEC REGISTER[index]

    MUL REGISTER[index], REGISTER[index]
    DIV REGISTER[index], REGISTER[index]
    SUB REGISTER[index], REGISTER[index]
    ADD REGISTER[index], REGISTER[index]
    MOD REGISTER[index], REGISTER[index]

    INIT REGISTER[index]


    */
    switch(code[0]) {
        case 0x1:

        case 0x2:

        case 0x3:

        case 0x4:
            break;
    }
}

int main(int argc, char ** argv) {
    char *outText;
    int fd;
    int fileSize, inputSize;
    unsigned int codeLen;
    char buffer[4096];

    std::string s{};
    std::string s2{};

    std::string oneChar;

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

    for (int i=0; i<codeLen; i+=3) {
        oneChar = s.substr(i, 3);
        std::cout << oneChar << std::endl;

        mapping(i/3, oneChar);


        if (1) { // emulate condition
            // emulate(code); // tmp code (it can be 1~5 byte->length?)

            if (1) { // check stdout <-> expected out
                correctAnswer();
                break;
            }

        } else {
            continue;
        }
    }

    wrongAnswer();

    /*
    if (!s.substr(0,3).compare(std::string("냐"))) {
        puts("ok");
    } else {
        puts("no");
    }
    */

    // Destroy used object and release memory
    api->End();
    delete [] outText;
    pixDestroy(&image);

    unlink(filename);

    cout << "Finish" << endl;

    return 0;
}


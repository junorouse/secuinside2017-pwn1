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


    mystdout -> 0x00000000006036A0
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

// define instruction

#define INS_GET 1
#define INS_PRT 2
#define INS_PRT_CHAR 50

#define INS_CMP 3
#define INS_JMP 30
#define INS_SAME_JMP 31
#define INS_LESS_JMP 32
#define INS_BIGGER_JMP 5

#define INS_MOV 6
#define INS_MOV_PTR 7 // under construction
#define INS_PTR_MOV 70 // under construction, heap leak

#define INS_INC 8
#define INS_DEC 9

#define INS_MUL 10
#define INS_DIV 11
#define INS_SUB 12
#define INS_ADD 13
#define INS_MOD 14
#define INS_INIT 15

#define INS_NOP 16

#define INS_SEPERATOR 20


using namespace std; // check

// global variables

char fuck[256] = "mynameisjunoim";

std::string ANSWERSTDOUT;
std::string MYSTDOUT;
int MYSTDIN[128] = {0,};

unsigned int REGISTERS[256];

unsigned int SAME_FLAG, LESS_FLAG, BIGGER_FLAG;

unsigned int CODE_RIP = 0;
unsigned int MYSTDIN_IDX = 0;

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
    // fucking have to make map functions

    if (!str.compare(string("두")))
        return INS_GET;

    if (!str.compare(string("유")))
        return 2;

    if (!str.compare(string("노")))
        return INS_SEPERATOR;

    if (!str.compare(string("우")))
        return INS_PRT;

    if (!str.compare(string("강")))
        return 2;

    if (!str.compare(string("남")))
        return INS_SEPERATOR;

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
    char buf[256];
    for (int i=0; i<5; i++) {
        MYSTDIN[i] = rand() % 30;
        memset(buf, 0, 256);
        sprintf(buf,"%d\n", MYSTDIN[i] * 10);
        ANSWERSTDOUT += std::string(buf);
    }

    // cout << ANSWERSTDOUT << endl;

}

void emulate(unsigned char code[]) {
    // I want to obfuscation

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

    char buf[256];

    if (CODE_RIP < 0) {
        exit(-1);
    }

    switch(code[0]) {
        case INS_GET:
            REGISTERS[code[1]] = MYSTDIN[MYSTDIN_IDX++];
            break;

        case INS_PRT:
            // MYSTDOUT += ~
            // user can choose format string
            memset(buf, 0, 256);
            sprintf(buf,"%d\n", REGISTERS[code[1]]);
            MYSTDOUT += std::string(buf);
            printf("%d\n", REGISTERS[code[1]]);
            break;

        case INS_PRT_CHAR:
            // MYSTDOUT += ~
            // user can choose format string
            memset(buf, 0, 256);
            sprintf(buf,"%c\n", REGISTERS[code[1]]);
            MYSTDOUT += std::string(buf);
            printf("%c\n", REGISTERS[code[1]]);
            break;


        case INS_CMP:
            if (REGISTERS[code[1]] > REGISTERS[code[2]]) {
                BIGGER_FLAG = 1;
                LESS_FLAG = 0;
                SAME_FLAG = 0;
            } else if (REGISTERS[code[1]] < REGISTERS[code[2]]) {
                LESS_FLAG = 1;
                BIGGER_FLAG = 0;
                SAME_FLAG = 0;
            } else if (REGISTERS[code[1]] == REGISTERS[code[2]]) {
                SAME_FLAG = 1;
                LESS_FLAG = 0;
                BIGGER_FLAG = 0;
            }
            break;

        case INS_JMP:
            CODE_RIP = CODE_RIP + code[1];
            break;

        case INS_LESS_JMP:
            if (LESS_FLAG) {
                CODE_RIP = CODE_RIP + code[1];
            }
            break;

        case INS_BIGGER_JMP:
            if (BIGGER_FLAG) {
                CODE_RIP = CODE_RIP + code[1];
            }
            break;

        case INS_SAME_JMP:
            if (SAME_FLAG) {
                CODE_RIP = CODE_RIP + code[1];
            }
            break;

        case INS_MOV:
            REGISTERS[code[1]] = REGISTERS[code[2]];
            break;

        case INS_MOV_PTR:
            break;

        case INS_INC:
            REGISTERS[code[1]]++;
            break;
        case INS_DEC:
            REGISTERS[code[1]]--;
            break;


        case INS_MUL:
            REGISTERS[code[1]] *= REGISTERS[code[2]];
            break;
        case INS_ADD:
            REGISTERS[code[1]] += REGISTERS[code[2]];
            break;
        case INS_DIV:
            REGISTERS[code[1]] /= REGISTERS[code[2]];
            break;
        case INS_SUB:
            REGISTERS[code[1]] -= REGISTERS[code[2]];
            break;
        case INS_MOD:
            REGISTERS[code[1]] %= REGISTERS[code[2]];
            break;

        case INS_INIT:
            REGISTERS[code[1]] = 0;
            break;

        case INS_NOP:
            break;
    }
}

int main(int argc, char ** argv) {
    char *outText;
    int fd;
    int fileSize, inputSize;
    unsigned int codeLen;
    char buffer[8192];

    std::string s{};
    std::string s2{};

    std::string oneChar;
    int liveCodeIdx = 0;

    unsigned char mappedCode[8192];
    unsigned char liveCode[20];

    srand(time(0));

    scanf("%d", &fileSize);


    char filename[] = "/tmp/prob/image.XXXXXX";
    fd = mkstemp(filename);

    while (fileSize) {
        inputSize = read(0, buffer, 4096);
        write(fd, buffer, inputSize);
        fileSize -= inputSize;
        if (fileSize < 0) {
            fileSize = 0; }
    }

    close(fd);

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();

    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "kor")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(-1);
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


    // code mapping
    for (int i=0; i<codeLen*3; i+=3) {
        oneChar = s.substr(i, 3);
        // std::cout << oneChar << std::endl; // test code

        mappedCode[i/3] = mapping(i/3, oneChar);

        // printf("%d: %d\n", i, mappedCode[i/3]);
    }

    // make test case

    makeTestCase(); // done

    // code emulate


    CODE_RIP = 0;

    puts("== output ==");

    
    unsigned char zzzz[] = {
        INS_INIT, 1, INS_SEPERATOR,

    INS_INC, 1, INS_SEPERATOR,
    INS_MOV, 2, 1, INS_SEPERATOR,


    INS_INC, 2, INS_SEPERATOR,
    INS_MOV, 3, 2, INS_SEPERATOR,


    INS_INC, 3, INS_SEPERATOR,
    INS_MOV, 4, 3, INS_SEPERATOR,


    INS_INC, 4, INS_SEPERATOR,
    INS_MOV, 5, 4, INS_SEPERATOR,


    INS_INC, 5, INS_SEPERATOR,
    INS_MOV, 6, 5, INS_SEPERATOR,


    INS_INC, 6, INS_SEPERATOR,
    INS_MOV, 7, 6, INS_SEPERATOR,


    INS_INC, 7, INS_SEPERATOR,
    INS_MOV, 8, 7, INS_SEPERATOR,


    INS_INC, 8, INS_SEPERATOR,
    INS_MOV, 9, 8, INS_SEPERATOR,


    INS_INC, 9, INS_SEPERATOR,
    INS_MOV, 10, 9, INS_SEPERATOR,


    INS_INC, 10, INS_SEPERATOR,
    INS_MOV, 11, 10, INS_SEPERATOR,


    INS_INC, 11, INS_SEPERATOR,
    INS_MOV, 12, 11, INS_SEPERATOR,


    INS_INC, 12, INS_SEPERATOR,
    INS_MOV, 13, 12, INS_SEPERATOR,


    INS_INC, 13, INS_SEPERATOR,
    INS_MOV, 14, 13, INS_SEPERATOR,


    INS_INC, 14, INS_SEPERATOR,
    INS_MOV, 15, 14, INS_SEPERATOR,


    INS_INC, 15, INS_SEPERATOR,
    INS_MOV, 16, 15, INS_SEPERATOR,


    INS_INC, 16, INS_SEPERATOR,
    INS_MOV, 17, 16, INS_SEPERATOR,


    INS_INC, 17, INS_SEPERATOR,
    INS_MOV, 18, 17, INS_SEPERATOR,


    INS_INC, 18, INS_SEPERATOR,
    INS_MOV, 19, 18, INS_SEPERATOR,


    INS_INC, 19, INS_SEPERATOR,
    INS_MOV, 20, 19, INS_SEPERATOR,


    INS_INC, 20, INS_SEPERATOR,
    INS_MOV, 21, 20, INS_SEPERATOR,


    INS_INC, 21, INS_SEPERATOR,
    INS_MOV, 22, 21, INS_SEPERATOR,


    INS_INC, 22, INS_SEPERATOR,
    INS_MOV, 23, 22, INS_SEPERATOR,


    INS_INC, 23, INS_SEPERATOR,
    INS_MOV, 24, 23, INS_SEPERATOR,


    INS_INC, 24, INS_SEPERATOR,
    INS_MOV, 25, 24, INS_SEPERATOR,


    INS_INC, 25, INS_SEPERATOR,
    INS_MOV, 26, 25, INS_SEPERATOR,


    INS_INC, 26, INS_SEPERATOR,
    INS_MOV, 27, 26, INS_SEPERATOR,


    INS_INC, 27, INS_SEPERATOR,
    INS_MOV, 28, 27, INS_SEPERATOR,


    INS_INC, 28, INS_SEPERATOR,
    INS_MOV, 29, 28, INS_SEPERATOR,


    INS_INC, 29, INS_SEPERATOR,
    INS_MOV, 30, 29, INS_SEPERATOR,


    INS_INC, 30, INS_SEPERATOR,
    INS_MOV, 31, 30, INS_SEPERATOR,


    INS_INC, 31, INS_SEPERATOR,
    INS_MOV, 32, 31, INS_SEPERATOR,


    INS_INC, 32, INS_SEPERATOR,
    INS_MOV, 33, 32, INS_SEPERATOR,


    INS_INC, 33, INS_SEPERATOR,
    INS_MOV, 34, 33, INS_SEPERATOR,


    INS_INC, 34, INS_SEPERATOR,
    INS_MOV, 35, 34, INS_SEPERATOR,


    INS_INC, 35, INS_SEPERATOR,
    INS_MOV, 36, 35, INS_SEPERATOR,


    INS_INC, 36, INS_SEPERATOR,
    INS_MOV, 37, 36, INS_SEPERATOR,


    INS_INC, 37, INS_SEPERATOR,
    INS_MOV, 38, 37, INS_SEPERATOR,


    INS_INC, 38, INS_SEPERATOR,
    INS_MOV, 39, 38, INS_SEPERATOR,


    INS_INC, 39, INS_SEPERATOR,
    INS_MOV, 40, 39, INS_SEPERATOR,


    INS_INC, 40, INS_SEPERATOR,
    INS_MOV, 41, 40, INS_SEPERATOR,


    INS_INC, 41, INS_SEPERATOR,
    INS_MOV, 42, 41, INS_SEPERATOR,


    INS_INC, 42, INS_SEPERATOR,
    INS_MOV, 43, 42, INS_SEPERATOR,


    INS_INC, 43, INS_SEPERATOR,
    INS_MOV, 44, 43, INS_SEPERATOR,


    INS_INC, 44, INS_SEPERATOR,
    INS_MOV, 45, 44, INS_SEPERATOR,


    INS_INC, 45, INS_SEPERATOR,
    INS_MOV, 46, 45, INS_SEPERATOR,


    INS_INC, 46, INS_SEPERATOR,
    INS_MOV, 47, 46, INS_SEPERATOR,


    INS_INC, 47, INS_SEPERATOR,
    INS_MOV, 48, 47, INS_SEPERATOR,


    INS_INC, 48, INS_SEPERATOR,
    INS_MOV, 49, 48, INS_SEPERATOR,


    INS_INC, 49, INS_SEPERATOR,
    INS_MOV, 50, 49, INS_SEPERATOR,


    INS_INC, 50, INS_SEPERATOR,
    INS_MOV, 51, 50, INS_SEPERATOR,


    INS_INC, 51, INS_SEPERATOR,
    INS_MOV, 52, 51, INS_SEPERATOR,


    INS_INC, 52, INS_SEPERATOR,
    INS_MOV, 53, 52, INS_SEPERATOR,


    INS_INC, 53, INS_SEPERATOR,
    INS_MOV, 54, 53, INS_SEPERATOR,


    INS_INC, 54, INS_SEPERATOR,
    INS_MOV, 55, 54, INS_SEPERATOR,


    INS_INC, 55, INS_SEPERATOR,
    INS_MOV, 56, 55, INS_SEPERATOR,


    INS_INC, 56, INS_SEPERATOR,
    INS_MOV, 57, 56, INS_SEPERATOR,


    INS_INC, 57, INS_SEPERATOR,
    INS_MOV, 58, 57, INS_SEPERATOR,


    INS_INC, 58, INS_SEPERATOR,
    INS_MOV, 59, 58, INS_SEPERATOR,


    INS_INC, 59, INS_SEPERATOR,
    INS_MOV, 60, 59, INS_SEPERATOR,


    INS_INC, 60, INS_SEPERATOR,
    INS_MOV, 61, 60, INS_SEPERATOR,


    INS_INC, 61, INS_SEPERATOR,
    INS_MOV, 62, 61, INS_SEPERATOR,


    INS_INC, 62, INS_SEPERATOR,
    INS_MOV, 63, 62, INS_SEPERATOR,


    INS_INC, 63, INS_SEPERATOR,
    INS_MOV, 64, 63, INS_SEPERATOR,


    INS_INC, 64, INS_SEPERATOR,
    INS_MOV, 65, 64, INS_SEPERATOR,


    INS_INC, 65, INS_SEPERATOR,
    INS_MOV, 66, 65, INS_SEPERATOR,


    INS_INC, 66, INS_SEPERATOR,
    INS_MOV, 67, 66, INS_SEPERATOR,


    INS_INC, 67, INS_SEPERATOR,
    INS_MOV, 68, 67, INS_SEPERATOR,


    INS_INC, 68, INS_SEPERATOR,
    INS_MOV, 69, 68, INS_SEPERATOR,


    INS_INC, 69, INS_SEPERATOR,
    INS_MOV, 70, 69, INS_SEPERATOR,


    INS_INC, 70, INS_SEPERATOR,
    INS_MOV, 71, 70, INS_SEPERATOR,


    INS_INC, 71, INS_SEPERATOR,
    INS_MOV, 72, 71, INS_SEPERATOR,


    INS_INC, 72, INS_SEPERATOR,
    INS_MOV, 73, 72, INS_SEPERATOR,


    INS_INC, 73, INS_SEPERATOR,
    INS_MOV, 74, 73, INS_SEPERATOR,


    INS_INC, 74, INS_SEPERATOR,
    INS_MOV, 75, 74, INS_SEPERATOR,


    INS_INC, 75, INS_SEPERATOR,
    INS_MOV, 76, 75, INS_SEPERATOR,


    INS_INC, 76, INS_SEPERATOR,
    INS_MOV, 77, 76, INS_SEPERATOR,


    INS_INC, 77, INS_SEPERATOR,
    INS_MOV, 78, 77, INS_SEPERATOR,


    INS_INC, 78, INS_SEPERATOR,
    INS_MOV, 79, 78, INS_SEPERATOR,


    INS_INC, 79, INS_SEPERATOR,
    INS_MOV, 80, 79, INS_SEPERATOR,


    INS_INC, 80, INS_SEPERATOR,
    INS_MOV, 81, 80, INS_SEPERATOR,


    INS_INC, 81, INS_SEPERATOR,
    INS_MOV, 82, 81, INS_SEPERATOR,


    INS_INC, 82, INS_SEPERATOR,
    INS_MOV, 83, 82, INS_SEPERATOR,


    INS_INC, 83, INS_SEPERATOR,
    INS_MOV, 84, 83, INS_SEPERATOR,


    INS_INC, 84, INS_SEPERATOR,
    INS_MOV, 85, 84, INS_SEPERATOR,


    INS_INC, 85, INS_SEPERATOR,
    INS_MOV, 86, 85, INS_SEPERATOR,


    INS_INC, 86, INS_SEPERATOR,
    INS_MOV, 87, 86, INS_SEPERATOR,


    INS_INC, 87, INS_SEPERATOR,
    INS_MOV, 88, 87, INS_SEPERATOR,


    INS_INC, 88, INS_SEPERATOR,
    INS_MOV, 89, 88, INS_SEPERATOR,


    INS_INC, 89, INS_SEPERATOR,
    INS_MOV, 90, 89, INS_SEPERATOR,


    INS_INC, 90, INS_SEPERATOR,
    INS_MOV, 91, 90, INS_SEPERATOR,


    INS_INC, 91, INS_SEPERATOR,
    INS_MOV, 92, 91, INS_SEPERATOR,


    INS_INC, 92, INS_SEPERATOR,
    INS_MOV, 93, 92, INS_SEPERATOR,


    INS_INC, 93, INS_SEPERATOR,
    INS_MOV, 94, 93, INS_SEPERATOR,


    INS_INC, 94, INS_SEPERATOR,
    INS_MOV, 95, 94, INS_SEPERATOR,


    INS_INC, 95, INS_SEPERATOR,
    INS_MOV, 96, 95, INS_SEPERATOR,


    INS_INC, 96, INS_SEPERATOR,
    INS_MOV, 97, 96, INS_SEPERATOR,


    INS_INC, 97, INS_SEPERATOR,
    INS_MOV, 98, 97, INS_SEPERATOR,


    INS_INC, 98, INS_SEPERATOR,
    INS_MOV, 99, 98, INS_SEPERATOR,


    INS_INC, 99, INS_SEPERATOR,
    INS_MOV, 100, 99, INS_SEPERATOR,


    INS_INC, 100, INS_SEPERATOR,
    INS_MOV, 101, 100, INS_SEPERATOR,


    INS_INC, 101, INS_SEPERATOR,
    INS_MOV, 102, 101, INS_SEPERATOR,


    INS_INC, 102, INS_SEPERATOR,
    INS_MOV, 103, 102, INS_SEPERATOR,


    INS_INC, 103, INS_SEPERATOR,
    INS_MOV, 104, 103, INS_SEPERATOR,


    INS_INC, 104, INS_SEPERATOR,
    INS_MOV, 105, 104, INS_SEPERATOR,


    INS_INC, 105, INS_SEPERATOR,
    INS_MOV, 106, 105, INS_SEPERATOR,


    INS_INC, 106, INS_SEPERATOR,
    INS_MOV, 107, 106, INS_SEPERATOR,


    INS_INC, 107, INS_SEPERATOR,
    INS_MOV, 108, 107, INS_SEPERATOR,


    INS_INC, 108, INS_SEPERATOR,
    INS_MOV, 109, 108, INS_SEPERATOR,


    INS_INC, 109, INS_SEPERATOR,
    INS_MOV, 110, 109, INS_SEPERATOR,


    INS_INC, 110, INS_SEPERATOR,
    INS_MOV, 111, 110, INS_SEPERATOR,


    INS_INC, 111, INS_SEPERATOR,
    INS_MOV, 112, 111, INS_SEPERATOR,


    INS_INC, 112, INS_SEPERATOR,
    INS_MOV, 113, 112, INS_SEPERATOR,


    INS_INC, 113, INS_SEPERATOR,
    INS_MOV, 114, 113, INS_SEPERATOR,


    INS_INC, 114, INS_SEPERATOR,
    INS_MOV, 115, 114, INS_SEPERATOR,


    INS_INC, 115, INS_SEPERATOR,
    INS_MOV, 116, 115, INS_SEPERATOR,


    INS_INC, 116, INS_SEPERATOR,
    INS_MOV, 117, 116, INS_SEPERATOR,


    INS_INC, 117, INS_SEPERATOR,
    INS_MOV, 118, 117, INS_SEPERATOR,


    INS_INC, 118, INS_SEPERATOR,
    INS_MOV, 119, 118, INS_SEPERATOR,


    INS_INC, 119, INS_SEPERATOR,
    INS_MOV, 120, 119, INS_SEPERATOR,


    INS_INC, 120, INS_SEPERATOR,
    INS_MOV, 121, 120, INS_SEPERATOR,


    INS_INC, 121, INS_SEPERATOR,
    INS_MOV, 122, 121, INS_SEPERATOR,


    INS_INC, 122, INS_SEPERATOR,
    INS_MOV, 123, 122, INS_SEPERATOR,


    INS_INC, 123, INS_SEPERATOR,
    INS_MOV, 124, 123, INS_SEPERATOR,


    INS_INC, 124, INS_SEPERATOR,
    INS_MOV, 125, 124, INS_SEPERATOR,


    INS_INC, 125, INS_SEPERATOR,
    INS_MOV, 126, 125, INS_SEPERATOR,


    INS_INC, 126, INS_SEPERATOR,
    INS_MOV, 127, 126, INS_SEPERATOR,

    INS_PRT_CHAR, 74, INS_SEPERATOR,
    INS_PRT_CHAR, 103, INS_SEPERATOR,
    INS_PRT_CHAR, 110, INS_SEPERATOR,
    INS_PRT_CHAR, 110, INS_SEPERATOR,
    INS_PRT_CHAR, 113, INS_SEPERATOR,
    INS_PRT_CHAR, 34, INS_SEPERATOR,
    INS_PRT_CHAR, 89, INS_SEPERATOR,
    INS_PRT_CHAR, 113, INS_SEPERATOR,
    INS_PRT_CHAR, 116, INS_SEPERATOR,
    INS_PRT_CHAR, 110, INS_SEPERATOR,
    INS_PRT_CHAR, 102, INS_SEPERATOR,
    INS_PRT_CHAR, 34, INS_SEPERATOR,
    INS_PRT_CHAR, 35, INS_SEPERATOR,
    };
    

    for (int k=0; k<sizeof(zzzz); k++)
        mappedCode[k] = zzzz[k];

    printf("size: %d\n", sizeof(zzzz));

    while (1) {
        if (CODE_RIP > 8192) {
            break;
        }

        if (mappedCode[CODE_RIP] == INS_SEPERATOR) {
            if (liveCodeIdx > 20) {
                //wtf
                break;
            }
            liveCodeIdx = 0;
            emulate(liveCode);
        } else {
            liveCode[liveCodeIdx++] = mappedCode[CODE_RIP];
        }
        CODE_RIP++;
    }

    // check answers
    if (!MYSTDOUT.compare(ANSWERSTDOUT)) {
        correctAnswer();
    } else {
        wrongAnswer();
    }

    // Destroy used object and release memory
    api->End();
    delete [] outText; // free_hook_ptr -> oneshot
    pixDestroy(&image);

    unlink(filename);

    cout << "Finish" << endl;

    return 0;
}



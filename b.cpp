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

#define INS_SEPERATOR 20


using namespace std; // check

// global variables

char fuck[256] = "mynameisjunoim";

std::string ANSWERSTDOUT;
std::string MYSTDOUT;
int MYSTDIN[128] = {0,};

unsigned int REGISTERS[32];

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

    cout << ANSWERSTDOUT << endl;

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
        std::cout << oneChar << std::endl; // test code

        mappedCode[i/3] = mapping(i/3, oneChar);
    }

    // make test case

    makeTestCase(); // done

    // code emulate


    CODE_RIP = 0;

    puts("GOGO");

    /*
    unsigned char zzzz[] = {
        INS_INC, 2, INS_SEPERATOR,
        INS_INC, 2, INS_SEPERATOR,
        INS_INC, 2, INS_SEPERATOR,
        INS_INC, 2, INS_SEPERATOR,
        INS_INC, 2, INS_SEPERATOR,
        INS_INC, 2, INS_SEPERATOR,
        INS_INC, 2, INS_SEPERATOR,
        INS_INC, 2, INS_SEPERATOR,
        INS_INC, 2, INS_SEPERATOR,
        INS_INC, 2, INS_SEPERATOR,
        // INS_PRT, 2, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_MUL, 3, 2, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_MUL, 3, 2, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_MUL, 3, 2, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_MUL, 3, 2, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_MUL, 3, 2, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
    };
    */
    unsigned char zzzz[] = {
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
        INS_GET, 3, INS_SEPERATOR,
        INS_PRT, 3, INS_SEPERATOR,
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


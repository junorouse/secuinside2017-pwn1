#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <string>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>

#include "mapper.h"

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
#define INS_PRT_CHAR 3

#define INS_CMP 4
#define INS_JMP 5
#define INS_SAME_JMP 6
#define INS_LESS_JMP 7
#define INS_BIGGER_JMP 8

#define INS_MOV 9
#define INS_MOV_PTR 10
#define INS_PTR_MOV 11

#define INS_INC 12
#define INS_DEC 13

#define INS_MUL 14
#define INS_DIV 15
#define INS_SUB 16
#define INS_ADD 17
#define INS_MOD 18
#define INS_INIT 19

#define INS_NOP 20

#define INS_RET 21

#define INS_SEPERATOR 31


using namespace std; // check

// global variables

// function ptrs (fucking mapping functions !)

std::string ANSWERSTDOUT;
std::string MYSTDOUT;
long MYSTDIN[128] = {0,};
char (*mapper[1024]) (int idx, int code); // get PIE base and calculate and bomb!

unsigned long REGISTERS[256];

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

void initMapper() {
    mapper[0] = mapper0;
    mapper[1] = mapper1;
    mapper[2] = mapper2;
    mapper[3] = mapper3;
    mapper[4] = mapper4;
    mapper[5] = mapper5;
    mapper[6] = mapper6;
    mapper[7] = mapper7;
    mapper[8] = mapper8;
    mapper[9] = mapper9;
    mapper[10] = mapper10;
    mapper[11] = mapper11;
    mapper[12] = mapper12;
    mapper[13] = mapper13;
    mapper[14] = mapper14;
    mapper[15] = mapper15;
    mapper[16] = mapper16;
    mapper[17] = mapper17;
    mapper[18] = mapper18;
    mapper[19] = mapper19;
    mapper[20] = mapper20;
    mapper[21] = mapper21;
    mapper[22] = mapper22;
    mapper[23] = mapper23;
    mapper[24] = mapper24;
    mapper[25] = mapper25;
    mapper[26] = mapper26;
    mapper[27] = mapper27;
    mapper[28] = mapper28;
    mapper[29] = mapper29;
    mapper[30] = mapper30;
    mapper[31] = mapper31;
    mapper[32] = mapper32;
    mapper[33] = mapper33;
    mapper[34] = mapper34;
    mapper[35] = mapper35;
    mapper[36] = mapper36;
    mapper[37] = mapper37;
    mapper[38] = mapper38;
    mapper[39] = mapper39;
    mapper[40] = mapper40;
    mapper[41] = mapper41;
    mapper[42] = mapper42;
    mapper[43] = mapper43;
    mapper[44] = mapper44;
    mapper[45] = mapper45;
    mapper[46] = mapper46;
    mapper[47] = mapper47;
    mapper[48] = mapper48;
    mapper[49] = mapper49;
    mapper[50] = mapper50;
    mapper[51] = mapper51;
    mapper[52] = mapper52;
    mapper[53] = mapper53;
    mapper[54] = mapper54;
    mapper[55] = mapper55;
    mapper[56] = mapper56;
    mapper[57] = mapper57;
    mapper[58] = mapper58;
    mapper[59] = mapper59;
    mapper[60] = mapper60;
    mapper[61] = mapper61;
    mapper[62] = mapper62;
    mapper[63] = mapper63;
    mapper[64] = mapper64;
    mapper[65] = mapper65;
    mapper[66] = mapper66;
    mapper[67] = mapper67;
    mapper[68] = mapper68;
    mapper[69] = mapper69;
    mapper[70] = mapper70;
    mapper[71] = mapper71;
    mapper[72] = mapper72;
    mapper[73] = mapper73;
    mapper[74] = mapper74;
    mapper[75] = mapper75;
    mapper[76] = mapper76;
    mapper[77] = mapper77;
    mapper[78] = mapper78;
    mapper[79] = mapper79;
    mapper[80] = mapper80;
    mapper[81] = mapper81;
    mapper[82] = mapper82;
    mapper[83] = mapper83;
    mapper[84] = mapper84;
    mapper[85] = mapper85;
    mapper[86] = mapper86;
    mapper[87] = mapper87;
    mapper[88] = mapper88;
    mapper[89] = mapper89;
    mapper[90] = mapper90;
    mapper[91] = mapper91;
    mapper[92] = mapper92;
    mapper[93] = mapper93;
    mapper[94] = mapper94;
    mapper[95] = mapper95;
    mapper[96] = mapper96;
    mapper[97] = mapper97;
    mapper[98] = mapper98;
    mapper[99] = mapper99;
    mapper[100] = mapper100;
    mapper[101] = mapper101;
    mapper[102] = mapper102;
    mapper[103] = mapper103;
    mapper[104] = mapper104;
    mapper[105] = mapper105;
    mapper[106] = mapper106;
    mapper[107] = mapper107;
    mapper[108] = mapper108;
    mapper[109] = mapper109;
    mapper[110] = mapper110;
    mapper[111] = mapper111;
    mapper[112] = mapper112;
    mapper[113] = mapper113;
    mapper[114] = mapper114;
    mapper[115] = mapper115;
    mapper[116] = mapper116;
    mapper[117] = mapper117;
    mapper[118] = mapper118;
    mapper[119] = mapper119;
    mapper[120] = mapper120;
    mapper[121] = mapper121;
    mapper[122] = mapper122;
    mapper[123] = mapper123;
    mapper[124] = mapper124;
    mapper[125] = mapper125;
    mapper[126] = mapper126;
    mapper[127] = mapper127;
    mapper[128] = mapper128;
    mapper[129] = mapper129;
    mapper[130] = mapper130;
    mapper[131] = mapper131;
    mapper[132] = mapper132;
    mapper[133] = mapper133;
    mapper[134] = mapper134;
    mapper[135] = mapper135;
    mapper[136] = mapper136;
    mapper[137] = mapper137;
    mapper[138] = mapper138;
    mapper[139] = mapper139;
    mapper[140] = mapper140;
    mapper[141] = mapper141;
    mapper[142] = mapper142;
    mapper[143] = mapper143;
    mapper[144] = mapper144;
    mapper[145] = mapper145;
    mapper[146] = mapper146;
    mapper[147] = mapper147;
    mapper[148] = mapper148;
    mapper[149] = mapper149;
    mapper[150] = mapper150;
    mapper[151] = mapper151;
    mapper[152] = mapper152;
    mapper[153] = mapper153;
    mapper[154] = mapper154;
    mapper[155] = mapper155;
    mapper[156] = mapper156;
    mapper[157] = mapper157;
    mapper[158] = mapper158;
    mapper[159] = mapper159;
    mapper[160] = mapper160;
    mapper[161] = mapper161;
    mapper[162] = mapper162;
    mapper[163] = mapper163;
    mapper[164] = mapper164;
    mapper[165] = mapper165;
    mapper[166] = mapper166;
    mapper[167] = mapper167;
    mapper[168] = mapper168;
    mapper[169] = mapper169;
    mapper[170] = mapper170;
    mapper[171] = mapper171;
    mapper[172] = mapper172;
    mapper[173] = mapper173;
    mapper[174] = mapper174;
    mapper[175] = mapper175;
    mapper[176] = mapper176;
    mapper[177] = mapper177;
    mapper[178] = mapper178;
    mapper[179] = mapper179;
    mapper[180] = mapper180;
    mapper[181] = mapper181;
    mapper[182] = mapper182;
    mapper[183] = mapper183;
    mapper[184] = mapper184;
    mapper[185] = mapper185;
    mapper[186] = mapper186;
    mapper[187] = mapper187;
    mapper[188] = mapper188;
    mapper[189] = mapper189;
    mapper[190] = mapper190;
    mapper[191] = mapper191;
    mapper[192] = mapper192;
    mapper[193] = mapper193;
    mapper[194] = mapper194;
    mapper[195] = mapper195;
    mapper[196] = mapper196;
    mapper[197] = mapper197;
    mapper[198] = mapper198;
    mapper[199] = mapper199;
    mapper[200] = mapper200;
    mapper[201] = mapper201;
    mapper[202] = mapper202;
    mapper[203] = mapper203;
    mapper[204] = mapper204;
    mapper[205] = mapper205;
    mapper[206] = mapper206;
    mapper[207] = mapper207;
    mapper[208] = mapper208;
    mapper[209] = mapper209;
    mapper[210] = mapper210;
    mapper[211] = mapper211;
    mapper[212] = mapper212;
    mapper[213] = mapper213;
    mapper[214] = mapper214;
    mapper[215] = mapper215;
    mapper[216] = mapper216;
    mapper[217] = mapper217;
    mapper[218] = mapper218;
    mapper[219] = mapper219;
    mapper[220] = mapper220;
    mapper[221] = mapper221;
    mapper[222] = mapper222;
    mapper[223] = mapper223;
    mapper[224] = mapper224;
    mapper[225] = mapper225;
    mapper[226] = mapper226;
    mapper[227] = mapper227;
    mapper[228] = mapper228;
    mapper[229] = mapper229;
    mapper[230] = mapper230;
    mapper[231] = mapper231;
    mapper[232] = mapper232;
    mapper[233] = mapper233;
    mapper[234] = mapper234;
    mapper[235] = mapper235;
    mapper[236] = mapper236;
    mapper[237] = mapper237;
    mapper[238] = mapper238;
    mapper[239] = mapper239;
    mapper[240] = mapper240;
    mapper[241] = mapper241;
    mapper[242] = mapper242;
    mapper[243] = mapper243;
    mapper[244] = mapper244;
    mapper[245] = mapper245;
    mapper[246] = mapper246;
    mapper[247] = mapper247;
    mapper[248] = mapper248;
    mapper[249] = mapper249;
    mapper[250] = mapper250;
    mapper[251] = mapper251;
    mapper[252] = mapper252;
    mapper[253] = mapper253;
    mapper[254] = mapper254;
    mapper[255] = mapper255;
    mapper[256] = mapper256;
    mapper[257] = mapper257;
    mapper[258] = mapper258;
    mapper[259] = mapper259;
    mapper[260] = mapper260;
    mapper[261] = mapper261;
    mapper[262] = mapper262;
    mapper[263] = mapper263;
    mapper[264] = mapper264;
    mapper[265] = mapper265;
    mapper[266] = mapper266;
    mapper[267] = mapper267;
    mapper[268] = mapper268;
    mapper[269] = mapper269;
    mapper[270] = mapper270;
    mapper[271] = mapper271;
    mapper[272] = mapper272;
    mapper[273] = mapper273;
    mapper[274] = mapper274;
    mapper[275] = mapper275;
    mapper[276] = mapper276;
    mapper[277] = mapper277;
    mapper[278] = mapper278;
    mapper[279] = mapper279;
    mapper[280] = mapper280;
    mapper[281] = mapper281;
    mapper[282] = mapper282;
    mapper[283] = mapper283;
    mapper[284] = mapper284;
    mapper[285] = mapper285;
    mapper[286] = mapper286;
    mapper[287] = mapper287;
    mapper[288] = mapper288;
    mapper[289] = mapper289;
    mapper[290] = mapper290;
    mapper[291] = mapper291;
    mapper[292] = mapper292;
    mapper[293] = mapper293;
    mapper[294] = mapper294;
    mapper[295] = mapper295;
    mapper[296] = mapper296;
    mapper[297] = mapper297;
    mapper[298] = mapper298;
    mapper[299] = mapper299;
    mapper[300] = mapper300;
    mapper[301] = mapper301;
    mapper[302] = mapper302;
    mapper[303] = mapper303;
    mapper[304] = mapper304;
    mapper[305] = mapper305;
    mapper[306] = mapper306;
    mapper[307] = mapper307;
    mapper[308] = mapper308;
    mapper[309] = mapper309;
    mapper[310] = mapper310;
    mapper[311] = mapper311;
    mapper[312] = mapper312;
    mapper[313] = mapper313;
    mapper[314] = mapper314;
    mapper[315] = mapper315;
    mapper[316] = mapper316;
    mapper[317] = mapper317;
    mapper[318] = mapper318;
    mapper[319] = mapper319;
    mapper[320] = mapper320;
    mapper[321] = mapper321;
    mapper[322] = mapper322;
    mapper[323] = mapper323;
    mapper[324] = mapper324;
    mapper[325] = mapper325;
    mapper[326] = mapper326;
    mapper[327] = mapper327;
    mapper[328] = mapper328;
    mapper[329] = mapper329;
    mapper[330] = mapper330;
    mapper[331] = mapper331;
    mapper[332] = mapper332;
    mapper[333] = mapper333;
    mapper[334] = mapper334;
    mapper[335] = mapper335;
    mapper[336] = mapper336;
    mapper[337] = mapper337;
    mapper[338] = mapper338;
    mapper[339] = mapper339;
    mapper[340] = mapper340;
    mapper[341] = mapper341;
    mapper[342] = mapper342;
    mapper[343] = mapper343;
    mapper[344] = mapper344;
    mapper[345] = mapper345;
    mapper[346] = mapper346;
    mapper[347] = mapper347;
    mapper[348] = mapper348;
    mapper[349] = mapper349;
    mapper[350] = mapper350;
    mapper[351] = mapper351;
    mapper[352] = mapper352;
    mapper[353] = mapper353;
    mapper[354] = mapper354;
    mapper[355] = mapper355;
    mapper[356] = mapper356;
    mapper[357] = mapper357;
    mapper[358] = mapper358;
    mapper[359] = mapper359;
    mapper[360] = mapper360;
    mapper[361] = mapper361;
    mapper[362] = mapper362;
    mapper[363] = mapper363;
    mapper[364] = mapper364;
    mapper[365] = mapper365;
    mapper[366] = mapper366;
    mapper[367] = mapper367;
    mapper[368] = mapper368;
    mapper[369] = mapper369;
    mapper[370] = mapper370;
    mapper[371] = mapper371;
    mapper[372] = mapper372;
    mapper[373] = mapper373;
    mapper[374] = mapper374;
    mapper[375] = mapper375;
    mapper[376] = mapper376;
    mapper[377] = mapper377;
    mapper[378] = mapper378;
    mapper[379] = mapper379;
    mapper[380] = mapper380;
    mapper[381] = mapper381;
    mapper[382] = mapper382;
    mapper[383] = mapper383;
    mapper[384] = mapper384;
    mapper[385] = mapper385;
    mapper[386] = mapper386;
    mapper[387] = mapper387;
    mapper[388] = mapper388;
    mapper[389] = mapper389;
    mapper[390] = mapper390;
    mapper[391] = mapper391;
    mapper[392] = mapper392;
    mapper[393] = mapper393;
    mapper[394] = mapper394;
    mapper[395] = mapper395;
    mapper[396] = mapper396;
    mapper[397] = mapper397;
    mapper[398] = mapper398;
    mapper[399] = mapper399;
    mapper[400] = mapper400;
    mapper[401] = mapper401;
    mapper[402] = mapper402;
    mapper[403] = mapper403;
    mapper[404] = mapper404;
    mapper[405] = mapper405;
    mapper[406] = mapper406;
    mapper[407] = mapper407;
    mapper[408] = mapper408;
    mapper[409] = mapper409;
    mapper[410] = mapper410;
    mapper[411] = mapper411;
    mapper[412] = mapper412;
    mapper[413] = mapper413;
    mapper[414] = mapper414;
    mapper[415] = mapper415;
    mapper[416] = mapper416;
    mapper[417] = mapper417;
    mapper[418] = mapper418;
    mapper[419] = mapper419;
    mapper[420] = mapper420;
    mapper[421] = mapper421;
    mapper[422] = mapper422;
    mapper[423] = mapper423;
    mapper[424] = mapper424;
    mapper[425] = mapper425;
    mapper[426] = mapper426;
    mapper[427] = mapper427;
    mapper[428] = mapper428;
    mapper[429] = mapper429;
    mapper[430] = mapper430;
    mapper[431] = mapper431;
    mapper[432] = mapper432;
    mapper[433] = mapper433;
    mapper[434] = mapper434;
    mapper[435] = mapper435;
    mapper[436] = mapper436;
    mapper[437] = mapper437;
    mapper[438] = mapper438;
    mapper[439] = mapper439;
    mapper[440] = mapper440;
    mapper[441] = mapper441;
    mapper[442] = mapper442;
    mapper[443] = mapper443;
    mapper[444] = mapper444;
    mapper[445] = mapper445;
    mapper[446] = mapper446;
    mapper[447] = mapper447;
    mapper[448] = mapper448;
    mapper[449] = mapper449;
    mapper[450] = mapper450;
    mapper[451] = mapper451;
    mapper[452] = mapper452;
    mapper[453] = mapper453;
    mapper[454] = mapper454;
    mapper[455] = mapper455;
    mapper[456] = mapper456;
    mapper[457] = mapper457;
    mapper[458] = mapper458;
    mapper[459] = mapper459;
    mapper[460] = mapper460;
    mapper[461] = mapper461;
    mapper[462] = mapper462;
    mapper[463] = mapper463;
    mapper[464] = mapper464;
    mapper[465] = mapper465;
    mapper[466] = mapper466;
    mapper[467] = mapper467;
    mapper[468] = mapper468;
    mapper[469] = mapper469;
    mapper[470] = mapper470;
    mapper[471] = mapper471;
    mapper[472] = mapper472;
    mapper[473] = mapper473;
    mapper[474] = mapper474;
    mapper[475] = mapper475;
    mapper[476] = mapper476;
    mapper[477] = mapper477;
    mapper[478] = mapper478;
    mapper[479] = mapper479;
    mapper[480] = mapper480;
    mapper[481] = mapper481;
    mapper[482] = mapper482;
    mapper[483] = mapper483;
    mapper[484] = mapper484;
    mapper[485] = mapper485;
    mapper[486] = mapper486;
    mapper[487] = mapper487;
    mapper[488] = mapper488;
    mapper[489] = mapper489;
    mapper[490] = mapper490;
    mapper[491] = mapper491;
    mapper[492] = mapper492;
    mapper[493] = mapper493;
    mapper[494] = mapper494;
    mapper[495] = mapper495;
    mapper[496] = mapper496;
    mapper[497] = mapper497;
    mapper[498] = mapper498;
    mapper[499] = mapper499;
    mapper[500] = mapper500;
    mapper[501] = mapper501;
    mapper[502] = mapper502;
    mapper[503] = mapper503;
    mapper[504] = mapper504;
    mapper[505] = mapper505;
    mapper[506] = mapper506;
    mapper[507] = mapper507;
    mapper[508] = mapper508;
    mapper[509] = mapper509;
    mapper[510] = mapper510;
    mapper[511] = mapper511;
    mapper[512] = mapper512;
    mapper[513] = mapper513;
    mapper[514] = mapper514;
    mapper[515] = mapper515;
    mapper[516] = mapper516;
    mapper[517] = mapper517;
    mapper[518] = mapper518;
    mapper[519] = mapper519;
    mapper[520] = mapper520;
    mapper[521] = mapper521;
    mapper[522] = mapper522;
    mapper[523] = mapper523;
    mapper[524] = mapper524;
    mapper[525] = mapper525;
    mapper[526] = mapper526;
    mapper[527] = mapper527;
    mapper[528] = mapper528;
    mapper[529] = mapper529;
    mapper[530] = mapper530;
    mapper[531] = mapper531;
    mapper[532] = mapper532;
    mapper[533] = mapper533;
    mapper[534] = mapper534;
    mapper[535] = mapper535;
    mapper[536] = mapper536;
    mapper[537] = mapper537;
    mapper[538] = mapper538;
    mapper[539] = mapper539;
    mapper[540] = mapper540;
    mapper[541] = mapper541;
    mapper[542] = mapper542;
    mapper[543] = mapper543;
    mapper[544] = mapper544;
    mapper[545] = mapper545;
    mapper[546] = mapper546;
    mapper[547] = mapper547;
    mapper[548] = mapper548;
    mapper[549] = mapper549;
    mapper[550] = mapper550;
    mapper[551] = mapper551;
    mapper[552] = mapper552;
    mapper[553] = mapper553;
    mapper[554] = mapper554;
    mapper[555] = mapper555;
    mapper[556] = mapper556;
    mapper[557] = mapper557;
    mapper[558] = mapper558;
    mapper[559] = mapper559;
    mapper[560] = mapper560;
    mapper[561] = mapper561;
    mapper[562] = mapper562;
    mapper[563] = mapper563;
    mapper[564] = mapper564;
    mapper[565] = mapper565;
    mapper[566] = mapper566;
    mapper[567] = mapper567;
    mapper[568] = mapper568;
    mapper[569] = mapper569;
    mapper[570] = mapper570;
    mapper[571] = mapper571;
    mapper[572] = mapper572;
    mapper[573] = mapper573;
    mapper[574] = mapper574;
    mapper[575] = mapper575;
    mapper[576] = mapper576;
    mapper[577] = mapper577;
    mapper[578] = mapper578;
    mapper[579] = mapper579;
    mapper[580] = mapper580;
    mapper[581] = mapper581;
    mapper[582] = mapper582;
    mapper[583] = mapper583;
    mapper[584] = mapper584;
    mapper[585] = mapper585;
    mapper[586] = mapper586;
    mapper[587] = mapper587;
    mapper[588] = mapper588;
    mapper[589] = mapper589;
    mapper[590] = mapper590;
    mapper[591] = mapper591;
    mapper[592] = mapper592;
    mapper[593] = mapper593;
    mapper[594] = mapper594;
    mapper[595] = mapper595;
    mapper[596] = mapper596;
    mapper[597] = mapper597;
    mapper[598] = mapper598;
    mapper[599] = mapper599;
    mapper[600] = mapper600;
    mapper[601] = mapper601;
    mapper[602] = mapper602;
    mapper[603] = mapper603;
    mapper[604] = mapper604;
    mapper[605] = mapper605;
    mapper[606] = mapper606;
    mapper[607] = mapper607;
    mapper[608] = mapper608;
    mapper[609] = mapper609;
    mapper[610] = mapper610;
    mapper[611] = mapper611;
    mapper[612] = mapper612;
    mapper[613] = mapper613;
    mapper[614] = mapper614;
    mapper[615] = mapper615;
    mapper[616] = mapper616;
    mapper[617] = mapper617;
    mapper[618] = mapper618;
    mapper[619] = mapper619;
    mapper[620] = mapper620;
    mapper[621] = mapper621;
    mapper[622] = mapper622;
    mapper[623] = mapper623;
    mapper[624] = mapper624;
    mapper[625] = mapper625;
    mapper[626] = mapper626;
    mapper[627] = mapper627;
    mapper[628] = mapper628;
    mapper[629] = mapper629;
    mapper[630] = mapper630;
    mapper[631] = mapper631;
    mapper[632] = mapper632;
    mapper[633] = mapper633;
    mapper[634] = mapper634;
    mapper[635] = mapper635;
    mapper[636] = mapper636;
    mapper[637] = mapper637;
    mapper[638] = mapper638;
    mapper[639] = mapper639;
    mapper[640] = mapper640;
    mapper[641] = mapper641;
    mapper[642] = mapper642;
    mapper[643] = mapper643;
    mapper[644] = mapper644;
    mapper[645] = mapper645;
    mapper[646] = mapper646;
    mapper[647] = mapper647;
    mapper[648] = mapper648;
    mapper[649] = mapper649;
    mapper[650] = mapper650;
    mapper[651] = mapper651;
    mapper[652] = mapper652;
    mapper[653] = mapper653;
    mapper[654] = mapper654;
    mapper[655] = mapper655;
    mapper[656] = mapper656;
    mapper[657] = mapper657;
    mapper[658] = mapper658;
    mapper[659] = mapper659;
    mapper[660] = mapper660;
    mapper[661] = mapper661;
    mapper[662] = mapper662;
    mapper[663] = mapper663;
    mapper[664] = mapper664;
    mapper[665] = mapper665;
    mapper[666] = mapper666;
    mapper[667] = mapper667;
    mapper[668] = mapper668;
    mapper[669] = mapper669;
    mapper[670] = mapper670;
    mapper[671] = mapper671;
    mapper[672] = mapper672;
    mapper[673] = mapper673;
    mapper[674] = mapper674;
    mapper[675] = mapper675;
    mapper[676] = mapper676;
    mapper[677] = mapper677;
    mapper[678] = mapper678;
    mapper[679] = mapper679;
    mapper[680] = mapper680;
    mapper[681] = mapper681;
    mapper[682] = mapper682;
    mapper[683] = mapper683;
    mapper[684] = mapper684;
    mapper[685] = mapper685;
    mapper[686] = mapper686;
    mapper[687] = mapper687;
    mapper[688] = mapper688;
    mapper[689] = mapper689;
    mapper[690] = mapper690;
    mapper[691] = mapper691;
    mapper[692] = mapper692;
    mapper[693] = mapper693;
    mapper[694] = mapper694;
    mapper[695] = mapper695;
    mapper[696] = mapper696;
    mapper[697] = mapper697;
    mapper[698] = mapper698;
    mapper[699] = mapper699;
    mapper[700] = mapper700;
    mapper[701] = mapper701;
    mapper[702] = mapper702;
    mapper[703] = mapper703;
    mapper[704] = mapper704;
    mapper[705] = mapper705;
    mapper[706] = mapper706;
    mapper[707] = mapper707;
    mapper[708] = mapper708;
    mapper[709] = mapper709;
    mapper[710] = mapper710;
    mapper[711] = mapper711;
    mapper[712] = mapper712;
    mapper[713] = mapper713;
    mapper[714] = mapper714;
    mapper[715] = mapper715;
    mapper[716] = mapper716;
    mapper[717] = mapper717;
    mapper[718] = mapper718;
    mapper[719] = mapper719;
    mapper[720] = mapper720;
    mapper[721] = mapper721;
    mapper[722] = mapper722;
    mapper[723] = mapper723;
    mapper[724] = mapper724;
    mapper[725] = mapper725;
    mapper[726] = mapper726;
    mapper[727] = mapper727;
    mapper[728] = mapper728;
    mapper[729] = mapper729;
    mapper[730] = mapper730;
    mapper[731] = mapper731;
    mapper[732] = mapper732;
    mapper[733] = mapper733;
    mapper[734] = mapper734;
    mapper[735] = mapper735;
    mapper[736] = mapper736;
    mapper[737] = mapper737;
    mapper[738] = mapper738;
    mapper[739] = mapper739;
    mapper[740] = mapper740;
    mapper[741] = mapper741;
    mapper[742] = mapper742;
    mapper[743] = mapper743;
    mapper[744] = mapper744;
    mapper[745] = mapper745;
    mapper[746] = mapper746;
    mapper[747] = mapper747;
    mapper[748] = mapper748;
    mapper[749] = mapper749;
    mapper[750] = mapper750;
    mapper[751] = mapper751;
    mapper[752] = mapper752;
    mapper[753] = mapper753;
    mapper[754] = mapper754;
    mapper[755] = mapper755;
    mapper[756] = mapper756;
    mapper[757] = mapper757;
    mapper[758] = mapper758;
    mapper[759] = mapper759;
    mapper[760] = mapper760;
    mapper[761] = mapper761;
    mapper[762] = mapper762;
    mapper[763] = mapper763;
    mapper[764] = mapper764;
    mapper[765] = mapper765;
    mapper[766] = mapper766;
    mapper[767] = mapper767;
    mapper[768] = mapper768;
    mapper[769] = mapper769;
    mapper[770] = mapper770;
    mapper[771] = mapper771;
    mapper[772] = mapper772;
    mapper[773] = mapper773;
    mapper[774] = mapper774;
    mapper[775] = mapper775;
    mapper[776] = mapper776;
    mapper[777] = mapper777;
    mapper[778] = mapper778;
    mapper[779] = mapper779;
    mapper[780] = mapper780;
    mapper[781] = mapper781;
    mapper[782] = mapper782;
    mapper[783] = mapper783;
    mapper[784] = mapper784;
    mapper[785] = mapper785;
    mapper[786] = mapper786;
    mapper[787] = mapper787;
    mapper[788] = mapper788;
    mapper[789] = mapper789;
    mapper[790] = mapper790;
    mapper[791] = mapper791;
    mapper[792] = mapper792;
    mapper[793] = mapper793;
    mapper[794] = mapper794;
    mapper[795] = mapper795;
    mapper[796] = mapper796;
    mapper[797] = mapper797;
    mapper[798] = mapper798;
    mapper[799] = mapper799;
    mapper[800] = mapper800;
    mapper[801] = mapper801;
    mapper[802] = mapper802;
    mapper[803] = mapper803;
    mapper[804] = mapper804;
    mapper[805] = mapper805;
    mapper[806] = mapper806;
    mapper[807] = mapper807;
    mapper[808] = mapper808;
    mapper[809] = mapper809;
    mapper[810] = mapper810;
    mapper[811] = mapper811;
    mapper[812] = mapper812;
    mapper[813] = mapper813;
    mapper[814] = mapper814;
    mapper[815] = mapper815;
    mapper[816] = mapper816;
    mapper[817] = mapper817;
    mapper[818] = mapper818;
    mapper[819] = mapper819;
    mapper[820] = mapper820;
    mapper[821] = mapper821;
    mapper[822] = mapper822;
    mapper[823] = mapper823;
    mapper[824] = mapper824;
    mapper[825] = mapper825;
    mapper[826] = mapper826;
    mapper[827] = mapper827;
    mapper[828] = mapper828;
    mapper[829] = mapper829;
    mapper[830] = mapper830;
    mapper[831] = mapper831;
    mapper[832] = mapper832;
    mapper[833] = mapper833;
    mapper[834] = mapper834;
    mapper[835] = mapper835;
    mapper[836] = mapper836;
    mapper[837] = mapper837;
    mapper[838] = mapper838;
    mapper[839] = mapper839;
    mapper[840] = mapper840;
    mapper[841] = mapper841;
    mapper[842] = mapper842;
    mapper[843] = mapper843;
    mapper[844] = mapper844;
    mapper[845] = mapper845;
    mapper[846] = mapper846;
    mapper[847] = mapper847;
    mapper[848] = mapper848;
    mapper[849] = mapper849;
    mapper[850] = mapper850;
    mapper[851] = mapper851;
    mapper[852] = mapper852;
    mapper[853] = mapper853;
    mapper[854] = mapper854;
    mapper[855] = mapper855;
    mapper[856] = mapper856;
    mapper[857] = mapper857;
    mapper[858] = mapper858;
    mapper[859] = mapper859;
    mapper[860] = mapper860;
    mapper[861] = mapper861;
    mapper[862] = mapper862;
    mapper[863] = mapper863;
    mapper[864] = mapper864;
    mapper[865] = mapper865;
    mapper[866] = mapper866;
    mapper[867] = mapper867;
    mapper[868] = mapper868;
    mapper[869] = mapper869;
    mapper[870] = mapper870;
    mapper[871] = mapper871;
    mapper[872] = mapper872;
    mapper[873] = mapper873;
    mapper[874] = mapper874;
    mapper[875] = mapper875;
    mapper[876] = mapper876;
    mapper[877] = mapper877;
    mapper[878] = mapper878;
    mapper[879] = mapper879;
    mapper[880] = mapper880;
    mapper[881] = mapper881;
    mapper[882] = mapper882;
    mapper[883] = mapper883;
    mapper[884] = mapper884;
    mapper[885] = mapper885;
    mapper[886] = mapper886;
    mapper[887] = mapper887;
    mapper[888] = mapper888;
    mapper[889] = mapper889;
    mapper[890] = mapper890;
    mapper[891] = mapper891;
    mapper[892] = mapper892;
    mapper[893] = mapper893;
    mapper[894] = mapper894;
    mapper[895] = mapper895;
    mapper[896] = mapper896;
    mapper[897] = mapper897;
    mapper[898] = mapper898;
    mapper[899] = mapper899;
    mapper[900] = mapper900;
    mapper[901] = mapper901;
    mapper[902] = mapper902;
    mapper[903] = mapper903;
    mapper[904] = mapper904;
    mapper[905] = mapper905;
    mapper[906] = mapper906;
    mapper[907] = mapper907;
    mapper[908] = mapper908;
    mapper[909] = mapper909;
    mapper[910] = mapper910;
    mapper[911] = mapper911;
    mapper[912] = mapper912;
    mapper[913] = mapper913;
    mapper[914] = mapper914;
    mapper[915] = mapper915;
    mapper[916] = mapper916;
    mapper[917] = mapper917;
    mapper[918] = mapper918;
    mapper[919] = mapper919;
    mapper[920] = mapper920;
    mapper[921] = mapper921;
    mapper[922] = mapper922;
    mapper[923] = mapper923;
    mapper[924] = mapper924;
    mapper[925] = mapper925;
    mapper[926] = mapper926;
    mapper[927] = mapper927;
    mapper[928] = mapper928;
    mapper[929] = mapper929;
    mapper[930] = mapper930;
    mapper[931] = mapper931;
    mapper[932] = mapper932;
    mapper[933] = mapper933;
    mapper[934] = mapper934;
    mapper[935] = mapper935;
    mapper[936] = mapper936;
    mapper[937] = mapper937;
    mapper[938] = mapper938;
    mapper[939] = mapper939;
    mapper[940] = mapper940;
    mapper[941] = mapper941;
    mapper[942] = mapper942;
    mapper[943] = mapper943;
    mapper[944] = mapper944;
    mapper[945] = mapper945;
    mapper[946] = mapper946;
    mapper[947] = mapper947;
    mapper[948] = mapper948;
    mapper[949] = mapper949;
    mapper[950] = mapper950;
    mapper[951] = mapper951;
    mapper[952] = mapper952;
    mapper[953] = mapper953;
    mapper[954] = mapper954;
    mapper[955] = mapper955;
    mapper[956] = mapper956;
    mapper[957] = mapper957;
    mapper[958] = mapper958;
    mapper[959] = mapper959;
    mapper[960] = mapper960;
    mapper[961] = mapper961;
    mapper[962] = mapper962;
    mapper[963] = mapper963;
    mapper[964] = mapper964;
    mapper[965] = mapper965;
    mapper[966] = mapper966;
    mapper[967] = mapper967;
    mapper[968] = mapper968;
    mapper[969] = mapper969;
    mapper[970] = mapper970;
    mapper[971] = mapper971;
    mapper[972] = mapper972;
    mapper[973] = mapper973;
    mapper[974] = mapper974;
    mapper[975] = mapper975;
    mapper[976] = mapper976;
    mapper[977] = mapper977;
    mapper[978] = mapper978;
    mapper[979] = mapper979;
    mapper[980] = mapper980;
    mapper[981] = mapper981;
    mapper[982] = mapper982;
    mapper[983] = mapper983;
    mapper[984] = mapper984;
    mapper[985] = mapper985;
    mapper[986] = mapper986;
    mapper[987] = mapper987;
    mapper[988] = mapper988;
    mapper[989] = mapper989;
    mapper[990] = mapper990;
    mapper[991] = mapper991;
    mapper[992] = mapper992;
    mapper[993] = mapper993;
    mapper[994] = mapper994;
    mapper[995] = mapper995;
    mapper[996] = mapper996;
    mapper[997] = mapper997;
    mapper[998] = mapper998;
    mapper[999] = mapper999;
    mapper[1000] = mapper1000;
    mapper[1001] = mapper1001;
    mapper[1002] = mapper1002;
    mapper[1003] = mapper1003;
    mapper[1004] = mapper1004;
    mapper[1005] = mapper1005;
    mapper[1006] = mapper1006;
    mapper[1007] = mapper1007;
    mapper[1008] = mapper1008;
    mapper[1009] = mapper1009;
    mapper[1010] = mapper1010;
    mapper[1011] = mapper1011;
    mapper[1012] = mapper1012;
    mapper[1013] = mapper1013;
    mapper[1014] = mapper1014;
    mapper[1015] = mapper1015;
    mapper[1016] = mapper1016;
    mapper[1017] = mapper1017;
    mapper[1018] = mapper1018;
    mapper[1019] = mapper1019;
    mapper[1020] = mapper1020;
    mapper[1021] = mapper1021;
    mapper[1022] = mapper1022;
    mapper[1023] = mapper1023;
}

char mapping(int idx, std::string &str) {
    // fucking have to make map functions

    char code;

    code = 99;

    if (!str.compare(string("두")) || !str.compare(string("두"))) code = 0;
    if (!str.compare(string("강")) || !str.compare(string("강"))) code = 1;
    if (!str.compare(string("남")) || !str.compare(string("납")) || !str.compare(string("냠")) || !str.compare(string("냡"))) code = 2;
    if (!str.compare(string("스")) || !str.compare(string("스"))) code = 3;
    if (!str.compare(string("타")) || !str.compare(string("타"))) code = 4;
    if (!str.compare(string("일")) || !str.compare(string("일"))) code = 5;
    if (!str.compare(string("김")) || !str.compare(string("검"))) code = 6;
    if (!str.compare(string("치")) || !str.compare(string("치"))) code = 7;
    if (!str.compare(string("불")) || !str.compare(string("불"))) code = 8;
    if (!str.compare(string("고")) || !str.compare(string("고"))) code = 9;
    if (!str.compare(string("기")) || !str.compare(string("기"))) code = 10;
    if (!str.compare(string("한")) || !str.compare(string("한"))) code = 11;
    if (!str.compare(string("우")) || !str.compare(string("우"))) code = 12;
    if (!str.compare(string("장")) || !str.compare(string("장"))) code = 13;
    if (!str.compare(string("어")) || !str.compare(string("어"))) code = 14;
    if (!str.compare(string("돼")) || !str.compare(string("돼"))) code = 15;
    if (!str.compare(string("지")) || !str.compare(string("지"))) code = 16;
    if (!str.compare(string("구")) || !str.compare(string("구"))) code = 17;
    if (!str.compare(string("글")) || !str.compare(string("글"))) code = 18;
    if (!str.compare(string("좋")) || !str.compare(string("좋"))) code = 19;
    if (!str.compare(string("아")) || !str.compare(string("아"))) code = 20;
    if (!str.compare(string("마")) || !str.compare(string("마"))) code = 21;
    if (!str.compare(string("법")) || !str.compare(string("빕"))) code = 22;
    if (!str.compare(string("사")) || !str.compare(string("사"))) code = 23;
    if (!str.compare(string("태")) || !str.compare(string("태"))) code = 24;
    if (!str.compare(string("국")) || !str.compare(string("국"))) code = 25;
    if (!str.compare(string("정")) || !str.compare(string("정"))) code = 26;
    if (!str.compare(string("보")) || !str.compare(string("보"))) code = 27;
    if (!str.compare(string("술")) || !str.compare(string("술"))) code = 28;
    if (!str.compare(string("원")) || !str.compare(string("윈"))) code = 29;
    if (!str.compare(string("임")) || !str.compare(string("임"))) code = 30;
    if (!str.compare(string("준")) || !str.compare(string("준"))) code = 31;

    // puts("mapping !");
    // printf("idx: %d\ncode: %d\n", idx, code);

    // return code;

    return (*mapper[idx % 1024])(idx, code);
}

// check answer WTF

void correctAnswer() {
    cout << "Correct" << endl;
    // if pwn the system, attacker also can get reversing flag.
    system("cat /home/kimchi-vm/reversing_flag.txt");
}

void wrongAnswer() {
    cout << "Wrong" << endl;
}


void makeTestCase() {
    unsigned int prevSTDIN = 0;

    char randa[4];

    int fd2 = open("/dev/urandom", O_RDONLY);
    read(fd2, randa, sizeof(randa));
    close(fd2);

    srand((*(int*)randa));


    char buf[256];
    for (int i=0; i<128; i++) {
        MYSTDIN[i] = rand() % 30;
        memset(buf, 0, 256);
        sprintf(buf,"%ld\n", MYSTDIN[i] * 10 + prevSTDIN);
        ANSWERSTDOUT += std::string(buf);

        prevSTDIN = MYSTDIN[i];
    }

    // cout << ANSWERSTDOUT << endl;

}

void emulate(char code[]) {
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
            sprintf(buf,"%ld\n", REGISTERS[code[1]]);
            MYSTDOUT += std::string(buf);
            // printf("%ld\n", REGISTERS[code[1]]);
            break;

        case INS_PRT_CHAR:
            // MYSTDOUT += ~
            // user can choose format string
            memset(buf, 0, 256);
            sprintf(buf,"%c\n", REGISTERS[code[1]]);
            MYSTDOUT += std::string(buf);
            // printf("%c\n", REGISTERS[code[1]]);
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
            // puts("JMP !");
            // printf("CODE_RIP: %d\n", CODE_RIP);
            CODE_RIP = CODE_RIP + REGISTERS[code[1]];
            // printf("CODE_RIP: %d\n", CODE_RIP);
            break;

        case INS_LESS_JMP:
            if (LESS_FLAG) {
                CODE_RIP = CODE_RIP + REGISTERS[code[1]];
            }
            break;

        case INS_BIGGER_JMP:
            if (BIGGER_FLAG) {
                CODE_RIP = CODE_RIP + REGISTERS[code[1]];
            }
            break;

        case INS_SAME_JMP:
            if (SAME_FLAG) {
                CODE_RIP = CODE_RIP + REGISTERS[code[1]];
            }
            break;

        case INS_MOV:
            REGISTERS[code[1]] = REGISTERS[code[2]];
            break;

        case INS_MOV_PTR:
            REGISTERS[code[1]] = *(long *)(REGISTERS[code[2]]);
            break;

        case INS_PTR_MOV:
            *(long *)(REGISTERS[code[1]]) = (REGISTERS[code[2]]);
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

        case INS_RET:
            CODE_RIP = 8192;
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

    char mappedCode[8192];
    char liveCode[20];

    setvbuf(stdin, 0, 2, 0);
    setvbuf(stdout, 0, 2, 0);

    initMapper();

    scanf("%d", &fileSize);


    char filename[] = "/tmp/prob/image.XXXXXX";
    fd = mkstemp(filename);

    while (fileSize) {
        inputSize = read(0, buffer, 1);
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

    // std::cout << s2 << "\n";

    codeLen = s2.length() / 3;

    // printf("codeLen: %d\n", codeLen);

    // puts("code mappp");

    // code mapping
    for (int i=0; i<codeLen*3; i+=3) {
        oneChar = s2.substr(i, 3);
        // std::cout << oneChar << std::endl; // test code

        mappedCode[i/3] = mapping(i/3, oneChar);

        // printf("%d, ", mappedCode[i/3]);
    }

    // puts("code mappp");

    delete [] outText; // get library code

    // make test case

    makeTestCase(); // done

    // code emulate


    CODE_RIP = 0;

    // puts("== output ==");

    
    /*

    char zzzz[] = {
        INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,INS_GET, 1, INS_SEPERATOR,
        INS_GET, 1, INS_SEPERATOR,
        INS_PRT, 1, INS_SEPERATOR
    };
    

    for (int k=0; k<sizeof(zzzz); k++)
        mappedCode[k] = zzzz[k];

    printf("size: %d\n", sizeof(zzzz));

    
    */


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
    pixDestroy(&image);

    unlink(filename);

    // cout << "Finish" << endl;

    exit(1);

    return 0;
}

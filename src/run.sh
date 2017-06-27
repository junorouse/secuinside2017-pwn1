#!/bin/sh

g++ -o main main.cpp mapper.h mapper.cpp -std=c++11 -I/usr/include/leptonica -I/usr/local/include/tesseract -llept -ltesseract
(python -c 'print "19836"+"\x00"*4091+open("rev.png").read()+"\n"';cat) | ./main


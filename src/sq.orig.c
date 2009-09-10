/*
* Copyright (c) 2009 David Roberts <d@vidr.cc>
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

/**
This file is the unobfuscated and commented version of sq.c, a simple SUBLEQ
interpreter. It should function similarly to sqrun[1].

USAGE:
sq file.sq

file.sq should be a file of the same format as that output by sqasm[1].
It must not contain any unresolved symbols
    e.g. #IN and #OUT should be replaced by -1

[1] <http://mazonka.com/subleq/>
*/

#include <stdio.h>
int main(int argc, char *argv[]) {
    FILE *fin = fopen(argv[1], "r");
    int PC = 0; /* Program Counter */
    int mem[9999];
    int *i = mem;
    while(fscanf(fin, "%d", i++) > 0); /* read fin into mem */
    while(PC >= 0) {
        int a = mem[PC++], b = mem[PC++], c = mem[PC++];
        if(a < 0) mem[b] += (int)getchar(); /* input */
        else if(b < 0) printf("%c", (char)mem[a]); /* output */
        else if((mem[b] -= mem[a]) <= 0) PC = c; /* subtract and branch */
    }
}

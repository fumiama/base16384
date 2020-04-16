//base16384.c
//MIT fumiama 20200416
#include <stdio.h>
#include <stdlib.h>
#include "base16384.h"

#define CHOICE argv[1][1]
#define READ argv[2]
#define WRITE argv[3]

int main(int argc, char **argv){
    FILE *fp = NULL;
    FILE *fpo = NULL;
    
    if(argc != 4){
        fputs("Usage: -[e|d] inputfile outputfile", stderr);
        fputs("       -e encode", stderr);
        fputs("       -d decode", stderr);
        exit(EXIT_FAILURE);
    }

    fp = fopen(READ, "rb");
    fpo = fopen(WRITE, "wb");
    if(!fp || !fpo){
        fputs("IO error!", stderr);
        exit(EXIT_FAILURE);
    }
    switch(CHOICE){
        case 'e': encode(fp, fpo); break;
        case 'd': decode(fp, fpo); break;
        default: break;
    }
    return 0;
}

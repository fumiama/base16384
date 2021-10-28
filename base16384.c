#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "base16384.h"

void encode_file(const char* input, const char* output) {
	FILE* fp = NULL;
	fp = fopen(input, "rb");
	if(fp) {
		FILE* fpo = NULL;
		fpo = fopen(output, "wb");
		if(fpo) {
			uint8_t* bufi = (uint8_t*)malloc(B14BUFSIZ/7*7);
			if(bufi) {
				int cnt = 0;
				fputc(0xFE, fpo);
    			fputc(0xFF, fpo);
				while((cnt = fread(bufi, sizeof(uint8_t), B14BUFSIZ/7*7, fp))) {
					LENDAT* ld = encode(bufi, cnt);
					if(fwrite(ld->data, ld->len, 1, fpo) <= 0) {
						puts("Write file error!");
						exit(EXIT_FAILURE);
					}
					free(ld->data);
					free(ld);
				}
				free(bufi);
			} else puts("Allocate input buffer error!");
			fclose(fpo);
		} else puts("Open output file error!");
		fclose(fp);
	} else puts("Open input file error!");
}

void rm_head(FILE* fp) {
	int ch = fgetc(fp);
	if(ch == 0xFE) fgetc(fp);
	else rewind(fp);
}

static int is_next_end(FILE* fp) {
	int ch = fgetc(fp);
	if(ch == '=') return fgetc(fp);
	else {
		ungetc(ch, fp);
		return 0;
	}
}

void decode_file(const char* input, const char* output) {
	FILE* fp = NULL;
	fp = fopen(input, "rb");
	if(fp) {
		FILE* fpo = NULL;
		fpo = fopen(output, "wb");
		if(fpo) {
			uint8_t* bufi = (uint8_t*)malloc(B14BUFSIZ/8*8 + 2);		//+2避免漏检结束偏移标志
			if(bufi) {
				int cnt = 0;
				int end = 0;
				rm_head(fp);
				while((cnt = fread(bufi, sizeof(uint8_t), B14BUFSIZ/8*8, fp))) {
					if((end = is_next_end(fp))) {
						bufi[cnt++] = '=';
						bufi[cnt++] = end;
					}
					LENDAT* ld = decode(bufi, cnt);
					if(fwrite(ld->data, ld->len, 1, fpo) <= 0) {
						puts("Write file error!");
						exit(EXIT_FAILURE);
					}
					free(ld->data);
					free(ld);
				}
				free(bufi);
			} else puts("Allocate input buffer error!");
			fclose(fpo);
		} else puts("Open output file error!");
		fclose(fp);
	} else puts("Open input file error!");
}

unsigned long get_start_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

#define CHOICE argv[1][1]
int main(int argc, char** argv) {
	if(argc == 4) {
		unsigned long t = get_start_ms();
        switch(CHOICE){
            case 'e': encode_file(argv[2], argv[3]); break;
            case 'd': decode_file(argv[2], argv[3]); break;
            default: break;
        }
        printf("spend time: %lums\n", get_start_ms() - t);
	} else {
        fputs("Usage: -[e|d] <inputfile> <outputfile>\n", stderr);
        fputs("\t-e encode\n", stderr);
        fputs("\t-d decode\n", stderr);
        exit(EXIT_FAILURE);
    }
    return 0;
}

#ifndef __cosmopolitan
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#ifdef __WINNT__
	#include <windows.h>
#endif
#endif
#include "base14.h"

#ifdef __cosmopolitan
#define get_file_size(filepath) ((off_t)GetFileSize(filepath))
#else
static off_t get_file_size(const char* filepath) {
    struct stat statbuf;
    return stat(filepath, &statbuf)?-1:statbuf.st_size;
}
#endif

void encode_file(const char* input, const char* output) {
	off_t inputsize = get_file_size(input);
	if(inputsize <= 0) {
		puts("Get file size error!");
		return;
	}
	FILE* fp = NULL;
	fp = fopen(input, "rb");
	if(!fp) {
		puts("Open input file error!");
		return;
	}
	FILE* fpo = NULL;
	fpo = fopen(output, "wb");
	if(!fpo) {
		puts("Open output file error!");
		return;
	}
	if(inputsize > BUFSIZ*1024) inputsize = BUFSIZ*1024/7*7; // big file
	char* bufi = (char*)malloc(inputsize);
	if(!bufi) {
		puts("Allocate input buffer error!");
		return;
	}
	int outputsize = encode_len(inputsize)+16;
	char* bufo = (char*)malloc(outputsize);
	if(!bufo) {
		puts("Allocate output buffer error!");
		return;
	}
	size_t cnt = 0;
	fputc(0xFE, fpo);
	fputc(0xFF, fpo);
	while((cnt = fread(bufi, sizeof(char), inputsize, fp))) {
		int n = encode(bufi, cnt, bufo, outputsize);
		if(fwrite(bufo, n, 1, fpo) <= 0) {
			puts("Write file error!");
			return;
		}
	}
	/* 由操作系统负责释放资源
		free(bufo);
		free(bufi);
		fclose(fpo);
		fclose(fp);
	以缩短程序运行时间 */
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
	off_t inputsize = get_file_size(input);
	if(inputsize <= 0) {
		puts("Get file size error!");
		return;
	}
	FILE* fp = NULL;
	fp = fopen(input, "rb");
	if(!fp) {
		puts("Open input file error!");
		return;
	}
	FILE* fpo = NULL;
	fpo = fopen(output, "wb");
	if(!fpo) {
		puts("Open output file error!");
		return;
	}
	if(inputsize > BUFSIZ*1024) inputsize = BUFSIZ*1024/8*8; // big file
	char* bufi = (char*)malloc(inputsize+2); // +2避免漏检结束偏移标志
	if(!bufi) {
		puts("Allocate input buffer error!");
		return;
	}
	int outputsize = decode_len(inputsize, 0)+16;
	char* bufo = (char*)malloc(outputsize);
	if(!bufo) {
		puts("Allocate output buffer error!");
		return;
	}
	int cnt = 0;
	int end = 0;
	rm_head(fp);
	while((cnt = fread(bufi, sizeof(char), inputsize, fp))) {
		if((end = is_next_end(fp))) {
			bufi[cnt++] = '=';
			bufi[cnt++] = end;
		}
		int n = decode(bufi, cnt, bufo, outputsize);
		if(fwrite(bufo, n, 1, fpo) <= 0) {
			puts("Write file error!");
			return;
		}
	}
	/* 由操作系统负责释放资源
		free(bufo);
		free(bufi);
		fclose(fpo);
		fclose(fp);
	以缩短程序运行时间 */
}

#ifndef __WINNT__
unsigned long get_start_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#endif

#define CHOICE argv[1][1]
int main(int argc, char** argv) {
	if(argc != 4) {
        fputs("Usage: -[e|d] <inputfile> <outputfile>\n", stderr);
        fputs("\t-e encode\n", stderr);
        fputs("\t-d decode\n", stderr);
        exit(EXIT_FAILURE);
    }
	#ifdef __WINNT__
		clock_t t = clock();
	#else
		unsigned long t = get_start_ms();
	#endif
	switch(CHOICE) {
		case 'e': encode_file(argv[2], argv[3]); break;
		case 'd': decode_file(argv[2], argv[3]); break;
		default: break;
	}
	#ifdef __WINNT__
		printf("spend time: %lums\n", clock() - t);
	#else
		printf("spend time: %lums\n", get_start_ms() - t);
	#endif
    return 0;
}

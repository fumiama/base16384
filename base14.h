#ifndef _BASE14_H_
#define _BASE14_H_

// base14.h
// fumiama 20220319

// encode_len calc min buf size to fill encode result
int encode_len(int dlen) {
	int outlen = dlen / 7 * 8;
	int offset = dlen % 7;
	switch(offset) {	// 算上偏移标志字符占用的2字节
		case 0: break;
		case 1: outlen += 4; break;
		case 2:
		case 3: outlen += 6; break;
		case 4:
		case 5: outlen += 8; break;
		case 6: outlen += 10; break;
		default: break;
	}
	return outlen + 8;	// 冗余的8B用于可能的结尾的覆盖
}

// decode_len calc min buf size to fill decode result
int decode_len(int dlen, int offset) {
	int outlen = dlen;
	switch(offset) {	// 算上偏移标志字符占用的2字节
		case 0: break;
		case 1: outlen -= 4; break;
		case 2:
		case 3: outlen -= 6; break;
		case 4:
		case 5: outlen -= 8; break;
		case 6: outlen -= 10; break;
		default: break;
	}
	return outlen / 8 * 7 + offset + 1; // 多出1字节用于循环覆盖
}

// encode data and write result into buf
int encode(const char* data, int dlen, char* buf, int blen);
// decode data and write result into buf
int decode(const char* data, int dlen, char* buf, int blen);

#endif
// base14.h
// fumiama 20220319

// encode_len calc min buf size to fill encode result
int encode_len(int dlen);
// decode_len calc min buf size to fill decode result
int decode_len(int dlen, int offset);

// encode data and write result into buf
int encode(const char* data, int dlen, char* buf, int blen);
// decode data and write result into buf
int decode(const char* data, int dlen, char* buf, int blen);

//base1464le.h
//fumiama 20210408
#include <stdint.h>

#define B14BUFSIZ 16384
struct LENDAT {
    uint8_t* data;
    uint64_t len;
};
typedef struct LENDAT LENDAT;

LENDAT* encode(const uint8_t* data, const u_int64_t len);
LENDAT* decode(const uint8_t* data, const u_int64_t len);

//base1464le.h
//fumiama 20210408
#include <stdint.h>

#define B14BUFSIZ 16384
struct LENDAT {
    uint8_t* data;
    int64_t len;
};
typedef struct LENDAT LENDAT;

LENDAT* encode(const uint8_t* data, const int64_t len);
LENDAT* decode(const uint8_t* data, const int64_t len);

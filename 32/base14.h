//base1432le.h
//fumiama 20210408
#include <stdint.h>

#define B14BUFSIZ 8192
struct LENDAT {
    uint8_t* data;
    int32_t len;
};
typedef struct LENDAT LENDAT;

LENDAT* encode(const uint8_t* data, const int32_t len);
LENDAT* decode(const uint8_t* data, const int32_t len);

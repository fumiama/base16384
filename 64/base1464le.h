#include <stdint.h>

#define B14BUFSIZ 16384
struct LENDAT {
    uint8_t* data;
    uint64_t len;
};
typedef struct LENDAT LENDAT;

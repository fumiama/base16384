#ifndef CPUBIT32
    #ifndef CPUBIT64
        #define CPUBIT32
    #endif
#endif
#ifdef CPUBIT32
    #include "./32/base1432le.h"
#endif
#ifdef CPUBIT64
    #include "./64/base1464le.h"
#endif


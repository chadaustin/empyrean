#ifndef PYR_ASSERT_H
#define PYR_ASSERT_H


#include <assert.h>


#define PYR_REQUIRE_SEMI \
    do { } while (false)



#if defined(DEBUG) || defined(_DEBUG)

    #ifdef _MSC_VER
    
        #include <windows.h>
    
        #define PYR_ASSERT(condition, label)                            \
            if (!(condition)) {                                         \
                MessageBox(NULL, (label), "Empyrean Assertion", MB_OK); \
                __asm int 3                                             \
            } PYR_REQUIRE_SEMI
        
    #else
    
        #define PYR_ASSERT(condition, label) \
            assert((condition) && (label))
    
    #endif

#else

    #define PYR_ASSERT(condition, label) \
        PYR_REQUIRE_SEMI

#endif


#endif

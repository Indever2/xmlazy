#ifdef __APPLE__
    #include "TargetConditionals.h"
    #ifdef TARGET_OS_MAC
        #include "osx_uchar_fixes.h"
    #endif
#else
    #include <uchar.h>
#endif


char32_t *char32_alloc(unsigned);
int strlen32(const char32_t*);

void *strncpy32(char32_t *, const char32_t *, size_t);
int utf8_multibyte_char_to_char32(char32_t **, const char *, size_t);
int utf8_char32_string_to_multibyte(char**, const char32_t*, size_t);

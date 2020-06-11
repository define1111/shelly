#include "../include/shelly_string.h"

int
string_search_2_symbols(const char *str, char ch1, char ch2) /* mb va_arg ? */
{
    for (; *str; ++str)
        if (*str == ch1 || *str == ch2)
            return 1;
    return 0;
}

int
string_length(const char *str)
{
    const char *ptr;
    for (ptr = str; *ptr; ++ptr) {}
    return ptr - str;
}

int 
string_compare(const char *str1, const char *str2)
{
    for (; *str1 && *str1 == *str2; ++str1, ++str2)
        ;
    return (unsigned char) *str1 - (unsigned char) *str2;
}

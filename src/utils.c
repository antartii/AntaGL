#include "utils.h"

char *int_to_str(int n, char *buffer)
{
    char *ptr_reversed = buffer;
    char *ptr = buffer;
    int tmp_n;
    char tmp_char;

    if (n < 0) {
        *ptr_reversed++ = '-';
        n = -n;
    }

    do {
        tmp_n = n;
        n /= 10;
        *ptr_reversed++ = '0' + (tmp_n - n * 10);
    } while (n);

    *ptr_reversed-- = '\0';
    
    while (ptr < ptr_reversed) {
        tmp_char = *ptr_reversed;
        *ptr_reversed-- = *ptr;
        *ptr = tmp_char;
    }

    return buffer;
}

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

int clamp_int(int val, int min, int max)
{
    return val < min ? min : (val > max ? max : val);
}

int max_int(int val, int max)
{
    return val > max ? val : max;
}

char *read_file(const char *file_name, uint32_t *buffer_size)
{
    FILE *file = fopen(file_name, "rb");
    *buffer_size = 0;
    char *buffer = NULL;

    if (file) {
        fseek(file, 0, SEEK_END);
        *buffer_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        buffer = malloc(sizeof(char) * (*buffer_size));
        if (buffer)
            fread(buffer, 1, *buffer_size, file);
        fclose(file);
    }
    #ifdef DEBUG 
    else
        printf("Couldn't open the file %s\n", file_name);
    #endif

    return buffer;
}

void find_circle_point(vec2 center, float radius, float degreesAngle, vec2 dest)
{
    double sinAngle = sin(DEGREES_TO_RADIANS(degreesAngle));
    double cosAngle = cos(DEGREES_TO_RADIANS(degreesAngle));

    glm_vec2_copy((vec2) {center[0] + (radius * cosAngle), center[1] + (radius * sinAngle)}, dest);
}

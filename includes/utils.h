#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

/**
 * @brief Convert an integer to a string
 * 
 * @param n Integer to convert to a string
 * @param buffer Pointer to a buffer that will store the string, the caller have to make sure the buffer have enough allocated space
 * @return The same pointer as the buffer
 */
char *int_to_str(int n, char *buffer);

struct version {
    int major;
    int minor;
    int patch;
};

int clamp_int(int val, int min, int max);
int max_int(int val, int max);
char *read_file(const char *file_name, uint32_t *buffer_size);

#endif

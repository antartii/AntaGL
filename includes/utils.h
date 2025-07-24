#ifndef _UTILS_H
    #define _UTILS_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <unistd.h>

    /** 
     * @def UNDEFINED
     * @brief Undefined state for integer results, enums or bitmasks
     */
    #define UNDEFINED -1
    /**
     * @def NONE
     * @brief None state when dealing with multiple choices
     */
    #define NONE 0

/**
 * @brief Convert an integer to a string
 * 
 * @param n Integer to convert to a string
 * @param buffer Pointer to a buffer that will store the string, the caller have to make sure the buffer have enough allocated space
 * @return The buffer used to store the string
 */
char *int_to_str(int n, char *buffer);

/**
 * @struct version
 * @brief structure representing a version
 * @var version::major
 * Major field of the version
 * @var version::minor
 * Minor field of the version
 * @var version::patch
 * Patch field of the version
 */
struct version {
    int major;
    int minor;
    int patch;
};

/**
 * @brief Clamps an integer value between a minimum and maximum.
 *
 * @param val Value to clamp.
 * @param min Lower bound of the range.
 * @param max Upper bound of the range.
 * @return The clamped integer value within [min, max].
 */
int clamp_int(int val, int min, int max);

/**
 * @brief Return the higher value between two integers
 * 
 * @param val First value to compare
 * @param max Second value to compare
 * @return The higher value between the two integers
 */
int max_int(int val, int max);

/**
 * @brief Read a file and store it's content into an allocated string
 * 
 * @param file_name Absolute or relative path of the file to read
 * @param buffer_size Pointer to a unsigned int, storing the size of the buffer
 * @return An allocated string containing the content of a file
 */
char *read_file(const char *file_name, uint32_t *buffer_size);

#endif

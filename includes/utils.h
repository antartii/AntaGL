#ifndef _UTILS_H
#define _UTILS_H

/**
 * @brief Convert an integer to a string
 * 
 * @param n Integer to convert to a string
 * @param buffer Pointer to a buffer that will store the string, the caller have to make sure the buffer have enough allocated space
 * @return The same pointer as the buffer
 */
char *int_to_str(int n, char *buffer);

#endif

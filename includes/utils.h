#ifndef _UTILS_H
#define _UTILS_H

struct version {
    int variant;
    int major;
    int minor;
    int patch;
};

char *int_to_str(int n, char *buffer);

#endif

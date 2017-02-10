#include "lwm2m.h"

/// BIG TODO ALL FREE FUNCTIONS


char *itoa(int num) {
    char *buffer = (char *) malloc(sizeof(char) * 10);
    sprintf(buffer, "%d", num);
    return buffer;
}

char *copy_str(char *str) {
    int len = strlen(str);
    char *copy = malloc(len + 1);
    copy[0] = copy[len] = 0;
    memcpy(copy, str, len);
    return copy;
}
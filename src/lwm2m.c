#include "lwm2m.h"

char *itoa(int num) {
    char *buffer = (char *) malloc(sizeof(char) * 10);
    sprintf(buffer, "%d", num);
    return buffer;
}

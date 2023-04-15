#ifndef COMMON_H
#define COMMON_H

#include <string>

#define READ 0
#define WRITE 1

typedef enum {IO, ADD, PIPELINE, NO_REDIRECTION} InputOutputType;

typedef struct {
    char *stream;
    InputOutputType type;
} Redirection;

#endif // COMMON_H
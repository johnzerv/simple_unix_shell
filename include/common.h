#ifndef COMMON_H
#define COMMON_H

#include <string>

typedef enum {IO, ADD, NO_REDIRECTION} RedirectionType;

typedef struct {
    char *stream;
    RedirectionType type;
} Redirection;

#endif // COMMON_H
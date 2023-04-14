#ifndef COMMON_H
#define COMMON_H

#include <string>

#define READ 0
#define WRITE 1

typedef enum {IO, ADD, NO_REDIRECTION} RedirectionType;

typedef struct {
    char *stream;
    RedirectionType type;
} Redirection;

#endif // COMMON_H
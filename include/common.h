#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <list>
#include <map>
#include <cassert>
#include <fstream>
#include <sstream>

#define READ 0
#define WRITE 1

typedef enum {IO, ADD, PIPELINE, NO_REDIRECTION} InputOutputType;

typedef struct {
    char *stream;
    InputOutputType type;
} RedirectionPacket;

#endif // COMMON_H
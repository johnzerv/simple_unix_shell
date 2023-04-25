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

typedef enum {IO, ADD, PIPELINE, NO_REDIRECTION} InputOutputType;   // Redirections, Pipeline, I/O

typedef struct {
    char *stream;
    InputOutputType type;
} IOPacket;         // Input-Output Packet

#endif // COMMON_H
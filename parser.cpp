
#include <cstring>
#include <assert.h>

#include "parser.h"

Command* Parser::parse(char *command) {
    unsigned int length = strlen(command);
    assert(length >= 0);

    char program[100];
    int i;
    for (i = 0; i < length && command[i] != ' '; i++) {
        program[i] = command[i];
    }
    program[i] = '\0';

    while(command[i++] == ' ');   // skip whitepaces

    char output_str[100], input_str[100];
    RedirectionType output_rt = NO_REDIRECTION, input_rt = NO_REDIRECTION;

    // if (command[i] == '>') {
    //     output_rt = 
    //     if (command[i+1] == '>') {
    //         output_rt = ADD;
    //     }
    // }
}
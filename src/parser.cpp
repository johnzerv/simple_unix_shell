
#include <cstring>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "parser.h"

// Command* Parser::parse(char *command) {
//     unsigned int length = strlen(command);
//     assert(length >= 0);

//     char program[100];
//     int i;
//     for (i = 0; i < length && command[i] != ' '; i++) {
//         program[i] = command[i];
//     }
//     program[i] = '\0';

//     while(command[i++] == ' ');   // skip whitepaces

//     char output_str[100], input_str[100];
//     RedirectionType output_rt = NO_REDIRECTION, input_rt = NO_REDIRECTION;

//     // if (command[i] == '>') {
//     //     output_rt = 
//     //     if (command[i+1] == '>') {
//     //         output_rt = ADD;
//     //     }
//     // }
// }

Parser::Parser(FILE *input_stream) {
    this->input_stream = input_stream;
    lookahead = fgetc(input_stream);
    exit_keyword_appeared = false;
    cmd = nullptr;
}

Parser::~Parser() {
    if (cmd != nullptr) {
        delete cmd;
    }
}

void Parser::consume(char symbol) {
    if (lookahead == symbol) {
        lookahead = fgetc(input_stream);
    }
}

bool Parser::is_valid_symbol(char symbol) {
    if ((symbol >= 'A' && symbol <= 'z') || (symbol >= '0' && symbol <= '9')) {
        return true;
    }

    return false;
}

bool Parser::is_invalid_symbol(char symbol) {
    if (symbol == ' ' || symbol == '\n' || symbol == '<'
     || symbol == '>' || symbol == '|' || symbol == '\t'
     || symbol == '&' || symbol == ';' || symbol == ':'
     || symbol ==   '/') {
        return true;
     }

    return false;
}

void Parser::command() {
    // Skip new lines
    while (lookahead == '\n') {
        consume(lookahead);
    }

    // Check if exit has been requested
    if (lookahead == 'e') {
        consume('e');
        if (lookahead == 'x') {
            consume('x');
            if (lookahead == 'i') {
                consume('i');
                if (lookahead == 't') {
                    consume('t');
                    exit_keyword_appeared = true;
                    return;

                } else ungetc(lookahead, input_stream);
            } else ungetc(lookahead, input_stream);
        } else ungetc(lookahead, input_stream);
    }

    std::string id(identifier());
    std::list<std::string> args = arguments();
    Redirection input_redirection = input_file();
    Redirection output_redirection = output_file();
    std::string input_r = (input_redirection.stream == nullptr) ? "" : input_redirection.stream;
    std::string output_r = (output_redirection.stream == nullptr) ? "" : output_redirection.stream;

    cmd = new Command(id, args, input_r, input_redirection.type,
                                    output_r, output_redirection.type);

}

std::list<std::string> Parser::arguments() {
    std::list<std::string> args = std::list<std::string>();

    while (lookahead == ' ' || lookahead == 't') {
        consume(lookahead);
    }

    if (is_valid_symbol(lookahead)) {
        std::string id(identifier());

        args.push_front(id);

        std::list<std::string> rest_args = arguments();

        args.splice(args.end(), rest_args); // merge two lists without sorting
    }

    return args;
}

Redirection Parser::input_file() {
    Redirection input_redirection;
    input_redirection.stream = nullptr;
    input_redirection.type = NO_REDIRECTION;

    while (lookahead == ' ' || lookahead == '\t') {
        consume(lookahead);
    }

    if (lookahead == '<') {
        consume('<');
        input_redirection.stream = identifier();
        input_redirection.type = IO;
    }

    return input_redirection;
}

Redirection Parser::output_file() {
    Redirection output_redirection;
    output_redirection.stream = nullptr;
    output_redirection.type = NO_REDIRECTION;

    while (lookahead == ' ' || lookahead == '\t') {
        consume(lookahead);
    }

    if (lookahead == '>') {
        consume('>');
        output_redirection.type = IO;

        if (lookahead == '>') {
            consume('>');
            output_redirection.type = ADD;
        }
        output_redirection.stream = identifier();
    }

    return output_redirection;
}

char* Parser::identifier() {
    char *id = new char[100];
    unsigned int index = 0;

    // Skip whitespaces
    while (lookahead == ' ' || lookahead == '\t') {
        consume(lookahead);
    }

    while (!is_invalid_symbol(lookahead)) {
        id[index++] = lookahead;
        consume(lookahead);
    }
    id[index] = '\0';

    return id;
}
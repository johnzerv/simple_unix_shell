
#include <cstring>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "parser.h"

Parser::Parser(FILE *input_stream) {
    this->input_stream = input_stream;
    lookahead = fgetc(input_stream);
    exit_keyword_appeared = false;
}

Parser::~Parser() {
    if (!commands.empty()) {
        for (std::list<Command*>::iterator it = commands.begin(); it != commands.end(); it++) {
            if ((*it) != nullptr) {
                delete *it;
            }
        }
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
     || symbol == '&' || symbol == ';' || symbol == ':') {
        return true;
     }

    return false;
}

bool Parser::command(bool is_from_pipeline) {
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
                    return true;

                } else ungetc(lookahead, input_stream);
            } else ungetc(lookahead, input_stream);
        } else ungetc(lookahead, input_stream);
    }

    std::string id(identifier());
    std::list<std::string> args = arguments();
    Redirection input_redirection = input_file();
    Redirection output_redirection = output_file();
    
    // Pipeline info, TODO: need of better comments
    if (is_from_pipeline) {
        if (input_redirection.stream != nullptr) {
            return false;
        } else {
            input_redirection.type = PIPELINE;
        }
    }

    bool has_pipeline = pipeline();
    if (has_pipeline) {
        if (output_redirection.stream != nullptr) {
            return false;
        } else {
            output_redirection.type = PIPELINE;
        }
    }

    // Convert char* to string for Command constructor
    std::string input_str = (input_redirection.stream == nullptr) ? "" : input_redirection.stream;
    std::string output_str = (output_redirection.stream == nullptr) ? "" : output_redirection.stream;

    Command *cmd = new Command(id, args, input_str, input_redirection.type,
                                    output_str, output_redirection.type, is_from_pipeline);
    commands.push_front(cmd);

    return true;
}

std::list<std::string> Parser::arguments() {
    std::list<std::string> args = std::list<std::string>();

    skip_whitespaces();

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

    skip_whitespaces();

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

    skip_whitespaces();

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

bool Parser::pipeline() {
    skip_whitespaces();

    if (lookahead != '|') {
        return false;
    }

    consume ('|');
    skip_whitespaces();

    command(true);

    return true;
}

char* Parser::identifier() {
    char *id = new char[100];
    unsigned int index = 0;

    // Skip whitespaces
    skip_whitespaces();

    while (!is_invalid_symbol(lookahead)) {
        id[index++] = lookahead;
        consume(lookahead);
    }
    id[index] = '\0';

    return id;
}

void Parser::skip_whitespaces() {
    while (lookahead == ' ' || lookahead == '\t') {
        consume(lookahead);
    }
}
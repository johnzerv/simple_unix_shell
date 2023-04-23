#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <glob.h>
#include <fstream>
#include <iostream>

#include "parser.h"

Parser::Parser(std::istream *input_stream) {
    this->input_stream = input_stream;
    lookahead = input_stream->get();
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
        lookahead = input_stream->get();
    }
}

bool Parser::is_valid_symbol(char symbol) {
    if ((symbol >= 'A' && symbol <= 'z') || (symbol >= '0' && symbol <= '9')
      || symbol == '*' || symbol == '?' || symbol == '.' || symbol == '_') {
        return true;
    }

    return false;
}

bool Parser::is_invalid_symbol(char symbol) {
    if (symbol == ' ' || symbol == '\n' || symbol == '<'
     || symbol == '>' || symbol == '|' || symbol == '\t'
     || symbol == '&' || symbol == ';' || symbol == ':' || symbol == '\r') {
        return true;
     }

    return false;
}

bool Parser::is_exit_keyword_given() {
    char tmp_lookahead = lookahead;

    if (lookahead == 'e') {
        tmp_lookahead = lookahead;
        consume('e'); 

        if (lookahead == 'x') {
            tmp_lookahead = lookahead;
            consume('x');

            if (lookahead == 'i') {
                tmp_lookahead = lookahead;
                consume('i');

                if (lookahead == 't') {
                    tmp_lookahead = lookahead;
                    consume('t');

                    return true;
                }
                else {
                    input_stream->unget();
                    lookahead = tmp_lookahead;
                }
            }
            else {
                    input_stream->unget();
                    lookahead = tmp_lookahead;
            }
        }
        else {
            input_stream->unget();
            lookahead = tmp_lookahead;
        }
    }

    return false;
}

bool Parser::command(bool is_from_pipeline) {    // Skip new lines

    while (lookahead == '\n') {
        consume(lookahead);
    }

    // Check if exit has been requested
    if (is_exit_keyword_given()) {
        exit_keyword_appeared = true;
        return true;
    }

    std::string id(identifier());

    std::list<std::string> args = arguments();

    RedirectionPacket input_redirection = input_file();
    RedirectionPacket output_redirection = output_file();
    
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

    bool in_background = background();

    Command *cmd = new Command(id, args, input_str, input_redirection.type,
                                    output_str, output_redirection.type, in_background);
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

    std::list<std::string>::iterator it;
    for (it = args.begin(); it != args.end(); it++) {
        std::string current_str = (*it);

        // Handle whild characters with glob function
        if ((current_str.find('*') != std::string::npos) || (current_str.find('?') != std::string::npos)) {

            glob_t matches;

            glob(current_str.c_str(), GLOB_TILDE, nullptr, &matches);

            for (uint i = 0; i < matches.gl_pathc; i++) {
                args.insert(it, std::string(matches.gl_pathv[i]));
            }
            
            // Erase the string that has wild character
            std::list<std::string>::iterator str_to_be_erased_it = it;
            it++; 
            args.erase(str_to_be_erased_it);

            globfree(&matches);
        }
    }

    return args;
}

RedirectionPacket Parser::input_file() {
    RedirectionPacket input_redirection;
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

RedirectionPacket Parser::output_file() {
    RedirectionPacket output_redirection;
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

bool Parser::background() {
    skip_whitespaces();

    if (lookahead == '&') {
        consume('&');
        return true;
    }

    return false;
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